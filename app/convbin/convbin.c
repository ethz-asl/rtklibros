/*------------------------------------------------------------------------------
* convbin.c : convert receiver binary log file to rinex obs/nav, sbas messages
*
*          Copyright (C) 2007-2011 by T.TAKASU, All rights reserved.
*
* options : -DWIN32 use windows file path separator
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 22:13:04 $
* history : 2008/06/22 1.0 new
*           2009/06/17 1.1 support glonass
*           2009/12/19 1.2 fix bug on disable of glonass
*                          fix bug on inproper header for rtcm2 and rtcm3
*           2010/07/18 1.3 add option -v, -t, -h, -x
*           2011/01/15 1.4 add option -ro, -hc, -hm, -hn, -ht, -ho, -hr, -ha,
*                            -hp, -hd, -y, -c, -q 
*                          support gw10 and javad receiver, galileo, qzss
*                          support rinex file name convention
*-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "rtklib.h"

static const char rcsid[]="$Id: convbin.c,v 1.1 2008/07/17 22:13:04 ttaka Exp $";

#define PRGNAME "CONVBIN"

/* help text -----------------------------------------------------------------*/
static const char *help[]={
"",
" Synopsys",
"",
" convbin [-ts y/m/d h:m:s] [-te y/m/d h:m:s] [-ti tint] [-r format] [-ro opts]",
"         [-f freq] [-hc comment] [-hm marker] [-hn markno] [-ht marktype]",
"         [-ho observ] [-hr rec] [-ha ant] [-hp pos] [-hd delta] [-v ver] [-od]",
"         [-os] [-x sat] [-y sys] [-d dir] [-c satid] [-o ofile] [-n nfile]",
"         [-g gfile] [-h hfile] [-q qfile] [-s sfile] file", 
"",
" Description",
"",
" Convert RTCM, receiver raw data log and RINEX file to RINEX and SBAS/LEX",
" message file. SBAS message file complies with RTKLIB SBAS/LEX messsage",
" format. It supports the following messages or files.",
"",
" RTCM 2                : Type 1, 3, 9, 14, 16, 17, 18, 19, 22",
" RTCM 3                : Type 1002, 1004, 1005, 1006, 1010, 1012, 1019, 1020",
" NovAtel OEMV/4,OEMStar: RANGECMPB, RANGEB, RAWEPHEMB, IONUTCB, RAWWASSFRAMEB",
" NovAtel OEM3          : RGEB, REGD, REPB, FRMB, IONB, UTCB",
" u-blox LEA-4T/LEA-5T  : RXM-RAW, RXM-SFRB",
" NovAtel Superstar II  : ID#20, ID#21, ID#22, ID#23, ID#67",
" Hemisphere            : BIN76, BIN80, BIN94, BIN95, BIN96",
" SkyTraq S1315F        : msg0xDD, msg0xE0, msg0xDC",
" GW10                  : msg0x08, msg0x03, msg0x27, msg0x20",
" Javad                 : [R*],[r*],[*R],[*r],[P*],[p*],[*P],[*p],[D*],[*d],",
"                         [E*],[*E],[F*],[TC],[GE],[NE],[EN],[QE],[UO],[IO],",
"                         [WD]",
" RINEX                 : OBS, NAV, GNAV, HNAV QNAV",
"",
" Options [default]",
"",
"     file      input receiver binary log file",
"     -ts y/m/d h:m:s  start time [all]",
"     -te y/m/d h:m:s  end time [all]",
"     -tr y/m/d h:m:s  approximated time for rtcm messages",
"     -ti tint  observation data interval (s) [all]",
"     -r format log format type",
"               rtcm2= RTCM 2",
"               rtcm3= RTCM 3",
"               nov  = NovAtel OEMV/4,OEMStar",
"               oem3 = NovAtel OEM3",
"               ubx  = ublox LEA-4T/LEA-5T",
"               ss2  = NovAtel Superstar II",
"               hemis= Hemisphere Eclipse/Crescent",
"               stq  = SkyTraq S1315F",
"               javad= Javad",
"               rinex= RINEX",
"     -ro opt   receiver options",
"     -f freq   number of frequencies [2]",
"     -hc comment  rinex header: comment line",
"     -hm marker   rinex header: marker name",
"     -hn markno   rinex header: marker number",
"     -ht marktype rinex header: marker type",
"     -ho observ   rinex header: oberver name and agency separated by /",
"     -hr rec      rinex header: receiver number, type and version seperated by /",
"     -ha ant      rinex header: antenna number and type seperated by /",
"     -hp pos      rinex header: approx position x/y/z seperated by /",
"     -hd delta    rinex header: antenna delta h/e/n seperated by /",
"     -v ver    rinex version [2.10]",
"     -od       include doppler frequency [off]",
"     -os       include snr [off]",
"     -x sat    exclude satellite",
"     -y sys    exclude systems (G:GPS,R:GLONASS,E:Galileo,J:QZSS,S:SBAS)",
"     -d dir    output directory [same as input file]",
"     -c staid  use RINEX file name convension with staid [off]",
"     -o ofile  output RINEX OBS file",
"     -n nfile  output RINEX NAV file",
"     -g gfile  output RINEX GNAV file",
"     -h hfile  output RINEX HNAV file",
"     -q qfile  output RINEX QNAV file",
"     -s sfile  output SBAS message file",
"",
" If any output file specified, default output files (<file>.obs,",
" <file>.nav, <file>.gnav, <file>.hnav, <file>.qnav and <file>.sbs) are used.",
"",
" If receiver type is not specified, type is recognized by the input",
" file extention as follows.",
"     *.rtcm2    RTCM 2",
"     *.rtcm3    RTCM 3",
"     *.gps      NovAtel OEMV/4,OEMStar",
"     *.ubx      u-blox LEA-4T/LEA-5T",
"     *.log      NovAtel Superstar II",
"     *.bin      Hemisphere Eclipse/Crescent",
"     *.stq      SkyTraq S1315F",
"     *.jps      Javad",
"     *.obs,*.*o RINEX OBS"
};
/* print help ----------------------------------------------------------------*/
static void printhelp(void)
{
    int i;
    for (i=0;i<sizeof(help)/sizeof(*help);i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
}
/* show message --------------------------------------------------------------*/
extern int showmsg(char *format, ...)
{
    va_list arg;
    va_start(arg,format); vfprintf(stderr,format,arg); va_end(arg);
    fprintf(stderr,"\r");
    return 0;
}
/* convert main --------------------------------------------------------------*/
static int convbin(int format, rnxopt_t *opt, const char *ifile, char **file,
                   char *dir)
{
    int i,def=!file[0]&&!file[1]&&!file[2]&&!file[3]&&!file[4]&&!file[5];
    char work[1024],ofile_[6][1024],*ofile[6],*p;
    char *extnav=opt->rnxver<=2.99||opt->navsys==SYS_GPS?"N":"P";
    char *extlog=format==STRFMT_LEXR?"lex":"sbs";
    
    for (i=0;i<6;i++) ofile[i]=ofile_[i];

    if (file[0]) strcpy(ofile[0],file[0]);
    else if (*opt->staid) {
        strcpy(ofile[0],"%r%n0.%yO");
    }
    else if (def) {
        strcpy(ofile[0],ifile);
        if ((p=strrchr(ofile[0],'.'))) strcpy(p,".obs");
        else strcat(ofile[0],".obs");
    }
    if (file[1]) strcpy(ofile[1],file[1]);
    else if (*opt->staid) {
        strcpy(ofile[1],"%r%n0.%y");
        strcat(ofile[1],extnav);
    }
    else if (def) {
        strcpy(ofile[1],ifile);
        if ((p=strrchr(ofile[1],'.'))) strcpy(p,".nav");
        else strcat(ofile[1],".nav");
    }
    if (file[2]) strcpy(ofile[2],file[2]);
    else if (opt->rnxver<=2.99&&*opt->staid) {
        strcpy(ofile[2],"%r%n0.%yG");
    }
    else if (opt->rnxver<=2.99&&def) {
        strcpy(ofile[2],ifile);
        if ((p=strrchr(ofile[2],'.'))) strcpy(p,".gnav");
        else strcat(ofile[2],".gnav");
    }
    if (file[3]) strcpy(ofile[3],file[3]);
    else if (opt->rnxver<=2.99&&*opt->staid) {
        strcpy(ofile[3],"%r%n0.%yH");
    }
    else if (opt->rnxver<=2.99&&def) {
        strcpy(ofile[3],ifile);
        if ((p=strrchr(ofile[3],'.'))) strcpy(p,".hnav");
        else strcat(ofile[3],".hnav");
    }
    if (file[4]) strcpy(ofile[4],file[4]);
    else if (opt->rnxver<=2.99&&*opt->staid) {
        strcpy(ofile[4],"%r%n0.%yQ");
    }
    else if (opt->rnxver<=2.99&&def) {
        strcpy(ofile[4],ifile);
        if ((p=strrchr(ofile[4],'.'))) strcpy(p,".qnav");
        else strcat(ofile[4],".qnav");
    }
    if (file[5]) strcpy(ofile[5],file[5]);
    else if (*opt->staid) {
        strcpy(ofile[5],"%r%n0_%y.");
        strcat(ofile[5],extlog);
    }
    else if (def) {
        strcpy(ofile[5],ifile);
        if ((p=strrchr(ofile[5],'.'))) strcpy(p,".");
        else strcat(ofile[5],".");
        strcat(ofile[5],extlog);
    }
    for (i=0;i<6;i++) {
        if (!dir||!*ofile[i]) continue;
        if ((p=strrchr(ofile[i],FILEPATHSEP))) strcpy(work,p+1);
/* slynen - decided that this is bullshit{        else strcpy(work,ofile[i]);
        sprintf(ofile[i],"%s%c%s",dir,FILEPATHSEP,work);

 */
    }
    fprintf(stderr,"input file  : %s (%s)\n",ifile,formatstrs[format]);
    
    if (*ofile[0]) fprintf(stderr,"->rinex obs : %s\n",ofile[0]);
    if (*ofile[1]) fprintf(stderr,"->rinex nav : %s\n",ofile[1]);
    if (*ofile[2]) fprintf(stderr,"->rinex gnav: %s\n",ofile[2]);
    if (*ofile[3]) fprintf(stderr,"->rinex hnav: %s\n",ofile[3]);
    if (*ofile[4]) fprintf(stderr,"->rinex qnav: %s\n",ofile[4]);
    if (*ofile[5]) fprintf(stderr,"->sbas log  : %s\n",ofile[5]);
    
    if (!convrnx(format,opt,ifile,ofile)) {
        fprintf(stderr,"\n");
        return -1;
    }
    fprintf(stderr,"\n");
    return 0;
}
/* analyize command line options ---------------------------------------------*/
static int cmdopts(int argc, char **argv, rnxopt_t *opt, char **ifile,
                   char **ofile, char **dir)
{
    double eps[]={1980,1,1,0,0,0},epe[]={2037,12,31,0,0,0};
    double epr[]={2010,1,1,0,0,0};
    int i,j,sat,nf=2,nc=2,format=-1;
    char *p,*sys,*fmt="";
    
    opt->rnxver =RNX2VER;
    opt->obstype=OBSTYPE_PR|OBSTYPE_CP;
    opt->navsys =SYS_GPS|SYS_GLO|SYS_GAL|SYS_QZS|SYS_SBS;
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-ts")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",eps,eps+1,eps+2);
            sscanf(argv[++i],"%lf:%lf:%lf",eps+3,eps+4,eps+5);
            opt->ts=epoch2time(eps);
        }
        else if (!strcmp(argv[i],"-te")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",epe,epe+1,epe+2);
            sscanf(argv[++i],"%lf:%lf:%lf",epe+3,epe+4,epe+5);
            opt->te=epoch2time(epe);
        }
        else if (!strcmp(argv[i],"-tr")&&i+2<argc) {
            sscanf(argv[++i],"%lf/%lf/%lf",epr,epr+1,epr+2);
            sscanf(argv[++i],"%lf:%lf:%lf",epr+3,epr+4,epr+5);
            opt->trtcm=epoch2time(epr);
        }
        else if (!strcmp(argv[i],"-ti")&&i+1<argc) {
            opt->tint=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-r" )&&i+1<argc) {
            fmt=argv[++i];
        }
        else if (!strcmp(argv[i],"-ro")&&i+1<argc) {
            strcpy(opt->rcvopt,argv[++i]);
        }
        else if (!strcmp(argv[i],"-f" )&&i+1<argc) {
            nf=atoi(argv[++i]);
        }
        else if (!strcmp(argv[i],"-hc")&&i+1<argc) {
            if (nc<MAXCOMMENT) strcpy(opt->comment[nc++],argv[++i]);
        }
        else if (!strcmp(argv[i],"-hm")&&i+1<argc) {
            strcpy(opt->marker,argv[++i]);
        }
        else if (!strcmp(argv[i],"-hn")&&i+1<argc) {
            strcpy(opt->markerno,argv[++i]);
        }
        else if (!strcmp(argv[i],"-ht")&&i+1<argc) {
            strcpy(opt->markertype,argv[++i]);
        }
        else if (!strcmp(argv[i],"-ho")&&i+1<argc) {
            for (j=0,p=strtok(argv[++i],"/");j<2&&p;j++,p=strtok(NULL,"/")) {
                strcpy(opt->name[j],p);
            }
        }
        else if (!strcmp(argv[i],"-hr")&&i+1<argc) {
            for (j=0,p=strtok(argv[++i],"/");j<3&&p;j++,p=strtok(NULL,"/")) {
                strcpy(opt->rec[j],p);
            }
        }
        else if (!strcmp(argv[i],"-ha")&&i+1<argc) {
            for (j=0,p=strtok(argv[++i],"/");j<3&&p;j++,p=strtok(NULL,"/")) {
                strcpy(opt->ant[j],p);
            }
        }
        else if (!strcmp(argv[i],"-hp")&&i+1<argc) {
            for (j=0,p=strtok(argv[++i],"/");j<3&&p;j++,p=strtok(NULL,"/")) {
                opt->apppos[j]=atof(p);
            }
        }
        else if (!strcmp(argv[i],"-hd")&&i+1<argc) {
            for (j=0,p=strtok(argv[++i],"/");j<3&&p;j++,p=strtok(NULL,"/")) {
                opt->antdel[j]=atof(p);
            }
        }
        else if (!strcmp(argv[i],"-v" )&&i+1<argc) {
            opt->rnxver=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-od")) {
            opt->obstype|=OBSTYPE_DOP;
        }
        else if (!strcmp(argv[i],"-os")) {
            opt->obstype|=OBSTYPE_SNR;
        }
        else if (!strcmp(argv[i],"-x" )&&i+1<argc) {
            if ((sat=satid2no(argv[++i]))) opt->exsats[sat-1]=1;
        }
        else if (!strcmp(argv[i],"-y" )&&i+1<argc) {
            sys=argv[++i];
            if      (!strcmp(sys,"G")) opt->navsys&=~SYS_GPS;
            else if (!strcmp(sys,"R")) opt->navsys&=~SYS_GLO;
            else if (!strcmp(sys,"E")) opt->navsys&=~SYS_GAL;
            else if (!strcmp(sys,"J")) opt->navsys&=~SYS_QZS;
            else if (!strcmp(sys,"S")) opt->navsys&=~SYS_SBS;
        }
        else if (!strcmp(argv[i],"-d" )&&i+1<argc) {
            *dir=argv[++i];
        }
        else if (!strcmp(argv[i],"-c" )&&i+1<argc) {
            strcpy(opt->staid,argv[++i]);
        }
        else if (!strcmp(argv[i],"-o" )&&i+1<argc) ofile[0]=argv[++i];
        else if (!strcmp(argv[i],"-n" )&&i+1<argc) ofile[1]=argv[++i];
        else if (!strcmp(argv[i],"-g" )&&i+1<argc) ofile[2]=argv[++i];
        else if (!strcmp(argv[i],"-h" )&&i+1<argc) ofile[3]=argv[++i];
        else if (!strcmp(argv[i],"-q" )&&i+1<argc) ofile[4]=argv[++i];
        else if (!strcmp(argv[i],"-s" )&&i+1<argc) ofile[5]=argv[++i];
        
        else if (!strncmp(argv[i],"-",1)) printhelp();
        
        else *ifile=argv[i];
    }
    if (nf>=1) opt->freqtype|=FREQTYPE_L1;
    if (nf>=2) opt->freqtype|=FREQTYPE_L2;
    if (nf>=3) opt->freqtype|=FREQTYPE_L5;
    if (nf>=4) opt->freqtype|=FREQTYPE_L6;
    if (nf>=5) opt->freqtype|=FREQTYPE_L7;
    if (nf>=6) opt->freqtype|=FREQTYPE_L8;
    
    if (*fmt) {
        if      (!strcmp(fmt,"rtcm2")) format=STRFMT_RTCM2;
        else if (!strcmp(fmt,"rtcm3")) format=STRFMT_RTCM3;
        else if (!strcmp(fmt,"nov"  )) format=STRFMT_OEM4;
        else if (!strcmp(fmt,"oem3" )) format=STRFMT_OEM3;
        else if (!strcmp(fmt,"ubx"  )) format=STRFMT_UBX;
        else if (!strcmp(fmt,"ss2"  )) format=STRFMT_SS2;
        else if (!strcmp(fmt,"hemis")) format=STRFMT_CRES;
        else if (!strcmp(fmt,"stq"  )) format=STRFMT_STQ;
        else if (!strcmp(fmt,"javad")) format=STRFMT_JAVAD;
        else if (!strcmp(fmt,"rinex")) format=STRFMT_RINEX;
    }
    else if ((p=strrchr(*ifile,'.'))) {
        if      (!strcmp(p,".rtcm2"))  format=STRFMT_RTCM2;
        else if (!strcmp(p,".rtcm3"))  format=STRFMT_RTCM3;
        else if (!strcmp(p,".gps"  ))  format=STRFMT_OEM4;
        else if (!strcmp(p,".ubx"  ))  format=STRFMT_UBX;
        else if (!strcmp(p,".log"  ))  format=STRFMT_SS2;
        else if (!strcmp(p,".bin"  ))  format=STRFMT_CRES;
        else if (!strcmp(p,".stq"  ))  format=STRFMT_STQ;
        else if (!strcmp(p,".jps"  ))  format=STRFMT_JAVAD;
        else if (!strcmp(p,".obs"  ))  format=STRFMT_RINEX;
        else if (!strcmp(p+3,"o"   ))  format=STRFMT_RINEX;
        else if (!strcmp(p+3,"O"   ))  format=STRFMT_RINEX;
    }
    return format;
}
/* main ----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
    rnxopt_t opt={{0}};
    int format;
    char *ifile="",*ofile[6]={0},*dir="";
    
    /* analyize command line options */
    format=cmdopts(argc,argv,&opt,&ifile,ofile,&dir);
    
    if (!*ifile) {
        fprintf(stderr,"no input file\n");
        return -1;
    }
    if (format<0) {
        fprintf(stderr,"input format can not be recognized\n");
        return -1;
    }
    sprintf(opt.prog,"%s %s",PRGNAME,VER_RTKLIB);
    sprintf(opt.comment[0],"log: %-55.55s",ifile);
    sprintf(opt.comment[1],"format: %s",formatstrs[format]);
    if (*opt.rcvopt) {
        strcat(opt.comment[1],", option: ");
        strcat(opt.comment[1],opt.rcvopt);
    }
    return convbin(format,&opt,ifile,ofile,dir);
}
