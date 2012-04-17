/*------------------------------------------------------------------------------
* streamsvr.c : stream server functions
*
*          Copyright (C) 2010-2011 by T.TAKASU, All rights reserved.
*
* options : -DWIN32    use WIN32 API
*
* version : $Revision:$ $Date:$
* history : 2010/07/18 1.0  moved from stream.c
*           2011/01/18 1.1  change api strsvrstart()
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id$";

/* stearm server thread ------------------------------------------------------*/
#ifdef WIN32
static DWORD WINAPI strsvrthread(void *arg)
#else
static void *strsvrthread(void *arg)
#endif
{
    strsvr_t *svr=(strsvr_t *)arg;
    unsigned int tick,ticknmea;
    int i,n;
    
    tracet(3,"strsvrthread:\n");
    
    svr->state=1;
    svr->tick=tickget();
    ticknmea=svr->tick-1000;
    
    while (svr->state) {
        tick=tickget();
        
        n=strread(svr->stream,svr->buff,svr->buffsize);
        for (i=1;i<svr->nstr;i++) {
            strwrite(svr->stream+i,svr->buff,n);
        }
        if (svr->nmeacycle>0&&(int)(tick-ticknmea)>=svr->nmeacycle) {
            strsendnmea(svr->stream,svr->nmeapos);
            ticknmea=tick;
        }
        lock(&svr->lock);
        for (i=0;i<n&&svr->npb<svr->buffsize;i++) {
            svr->pbuf[svr->npb++]=svr->buff[i];
        }
        unlock(&svr->lock);
        
        sleepms(svr->cycle-(int)(tickget()-tick));
    }
    for (i=0;i<svr->nstr;i++) strclose(svr->stream+i);
    svr->npb=0;
    free(svr->buff); svr->buff=NULL;
    free(svr->pbuf); svr->pbuf=NULL;
    
    return 0;
}
/* initialize stream server ----------------------------------------------------
* initialize stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          int    nout      I   number of output streams
* return : none
*-----------------------------------------------------------------------------*/
extern void strsvrinit(strsvr_t *svr, int nout)
{
    int i;
    
    tracet(3,"strsvrinit: nout=%d\n",nout);
    
    svr->state=0;
    svr->cycle=0;
    svr->buffsize=0;
    svr->nmeacycle=0;
    svr->npb=0;
    for (i=0;i<3;i++) svr->nmeapos[i]=0.0;
    svr->buff=svr->pbuf=NULL;
    svr->tick=0;
    for (i=0;i<nout+1&&i<16;i++) strinit(svr->stream+i);
    svr->nstr=i;
    svr->thread=0;
    initlock(&svr->lock);
}
/* start stream server ---------------------------------------------------------
* start stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          int    *opts     I   stream options
*              opts[0]= inactive timeout (ms)
*              opts[1]= interval to reconnect (ms)
*              opts[2]= averaging time of data rate (ms)
*              opts[3]= receive/send buffer size (bytes);
*              opts[4]= server cycle (ms)
*              opts[5]= nmea request cycle (ms) (0:no)
*              opts[6]= file swap margin (s)
*          int    *strs     I   stream types (STR_???)
*              strs[0]= input stream
*              strs[1]= output stream
*              ...
*          char   **paths   I   stream paths
*          char   *cmd      I   input stream start command (NULL: no cmd)
*          double *nmeapos  I   nmea request position (ecef) (m) (NULL: no)
* return : status (0:error,1:ok)
*-----------------------------------------------------------------------------*/
extern int strsvrstart(strsvr_t *svr, int *opts, int *strs, char **paths,
                       const char *cmd, const double *nmeapos)
{
    int i,rw,stropt[5]={0};
    char file1[MAXSTRPATH],file2[MAXSTRPATH],*p;
    
    tracet(3,"strsvrstart:\n");

    if (svr->state) return 0;
    
    strinitcom();
    
    for (i=0;i<4;i++) stropt[i]=opts[i];
    stropt[4]=opts[6];
    strsetopt(stropt);
    svr->cycle=opts[4];
    svr->buffsize=opts[3]<4096?4096:opts[3]; /* >=4096byte */
    svr->nmeacycle=0<opts[5]&&opts[5]<1000?1000:opts[5]; /* >=1s */
    for (i=0;i<3;i++) svr->nmeapos[i]=nmeapos?nmeapos[i]:0.0;
    
    if (!(svr->buff=(unsigned char *)malloc(svr->buffsize))||
        !(svr->pbuf=(unsigned char *)malloc(svr->buffsize))) {
        free(svr->buff); free(svr->pbuf);
        return 0;
    }
    /* open streams */
    for (i=0;i<svr->nstr;i++) {
        strcpy(file1,paths[0]); if ((p=strstr(file1,"::"))) *p='\0';
        strcpy(file2,paths[i]); if ((p=strstr(file2,"::"))) *p='\0';
        if (i>0&&!strcmp(file1,file2)) {
            sprintf(svr->stream[i].msg,"output path error: %s",file2);
            for (i--;i>=0;i--) strclose(svr->stream+i);
            return 0;
        }
        rw=i==0?STR_MODE_R:STR_MODE_W;
        if (strs[i]!=STR_FILE) rw|=STR_MODE_W;

    fprintf(stdout,"opening stream %i %s\n",i, paths[i]);
       if (stropen(svr->stream+i,strs[i],rw,paths[i])){
 continue;
}else{
    fprintf(stdout,"Error opening port %i: %s\n",i, paths[i]);  
}
        for (i--;i>=0;i--) strclose(svr->stream+i);
        return 0;
    }
    /* write start command to input stream */
    if (cmd) strsendcmd(svr->stream,cmd);
    
    /* create stream server thread */
#ifdef WIN32
    if (!(svr->thread=CreateThread(NULL,0,strsvrthread,svr,0,NULL))) {
#else
    if (pthread_create(&svr->thread,NULL,strsvrthread,svr)) {
#endif
 fprintf(stderr,"thread creation fail\n");  
        for (i=0;i<svr->nstr;i++) strclose(svr->stream+i);
        return 0;
    }
    return 1;
}
/* stop stream server ----------------------------------------------------------
* start stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          char  *cmd       I   input stop command (NULL: no cmd)
* return : none
*-----------------------------------------------------------------------------*/
extern void strsvrstop(strsvr_t *svr, const char *cmd)
{
    tracet(3,"strsvrstop:\n");
    
    if (cmd) strsendcmd(svr->stream,cmd);
    
    svr->state=0;
    
#ifdef WIN32
    WaitForSingleObject(svr->thread,10000);
    CloseHandle(svr->thread);
#else
    pthread_join(svr->thread,NULL);
#endif
}
/* get stream server status ----------------------------------------------------
* get status of stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          int    *stat     O   stream status
*          int    *byte     O   bytes received/sent
*          int    *bps      O   bitrate received/sent
*          char   *msg      O   messages
* return : none
*-----------------------------------------------------------------------------*/
extern void strsvrstat(strsvr_t *svr, int *stat, int *byte, int *bps, char *msg)
{
    char s[MAXSTRMSG]="",*p=msg;
    int i;
    
    tracet(4,"strsvrstat:\n");
    
    for (i=0;i<svr->nstr;i++) {
        if (i==0) {
            strsum(svr->stream,byte,bps,NULL,NULL);
            stat[i]=strstat(svr->stream,s);
        }
        else {
            strsum(svr->stream+i,NULL,NULL,byte+i,bps+i);
            stat[i]=strstat(svr->stream+i,s);
        }
        if (*s) p+=sprintf(p,"(%d) %s ",i,s);
    }
}
/* peek input/output stream ----------------------------------------------------
* peek input/output stream of stream server
* args   : strsvr_t *svr    IO  stream sever struct
*          unsigend char *msg O stream buff
*          int    nmax      I  buffer size (bytes)
* return : stream size (bytes)
*-----------------------------------------------------------------------------*/
extern int strsvrpeek(strsvr_t *svr, unsigned char *buff, int nmax)
{
    int n;
    
    if (!svr->state) return 0;
    
    lock(&svr->lock);
    n=svr->npb<nmax?svr->npb:nmax;
    if (n>0) {
        memcpy(buff,svr->pbuf,n);
    }
    if (n<svr->npb) {
        memmove(svr->pbuf,svr->pbuf+n,svr->npb-n);
    }
    svr->npb-=n;
    unlock(&svr->lock);
    return n;
}
