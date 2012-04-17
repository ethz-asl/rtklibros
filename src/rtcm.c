/*------------------------------------------------------------------------------
* rtcm.c : rtcm functions
*
*          Copyright (C) 2009-2010 by T.TAKASU, All rights reserved.
*
* references :
*     [1] RTCM Recommended Standards for Differential GNSS (Global Navigation
*         Satellite Systems) Service version 2.3, August 20, 2001
*     [2] RTCM Standard 10403.1 for Differential GNSS (Global Navigation
*         Satellite Systems) Services - Version 3, Octobar 27, 2006
*     [3] RTCM 10403.1-Amendment 3, Amendment 3 to RTCM Standard 10403.1
*     [4] RTCM Paper, April 12, 2010, Proposed SSR Messages for SV Orbit Clock,
*         Code Biases, URA
*     [5] RTCM Paper 012-2009-SC104-528, January 28, 2009 (previous ver of [4])
*     [6] RTCM Paper 012-2009-SC104-582, February 2, 2010 (previous ver of [4])
*
* version : $Revision:$ $Date:$
* history : 2009/04/10 1.0  new
*           2009/06/29 1.1  support type 1009-1012 to get synchronous-gnss-flag
*           2009/12/04 1.2  support type 1010,1012,1020
*           2010/07/15 1.3  support type 1057-1068 for ssr corrections
*                           support type 1007,1008,1033 for antenna info
*           2010/09/08 1.4  fix problem of ephemeris and ssr sequence upset
*                           (2.4.0_p8)
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

/* constants -----------------------------------------------------------------*/

#define RTCM2PREAMB 0x66        /* rtcm ver.2 frame preamble */
#define RTCM3PREAMB 0xD3        /* rtcm ver.3 frame preamble */
#define PRUNIT_GPS  299792.458  /* rtcm ver.3 unit of gps pseudorange (m) */
#define PRUNIT_GLO  599584.916  /* rtcm ver.3 unit of glonass pseudorange (m) */

static const double ssrudint[16]={ /* ssr update intervals */
    1,2,5,10,15,30,60,120,240,300,600,900,1800,3600,7200,10800
};
/* get sign-magnitude bits ---------------------------------------------------*/
static double getbitg(const unsigned char *buff, int pos, int len)
{
    double value=getbitu(buff,pos+1,len-1);
    return getbitu(buff,pos,1)?-value:value;
}
/* adjust hourly rollover of rtcm 2 time -------------------------------------*/
static void adjhour(rtcm_t *rtcm, double zcnt)
{
    double tow,hour,sec;
    int week;
    
    /* if no time, get cpu time */
    if (rtcm->time.time==0) rtcm->time=utc2gpst(timeget());
    tow=time2gpst(rtcm->time,&week);
    hour=floor(tow/3600.0);
    sec=tow-hour*3600.0;
    if      (zcnt<sec-1800.0) zcnt+=3600.0;
    else if (zcnt>sec+1800.0) zcnt-=3600.0;
    rtcm->time=gpst2time(week,hour*3600+zcnt);
}
/* adjust weekly rollover of gps time ----------------------------------------*/
static void adjweek(rtcm_t *rtcm, double tow)
{
    double tow_p;
    int week;
    
    /* if no time, get cpu time */
    if (rtcm->time.time==0) rtcm->time=utc2gpst(timeget());
    tow_p=time2gpst(rtcm->time,&week);
    if      (tow<tow_p-302400.0) tow+=604800.0;
    else if (tow>tow_p+302400.0) tow-=604800.0;
    rtcm->time=gpst2time(week,tow);
}
/* adjust daily rollover of glonass time -------------------------------------*/
static void adjday_glot(rtcm_t *rtcm, double tod)
{
    gtime_t time;
    double tow,tod_p;
    int week;
    
    if (rtcm->time.time==0) rtcm->time=utc2gpst(timeget());
    time=timeadd(gpst2utc(rtcm->time),10800.0); /* glonass time */
    tow=time2gpst(time,&week);
    tod_p=fmod(tow,86400.0); tow-=tod_p;
    if      (tod<tod_p-43200.0) tod+=86400.0;
    else if (tod>tod_p+43200.0) tod-=86400.0;
    time=gpst2time(week,tow+tod);
    rtcm->time=utc2gpst(timeadd(time,-10800.0));
}
/* adjust carrier-phase rollover ---------------------------------------------*/
static double adjcp(rtcm_t *rtcm, int sat, int freq, double cp)
{
    if (rtcm->cp[sat-1][freq]==0.0) ;
    else if (cp<rtcm->cp[sat-1][freq]-750.0) cp+=1500.0;
    else if (cp>rtcm->cp[sat-1][freq]+750.0) cp-=1500.0;
    rtcm->cp[sat-1][freq]=cp;
    return cp;
}
/* loss-of-lock indicator ----------------------------------------------------*/
static int lossoflock(rtcm_t *rtcm, int sat, int freq, int lock)
{
    int lli=lock<rtcm->lock[sat-1][freq]-lock;
    rtcm->lock[sat-1][freq]=lock;
    return lli;
}
/* s/n ratio -----------------------------------------------------------------*/
static unsigned char snratio(double snr)
{
    return (unsigned char)(snr<=0.0||255.5<=snr?0.0:snr*4.0+0.5);
}
/* get observation data index ------------------------------------------------*/
static int obsindex(obs_t *obs, gtime_t time, int sat)
{
    int i,j;
    
    for (i=0;i<obs->n;i++) {
        if (obs->data[i].sat==sat) return i; /* field already exists */
    }
    if (i>=MAXOBS) return -1; /* overflow */
    
    /* add new field */
    obs->data[i].time=time;
    obs->data[i].sat=sat;
    for (j=0;j<NFREQ;j++) {
        obs->data[i].L[j]=obs->data[i].P[j]=0.0;
        obs->data[i].D[j]=0.0;
        obs->data[i].SNR[j]=obs->data[i].LLI[j]=obs->data[i].code[j]=0;
    }
    obs->n++;
    return i;
}
/* decode type 1/9: differential gps correction/partial correction set -------*/
static int decode_type1(rtcm_t *rtcm)
{
    int i=48,fact,udre,prn,sat,iod;
    double prc,rrc;
    
    trace(4,"decode_type1: len=%d\n",rtcm->len);
    
    while (i+40<=rtcm->len*8) {
        fact=getbitu(rtcm->buff,i, 1); i+= 1;
        udre=getbitu(rtcm->buff,i, 2); i+= 2;
        prn =getbitu(rtcm->buff,i, 5); i+= 5;
        prc =getbits(rtcm->buff,i,16); i+=16;
        rrc =getbits(rtcm->buff,i, 8); i+= 8;
        iod =getbits(rtcm->buff,i, 8); i+= 8;
        if (prn==0) prn=32;
        if (prc==0x80000000||rrc==0xFFFF8000) {
            trace(2,"rtcm2 1 prc/rrc indicates satellite problem: prn=%d\n",prn);
            continue;
        }
        if (rtcm->dgps) {
            sat=satno(SYS_GPS,prn);
            rtcm->dgps[sat-1].t0=rtcm->time;
            rtcm->dgps[sat-1].prc=prc*(fact?0.32:0.02);
            rtcm->dgps[sat-1].rrc=rrc*(fact?0.032:0.002);
            rtcm->dgps[sat-1].iod=iod;
            rtcm->dgps[sat-1].udre=udre;
        }
    }
    return 7;
}
/* decode type 3: reference station parameter --------------------------------*/
static int decode_type3(rtcm_t *rtcm)
{
    int i=48;
    
    trace(4,"decode_type3: len=%d\n",rtcm->len);
    
    if (i+96<=rtcm->len*8) {
        rtcm->sta.pos[0]=getbits(rtcm->buff,i,32)*0.01; i+=32;
        rtcm->sta.pos[1]=getbits(rtcm->buff,i,32)*0.01; i+=32;
        rtcm->sta.pos[2]=getbits(rtcm->buff,i,32)*0.01;
    }
    else {
        trace(2,"rtcm2 3 length error: len=%d\n",rtcm->len);
        return -1;
    }
    return 5;
}
/* decode type 14: gps time of week ------------------------------------------*/
static int decode_type14(rtcm_t *rtcm)
{
    double zcnt;
    int i=48,week,hour,leaps;
    
    trace(4,"decode_type14: len=%d\n",rtcm->len);
    
    zcnt=getbitu(rtcm->buff,24,13);
    if (i+24<=rtcm->len*8) {
        week =getbitu(rtcm->buff,i,10); i+=10;
        hour =getbitu(rtcm->buff,i, 8); i+= 8;
        leaps=getbitu(rtcm->buff,i, 6);
    }
    else {
        trace(2,"rtcm2 14 length error: len=%d\n",rtcm->len);
        return -1;
    }
    rtcm->time=gpst2time(week,hour*3600.0+zcnt*0.6);
    rtcm->nav.leaps=leaps;
    return 6;
}
/* decode type 16: gps special message ---------------------------------------*/
static int decode_type16(rtcm_t *rtcm)
{
    int i=48,n=0;
    
    trace(4,"decode_type16: len=%d\n",rtcm->len);
    
    while (i+8<=rtcm->len*8&&n<90) {
        rtcm->msg[n++]=getbitu(rtcm->buff,i,8); i+=8;
    }
    rtcm->msg[n]='\0';
    
    trace(3,"rtcm2 16 message: %s\n",rtcm->msg);
    return 9;
}
/* decode type 17: gps ephemerides -------------------------------------------*/
static int decode_type17(rtcm_t *rtcm)
{
    eph_t eph={0};
    double toc,sqrtA;
    int i=48,week,prn,sat;
    
    trace(4,"decode_type17: len=%d\n",rtcm->len);
    
    if (i+480<=rtcm->len*8) {
        week      =getbitu(rtcm->buff,i,10);              i+=10;
        eph.idot  =getbits(rtcm->buff,i,14)*P2_43*SC2RAD; i+=14;
        eph.iode  =getbitu(rtcm->buff,i, 8);              i+= 8;
        toc       =getbitu(rtcm->buff,i,16)*16.0;         i+=16;
        eph.f1    =getbits(rtcm->buff,i,16)*P2_43;        i+=16;
        eph.f2    =getbits(rtcm->buff,i, 8)*P2_55;        i+= 8;
        eph.crs   =getbits(rtcm->buff,i,16)*P2_5;         i+=16;
        eph.deln  =getbits(rtcm->buff,i,16)*P2_43*SC2RAD; i+=16;
        eph.cuc   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.e     =getbitu(rtcm->buff,i,32)*P2_33;        i+=32;
        eph.cus   =getbits(rtcm->buff,i,16);              i+=16;
        sqrtA     =getbitu(rtcm->buff,i,32)*P2_19;        i+=32;
        eph.toes  =getbitu(rtcm->buff,i,16);              i+=16;
        eph.OMG0  =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.cic   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.i0    =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.cis   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.omg   =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.crc   =getbits(rtcm->buff,i,16)*P2_5;         i+=16;
        eph.OMGd  =getbits(rtcm->buff,i,24)*P2_43*SC2RAD; i+=24;
        eph.M0    =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.iodc  =getbitu(rtcm->buff,i,10);              i+=10;
        eph.f0    =getbits(rtcm->buff,i,22)*P2_31;        i+=22;
        prn       =getbitu(rtcm->buff,i, 5);              i+= 5+3;
        eph.tgd[0]=getbits(rtcm->buff,i, 8)*P2_31;        i+= 8;
        eph.code  =getbitu(rtcm->buff,i, 2);              i+= 2;
        eph.sva   =getbitu(rtcm->buff,i, 4);              i+= 4;
        eph.svh   =getbitu(rtcm->buff,i, 6);              i+= 6;
        eph.flag  =getbitu(rtcm->buff,i, 1);
    }
    else {
        trace(2,"rtcm2 17 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (prn==0) prn=32;
    sat=satno(SYS_GPS,prn);
    eph.sat=sat;
    eph.week=adjgpsweek(week);
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=rtcm->time;
    eph.A=sqrtA*sqrtA;
    rtcm->nav.eph[sat-1]=eph;
    rtcm->ephsat=sat;
    return 2;
}
/* decode type 18: rtk uncorrected carrier-phase -----------------------------*/
static int decode_type18(rtcm_t *rtcm)
{
    gtime_t time;
    double usec,cp,tt;
    int i=48,index,freq,sync=1,code,sys,prn,sat,loss;
    
    trace(4,"decode_type18: len=%d\n",rtcm->len);
    
    if (i+24<=rtcm->len*8) {
        freq=getbitu(rtcm->buff,i, 2); i+= 2+2;
        usec=getbitu(rtcm->buff,i,20); i+=20;
    }
    else {
        trace(2,"rtcm2 18 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (freq&0x1) {
        trace(2,"rtcm2 18 not supported frequency: freq=%d\n",freq);
        return -1;
    }
    freq>>=1;
    
    while (i+48<=rtcm->len*8&&rtcm->obs.n<MAXOBS) {
        sync=getbitu(rtcm->buff,i, 1); i+= 1;
        code=getbitu(rtcm->buff,i, 1); i+= 1;
        sys =getbitu(rtcm->buff,i, 1); i+= 1;
        prn =getbitu(rtcm->buff,i, 5); i+= 5+3;
        loss=getbitu(rtcm->buff,i, 5); i+= 5;
        cp  =getbits(rtcm->buff,i,32); i+=32;
        if (prn==0) prn=32;
        if (!(sat=satno(sys?SYS_GLO:SYS_GPS,prn))) {
            trace(2,"rtcm2 18 satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        time=timeadd(rtcm->time,usec*1E-6);
        if (sys) time=utc2gpst(time); /* convert glonass time to gpst */
        
        tt=timediff(rtcm->obs.data[0].time,time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,time,sat))>=0) {
            rtcm->obs.data[index].L[freq]=-cp/256.0;
            rtcm->obs.data[index].LLI[freq]=rtcm->loss[sat-1][freq]!=loss;
            rtcm->obs.data[index].code[freq]=
                !freq?(code?CODE_L1P:CODE_L1C):(code?CODE_L2P:CODE_L2C);
            rtcm->loss[sat-1][freq]=loss;
        }
    }
    rtcm->obsflag=!sync;
    return sync?0:1;
}
/* decode type 19: rtk uncorrected pseudorange -------------------------------*/
static int decode_type19(rtcm_t *rtcm)
{
    gtime_t time;
    double usec,pr,tt;
    int i=48,index,freq,sync=1,code,sys,prn,sat;
    
    trace(4,"decode_type19: len=%d\n",rtcm->len);
    
    if (i+24<=rtcm->len*8) {
        freq=getbitu(rtcm->buff,i, 2); i+= 2+2;
        usec=getbitu(rtcm->buff,i,20); i+=20;
    }
    else {
        trace(2,"rtcm2 19 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (freq&0x1) {
        trace(2,"rtcm2 19 not supported frequency: freq=%d\n",freq);
        return -1;
    }
    freq>>=1;
    
    while (i+48<=rtcm->len*8&&rtcm->obs.n<MAXOBS) {
        sync=getbitu(rtcm->buff,i, 1); i+= 1;
        code=getbitu(rtcm->buff,i, 1); i+= 1;
        sys =getbitu(rtcm->buff,i, 1); i+= 1;
        prn =getbitu(rtcm->buff,i, 5); i+= 5+8;
        pr  =getbitu(rtcm->buff,i,32); i+=32;
        if (prn==0) prn=32;
        if (!(sat=satno(sys?SYS_GLO:SYS_GPS,prn))) {
            trace(2,"rtcm2 19 satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        time=timeadd(rtcm->time,usec*1E-6);
        if (sys) time=utc2gpst(time); /* convert glonass time to gpst */
        
        tt=timediff(rtcm->obs.data[0].time,time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,time,sat))>=0) {
            rtcm->obs.data[index].P[freq]=pr*0.02;
            rtcm->obs.data[index].code[freq]=
                !freq?(code?CODE_L1P:CODE_L1C):(code?CODE_L2P:CODE_L2C);
        }
    }
    rtcm->obsflag=!sync;
    return sync?0:1;
}
/* decode type 22: extended reference station parameter ----------------------*/
static int decode_type22(rtcm_t *rtcm)
{
    double del[2][3]={{0}},hgt=0.0;
    int i=48,j,noh;
    
    trace(4,"decode_type22: len=%d\n",rtcm->len);
    
    if (i+24<=rtcm->len*8) {
        del[0][0]=getbits(rtcm->buff,i,8)/25600.0; i+=8;
        del[0][1]=getbits(rtcm->buff,i,8)/25600.0; i+=8;
        del[0][2]=getbits(rtcm->buff,i,8)/25600.0; i+=8;
    }
    else {
        trace(2,"rtcm2 22 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (i+24<=rtcm->len*8) {
        i+=5; noh=getbits(rtcm->buff,i,1); i+=1;
        hgt=noh?0.0:getbitu(rtcm->buff,i,18)/25600.0;
        i+=18;
    }
    if (i+24<=rtcm->len*8) {
        del[1][0]=getbits(rtcm->buff,i,8)/1600.0; i+=8;
        del[1][1]=getbits(rtcm->buff,i,8)/1600.0; i+=8;
        del[1][2]=getbits(rtcm->buff,i,8)/1600.0;
    }
    rtcm->sta.deltype=1; /* xyz */
    for (j=0;j<3;j++) rtcm->sta.del[j]=del[0][j];
    rtcm->sta.hgt=hgt;
    return 5;
}
/* decode type 23: antenna type definition record ----------------------------*/
static int decode_type23(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 24: antenna reference point (arp) -----------------------------*/
static int decode_type24(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 31: differential glonass correction ---------------------------*/
static int decode_type31(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 32: differential glonass reference station parameters ---------*/
static int decode_type32(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 34: glonass partial differential correction set ---------------*/
static int decode_type34(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 36: glonass special message -----------------------------------*/
static int decode_type36(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 37: gnss system time offset -----------------------------------*/
static int decode_type37(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 59: proprietary message ---------------------------------------*/
static int decode_type59(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 1001-1004 message header --------------------------------------*/
static int decode_head1001(rtcm_t *rtcm, int *sync)
{
    double tow;
    int i=24,staid,nsat,type;
    
    type=getbitu(rtcm->buff,i,12); i+=12;
    
    if (i+52<=rtcm->len*8) {
        staid=getbitu(rtcm->buff,i,12);       i+=12;
        tow  =getbitu(rtcm->buff,i,30)*0.001; i+=30;
        *sync=getbitu(rtcm->buff,i, 1);       i+= 1;
        nsat =getbitu(rtcm->buff,i, 5);
    }
    else {
        trace(2,"rtcm3 %d length error: len=%d\n",type,rtcm->len);
        return -1;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm3 %d station id invalid: %d %d\n",type,staid,rtcm->staid);
        return -1;
    }
    adjweek(rtcm,tow);
    
    trace(4,"decode_head1001: time=%s nsat=%d sync=%d\n",time_str(rtcm->time,1),
          nsat,*sync);
    
    return nsat;
}
/* decode type 1001: L1-only gps rtk observation -----------------------------*/
static int decode_type1001(rtcm_t *rtcm)
{
    int sync;
    if (decode_head1001(rtcm,&sync)<0) return -1;
    return sync?0:1;
}
/* decode type 1002: extended L1-only gps rtk observables --------------------*/
static int decode_type1002(rtcm_t *rtcm)
{
    double pr1,cnr1,tt,cp1;
    int i=24+64,j,index,nsat,sync,prn,code,sat,ppr1,lock1,amb,sys;
    
    if ((nsat=decode_head1001(rtcm,&sync))<0) return -1;
    
    for (j=0;j<nsat&&rtcm->obs.n<MAXOBS&&i+74<=rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 6); i+= 6;
        code =getbitu(rtcm->buff,i, 1); i+= 1;
        pr1  =getbitu(rtcm->buff,i,24); i+=24;
        ppr1 =getbits(rtcm->buff,i,20); i+=20;
        lock1=getbitu(rtcm->buff,i, 7); i+= 7;
        amb  =getbitu(rtcm->buff,i, 8); i+= 8;
        cnr1 =getbitu(rtcm->buff,i, 8); i+= 8;
        if (prn<40) {
            sys=SYS_GPS;
        }
        else {
            sys=SYS_SBS; prn+=80;
        }
        if (!(sat=satno(sys,prn))) {
            trace(2,"rtcm3 1002 satellite number error: prn=%d\n",prn);
            continue;
        }
        tt=timediff(rtcm->obs.data[0].time,rtcm->time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,rtcm->time,sat))<0) continue;
        pr1=pr1*0.02+amb*PRUNIT_GPS;
        if (ppr1!=0xFFF80000) {
            rtcm->obs.data[index].P[0]=pr1;
            cp1=adjcp(rtcm,sat,0,ppr1*0.0005/lam[0]);
            rtcm->obs.data[index].L[0]=pr1/lam[0]+cp1;
        }
        rtcm->obs.data[index].LLI[0]=lossoflock(rtcm,prn,0,lock1);
        rtcm->obs.data[index].SNR[0]=snratio(cnr1*0.25);
        rtcm->obs.data[index].code[0]=code?CODE_L1P:CODE_L1C;
    }
    return sync?0:1;
}
/* decode type 1003: L1&L2 gps rtk observables -------------------------------*/
static int decode_type1003(rtcm_t *rtcm)
{
    int sync;
    if (decode_head1001(rtcm,&sync)<0) return -1;
    return sync?0:1;
}
/* decode type 1004: extended L1&L2 gps rtk observables ----------------------*/
static int decode_type1004(rtcm_t *rtcm)
{
    double pr1,cnr1,cnr2,tt,cp1,cp2;
    int i=24+64,j,index,nsat,sync,prn,sat,code1,code2,pr21,ppr1,ppr2;
    int lock1,lock2,amb,sys;
    
    if ((nsat=decode_head1001(rtcm,&sync))<0) return -1;
    
    for (j=0;j<nsat&&rtcm->obs.n<MAXOBS&&i+125<=rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 6); i+= 6;
        code1=getbitu(rtcm->buff,i, 1); i+= 1;
        pr1  =getbitu(rtcm->buff,i,24); i+=24;
        ppr1 =getbits(rtcm->buff,i,20); i+=20;
        lock1=getbitu(rtcm->buff,i, 7); i+= 7;
        amb  =getbitu(rtcm->buff,i, 8); i+= 8;
        cnr1 =getbitu(rtcm->buff,i, 8); i+= 8;
        code2=getbitu(rtcm->buff,i, 2); i+= 2;
        pr21 =getbits(rtcm->buff,i,14); i+=14;
        ppr2 =getbits(rtcm->buff,i,20); i+=20;
        lock2=getbitu(rtcm->buff,i, 7); i+= 7;
        cnr2 =getbitu(rtcm->buff,i, 8); i+= 8;
        if (prn<40) {
            sys=SYS_GPS;
        }
        else {
            sys=SYS_SBS; prn+=80;
        }
        if (!(sat=satno(sys,prn))) {
            trace(2,"rtcm3 1004 satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        tt=timediff(rtcm->obs.data[0].time,rtcm->time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,rtcm->time,sat))<0) continue;
        pr1=pr1*0.02+amb*PRUNIT_GPS;
        if (ppr1!=0xFFF80000) {
            rtcm->obs.data[index].P[0]=pr1;
            cp1=adjcp(rtcm,sat,0,ppr1*0.0005/lam[0]);
            rtcm->obs.data[index].L[0]=pr1/lam[0]+cp1;
        }
        rtcm->obs.data[index].LLI[0]=lossoflock(rtcm,prn,0,lock1);
        rtcm->obs.data[index].SNR[0]=snratio(cnr1*0.25);
        rtcm->obs.data[index].code[0]=code1?CODE_L1P:CODE_L1C;
        
        if (pr21!=0xFFFF2000) {
            rtcm->obs.data[index].P[1]=pr1+pr21*0.02;
        }
        if (ppr2!=0xFFF80000) {
            cp2=adjcp(rtcm,sat,1,ppr2*0.0005/lam[1]);
            rtcm->obs.data[index].L[1]=pr1/lam[1]+cp2;
        }
        rtcm->obs.data[index].LLI[1]=lossoflock(rtcm,prn,1,lock2);
        rtcm->obs.data[index].SNR[1]=snratio(cnr2*0.25);
        rtcm->obs.data[index].code[1]=code2?CODE_L2P:CODE_L2C;
    }
    rtcm->obsflag=!sync;
    return sync?0:1;
}
/* get signed 38bit field ----------------------------------------------------*/
static double getbits_38(const unsigned char *buff, int pos)
{
    return (double)getbits(buff,pos,32)*64.0+getbitu(buff,pos+32,6);
}
/* decode type 1005: stationary rtk reference station arp --------------------*/
static int decode_type1005(rtcm_t *rtcm)
{
    double rr[3];
    int i=24+12,j,staid,itrf;
    
    if (i+140==rtcm->len*8) {
        staid=getbitu(rtcm->buff,i,12); i+=12;
        itrf =getbitu(rtcm->buff,i, 6); i+= 6+4;
        rr[0]=getbits_38(rtcm->buff,i); i+=38+2;
        rr[1]=getbits_38(rtcm->buff,i); i+=38+2;
        rr[2]=getbits_38(rtcm->buff,i);
    }
    else {
        trace(2,"rtcm3 1005 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm3 1005 station id changed: %d->%d\n",rtcm->staid,staid);
    }
    rtcm->staid=staid;
    rtcm->sta.deltype=0; /* xyz */
    for (j=0;j<3;j++) {
        rtcm->sta.pos[j]=rr[j]*0.0001;
        rtcm->sta.del[j]=0.0;
    }
    rtcm->sta.hgt=0.0;
    rtcm->sta.itrf=itrf;
    return 5;
}
/* decode type 1006: stationary rtk reference station arp with height --------*/
static int decode_type1006(rtcm_t *rtcm)
{
    double rr[3],anth;
    int i=24+12,j,staid,itrf;
    
    if (i+156<=rtcm->len*8) {
        staid=getbitu(rtcm->buff,i,12); i+=12;
        itrf =getbitu(rtcm->buff,i, 6); i+= 6+4;
        rr[0]=getbits_38(rtcm->buff,i); i+=38+2;
        rr[1]=getbits_38(rtcm->buff,i); i+=38+2;
        rr[2]=getbits_38(rtcm->buff,i); i+=38;
        anth =getbitu(rtcm->buff,i,16);
    }
    else {
        trace(2,"rtcm3 1006 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm3 1006 station id changed: %d->%d\n",rtcm->staid,staid);
    }
    rtcm->staid=staid;
    rtcm->sta.deltype=1; /* xyz */
    for (j=0;j<3;j++) {
        rtcm->sta.pos[j]=rr[j]*0.0001;
        rtcm->sta.del[j]=0.0;
    }
    rtcm->sta.hgt=anth*0.0001;
    rtcm->sta.itrf=itrf;
    return 5;
}
/* decode type 1007: antenna descriptor --------------------------------------*/
static int decode_type1007(rtcm_t *rtcm)
{
    char des[32]="";
    int i=24+12,j,staid,n,setup;
    
    n=getbitu(rtcm->buff,i+12,8);
    
    if (i+28+8*n<=rtcm->len*8) {
        staid=getbitu(rtcm->buff,i,12); i+=12+8;
        for (j=0;j<n&&j<31;j++) {
            des[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
        setup=getbitu(rtcm->buff,i, 8);
    }
    else {
        trace(2,"rtcm3 1007 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm3 1007 station id changed: %d->%d\n",rtcm->staid,staid);
    }
    rtcm->staid=staid;
    strncpy(rtcm->sta.antdes,des,n); rtcm->sta.antdes[n]='\0';
    rtcm->sta.antsetup=setup;
    rtcm->sta.antsno[0]='\0';
    return 5;
}
/* decode type 1008: antenna descriptor & serial number ----------------------*/
static int decode_type1008(rtcm_t *rtcm)
{
    char des[32]="",sno[32]="";
    int i=24+12,j,staid,n,m,setup;
    
    n=getbitu(rtcm->buff,i+12,8);
    m=getbitu(rtcm->buff,i+28+8*n,8);
    
    if (i+36+8*(n+m)<=rtcm->len*8) {
        staid=getbitu(rtcm->buff,i,12); i+=12+8;
        for (j=0;j<n&&j<31;j++) {
            des[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
        setup=getbitu(rtcm->buff,i, 8); i+=8+8;
        for (j=0;j<m&&j<31;j++) {
            sno[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
    }
    else {
        trace(2,"rtcm3 1008 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm3 1008 station id changed: %d->%d\n",rtcm->staid,staid);
    }
    rtcm->staid=staid;
    strncpy(rtcm->sta.antdes,des,n); rtcm->sta.antdes[n]='\0';
    rtcm->sta.antsetup=setup;
    strncpy(rtcm->sta.antsno,sno,m); rtcm->sta.antsno[m]='\0';
    return 5;
}
/* decode type 1009-1012 message header --------------------------------------*/
static int decode_head1009(rtcm_t *rtcm, int *sync)
{
    double tod;
    int i=24,staid,nsat,type;
    
    type=getbitu(rtcm->buff,i,12); i+=12;
    
    if (i+49<=rtcm->len*8) {
        staid=getbitu(rtcm->buff,i,12);       i+=12;
        tod  =getbitu(rtcm->buff,i,27)*0.001; i+=27; /* sec in a day */
        *sync=getbitu(rtcm->buff,i, 1);       i+= 1;
        nsat =getbitu(rtcm->buff,i, 5);
    }
    else {
        trace(2,"rtcm3 %d length error: len=%d\n",type,rtcm->len);
        return -1;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm3 %d station id invalid: %d %d\n",type,staid,rtcm->staid);
        return -1;
    }
    adjday_glot(rtcm,tod);
    
    trace(4,"decode_head1009: time=%s nsat=%d sync=%d\n",time_str(rtcm->time,1),
          nsat,*sync);
    
    return nsat;
}
/* decode type 1009: L1-only glonass rtk observables -------------------------*/
static int decode_type1009(rtcm_t *rtcm)
{
    int sync;
    if (decode_head1009(rtcm,&sync)<0) return -1;
    return sync?0:1;
}
/* decode type 1010: extended L1-only glonass rtk observables ----------------*/
static int decode_type1010(rtcm_t *rtcm)
{
    double pr1,cnr1,tt,cp1,lam1;
    int i=24+61,j,index,nsat,sync,prn,sat,code,freq,ppr1,lock1,amb,sys=SYS_GLO;
    
    if ((nsat=decode_head1009(rtcm,&sync))<0) return -1;
    
    for (j=0;j<nsat&&rtcm->obs.n<MAXOBS&&i+79<=rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 6); i+= 6;
        code =getbitu(rtcm->buff,i, 1); i+= 1;
        freq =getbitu(rtcm->buff,i, 5); i+= 5;
        pr1  =getbitu(rtcm->buff,i,25); i+=25;
        ppr1 =getbits(rtcm->buff,i,20); i+=20;
        lock1=getbitu(rtcm->buff,i, 7); i+= 7;
        amb  =getbitu(rtcm->buff,i, 7); i+= 7;
        cnr1 =getbitu(rtcm->buff,i, 8); i+= 8;
        if (!(sat=satno(sys,prn))) {
            trace(2,"rtcm3 1010 satellite number error: prn=%d\n",prn);
            continue;
        }
        tt=timediff(rtcm->obs.data[0].time,rtcm->time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,rtcm->time,sat))<0) continue;
        pr1=pr1*0.02+amb*PRUNIT_GLO;
        if (ppr1!=0xFFF80000) {
            rtcm->obs.data[index].P[0]=pr1;
            lam1=CLIGHT/(FREQ1_GLO+DFRQ1_GLO*(freq-7));
            cp1=adjcp(rtcm,sat,0,ppr1*0.0005/lam1);
            rtcm->obs.data[index].L[0]=pr1/lam1+cp1;
        }
        rtcm->obs.data[index].LLI[0]=lossoflock(rtcm,prn,0,lock1);
        rtcm->obs.data[index].SNR[0]=snratio(cnr1*0.25);
        rtcm->obs.data[index].code[0]=code?CODE_L1P:CODE_L1C;
    }
    return sync?0:1;
}
/* decode type 1011: L1&L2 glonass rtk observables ---------------------------*/
static int decode_type1011(rtcm_t *rtcm)
{
    int sync;
    if (decode_head1009(rtcm,&sync)<0) return -1;
    return sync?0:1;
}
/* decode type 1012: extended L1&L2 glonass rtk observables ------------------*/
static int decode_type1012(rtcm_t *rtcm)
{
    double pr1,cnr1,cnr2,tt,cp1,cp2,lam1,lam2;
    int i=24+61,j,index,nsat,sync,prn,sat,freq,code1,code2,pr21,ppr1,ppr2;
    int lock1,lock2,amb,sys=SYS_GLO;
    
    if ((nsat=decode_head1009(rtcm,&sync))<0) return -1;
    
    for (j=0;j<nsat&&rtcm->obs.n<MAXOBS&&i+130<=rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 6); i+= 6;
        code1=getbitu(rtcm->buff,i, 1); i+= 1;
        freq =getbitu(rtcm->buff,i, 5); i+= 5;
        pr1  =getbitu(rtcm->buff,i,25); i+=25;
        ppr1 =getbits(rtcm->buff,i,20); i+=20;
        lock1=getbitu(rtcm->buff,i, 7); i+= 7;
        amb  =getbitu(rtcm->buff,i, 7); i+= 7;
        cnr1 =getbitu(rtcm->buff,i, 8); i+= 8;
        code2=getbitu(rtcm->buff,i, 2); i+= 2;
        pr21 =getbits(rtcm->buff,i,14); i+=14;
        ppr2 =getbits(rtcm->buff,i,20); i+=20;
        lock2=getbitu(rtcm->buff,i, 7); i+= 7;
        cnr2 =getbitu(rtcm->buff,i, 8); i+= 8;
        if (!(sat=satno(sys,prn))) {
            trace(2,"rtcm3 1012 satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        tt=timediff(rtcm->obs.data[0].time,rtcm->time);
        if (rtcm->obsflag||fabs(tt)>1E-9) {
            rtcm->obs.n=rtcm->obsflag=0;
        }
        if ((index=obsindex(&rtcm->obs,rtcm->time,sat))<0) continue;
        pr1=pr1*0.02+amb*PRUNIT_GLO;
        if (ppr1!=0xFFF80000) {
            lam1=CLIGHT/(FREQ1_GLO+DFRQ1_GLO*(freq-7));
            rtcm->obs.data[index].P[0]=pr1;
            cp1=adjcp(rtcm,sat,0,ppr1*0.0005/lam1);
            rtcm->obs.data[index].L[0]=pr1/lam1+cp1;
        }
        rtcm->obs.data[index].LLI[0]=lossoflock(rtcm,prn,0,lock1);
        rtcm->obs.data[index].SNR[0]=snratio(cnr1*0.25);
        rtcm->obs.data[index].code[0]=code1?CODE_L1P:CODE_L1C;
        
        if (pr21!=0xFFFF2000) {
            rtcm->obs.data[index].P[1]=pr1+pr21*0.02;
        }
        if (ppr2!=0xFFF80000) {
            lam2=CLIGHT/(FREQ2_GLO+DFRQ2_GLO*(freq-7));
            cp2=adjcp(rtcm,sat,1,ppr2*0.0005/lam2);
            rtcm->obs.data[index].L[1]=pr1/lam2+cp2;
        }
        rtcm->obs.data[index].LLI[1]=lossoflock(rtcm,prn,1,lock2);
        rtcm->obs.data[index].SNR[1]=snratio(cnr2*0.25);
        rtcm->obs.data[index].code[1]=code2?CODE_L2P:CODE_L2C;
    }
    rtcm->obsflag=!sync;
    return sync?0:1;
}
/* decode type 1013: system parameters ---------------------------------------*/
static int decode_type1013(rtcm_t *rtcm)
{
    return 0;
}
/* decode type 1019: gps ephemerides -----------------------------------------*/
static int decode_type1019(rtcm_t *rtcm)
{
    eph_t eph={0};
    double toc,sqrtA;
    int i=24+12,prn,sat,week,sys=SYS_GPS;
    
    if (i+476<=rtcm->len*8) {
        prn       =getbitu(rtcm->buff,i, 6);              i+= 6;
        week      =getbitu(rtcm->buff,i,10);              i+=10;
        eph.sva   =getbitu(rtcm->buff,i, 4);              i+= 4;
        eph.code  =getbitu(rtcm->buff,i, 2);              i+= 2;
        eph.idot  =getbits(rtcm->buff,i,14)*P2_43*SC2RAD; i+=14;
        eph.iode  =getbitu(rtcm->buff,i, 8);              i+= 8;
        toc       =getbitu(rtcm->buff,i,16)*16.0;         i+=16;
        eph.f2    =getbits(rtcm->buff,i, 8)*P2_55;        i+= 8;
        eph.f1    =getbits(rtcm->buff,i,16)*P2_43;        i+=16;
        eph.f0    =getbits(rtcm->buff,i,22)*P2_31;        i+=22;
        eph.iodc  =getbitu(rtcm->buff,i,10);              i+=10;
        eph.crs   =getbits(rtcm->buff,i,16)*P2_5;         i+=16;
        eph.deln  =getbits(rtcm->buff,i,16)*P2_43*SC2RAD; i+=16;
        eph.M0    =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.cuc   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.e     =getbitu(rtcm->buff,i,32)*P2_33;        i+=32;
        eph.cus   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        sqrtA     =getbitu(rtcm->buff,i,32)*P2_19;        i+=32;
        eph.toes  =getbitu(rtcm->buff,i,16)*16.0;         i+=16;
        eph.cic   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.OMG0  =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.cis   =getbits(rtcm->buff,i,16)*P2_29;        i+=16;
        eph.i0    =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.crc   =getbits(rtcm->buff,i,16)*P2_5;         i+=16;
        eph.omg   =getbits(rtcm->buff,i,32)*P2_31*SC2RAD; i+=32;
        eph.OMGd  =getbits(rtcm->buff,i,24)*P2_43*SC2RAD; i+=24;
        eph.tgd[0]=getbits(rtcm->buff,i, 8)*P2_31;        i+= 8;
        eph.svh   =getbitu(rtcm->buff,i, 6);              i+= 6;
        eph.flag  =getbitu(rtcm->buff,i, 1);              i+= 1;
        eph.fit   =getbitu(rtcm->buff,i, 1);
    }
    else {
        trace(2,"rtcm3 1019 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (prn>=40) {
        sys=SYS_SBS; prn+=80;
    }
    trace(4,"decode_type1019: prn=%d iode=%d toe=%.0f\n",prn,eph.iode,eph.toes);
    
    if (!(sat=satno(sys,prn))) {
        trace(2,"rtcm3 1019 satellite number error: prn=%d\n",prn);
        return -1;
    }
    eph.sat=sat;
    eph.week=adjgpsweek(week);
    eph.toe=gpst2time(eph.week,eph.toes);
    eph.toc=gpst2time(eph.week,toc);
    eph.ttr=rtcm->time;
    eph.A=sqrtA*sqrtA;
    if (eph.iode==rtcm->nav.eph[sat-1].iode) return 0; /* unchanged */
    rtcm->nav.eph[sat-1]=eph;
    rtcm->ephsat=sat;
    return 2;
}
/* decode type 1020: glonass ephemerides -------------------------------------*/
static int decode_type1020(rtcm_t *rtcm)
{
    geph_t geph={0};
    double tk_h,tk_m,tk_s,toe,tow,tod,tof;
    int i=24+12,prn,sat,week,tb,bn,sys=SYS_GLO;
    
    if (i+348<=rtcm->len*8) {
        prn        =getbitu(rtcm->buff,i, 6);           i+= 6;
        geph.frq   =getbitu(rtcm->buff,i, 5)-7;         i+= 5+2+2;
        tk_h       =getbitu(rtcm->buff,i, 5);           i+= 5;
        tk_m       =getbitu(rtcm->buff,i, 6);           i+= 6;
        tk_s       =getbitu(rtcm->buff,i, 1)*30.0;      i+= 1;
        bn         =getbitu(rtcm->buff,i, 1);           i+= 1+1;
        tb         =getbitu(rtcm->buff,i, 7);           i+= 7;
        geph.vel[0]=getbitg(rtcm->buff,i,24)*P2_20*1E3; i+=24;
        geph.pos[0]=getbitg(rtcm->buff,i,27)*P2_11*1E3; i+=27;
        geph.acc[0]=getbitg(rtcm->buff,i, 5)*P2_30*1E3; i+= 5;
        geph.vel[1]=getbitg(rtcm->buff,i,24)*P2_20*1E3; i+=24;
        geph.pos[1]=getbitg(rtcm->buff,i,27)*P2_11*1E3; i+=27;
        geph.acc[1]=getbitg(rtcm->buff,i, 5)*P2_30*1E3; i+= 5;
        geph.vel[2]=getbitg(rtcm->buff,i,24)*P2_20*1E3; i+=24;
        geph.pos[2]=getbitg(rtcm->buff,i,27)*P2_11*1E3; i+=27;
        geph.acc[2]=getbitg(rtcm->buff,i, 5)*P2_30*1E3; i+= 5+1;
        geph.gamn  =getbitg(rtcm->buff,i,11)*P2_40;     i+=11+3;
        geph.taun  =getbitg(rtcm->buff,i,22)*P2_30;
    }
    else {
        trace(2,"rtcm3 1020 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (!(sat=satno(sys,prn))) {
        trace(2,"rtcm3 1020 satellite number error: prn=%d\n",prn);
        return -1;
    }
    trace(4,"decode_type1020: prn=%d tk=%02.0f:%02.0f:%02.0f\n",prn,tk_h,tk_m,tk_s);
    
    geph.sat=sat;
    geph.svh=bn;
    geph.iode=tb&0x7F;
    if (rtcm->time.time==0) rtcm->time=utc2gpst(timeget());
    tow=time2gpst(gpst2utc(rtcm->time),&week);
    tod=fmod(tow,86400.0); tow-=tod;
    tof=tk_h*3600.0+tk_m*60.0+tk_s-10800.0; /* lt->utc */
    if      (tof<tod-43200.0) tof+=86400.0;
    else if (tof>tod+43200.0) tof-=86400.0;
    geph.tof=utc2gpst(gpst2time(week,tow+tof));
    toe=tb*900.0-10800.0; /* lt->utc */
    if      (toe<tod-43200.0) toe+=86400.0;
    else if (toe>tod+43200.0) toe-=86400.0;
    geph.toe=utc2gpst(gpst2time(week,tow+toe)); /* utc->gpst */
    
    if (fabs(timediff(geph.toe,rtcm->nav.geph[prn-1].toe))<1.0&&
        geph.svh==rtcm->nav.geph[prn-1].svh) return 0; /* unchanged */
    rtcm->nav.geph[prn-1]=geph;
    rtcm->ephsat=sat;
    return 2;
}
/* decode type 1033: receiver and antenna descriptor -------------------------*/
static int decode_type1033(rtcm_t *rtcm)
{
    char des[32]="",sno[32]="",rec[32]="",ver[32]="",rsn[32]="";
    int i=24+12,j,staid,n,m,n1,n2,n3,setup;
    
    n =getbitu(rtcm->buff,i+12,8);
    m =getbitu(rtcm->buff,i+28+8*n,8);
    n1=getbitu(rtcm->buff,i+36+8*(n+m),8);
    n2=getbitu(rtcm->buff,i+44+8*(n+m+n1),8);
    n3=getbitu(rtcm->buff,i+52+8*(n+m+n1+n2),8);
    
    if (i+60+8*(n+m+n1+n2+n3)<=rtcm->len*8) {
        staid=getbitu(rtcm->buff,i,12); i+=12+8;
        for (j=0;j<n&&j<31;j++) {
            des[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
        setup=getbitu(rtcm->buff,i, 8); i+=8+8;
        for (j=0;j<m&&j<31;j++) {
            sno[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
        i+=8;
        for (j=0;j<n1&&j<31;j++) {
            rec[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
        i+=8;
        for (j=0;j<n2&&j<31;j++) {
            ver[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
        i+=8;
        for (j=0;j<n3&&j<31;j++) {
            rsn[j]=(char)getbitu(rtcm->buff,i,8); i+=8;
        }
    }
    else {
        trace(2,"rtcm3 1033 length error: len=%d\n",rtcm->len);
        return -1;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm3 1033 station id changed: %d->%d\n",rtcm->staid,staid);
    }
    rtcm->staid=staid;
    strncpy(rtcm->sta.antdes, des,n ); rtcm->sta.antdes [n] ='\0';
    rtcm->sta.antsetup=setup;
    strncpy(rtcm->sta.antsno, sno,m ); rtcm->sta.antsno [m] ='\0';
    strncpy(rtcm->sta.rectype,rec,n1); rtcm->sta.rectype[n1]='\0';
    strncpy(rtcm->sta.recver, ver,n2); rtcm->sta.recver [n2]='\0';
    strncpy(rtcm->sta.recsno, rsn,n3); rtcm->sta.recsno [n3]='\0';
    return 5;
}
/* decode type 1057-1062 message header --------------------------------------*/
static int decode_head1057(rtcm_t *rtcm, int *sync, double *udint, int *refd)
{
    double tow;
    int i=24+12,nsat,udi;
    
    if (i+36>rtcm->len*8) return -1;
    tow   =getbitu(rtcm->buff,i,20); i+=20;
    udi   =getbitu(rtcm->buff,i, 4); i+= 4;
    *sync =getbitu(rtcm->buff,i, 1); i+= 1;
    *refd =getbitu(rtcm->buff,i, 1); i+= 1+4; /* satellite ref datum */
    nsat  =getbitu(rtcm->buff,i, 6);
    *udint=ssrudint[udi];
    adjweek(rtcm,tow);
    
    trace(4,"decode_head1057: time=%s nsat=%d sync=%d\n",time_str(rtcm->time,1),
          nsat,*sync);
    
    return nsat;
}
/* decode type 1057: gps orbit corrections -----------------------------------*/
static int decode_type1057(rtcm_t *rtcm)
{
    double udint,deph[3],ddeph[3];
    int i=24+48,j,k,sync,nsat,prn,sat,iode,refd=0;
    
    if ((nsat=decode_head1057(rtcm,&sync,&udint,&refd))<0) {
        trace(2,"rtcm3 1057 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+135<rtcm->len*8;j++) {
        prn     =getbitu(rtcm->buff,i, 6);      i+= 6;
        iode    =getbitu(rtcm->buff,i, 8);      i+= 8;
        deph [0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        deph [1]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        deph [2]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        ddeph[0]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        ddeph[1]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        ddeph[2]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        
        if (!(sat=satno(SYS_GPS,prn))) {
            trace(2,"rtcm3 1057 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].iode=iode;
        rtcm->ssr[sat-1].refd=refd;
        
        for (k=0;k<3;k++) {
            rtcm->ssr[sat-1].deph [k]=deph [k];
            rtcm->ssr[sat-1].ddeph[k]=ddeph[k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1058: gps clock corrections -----------------------------------*/
static int decode_type1058(rtcm_t *rtcm)
{
    double udint,dclk[3];
    int i=24+48,j,k,sync,nsat,prn,sat,rsv=0;
    
    if ((nsat=decode_head1057(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1058 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+76<rtcm->len*8;j++) {
        prn    =getbitu(rtcm->buff,i, 6);      i+= 6;
        dclk[0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        dclk[1]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        dclk[2]=getbits(rtcm->buff,i,27)*2E-8; i+=27;
        
        if (!(sat=satno(SYS_GPS,prn))) {
            trace(2,"rtcm3 1058 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        
        for (k=0;k<3;k++) {
            rtcm->ssr[sat-1].dclk[k]=dclk[k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1059: gps satellite code biases -------------------------------*/
static int decode_type1059(rtcm_t *rtcm)
{
    const int codes[]={
        CODE_L1C,CODE_L1P,CODE_L1W,CODE_L1Y,CODE_L1M,CODE_L2C,CODE_L2D,CODE_L2S,
        CODE_L2L,CODE_L2X,CODE_L2P,CODE_L2W,CODE_L2Y,CODE_L2M,CODE_L5I,CODE_L5Q,
        CODE_L5X
    };
    double udint,bias,cbias[MAXCODE];
    int i=24+48,j,k,mode,sync,nsat,prn,sat,nbias,rsv=0;
    
    if ((nsat=decode_head1057(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1059 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+11<rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 6); i+= 6;
        nbias=getbitu(rtcm->buff,i, 5); i+= 5;
        
        for (k=0;k<MAXCODE;k++) cbias[k]=0.0;
        for (k=0;k<nbias&&i+19<rtcm->len*8;k++) {
            mode=getbitu(rtcm->buff,i, 5);      i+= 5;
            bias=getbits(rtcm->buff,i,14)*0.01; i+=14;
            if (mode<=16) {
                cbias[codes[mode]-1]=(float)bias;
            }
            else {
                trace(2,"rtcm3 1059 not supported mode: mode=%d\n",mode);
            }
        }
        if (!(sat=satno(SYS_GPS,prn))) {
            trace(2,"rtcm3 1059 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        for (k=0;k<MAXCODE;k++) {
            rtcm->ssr[sat-1].cbias[k]=(float)cbias[k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1060: gps combined orbit and clock corrections ----------------*/
static int decode_type1060(rtcm_t *rtcm)
{
    double udint,deph[3],ddeph[3],dclk[3];
    int i=24+48,j,k,nsat,sync,prn,sat,iode,refd=0;
    
    if ((nsat=decode_head1057(rtcm,&sync,&udint,&refd))<0) {
        trace(2,"rtcm3 1060 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+205<rtcm->len*8;j++) {
        prn     =getbitu(rtcm->buff,i, 6);      i+= 6;
        iode    =getbitu(rtcm->buff,i, 8);      i+= 8;
        deph [0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        deph [1]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        deph [2]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        ddeph[0]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        ddeph[1]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        ddeph[2]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        
        dclk [0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        dclk [1]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        dclk [2]=getbits(rtcm->buff,i,27)*2E-8; i+=27;
        
        if (!(sat=satno(SYS_GPS,prn))) {
            trace(2,"rtcm3 1060 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].iode=iode;
        rtcm->ssr[sat-1].refd=refd;
        
        for (k=0;k<3;k++) {
            rtcm->ssr[sat-1].deph [k]=deph [k];
            rtcm->ssr[sat-1].ddeph[k]=ddeph[k];
            rtcm->ssr[sat-1].dclk [k]=dclk [k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1061: gps ura -------------------------------------------------*/
static int decode_type1061(rtcm_t *rtcm)
{
    double udint;
    int i=24+48,j,nsat,sync,prn,sat,ura,rsv=0;
    
    if ((nsat=decode_head1057(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1061 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+12<rtcm->len*8;j++) {
        prn=getbitu(rtcm->buff,i, 6); i+= 6;
        ura=getbitu(rtcm->buff,i, 6); i+= 6;
        
        if (!(sat=satno(SYS_GPS,prn))) {
            trace(2,"rtcm3 1061 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].ura=ura;
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1062: gps high rate clock correction --------------------------*/
static int decode_type1062(rtcm_t *rtcm)
{
    double udint,hrclk;
    int i=24+48,j,nsat,sync,prn,sat,rsv=0;
    
    if ((nsat=decode_head1057(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1062 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+28<rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 6);      i+= 6;
        hrclk=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        
        if (!(sat=satno(SYS_GPS,prn))) {
            trace(2,"rtcm3 1062 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].hrclk=hrclk;
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1063-1068 message header --------------------------------------*/
static int decode_head1063(rtcm_t *rtcm, int *sync, double *udint, int *refd)
{
    double tod;
    int i=24+12,nsat,udi;
    
    if (i+33>rtcm->len*8) return -1;
    tod   =getbitu(rtcm->buff,i,17); i+=17;
    udi   =getbitu(rtcm->buff,i, 4); i+= 4;
    *sync =getbitu(rtcm->buff,i, 1); i+= 1;
    *refd =getbitu(rtcm->buff,i, 1); i+= 1+4; /* reference datum */
    nsat  =getbitu(rtcm->buff,i, 6);
    *udint=ssrudint[udi];
    adjday_glot(rtcm,tod);
    
    trace(4,"decode_head1063: time=%s nsat=%d sync=%d\n",time_str(rtcm->time,1),
          nsat,*sync);
    return nsat;
}
/* decode type 1063: glonass orbit corrections -------------------------------*/
static int decode_type1063(rtcm_t *rtcm)
{
    double udint,deph[3],ddeph[3];
    int i=24+45,j,k,sync,nsat,prn,sat,iode,refd=0;
    
    if ((nsat=decode_head1063(rtcm,&sync,&udint,&refd))<0) {
        trace(2,"rtcm3 1063 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+134<rtcm->len*8;j++) {
        prn     =getbitu(rtcm->buff,i, 5);      i+= 5;
        iode    =getbitu(rtcm->buff,i, 8);      i+= 8;
        deph [0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        deph [1]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        deph [2]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        ddeph[0]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        ddeph[1]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        ddeph[2]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        
        if (!(sat=satno(SYS_GLO,prn))) {
            trace(2,"rtcm3 1063 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].iode=iode;
        rtcm->ssr[sat-1].refd=refd;
        
        for (k=0;k<3;k++) {
            rtcm->ssr[sat-1].deph [k]=deph [k];
            rtcm->ssr[sat-1].ddeph[k]=ddeph[k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1064: glonass clock corrections -------------------------------*/
static int decode_type1064(rtcm_t *rtcm)
{
    double udint,dclk[3];
    int i=24+45,j,k,sync,nsat,prn,sat,rsv=0;
    
    if ((nsat=decode_head1063(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1064 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+75<rtcm->len*8;j++) {
        prn    =getbitu(rtcm->buff,i, 5);      i+= 5;
        dclk[0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        dclk[1]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        dclk[2]=getbits(rtcm->buff,i,27)*2E-8; i+=27;
        
        if (!(sat=satno(SYS_GLO,prn))) {
            trace(2,"rtcm3 1064 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        
        for (k=0;k<3;k++) {
            rtcm->ssr[sat-1].dclk[k]=dclk[k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1065: glonass code biases -------------------------------------*/
static int decode_type1065(rtcm_t *rtcm)
{
    const int codes[]={
        CODE_L1C,CODE_L1P,CODE_L2C,CODE_L2P
    };
    double udint,bias,cbias[MAXCODE];
    int i=24+45,j,k,mode,sync,nsat,prn,sat,nbias,rsv=0;
    
    if ((nsat=decode_head1063(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1065 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+10<rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 5); i+= 5;
        nbias=getbitu(rtcm->buff,i, 5); i+= 5;
        
        for (k=0;k<MAXCODE;k++) cbias[k]=0.0;
        for (k=0;k<nbias&&i+19<rtcm->len*8;k++) {
            mode=getbitu(rtcm->buff,i, 5);      i+= 5;
            bias=getbits(rtcm->buff,i,14)*0.01; i+=14;
            if (mode<=3) {
                cbias[codes[mode]-1]=(float)bias;
            }
            else {
                trace(2,"rtcm3 1065 not supported mode: mode=%d\n",mode);
            }
        }
        if (!(sat=satno(SYS_GLO,prn))) {
            trace(2,"rtcm3 1065 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        for (k=0;k<MAXCODE;k++) {
            rtcm->ssr[sat-1].cbias[k]=(float)cbias[k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1066: glonass combined orbit and clock corrections ------------*/
static int decode_type1066(rtcm_t *rtcm)
{
    double udint,deph[3],ddeph[3],dclk[3];
    int i=24+45,j,k,nsat,sync,prn,sat,iode,refd=0;
    
    if ((nsat=decode_head1063(rtcm,&sync,&udint,&refd))<0) {
        trace(2,"rtcm3 1066 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+204<rtcm->len*8;j++) {
        prn     =getbitu(rtcm->buff,i, 5);      i+= 5;
        iode    =getbitu(rtcm->buff,i, 8);      i+= 8;
        deph [0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        deph [1]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        deph [2]=getbits(rtcm->buff,i,20)*4E-4; i+=20;
        ddeph[0]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        ddeph[1]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        ddeph[2]=getbits(rtcm->buff,i,19)*4E-6; i+=19;
        
        dclk [0]=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        dclk [1]=getbits(rtcm->buff,i,21)*1E-6; i+=21;
        dclk [2]=getbits(rtcm->buff,i,27)*2E-8; i+=27;
        
        if (!(sat=satno(SYS_GLO,prn))) {
            trace(2,"rtcm3 1066 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].iode=iode;
        rtcm->ssr[sat-1].refd=refd;
        
        for (k=0;k<3;k++) {
            rtcm->ssr[sat-1].deph [k]=deph [k];
            rtcm->ssr[sat-1].ddeph[k]=ddeph[k];
            rtcm->ssr[sat-1].dclk [k]=dclk [k];
        }
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1067: glonass ura ---------------------------------------------*/
static int decode_type1067(rtcm_t *rtcm)
{
    double udint;
    int i=24+45,j,nsat,sync,prn,sat,ura,rsv=0;
    
    if ((nsat=decode_head1063(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1067 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+11<rtcm->len*8;j++) {
        prn =getbitu(rtcm->buff,i, 5); i+= 5;
        ura =getbitu(rtcm->buff,i, 6); i+= 6;
        
        if (!(sat=satno(SYS_GLO,prn))) {
            trace(2,"rtcm3 1067 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].ura=ura;
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode type 1068: glonass high rate clock correction ----------------------*/
static int decode_type1068(rtcm_t *rtcm)
{
    double udint,hrclk;
    int i=24+45,j,nsat,sync,prn,sat,rsv=0;
    
    if ((nsat=decode_head1063(rtcm,&sync,&udint,&rsv))<0) {
        trace(2,"rtcm3 1068 length error: len=%d\n",rtcm->len);
        return -1;
    }
    for (j=0;j<nsat&&i+27<rtcm->len*8;j++) {
        prn  =getbitu(rtcm->buff,i, 5);      i+= 5;
        hrclk=getbits(rtcm->buff,i,22)*1E-4; i+=22;
        
        if (!(sat=satno(SYS_GLO,prn))) {
            trace(2,"rtcm3 1068 satellite number error: prn=%d\n",prn);
            continue;
        }
        rtcm->ssr[sat-1].t0=rtcm->time;
        rtcm->ssr[sat-1].udint=udint;
        rtcm->ssr[sat-1].hrclk=hrclk;
        rtcm->ssr[sat-1].update=1;
    }
    return sync?0:10;
}
/* decode rtcm ver.2 message -------------------------------------------------*/
static int decode_rtcm2(rtcm_t *rtcm)
{
    double zcnt;
    int staid,seqno,stah,ret=0,type=getbitu(rtcm->buff,8,6);
    
    trace(3,"decode_rtcm2: type=%2d len=%3d\n",type,rtcm->len);
    
    if ((zcnt=getbitu(rtcm->buff,24,13)*0.6)>=3600.0) {
        trace(2,"rtcm2 modified z-count error: zcnt=%.1f\n",zcnt);
        return -1;
    }
    adjhour(rtcm,zcnt);
    staid=getbitu(rtcm->buff,14,10);
    seqno=getbitu(rtcm->buff,37, 3);
    stah =getbitu(rtcm->buff,45, 3);
    if (seqno-rtcm->seqno!=1&&seqno-rtcm->seqno!=-7) {
        trace(2,"rtcm2 message outage: seqno=%d->%d\n",rtcm->seqno,seqno);
    }
    rtcm->seqno=seqno;
    rtcm->stah =stah;
    
    if (type==3||type==22||type==23||type==24) {
        if (rtcm->staid!=0&&staid!=rtcm->staid) {
           trace(2,"rtcm2 station id changed: %d->%d\n",rtcm->staid,staid);
        }
        rtcm->staid=staid;
    }
    if (rtcm->staid!=0&&staid!=rtcm->staid) {
        trace(2,"rtcm2 station id invalid: %d %d\n",staid,rtcm->staid);
        return -1;
    }
    switch (type) {
        case  1: ret=decode_type1 (rtcm); break;
        case  3: ret=decode_type3 (rtcm); break;
        case  9: ret=decode_type1 (rtcm); break;
        case 14: ret=decode_type14(rtcm); break;
        case 16: ret=decode_type16(rtcm); break;
        case 17: ret=decode_type17(rtcm); break;
        case 18: ret=decode_type18(rtcm); break;
        case 19: ret=decode_type19(rtcm); break;
        case 22: ret=decode_type22(rtcm); break;
        case 23: ret=decode_type23(rtcm); break; /* not supported */
        case 24: ret=decode_type24(rtcm); break; /* not supported */
        case 31: ret=decode_type31(rtcm); break; /* not supported */
        case 32: ret=decode_type32(rtcm); break; /* not supported */
        case 34: ret=decode_type34(rtcm); break; /* not supported */
        case 36: ret=decode_type36(rtcm); break; /* not supported */
        case 37: ret=decode_type37(rtcm); break; /* not supported */
        case 59: ret=decode_type59(rtcm); break; /* not supported */
    }
    sprintf(rtcm->msgtype,"RTCM2: type=%2d len=%3d zcnt=%7.1f staid=%3d seqno=%1d",
            type,rtcm->len,zcnt,staid,seqno);
    if (ret>=0) {
        if (1<=type&&type<=99) rtcm->nmsg2[type]++; else rtcm->nmsg2[0]++;
    }
    return ret;
}
/* decode rtcm ver.3 message -------------------------------------------------*/
static int decode_rtcm3(rtcm_t *rtcm)
{
    int ret=0,type=getbitu(rtcm->buff,24,12);
    
    trace(3,"decode_rtcm3: len=%3d type=%d\n",rtcm->len,type);
    
    switch (type) {
        case 1001: ret=decode_type1001(rtcm); break; /* not supported */
        case 1002: ret=decode_type1002(rtcm); break;
        case 1003: ret=decode_type1003(rtcm); break; /* not supported */
        case 1004: ret=decode_type1004(rtcm); break;
        case 1005: ret=decode_type1005(rtcm); break;
        case 1006: ret=decode_type1006(rtcm); break;
        case 1007: ret=decode_type1007(rtcm); break;
        case 1008: ret=decode_type1008(rtcm); break;
        case 1009: ret=decode_type1009(rtcm); break; /* not supported */
        case 1010: ret=decode_type1010(rtcm); break;
        case 1011: ret=decode_type1011(rtcm); break; /* not supported */
        case 1012: ret=decode_type1012(rtcm); break;
        case 1013: ret=decode_type1013(rtcm); break; /* not supported */
        case 1019: ret=decode_type1019(rtcm); break;
        case 1020: ret=decode_type1020(rtcm); break;
        case 1033: ret=decode_type1033(rtcm); break;
        case 1057: ret=decode_type1057(rtcm); break;
        case 1058: ret=decode_type1058(rtcm); break;
        case 1059: ret=decode_type1059(rtcm); break;
        case 1060: ret=decode_type1060(rtcm); break;
        case 1061: ret=decode_type1061(rtcm); break;
        case 1062: ret=decode_type1062(rtcm); break;
        case 1063: ret=decode_type1063(rtcm); break;
        case 1064: ret=decode_type1064(rtcm); break;
        case 1065: ret=decode_type1065(rtcm); break;
        case 1066: ret=decode_type1066(rtcm); break;
        case 1067: ret=decode_type1067(rtcm); break;
        case 1068: ret=decode_type1068(rtcm); break;
    }
    sprintf(rtcm->msgtype,"RTCM3: type=%d len=%3d",type,rtcm->len);
    if (ret>=0) {
        type-=1000;
        if (1<=type&&type<=99) rtcm->nmsg3[type]++; else rtcm->nmsg3[0]++;
    }
    return ret;
}
/* initialize rtcm control -----------------------------------------------------
* initialize rtcm control struct and reallocate memory for observation and
* ephemeris buffer in rtcm control struct
* args   : rtcm_t *raw   IO     rtcm control struct
* return : status (1:ok,0:memory allocation error)
*-----------------------------------------------------------------------------*/
extern int init_rtcm(rtcm_t *rtcm)
{
    gtime_t time0={0};
    obsd_t data0={{0}};
    eph_t  eph0 ={0,-1,-1};
    geph_t geph0={0,-1};
    ssr_t ssr0={{0}};
    int i,j;
    
    trace(3,"init_rtcm:\n");
    
    rtcm->staid=rtcm->stah=rtcm->seqno=0;
    rtcm->time=time0;
    rtcm->sta.name[0]=rtcm->sta.marker[0]='\0';
    rtcm->sta.antdes[0]=rtcm->sta.antsno[0]='\0';
    rtcm->sta.rectype[0]=rtcm->sta.recver[0]=rtcm->sta.recsno[0]='\0';
    rtcm->sta.antsetup=rtcm->sta.itrf=rtcm->sta.deltype=0;
    for (i=0;i<3;i++) {
        rtcm->sta.pos[i]=rtcm->sta.del[i]=0.0;
    }
    rtcm->sta.hgt=0.0;
    rtcm->dgps=NULL;
    for (i=0;i<MAXSAT;i++) {
        rtcm->ssr[i]=ssr0;
    }
    rtcm->msg[0]=rtcm->msgtype[0]=rtcm->opt[0]='\0';
    rtcm->obsflag=rtcm->ephsat=0;
    for (i=0;i<MAXSAT;i++) for (j=0;j<NFREQ;j++) {
        rtcm->cp[i][j]=0.0;
        rtcm->lock[i][j]=rtcm->loss[i][j]=0;
    }
    rtcm->nbyte=rtcm->nbit=rtcm->len=0;
    rtcm->word=0;
    for (i=0;i<100;i++) rtcm->nmsg2[i]=rtcm->nmsg3[i]=0;
    
    free_rtcm(rtcm);
    
    /* reallocate memory for observation and ephemris buffer */
    if (!(rtcm->obs.data=(obsd_t *)malloc(sizeof(obsd_t)*MAXOBS))||
        !(rtcm->nav.eph =(eph_t  *)malloc(sizeof(eph_t )*MAXSAT))||
        !(rtcm->nav.geph=(geph_t *)malloc(sizeof(geph_t)*MAXPRNGLO))) {
        free_rtcm(rtcm);
        return 0;
    }
    for (i=0;i<MAXOBS   ;i++) rtcm->obs.data[i]=data0;
    for (i=0;i<MAXSAT   ;i++) rtcm->nav.eph [i]=eph0;
    for (i=0;i<MAXPRNGLO;i++) rtcm->nav.geph[i]=geph0;
    return 1;
}
/* free rtcm control ----------------------------------------------------------
* free observation and ephemris buffer in rtcm control struct
* args   : rtcm_t *raw   IO     rtcm control struct
* return : none
*-----------------------------------------------------------------------------*/
extern void free_rtcm(rtcm_t *rtcm)
{
    trace(3,"free_rtcm:\n");
    
    /* free memory for observation and ephemeris buffer */
    free(rtcm->obs.data); rtcm->obs.data=NULL; rtcm->obs.n=0;
    free(rtcm->nav.eph ); rtcm->nav.eph =NULL; rtcm->nav.n=0;
    free(rtcm->nav.geph); rtcm->nav.geph=NULL; rtcm->nav.ng=0;
}
/* input rtcm message from stream ----------------------------------------------
* fetch next rtcm message and input a message from byte stream
* args   : rtcm_t *rtcm IO   rtcm control struct
*          unsigned char data I stream data (1 byte)
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 5: input station pos/ant parameters,
*                  6: input time parameter, 7: input dgps corrections,
*                  8: input special messages, 10: input ssr messages)
* notes  : before firstly calling the function, time in rtcm control struct has
*          to be set to the approximate time within 1/2 hour (for rtcm2) or
*          1/2 week (for rtcm3) in order to resolve ambiguity of time in rtcm
*          messages.
*          supported msgs RTCM ver.2: 1,3,9,14,16,17,18,19,22
*                         RTCM ver.3: 1002,1004-1008,1010,1012,1019,1020,1033
*                                     1057-1068 (draft)
*          refer [1] for RTCM ver.2, refer [2][3] for RTCM ver.3,
*          refer [4] for RTCM ver.3 message 1057-1068
*-----------------------------------------------------------------------------*/
extern int input_rtcm2(rtcm_t *rtcm, unsigned char data)
{
    unsigned char preamb;
    int i;
    
    trace(5,"input_rtcm2: data=%02x\n",data);
    
    if ((data&0xC0)!=0x40) return 0; /* ignore if upper 2bit != 01 */
    
    for (i=0;i<6;i++,data>>=1) { /* decode 6-of-8 form */
        rtcm->word=(rtcm->word<<1)+(data&1);
        
        /* synchronize frame */
        if (rtcm->nbyte==0) {
            preamb=(unsigned char)(rtcm->word>>22);
            if (rtcm->word&0x40000000) preamb^=0xFF; /* decode preamble */
            if (preamb!=RTCM2PREAMB) continue;
            
            /* check parity */
            if (!decode_word(rtcm->word,rtcm->buff)) continue;
            rtcm->nbyte=3; rtcm->nbit=0;
            continue;
        }
        if (++rtcm->nbit<30) continue; else rtcm->nbit=0;
        
        /* check parity */
        if (!decode_word(rtcm->word,rtcm->buff+rtcm->nbyte)) {
            trace(2,"rtcm2 partity error: i=%d word=%08x\n",i,rtcm->word);
            rtcm->nbyte=0; rtcm->word&=0x3;
            continue;
        }
        rtcm->nbyte+=3;
        if (rtcm->nbyte==6) rtcm->len=(rtcm->buff[5]>>3)*3+6;
        if (rtcm->nbyte<rtcm->len) continue;
        rtcm->nbyte=0; rtcm->word&=0x3;
        
        /* decode rtcm2 message */
        return decode_rtcm2(rtcm);
    }
    return 0;
}
extern int input_rtcm3(rtcm_t *rtcm, unsigned char data)
{
    trace(5,"input_rtcm3: data=%02x\n",data);
    
    /* synchronize frame */
    if (rtcm->nbyte==0) {
        if (data!=RTCM3PREAMB) return 0;
        rtcm->buff[rtcm->nbyte++]=data;
        return 0;
    }
    rtcm->buff[rtcm->nbyte++]=data;
    
    if (rtcm->nbyte==3) {
        rtcm->len=getbitu(rtcm->buff,14,10)+3; /* length without parity */
    }
    if (rtcm->nbyte<3||rtcm->nbyte<rtcm->len+3) return 0;
    rtcm->nbyte=0;
    
    /* check parity */
    if (crc24q(rtcm->buff,rtcm->len)!=getbitu(rtcm->buff,rtcm->len*8,24)) {
        trace(2,"rtcm3 parity error: len=%d\n",rtcm->len);
        return 0;
    }
    /* decode rtcm3 message */
    return decode_rtcm3(rtcm);
}
/* input rtcm message from file ------------------------------------------------
* fetch next rtcm message and input a messsage from file
* args   : rtcm_t *rtcm IO   rtcm control struct
*          FILE  *fp    I    file pointer
* return : status (-2: end of file, -1...10: same as above)
* notes  : same as above
*-----------------------------------------------------------------------------*/
extern int input_rtcm2f(rtcm_t *rtcm, FILE *fp)
{
    int i,data=0,ret;
    
    trace(4,"input_rtcm2f: data=%02x\n",data);
    
    for (i=0;i<4096;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        if ((ret=input_rtcm2(rtcm,(unsigned char)data))) return ret;
    }
    return 0; /* return at every 4k bytes */
}
extern int input_rtcm3f(rtcm_t *rtcm, FILE *fp)
{
    int i,data=0,ret;
    
    trace(4,"input_rtcm3f: data=%02x\n",data);
    
    for (i=0;i<4096;i++) {
        if ((data=fgetc(fp))==EOF) return -2;
        if ((ret=input_rtcm3(rtcm,(unsigned char)data))) return ret;
    }
    return 0; /* return at every 4k bytes */
}
