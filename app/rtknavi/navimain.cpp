//---------------------------------------------------------------------------
// rtknavi : real-time positioning ap
//
//          Copyright (C) 2007-2011 by T.TAKASU, All rights reserved.
//
// options : rtknavi [-t title][-i file]
//
//           -t title   window title
//           -i file    ini file path
//
// version : $Revision:$ $Date:$
// history : 2008/07/14  1.0 new
//           2010/07/18  1.1 rtklib 2.4.0
//           2010/08/16  1.2 fix bug on setting of satellite antenna model
//           2010/09/04  1.3 fix bug on setting of receiver antenna delta
//           2011/06/10  1.4 rtklib 2.4.1
//---------------------------------------------------------------------------
#include <vcl.h>
#include <vcl\inifiles.hpp>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>
#pragma hdrstop

#include "rtklib.h"
#include "instrdlg.h"
#include "outstrdlg.h"
#include "logstrdlg.h"
#include "mondlg.h"
#include "tcpoptdlg.h"
#include "confdlg.h"
#include "aboutdlg.h"
#include "viewer.h"
#include "naviopt.h"
#include "navimain.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

#define PRGNAME     "RTKNAVI"           // program name
#define TRACEFILE   "rtknavi.trace"     // debug trace file
#define CLORANGE    (TColor)0x00AAFF
#define CLLGRAY     (TColor)0xDDDDDD
#define CHARDEG     "\260"              // character code of degree
#define SATSIZE     20                  // satellite circle size in skyplot
#define MINSNR      10                  // minimum snr
#define MAXSNR      60                  // maximum snr
#define KEYF6       0x75                // code of function key F6
#define KEYF7       0x76                // code of function key F7
#define KEYF8       0x77                // code of function key F8
#define KEYF9       0x78                // code of function key F9
#define KEYF10      0x79                // code of function key F10
#define POSFONTNAME "Palatino Linotype"
#define POSFONTSIZE 12
#define MINBLLEN    0.01                // minimum baseline length to show

#define KACYCLE     1000                // keep alive cycle (ms)
#define TIMEOUT     10000               // inactive timeout time (ms)
#define DEFAULTPORT 52001               // default monitor port number
#define MAXPORTOFF  9                   // max port number offset

#define SQRT(x)     ((x)<0.0?0.0:sqrt(x))

//---------------------------------------------------------------------------

rtksvr_t rtksvr;                        // rtk server struct
stream_t monistr;                       // monitor stream

// show message in message area ---------------------------------------------
extern "C" {
extern int showmsg(char *format,...) {return 0;}
}
// convert degree to deg-min-sec --------------------------------------------
static void degtodms(double deg, double *dms)
{
    double sgn=1.0;
    if (deg<0.0) {deg=-deg; sgn=-1.0;}
    dms[0]=floor(deg);
    dms[1]=floor((deg-dms[0])*60.0);
    dms[2]=(deg-dms[0]-dms[1]/60.0)*3600;
    dms[0]*=sgn;
}
// convert deg-min-sec to degree --------------------------------------------
int __fastcall TMainForm::ExecCmd(AnsiString cmd, int show)
{
    PROCESS_INFORMATION info;
    STARTUPINFO si={0};
    si.cb=sizeof(si);
    char *p=cmd.c_str();
    if (!CreateProcess(NULL,p,NULL,NULL,false,show?0:CREATE_NO_WINDOW,NULL,
                       NULL,&si,&info)) return 0;
    CloseHandle(info.hProcess);
    CloseHandle(info.hThread);
    return 1;
}
// constructor --------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
    : TForm(Owner)
{
    SvrCycle=SvrBuffSize=0;
    SolBuffSize=1000;
    for (int i=0;i<8;i++) {
        StreamC[i]=Stream[i]=Format[i]=CmdEna[i][0]=CmdEna[i][1]=0;
    }
    TimeSys=SolType=PlotType=0;
    PSol=PSolS=PSolE=Nsat[0]=Nsat[1]=0;
    NMapPnt=0;
    OpenPort=0;
    Time=NULL;
    SolStat=Nvsat=NULL;
    SolCurrentStat=0;
    SolRov=SolRef=Qr=VelRov=Age=Ratio=NULL;
    for (int i=0;i<2;i++) for (int j=0;j<MAXSAT;j++) {
        Sat[i][j]=Vsat[i][j]=0;
        Az[i][j]=El[i][j]=0.0;
        for (int k=0;k<NFREQ;k++) Snr[i][j][k]=0;
    }
    PrcOpt=prcopt_default;
    SolOpt=solopt_default;
    PosFont=new TFont;
    
    rtksvrinit(&rtksvr);
    strinit(&monistr);
    
    Caption=PRGNAME;
    Caption=Caption+" ver."+VER_RTKLIB;
    DoubleBuffered=true;
}
// callback on form create --------------------------------------------------
void __fastcall TMainForm::FormCreate(TObject *Sender)
{
    char *p,*argv[32],buff[1024];
    int argc=0;
    
    trace(3,"FormCreate\n");
    
    IniFile="rtknavi.ini";
    
    InitSolBuff();
    SetTrayIcon(1);
    strinitcom();
    
    strcpy(buff,GetCommandLine());
    
    for (p=buff;*p&&argc<32;p++) {
        if (*p==' ') continue;
        if (*p=='"') {
            argv[argc++]=p+1;
            if (!(p=strchr(p+1,'"'))) break;
        }
        else {
            argv[argc++]=p;
            if (!(p=strchr(p+1,' '))) break;
        }
        *p='\0';
    }
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-i")&&i+1<argc) IniFile=argv[++i];
    }
    LoadOpt();
    
    for (int i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-t")&&i+1<argc) Caption=argv[++i];
    }
    LoadNav(&rtksvr.nav);
    
    OpenMoniPort(MoniPort);
}
// callback on form show ----------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
    trace(3,"FormShow\n");
    
    Panel21->Visible=PlotType<=4;
    IndQ->Visible=!Panel21->Visible;
    BtnSolType2->Visible=!Panel21->Visible;
    UpdateTimeSys();
    UpdateSolType();
    UpdateFont();
    UpdatePos();
    UpdatePlot();
}
// callback on form close ---------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
    trace(3,"FormClose\n");
    
    if (OpenPort>0) {
        // send disconnect message
        strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));
        
        strclose(&monistr);
    }
    SaveOpt();
    SaveNav(&rtksvr.nav);
}
// callback panel 22 resize -------------------------------------------------
void __fastcall TMainForm::Panel22Resize(TObject *Sender)
{
    trace(3,"Panel22Resize\n");
    
    PlotWidth =Panel22->Width -2;
    PlotHeight=Panel22->Height-2;
    BtnPlotType->Left=Panel22->Width-BtnPlotType->Width-2;
    BtnFreqType->Left=BtnPlotType->Left-BtnFreqType->Width;
    BtnSolType2->Left=BtnFreqType->Left-BtnFreqType->Width;
}
// callback on button-exit --------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
    trace(3,"BtnExitClick\n");
    
    Close();
}
// callback on button-start -------------------------------------------------
void __fastcall TMainForm::BtnStartClick(TObject *Sender)
{
    trace(3,"BtnStartClick\n");
    
    SvrStart();
}
// callback on button-stopp -------------------------------------------------
void __fastcall TMainForm::BtnStopClick(TObject *Sender)
{
    trace(3,"BtnStopClick\n");
    
    SvrStop();
}
// callback on button-plot --------------------------------------------------
void __fastcall TMainForm::BtnPlotClick(TObject *Sender)
{
    AnsiString cmd;
    
    trace(3,"BtnPlotClick\n");
    
    if (OpenPort<=0) {
        ShowMessage("monitor port not open");
        return;
    }
    cmd.sprintf("rtkplot -p tcpcli://localhost:%d -t \"%s %s\"",OpenPort,
                Caption.c_str(),": RTKPLOT");
    if (!ExecCmd(cmd,1)) {
        ShowMessage("error : rtkplot execution");
    }
}
// callback on button-options -----------------------------------------------
void __fastcall TMainForm::BtnOptClick(TObject *Sender)
{
    int i,chgmoni=0;
    
    trace(3,"BtnOptClick\n");
    
    OptDialog->PrcOpt     =PrcOpt;
    OptDialog->SolOpt     =SolOpt;
    OptDialog->DebugStatusF=DebugStatusF;
    OptDialog->DebugTraceF=DebugTraceF;
    OptDialog->BaselineC  =BaselineC;
    OptDialog->Baseline[0]=Baseline[0];
    OptDialog->Baseline[1]=Baseline[1];
    
    OptDialog->RovPosTypeF=RovPosTypeF;
    OptDialog->RefPosTypeF=RefPosTypeF;
    OptDialog->RovAntPcvF =RovAntPcvF;
    OptDialog->RefAntPcvF =RefAntPcvF;
    OptDialog->RovAntF    =RovAntF;
    OptDialog->RefAntF    =RefAntF;
    
    OptDialog->SatPcvFileF=SatPcvFileF;
    OptDialog->AntPcvFileF=AntPcvFileF;
    OptDialog->StaPosFileF=StaPosFileF;
    OptDialog->GeoidDataFileF=GeoidDataFileF;
    OptDialog->DCBFileF   =DCBFileF;
    OptDialog->LocalDirectory=LocalDirectory;
    
    OptDialog->SvrCycle   =SvrCycle;
    OptDialog->TimeoutTime=TimeoutTime;
    OptDialog->ReconTime  =ReconTime;
    OptDialog->NmeaCycle  =NmeaCycle;
    OptDialog->FileSwapMargin=FileSwapMargin;
    OptDialog->SvrBuffSize=SvrBuffSize;
    OptDialog->SolBuffSize=SolBuffSize;
    OptDialog->SavedSol   =SavedSol;
    OptDialog->NavSelect  =NavSelect;
    OptDialog->DgpsCorr   =DgpsCorr;
    OptDialog->SbasCorr   =SbasCorr;
    OptDialog->ExSats     =ExSats;
    OptDialog->ProxyAddr  =ProxyAddr;
    OptDialog->MoniPort   =MoniPort;
    
    for (i=0;i<3;i++) {
        OptDialog->RovAntDel[i]=RovAntDel[i];
        OptDialog->RefAntDel[i]=RefAntDel[i];
        OptDialog->RovPos   [i]=RovPos   [i];
        OptDialog->RefPos   [i]=RefPos   [i];
    }
    OptDialog->PosFont->Assign(PosFont);
    
    if (OptDialog->ShowModal()!=mrOk) return;
    
    PrcOpt     =OptDialog->PrcOpt;
    SolOpt     =OptDialog->SolOpt;
    DebugStatusF=OptDialog->DebugStatusF;
    DebugTraceF=OptDialog->DebugTraceF;
    BaselineC  =OptDialog->BaselineC;
    Baseline[0]=OptDialog->Baseline[0];
    Baseline[1]=OptDialog->Baseline[1];
    
    RovPosTypeF=OptDialog->RovPosTypeF;
    RefPosTypeF=OptDialog->RefPosTypeF;
    RovAntPcvF =OptDialog->RovAntPcvF;
    RefAntPcvF =OptDialog->RefAntPcvF;
    RovAntF    =OptDialog->RovAntF;
    RefAntF    =OptDialog->RefAntF;
    
    SatPcvFileF=OptDialog->SatPcvFileF;
    AntPcvFileF=OptDialog->AntPcvFileF;
    StaPosFileF=OptDialog->StaPosFileF;
    GeoidDataFileF=OptDialog->GeoidDataFileF;
    DCBFileF   =OptDialog->DCBFileF;
    LocalDirectory=OptDialog->LocalDirectory;
    
    SvrCycle   =OptDialog->SvrCycle;
    TimeoutTime=OptDialog->TimeoutTime;
    ReconTime  =OptDialog->ReconTime;
    NmeaCycle  =OptDialog->NmeaCycle;
    FileSwapMargin=OptDialog->FileSwapMargin;
    SvrBuffSize=OptDialog->SvrBuffSize;
    SavedSol   =OptDialog->SavedSol;
    NavSelect  =OptDialog->NavSelect;
    DgpsCorr   =OptDialog->DgpsCorr;
    SbasCorr   =OptDialog->SbasCorr;
    ExSats     =OptDialog->ExSats;
    ProxyAddr  =OptDialog->ProxyAddr;
    if (MoniPort!=OptDialog->MoniPort) chgmoni=1;
    MoniPort   =OptDialog->MoniPort;
    
    if (SolBuffSize!=OptDialog->SolBuffSize) {
        SolBuffSize=OptDialog->SolBuffSize;
        InitSolBuff();
        UpdateTime();
        UpdatePos();
        UpdatePlot();
    }
    for (i=0;i<3;i++) {
        RovAntDel[i]=OptDialog->RovAntDel[i];
        RefAntDel[i]=OptDialog->RefAntDel[i];
        RovPos   [i]=OptDialog->RovPos   [i];
        RefPos   [i]=OptDialog->RefPos   [i];
    }
    PosFont->Assign(OptDialog->PosFont);
    
    UpdateFont();
    
    if (!chgmoni) return;
    
    // send disconnect message
    if (OpenPort>0) {
        strwrite(&monistr,(unsigned char *)MSG_DISCONN,strlen(MSG_DISCONN));
        
        strclose(&monistr);
    }
    // reopen monitor stream
    OpenMoniPort(MoniPort);
}
// callback on button-input-streams -----------------------------------------
void __fastcall TMainForm::BtnInputStrClick(TObject *Sender)
{
    int i,j;
    
    trace(3,"BtnInputStrClick\n");
    
    for (i=0;i<3;i++) {
        InputStrDialog->StreamC[i]=StreamC[i];
        InputStrDialog->Stream [i]=Stream [i];
        InputStrDialog->Format [i]=Format [i];
        InputStrDialog->RcvOpt [i]=RcvOpt [i];
        
        /* Paths[0]:serial,[1]:tcp,[2]:file,[3]:ftp */
        for (j=0;j<4;j++) InputStrDialog->Paths[i][j]=Paths[i][j];
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        InputStrDialog->CmdEna[i][j]=CmdEna[i][j];
        InputStrDialog->Cmds  [i][j]=Cmds  [i][j];
    }
    for (i=0;i<10;i++) {
        InputStrDialog->History [i]=History [i];
        InputStrDialog->MntpHist[i]=MntpHist[i];
    }
    InputStrDialog->NmeaReq   =NmeaReq;
    InputStrDialog->TimeTag   =InTimeTag;
    InputStrDialog->TimeSpeed =InTimeSpeed;
    InputStrDialog->TimeStart =InTimeStart;
    InputStrDialog->NmeaPos[0]=NmeaPos[0];
    InputStrDialog->NmeaPos[1]=NmeaPos[1];
    
    if (InputStrDialog->ShowModal()!=mrOk) return;
    
    for (i=0;i<3;i++) {
        StreamC[i]=InputStrDialog->StreamC[i];
        Stream [i]=InputStrDialog->Stream[i];
        Format [i]=InputStrDialog->Format[i];
        RcvOpt [i]=InputStrDialog->RcvOpt[i];
        for (j=0;j<4;j++) Paths[i][j]=InputStrDialog->Paths[i][j];
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        CmdEna[i][j]=InputStrDialog->CmdEna[i][j];
        Cmds  [i][j]=InputStrDialog->Cmds  [i][j];
    }
    for (i=0;i<10;i++) {
        History [i]=InputStrDialog->History [i];
        MntpHist[i]=InputStrDialog->MntpHist[i];
    }
    NmeaReq=InputStrDialog->NmeaReq;
    InTimeTag  =InputStrDialog->TimeTag;
    InTimeSpeed=InputStrDialog->TimeSpeed;
    InTimeStart=InputStrDialog->TimeStart;
    NmeaPos[0] =InputStrDialog->NmeaPos[0];
    NmeaPos[1] =InputStrDialog->NmeaPos[1];
}
// confirm overwrite --------------------------------------------------------
int __fastcall TMainForm::ConfOverwrite(const char *path)
{
    AnsiString s;
    FILE *fp;
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int i;
    char buff1[1024],buff2[1024],*p;
    
    trace(3,"ConfOverwrite\n");
    
    strcpy(buff1,path);
    
    if ((p=strstr(buff1,"::"))) *p='\0';
    
    if (!(fp=fopen(buff1,"r"))) return 1; // file not exists
    fclose(fp);
    
    // check overwrite input files
    for (i=0;i<3;i++) {
        if (!StreamC[i]||itype[Stream[i]]!=STR_FILE) continue;
        
        strcpy(buff2,Paths[i][2].c_str());
        if ((p=strstr(buff2,"::"))) *p='\0';
        
        if (!strcmp(buff1,buff2)) {
            Message->Caption=s.sprintf("invalid output %s",buff1);
            return 0;
        }
    }
    ConfDialog->Label2->Caption=buff1;
    
    return ConfDialog->ShowModal()==mrOk;
}
// callback on button-output-streams ----------------------------------------
void __fastcall TMainForm::BtnOutputStrClick(TObject *Sender)
{
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,j,str,update[2]={0};
    char *path;
    
    trace(3,"BtnOutputStrClick\n");
    
    for (i=3;i<5;i++) {
        OutputStrDialog->StreamC[i-3]=StreamC[i];
        OutputStrDialog->Stream [i-3]=Stream[i];
        OutputStrDialog->Format [i-3]=Format[i];
        for (j=0;j<4;j++) OutputStrDialog->Paths[i-3][j]=Paths[i][j];
    }
    for (i=0;i<10;i++) {
        OutputStrDialog->History [i]=History [i];
        OutputStrDialog->MntpHist[i]=MntpHist[i];
    }
    OutputStrDialog->OutTimeTag=OutTimeTag;
    OutputStrDialog->OutAppend =OutAppend;
    OutputStrDialog->SwapInterval=OutSwapInterval;
    
    if (OutputStrDialog->ShowModal()!=mrOk) return;
    
    for (i=3;i<5;i++) {
        if (StreamC[i]!=OutputStrDialog->StreamC[i-3]||
            Stream [i]!=OutputStrDialog->Stream[i-3]||
            Format [i]!=OutputStrDialog->Format[i-3]||
            Paths[i][0]!=OutputStrDialog->Paths[i-3][0]||
            Paths[i][1]!=OutputStrDialog->Paths[i-3][1]||
            Paths[i][2]!=OutputStrDialog->Paths[i-3][2]||
            Paths[i][3]!=OutputStrDialog->Paths[i-3][3]) update[i-3]=1;
        StreamC[i]=OutputStrDialog->StreamC[i-3];
        Stream [i]=OutputStrDialog->Stream[i-3];
        Format [i]=OutputStrDialog->Format[i-3];
        for (j=0;j<4;j++) Paths[i][j]=OutputStrDialog->Paths[i-3][j];
    }
    for (i=0;i<10;i++) {
        History [i]=OutputStrDialog->History [i];
        MntpHist[i]=OutputStrDialog->MntpHist[i];
    }
    OutTimeTag=OutputStrDialog->OutTimeTag;
    OutAppend =OutputStrDialog->OutAppend;
    OutSwapInterval=OutputStrDialog->SwapInterval;
    
    if (BtnStart->Enabled) return;
    
    for (i=3;i<5;i++) {
        if (!update[i-3]) continue;
        
        rtksvrclosestr(&rtksvr,i);
        
        if (!StreamC[i]) continue;
        
        str=otype[Stream[i]];
        if      (str==STR_SERIAL)             path=Paths[i][0].c_str();
        else if (str==STR_FILE  )             path=Paths[i][2].c_str();
        else if (str==STR_FTP||str==STR_HTTP) path=Paths[i][3].c_str();
        else                                  path=Paths[i][1].c_str();
        if (str==STR_FILE&&!ConfOverwrite(path)) {
            StreamC[i]=0;
            continue;
        }
        SolOpt.posf=Format[i];
        rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
    }
}
// callback on button-log-streams -------------------------------------------
void __fastcall TMainForm::BtnLogStrClick(TObject *Sender)
{
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,j,str,update[3]={0};
    char *path;
    
    trace(3,"BtnLogStrClick\n");
    
    for (i=5;i<8;i++) {
        LogStrDialog->StreamC[i-5]=StreamC[i];
        LogStrDialog->Stream [i-5]=Stream [i];
        for (j=0;j<4;j++) LogStrDialog->Paths[i-5][j]=Paths[i][j];
    }
    for (i=0;i<10;i++) {
        LogStrDialog->History [i]=History [i];
        LogStrDialog->MntpHist[i]=MntpHist[i];
    }
    LogStrDialog->LogTimeTag=LogTimeTag;
    LogStrDialog->LogAppend =LogAppend;
    LogStrDialog->SwapInterval=LogSwapInterval;
    
    if (LogStrDialog->ShowModal()!=mrOk) return;
    
    for (i=5;i<8;i++) {
        if (StreamC[i]!=OutputStrDialog->StreamC[i-5]||
            Stream [i]!=OutputStrDialog->Stream[i-5]||
            Paths[i][0]!=OutputStrDialog->Paths[i-3][0]||
            Paths[i][1]!=OutputStrDialog->Paths[i-3][1]||
            Paths[i][2]!=OutputStrDialog->Paths[i-3][2]||
            Paths[i][3]!=OutputStrDialog->Paths[i-3][3]) update[i-5]=1;
        StreamC[i]=LogStrDialog->StreamC[i-5];
        Stream [i]=LogStrDialog->Stream [i-5];
        for (j=0;j<4;j++) Paths[i][j]=LogStrDialog->Paths[i-5][j];
    }
    for (i=0;i<10;i++) {
        History [i]=LogStrDialog->History [i];
        MntpHist[i]=LogStrDialog->MntpHist[i];
    }
    LogTimeTag=LogStrDialog->LogTimeTag;
    LogAppend =LogStrDialog->LogAppend;
    LogSwapInterval=LogStrDialog->SwapInterval;
    
    if (BtnStart->Enabled) return;
    
    for (i=5;i<8;i++) {
        if (!update[i-5]) continue;
        
        rtksvrclosestr(&rtksvr,i);
        
        if (!StreamC[i]) continue;
        
        str=otype[Stream[i]];
        if      (str==STR_SERIAL)             path=Paths[i][0].c_str();
        else if (str==STR_FILE  )             path=Paths[i][2].c_str();
        else if (str==STR_FTP||str==STR_HTTP) path=Paths[i][3].c_str();
        else                                  path=Paths[i][1].c_str();
        if (str==STR_FILE&&!ConfOverwrite(path)) {
            StreamC[i]=0;
            continue;
        }
        rtksvropenstr(&rtksvr,i,str,path,&SolOpt);
    }
}
// callback on button-time-system -------------------------------------------
void __fastcall TMainForm::BtnTimeSysClick(TObject *Sender)
{
    trace(3,"BtnTimeSysClick\n");
    
    if (++TimeSys>3) TimeSys=0;
    UpdateTimeSys();
}
// callback on button-solution-type -----------------------------------------
void __fastcall TMainForm::BtnSolTypeClick(TObject *Sender)
{
    trace(3,"BtnSolTypeClick\n");
    
    if (++SolType>4) SolType=0;
    UpdateSolType();
}
// callback on button frequency-type ----------------------------------------
void __fastcall TMainForm::BtnFreqTypeClick(TObject *Sender)
{
    trace(3,"BtnFreqTypeClick\n");
    
    if (++FreqType>NFREQ) FreqType=0;
    UpdateSolType();
}
// callback on button-plottype ----------------------------------------------
void __fastcall TMainForm::BtnPlotTypeClick(TObject *Sender)
{
    trace(3,"BtnPlotTypeClick\n");
    
    ChangePlot();
}
// callback on button-rtk-monitor -------------------------------------------
void __fastcall TMainForm::BtnMonitorClick(TObject *Sender)
{
    TMonitorDialog *monitor=new TMonitorDialog(Application);
    
    trace(3,"BtnMonitorClick\n");
    
    monitor->Caption=Caption+": RTK Monitor";
    monitor->Show();
}
// callback on scroll-solution change ---------------------------------------
void __fastcall TMainForm::ScbSolChange(TObject *Sender)
{
    trace(3,"ScbSolChange\n");
    
    PSol=PSolS+ScbSol->Position;
    if (PSol>=SolBuffSize) PSol-=SolBuffSize;
    UpdateTime();
    UpdatePos();
}
// callback on button-save --------------------------------------------------
void __fastcall TMainForm::BtnSaveClick(TObject *Sender)
{
    trace(3,"BtnSaveClick\n");
    
    SaveLog();
}
// callback on button-about -------------------------------------------------
void __fastcall TMainForm::BtnAboutClick(TObject *Sender)
{
    AnsiString prog=PRGNAME;
    
    trace(3,"BtnAboutClick\n");
#ifdef MKL
    prog+="_MKL";
#endif
    AboutDialog->About=prog;
    AboutDialog->IconIndex=5;
    AboutDialog->ShowModal();
}
// callback on button-tasktray ----------------------------------------------
void __fastcall TMainForm::BtnTaskTrayClick(TObject *Sender)
{
    trace(3,"BtnTaskTrayClick\n");
    
    Visible=false;
    TrayIcon->Hint=Caption;
    TrayIcon->Visible=true;
}
// callback on button-tasktray ----------------------------------------------
void __fastcall TMainForm::TrayIconDblClick(TObject *Sender)
{
    trace(3,"TaskIconDblClick\n");
    
    Visible=true;
    TrayIcon->Visible=false;
}
// callback on task-tray-icon mouse down with right-button ------------------
void __fastcall TMainForm::TrayIconMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
   trace(3,"TaskIconMouseDown\n");
    
   if (Shift.Contains(ssRight)) PopupMenu->Popup(X,Y); 
}
// callback on menu-expand --------------------------------------------------
void __fastcall TMainForm::MenuExpandClick(TObject *Sender)
{
    trace(3,"MenuExpandClick\n");
    
    Visible=true;
    TrayIcon->Visible=false;
}
// callback on menu-start ---------------------------------------------------
void __fastcall TMainForm::MenuStartClick(TObject *Sender)
{
    trace(3,"MenuStartClick\n");
    
    BtnStartClick(Sender);
}
// callback on menu-stop ----------------------------------------------------
void __fastcall TMainForm::MenuStopClick(TObject *Sender)
{
    trace(3,"MenuStopClick\n");
    
    BtnStopClick(Sender);
}
// callback on menu-monitor -------------------------------------------------
void __fastcall TMainForm::MenuMonitorClick(TObject *Sender)
{
    trace(3,"MenuMonitorClick\n");
    
    BtnMonitorClick(Sender);
}
// callback on menu-plot ----------------------------------------------------
void __fastcall TMainForm::MenuPlotClick(TObject *Sender)
{
    trace(3,"MenuPlotClick\n");
    
    BtnPlotClick(Sender);
}
// callback on menu-exit ----------------------------------------------------
void __fastcall TMainForm::MenuExitClick(TObject *Sender)
{
    trace(3,"MenuExitClick\n");
    
    BtnExitClick(Sender);
}
// start rtk server ---------------------------------------------------------
void __fastcall TMainForm::SvrStart(void)
{
    AnsiString s;
    solopt_t solopt[2];
    double ep[6],pos[3],nmeapos[3];
    int itype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPCLI,STR_FILE,STR_FTP,STR_HTTP};
    int otype[]={STR_SERIAL,STR_TCPCLI,STR_TCPSVR,STR_NTRIPSVR,STR_FILE};
    int i,strs[MAXSTRRTK]={0},sat,ex,stropt[8]={0};
    char *paths[8],*cmds[3]={0},*rcvopts[3]={0},buff[1024],*p;
    char file[1024],*type;
    FILE *fp;
    gtime_t time=timeget();
    pcvs_t pcvr={0},pcvs={0};
    pcv_t *pcv;
    
    trace(3,"SvrStart\n");
    
    Message->Caption="";
    
    if (RovPosTypeF<=2) {
        PrcOpt.rovpos=0;
        PrcOpt.ru[0]=RovPos[0];
        PrcOpt.ru[1]=RovPos[1];
        PrcOpt.ru[2]=RovPos[2];
    }
    else {
        PrcOpt.rovpos=4;
        for (i=0;i<3;i++) PrcOpt.ru[i]=0.0;
    }
    if (RefPosTypeF<=2) {
        PrcOpt.refpos=0;
        PrcOpt.rb[0]=RefPos[0];
        PrcOpt.rb[1]=RefPos[1];
        PrcOpt.rb[2]=RefPos[2];
    }
    else {
        PrcOpt.refpos=4;
        for (i=0;i<3;i++) PrcOpt.rb[i]=0.0;
    }
    for (i=0;i<MAXSAT;i++) {
        PrcOpt.exsats[i]=0;
    }
    if (ExSats!="") { // excluded satellites
        strcpy(buff,ExSats.c_str());
        for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
            if (*p=='+') {ex=2; p++;} else ex=1;
            if (!(sat=satid2no(p))) continue;
            PrcOpt.exsats[sat-1]=ex;
        }
    }
    if ((RovAntPcvF||RefAntPcvF)&&!readpcv(AntPcvFileF.c_str(),&pcvr)) {
        Message->Caption=s.sprintf("rcv ant file read error %s",AntPcvFileF.c_str());
        return;
    }
    if (RovAntPcvF) {
        type=RovAntF.c_str();
        if ((pcv=searchpcv(0,type,time,&pcvr))) {
            PrcOpt.pcvr[0]=*pcv;
        }
        else Message->Caption=s.sprintf("no antenna pcv %s",type);
        
        for (i=0;i<3;i++) PrcOpt.antdel[0][i]=RovAntDel[i];
    }
    if (RefAntPcvF) {
        type=RefAntF.c_str();
        if ((pcv=searchpcv(0,type,time,&pcvr))) {
            PrcOpt.pcvr[1]=*pcv;
        }
        else Message->Caption=s.sprintf("no antenna pcv %s",type);
        
        for (i=0;i<3;i++) PrcOpt.antdel[1][i]=RefAntDel[i];
    }
    if (RovAntPcvF||RefAntPcvF) {
        free(pcvr.pcv);
    }
    if (PrcOpt.sateph==EPHOPT_PREC||PrcOpt.sateph==EPHOPT_SSRCOM) {
        if (!readpcv(SatPcvFileF.c_str(),&pcvs)) {
            Message->Caption=s.sprintf("sat ant file read error %s",SatPcvFileF.c_str());
            return;
        }
        for (i=0;i<MAXSAT;i++) {
            if (!(pcv=searchpcv(i+1,"",time,&pcvs))) continue;
            rtksvr.nav.pcvs[i]=*pcv;
        }
        free(pcvs.pcv);
    }
    if (BaselineC) {
        PrcOpt.baseline[0]=Baseline[0];
        PrcOpt.baseline[1]=Baseline[1];
    }
    else {
        PrcOpt.baseline[0]=0.0;
        PrcOpt.baseline[1]=0.0;
    }
    for (i=0;i<3;i++) strs[i]=StreamC[i]?itype[Stream[i]]:STR_NONE;
    for (i=3;i<5;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
    for (i=5;i<8;i++) strs[i]=StreamC[i]?otype[Stream[i]]:STR_NONE;
    for (i=0;i<8;i++) {
        if      (strs[i]==STR_NONE  ) paths[i]="";
        else if (strs[i]==STR_SERIAL) paths[i]=Paths[i][0].c_str();
        else if (strs[i]==STR_FILE  ) paths[i]=Paths[i][2].c_str();
        else if (strs[i]==STR_FTP||strs[i]==STR_HTTP) paths[i]=Paths[i][3].c_str();
        else paths[i]=Paths[i][1].c_str();
    }
    for (i=0;i<3;i++) {
        if (CmdEna[i][0]) cmds[i]=Cmds[i][0].c_str();
        rcvopts[i]=RcvOpt[i].c_str();
    }
    NmeaCycle=NmeaCycle<1000?1000:NmeaCycle;
    pos[0]=NmeaPos[0]*D2R;
    pos[1]=NmeaPos[1]*D2R;
    pos[2]=0.0;
    pos2ecef(pos,nmeapos);
    
    strsetdir(LocalDirectory.c_str());
    strsetproxy(ProxyAddr.c_str());
    
    for (i=3;i<8;i++) {
        if (strs[i]==STR_FILE&&!ConfOverwrite(paths[i])) return;
    }
    time2epoch(utc2gpst(timeget()),ep);
    if (DebugTraceF>0) {
        sprintf(file,"rtknavi_%04.0f%02.0f%02.0f%02.0f%02.0f%02.0f.trace",
                ep[0],ep[1],ep[2],ep[3],ep[4],ep[5]);
        traceopen(file);
        tracelevel(DebugTraceF);
    }
    if (DebugStatusF>0) {
        sprintf(file,"rtknavi_%04.0f%02.0f%02.0f%02.0f%02.0f%02.0f.stat",
                ep[0],ep[1],ep[2],ep[3],ep[4],ep[5]);
        rtkopenstat(file,DebugStatusF);
    }
    if (SolOpt.geoid>0&&GeoidDataFileF!="") {
        opengeoid(SolOpt.geoid,GeoidDataFileF.c_str());
    }
    if (DCBFileF!="") {
        readdcb(DCBFileF.c_str(),&rtksvr.nav);
    }
    for (i=0;i<2;i++) {
        solopt[i]=SolOpt;
        solopt[i].posf=Format[i+3];
    }
    stropt[0]=TimeoutTime;
    stropt[1]=ReconTime;
    stropt[2]=1000;
    stropt[3]=SvrBuffSize;
    stropt[4]=FileSwapMargin;
    strsetopt(stropt);
    
    // start rtk server
    if (!rtksvrstart(&rtksvr,SvrCycle,SvrBuffSize,strs,paths,Format,NavSelect,
                     cmds,rcvopts,NmeaCycle,NmeaReq,nmeapos,&PrcOpt,solopt,
                     &monistr)) {
        traceclose();
        return;
    }
    PSol=PSolS=PSolE=0;
    SolStat[0]=Nvsat[0]=0;
    for (i=0;i<3;i++) SolRov[i]=SolRef[i]=VelRov[i]=0.0;
    for (i=0;i<9;i++) Qr[i]=0.0;
    Age[0]=Ratio[0]=0.0;
    Nsat[0]=Nsat[1]=0;
    UpdatePos();
    UpdatePlot();
    BtnStart    ->Enabled=false;
    BtnOpt      ->Enabled=false;
    BtnExit     ->Enabled=false;
    BtnInputStr ->Enabled=false;
    MenuStart   ->Enabled=false;
    MenuExit    ->Enabled=false;
    ScbSol      ->Enabled=false;
    BtnStop     ->Enabled=true;
    MenuStop    ->Enabled=true;
    Svr->Color=CLORANGE;
    SetTrayIcon(0);
}
// strop rtk server ---------------------------------------------------------
void __fastcall TMainForm::SvrStop(void)
{
    char *cmds[3]={0};
    int i,n,m;
    
    trace(3,"SvrStop\n");
    
    for (i=0;i<3;i++) {
        if (CmdEna[i][1]) cmds[i]=Cmds[i][1].c_str();
    }
    rtksvrstop(&rtksvr,cmds);
    
    BtnStart    ->Enabled=true;
    BtnOpt      ->Enabled=true;
    BtnExit     ->Enabled=true;
    BtnInputStr ->Enabled=true;
    MenuStart   ->Enabled=true;
    MenuExit    ->Enabled=true;
    ScbSol      ->Enabled=true;
    BtnStop     ->Enabled=false;
    MenuStop    ->Enabled=false;
    Svr->Color=clBtnFace;
    SetTrayIcon(1);
    
    LabelTime->Font->Color=clGray;
    IndSol->Color=clWhite;
    n=PSolE-PSolS; if (n<0) n+=SolBuffSize;
    m=PSol-PSolS;  if (m<0) m+=SolBuffSize;
    if (n>0) {
        ScbSol->Max=n-1; ScbSol->Position=m;
    }
    Message->Caption="";
    
    if (DebugTraceF>0) traceclose();
    if (DebugStatusF>0) rtkclosestat();
    if (OutputGeoidF>0&&GeoidDataFileF!="") closegeoid();
}
// callback on interval timer -----------------------------------------------
void __fastcall TMainForm::TimerTimer(TObject *Sender)
{
    static int n=0,inactive=0;
    sol_t *sol;
    int i,update=0;
    unsigned char buff[8];
    
    trace(4,"TimerTimer\n");
    
    rtksvrlock(&rtksvr);
    
    for (i=0;i<rtksvr.nsol;i++) {
        sol=rtksvr.solbuf+i;
        UpdateLog(sol->stat,sol->time,sol->rr,sol->qr,rtksvr.rtk.rb,sol->ns,
                  sol->age,sol->ratio);
        update=1;
    }
    rtksvr.nsol=0;
    SolCurrentStat=rtksvr.state?rtksvr.rtk.sol.stat:0;
    
    rtksvrunlock(&rtksvr);
    
    if (update) {
        UpdateTime();
        UpdatePos();
        inactive=0;
    }
    else {
        if (++inactive*Timer->Interval>TIMEOUT) SolCurrentStat=0;
    }
    if (SolCurrentStat) {
        Svr->Color=clLime;
        LabelTime->Font->Color=clBlack;
    }
    else {
        IndSol->Color=clWhite;
        Solution->Font->Color=clGray;
        Svr->Color=rtksvr.state?clGreen:clBtnFace;
    }
    if (!(++n%5)) UpdatePlot();
    UpdateStr();
    
    // keep alive for monitor port
    if (!(++n%(KACYCLE/Timer->Interval))&&OpenPort) {
        strwrite(&monistr,"\r",1);
    }
}
// change plot type ---------------------------------------------------------
void __fastcall TMainForm::ChangePlot(void)
{
    trace(3,"ChangePlot\n");
    
    if (++PlotType>6) PlotType=0;
    Panel21->Visible=PlotType<=4;
    IndQ->Visible=!Panel21->Visible;
    BtnSolType2->Visible=!Panel21->Visible;
    UpdatePlot();
    UpdatePos();
}
// update time-system -------------------------------------------------------
void __fastcall TMainForm::UpdateTimeSys(void)
{
    AnsiString label[]={"GPST","UTC","JST","GPST"};
    
    trace(3,"UpdateTimeSys\n");
    
    BtnTimeSys->Caption=label[TimeSys];
    UpdateTime();
}
// update solution type -----------------------------------------------------
void __fastcall TMainForm::UpdateSolType(void)
{
    AnsiString label[]={
        "Lat/Lon/Height","Lat/Lon/Height","X/Y/Z-ECEF","E/N/U-Baseline",
        "Pitch/Yaw/Length-Baseline",""
    };
    trace(3,"UpdateSolType\n");
    
    Plabel0->Caption=label[SolType];
    UpdatePos();
}
// update log ---------------------------------------------------------------
void __fastcall TMainForm::UpdateLog(int stat, gtime_t time, double *rr,
    float *qr, double *rb, int ns, double age, double ratio)
{
    int i,ena;
    
    if (!stat) return;
    
    trace(4,"UpdateLog\n");
    
    SolStat[PSolE]=stat; Time[PSolE]=time; Nvsat[PSolE]=ns; Age[PSolE]=age;
    Ratio[PSolE]=ratio;
    for (i=0;i<3;i++) {
        SolRov[i+PSolE*3]=rr[i];
        SolRef[i+PSolE*3]=rb[i];
        VelRov[i+PSolE*3]=rr[i+3];
    }
    Qr[  PSolE*9]=qr[0];
    Qr[4+PSolE*9]=qr[1];
    Qr[8+PSolE*9]=qr[2];
    Qr[1+PSolE*9]=Qr[3+PSolE*9]=qr[3];
    Qr[5+PSolE*9]=Qr[7+PSolE*9]=qr[4];
    Qr[2+PSolE*9]=Qr[6+PSolE*9]=qr[5];
    
    PSol=PSolE;
    if (++PSolE>=SolBuffSize) PSolE=0;
    if (PSolE==PSolS&&++PSolS>=SolBuffSize) PSolS=0;
}
// update font --------------------------------------------------------------
void __fastcall TMainForm::UpdateFont(void)
{
    TLabel *label[]={
        PlabelA,Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3,Solution,LabelStd,LabelNSat
    };
    TColor color=label[7]->Font->Color;
    int i;
    
    trace(4,"UpdateFont\n");
    
    for (i=0;i<10;i++) label[i]->Font->Assign(PosFont);
    label[0]->Font->Size=9; label[7]->Font->Color=color;
    label[8]->Font->Size=8; label[8]->Font->Color=clGray;
    label[9]->Font->Size=8; label[9]->Font->Color=clGray;
}
// update time --------------------------------------------------------------
void __fastcall TMainForm::UpdateTime(void)
{
    gtime_t time=Time[PSol];
    double tow;
    int week;
    char tstr[64];
    
    trace(4,"UpdateTime\n");
    
    if      (TimeSys==0) time2str(time,tstr,1);
    else if (TimeSys==1) time2str(gpst2utc(time),tstr,1);
    else if (TimeSys==2) time2str(timeadd(gpst2utc(time),9*3600.0),tstr,1);
    else if (TimeSys==3) {
        tow=time2gpst(time,&week); sprintf(tstr,"week %04d %8.1f s",week,tow);
    }
    LabelTime->Caption=tstr;
}
// update solution display --------------------------------------------------
void __fastcall TMainForm::UpdatePos(void)
{
    TLabel *label[]={Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3,LabelStd,LabelNSat};
    AnsiString sol[]={"----","FIX","FLOAT","SBAS","DGPS","SINGLE","PPP"},s[8];
    TColor color[]={clSilver,clGreen,CLORANGE,clFuchsia,clBlue,clRed,clTeal};
    gtime_t time;
    double *rr=SolRov+PSol*3,*rb=SolRef+PSol*3,*qr=Qr+PSol*9,pos[3]={0},Qe[9]={0};
    double dms1[3]={0},dms2[3]={0},bl[3]={0},enu[3]={0},pitch=0.0,yaw=0.0,len;
    int i,stat=SolStat[PSol];
    
    trace(4,"UpdatePos\n");
    
    Solution->Caption=sol[stat];
    Solution->Font->Color=rtksvr.state?color[stat]:clGray;
    IndSol->Color=rtksvr.state&&stat?color[stat]:clWhite;
    if (norm(rr,3)>0.0&&norm(rb,3)>0.0) {
        for (i=0;i<3;i++) bl[i]=rr[i]-rb[i];
    }
    len=norm(bl,3);
    if (SolType==0) {
        if (norm(rr,3)>0.0) {
            ecef2pos(rr,pos); covenu(pos,qr,Qe);
            degtodms(pos[0]*R2D,dms1);
            degtodms(pos[1]*R2D,dms2);
            if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
        }
        s[0]=pos[0]<0?"S:":"N:"; s[1]=pos[1]<0?"W:":"E:"; s[2]="H:";
        s[3].sprintf("%.0f%s %02.0f' %07.4f\"",fabs(dms1[0]),CHARDEG,dms1[1],dms1[2]);
        s[4].sprintf("%.0f%s %02.0f' %07.4f\"",fabs(dms2[0]),CHARDEG,dms2[1],dms2[2]);
        s[5].sprintf("%.3f m",pos[2]);
        s[6].sprintf("N:%6.3f E:%6.3f U:%6.3f m",SQRT(Qe[4]),SQRT(Qe[0]),SQRT(Qe[8]));
    }
    else if (SolType==1) {
        if (norm(rr,3)>0.0) {
            ecef2pos(rr,pos); covenu(pos,qr,Qe);
            if (SolOpt.height==1) pos[2]-=geoidh(pos); /* geodetic */
        }
        s[0]=pos[0]<0?"S:":"N:"; s[1]=pos[1]<0?"W:":"E:"; s[2]="H:";
        s[3].sprintf("%.8f %s",fabs(pos[0])*R2D,CHARDEG);
        s[4].sprintf("%.8f %s",fabs(pos[1])*R2D,CHARDEG);
        s[5].sprintf("%.3f m",pos[2]);
        s[6].sprintf("E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
    }
    else if (SolType==2) {
        s[0]="X:"; s[1]="Y:"; s[2]="Z:";
        s[3].sprintf("%.3f m",rr[0]);
        s[4].sprintf("%.3f m",rr[1]);
        s[5].sprintf("%.3f m",rr[2]);
        s[6].sprintf("X:%6.3f Y:%6.3f Z:%6.3f m",SQRT(qr[0]),SQRT(qr[4]),SQRT(qr[8]));
    }
    else if (SolType==3) {
        if (len>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
        }
        s[0]="E:"; s[1]="N:"; s[2]="U:";
        s[3].sprintf("%.3f m",enu[0]);
        s[4].sprintf("%.3f m",enu[1]);
        s[5].sprintf("%.3f m",enu[2]);
        s[6].sprintf("E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
    }
    else {
        if (len>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu); covenu(pos,qr,Qe);
            pitch=asin(enu[2]/len);
            yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
        }
        s[0]="P:"; s[1]="Y:"; s[2]="L:";
        s[3].sprintf("%.3f %s",pitch*R2D,CHARDEG);
        s[4].sprintf("%.3f %s",yaw*R2D,CHARDEG);
        s[5].sprintf("%.3f m",len);
        s[6].sprintf("E:%6.3f N:%6.3f U:%6.3f m",SQRT(Qe[0]),SQRT(Qe[4]),SQRT(Qe[8]));
    }
    s[7].sprintf("Age:%4.1f s Ratio:%4.1f # of Sat:%2d",Age[PSol],Ratio[PSol],Nvsat[PSol]);
    
    for (i=0;i<8;i++) label[i]->Caption=s[i];
    for (i=3;i<6;i++) {
        label[i]->Font->Color=PrcOpt.mode==PMODE_MOVEB&&SolType<=2?clGray:clBlack;
    }
}
// update stream status indicators ------------------------------------------
void __fastcall TMainForm::UpdateStr(void)
{
    TColor color[]={clRed,clBtnFace,CLORANGE,clGreen,clLime};
    TPanel *ind[MAXSTRRTK]={Str1,Str2,Str3,Str4,Str5,Str6,Str7,Str8};
    int i,sstat[MAXSTRRTK]={0};
    char msg[MAXSTRMSG]="";
    
    trace(4,"UpdateStr\n");
    
    rtksvrsstat(&rtksvr,sstat,msg);
    for (i=0;i<MAXSTRRTK;i++) {
        ind[i]->Color=color[sstat[i]+1];
        if (sstat[i]) Message->Caption=msg;
    }
}
// update solution plot -----------------------------------------------------
void __fastcall TMainForm::UpdatePlot(void)
{
    AnsiString s;
    gtime_t time;
    TCanvas *c=Plot->Canvas;
    TLabel *label[]={Plabel1,Plabel2,Plabel3,Pos1,Pos2,Pos3};
    char *fstr[]={"","L1 ","L2 ","L5 ","L6 ","L7 ","L8 "};
    int w=PlotWidth,h=PlotHeight;
    int i,j,sat[2][MAXSAT],ns[2],snr[2][MAXSAT][NFREQ],vsat[2][MAXSAT];
    int *snr0[MAXSAT],*snr1[MAXSAT];
    double az[2][MAXSAT],el[2][MAXSAT];
    
    trace(4,"UpdatePlot\n");
    
    for (i=0;i<MAXSAT;i++) {
        snr0[i]=snr[0][i];
        snr1[i]=snr[1][i];
    }
    ns[0]=rtksvrostat(&rtksvr,0,&time,sat[0],az[0],el[0],snr0,vsat[0]);
    ns[1]=rtksvrostat(&rtksvr,1,&time,sat[1],az[1],el[1],snr1,vsat[1]);
     
    for (i=0;i<2;i++) {
        if (ns[i]>0) {
            Nsat[i]=ns[i];
            for (int j=0;j<ns[i];j++) {
                Sat [i][j]=sat [i][j];
                Az  [i][j]=az  [i][j];
                El  [i][j]=el  [i][j];
                for (int k=0;k<NFREQ;k++) {
                    Snr[i][j][k]=snr[i][j][k];
                }
                Vsat[i][j]=vsat[i][j];
            }
        }
        else {
            for (j=0;j<Nsat[i];j++) {
                Vsat[i][j]=0;
                for (int k=0;k<NFREQ;k++) {
                    Snr[i][j][k]=0;
                }
            }
        }
    }
    c->Brush->Style=bsSolid;
    c->Brush->Color=clWhite;
    c->FillRect(Plot->ClientRect);
    if (PlotType==0) {
        DrawSnr(c,w,(h-12)/2,15,0);
        DrawSnr(c,w,(h-12)/2,14+(h-12)/2,1);
        s.sprintf("Rover:Base %sSNR (dBHz)",fstr[FreqType]);
        DrawText(c,3,1,s,clGray,0);
    }
    else if (PlotType==1) {
        DrawSnr(c,w,h-15,15,0);
        s.sprintf("Rover %s SNR (dBHz)",fstr[FreqType]);
        DrawText(c,3,1,s,clGray,0);
    }
    else if (PlotType==2) {
        DrawSat(c,w,h,0);
        s.sprintf("Rover %s",fstr[!FreqType?1:FreqType]);
        DrawText(c,3,1,s,clGray,0);
    }
    else if (PlotType==3) {
        DrawSat(c,w,h,1);
        s.sprintf("Base %s",fstr[!FreqType?1:FreqType]);
        DrawText(c,3,1,s,clGray,0);
    }
    else if (PlotType==4) {
        DrawBL(c,w,h);
        DrawText(c,3,1,"Baseline",clGray,0);
    }
    else if (PlotType==5) {
        DrawSnr(c,w,(h-12)/2,15,0);
        DrawSnr(c,w,(h-12)/2,14+(h-12)/2,1);
    }
    else if (PlotType==6) {
        DrawSnr(c,w,h-15,15,0);
    }
    if (PlotType>=5) {
        IndQ->Color=IndSol->Color;
        s=label[0]->Caption+" "+label[3]->Caption+" "+
          label[1]->Caption+" "+label[4]->Caption+" "+
          label[2]->Caption+" "+label[5]->Caption;
        DrawText(c,19,1,s,LabelTime->Font->Color,0);
        s.sprintf("%s",fstr[FreqType]);
        DrawText(c,6,18,s,clGray,0);
    }
    Disp->Canvas->CopyRect(Disp->ClientRect,c,Disp->ClientRect);
}
// draw snr plot ------------------------------------------------------------
void __fastcall TMainForm::DrawSnr(TCanvas *c, int w, int h, int top, int index)
{
    TColor color[]={clGreen,CLORANGE,clFuchsia,clBlue,clRed,clGray};
    AnsiString s; 
    int i,j,k,x1,x2,y1,y2,y3,k1,hh=h-15,ww,www,snr[NFREQ+1];
    char id[16];
    
    trace(4,"DrawSnr: w=%d h=%d top=%d index=%d\n",w,h,top,index);
    
    c->Pen->Color=clSilver;
    for (snr[0]=MINSNR+10;snr[0]<MAXSNR;snr[0]+=10) {
        y1=top+hh-(snr[0]-MINSNR)*hh/(MAXSNR-MINSNR);
        c->MoveTo(3,y1); c->LineTo(w-13,y1);
        DrawText(c,w-9,y1,s.sprintf("%d",snr[0]),clGray,1);
    }
    y1=top+hh;
    TRect b(1,top,w-2,y1);
    c->Pen->Color=clGray;
    c->Brush->Style=bsClear;
    c->Rectangle(b);
    
    for (i=0;i<Nsat[index]&&i<MAXSAT;i++) {
        
        ww=(w-16)/Nsat[index];
        www=ww-2<8?ww-2:8;
        x1=i*(w-16)/Nsat[index]+ww/2;
        
        for (j=snr[0]=0;j<NFREQ;j++) {
            snr[j+1]=Snr[index][i][j];
            if ((FreqType&&FreqType==j+1)||(!FreqType&&snr[j+1]>snr[0])) {
                snr[0]=snr[j+1];
            }
        }
        for (j=0;j<NFREQ+2;j++) {
            k=j<NFREQ+1?j:0;
            y3=j<NFREQ+1?0:2;
            y2=y1-y3;
            if (snr[k]>0) y2-=(snr[k]-MINSNR)*hh/(MAXSNR-MINSNR)-y3;
            y2=y2<2?2:(y1<y2?y1:y2);
            
            TRect r1(x1,y1,x1+www,y2);
            if (j==0) {
                k1=(49-snr[k])/5;
                c->Brush->Style=bsSolid;
                c->Brush->Color=color[k1<0?0:(k1>5?5:k1)];
                if (!Vsat[index][i]) c->Brush->Color=clSilver;
                c->Rectangle(r1);
            }
            else {
                c->Pen->Color=j<NFREQ+1?clSilver:clGray;
                c->Brush->Style=bsClear;
                c->Rectangle(r1);
            }
        }
        satno2id(Sat[index][i],id);
        DrawText(c,x1+www/2,y1+6,(s=id),Vsat[index][i]?clGray:clSilver,1);
    }
}
// draw satellites in skyplot -----------------------------------------------
void __fastcall TMainForm::DrawSat(TCanvas *c, int w, int h, int index)
{
    TColor color[]={clGreen,CLORANGE,clFuchsia,clBlue,clRed,clGray};
    AnsiString s;
    TPoint p(w/2,h/2);
    double r=w*0.85/2,azel[MAXSAT*2],dop[4];
    int i,j,k,d,x[MAXSAT],y[MAXSAT],ns=0,f=!FreqType?0:FreqType-1;
    char id[16];
    
    trace(4,"DrawSat: w=%d h=%d index=%d\n",w,h,index);
    
    DrawSky(c,w,h);
    
    for (i=0,k=Nsat[index]-1;i<Nsat[index]&&i<MAXSAT;i++,k--) {
        if (El[index][k]<=0.0) continue;
        if (Vsat[index][k]) {
            azel[ns*2]=Az[index][k]; azel[1+ns*2]=El[index][k];
            ns++;
        }
        x[i]=(int)(p.x+r*(90-El[index][k]*R2D)/90*sin(Az[index][k]));
        y[i]=(int)(p.y-r*(90-El[index][k]*R2D)/90*cos(Az[index][k]));
        c->Pen->Color=clGray;
        c->Brush->Style=bsSolid;
        j=(49-Snr[index][k][f])/5;
        d=SATSIZE/2;
        c->Brush->Color=Vsat[index][k]?color[j<0?0:(j>5?5:j)]:clSilver;
        c->Ellipse(x[i]-d,y[i]-d,x[i]+d+1,y[i]+d+1);
        c->Brush->Style=bsClear;
        satno2id(Sat[index][k],id);
        DrawText(c,x[i],y[i],(s=id),clWhite,1);
    }
    c->Brush->Style=bsClear;
    dops(ns,azel,0.0,dop);
    DrawText(c,3,h-15,s.sprintf("# of Sat:%2d",Nsat[index]),clGray,0);
    DrawText(c,w-3,h-15,s.sprintf("GDOP:%.1f",dop[0]),clGray,2);
}
// draw baseline plot -------------------------------------------------------
void __fastcall TMainForm::DrawBL(TCanvas *c, int w, int h)
{
    TColor color[]={clSilver,clGreen,CLORANGE,clFuchsia,clBlue,clRed,clTeal};
    AnsiString s,label[]={"N","E","S","W"};
    TPoint p(w/2,h/2),p1,p2,pp;
    double r=w*0.85/2;
    double *rr=SolRov+PSol*3,*rb=SolRef+PSol*3;
    double bl[3]={0},pos[3],enu[3],len=0.0,pitch=0.0,yaw=0.0;
    double cp,q;
    TColor col=clWhite;
    int i,d1=9,d2=16,d3=10,cy=0,sy=0,cya=0,sya=0,a,x1,x2,y1,y2,digit;
    
    trace(4,"DrawBL: w=%d h=%d\n",w,h);
    
    if (PMODE_DGPS<=PrcOpt.mode&&PrcOpt.mode<=PMODE_FIXED) {
        col=rtksvr.state&&SolStat[PSol]&SolCurrentStat?color[SolStat[PSol]]:clWhite;
        
        if (norm(rr,3)>0.0&&norm(rb,3)>0.0) {
            for (i=0;i<3;i++) bl[i]=rr[i]-rb[i];
        }
        if ((len=norm(bl,3))>0.0) {
            ecef2pos(rb,pos); ecef2enu(pos,bl,enu);
            pitch=asin(enu[2]/len);
            yaw=atan2(enu[0],enu[1]); if (yaw<0.0) yaw+=2.0*PI;
        }
    }
    if (len>=MINBLLEN) {
        cp =cos(pitch);
        cy =(int)((r-d1-d2/2)*cp*cos(yaw));
        sy =(int)((r-d1-d2/2)*cp*sin(yaw));
        cya=(int)(((r-d1-d2/2)*cp-d2/2-4)*cos(yaw));
        sya=(int)(((r-d1-d2/2)*cp-d2/2-4)*sin(yaw));
    }
    p1.x=p.x-sy; p1.y=p.y+cy; // base
    p2.x=p.x+sy; p2.y=p.y-cy; // rover
    
    c->Pen->Color=clGray;
    c->Ellipse(p.x-r,p.y-r,p.x+r+1,p.y+r+1);
    c->Pen->Color=clSilver;
    c->Ellipse(p.x-r+d1,p.y-r+d1,p.x+r-d1+1,p.y+r-d1+1);
    c->Brush->Style=bsSolid;
    
    pp=pitch<0.0?p2:p1;
    c->Pen->Color=clSilver;
    c->MoveTo(p.x,p.y); c->LineTo(pp.x,pp.y);
    if (pitch<0.0) {
        c->Brush->Color=clWhite;
        c->Ellipse(pp.x-d2/2,pp.y-d2/2,pp.x+d2/2+1,pp.y+d2/2+1);
        DrawArrow(c,p.x+sya,p.y-cya,d3,(int)(yaw*R2D),clSilver);
    }
    c->Brush->Color=col;
    c->Ellipse(pp.x-d2/2+2,pp.y-d2/2+2,pp.x+d2/2-1,pp.y+d2/2-1);
    c->Brush->Color=clWhite;
    for (a=0;a<360;a+=15) {
        q=a%45==0?r/2:r-d1;
        x1=(int)(r*sin(a*D2R));
        y1=(int)(r*cos(a*D2R));
        x2=(int)(q*sin(a*D2R));
        y2=(int)(q*cos(a*D2R));
        c->Pen->Color=clSilver;
        c->MoveTo(p.x+x1,p.y+y1); c->LineTo(p.x+x2,p.y+y2);
        if (a%90==0) DrawText(c,p.x+x1,p.y-y1,label[a/90],clGray,1);
    }
    pp=pitch>=0.0?p2:p1;
    c->Pen->Color=clGray;
    c->MoveTo(p.x,p.y); c->LineTo(pp.x,pp.y);
    if (pitch>=0.0) {
        c->Brush->Color=clWhite;
        c->Ellipse(pp.x-d2/2,pp.y-d2/2,pp.x+d2/2+1,pp.y+d2/2+1);
        DrawArrow(c,p.x+sya,p.y-cya,d3,(int)(yaw*R2D),clGray);
    }
    c->Brush->Color=col;
    c->Ellipse(pp.x-d2/2+2,pp.y-d2/2+2,pp.x+d2/2-1,pp.y+d2/2-1);
    c->Brush->Color=clWhite;
    digit=len<10.0?3:(len<100.0?2:(len<1000.0?1:0));
    DrawText(c,p.x,p.y ,s.sprintf("%.*f m",digit,len),clGray,1);
    DrawText(c,3,  h-15,s.sprintf("Y: %.1f%s",yaw*R2D,CHARDEG),clGray,0);
    DrawText(c,w-3,h-15,s.sprintf("P: %.1f%s",pitch*R2D,CHARDEG),clGray,2);
}
// draw skyplot -------------------------------------------------------------
void __fastcall TMainForm::DrawSky(TCanvas *c, int w, int h)
{
    TPoint p(w/2,h/2);
    double r=w*0.85/2;
    int a,e,d,x,y;
    
    c->Brush->Color=clWhite;
    c->Brush->Style=bsSolid;
    for (e=0;e<90;e+=30) {
        d=(int)(r*(90-e)/90);
        c->Pen->Color=e==0?clGray:clSilver;
        c->Ellipse(p.x-d,p.y-d,p.x+d+1,p.y+d+1);
    }
    AnsiString label[]={"N","E","S","W"};
    for (a=0;a<360;a+=45) {
        x=(int)(r*sin(a*D2R));
        y=(int)(r*cos(a*D2R));
        c->Pen->Color=clSilver;
        c->MoveTo(p.x,p.y); c->LineTo(p.x+x,p.y-y);
        if (a%90==0) DrawText(c,p.x+x,p.y-y,label[a/90],clGray,1);
    }
}
// draw text ----------------------------------------------------------------
void __fastcall TMainForm::DrawText(TCanvas *c, int x, int y, AnsiString s,
    TColor color, int align)
{
    TSize off=c->TextExtent(s);
    c->Font->Charset=ANSI_CHARSET;
    if (align==1) {x-=off.cx/2; y-=off.cy/2;} else if (align==2) x-=off.cx;
    c->Font->Color=color;
    c->TextOut(x,y,s);
}
// draw arrow ---------------------------------------------------------------
void __fastcall TMainForm::DrawArrow(TCanvas *c, int x, int y, int siz,
    int ang, TColor color)
{
    TPoint p1[4],p2[4];
    int i;
    
    p1[0].x=0; p1[1].x=siz/2; p1[2].x=-siz/2; p1[3].x=0;
    p1[0].y=siz/2; p1[1].y=p1[2].y=-siz/2; p1[3].y=siz/2;
    
    for (i=0;i<4;i++) {
        p2[i].x=x+(int)(p1[i].x*cos(-ang*D2R)-p1[i].y*sin(-ang*D2R)+0.5);
        p2[i].y=y-(int)(p1[i].x*sin(-ang*D2R)+p1[i].y*cos(-ang*D2R)+0.5);
    }
    c->Brush->Style=bsSolid;
    c->Brush->Color=color;
    c->Pen->Color=color;
    c->Polygon(p2,3);
}
// open monitor port --------------------------------------------------------
void __fastcall TMainForm::OpenMoniPort(int port)
{
    AnsiString s;
    int i;
    char path[64];
    
    if (port<=0) return;
    
    trace(3,"OpenMoniPort: port=%d\n",port);
    
    for (i=0;i<=MAXPORTOFF;i++) {
        
        sprintf(path,":%d",port+i);
        
        if (stropen(&monistr,STR_TCPSVR,STR_MODE_RW,path)) {
            strsettimeout(&monistr,TimeoutTime,ReconTime);
            if (i>0) Caption=s.sprintf("%s ver.%s (%d)",PRGNAME,VER_RTKLIB,i+1);
            OpenPort=MoniPort+i;
            return;
        }
    }
    ShowMessage(s.sprintf("monitor port %d-%d open error",port,port+MAXPORTOFF));
    OpenPort=0;
}
// initialize solution buffer -----------------------------------------------
void __fastcall TMainForm::InitSolBuff(void)
{
    double ep[]={2000,1,1,0,0,0};
    int i,j;
    
    trace(3,"InitSolBuff\n");
    
    delete [] Time;   delete [] SolStat; delete [] Nvsat;  delete [] SolRov;
    delete [] SolRef; delete [] Qr;      delete [] VelRov; delete [] Age;
    delete [] Ratio;
    
    if (SolBuffSize<=0) SolBuffSize=1;
    Time   =new gtime_t[SolBuffSize];
    SolStat=new int[SolBuffSize];
    Nvsat  =new int[SolBuffSize];
    SolRov =new double[SolBuffSize*3];
    SolRef =new double[SolBuffSize*3];
    VelRov =new double[SolBuffSize*3];
    Qr     =new double[SolBuffSize*9];
    Age    =new double[SolBuffSize];
    Ratio  =new double[SolBuffSize];
    PSol=PSolS=PSolE=0;
    for (i=0;i<SolBuffSize;i++) {
        Time[i]=epoch2time(ep);
        SolStat[i]=Nvsat[i]=0;
        for (j=0;j<3;j++) SolRov[j+i*3]=SolRef[j+i*3]=VelRov[j+i*3]=0.0;
        for (j=0;j<9;j++) Qr[j+i*9]=0.0;
        Age[i]=Ratio[i]=0.0;
    }
    ScbSol->Max=0; ScbSol->Position=0;
}
// save log file ------------------------------------------------------------
void __fastcall TMainForm::SaveLog(void)
{
    FILE *fp;
    int posf[]={SOLF_LLH,SOLF_LLH,SOLF_XYZ,SOLF_ENU,SOLF_ENU,SOLF_LLH};
    solopt_t opt;
    double  ep[6],pos[3];
    char file[1024];
    
    trace(3,"SaveLog\n");
    
    time2epoch(timeget(),ep);
    sprintf(file,"rtk_%04.0f%02.0f%02.0f%02.0f%02.0f%02.0f.txt",
            ep[0],ep[1],ep[2],ep[3],ep[4],ep[5]);
    SaveDialog->FileName=file;
    if (!SaveDialog->Execute()) return;
    if (!(fp=fopen(SaveDialog->FileName.c_str(),"wt"))) {
        Message->Caption="log file open error"; return;
    }
    opt=SolOpt;
    opt.posf=posf[SolType];
    if (SolOpt.outhead) {
        fprintf(fp,"%% program   : %s ver.%s\n",PRGNAME,VER_RTKLIB);
        if (PrcOpt.mode==PMODE_DGPS||PrcOpt.mode==PMODE_KINEMA||
            PrcOpt.mode==PMODE_STATIC) {
            ecef2pos(PrcOpt.rb,pos);
            fprintf(fp,"%% ref pos   :%13.9f %14.9f %10.4f\n",pos[0]*R2D,
                    pos[1]*R2D,pos[2]);
        }
        fprintf(fp,"%%\n");
    }
    outsolhead(fp,&opt);
    fclose(fp);
}
// load navigation data -----------------------------------------------------
void __fastcall TMainForm::LoadNav(nav_t *nav)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString str,s;
    eph_t eph0={0};
    char buff[2049],id[32],*p;
    int i;
    
    trace(3,"LoadNav\n");
    
    for (i=0;i<MAXSAT;i++) {
        if ((str=ini->ReadString("navi",s.sprintf("eph_%02d",i),""))=="") continue;
        nav->eph[i]=eph0;
        strcpy(buff,str.c_str());
        if (!(p=strchr(buff,','))) continue;
        *p='\0';
        if (!(nav->eph[i].sat=satid2no(buff))) continue;
        sscanf(p+1,"%d,%d,%d,%d,%ld,%ld,%ld,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%d,%d",
               &nav->eph[i].iode,
               &nav->eph[i].iodc,
               &nav->eph[i].sva ,
               &nav->eph[i].svh ,
               &nav->eph[i].toe.time,
               &nav->eph[i].toc.time,
               &nav->eph[i].ttr.time,
               &nav->eph[i].A   ,
               &nav->eph[i].e   ,
               &nav->eph[i].i0  ,
               &nav->eph[i].OMG0,
               &nav->eph[i].omg ,
               &nav->eph[i].M0  ,
               &nav->eph[i].deln,
               &nav->eph[i].OMGd,
               &nav->eph[i].idot,
               &nav->eph[i].crc ,
               &nav->eph[i].crs ,
               &nav->eph[i].cuc ,
               &nav->eph[i].cus ,
               &nav->eph[i].cic ,
               &nav->eph[i].cis ,
               &nav->eph[i].toes,
               &nav->eph[i].fit ,
               &nav->eph[i].f0  ,
               &nav->eph[i].f1  ,
               &nav->eph[i].f2  ,
               &nav->eph[i].tgd[0],
               &nav->eph[i].code,
               &nav->eph[i].flag);
    }
    str=ini->ReadString("navi","ion","");
    for (i=0;i<8;i++) nav->ion_gps[i]=0.0;
    sscanf(str.c_str(),"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
           nav->ion_gps  ,nav->ion_gps+1,nav->ion_gps+2,nav->ion_gps+3,
           nav->ion_gps+4,nav->ion_gps+5,nav->ion_gps+6,nav->ion_gps+7);
    str=ini->ReadString("navi","utc","");
    
    for (i=0;i<4;i++) nav->utc_gps[i]=0.0;
    sscanf(str.c_str(),"%lf,%lf,%lf,%lf",
           nav->utc_gps,nav->utc_gps+1,nav->utc_gps+2,nav->utc_gps+3);
    
    nav->leaps=ini->ReadInteger("navi","leaps",0);
    
    delete ini;
}
// save navigation data -----------------------------------------------------
void __fastcall TMainForm::SaveNav(nav_t *nav)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString str,s;
    char id[32];
    int i;
    
    trace(3,"SaveNav\n");
    
    for (i=0;i<MAXSAT;i++) {
        if (nav->eph[i].ttr.time==0) continue;
        str="";
        satno2id(nav->eph[i].sat,id);
        str=str+s.sprintf("%s,",id);
        str=str+s.sprintf("%d,",nav->eph[i].iode);
        str=str+s.sprintf("%d,",nav->eph[i].iodc);
        str=str+s.sprintf("%d,",nav->eph[i].sva);
        str=str+s.sprintf("%d,",nav->eph[i].svh);
        str=str+s.sprintf("%d,",(int)nav->eph[i].toe.time);
        str=str+s.sprintf("%d,",(int)nav->eph[i].toc.time);
        str=str+s.sprintf("%d,",(int)nav->eph[i].ttr.time);
        str=str+s.sprintf("%.14E,",nav->eph[i].A);
        str=str+s.sprintf("%.14E,",nav->eph[i].e);
        str=str+s.sprintf("%.14E,",nav->eph[i].i0);
        str=str+s.sprintf("%.14E,",nav->eph[i].OMG0);
        str=str+s.sprintf("%.14E,",nav->eph[i].omg);
        str=str+s.sprintf("%.14E,",nav->eph[i].M0);
        str=str+s.sprintf("%.14E,",nav->eph[i].deln);
        str=str+s.sprintf("%.14E,",nav->eph[i].OMGd);
        str=str+s.sprintf("%.14E,",nav->eph[i].idot);
        str=str+s.sprintf("%.14E,",nav->eph[i].crc);
        str=str+s.sprintf("%.14E,",nav->eph[i].crs);
        str=str+s.sprintf("%.14E,",nav->eph[i].cuc);
        str=str+s.sprintf("%.14E,",nav->eph[i].cus);
        str=str+s.sprintf("%.14E,",nav->eph[i].cic);
        str=str+s.sprintf("%.14E,",nav->eph[i].cis);
        str=str+s.sprintf("%.14E,",nav->eph[i].toes);
        str=str+s.sprintf("%.14E,",nav->eph[i].fit);
        str=str+s.sprintf("%.14E,",nav->eph[i].f0);
        str=str+s.sprintf("%.14E,",nav->eph[i].f1);
        str=str+s.sprintf("%.14E,",nav->eph[i].f2);
        str=str+s.sprintf("%.14E,",nav->eph[i].tgd[0]);
        str=str+s.sprintf("%d,",nav->eph[i].code);
        str=str+s.sprintf("%d,",nav->eph[i].flag);
        ini->WriteString("navi",s.sprintf("eph_%02d",i),str);
    }
    str="";
    for (i=0;i<8;i++) str=str+s.sprintf("%.14E,",nav->ion_gps[i]);
    ini->WriteString("navi","ion",str);
    
    str="";
    for (i=0;i<4;i++) str=str+s.sprintf("%.14E,",nav->utc_gps[i]);
    ini->WriteString("navi","utc",str);
    
    ini->WriteInteger("navi","leaps",nav->leaps);
    
    delete ini;
}
// set tray icon ------------------------------------------------------------
void __fastcall TMainForm::SetTrayIcon(int index)
{
    TIcon *icon=new TIcon;
    ImageList->GetIcon(index,icon);
    TrayIcon->Icon=icon;
    delete icon;
}
// load option from ini file ------------------------------------------------
void __fastcall TMainForm::LoadOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s;
    int i,j,no,strno[]={0,1,6,2,3,4,5,7};
    char *p;
    
    trace(3,"LoadOpt\n");
    
    for (i=0;i<8;i++) {
        no=strno[i];
        StreamC[i]=ini->ReadInteger("stream",s.sprintf("streamc%d",no),0);
        Stream [i]=ini->ReadInteger("stream",s.sprintf("stream%d", no),0);
        Format [i]=ini->ReadInteger("stream",s.sprintf("format%d", no),0);
        for (j=0;j<4;j++) {
            Paths[i][j]=ini->ReadString("stream",s.sprintf("path_%d_%d",no,j),"");
        }
    }
    for (i=0;i<3;i++) {
        RcvOpt [i]=ini->ReadString("stream",s.sprintf("rcvopt%d",i+1),"");
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        Cmds[i][j]=ini->ReadString("serial",s.sprintf("cmd_%d_%d",i,j),"");
        CmdEna[i][j]=ini->ReadInteger("serial",s.sprintf("cmdena_%d_%d",i,j),0);
        for (p=Cmds[i][j].c_str();*p;p++) {
            if ((p=strstr(p,"@@"))) strncpy(p,"\r\n",2); else break;
        }
    }
    PrcOpt.mode     =ini->ReadInteger("prcopt", "mode",            0);
    PrcOpt.nf       =ini->ReadInteger("prcopt", "nf",              2);
    PrcOpt.elmin    =ini->ReadFloat  ("prcopt", "elmin",    15.0*D2R);
    PrcOpt.snrmin   =ini->ReadFloat  ("prcopt", "snrmin",        0.0);
    PrcOpt.dynamics =ini->ReadInteger("prcopt", "dynamics",        0);
    PrcOpt.tidecorr =ini->ReadInteger("prcopt", "tidecorr",        0);
    PrcOpt.modear   =ini->ReadInteger("prcopt", "modear",          1);
    PrcOpt.glomodear=ini->ReadInteger("prcopt", "glomodear",       0);
    PrcOpt.maxout   =ini->ReadInteger("prcopt", "maxout",          5);
    PrcOpt.minlock  =ini->ReadInteger("prcopt", "minlock",         0);
    PrcOpt.minfix   =ini->ReadInteger("prcopt", "minfix",         10);
    PrcOpt.ionoopt  =ini->ReadInteger("prcopt", "ionoopt",IONOOPT_BRDC);
    PrcOpt.tropopt  =ini->ReadInteger("prcopt", "tropopt",TROPOPT_SAAS);
    PrcOpt.sateph   =ini->ReadInteger("prcopt", "ephopt",  EPHOPT_BRDC);
    PrcOpt.niter    =ini->ReadInteger("prcopt", "niter",           1);
    PrcOpt.eratio[0]=ini->ReadFloat  ("prcopt", "eratio0",     100.0);
    PrcOpt.eratio[1]=ini->ReadFloat  ("prcopt", "eratio1",     100.0);
    PrcOpt.err[1]   =ini->ReadFloat  ("prcopt", "err1",        0.003);
    PrcOpt.err[2]   =ini->ReadFloat  ("prcopt", "err2",        0.003);
    PrcOpt.err[3]   =ini->ReadFloat  ("prcopt", "err3",          0.0);
    PrcOpt.err[4]   =ini->ReadFloat  ("prcopt", "err4",          1.0);
    PrcOpt.prn[0]   =ini->ReadFloat  ("prcopt", "prn0",         1E-4);
    PrcOpt.prn[1]   =ini->ReadFloat  ("prcopt", "prn1",         1E-3);
    PrcOpt.prn[2]   =ini->ReadFloat  ("prcopt", "prn2",         1E-4);
    PrcOpt.prn[3]   =ini->ReadFloat  ("prcopt", "prn3",         10.0);
    PrcOpt.prn[4]   =ini->ReadFloat  ("prcopt", "prn4",         10.0);
    PrcOpt.sclkstab =ini->ReadFloat  ("prcopt", "sclkstab",    5E-12);
    PrcOpt.thresar  =ini->ReadFloat  ("prcopt", "thresar",       3.0);
    PrcOpt.elmaskar =ini->ReadFloat  ("prcopt", "elmaskar",      0.0);
    PrcOpt.elmaskhold=ini->ReadFloat ("prcopt", "elmaskhold",    0.0);
    PrcOpt.thresslip=ini->ReadFloat  ("prcopt", "thresslip",    0.05);
    PrcOpt.maxtdiff =ini->ReadFloat  ("prcopt", "maxtdiff",     30.0);
    PrcOpt.maxgdop  =ini->ReadFloat  ("prcopt", "maxgdop",      30.0);
    PrcOpt.maxinno  =ini->ReadFloat  ("prcopt", "maxinno",      30.0);
    ExSats          =ini->ReadString ("prcopt", "exsats",         "");
    PrcOpt.navsys   =ini->ReadInteger("prcopt", "navsys",    SYS_GPS);
    
    BaselineC       =ini->ReadInteger("prcopt", "baselinec",       0);
    Baseline[0]     =ini->ReadFloat  ("prcopt", "baseline1",     0.0);
    Baseline[1]     =ini->ReadFloat  ("prcopt", "baseline2",     0.0);
    
    SolOpt.posf     =ini->ReadInteger("solopt", "posf",            0);
    SolOpt.times    =ini->ReadInteger("solopt", "times",           0);
    SolOpt.timef    =ini->ReadInteger("solopt", "timef",           1);
    SolOpt.timeu    =ini->ReadInteger("solopt", "timeu",           3);
    SolOpt.degf     =ini->ReadInteger("solopt", "degf",            0);
    s=ini->ReadString("solopt","sep"," ");
    strcpy(SolOpt.sep,s.c_str());
    SolOpt.outhead  =ini->ReadInteger("solopt", "outhead",         0);
    SolOpt.outopt   =ini->ReadInteger("solopt", "outopt",          0);
    SolOpt.datum    =ini->ReadInteger("solopt", "datum",           0);
    SolOpt.height   =ini->ReadInteger("solopt", "height",          0);
    SolOpt.geoid    =ini->ReadInteger("solopt", "geoid",           0);
    SolOpt.nmeaintv[0]=ini->ReadFloat("solopt", "nmeaintv1",     0.0);
    SolOpt.nmeaintv[1]=ini->ReadFloat("solopt", "nmeaintv2",     0.0);
    DebugStatusF    =ini->ReadInteger("setting","debugstatus",     0);
    DebugTraceF     =ini->ReadInteger("setting","debugtrace",      0);
    
    RovPosTypeF     =ini->ReadInteger("setting","rovpostype",      0);
    RefPosTypeF     =ini->ReadInteger("setting","refpostype",      0);
    RovAntPcvF      =ini->ReadInteger("setting","rovantpcv",       0);
    RefAntPcvF      =ini->ReadInteger("setting","refantpcv",       0);
    RovAntF         =ini->ReadString ("setting","rovant",         "");
    RefAntF         =ini->ReadString ("setting","refant",         "");
    SatPcvFileF     =ini->ReadString ("setting","satpcvfile",     "");
    AntPcvFileF     =ini->ReadString ("setting","antpcvfile",     "");
    StaPosFileF     =ini->ReadString ("setting","staposfile",     "");
    GeoidDataFileF  =ini->ReadString ("setting","geoiddatafile",  "");
    DCBFileF        =ini->ReadString ("setting","dcbfile",        "");
    LocalDirectory  =ini->ReadString ("setting","localdirectory","C:\\Temp");
    
    SvrCycle        =ini->ReadInteger("setting","svrcycle",       10);
    TimeoutTime     =ini->ReadInteger("setting","timeouttime", 10000);
    ReconTime       =ini->ReadInteger("setting","recontime",   10000);
    NmeaCycle       =ini->ReadInteger("setting","nmeacycle",    5000);
    SvrBuffSize     =ini->ReadInteger("setting","svrbuffsize", 32768);
    SolBuffSize     =ini->ReadInteger("setting","solbuffsize",  1000);
    SavedSol        =ini->ReadInteger("setting","savedsol",      100);
    NavSelect       =ini->ReadInteger("setting","navselect",       0);
    PrcOpt.sbassatsel=ini->ReadInteger("setting","sbassat",        0);
    DgpsCorr        =ini->ReadInteger("setting","dgpscorr",        0);
    SbasCorr        =ini->ReadInteger("setting","sbascorr",        0);
    
    NmeaReq         =ini->ReadInteger("setting","nmeareq",         0);
    InTimeTag       =ini->ReadInteger("setting","intimetag",       0);
    InTimeSpeed     =ini->ReadString ("setting","intimespeed",  "x1");
    InTimeStart     =ini->ReadString ("setting","intimestart",   "0");
    OutTimeTag      =ini->ReadInteger("setting","outtimetag",      0);
    OutAppend       =ini->ReadInteger("setting","outappend",       0);
    OutSwapInterval =ini->ReadString ("setting","outswapinterval","");
    LogTimeTag      =ini->ReadInteger("setting","logtimetag",      0);
    LogAppend       =ini->ReadInteger("setting","logappend",       0);
    LogSwapInterval =ini->ReadString ("setting","logswapinterval","");
    NmeaPos[0]      =ini->ReadFloat  ("setting","nmeapos1",      0.0);
    NmeaPos[1]      =ini->ReadFloat  ("setting","nmeapos2",      0.0);
    FileSwapMargin  =ini->ReadInteger("setting","fswapmargin",    30);
    
    TimeSys         =ini->ReadInteger("setting","timesys",         0);
    SolType         =ini->ReadInteger("setting","soltype",         0);
    PlotType        =ini->ReadInteger("setting","plottype",        0);
    ProxyAddr       =ini->ReadString ("setting","proxyaddr",      "");
    MoniPort        =ini->ReadInteger("setting","moniport",DEFAULTPORT);
    
    for (i=0;i<3;i++) {
        RovAntDel[i]=ini->ReadFloat("setting",s.sprintf("rovantdel_%d",i),0.0);
        RefAntDel[i]=ini->ReadFloat("setting",s.sprintf("refantdel_%d",i),0.0);
        RovPos   [i]=ini->ReadFloat("setting",s.sprintf("rovpos_%d",   i),0.0);
        RefPos   [i]=ini->ReadFloat("setting",s.sprintf("refpos_%d",   i),0.0);
    }
    for (i=0;i<10;i++) {
        History[i]=ini->ReadString ("tcpopt",s.sprintf("history%d", i),"");
    }
    for (i=0;i<10;i++) {
        MntpHist[i]=ini->ReadString("tcpopt",s.sprintf("mntphist%d",i),"");
    }
    NMapPnt        =ini->ReadInteger("mapopt","nmappnt",0);
    for (i=0;i<NMapPnt;i++) {
        PntName[i]=ini->ReadString("mapopt",s.sprintf("pntname%d",i+1),"");
        AnsiString pos=ini->ReadString("mapopt",s.sprintf("pntpos%d",i+1),"0,0,0");
        PntPos[i][0]=PntPos[i][1]=PntPos[i][2]=0.0;
        sscanf(pos.c_str(),"%lf,%lf,%lf",PntPos[i],PntPos[i]+1,PntPos[i]+2);
    }
    PosFont->Charset=ANSI_CHARSET;
    PosFont->Name=ini->ReadString ("setting","posfontname",POSFONTNAME);
    PosFont->Size=ini->ReadInteger("setting","posfontsize",POSFONTSIZE);
    PosFont->Color=(TColor)ini->ReadInteger("setting","posfontcolor",(int)clBlack);
    if (ini->ReadInteger("setting","posfontbold",  0)) PosFont->Style=PosFont->Style<<fsBold;
    if (ini->ReadInteger("setting","posfontitalic",0)) PosFont->Style=PosFont->Style<<fsItalic;
    
    TTextViewer::Color1=(TColor)ini->ReadInteger("viewer","color1",(int)clBlack);
    TTextViewer::Color2=(TColor)ini->ReadInteger("viewer","color2",(int)clWhite);
    TTextViewer::FontD=new TFont;
    TTextViewer::FontD->Name=ini->ReadString ("viewer","fontname","Courier New");
    TTextViewer::FontD->Size=ini->ReadInteger("viewer","fontsize",9);
    
    delete ini;
}
// save option to ini file --------------------------------------------------
void __fastcall TMainForm::SaveOpt(void)
{
    TIniFile *ini=new TIniFile(IniFile);
    AnsiString s;
    int i,j,no,strno[]={0,1,6,2,3,4,5,7};
    char *p;
    
    trace(3,"SaveOpt\n");
    
    for (i=0;i<8;i++) {
        no=strno[i];
        ini->WriteInteger("stream",s.sprintf("streamc%d",no),StreamC[i]);
        ini->WriteInteger("stream",s.sprintf("stream%d" ,no),Stream [i]);
        ini->WriteInteger("stream",s.sprintf("format%d" ,no),Format [i]);
        for (j=0;j<4;j++) {
            ini->WriteString("stream",s.sprintf("path_%d_%d",no,j),Paths[i][j]);
        }
    }
    for (i=0;i<3;i++) {
        ini->WriteString("stream",s.sprintf("rcvopt%d",i+1),RcvOpt[i]);
    }
    for (i=0;i<3;i++) for (j=0;j<2;j++) {
        for (p=Cmds[i][j].c_str();*p;p++) {
            if ((p=strstr(p,"\r\n"))) strncpy(p,"@@",2); else break;
        }
        ini->WriteString ("serial",s.sprintf("cmd_%d_%d"   ,i,j),Cmds  [i][j]);
        ini->WriteInteger("serial",s.sprintf("cmdena_%d_%d",i,j),CmdEna[i][j]);
    }
    ini->WriteInteger("prcopt", "mode",       PrcOpt.mode        );
    ini->WriteInteger("prcopt", "nf",         PrcOpt.nf          );
    ini->WriteFloat  ("prcopt", "elmin",      PrcOpt.elmin       );
    ini->WriteFloat  ("prcopt", "snrmin",     PrcOpt.snrmin      );
    ini->WriteInteger("prcopt", "dynamics",   PrcOpt.dynamics    );
    ini->WriteInteger("prcopt", "tidecorr",   PrcOpt.tidecorr    );
    ini->WriteInteger("prcopt", "modear",     PrcOpt.modear      );
    ini->WriteInteger("prcopt", "glomodear",  PrcOpt.glomodear   );
    ini->WriteInteger("prcopt", "maxout",     PrcOpt.maxout      );
    ini->WriteInteger("prcopt", "minlock",    PrcOpt.minlock     );
    ini->WriteInteger("prcopt", "minfix",     PrcOpt.minfix      );
    ini->WriteInteger("prcopt", "ionoopt",    PrcOpt.ionoopt     );
    ini->WriteInteger("prcopt", "tropopt",    PrcOpt.tropopt     );
    ini->WriteInteger("prcopt", "ephopt",     PrcOpt.sateph      );
    ini->WriteInteger("prcopt", "niter",      PrcOpt.niter       );
    ini->WriteFloat  ("prcopt", "eratio0",    PrcOpt.eratio[0]   );
    ini->WriteFloat  ("prcopt", "eratio1",    PrcOpt.eratio[1]   );
    ini->WriteFloat  ("prcopt", "err1",       PrcOpt.err[1]      );
    ini->WriteFloat  ("prcopt", "err2",       PrcOpt.err[2]      );
    ini->WriteFloat  ("prcopt", "err3",       PrcOpt.err[3]      );
    ini->WriteFloat  ("prcopt", "err4",       PrcOpt.err[4]      );
    ini->WriteFloat  ("prcopt", "prn0",       PrcOpt.prn[0]      );
    ini->WriteFloat  ("prcopt", "prn1",       PrcOpt.prn[1]      );
    ini->WriteFloat  ("prcopt", "prn2",       PrcOpt.prn[2]      );
    ini->WriteFloat  ("prcopt", "prn3",       PrcOpt.prn[3]      );
    ini->WriteFloat  ("prcopt", "prn4",       PrcOpt.prn[4]      );
    ini->WriteFloat  ("prcopt", "sclkstab",   PrcOpt.sclkstab    );
    ini->WriteFloat  ("prcopt", "thresar",    PrcOpt.thresar     );
    ini->WriteFloat  ("prcopt", "elmaskar",   PrcOpt.elmaskar    );
    ini->WriteFloat  ("prcopt", "elmaskhold", PrcOpt.elmaskhold  );
    ini->WriteFloat  ("prcopt", "thresslip",  PrcOpt.thresslip   );
    ini->WriteFloat  ("prcopt", "maxtdiff",   PrcOpt.maxtdiff    );
    ini->WriteFloat  ("prcopt", "maxgdop",    PrcOpt.maxgdop     );
    ini->WriteFloat  ("prcopt", "maxinno",    PrcOpt.maxinno     );
    ini->WriteString ("prcopt", "exsats",     ExSats             );
    ini->WriteInteger("prcopt", "navsys",     PrcOpt.navsys      );
    
    ini->WriteFloat  ("prcopt", "baselinec",  BaselineC          );
    ini->WriteFloat  ("prcopt", "baseline1",  Baseline[0]        );
    ini->WriteFloat  ("prcopt", "baseline2",  Baseline[1]        );
    
    ini->WriteInteger("solopt", "posf",       SolOpt.posf        );
    ini->WriteInteger("solopt", "times",      SolOpt.times       );
    ini->WriteInteger("solopt", "timef",      SolOpt.timef       );
    ini->WriteInteger("solopt", "timeu",      SolOpt.timeu       );
    ini->WriteInteger("solopt", "degf",       SolOpt.degf        );
    ini->WriteString ("solopt", "sep",        SolOpt.sep         );
    ini->WriteInteger("solopt", "outhead",    SolOpt.outhead     );
    ini->WriteInteger("solopt", "outopt",     SolOpt.outopt      );
    ini->WriteInteger("solopt", "datum",      SolOpt.datum       );
    ini->WriteInteger("solopt", "height",     SolOpt.height      );
    ini->WriteInteger("solopt", "geoid",      SolOpt.geoid       );
    ini->WriteFloat  ("solopt", "nmeaintv1",  SolOpt.nmeaintv[0] );
    ini->WriteFloat  ("solopt", "nmeaintv2",  SolOpt.nmeaintv[1] );
    ini->WriteInteger("setting","debugstatus",DebugStatusF       );
    ini->WriteInteger("setting","debugtrace", DebugTraceF        );
    
    ini->WriteInteger("setting","rovpostype", RovPosTypeF        );
    ini->WriteInteger("setting","refpostype", RefPosTypeF        );
    ini->WriteInteger("setting","rovantpcv",  RovAntPcvF         );
    ini->WriteInteger("setting","refantpcv",  RefAntPcvF         );
    ini->WriteString ("setting","rovant",     RovAntF            );
    ini->WriteString ("setting","refant",     RefAntF            );
    ini->WriteString ("setting","satpcvfile", SatPcvFileF        );
    ini->WriteString ("setting","antpcvfile", AntPcvFileF        );
    ini->WriteString ("setting","staposfile", StaPosFileF        );
    ini->WriteString ("setting","geoiddatafile",GeoidDataFileF   );
    ini->WriteString ("setting","dcbfile",    DCBFileF           );
    ini->WriteString ("setting","localdirectory",LocalDirectory  );
    
    ini->WriteInteger("setting","svrcycle",   SvrCycle           );
    ini->WriteInteger("setting","timeouttime",TimeoutTime        );
    ini->WriteInteger("setting","recontime",  ReconTime          );
    ini->WriteInteger("setting","nmeacycle",  NmeaCycle          );
    ini->WriteInteger("setting","svrbuffsize",SvrBuffSize        );
    ini->WriteInteger("setting","solbuffsize",SolBuffSize        );
    ini->WriteInteger("setting","savedsol",   SavedSol           );
    ini->WriteInteger("setting","navselect",  NavSelect          );
    ini->WriteInteger("setting","sbassat",    PrcOpt.sbassatsel  );
    ini->WriteInteger("setting","dgpscorr",   DgpsCorr           );
    ini->WriteInteger("setting","sbascorr",   SbasCorr           );
    
    ini->WriteInteger("setting","nmeareq",    NmeaReq            );
    ini->WriteInteger("setting","intimetag",  InTimeTag          );
    ini->WriteString ("setting","intimespeed",InTimeSpeed        );
    ini->WriteString ("setting","intimestart",InTimeStart        );
    ini->WriteInteger("setting","outtimetag", OutTimeTag         );
    ini->WriteInteger("setting","outappend",  OutAppend          );
    ini->WriteString ("setting","outswapinterval",OutSwapInterval);
    ini->WriteInteger("setting","logtimetag", LogTimeTag         );
    ini->WriteInteger("setting","logappend",  LogAppend          );
    ini->WriteString ("setting","logswapinterval",LogSwapInterval);
    ini->WriteFloat  ("setting","nmeapos1",   NmeaPos[0]         );
    ini->WriteFloat  ("setting","nmeapos2",   NmeaPos[1]         );
    ini->WriteInteger("setting","fswapmargin",FileSwapMargin     );
    
    ini->WriteInteger("setting","timesys",    TimeSys            );
    ini->WriteInteger("setting","soltype",    SolType            );
    ini->WriteInteger("setting","plottype",   PlotType           );
    ini->WriteString ("setting","proxyaddr",  ProxyAddr          );
    ini->WriteInteger("setting","moniport",   MoniPort           );
    
    for (i=0;i<3;i++) {
        ini->WriteFloat("setting",s.sprintf("rovantdel_%d",i),RovAntDel[i]);
        ini->WriteFloat("setting",s.sprintf("refantdel_%d",i),RefAntDel[i]);
        ini->WriteFloat("setting",s.sprintf("rovpos_%d",i),   RovPos[i]);
        ini->WriteFloat("setting",s.sprintf("refpos_%d",i),   RefPos[i]);
    }
    for (i=0;i<10;i++) {
        ini->WriteString("tcpopt",s.sprintf("history%d" ,i),History [i]);
    }
    for (i=0;i<10;i++) {
        ini->WriteString("tcpopt",s.sprintf("mntphist%d",i),MntpHist[i]);
    }
    ini->WriteInteger("mapopt","nmappnt",NMapPnt);
    for (i=0;i<NMapPnt;i++) {
        AnsiString s1,s2;
        ini->WriteString("mapopt",s1.sprintf("pntname%d",i+1),PntName[i]);
        ini->WriteString("mapopt",s1.sprintf("pntpos%d" ,i+1),
            s2.sprintf("%.4f,%.4f,%.4f",PntPos[i][0],PntPos[i][1],PntPos[i][2]));
    }
    ini->WriteString ("setting","posfontname", PosFont->Name    );
    ini->WriteInteger("setting","posfontsize", PosFont->Size    );
    ini->WriteInteger("setting","posfontcolor",(int)PosFont->Color);
    ini->WriteInteger("setting","posfontbold",  PosFont->Style.Contains(fsBold));
    ini->WriteInteger("setting","posfontitalic",PosFont->Style.Contains(fsItalic));
    
    ini->WriteInteger("viewer","color1",  (int)TTextViewer::Color1);
    ini->WriteInteger("viewer","color2",  (int)TTextViewer::Color2);
    ini->WriteString ("viewer","fontname",TTextViewer::FontD->Name);
    ini->WriteInteger("viewer","fontsize",TTextViewer::FontD->Size);
    
    delete ini;
}
//---------------------------------------------------------------------------

