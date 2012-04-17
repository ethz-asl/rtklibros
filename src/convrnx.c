/*------------------------------------------------------------------------------
* convrnx.c : rinex translator for rtcm and receiver raw data log
*
*          Copyright (C) 2009-2011 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:48:06 $
* history : 2009/04/10 1.0  new
*           2009/06/02 1.1  support glonass
*           2009/12/18 1.2  add check return of init_rtcm()/init_raw()
*           2010/07/15 1.3  support wildcard expansion of input file
*                           support rinex 3.00
*                           support rinex as input format
*                           support output of geo navigation message
*                           support rtcm antenna and receiver info
*                           changed api:
*                               convrnx()
*           2011/05/27 1.4  support GW10, Javad, LEX receiver
*                           support lex message conversion
*                           change api convrnx()
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define NOUTFILE        6       /* number of output files */
#define TSTARTMARGIN    60.0    /* time margin for file name replacement */

/* type definition -----------------------------------------------------------*/

typedef struct {                /* rinex data set */
    gtime_t time;               /* current time */
    int    ephsat;              /* ephemeris satellite number */
    int    iobs,ieph,igeph,iseph; /* pointer of data */
    obs_t  obs;                 /* observation data */
    nav_t  nav;                 /* navidation data */
    sta_t  sta;                 /* station info */
    obs_t  obs_buf;             /* observation data buffer */
    nav_t  nav_buf;             /* navigation data buffer */
    char file[1024];            /* source file */
} rnxset_t;

typedef struct {                /* steram file type */
    int    format;              /* stream format (STRFMT_???) */
    int    sat;                 /* input satellite */
    obs_t  *obs;                /* input observation data */
    nav_t  *nav;                /* input navigation data */
    gtime_t time;               /* current time */
    rtcm_t rtcm;                /* receiver raw data st*/
    raw_t  raw;                 /* rtcm data */
    rnxset_t rnxset;            /* rinex data set */
    FILE   *fp;                 /* file pointer */
} strfile_t;

/* initialize rinex data set -------------------------------------------------*/
static int init_rnxset(rnxset_t *set)
{
    gtime_t time0={0};
    eph_t  eph0={0};
    geph_t geph0={0};
    seph_t seph0={0};
    int i;
    
    trace(3,"init_rnxset:\n");
    
    set->time=time0;
    set->ephsat=0;
    set->iobs=set->ieph=set->igeph=set->iseph=0;
    
    if (!(set->nav.eph =(eph_t  *)malloc(sizeof(eph_t )*MAXSAT ))||
        !(set->nav.geph=(geph_t *)malloc(sizeof(geph_t)*NSATGLO))||
        !(set->nav.seph=(seph_t *)malloc(sizeof(seph_t)*NSATSBS*2))) {
        return 0;
    }
    for (i=0;i<MAXSAT   ;i++) set->nav.eph [i]=eph0;
    for (i=0;i<NSATGLO  ;i++) set->nav.geph[i]=geph0;
    for (i=0;i<NSATSBS*2;i++) set->nav.seph[i]=seph0;
    set->obs.n=0;
    set->obs.data=set->obs_buf.data;
    set->nav.n =MAXSAT;
    set->nav.ng=NSATGLO;
    set->nav.ns=NSATSBS*2;
    *set->file='\0';
    return 1;
}
/* free rinex data set -------------------------------------------------------*/
static void free_rnxset(rnxset_t *set)
{
    trace(3,"free_rnxset:\n");
    
    free(set->nav.eph ); set->nav.eph =NULL; set->nav.n =0;
    free(set->nav.geph); set->nav.geph=NULL; set->nav.ng=0;
    free(set->nav.seph); set->nav.seph=NULL; set->nav.ns=0;
}
/* open rinex data set -------------------------------------------------------*/
static int open_rnxset(rnxset_t *set, const char *file)
{
    int i;
    
    trace(3,"open_rnxset: file=%s\n",file);
    
    if (readrnx(file,1,&set->obs_buf,&set->nav_buf,&set->sta)<0) return 0;
    
    if (set->obs_buf.n <=0&&set->nav_buf.n <=0&&
        set->nav_buf.ng<=0&&set->nav_buf.ns<=0) return 0;
    
    sortobs(&set->obs_buf);
    uniqnav(&set->nav_buf);
    
    set->ephsat=0;
    set->iobs=set->ieph=set->igeph=set->iseph=0;
    strcpy(set->file,file);
    for (i=0;i<8;i++) set->nav.ion_gps[i]=set->nav_buf.ion_gps[i];
    for (i=0;i<4;i++) set->nav.utc_gps[i]=set->nav_buf.utc_gps[i];
    for (i=0;i<4;i++) set->nav.ion_gal[i]=set->nav_buf.ion_gal[i];
    for (i=0;i<8;i++) set->nav.ion_qzs[i]=set->nav_buf.ion_qzs[i];
    for (i=0;i<4;i++) set->nav.utc_qzs[i]=set->nav_buf.utc_qzs[i];
    set->nav.leaps=set->nav_buf.leaps;
    return 1;
}
/* close rinex data set ------------------------------------------------------*/
static void close_rnxset(rnxset_t *set)
{
    trace(3,"close_rnxset:\n");
    
    free(set->obs_buf.data); set->obs_buf.data=NULL; set->obs_buf.n=set->obs_buf.nmax=0;
    free(set->nav_buf.eph ); set->nav_buf.eph =NULL; set->nav_buf.n =0;
    free(set->nav_buf.geph); set->nav_buf.geph=NULL; set->nav_buf.ng=0;
    free(set->nav_buf.seph); set->nav_buf.seph=NULL; set->nav_buf.ns=0;
    strcpy(set->file,"");
}
/* input rinex data set ------------------------------------------------------*/
static int input_rnxset(rnxset_t *set)
{
    obsd_t *p=NULL;
    eph_t  *q=NULL;
    geph_t *r=NULL;
    seph_t *s=NULL;
    int i,prn;
    
    trace(4,"input_rnxset: iobs=%d ieph=%d igeph=%d iseph=%d\n",set->iobs,
          set->ieph,set->igeph,set->iseph);
    
    if (set->iobs <set->obs_buf.n ) p=set->obs_buf.data+set->iobs;
    if (set->ieph <set->nav_buf.n ) q=set->nav_buf.eph +set->ieph;
    if (set->igeph<set->nav_buf.ng) r=set->nav_buf.geph+set->igeph;
    if (set->iseph<set->nav_buf.ns) s=set->nav_buf.seph+set->iseph;
    
    if (p&&(!q||timediff(p->time,q->ttr)<0.0)&&
           (!r||timediff(p->time,r->tof)<0.0)&&
           (!s||timediff(p->time,s->tof)<0.0)) {
        
        for (i=set->iobs+1;i<set->obs_buf.n;i++) {
            if (timediff(set->obs_buf.data[i].time,p->time)>DTTOL) break;
        }
        set->time=p->time;
        set->obs.data=set->obs_buf.data+set->iobs;
        set->obs.n=i-set->iobs<MAXOBS?i-set->iobs:MAXOBS;
        set->iobs=i;
        return 1;
    }
    else if (q&&(!r||timediff(q->ttr,r->tof)<=0.0)&&
                (!s||timediff(q->ttr,s->tof)<=0.0)) {
        set->time=q->ttr;
        set->ephsat=q->sat;
        set->nav.eph[q->sat-MINPRNGPS]=*q;
        set->ieph++;
        return 2;
    }
    else if (r&&(!q||timediff(r->tof,q->ttr)<=0.0)&&
                (!s||timediff(r->tof,s->tof)<=0.0)) {
        set->time=r->tof;
        set->ephsat=r->sat;
        satsys(r->sat,&prn);
        set->nav.geph[prn-MINPRNGLO]=*r;
        set->igeph++;
        return 2;
    }
    else if (s&&(!q||timediff(s->tof,q->ttr)<=0.0)&&
                (!r||timediff(s->tof,r->tof)<=0.0)) {
        set->time=s->tof;
        set->ephsat=s->sat;
        satsys(s->sat,&prn);
        set->nav.seph[prn-MINPRNSBS]=*s;
        set->iseph++;
        return 2;
    }
    return -2; /* end of data */
}
/* set rinex antenna and receiver info to options ----------------------------*/
static void rnxset2opt(const rnxset_t *set, rnxopt_t *opt)
{
    double pos[3],enu[3];
    int i;
    
    trace(3,"rnxset2opt:\n");
    
    /* receiver and antenna info */
    if (!*opt->marker&&!*opt->markerno) {
        strcpy(opt->marker,set->sta.name);
        strcpy(opt->markerno,set->sta.marker);
    }
    if (!*opt->rec[0]&&!*opt->rec[1]&&!*opt->rec[2]) {
        strcpy(opt->rec[0],set->sta.recsno);
        strcpy(opt->rec[1],set->sta.rectype);
        strcpy(opt->rec[2],set->sta.recver);
    }
    if (!*opt->ant[0]&&!*opt->ant[1]) {
        strcpy(opt->ant[0],set->sta.antsno);
        strcpy(opt->ant[1],set->sta.antdes);
    }
    /* antenna approx position */
    if (norm(set->sta.pos,3)>0.0) {
        for (i=0;i<3;i++) opt->apppos[i]=set->sta.pos[i];
    }
    /* antenna delta */
    if (norm(set->sta.del,3)>0.0) {
        if (!set->sta.deltype) { /* enu */
            opt->antdel[0]=set->sta.del[2]; /* h */
            opt->antdel[1]=set->sta.del[0]; /* e */
            opt->antdel[2]=set->sta.del[1]; /* n */
        }
        else if (norm(opt->apppos,3)>0.0) { /* xyz */
            ecef2pos(opt->apppos,pos);
            ecef2enu(pos,set->sta.del,enu);
            opt->antdel[0]=enu[2];
            opt->antdel[1]=enu[0];
            opt->antdel[2]=enu[1];
        }
    }
}
/* set rtcm antenna and receiver info to options -----------------------------*/
static void rtcm2opt(const rtcm_t *rtcm, rnxopt_t *opt)
{
    double pos[3],enu[3];
    int i;
    
    trace(3,"rtcm2opt:\n");
    
    /* comment */
    sprintf(opt->comment[1]+strlen(opt->comment[1]),", station ID: %d",
            rtcm->staid);
    
    /* receiver and antenna info */
    if (!*opt->rec[0]&&!*opt->rec[1]&&!*opt->rec[2]) {
        strcpy(opt->rec[0],rtcm->sta.recsno);
        strcpy(opt->rec[1],rtcm->sta.rectype);
        strcpy(opt->rec[2],rtcm->sta.recver);
    }
    if (!*opt->ant[0]&&!*opt->ant[1]&&!*opt->ant[2]) {
        strcpy(opt->ant[0],rtcm->sta.antsno);
        strcpy(opt->ant[1],rtcm->sta.antdes);
        if (rtcm->sta.antsetup) {
            sprintf(opt->ant[2],"%d",rtcm->sta.antsetup);
        }
        else *opt->ant[2]='\0';
    }
    /* antenna approx position */
    if (norm(rtcm->sta.pos,3)>0.0) {
        for (i=0;i<3;i++) opt->apppos[i]=rtcm->sta.pos[i];
    }
    /* antenna delta */
    if (norm(rtcm->sta.del,3)>0.0) {
        if (!rtcm->sta.deltype&&norm(rtcm->sta.del,3)>0.0) { /* enu */
            opt->antdel[0]=rtcm->sta.del[2]; /* h */
            opt->antdel[1]=rtcm->sta.del[0]; /* e */
            opt->antdel[2]=rtcm->sta.del[1]; /* n */
        }
        else if (norm(rtcm->sta.pos,3)>0.0) { /* xyz */
            ecef2pos(rtcm->sta.pos,pos);
            ecef2enu(pos,rtcm->sta.del,enu);
            opt->antdel[0]=enu[2]; /* h */
            opt->antdel[1]=enu[0]; /* e */
            opt->antdel[2]=enu[1]; /* n */
        }
    }
    else {
        opt->antdel[0]=rtcm->sta.hgt;
        opt->antdel[0]=0.0;
        opt->antdel[0]=0.0;
    }
}
/* initialize stream file ----------------------------------------------------*/
static int init_strfile(strfile_t *str, int format, const char *opt,
                        gtime_t trtcm)
{
    trace(3,"init_strfile:\n");
    
    if (format==STRFMT_RTCM2||format==STRFMT_RTCM3) {
        if (!init_rtcm(&str->rtcm)) {
            showmsg("init rtcm error");
            return 0;
        }
        str->rtcm.time=trtcm;
        str->obs=&str->rtcm.obs;
        str->nav=&str->rtcm.nav; 
        strcpy(str->rtcm.opt,opt);
    }
    else if (format<=MAXRCVFMT) {
        if (!init_raw(&str->raw)) {
            showmsg("init raw error");
            return 0;
        }
        str->obs=&str->raw.obs;
        str->nav=&str->raw.nav;
        strcpy(str->raw.opt,opt);
    }
    else if (format==STRFMT_RINEX) {
        if (!init_rnxset(&str->rnxset)) {
            showmsg("init rnxset error");
            return 0;
        }
        str->obs=&str->rnxset.obs;
        str->nav=&str->rnxset.nav;
    }
    str->format=format;
    str->sat=0;
    str->fp=NULL;
    return 1;
}
/* free stream file ----------------------------------------------------------*/
static void free_strfile(strfile_t *str)
{
    trace(3,"free_strfile:\n");
    
    if (str->format==STRFMT_RTCM2||str->format==STRFMT_RTCM3) {
        free_rtcm(&str->rtcm);
    }
    else if (str->format<=MAXRCVFMT) {
        free_raw(&str->raw);
    }
    else if (str->format==STRFMT_RINEX) {
        free_rnxset(&str->rnxset);
    }
}
/* input stream file ---------------------------------------------------------*/
static int input_strfile(strfile_t *str)
{
    int type=0;
    
    trace(4,"input_strfile:\n");
    
    if (str->format==STRFMT_RTCM2) {
        if ((type=input_rtcm2f(&str->rtcm,str->fp))>=1) {
            str->time=str->rtcm.time;
            str->sat=str->rtcm.ephsat;
        }
    }
    else if (str->format==STRFMT_RTCM3) {
        if ((type=input_rtcm3f(&str->rtcm,str->fp))>=1) {
            str->time=str->rtcm.time;
            str->sat=str->rtcm.ephsat;
        }
    }
    else if (str->format<=MAXRCVFMT) {
        if ((type=input_rawf(&str->raw,str->format,str->fp))>=1) {
            str->time=str->raw.time;
            str->sat=str->raw.ephsat;
        }
    }
    else if (str->format==STRFMT_RINEX) {
        if ((type=input_rnxset(&str->rnxset))>=1) {
            str->time=str->rnxset.time;
            str->sat=str->rnxset.ephsat;
        }
    }
    trace(4,"input_strfile: time=%s type=%d sat=%2d\n",time_str(str->time,3),
          type,str->sat);
    return type;
}
/* open stream file ----------------------------------------------------------*/
static int open_strfile(strfile_t *str, const char *file)
{
    trace(3,"open_strfile: file=%s\n",file);
    
    if (str->format==STRFMT_RTCM2||str->format==STRFMT_RTCM3) {
        if (!(str->fp=fopen(file,"rb"))) {
            showmsg("rtcm open error: %s",file);
            return 0;
        }
    }
    else if (str->format<=MAXRCVFMT) {
        if (!(str->fp=fopen(file,"rb"))) {
            showmsg("log open error: %s",file);
            return 0;
        }
        /* read head to resolve time ambiguity */
        if (str->time.time==0) {
            str->raw.flag=1;
            while (input_strfile(str)>=-1&&str->time.time==0) ;
            str->raw.flag=1;
            rewind(str->fp);
        }
    }
    else if (str->format==STRFMT_RINEX) {
        if (!open_rnxset(&str->rnxset,file)) {
            showmsg("rinex open error: %s",file);
            return 0;
        }
    }
    return 1;
}
/* close stream file ---------------------------------------------------------*/
static void close_strfile(strfile_t *str)
{
    trace(3,"close_strfile:\n");
    
    if (str->format==STRFMT_RTCM2||str->format==STRFMT_RTCM3) {
        if (str->fp) fclose(str->fp);
    }
    else if (str->format<=MAXRCVFMT) {
        if (str->fp) fclose(str->fp);
    }
    else if (str->format==STRFMT_RINEX) {
        close_rnxset(&str->rnxset);
    }
}
/* get start time of stream file ---------------------------------------------*/
static void gettime_strfile(strfile_t *str, const char *file, gtime_t *time)
{
    int type;
    
    trace(3,"gettime_strfile: file=%s\n",file);
    
    /* read first obs time */
    if (!open_strfile(str,file)) return;
    
    while ((type=input_strfile(str))>=-1) {
        if (type!=1||str->obs->n<=0) continue;
        *time=str->obs->data[0].time;
        break;
    }
    close_strfile(str);
}
/* save slip conditions ------------------------------------------------------*/
static void saveslips(unsigned char slips[][NFREQ], obsd_t *data, int n)
{
    int i,j;
    
    for (i=0;i<n;i++) for (j=0;j<NFREQ;j++) {
        if (data[i].LLI[j]&1) slips[data[i].sat-1][j]|=1;
    }
}
/* restore slip conditions ---------------------------------------------------*/
static void restslips(unsigned char slips[][NFREQ], obsd_t *data, int n)
{
    int i,j;
    
    for (i=0;i<n;i++) for (j=0;j<NFREQ;j++) {
        if (slips[data[i].sat-1][j]&1) data[i].LLI[j]|=1;
        slips[data[i].sat-1][j]=0;
    }
}
/* open output files ---------------------------------------------------------*/
static int openfile(FILE **ofp, char *files[], const char *file,
                    const rnxopt_t *opt, nav_t *nav)
{
    char path[1024];
    int i;
    
    trace(3,"openfile:\n");
    
    for (i=0;i<NOUTFILE;i++) {
        
        if (!*files[i]) continue;
        
        strcpy(path,files[i]);
        
        /* check overwrite input file and modify output file */
        if (!strcmp(path,file)) strcat(path,"_");
        
        /* create directory if not exist */
        createdir(path);
        
        if (!(ofp[i]=fopen(path,"w"))) {
            showmsg("file open error: %s",path);
            for (i--;i>=0;i--) if (ofp[i]) fclose(ofp[i]);
            return 0;
        }
        /* write header to file */
        switch (i) {
            case 0: outrnxobsh (ofp[0],opt);     break;
            case 1: outrnxnavh (ofp[1],opt,nav); break;
            case 2: outrnxgnavh(ofp[2],opt,nav); break;
            case 3: outrnxhnavh(ofp[3],opt,nav); break;
            case 4: outrnxqnavh(ofp[4],opt,nav); break;
        }
    }
    return 1;
}
/* close output files --------------------------------------------------------*/
static void closefile(FILE **ofp, const rnxopt_t *opt, nav_t *nav)
{
    int i;
    
    trace(3,"closefile:\n");
    
    for (i=0;i<NOUTFILE;i++) {
        
        if (!ofp[i]) continue;
        
        /* rewrite header to file */
        rewind(ofp[i]);
        switch (i) {
            case 0: outrnxobsh (ofp[0],opt);     break;
            case 1: outrnxnavh (ofp[1],opt,nav); break;
            case 2: outrnxgnavh(ofp[2],opt,nav); break;
            case 3: outrnxhnavh(ofp[3],opt,nav); break;
            case 4: outrnxqnavh(ofp[4],opt,nav); break;
        }
        fclose(ofp[i]);
    }
}
/* convert obs message -------------------------------------------------------*/
static void convobs(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *n,
                    unsigned char slips[][NFREQ])
{
    gtime_t time;
    
    trace(3,"convobs :\n");
    
    if (!ofp[0]||str->obs->n<=0) return;
    
    time=str->obs->data[0].time;
    
    /* save slips */
    saveslips(slips,str->obs->data,str->obs->n);
    
    if (!screent(time,opt->ts,opt->te,opt->tint)) return;
    
    /* restore slips */
    restslips(slips,str->obs->data,str->obs->n);
    
    /* output rinex obs */
    outrnxobsb(ofp[0],opt,str->obs->data,str->obs->n,0);
    
    if (opt->tstart.time==0) opt->tstart=time;
    opt->tend=time;
    
    n[0]++;
}
/* convert nav message -------------------------------------------------------*/
static void convnav(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *n)
{
    gtime_t ts1,te1,ts2,te2;
    int sys,prn;
    
    trace(3,"convnav :\n");
    
    ts1=opt->ts; if (ts1.time!=0) ts1=timeadd(ts1,-MAXDTOE);
    te1=opt->te; if (te1.time!=0) te1=timeadd(te1, MAXDTOE);
    ts2=opt->ts; if (ts2.time!=0) ts2=timeadd(ts2,-MAXDTOE_GLO);
    te2=opt->te; if (te2.time!=0) te2=timeadd(te2, MAXDTOE_GLO);
    
    sys=satsys(str->sat,&prn)&opt->navsys;
    
    if (sys==SYS_GPS) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
    }
    else if (sys==SYS_GLO) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts2,te2,0.0)) return;
        
        if (ofp[1]&&opt->rnxver>2.99) {
            
            /* output rinex nav */
            outrnxgnavb(ofp[1],opt,str->nav->geph+prn-1);
            n[1]++;
        }
        if (ofp[2]&&opt->rnxver<=2.99) {
            
            /* output rinex gnav */
            outrnxgnavb(ofp[2],opt,str->nav->geph+prn-1);
            n[2]++;
        }
    }
    else if (sys==SYS_SBS) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&opt->rnxver>2.99) {
            
            /* output rinex nav */
            outrnxhnavb(ofp[1],opt,str->nav->seph+prn-MINPRNSBS);
            n[1]++;
        }
        if (ofp[3]&&opt->rnxver<=2.99) {
            
            /* output rinex hnav */
            outrnxhnavb(ofp[3],opt,str->nav->seph+prn-MINPRNSBS);
            n[3]++;
        }
    }
    else if (sys==SYS_QZS) {
        
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&opt->rnxver>2.99) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
        if (ofp[4]&&opt->rnxver<=2.99) {
            
            /* output rinex qnav */
            outrnxnavb(ofp[4],opt,str->nav->eph+str->sat-1);
            n[4]++;
        }
    }
    else if (sys==SYS_GAL) {
        if (opt->exsats[str->sat-1]==1||!screent(str->time,ts1,te1,0.0)) return;
        
        if (ofp[1]&&opt->rnxver>2.99) {
            
            /* output rinex nav */
            outrnxnavb(ofp[1],opt,str->nav->eph+str->sat-1);
            n[1]++;
        }
    }
}
/* convert sbas message ------------------------------------------------------*/
static void convsbs(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *n)
{
    gtime_t ts1,te1;
    int msg,prn,sat,sys;
    
    trace(3,"convsbs :\n");
    
    ts1=opt->ts; if (ts1.time!=0) ts1=timeadd(ts1,-MAXDTOE);
    te1=opt->te; if (te1.time!=0) te1=timeadd(te1, MAXDTOE);
    
    msg=sbsupdatecorr(&str->raw.sbsmsg,str->nav);
    
    prn=str->raw.sbsmsg.prn;
    if      (MINPRNSBS<=prn&&prn<=MAXPRNSBS) sys=SYS_SBS;
    else if (MINPRNQZS<=prn&&prn<=MAXPRNQZS) sys=SYS_QZS;
    else {
        trace(2,"sbas message satellite error: prn=%d\n",prn);
        return;
    }
    if (!(sat=satno(sys,prn))||opt->exsats[sat-1]==1) return;
    
    if (ofp[5]) { /* output sbas log */
        if (screent(gpst2time(str->raw.sbsmsg.week,str->raw.sbsmsg.tow),opt->ts,
                    opt->te,0.0)) {
            sbsoutmsg(ofp[5],&str->raw.sbsmsg); n[5]++;
        }
    }
    if (!(opt->navsys&SYS_SBS)||msg!=9||
        !screent(str->time,ts1,te1,0.0)) return;
    
    if (ofp[1]&&opt->rnxver>2.99) {
        
        /* output rinex nav */
        outrnxhnavb(ofp[1],opt,str->nav->seph+prn-MINPRNSBS);
        n[1]++;
    }
    if (ofp[3]&&opt->rnxver<=2.99) {
        
        /* output rinex hnav */
        outrnxhnavb(ofp[3],opt,str->nav->seph+prn-MINPRNSBS);
        n[3]++;
    }
}
/* convert lex message -------------------------------------------------------*/
static void convlex(FILE **ofp, rnxopt_t *opt, strfile_t *str, int *n)
{
    gtime_t ts1,te1;
    
    trace(3,"convlex :\n");
    
    ts1=opt->ts; if (ts1.time!=0) ts1=timeadd(ts1,-MAXDTOE);
    te1=opt->te; if (te1.time!=0) te1=timeadd(te1, MAXDTOE);
    
    if (ofp[5]&&screent(str->time,opt->ts,opt->te,0.0)) {
        lexoutmsg(ofp[5],&str->raw.lexmsg); n[5]++;
    }
}
/* show status message -------------------------------------------------------*/
static int showstat(int sess, gtime_t ts, gtime_t te, int *n)
{
    const char type[]="ONGHQSE";
    char msg[1024]="",*p=msg,s[64];
    int i;
    
    if (sess>0) {
        p+=sprintf(p,"(%d) ",sess);
    }
    if (ts.time!=0) {
        time2str(ts,s,0);
        p+=sprintf(p,"%s",s);
    }
    if (te.time!=0&&timediff(te,ts)>0.9) {
        time2str(te,s,0);
        p+=sprintf(p,"-%s",s+5);
    }
    p+=sprintf(p,": ");
    
    for (i=0;i<7;i++) {
        if (n[i]==0) continue;
        p+=sprintf(p,"%c=%d%s",type[i],n[i],i<6?" ":"");
    }
    return showmsg(msg);
}
/* rinex converter for single-session ----------------------------------------*/
static int convrnx_s(int sess, int format, rnxopt_t *opt, const char *file,
                     char **ofile)
{
    FILE *ofp[NOUTFILE]={NULL};
    strfile_t *str;
    gtime_t ts={0},te={0},tend={0},time={0};
    unsigned char slips[MAXSAT][NFREQ]={{0}};
    int i,j,nf,type,n[NOUTFILE+1]={0},abort=0;
    char path[1024],*paths[NOUTFILE],s[NOUTFILE][1024];
    char *epath[MAXEXFILE]={0},*staid=*opt->staid?opt->staid:"0000";
    
    trace(3,"convrnx_s: sess=%d format=%d file=%s ofile=%s %s %s %s %s %s\n",
          sess,format,file,ofile[0],ofile[1],ofile[2],ofile[3],ofile[4],ofile[5]);
    
    /* replace keywords in input file */
    if (reppath(file,path,opt->ts,staid,"")<0) {
        showmsg("no time for input file: %s",file);
        return 0;
    }
    if (!(str=(strfile_t *)malloc(sizeof(strfile_t)))) return 0;
    
    /* initialize stream file */
    if (!init_strfile(str,format,opt->rcvopt,opt->trtcm)) {
        free(str);
        return 0;
    }
    /* expand wild-cards in input file */
    for (i=0;i<MAXEXFILE;i++) {
        if (!(epath[i]=(char *)malloc(1024))) {
            for (i=0;i<MAXEXFILE;i++) free(epath[i]);
            free_strfile(str); free(str);
            return 0;
        }
    }
    nf=expath(path,epath,MAXEXFILE);
    
    /* get start time of stream file */
    gettime_strfile(str,epath[0],&time);
    time=opt->ts.time?opt->ts:(time.time?timeadd(time,TSTARTMARGIN):time);
    
    /* replace keywords in output file */
    for (i=0;i<NOUTFILE;i++) {
        paths[i]=s[i];
        if (reppath(ofile[i],paths[i],time,staid,"")<0) {
            showmsg("no time for output path: %s",ofile[i]);
            for (i=0;i<MAXEXFILE;i++) free(epath[i]);
            free_strfile(str); free(str);
            return 0;
        }
    }
    /* open output files */
    if (!openfile(ofp,paths,path,opt,str->nav)) {
        for (i=0;i<MAXEXFILE;i++) free(epath[i]);
        free_strfile(str); free(str);
        return 0;
    }
    for (i=0;i<nf&&!abort;i++) {
        
        /* open stream file */
        if (!open_strfile(str,epath[i])) continue;
        
        /* input message */
        for (j=0;(type=input_strfile(str))>=-1;j++) {
            
            if (j%11==1&&(abort=showstat(sess,te,te,n))) break;
            
            /* avioid duplicated if overlapped data */
            if (tend.time&&timediff(str->time,tend)<=0.0) continue;
            
            /* convert message */
            switch (type) {
                case  1: convobs(ofp,opt,str,n,slips); break;
                case  2: convnav(ofp,opt,str,n);       break;
                case  3: convsbs(ofp,opt,str,n);       break;
                case 31: convlex(ofp,opt,str,n);       break;
                case -1: n[NOUTFILE]++; break; /* error */
            }
            te=str->time; if (ts.time==0) ts=te;
            
            if (opt->te.time&&timediff(te,opt->te)>10.0) break;
        }
        /* close stream file */
        close_strfile(str);
        
        tend=te; /* end time of a file */
    }
    /* set receiver and antenna information to rnxopt */
    if (format==STRFMT_RTCM2||format==STRFMT_RTCM3) {
        rtcm2opt(&str->rtcm,opt);
    }
    else if (format==STRFMT_RINEX) {
        rnxset2opt(&str->rnxset,opt);
    }
    /* close output files */
    closefile(ofp,opt,str->nav);
    
    /* remove empty output files */
    for (i=0;i<NOUTFILE;i++) {
        if (ofp[i]&&n[i]<=0) remove(ofile[i]);
    }
    if (ts.time>0) showstat(sess,ts,te,n);
    
    for (i=0;i<MAXEXFILE;i++) free(epath[i]);
    free_strfile(str); free(str);
    
    if (opt->tstart.time==0) opt->tstart=opt->ts;
    if (opt->tend  .time==0) opt->tend  =opt->te;
    
    return abort?-1:1;
}
/* rinex converter -------------------------------------------------------------
* convert receiver log file to rinex obs/nav, sbas log files
* args   : int    format I      receiver raw format (STRFMT_???)
*          rnxopt_t *opt IO     rinex options (see below)
*          char   *file  I      rtcm, receiver raw or rinex file
*                               (wild-cards (*) are expanded)
*          char   **ofile IO    output files
*                               ofile[0] rinex obs file   ("": no output)
*                               ofile[1] rinex nav file   ("": no output)
*                               ofile[2] rinex gnav file  ("": no output)
*                               ofile[3] rinex hnav file  ("": no output)
*                               ofile[4] rinex qnav file  ("": no output)
*                               ofile[5] sbas/lex log file("": no output)
* return : status (1:ok,0:error,-1:abort)
* notes  : the following members of opt are replaced by information in last
*          converted rinex: opt->tstart, opt->tend
*          keywords in ofile[] are replaced by first obs date/time and station
*          id (%r)
*          the order of wild-card expanded files must be in-order by time
*-----------------------------------------------------------------------------*/
extern int convrnx(int format, rnxopt_t *opt, const char *file, char **ofile)
{
    gtime_t t0={0};
    rnxopt_t opt_=*opt;
    double tu,ts;
    int i,week,stat=1;
    
    trace(3,"convrnx: format=%d file=%s ofile=%s %s %s %s %s %s\n",
          format,file,ofile[0],ofile[1],ofile[2],ofile[3],ofile[4],ofile[5]);
    
    showmsg("");
    
    if (opt->ts.time==0||opt->te.time==0||opt->tunit<=0.0) {
        
        /* single-session */
        opt_.tstart=opt_.tend=t0;
        stat=convrnx_s(0,format,&opt_,file,ofile);
    }
    else if (timediff(opt->ts,opt->te)<=0.0) {
        
        /* multiple-session */
        tu=opt->tunit<86400.0?opt->tunit:86400.0;
        ts=tu*(int)floor(time2gpst(opt->ts,&week)/tu);
        
        for (i=0;;i++) { /* for each session */
            opt_.ts=gpst2time(week,ts+i*tu);
            opt_.te=timeadd(opt_.ts,tu-DTTOL-0.001);
            opt_.trtcm=timeadd(opt->trtcm,timediff(opt_.ts,opt->ts));
            if (timediff(opt_.ts,opt->te)>0.0) break;
            
            if (timediff(opt_.ts,opt->ts)<0.0) opt_.ts=opt->ts;
            if (timediff(opt_.te,opt->te)>0.0) opt_.te=opt->te;
            opt_.tstart=opt_.tend=t0;
            if ((stat=convrnx_s(i+1,format,&opt_,file,ofile))<0) break;
        }
    }
    else {
        showmsg("no period");
        return 0;
    }
    /* output start/end time */
    opt->tstart=opt_.tstart; opt->tend=opt_.tend;
    
    return stat;
}
