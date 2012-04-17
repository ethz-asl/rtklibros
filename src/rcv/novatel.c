/*------------------------------------------------------------------------------
* notvatel.c : NovAtel OEM4/OEM3 receiver functions
*
*          Copyright (C) 2007-2011 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] NovAtel, OM-20000094 Rev6 OEMV Family Firmware Reference Manual, 2008
*     [2] NovAtel, OM-20000053 Rev2 MiLLennium GPSCard Software Versions 4.503
*         and 4.52 Command Descriptions Manual, 2001
*
* version : $Revision: 1.2 $ $Date: 2008/07/14 00:05:05 $
* history : 2007/10/08 1.0 new
*           2008/05/09 1.1 fix bug lli flag outage
*           2008/06/16 1.2 separate common functions to rcvcmn.c
*           2009/04/01 1.3 add prn number check for raw obs data
*           2009/04/10 1.4 refactored
*                          add oem3, oem4 rangeb support
*           2009/06/06 1.5 fix bug on numerical exception with illegal snr
*                          support oem3 regd message
*           2009/12/09 1.6 support oem4 gloephemerisb message
*                          invalid if parity unknown in GLONASS range
*                          fix bug of dopper polarity inversion for oem3 regd
*           2010/04/29 1.7 add tod field in geph_t
*           2011/05/27 1.4 support RAWALM for oem4/v
*                          add almanac decoding
*                          add -EPHALL option
*                          fix problem on ARM compiler
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id: novatel.c,v 1.2 2008/07/14 00:05:05 TTAKA Exp $";

#define OEM4SYNC1   0xAA        /* oem4 message start sync code 1 */
#define OEM4SYNC2   0x44        /* oem4 message start sync code 2 */
#define OEM4SYNC3   0x12        /* oem4 message start sync code 3 */
#define OEM3SYNC1   0xAA        /* oem3 message start sync code 1 */
#define OEM3SYNC2   0x44        /* oem3 message start sync code 2 */
#define OEM3SYNC3   0x11        /* oem3 message start sync code 3 */

#define OEM4HLEN    28          /* oem4 message header length (bytes) */
#define OEM3HLEN    12          /* oem3 message header length (bytes) */

#define ID_ALMANAC  73          /* message id: oem4 decoded almanac */
#define ID_GLOALMANAC 718       /* message id: oem4 glonass decoded almanac */
#define ID_GLOEPHEMERIS 723     /* message id: oem4 glonass ephemeris */
#define ID_IONUTC   8           /* message id: oem4 iono and utc data */
#define ID_RANGE    43          /* message id: oem4 range measurement */
#define ID_RANGECMP 140         /* message id: oem4 range compressed */
#define ID_RAWALM   74          /* message id: oem4 raw almanac */
#define ID_RAWEPHEM 41          /* message id: oem4 raw ephemeris */
#define ID_RAWWAASFRAME 287     /* message id: oem4 raw waas frame */

#define ID_ALMB     18          /* message id: oem3 decoded almanac */
#define ID_IONB     16          /* message id: oem3 iono parameters */
#define ID_UTCB     17          /* message id: oem3 utc parameters */
#define ID_FRMB     54          /* message id: oem3 framed raw navigation data */
#define ID_RALB     15          /* message id: oem3 raw almanac */
#define ID_RASB     66          /* message id: oem3 raw almanac set */
#define ID_REPB     14          /* message id: oem3 raw ephemeris */
#define ID_RGEB     32          /* message id: oem3 range measurement */
#define ID_RGED     65          /* message id: oem3 range compressed */

#define WL1         0.1902936727984
#define WL2         0.2442102134246
#define MAXVAL      8388608.0

#define OFF_FRQNO   -7          /* F/W ver.3.620 */

/* get fields (little-endian) ------------------------------------------------*/
#define U1(p)       (*((unsigned char *)(p)))
#define I1(p)       (*((char *)(p)))
#define U2(p)       (*((unsigned short *)(p)))
#define I2(p)       (*((short *)(p)))
#define U4(p)       (*((unsigned int *)(p)))
#define I4(p)       (*((int *)(p)))
#define R4(p)       (*((float *)(p)))

static double R8(const unsigned char *p)
{
    double value;
    unsigned char *q=(unsigned char *)&value;
    int i;
    for (i=0;i<8;i++) *q++=*p++;
    return value;
}
/* extend sign ---------------------------------------------------------------*/
static int exsign(unsigned int v, int bits)
{
    return (int)(v&(1<<(bits-1))?v|(~0u<<bits):v);
}
/* checksum ------------------------------------------------------------------*/
static unsigned char chksum(const unsigned char *buff, int len)
{
    unsigned char sum=0;
    int i;
    for (i=0;i<len;i++) sum^=buff[i];
    return sum;
}
/* get observation data index ------------------------------------------------*/
static int obsindex(obs_t *obs, gtime_t time, int sat)
{
    int i,j;
    
    if (obs->n>=MAXOBS) return -1;
    for (i=0;i<obs->n;i++) {
        if (obs->data[i].sat==sat) return i;
    }
    obs->data[i].time=time;
    obs->data[i].sat=sat;
    for (j=0;j<NFREQ;j++) {
        obs->data[i].L[j]=obs->data[i].P[j]=0.0;
        obs->data[i].D[j]=0.0;
        obs->data[i].SNR[j]=obs->data[i].LLI[j]=0;
        obs->data[i].code[j]=CODE_NONE;
    }
    obs->n++;
    return i;
}
/* decode oem4 tracking status -----------------------------------------------*/
static int decode_trackstat(unsigned int stat, int *track, int *plock,
                           int *clock, int *parity, int *halfc)
{
    /*--------------------------------------------------------------------------
    * track : tracking state
    *             0=L1 idle                  8=L2 idle
    *             1=L1 sky search            9=L2 p-code align
    *             2=L1 wide freq pull-in    10=L2 search
    *             3=L1 narrow freq pull-in  11=L2 pll
    *             4=L1 pll                  12=L2 steering
    *             5=L1 reacq
    *             6=L1 steering
    *             7=L1 fll
    * plock : phase-lock flag   0=not locked, 1=locked
    * clock : code-lock flag    0=not locked, 1=locked
    * parity: parity known flag 0=not known,  1=known
    * halfc : phase measurement 0=half-cycle not added, 1=added
    * return: signal type
    *             GPS: 0=L1C/A, 5=L2P, 9=L2Pcodeless, 17=L2C
    *             SBS: 0=L1C/A
    *             GLO: 0=L1C/A, 4=L1P, 5=L2P
    *-------------------------------------------------------------------------*/
    *track =stat&0x1F;
    *plock =(stat>>10)&1;
    *parity=(stat>>11)&1;
    *clock =(stat>>12)&1;
    *halfc =(stat>>28)&1;
    return (stat>>21)&0x1F;
}
/* decode rangecmpb ----------------------------------------------------------*/
static int decode_rangecmpb(raw_t *raw)
{
    double psr,adr,adr_rolls,lockt,tt,dop,snr;
    int i,index,nobs,prn,sat,sys,type,freq,track,plock,clock,parity,halfc,lli;
    unsigned char *p=raw->buff+OEM4HLEN;
    
    trace(3,"decode_rangecmpb: len=%d\n",raw->len);
    
    nobs=U4(p);
    
    if (raw->len<OEM4HLEN+4+nobs*24) {
        trace(2,"oem4 rangecmpb length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    for (i=0,p+=4;i<nobs;i++,p+=24) {
        prn=U1(p+17);
        if      (  1<=prn&&prn<= 32) sys=SYS_GPS;
        else if ( 38<=prn&&prn<= 61) {sys=SYS_GLO; prn-=37;}
        else if (120<=prn&&prn<=138) sys=SYS_SBS;
        else {
            trace(2,"oem4 rangecmpb satellite prn error: prn=%d\n",prn);
            continue;
        }
        if (!(sat=satno(sys,prn))) {
            /*trace(2,"oem4 rangecmpb satellite number error: sys=%d,prn=%d\n",sys,prn);*/
            continue;
        }
        type=decode_trackstat(U4(p),&track,&plock,&clock,&parity,&halfc);
        
        if (sys==SYS_GLO&&!parity) continue; /* invalid if GLO parity unknown */
        
        if      (type==0||type==4)           freq=0; /* L1 */
        else if (type==5||type==9||type==17) freq=1; /* L2 */
        else {
            trace(2,"oem4 rangecmpb sig-type error: prn=%d type=%d\n",prn,type);
            continue;
        }
        dop  =exsign(U4(p+4)&0xFFFFFFF,28)/256.0;
        psr  =(U4(p+7)>>4)/128.0+U1(p+11)*2097152.0;
        adr  =I4(p+12)/256.0;
        adr_rolls=(psr/(type==0?WL1:WL2)+adr)/MAXVAL;
        adr  =-adr+MAXVAL*floor(adr_rolls+(adr_rolls<=0?-0.5:0.5));
        lockt=(U4(p+18)&0x1FFFFF)/32.0; /* lock time */
        
        tt=timediff(raw->time,raw->tobs);
        if (raw->tobs.time!=0) {
            lli=(lockt<65535.968&&lockt-raw->lockt[sat-1][freq]+0.05<=tt)||
                halfc!=raw->halfc[sat-1][freq];
        }
        else {
            lli=0;
        }
        if (!parity) lli|=2;
        raw->lockt[sat-1][freq]=lockt;
        raw->halfc[sat-1][freq]=halfc;
        
        snr=((U2(p+20)&0x3FF)>>5)+20.0;
        if (!clock) psr=0.0;     /* code unlock */
        if (!plock) adr=dop=0.0; /* phase unlock */
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [freq]=adr;
            raw->obs.data[index].P  [freq]=psr;
            raw->obs.data[index].D  [freq]=(float)dop;
            raw->obs.data[index].SNR[freq]=
                0.0<=snr&&snr<255.0?(unsigned char)(snr*4.0+0.5):0;
            raw->obs.data[index].LLI[freq]=(unsigned char)lli;
            raw->obs.data[index].code[freq]=
                type== 0?CODE_L1C:
               (type== 4?CODE_L1P:
               (type== 5?CODE_L2P:
               (type== 9?CODE_L2P:
               (type==17?CODE_L2C:CODE_NONE))));
            
            /* L2C phase shift correction (L2C->L2P) */
            if (type==17) {
                raw->obs.data[index].L[freq]+=0.25;
                trace(2,"oem4 L2C phase shift corrected: prn=%2d\n",prn);
            }
        }
    }
    raw->tobs=raw->time;
    return 1;
}
/* decode rangeb -------------------------------------------------------------*/
static int decode_rangeb(raw_t *raw)
{
    double psr,adr,dop,snr,lockt,tt;
    int i,index,nobs,prn,sat,sys,type,freq,track,plock,clock,parity,halfc,lli;
    unsigned char *p=raw->buff+OEM4HLEN;
    
    trace(3,"decode_rangeb: len=%d\n",raw->len);
    
    nobs=U4(p);
    
    if (raw->len<OEM4HLEN+4+nobs*44) {
        trace(2,"oem4 rangeb length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    for (i=0,p+=4;i<nobs;i++,p+=44) {
        prn=U2(p);
        if      (  1<=prn&&prn<= 32) sys=SYS_GPS;
        else if ( 38<=prn&&prn<= 61) {sys=SYS_GLO; prn-=37;}
        else if (120<=prn&&prn<=138) sys=SYS_SBS;
        else {
            trace(2,"oem4 rangeb satellite prn error: prn=%d\n",prn);
            continue;
        }
        if (!(sat=satno(sys,prn))) {
            trace(2,"oem4 rangeb satellite number error: sys=%d,prn=%d\n",sys,prn);
            continue;
        }
        type=decode_trackstat(U4(p+40),&track,&plock,&clock,&parity,&halfc);
        
        if (sys==SYS_GLO&&!parity) continue; /* invalid if GLO parity unknown */
        
        if      (type==0)                    freq=0; /* L1 */
        else if (type==5||type==9||type==17) freq=1; /* L2 */
        else {
            trace(2,"oem4 rangeb sig-type error: prn=%d type=%d\n",prn,type);
            continue;
        }
        psr  =R8(p+ 4);
        adr  =R8(p+16);
        dop  =R4(p+28);
        snr  =R4(p+32);
        lockt=R4(p+36);
        
        tt=timediff(raw->time,raw->tobs);
        if (raw->tobs.time!=0) {
            lli=lockt-raw->lockt[sat-1][freq]+0.05<=tt||
                halfc!=raw->halfc[sat-1][freq];
        }
        else {
            lli=0;
        }
        if (!parity) lli|=2;
        raw->lockt[sat-1][freq]=lockt;
        raw->halfc[sat-1][freq]=halfc;
        if (!clock) psr=0.0;     /* code unlock */
        if (!plock) adr=dop=0.0; /* phase unlock */
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [freq]=-adr;
            raw->obs.data[index].P  [freq]=psr;
            raw->obs.data[index].D  [freq]=(float)dop;
            raw->obs.data[index].SNR[freq]=
                0.0<=snr&&snr<255.0?(unsigned char)(snr*4.0+0.5):0;
            raw->obs.data[index].LLI[freq]=(unsigned char)lli;
            raw->obs.data[index].code[freq]=
                type== 0?CODE_L1C:
               (type== 4?CODE_L1P:
               (type== 5?CODE_L2P:
               (type== 9?CODE_L2P:
               (type==17?CODE_L2C:CODE_NONE))));
            
            /* L2C phase shift correction */
            if (type==17) {
                raw->obs.data[index].L[freq]+=0.25;
                trace(2,"oem4 L2C phase shift corrected: prn=%2d\n",prn);
            }
        }
    }
    raw->tobs=raw->time;
    return 1;
}
/* decode rawephemb ----------------------------------------------------------*/
static int decode_rawephemb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM4HLEN;
    eph_t eph={0};
    int prn,sat;
    
    trace(3,"decode_rawephemb: len=%d\n",raw->len);
    
    if (raw->len<OEM4HLEN+102) {
        trace(2,"oem4 rawephemb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p);
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"oem4 rawephemb satellite number error: prn=%d\n",prn);
        return -1;
    }
    if (decode_frame(p+ 12,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(p+ 42,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(p+ 72,&eph,NULL,NULL,NULL,NULL)!=3) {
        trace(2,"oem4 rawephemb subframe error: prn=%d\n",prn);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    trace(4,"decode_rawephemb: sat=%2d\n",sat);
    return 2;
}
/* decode ionutcb ------------------------------------------------------------*/
static int decode_ionutcb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM4HLEN;
    int i;
    
    trace(3,"decode_ionutcb: len=%d\n",raw->len);
    
    if (raw->len<OEM4HLEN+108) {
        trace(2,"oem4 ionutcb length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=0;i<8;i++) raw->nav.ion_gps[i]=R8(p+i*8);
    raw->nav.utc_gps[0]=R8(p+72);
    raw->nav.utc_gps[1]=R8(p+80);
    raw->nav.utc_gps[2]=U4(p+68);
    raw->nav.utc_gps[3]=U4(p+64);
    raw->nav.leaps =I4(p+96);
    return 9;
}
/* decode rawwaasframeb ------------------------------------------------------*/
static int decode_rawwaasframeb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM4HLEN;
    int i,prn;
    
    trace(3,"decode_rawwaasframeb: len=%d\n",raw->len);
    
    if (raw->len<OEM4HLEN+48) {
        trace(2,"oem4 rawwaasframeb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p+4);
    if (prn<MINPRNSBS||MAXPRNSBS<prn) return 0;
    raw->sbsmsg.tow=(int)time2gpst(raw->time,&raw->sbsmsg.week);
    raw->sbsmsg.prn=prn;
    for (i=0,p+=12;i<29;i++,p++) raw->sbsmsg.msg[i]=*p;
    return 3;
}
/* decode gloephemerisb ------------------------------------------------------*/
static int decode_gloephemerisb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM4HLEN;
    geph_t geph={0};
    double tow,tof,toff;
    int prn,sat,week;
    
    trace(3,"decode_gloephemerisb: len=%d\n",raw->len);
    
    if (raw->len<OEM4HLEN+144) {
        trace(2,"oem4 gloephemerisb length error: len=%d\n",raw->len);
        return -1;
    }
    prn        =U2(p)-37;
    if (!(sat=satno(SYS_GLO,prn))) {
        trace(2,"oem4 gloephemerisb prn error: prn=%d\n",prn);
        return -1;
    }
    geph.frq   =U2(p+  2)+OFF_FRQNO;
    week       =U2(p+  6);
    tow        =floor(U4(p+8)/1000.0+0.5); /* rounded to integer sec */
    toff       =U4(p+ 12);
    geph.iode  =U4(p+ 20)&0x7F;
    geph.svh   =U4(p+ 24);
    geph.pos[0]=R8(p+ 28);
    geph.pos[1]=R8(p+ 36);
    geph.pos[2]=R8(p+ 44);
    geph.vel[0]=R8(p+ 52);
    geph.vel[1]=R8(p+ 60);
    geph.vel[2]=R8(p+ 68);
    geph.acc[0]=R8(p+ 76);
    geph.acc[1]=R8(p+ 84);
    geph.acc[2]=R8(p+ 92);
    geph.taun  =R8(p+100);
    geph.gamn  =R8(p+116);
    tof        =U4(p+124)-toff; /* glonasst->gpst */
    geph.age   =U4(p+136);
    geph.toe=gpst2time(week,tow);
    tof+=floor(tow/86400.0)*86400;
    if      (tof<tow-43200.0) tof+=86400.0;
    else if (tof>tow+43200.0) tof-=86400.0;
    geph.tof=gpst2time(week,tof);
    
    if (!strstr(raw->opt,"-EPHALL")) {
        if (fabs(timediff(geph.toe,raw->nav.geph[prn-1].toe))<1.0&&
            geph.svh==raw->nav.geph[prn-1].svh) return 0; /* unchanged */
    }
    geph.sat=sat;
    raw->nav.geph[prn-1]=geph;
    raw->ephsat=sat;
    return 2;
}
/* decode rgeb ---------------------------------------------------------------*/
static int decode_rgeb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM3HLEN;
    double tow,psr,adr,tt,lockt,dop,snr;
    int i,week,nobs,prn,sat,stat,sys,parity,lli,index,freq;
    
    trace(3,"decode_rgeb: len=%d\n",raw->len);
    
    week=adjgpsweek(U4(p));
    tow =R8(p+ 4);
    nobs=U4(p+12);
    raw->time=gpst2time(week,tow);
    
    if (raw->len!=OEM3HLEN+20+nobs*44) {
        trace(2,"oem3 regb length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    for (i=0,p+=20;i<nobs;i++,p+=44) {
        prn   =U4(p   );
        psr   =R8(p+ 4);
        adr   =R8(p+16);
        dop   =R4(p+28);
        snr   =R4(p+32);
        lockt =R4(p+36);     /* lock time (s) */
        stat  =I4(p+40);     /* tracking status */
        freq  =(stat>>20)&1; /* L1:0,L2:1 */
        sys   =(stat>>15)&7; /* satellite sys (0:GPS,1:GLONASS,2:WAAS) */
        parity=(stat>>10)&1; /* parity known */
        if (!(sat=satno(sys==1?SYS_GLO:(sys==2?SYS_SBS:SYS_GPS),prn))) {
            trace(2,"oem3 regb satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        tt=timediff(raw->time,raw->tobs);
        if (raw->tobs.time!=0) {
            lli=lockt-raw->lockt[sat-1][freq]+0.05<tt||
                parity!=raw->halfc[sat-1][freq];
        }
        else {
            lli=0;
        }
        if (!parity) lli|=2;
        raw->lockt[sat-1][freq]=lockt;
        raw->halfc[sat-1][freq]=parity;
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }        
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [freq]=-adr; /* flip sign */
            raw->obs.data[index].P  [freq]=psr;
            raw->obs.data[index].D  [freq]=(float)dop;
            raw->obs.data[index].SNR[freq]=
                0.0<=snr&&snr<255.0?(unsigned char)(snr*4.0+0.5):0;
            raw->obs.data[index].LLI[freq]=(unsigned char)lli;
            raw->obs.data[index].code[freq]=freq==0?CODE_L1C:CODE_L2P;
        }
    }
    raw->tobs=raw->time;
    return 1;
}
/* decode rged ---------------------------------------------------------------*/
static int decode_rged(raw_t *raw)
{
    unsigned int word;
    unsigned char *p=raw->buff+OEM3HLEN;
    double tow,psrh,psrl,psr,adr,adr_rolls,tt,lockt,dop;
    int i,week,nobs,prn,sat,stat,sys,parity,lli,index,freq,snr;
    
    trace(3,"decode_rged: len=%d\n",raw->len);
    
    nobs=U2(p);
    week=adjgpsweek(U2(p+2));
    tow =U4(p+4)/100.0;
    raw->time=gpst2time(week,tow);
    if (raw->len!=OEM3HLEN+12+nobs*20) {
        trace(2,"oem3 regd length error: len=%d nobs=%d\n",raw->len,nobs);
        return -1;
    }
    for (i=0,p+=12;i<nobs;i++,p+=20) {
        word  =U4(p);
        prn   =word&0x3F;
        snr   =((word>>6)&0x1F)+20;
        lockt =(word>>11)/32.0;
        adr   =-I4(p+4)/256.0;
        word  =U4(p+8);
        psrh  =word&0xF;
        dop   =exsign(word>>4,28)/256.0;
        psrl  =U4(p+12);
        stat  =U4(p+16)>>8;
        freq  =(stat>>20)&1; /* L1:0,L2:1 */
        sys   =(stat>>15)&7; /* satellite sys (0:GPS,1:GLONASS,2:WAAS) */
        parity=(stat>>10)&1; /* parity known */
        if (!(sat=satno(sys==1?SYS_GLO:(sys==2?SYS_SBS:SYS_GPS),prn))) {
            trace(2,"oem3 regd satellite number error: sys=%d prn=%d\n",sys,prn);
            continue;
        }
        tt=timediff(raw->time,raw->tobs);
        psr=(psrh*4294967296.0+psrl)/128.0;
        adr_rolls=floor((psr/(freq==0?WL1:WL2)-adr)/MAXVAL+0.5);
        adr=adr+MAXVAL*adr_rolls;
        
        if (raw->tobs.time!=0) {
            lli=lockt-raw->lockt[sat-1][freq]+0.05<tt||
                parity!=raw->halfc[sat-1][freq];
        }
        else {
            lli=0;
        }
        if (!parity) lli|=2;
        raw->lockt[sat-1][freq]=lockt;
        raw->halfc[sat-1][freq]=parity;
        
        if (fabs(timediff(raw->obs.data[0].time,raw->time))>1E-9) {
            raw->obs.n=0;
        }        
        if ((index=obsindex(&raw->obs,raw->time,sat))>=0) {
            raw->obs.data[index].L  [freq]=adr;
            raw->obs.data[index].P  [freq]=psr;
            raw->obs.data[index].D  [freq]=(float)dop;
            raw->obs.data[index].SNR[freq]=(unsigned char)(snr*4.0+0.5);
            raw->obs.data[index].LLI[freq]=(unsigned char)lli;
            raw->obs.data[index].code[freq]=freq==0?CODE_L1C:CODE_L2P;
        }
    }
    raw->tobs=raw->time;
    return 1;
}
/* decode repb ---------------------------------------------------------------*/
static int decode_repb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM3HLEN;
    eph_t eph={0};
    int prn,sat;
    
    trace(3,"decode_repb: len=%d\n",raw->len);
    
    if (raw->len!=OEM3HLEN+96) {
        trace(2,"oem3 repb length error: len=%d\n",raw->len);
        return -1;
    }
    prn=U4(p);
    if (!(sat=satno(SYS_GPS,prn))) {
        trace(2,"oem3 repb satellite number error: prn=%d\n",prn);
        return -1;
    }
    if (decode_frame(p+ 4,&eph,NULL,NULL,NULL,NULL)!=1||
        decode_frame(p+34,&eph,NULL,NULL,NULL,NULL)!=2||
        decode_frame(p+64,&eph,NULL,NULL,NULL,NULL)!=3) {
        trace(2,"oem3 repb subframe error: prn=%d\n",prn);
        return -1;
    }
    if (!strstr(raw->opt,"-EPHALL")) {
        if (eph.iode==raw->nav.eph[sat-1].iode) return 0; /* unchanged */
    }
    eph.sat=sat;
    raw->nav.eph[sat-1]=eph;
    raw->ephsat=sat;
    return 2;
}
/* decode frmb --------------------------------------------------------------*/
static int decode_frmb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM3HLEN;
    double tow;
    int i,week,prn,nbit;
    
    trace(3,"decode_frmb: len=%d\n",raw->len);
    
    week=adjgpsweek(U4(p));
    tow =R8(p+ 4);
    prn =U4(p+12);
    nbit=U4(p+20);
    raw->time=gpst2time(week,tow);
    if (nbit!=250) return 0;
    if (prn<MINPRNSBS||MAXPRNSBS<prn) {
        trace(2,"oem3 frmb satellite number error: prn=%d\n",prn);
        return -1;
    }
    raw->sbsmsg.week=week;
    raw->sbsmsg.tow=(int)tow;
    raw->sbsmsg.prn=prn;
    for (i=0;i<29;i++) raw->sbsmsg.msg[i]=p[24+i];
    return 3;
}
/* decode ionb ---------------------------------------------------------------*/
static int decode_ionb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM3HLEN;
    int i;
    
    if (raw->len!=64+OEM3HLEN) {
        trace(2,"oem3 ionb length error: len=%d\n",raw->len);
        return -1;
    }
    for (i=0;i<8;i++) raw->nav.ion_gps[i]=R8(p+i*8);
    return 9;
}
/* decode utcb ---------------------------------------------------------------*/
static int decode_utcb(raw_t *raw)
{
    unsigned char *p=raw->buff+OEM3HLEN;
    
    trace(3,"decode_utcb: len=%d\n",raw->len);
    
    if (raw->len!=40+OEM3HLEN) {
        trace(2,"oem3 utcb length error: len=%d\n",raw->len);
        return -1;
    }
    raw->nav.utc_gps[0]=R8(p   );
    raw->nav.utc_gps[1]=R8(p+ 8);
    raw->nav.utc_gps[2]=U4(p+16);
    raw->nav.utc_gps[3]=adjgpsweek(U4(p+20));
    raw->nav.leaps =I4(p+28);
    return 9;
}
/* decode oem4 message -------------------------------------------------------*/
static int decode_oem4(raw_t *raw)
{
    double tow;
    int msg,week,type=U2(raw->buff+4);
    
    trace(3,"decode_oem4: type=%3d len=%d\n",type,raw->len);
    
    /* check crc32 */
    if (crc32(raw->buff,raw->len)!=U4(raw->buff+raw->len)) {
        trace(2,"oem4 crc error: type=%3d len=%d\n",type,raw->len);
        return -1;
    }
    msg =(U1(raw->buff+6)>>4)&0x3;
    week=adjgpsweek(U2(raw->buff+14));
    tow =U4(raw->buff+16)*0.001;
    raw->time=gpst2time(week,tow);
    sprintf(raw->msgtype,"OEM4/V: type=%2d len=%3d msg=%d week=%4d tow=%.2f",
            type,raw->len,msg,week,tow);
    
    if (msg!=0) return 0; /* message type: 0=binary,1=ascii */
    
    switch (type) {
        case ID_RANGECMP    : return decode_rangecmpb    (raw);
        case ID_RANGE       : return decode_rangeb       (raw);
        case ID_RAWEPHEM    : return decode_rawephemb    (raw);
        case ID_RAWWAASFRAME: return decode_rawwaasframeb(raw);
        case ID_IONUTC      : return decode_ionutcb      (raw);
        case ID_GLOEPHEMERIS: return decode_gloephemerisb(raw);
    }
    return 0;
}
/* decode oem3 message -------------------------------------------------------*/
static int decode_oem3(raw_t *raw)
{
    int type=U4(raw->buff+4);
    
    trace(3,"decode_oem3: type=%3d len=%d\n",type,raw->len);
    
    /* checksum */
    if (chksum(raw->buff,raw->len)) {
        trace(2,"oem3 checksum error: type=%3d len=%d\n",type,raw->len);
        return -1;
    }
    sprintf(raw->msgtype,"OEM3: type=%2d len=%3d",type,raw->len);
    
    switch (type) {
        case ID_RGEB: return decode_rgeb(raw);
        case ID_RGED: return decode_rged(raw);
        case ID_REPB: return decode_repb(raw);
        case ID_FRMB: return decode_frmb(raw);
        case ID_IONB: return decode_ionb(raw);
        case ID_UTCB: return decode_utcb(raw);
    }
    return 0;
}
/* sync header ---------------------------------------------------------------*/
static int sync_oem4(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=data;
    return buff[0]==OEM4SYNC1&&buff[1]==OEM4SYNC2&&buff[2]==OEM4SYNC3;
}
static int sync_oem3(unsigned char *buff, unsigned char data)
{
    buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=data;
    return buff[0]==OEM3SYNC1&&buff[1]==OEM3SYNC2&&buff[2]==OEM3SYNC3;
}
/* input oem4/oem3 raw data from stream ----------------------------------------
* fetch next novatel oem4/oem3 raw data and input a mesasge from stream
* args   : raw_t *raw   IO     receiver raw data control struct
*          unsigned char data I stream data (1 byte)
*            raw->rcvopt : novatel raw options
*                "-EPHALL"  : output all ephemerides
* return : status (-1: error message, 0: no message, 1: input observation data,
*                  2: input ephemeris, 3: input sbas message,
*                  9: input ion/utc parameter)
*-----------------------------------------------------------------------------*/
extern int input_oem4(raw_t *raw, unsigned char data)
{
    trace(5,"input_oem4: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (sync_oem4(raw->buff,data)) raw->nbyte=3;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==10&&(raw->len=U2(raw->buff+8)+OEM4HLEN)>MAXRAWLEN-4) {
        trace(2,"oem4 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (raw->nbyte<10||raw->nbyte<raw->len+4) return 0;
    raw->nbyte=0;
    
    /* decode oem4 message */
    return decode_oem4(raw);
}
extern int input_oem3(raw_t *raw, unsigned char data)
{
    trace(5,"input_oem3: data=%02x\n",data);
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        if (sync_oem3(raw->buff,data)) raw->nbyte=3;
        return 0;
    }
    raw->buff[raw->nbyte++]=data;
    
    if (raw->nbyte==12&&(raw->len=U4(raw->buff+8))>MAXRAWLEN) {
        trace(2,"oem3 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (raw->nbyte<12||raw->nbyte<raw->len) return 0;
    raw->nbyte=0;
    
    /* decode oem3 message */
    return decode_oem3(raw);
}
/* input oem4/oem3 raw data from file ------------------------------------------
* fetch next novatel oem4/oem3 raw data and input a message from file
* args   : raw_t  *raw   IO     receiver raw data control struct
*          int    format I      receiver raw data format (STRFMT_???)
*          FILE   *fp    I      file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
extern int input_oem4f(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_oem4f:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_oem4(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+3,7,1,fp)<1) return -2;
    raw->nbyte=10;
    
    if ((raw->len=U2(raw->buff+8)+OEM4HLEN)>MAXRAWLEN-4) {
        trace(2,"oem4 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+10,raw->len-6,1,fp)<1) return -2;
    raw->nbyte=0;
    
    /* decode oem4 message */
    return decode_oem4(raw);
}
extern int input_oem3f(raw_t *raw, FILE *fp)
{
    int i,data;
    
    trace(4,"input_oem3f:\n");
    
    /* synchronize frame */
    if (raw->nbyte==0) {
        for (i=0;;i++) {
            if ((data=fgetc(fp))==EOF) return -2;
            if (sync_oem3(raw->buff,(unsigned char)data)) break;
            if (i>=4096) return 0;
        }
    }
    if (fread(raw->buff+3,1,9,fp)<9) return -2;
    raw->nbyte=12;
    
    if ((raw->len=U4(raw->buff+8))>MAXRAWLEN) {
        trace(2,"oem3 length error: len=%d\n",raw->len);
        raw->nbyte=0;
        return -1;
    }
    if (fread(raw->buff+12,1,raw->len-12,fp)<(size_t)(raw->len-12)) return -2;
    raw->nbyte=0;
    
    /* decode oem3 message */
    return decode_oem3(raw);
}
