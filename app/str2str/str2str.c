/*------------------------------------------------------------------------------
* str2str.c : console version of stream server
*
*          Copyright (C) 2007-2011 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:54:53 $
* history : 2009/06/17  1.0 new
*           2011/05/29  1.1 add -f, -l and -x option
*-----------------------------------------------------------------------------*/
#include <signal.h>
#include <unistd.h>
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define PRGNAME     "str2str"          /* program name */
#define MAXSTR      5                  /* max number of streams */
#define MAXRCVCMD   4096               /* max length of receiver command */
#define TRFILE      "str2str.trace"    /* trace file */

/* global variables ----------------------------------------------------------*/
static strsvr_t strsvr;                /* stream server */
static int intrflg=0;                  /* interrupt flag */

/* help text -----------------------------------------------------------------*/
static const char *help[]={
"",
" usage: str2str -in stream -out stream [-out stream...] [options]",
"",
" Input data from a stream and devide and output them to multiple streams",
" The input stream can be serial, tcp client, tcp server, ntrip client, or",
" file. The output stream can be serial, tcp client, tcp server, ntrip server,",
" or file. str2str is a resident type application. To stop it, type ctr-c in",
" console if run foreground or send signal SIGINT for background process.",
" Command options are as follows.",
"",
" -in stream    input stream path",
" -out stream   output stream path",
"                 serial://port[:brate[:bsize[:parity[:stopb[:fctr]]]]]",
"                 tcpsvr://:port",
"                 tcpcli://addr[:port]",
"                 ntrip://[user[:passwd]@]addr[:port][/mntpnt]",
"                 ntrips://[user[:passwd]@]addr[:port][/mntpnt[:str]]",
"                 file://path",
" -d msec       status disply interval (ms) [5000]",
" -s msec       timeout time (ms) [10000]",
" -r msec       reconnect interval (ms) [10000]",
" -n msec       nmea request cycle (m) [0]",
" -f sec        file swap margin (s) [30]",
" -c file       receiver commands file [no]",
" -p lat lon    nmea position (latitude/longitude) (deg)",
" -l local_dir  ftp/http local directory []",
" -x proxy_addr http/ntrip proxy address [no]",
" -t level      trace level [0]",
" -h            print help",
};
/* print help ----------------------------------------------------------------*/
static void printhelp(void)
{
    int i;
    for (i=0;i<sizeof(help)/sizeof(*help);i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
}
/* signal handler ------------------------------------------------------------*/
static void sigfunc(int sig)
{
    intrflg=1;
}
/* set path ------------------------------------------------------------------*/
static int setstrpath(const char *path, int *type, char *strpath)
{
    char *p;
    if (!(p=strstr(path,"://"))) return 0;
    if      (!strncmp(path,"serial",6)) *type=STR_SERIAL;
    else if (!strncmp(path,"tcpsvr",6)) *type=STR_TCPSVR;
    else if (!strncmp(path,"tcpcli",6)) *type=STR_TCPCLI;
    else if (!strncmp(path,"ntrip", 5)) *type=STR_NTRIPCLI;
    else if (!strncmp(path,"ntrips",6)) *type=STR_NTRIPSVR;
    else if (!strncmp(path,"file",  4)) *type=STR_FILE;
    else return 0;
    strcpy(strpath,p+3);
    return 1;
}
/* read receiver commands ----------------------------------------------------*/
static void readcmd(const char *file, char *cmd, int type)
{
    FILE *fp;
    char buff[MAXSTR],*p=cmd;
    int i=0;
    
    *p='\0';
    
    if (!(fp=fopen(file,"r"))) return;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (*buff=='@') i=1;
        else if (i==type&&p+strlen(buff)+1<cmd+MAXRCVCMD) {
            p+=sprintf(p,"%s",buff);
        }
    }
    fclose(fp);
}
/* str2str -------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    static char cmd[MAXRCVCMD]="";
    const char ss[]={'E','-','W','C','C'};
    double pos[2]={0};
    char *paths[MAXSTR],s[MAXSTR][MAXSTRPATH]={{0}},*cmdfile="";
    char *local="",*proxy="";
    char *p,str[64]="",msg[1024]="";
    int i,n=0,dispint=5000,trlevel=0,opts[]={10000,10000,2000,32768,10,0,30};
    int types[MAXSTR]={0},stat[MAXSTR]={0},byte[MAXSTR]={0},bps[MAXSTR]={0};
    
    for (i=0;i<MAXSTR;i++) paths[i]=s[i];
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-in")&&i+1<argc) {
            if (!setstrpath(argv[++i],types,paths[0])) {
                fprintf(stderr,"input stream path error: %s\n",argv[i-1]);
            }
        }
        else if (!strcmp(argv[i],"-out")&&i+1<argc&&n<MAXSTR-1) {
            if (!setstrpath(argv[++i],types+n+1,paths[n+1])) {
                fprintf(stderr,"output stream path error: %s\n",argv[i-1]);
            }
            n++;
        }
        else if (!strcmp(argv[i],"-p")&&i+2<argc) {
            pos[0]=atof(argv[++i]);
            pos[1]=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-d" )&&i+1<argc) dispint=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-s" )&&i+1<argc) opts[0]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-r" )&&i+1<argc) opts[1]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-n" )&&i+1<argc) opts[5]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-f" )&&i+1<argc) opts[6]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-c" )&&i+1<argc) cmdfile=argv[++i];
        else if (!strcmp(argv[i],"-l" )&&i+1<argc) local=argv[++i];
        else if (!strcmp(argv[i],"-x" )&&i+1<argc) proxy=argv[++i];
        else if (!strcmp(argv[i],"-t" )&&i+1<argc) trlevel=atoi(argv[++i]);
        else if (*argv[i]=='-') printhelp();
    }
    if (!*paths[0]) {
        fprintf(stderr,"specify input stream\n");
        return -1;
    }
    if (n<=0) {
        fprintf(stderr,"specify output stream(s)\n");
        return -1;
    }
    signal(SIGINT,sigfunc);
    signal(SIGPIPE,SIG_IGN);
    
    strsvrinit(&strsvr,n+1);
    
    traceopen(TRFILE);
    tracelevel(trlevel);
    
    fprintf(stderr,"stream server start\n");
    
    strsetdir(local);
    strsetproxy(proxy);
    
    /* start stream server */
    if (*cmdfile) readcmd(cmdfile,cmd,0);
    if (!strsvrstart(&strsvr,opts,types,paths,*cmd?cmd:NULL,pos)) {
        fprintf(stderr,"stream server start error\n");
        return -1;
    }
    for (intrflg=0;!intrflg;) {
        strsvrstat(&strsvr,stat,byte,bps,msg);
        for (i=0,p=str;i<MAXSTR;i++) p+=sprintf(p,"%c",ss[stat[i]+1]);
        fprintf(stderr,"%s [%s] %10d B %6d bps %s\n",
                time_str(utc2gpst(timeget()),0),str,byte[0],bps[0],msg);
        sleepms(dispint);
    }
    if (*cmdfile) readcmd(cmdfile,cmd,0);
    strsvrstop(&strsvr,*cmd?cmd:NULL);
    fprintf(stderr,"stream server stop\n");
    return 0;
}
