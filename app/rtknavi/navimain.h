//---------------------------------------------------------------------------
#ifndef navimainH
#define navimainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Graphics.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <ImgList.hpp>
#include <Menus.hpp>

#include "rtklib.h"

#define MAXSCALE	18
#define MAXMAPPNT	10

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TPanel *Panel1;
	TPanel *Panel2;
	TPanel *Panel21;
	TPanel *Panel211;
	TPanel *Panel22;
	TPanel *Panel3;
	TPanel *Panel4;
	TPanel *Panel5;
	TPanel *Panel6;
	TPanel *Panel7;
	
	TPanel *IndSol;
	TPanel *Svr;
	TPanel *Str1;
	TPanel *Str2;
	TPanel *Str4;
	TPanel *Str3;
	TPanel *Str5;
	TPanel *Str7;
	TPanel *Str8;
	TPanel *Str6;
	
	TLabel *LabelTime;
	TLabel *Solution;
	TLabel *Pos1;
	TLabel *Pos2;
	TLabel *Pos3;
	TLabel *LabelNSat;
	TLabel *LabelStd;
	TLabel *Message;
	TLabel *Plabel0;
	TLabel *Plabel1;
	TLabel *Plabel2;
	TLabel *Plabel3;
	TLabel *PlabelA;
	
	TButton *BtnPlot;
	TButton *BtnStart;
	TButton *BtnStop;
	TButton *BtnOpt;
	TButton *BtnSave;
	TButton *BtnExit;
	
	TPopupMenu *PopupMenu;
	TMenuItem *MenuMonitor;
	TMenuItem *MenuExpand;
	TMenuItem *N1;
	TMenuItem *MenuStart;
	TMenuItem *MenuPlot;
	TMenuItem *N2;
	TMenuItem *MenuExit;
	TMenuItem *MenuStop;
	
	TSpeedButton *BtnTimeSys;
	TSpeedButton *BtnMonitor;
	TSpeedButton *BtnAbout;
	TSpeedButton *BtnInputStr;
	TSpeedButton *BtnOutputStr;
	TSpeedButton *BtnLogStr;
	
	TSpeedButton *BtnTaskTray;
	
	TScrollBar *ScbSol;
	
	TTrayIcon *TrayIcon;
	
	TSaveDialog *SaveDialog;
	
	TImageList *ImageList;
	TImage *Image1;
	TImage *Image2;
	
	TTimer *Timer;
	TPanel *IndQ;
	TSpeedButton *BtnPlotType;
	TSpeedButton *BtnFreqType;
	TSpeedButton *BtnSolType;
	TSpeedButton *BtnSolType2;
	TImage *Plot;
	TImage *Disp;
	
	void __fastcall FormCreate        (TObject *Sender);
	void __fastcall FormShow          (TObject *Sender);
	void __fastcall FormClose         (TObject *Sender, TCloseAction &Action);
	
	void __fastcall TimerTimer        (TObject *Sender);
	
	void __fastcall BtnStartClick     (TObject *Sender);
	void __fastcall BtnStopClick      (TObject *Sender);
	void __fastcall BtnPlotClick      (TObject *Sender);
	void __fastcall BtnOptClick       (TObject *Sender);
	void __fastcall BtnExitClick      (TObject *Sender);
	
	void __fastcall BtnTimeSysClick   (TObject *Sender);
	void __fastcall BtnInputStrClick  (TObject *Sender);
	void __fastcall BtnOutputStrClick (TObject *Sender);
	void __fastcall BtnLogStrClick    (TObject *Sender);
	void __fastcall BtnSolTypeClick   (TObject *Sender);
	void __fastcall BtnPlotTypeClick  (TObject *Sender);
	
	void __fastcall BtnMonitorClick   (TObject *Sender);
	void __fastcall BtnSaveClick      (TObject *Sender);
	void __fastcall BtnAboutClick     (TObject *Sender);
	void __fastcall BtnTaskTrayClick  (TObject *Sender);
	
	void __fastcall MenuExpandClick   (TObject *Sender);
	void __fastcall MenuStartClick    (TObject *Sender);
	void __fastcall MenuStopClick     (TObject *Sender);
	void __fastcall MenuPlotClick     (TObject *Sender);
	void __fastcall MenuMonitorClick  (TObject *Sender);
	void __fastcall MenuExitClick     (TObject *Sender);
	
	void __fastcall ScbSolChange      (TObject *Sender);
	
	void __fastcall TrayIconDblClick  (TObject *Sender);
	void __fastcall TrayIconMouseDown (TObject *Sender, TMouseButton Button,
							           TShiftState Shift, int X, int Y);
	void __fastcall BtnFreqTypeClick(TObject *Sender);
	void __fastcall Panel22Resize(TObject *Sender);
private:
	int PlotWidth,PlotHeight;

	void __fastcall UpdateLog    (int stat, gtime_t time, double *rr, float *qr,
								  double *rb, int ns, double age, double ratio);
	void __fastcall SvrStart     (void);
	void __fastcall SvrStop      (void);
	void __fastcall UpdateTimeSys(void);
	void __fastcall UpdateSolType(void);
	void __fastcall UpdateFont   (void);
	void __fastcall UpdateTime   (void);
	void __fastcall UpdatePos    (void);
	void __fastcall UpdateStr    (void);
	void __fastcall UpdatePlot   (void);
	void __fastcall ChangePlot   (void);
	int  __fastcall ConfOverwrite(const char *path);
	
	void __fastcall DrawSnr      (TCanvas *c, int w, int h, int top, int index);
	void __fastcall DrawSat      (TCanvas *c, int w, int h, int index);
	void __fastcall DrawBL       (TCanvas *c, int w, int h);
	void __fastcall DrawSky      (TCanvas *c, int w, int h);
	void __fastcall DrawText     (TCanvas *c, int x, int y, AnsiString s,
								  TColor color, int align);
	void __fastcall DrawArrow    (TCanvas *c, int x, int y, int siz,
								  int ang, TColor color);
	void __fastcall OpenMoniPort (int port);
	void __fastcall InitSolBuff  (void);
	void __fastcall SaveLog      (void);
	void __fastcall LoadNav      (nav_t *nav);
	void __fastcall SaveNav      (nav_t *nav);
	void __fastcall LoadOpt      (void);
	void __fastcall SaveOpt      (void);
	void __fastcall SetTrayIcon  (int index);
	int  __fastcall ExecCmd      (AnsiString cmd, int show);
public:
	AnsiString IniFile;
	
	int SvrCycle,SvrBuffSize,Scale,SolBuffSize,NavSelect,SavedSol;
	int NmeaReq,NmeaCycle,InTimeTag,OutTimeTag,OutAppend,LogTimeTag,LogAppend;
	int TimeoutTime,ReconTime,SbasCorr,DgpsCorr,TideCorr,FileSwapMargin;
	int Stream[MAXSTRRTK],StreamC[MAXSTRRTK],Format[MAXSTRRTK];
	int CmdEna[2][2],TimeSys,SolType,PlotType,FreqType,MoniPort,OpenPort;
	
	int PSol,PSolS,PSolE,Nsat[2],SolCurrentStat;
	int Sat[2][MAXSAT],Snr[2][MAXSAT][NFREQ],Vsat[2][MAXSAT];
	double Az[2][MAXSAT],El[2][MAXSAT];
	gtime_t *Time;
	int *SolStat,*Nvsat;
	double *SolRov,*SolRef,*Qr,*VelRov,*Age,*Ratio;
	AnsiString Paths[MAXSTRRTK][4],Cmds[3][2],InTimeStart,InTimeSpeed,ExSats;
	AnsiString RcvOpt[3],ProxyAddr;
	AnsiString OutSwapInterval,LogSwapInterval;
	prcopt_t PrcOpt;
	solopt_t SolOpt;
	TFont *PosFont;
	int DebugTraceF,DebugStatusF,OutputGeoidF,BaselineC;
	int RovPosTypeF,RefPosTypeF,RovAntPcvF,RefAntPcvF;
	AnsiString RovAntF,RefAntF,SatPcvFileF,AntPcvFileF;
	double RovAntDel[3],RefAntDel[3],RovPos[3],RefPos[3],NmeaPos[2];
	double Baseline[2];
	AnsiString History[10],MntpHist[10];
	
	AnsiString GeoidDataFileF,StaPosFileF,DCBFileF,LocalDirectory;
	AnsiString PntName[MAXMAPPNT];
	double PntPos[MAXMAPPNT][3];
	int NMapPnt;
	
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
