//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "postmain.h"
#include "postopt.h"
#include "keydlg.h"
#include "viewer.h"
#include "refdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TOptDialog *OptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TOptDialog::TOptDialog(TComponent* Owner)
    : TForm(Owner)
{
    AnsiString label,s;
    int freq[]={1,2,5,6,7,8};
    int nglo=MAXPRNGLO,ngal=MAXPRNGAL,nqzs=MAXPRNQZS,ncmp=MAXPRNCMP;
    
    Freq->Items->Clear();
    for (int i=0;i<NFREQ;i++) {
        label=label+(i>0?"+":"")+s.sprintf("L%d",freq[i]);
        Freq->Items->Add(label);
    }
    if (nglo<=0) NavSys2->Enabled=false;
    if (ngal<=0) NavSys3->Enabled=false;
    if (nqzs<=0) NavSys4->Enabled=false;
    if (ncmp<=0) NavSys6->Enabled=false;
#ifdef EXTLEX
    IonoOpt ->Items->Add("QZSS LEX");
    SatEphem->Items->Add("QZSS LEX");
#endif
    UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::FormShow(TObject *Sender)
{
	GetOpt();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnOkClick(TObject *Sender)
{
	SetOpt();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnLoadClick(TObject *Sender)
{
	OpenDialog->Title="Load Options";
	OpenDialog->FilterIndex=4;
	if (!OpenDialog->Execute()) return;
	LoadOpt(OpenDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSaveClick(TObject *Sender)
{
	SaveDialog->Title="Save Options";
	SaveDialog->FilterIndex=2;
	if (!SaveDialog->Execute()) return;
	SaveOpt(SaveDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnStaPosViewClick(TObject *Sender)
{
	if (StaPosFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(StaPosFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnStaPosFileClick(TObject *Sender)
{
	OpenDialog->Title="Station Postion File";
	OpenDialog->FilterIndex=3;
	if (!OpenDialog->Execute()) return;
	StaPosFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovPosTypeChange(TObject *Sender)
{
	TEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double pos[3];
	GetPos(RovPosTypeP,edit,pos);
	SetPos(RovPosType->ItemIndex,edit,pos);
	RovPosTypeP=RovPosType->ItemIndex;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RefPosTypeChange(TObject *Sender)
{
	TEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double pos[3];
	GetPos(RefPosTypeP,edit,pos);
	SetPos(RefPosType->ItemIndex,edit,pos);
	RefPosTypeP=RefPosType->ItemIndex;
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnRovPosClick(TObject *Sender)
{
	TEdit *edit[]={RovPos1,RovPos2,RovPos3};
	double p[3],pos[3];
	GetPos(RovPosType->ItemIndex,edit,p);
	ecef2pos(p,pos);
	RefDialog->RovPos[0]=pos[0]*R2D;
	RefDialog->RovPos[1]=pos[1]*R2D;
	RefDialog->Pos[2]=pos[2];
	RefDialog->StaPosFile=StaPosFile->Text;
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	pos[0]=RefDialog->Pos[0]*D2R;
	pos[1]=RefDialog->Pos[1]*D2R;
	pos[2]=RefDialog->Pos[2];
	pos2ecef(pos,p);
	SetPos(RovPosType->ItemIndex,edit,p);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnRefPosClick(TObject *Sender)
{
	TEdit *edit[]={RefPos1,RefPos2,RefPos3};
	double p[3],pos[3];
	GetPos(RefPosType->ItemIndex,edit,p);
	ecef2pos(p,pos);
	RefDialog->RovPos[0]=pos[0]*R2D;
	RefDialog->RovPos[1]=pos[1]*R2D;
	RefDialog->RovPos[2]=pos[2];
	RefDialog->StaPosFile=StaPosFile->Text;
	RefDialog->Left=Left+Width/2-RefDialog->Width/2;
	RefDialog->Top=Top+Height/2-RefDialog->Height/2;
	if (RefDialog->ShowModal()!=mrOk) return;
	pos[0]=RefDialog->Pos[0]*D2R;
	pos[1]=RefDialog->Pos[1]*D2R;
	pos[2]=RefDialog->Pos[2];
	pos2ecef(pos,p);
	SetPos(RefPosType->ItemIndex,edit,p);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSatPcvViewClick(TObject *Sender)
{
	if (SatPcvFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(SatPcvFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnSatPcvFileClick(TObject *Sender)
{
	OpenDialog->Title="Satellite Antenna PCV File";
	OpenDialog->FilterIndex=2;
	if (!OpenDialog->Execute()) return;
	SatPcvFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnAntPcvViewClick(TObject *Sender)
{
	if (AntPcvFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(AntPcvFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnAntPcvFileClick(TObject *Sender)
{
	OpenDialog->Title="Receiver Antenna PCV File";
	OpenDialog->FilterIndex=2;
	if (!OpenDialog->Execute()) return;
	AntPcvFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnGeoidDataFileClick(TObject *Sender)
{
	OpenDialog->Title="Geoid Data File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	GeoidDataFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnIonoFileClick(TObject *Sender)
{
	OpenDialog->Title="Iono Data File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	IonoFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnIonoViewClick(TObject *Sender)
{
	if (IonoFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(IonoFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnDCBFileClick(TObject *Sender)
{
	OpenDialog->Title="DCB Data File";
	OpenDialog->FilterIndex=1;
	if (!OpenDialog->Execute()) return;
	DCBFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnDCBViewClick(TObject *Sender)
{
	if (DCBFile->Text=="") return;
	TTextViewer *viewer=new TTextViewer(Application);
	viewer->Show();
	viewer->Read(DCBFile->Text);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BtnGoogleEarthFileClick(TObject *Sender)
{
	OpenDialog->Title="Google Earth Exe File";
	OpenDialog->FilterIndex=5;
	if (!OpenDialog->Execute()) return;
	GoogleEarthFile->Text=OpenDialog->FileName;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::FreqChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::IonoOptChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::TropOptChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::DynamicModelChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SatEphemChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SolFormatChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::PosModeChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SatEphemClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::NavSys2Click(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::AmbResChange(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovAntPcvClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::NetRSCorrClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SatClkCorrClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovPosClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RefPosClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SbasCorrClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::OutputHeightClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::BaselineConstClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::RovAntClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------

void __fastcall TOptDialog::RefAntClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::GetOpt(void)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	AnsiString s;
	PosMode		 ->ItemIndex	=MainForm->PosMode;
	Freq		 ->ItemIndex	=MainForm->Freq>NFREQ-1?NFREQ-1:MainForm->Freq;
	Solution	 ->ItemIndex	=MainForm->Solution;
	ElMask		 ->Text			=s.sprintf("%.0f",MainForm->ElMask);
	SnrMask		 ->Text			=s.sprintf("%.0f",MainForm->SnrMask);
	DynamicModel ->ItemIndex	=MainForm->DynamicModel;
	TideCorr	 ->ItemIndex	=MainForm->TideCorr;
	IonoOpt		 ->ItemIndex	=MainForm->IonoOpt;
	TropOpt		 ->ItemIndex	=MainForm->TropOpt;
	SatEphem	 ->ItemIndex	=MainForm->SatEphem;
	ExSats	     ->Text			=MainForm->ExSats;
	NavSys1	     ->Checked		=MainForm->NavSys&SYS_GPS;
	NavSys2	     ->Checked		=MainForm->NavSys&SYS_GLO;
	NavSys3	     ->Checked		=MainForm->NavSys&SYS_GAL;
	NavSys4	     ->Checked		=MainForm->NavSys&SYS_QZS;
	NavSys5	     ->Checked		=MainForm->NavSys&SYS_SBS;
	NavSys6	     ->Checked		=MainForm->NavSys&SYS_CMP;
	
	AmbRes		 ->ItemIndex	=MainForm->AmbRes;
	GloAmbRes	 ->ItemIndex	=MainForm->GloAmbRes;
	ValidThresAR ->Text			=s.sprintf("%.1f",MainForm->ValidThresAR);
	OutCntResetAmb->Text		=s.sprintf("%d",MainForm->OutCntResetAmb);
	FixCntHoldAmb->Text			=s.sprintf("%d",MainForm->FixCntHoldAmb);
	LockCntFixAmb->Text			=s.sprintf("%d",MainForm->LockCntFixAmb);
	ElMaskAR	 ->Text			=s.sprintf("%.0f",MainForm->ElMaskAR);
	ElMaskHold	 ->Text			=s.sprintf("%.0f",MainForm->ElMaskHold);
	MaxAgeDiff	 ->Text			=s.sprintf("%.1f",MainForm->MaxAgeDiff);
	RejectGdop   ->Text			=s.sprintf("%.1f",MainForm->RejectGdop);
	RejectThres  ->Text			=s.sprintf("%.1f",MainForm->RejectThres);
	SlipThres	 ->Text			=s.sprintf("%.3f",MainForm->SlipThres);
	NumIter		 ->Text			=s.sprintf("%d",  MainForm->NumIter);
	BaselineLen	 ->Text			=s.sprintf("%.3f",MainForm->BaseLine[0]);
	BaselineSig	 ->Text			=s.sprintf("%.3f",MainForm->BaseLine[1]);
	BaselineConst->Checked		=MainForm->BaseLineConst;
	
	SolFormat	 ->ItemIndex	=MainForm->SolFormat;
	TimeFormat	 ->ItemIndex	=MainForm->TimeFormat;
	TimeDecimal	 ->Text			=s.sprintf("%d",MainForm->TimeDecimal);
	LatLonFormat ->ItemIndex	=MainForm->LatLonFormat;
	FieldSep	 ->Text			=MainForm->FieldSep;
	OutputHead	 ->ItemIndex	=MainForm->OutputHead;
	OutputOpt	 ->ItemIndex	=MainForm->OutputOpt;
	OutputDatum  ->ItemIndex	=MainForm->OutputDatum;
	OutputHeight ->ItemIndex	=MainForm->OutputHeight;
	OutputGeoid  ->ItemIndex	=MainForm->OutputGeoid;
	SolStatic    ->ItemIndex	=MainForm->SolStatic;
	DebugTrace	 ->ItemIndex	=MainForm->DebugTrace;
	DebugStatus	 ->ItemIndex	=MainForm->DebugStatus;
	
	MeasErrR1	 ->Text			=s.sprintf("%.1f",MainForm->MeasErrR1);
	MeasErrR2	 ->Text			=s.sprintf("%.1f",MainForm->MeasErrR2);
	MeasErr2	 ->Text			=s.sprintf("%.3f",MainForm->MeasErr2);
	MeasErr3	 ->Text			=s.sprintf("%.3f",MainForm->MeasErr3);
	MeasErr4	 ->Text			=s.sprintf("%.3f",MainForm->MeasErr4);
	MeasErr5	 ->Text			=s.sprintf("%.3f",MainForm->MeasErr5);
	SatClkStab	 ->Text			=s.sprintf("%.2E",MainForm->SatClkStab);
	PrNoise1	 ->Text			=s.sprintf("%.2E",MainForm->PrNoise1);
	PrNoise2	 ->Text			=s.sprintf("%.2E",MainForm->PrNoise2);
	PrNoise3	 ->Text			=s.sprintf("%.2E",MainForm->PrNoise3);
	PrNoise4	 ->Text			=s.sprintf("%.2E",MainForm->PrNoise4);
	PrNoise5	 ->Text			=s.sprintf("%.2E",MainForm->PrNoise5);
	
	RovAntPcv	 ->Checked		=MainForm->RovAntPcv;
	RefAntPcv	 ->Checked		=MainForm->RefAntPcv;
	RovAnt		 ->Text			=MainForm->RovAnt;
	RefAnt		 ->Text			=MainForm->RefAnt;
	RovAntE		 ->Text			=s.sprintf("%.4f",MainForm->RovAntE);
	RovAntN		 ->Text			=s.sprintf("%.4f",MainForm->RovAntN);
	RovAntU		 ->Text			=s.sprintf("%.4f",MainForm->RovAntU);
	RefAntE		 ->Text			=s.sprintf("%.4f",MainForm->RefAntE);
	RefAntN		 ->Text			=s.sprintf("%.4f",MainForm->RefAntN);
	RefAntU		 ->Text			=s.sprintf("%.4f",MainForm->RefAntU);
	AntPcvFile	 ->Text			=MainForm->AntPcvFile;
	
	IntpRefObs	 ->ItemIndex	=MainForm->IntpRefObs;
	SbasSat		 ->Text			=s.sprintf("%d",MainForm->SbasSat);
	SatPcvFile   ->Text			=MainForm->SatPcvFile;
	StaPosFile	 ->Text			=MainForm->StaPosFile;
	GeoidDataFile->Text			=MainForm->GeoidDataFile;
	IonoFile	 ->Text			=MainForm->IonoFile;
	DCBFile		 ->Text			=MainForm->DCBFile;
	GoogleEarthFile->Text		=MainForm->GoogleEarthFile;
	RovPosType	 ->ItemIndex	=MainForm->RovPosType;
	RefPosType	 ->ItemIndex	=MainForm->RefPosType;
	RovPosTypeP					=RovPosType->ItemIndex;
	RefPosTypeP					=RefPosType->ItemIndex;
	SetPos(RovPosType->ItemIndex,editu,MainForm->RovPos);
	SetPos(RefPosType->ItemIndex,editr,MainForm->RefPos);
	ReadAntList();
	
	RovList		 ->Text			=MainForm->RovList;
	BaseList	 ->Text			=MainForm->BaseList;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SetOpt(void)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	
	MainForm->PosMode		=PosMode	->ItemIndex;
	MainForm->Freq			=Freq		->ItemIndex;
	MainForm->Solution		=Solution   ->ItemIndex;
	MainForm->ElMask		=str2dbl(ElMask	->Text);
	MainForm->SnrMask		=str2dbl(SnrMask->Text);
	MainForm->DynamicModel	=DynamicModel->ItemIndex;
	MainForm->TideCorr		=TideCorr	->ItemIndex;
	MainForm->IonoOpt	  	=IonoOpt	->ItemIndex;
	MainForm->TropOpt	  	=TropOpt	->ItemIndex;
	MainForm->SatEphem	  	=SatEphem	->ItemIndex;
	MainForm->ExSats	  	=ExSats		->Text;
	MainForm->NavSys	  	=0;
	if (NavSys1->Checked) MainForm->NavSys|=SYS_GPS;
	if (NavSys2->Checked) MainForm->NavSys|=SYS_GLO;
	if (NavSys3->Checked) MainForm->NavSys|=SYS_GAL;
	if (NavSys4->Checked) MainForm->NavSys|=SYS_QZS;
	if (NavSys5->Checked) MainForm->NavSys|=SYS_SBS;
	if (NavSys6->Checked) MainForm->NavSys|=SYS_CMP;
	
	MainForm->AmbRes	  	=AmbRes		->ItemIndex;
	MainForm->GloAmbRes	  	=GloAmbRes	->ItemIndex;
	MainForm->ValidThresAR	=str2dbl(ValidThresAR->Text);
	MainForm->OutCntResetAmb=OutCntResetAmb->Text.ToInt();
	MainForm->FixCntHoldAmb =FixCntHoldAmb->Text.ToInt();
	MainForm->OutCntResetAmb=OutCntResetAmb->Text.ToInt();
	MainForm->LockCntFixAmb	=LockCntFixAmb->Text.ToInt();
	MainForm->ElMaskAR	  	=ElMaskAR   ->Text.ToInt();
	MainForm->ElMaskHold  	=ElMaskHold ->Text.ToInt();
	MainForm->MaxAgeDiff  	=str2dbl(MaxAgeDiff ->Text);
	MainForm->RejectGdop 	=str2dbl(RejectGdop ->Text);
	MainForm->RejectThres 	=str2dbl(RejectThres->Text);
	MainForm->SlipThres   	=str2dbl(SlipThres  ->Text);
	MainForm->NumIter	  	=NumIter	  ->Text.ToInt();
	MainForm->BaseLine[0]  	=str2dbl(BaselineLen->Text);
	MainForm->BaseLine[1]  	=str2dbl(BaselineSig->Text);
	MainForm->BaseLineConst	=BaselineConst->Checked;
	
	MainForm->SolFormat   	=SolFormat  ->ItemIndex;
	MainForm->TimeFormat  	=TimeFormat ->ItemIndex;
	MainForm->TimeDecimal  	=str2dbl(TimeDecimal->Text);
	MainForm->LatLonFormat	=LatLonFormat->ItemIndex;
	MainForm->FieldSep	  	=FieldSep   ->Text;
	MainForm->OutputHead  	=OutputHead ->ItemIndex;
	MainForm->OutputOpt   	=OutputOpt  ->ItemIndex;
	MainForm->OutputDatum 	=OutputDatum->ItemIndex;
	MainForm->OutputHeight	=OutputHeight->ItemIndex;
	MainForm->OutputGeoid 	=OutputGeoid->ItemIndex;
	MainForm->SolStatic	 	=SolStatic  ->ItemIndex;
	MainForm->DebugTrace  	=DebugTrace ->ItemIndex;
	MainForm->DebugStatus  	=DebugStatus->ItemIndex;
	
	MainForm->MeasErrR1	  =str2dbl(MeasErrR1  ->Text);
	MainForm->MeasErrR2	  =str2dbl(MeasErrR2  ->Text);
	MainForm->MeasErr2	  =str2dbl(MeasErr2   ->Text);
	MainForm->MeasErr3	  =str2dbl(MeasErr3   ->Text);
	MainForm->MeasErr4	  =str2dbl(MeasErr4   ->Text);
	MainForm->MeasErr5	  =str2dbl(MeasErr5   ->Text);
	MainForm->SatClkStab  =str2dbl(SatClkStab ->Text);
	MainForm->PrNoise1	  =str2dbl(PrNoise1   ->Text);
	MainForm->PrNoise2	  =str2dbl(PrNoise2   ->Text);
	MainForm->PrNoise3	  =str2dbl(PrNoise3   ->Text);
	MainForm->PrNoise4	  =str2dbl(PrNoise4   ->Text);
	MainForm->PrNoise5	  =str2dbl(PrNoise5   ->Text);
	
	MainForm->RovAntPcv   =RovAntPcv	->Checked;
	MainForm->RefAntPcv   =RefAntPcv	->Checked;
	MainForm->RovAnt	  =RovAnt		->Text;
	MainForm->RefAnt	  =RefAnt		->Text;
	MainForm->RovAntE	  =str2dbl(RovAntE	->Text);
	MainForm->RovAntN	  =str2dbl(RovAntN	->Text);
	MainForm->RovAntU	  =str2dbl(RovAntU	->Text);
	MainForm->RefAntE	  =str2dbl(RefAntE	->Text);
	MainForm->RefAntN	  =str2dbl(RefAntN	->Text);
	MainForm->RefAntU	  =str2dbl(RefAntU	->Text);
	
	MainForm->IntpRefObs  =IntpRefObs	->ItemIndex;
	MainForm->SbasSat     =SbasSat		->Text.ToInt();
	MainForm->AntPcvFile  =AntPcvFile	->Text;
	MainForm->SatPcvFile  =SatPcvFile	->Text;
	MainForm->StaPosFile  =StaPosFile	->Text;
	MainForm->GeoidDataFile=GeoidDataFile->Text;
	MainForm->IonoFile    =IonoFile		->Text;
	MainForm->DCBFile     =DCBFile		->Text;
	MainForm->GoogleEarthFile=GoogleEarthFile->Text;
	MainForm->RovPosType  =RovPosType	->ItemIndex;
	MainForm->RefPosType  =RefPosType	->ItemIndex;
	GetPos(RovPosType->ItemIndex,editu,MainForm->RovPos);
	GetPos(RefPosType->ItemIndex,editr,MainForm->RefPos);
	
	MainForm->RovList	  =RovList		->Text;
	MainForm->BaseList	  =BaseList		->Text;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::LoadOpt(AnsiString file)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	AnsiString s;
	char buff[1024]="",*p,id[32];
	int sat;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
	filopt_t filopt={""};
	
	resetsysopts();
	if (!loadopts(file.c_str(),sysopts)) return;
	getsysopts(&prcopt,&solopt,&filopt);
	
	PosMode		 ->ItemIndex	=prcopt.mode;
	Freq		 ->ItemIndex	=prcopt.nf>NFREQ-1?NFREQ-1:prcopt.nf-1;
	Solution	 ->ItemIndex	=prcopt.soltype;
	ElMask		 ->Text			=s.sprintf("%.0f",prcopt.elmin*R2D);
	SnrMask		 ->Text			=s.sprintf("%.0f",prcopt.snrmin);
	DynamicModel ->ItemIndex	=prcopt.dynamics;
	TideCorr	 ->ItemIndex	=prcopt.tidecorr;
	IonoOpt		 ->ItemIndex	=prcopt.ionoopt;
	TropOpt		 ->ItemIndex	=prcopt.tropopt;
	SatEphem	 ->ItemIndex	=prcopt.sateph;
	ExSats	     ->Text			="";
	for (sat=1,p=buff;sat<=MAXSAT;sat++) {
		if (!prcopt.exsats[sat-1]) continue;
		satno2id(sat,id);
		p+=sprintf(p,"%s%s%s",p==buff?"":" ",prcopt.exsats[sat-1]==2?"+":"",id);
	}
	ExSats		 ->Text			=buff;
	NavSys1	     ->Checked		=prcopt.navsys&SYS_GPS;
	NavSys2	     ->Checked		=prcopt.navsys&SYS_GLO;
	NavSys3	     ->Checked		=prcopt.navsys&SYS_GAL;
	NavSys4	     ->Checked		=prcopt.navsys&SYS_QZS;
	NavSys5	     ->Checked		=prcopt.navsys&SYS_SBS;
	NavSys6	     ->Checked		=prcopt.navsys&SYS_CMP;
	
	AmbRes		 ->ItemIndex	=prcopt.modear;
	GloAmbRes	 ->ItemIndex	=prcopt.glomodear;
	ValidThresAR ->Text			=s.sprintf("%.1f",prcopt.thresar  );
	OutCntResetAmb->Text		=s.sprintf("%d"  ,prcopt.maxout   );
	FixCntHoldAmb->Text			=s.sprintf("%d"  ,prcopt.minfix   );
	LockCntFixAmb  ->Text		=s.sprintf("%d"  ,prcopt.minlock  );
	ElMaskAR	 ->Text			=s.sprintf("%.0f",prcopt.elmaskar*R2D);
	ElMaskHold	 ->Text			=s.sprintf("%.0f",prcopt.elmaskhold*R2D);
	MaxAgeDiff	 ->Text			=s.sprintf("%.1f",prcopt.maxtdiff );
	RejectGdop   ->Text			=s.sprintf("%.1f",prcopt.maxgdop  );
	RejectThres  ->Text			=s.sprintf("%.1f",prcopt.maxinno  );
	SlipThres	 ->Text			=s.sprintf("%.3f",prcopt.thresslip);
	NumIter		 ->Text			=s.sprintf("%d",  prcopt.niter    );
	BaselineLen	 ->Text			=s.sprintf("%.3f",prcopt.baseline[0]);
	BaselineSig	 ->Text			=s.sprintf("%.3f",prcopt.baseline[1]);
	BaselineConst->Checked		=prcopt.baseline[0]>0.0;
	
	SolFormat	 ->ItemIndex	=solopt.posf;
	TimeFormat	 ->ItemIndex	=solopt.timef==0?0:solopt.times+1;
	TimeDecimal	 ->Text			=s.sprintf("%d",solopt.timeu);
	LatLonFormat ->ItemIndex	=solopt.degf;
	FieldSep	 ->Text			=solopt.sep;
	OutputHead	 ->ItemIndex	=solopt.outhead;
	OutputOpt	 ->ItemIndex	=solopt.outopt;
	OutputDatum  ->ItemIndex	=solopt.datum;
	OutputHeight ->ItemIndex	=solopt.height;
	OutputGeoid  ->ItemIndex	=solopt.geoid;
	SolStatic    ->ItemIndex	=solopt.solstatic;
	NmeaIntv1	 ->Text			=s.sprintf("%.2g",solopt.nmeaintv[0]);
	NmeaIntv2	 ->Text			=s.sprintf("%.2g",solopt.nmeaintv[1]);
	DebugTrace	 ->ItemIndex	=solopt.trace;
	DebugStatus	 ->ItemIndex	=solopt.sstat;
	
	MeasErrR1	 ->Text			=s.sprintf("%.1f",prcopt.eratio[0]);
	MeasErrR2	 ->Text			=s.sprintf("%.1f",prcopt.eratio[1]);
	MeasErr2	 ->Text			=s.sprintf("%.3f",prcopt.err[1]);
	MeasErr3	 ->Text			=s.sprintf("%.3f",prcopt.err[2]);
	MeasErr4	 ->Text			=s.sprintf("%.3f",prcopt.err[3]);
	MeasErr5	 ->Text			=s.sprintf("%.3f",prcopt.err[4]);
	SatClkStab	 ->Text			=s.sprintf("%.2E",prcopt.sclkstab);
	PrNoise1	 ->Text			=s.sprintf("%.2E",prcopt.prn[0]);
	PrNoise2	 ->Text			=s.sprintf("%.2E",prcopt.prn[1]);
	PrNoise3	 ->Text			=s.sprintf("%.2E",prcopt.prn[2]);
	PrNoise4	 ->Text			=s.sprintf("%.2E",prcopt.prn[3]);
	PrNoise5	 ->Text			=s.sprintf("%.2E",prcopt.prn[4]);
	
	RovAntPcv	 ->Checked		=*prcopt.anttype[0];
	RefAntPcv	 ->Checked		=*prcopt.anttype[1];
	RovAnt		 ->Text			=prcopt.anttype[0];
	RefAnt		 ->Text			=prcopt.anttype[1];
	RovAntE		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][0]);
	RovAntN		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][1]);
	RovAntU		 ->Text			=s.sprintf("%.4f",prcopt.antdel[0][2]);
	RefAntE		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][0]);
	RefAntN		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][1]);
	RefAntU		 ->Text			=s.sprintf("%.4f",prcopt.antdel[1][2]);
	
	IntpRefObs	 ->ItemIndex	=prcopt.intpref;
	SbasSat		 ->Text			=s.sprintf("%d",prcopt.sbassatsel);
	RovPosType	 ->ItemIndex	=prcopt.rovpos==0?0:prcopt.rovpos+2;
	RefPosType	 ->ItemIndex	=prcopt.refpos==0?0:prcopt.refpos+2;
	RovPosTypeP					=RovPosType->ItemIndex;
	RefPosTypeP					=RefPosType->ItemIndex;
	SetPos(RovPosType->ItemIndex,editu,prcopt.ru);
	SetPos(RefPosType->ItemIndex,editr,prcopt.rb);
	
	SatPcvFile ->Text			=filopt.satantp;
	AntPcvFile ->Text			=filopt.rcvantp;
	StaPosFile ->Text			=filopt.stapos;
	GeoidDataFile->Text			=filopt.geoid;
	IonoFile   ->Text			=filopt.iono;
	DCBFile	   ->Text			=filopt.dcb;
	GoogleEarthFile->Text		=filopt.geexe;
	
	ReadAntList();
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SaveOpt(AnsiString file)
{
	TEdit *editu[]={RovPos1,RovPos2,RovPos3};
	TEdit *editr[]={RefPos1,RefPos2,RefPos3};
	char buff[1024],*p,id[32],comment[256],s[64];
	int sat,ex;
	prcopt_t prcopt=prcopt_default;
	solopt_t solopt=solopt_default;
	filopt_t filopt={""};
	
	prcopt.mode		=PosMode	 ->ItemIndex;
	prcopt.nf		=Freq		 ->ItemIndex+1;
	prcopt.soltype	=Solution	 ->ItemIndex;
	prcopt.elmin	=str2dbl(ElMask	->Text)*D2R;
	prcopt.snrmin	=str2dbl(SnrMask->Text);
	prcopt.dynamics	=DynamicModel->ItemIndex;
	prcopt.tidecorr	=TideCorr	 ->ItemIndex;
	prcopt.ionoopt	=IonoOpt	 ->ItemIndex;
	prcopt.tropopt	=TropOpt	 ->ItemIndex;
	prcopt.sateph	=SatEphem	 ->ItemIndex;
	if (ExSats->Text!="") {
		strcpy(buff,ExSats->Text.c_str());
		for (p=strtok(buff," ");p;p=strtok(NULL," ")) {
			if (*p=='+') {ex=2; p++;} else ex=1;
			if (!(sat=satid2no(p))) continue;
			prcopt.exsats[sat-1]=ex;
		}
	}
	prcopt.navsys	= (NavSys1->Checked?SYS_GPS:0)|
					  (NavSys2->Checked?SYS_GLO:0)|
					  (NavSys3->Checked?SYS_GAL:0)|
					  (NavSys4->Checked?SYS_QZS:0)|
					  (NavSys5->Checked?SYS_SBS:0)|
					  (NavSys6->Checked?SYS_CMP:0);
	prcopt.modear	=AmbRes		->ItemIndex;
	prcopt.glomodear=GloAmbRes	->ItemIndex;
	prcopt.thresar	=str2dbl(ValidThresAR->Text);
	prcopt.maxout	=str2dbl(OutCntResetAmb->Text);
	prcopt.minfix	=str2dbl(FixCntHoldAmb->Text);
	prcopt.minlock	=str2dbl(LockCntFixAmb->Text);
	prcopt.elmaskar	=str2dbl(ElMaskAR	->Text)*D2R;
	prcopt.elmaskhold=str2dbl(ElMaskHold->Text)*D2R;
	prcopt.maxtdiff	=str2dbl(MaxAgeDiff	->Text);
	prcopt.maxgdop	=str2dbl(RejectGdop ->Text);
	prcopt.maxinno	=str2dbl(RejectThres->Text);
	prcopt.thresslip=str2dbl(SlipThres	->Text);
	prcopt.niter	=str2dbl(NumIter	->Text);
	if (prcopt.mode==PMODE_MOVEB&&BaselineConst->Checked) {
		prcopt.baseline[0]=str2dbl(BaselineLen->Text);
		prcopt.baseline[1]=str2dbl(BaselineSig->Text);
	}
	solopt.posf		=SolFormat	->ItemIndex;
	solopt.timef	=TimeFormat	->ItemIndex==0?0:1;
	solopt.times	=TimeFormat	->ItemIndex==0?0:TimeFormat->ItemIndex-1;
	solopt.timeu	=str2dbl(TimeDecimal ->Text);
	solopt.degf		=LatLonFormat->ItemIndex;
	strcpy(solopt.sep,FieldSep->Text.c_str());
	solopt.outhead	=OutputHead	 ->ItemIndex;
	solopt.outopt	=OutputOpt	 ->ItemIndex;
	solopt.datum	=OutputDatum ->ItemIndex;
	solopt.height	=OutputHeight->ItemIndex;
	solopt.geoid	=OutputGeoid ->ItemIndex;
	solopt.solstatic=SolStatic   ->ItemIndex;
	solopt.nmeaintv[0]=str2dbl(NmeaIntv1->Text);
	solopt.nmeaintv[1]=str2dbl(NmeaIntv2->Text);
	solopt.trace	=DebugTrace	 ->ItemIndex;
	solopt.sstat	=DebugStatus ->ItemIndex;
	
	prcopt.eratio[0]=str2dbl(MeasErrR1->Text);
	prcopt.eratio[1]=str2dbl(MeasErrR2->Text);
	prcopt.err[1]	=str2dbl(MeasErr2->Text);
	prcopt.err[2]	=str2dbl(MeasErr3->Text);
	prcopt.err[3]	=str2dbl(MeasErr4->Text);
	prcopt.err[4]	=str2dbl(MeasErr5->Text);
	prcopt.sclkstab	=str2dbl(SatClkStab->Text);
	prcopt.prn[0]	=str2dbl(PrNoise1->Text);
	prcopt.prn[1]	=str2dbl(PrNoise2->Text);
	prcopt.prn[2]	=str2dbl(PrNoise3->Text);
	prcopt.prn[3]	=str2dbl(PrNoise4->Text);
	prcopt.prn[4]	=str2dbl(PrNoise5->Text);
	
	if (RovAntPcv->Checked) strcpy(prcopt.anttype[0],RovAnt->Text.c_str());
	if (RefAntPcv->Checked) strcpy(prcopt.anttype[1],RefAnt->Text.c_str());
	prcopt.antdel[0][0]=str2dbl(RovAntE->Text);
	prcopt.antdel[0][1]=str2dbl(RovAntN->Text);
	prcopt.antdel[0][2]=str2dbl(RovAntU->Text);
	prcopt.antdel[1][0]=str2dbl(RefAntE->Text);
	prcopt.antdel[1][1]=str2dbl(RefAntN->Text);
	prcopt.antdel[1][2]=str2dbl(RefAntU->Text);
	
	prcopt.intpref	=IntpRefObs->ItemIndex;
	prcopt.sbassatsel=SbasSat->Text.ToInt();
	prcopt.rovpos=RovPosType->ItemIndex<3?0:RovPosType->ItemIndex-2;
	prcopt.refpos=RefPosType->ItemIndex<3?0:RefPosType->ItemIndex-2;
	if (prcopt.rovpos==0) GetPos(RovPosType->ItemIndex,editu,prcopt.ru);
	if (prcopt.refpos==0) GetPos(RefPosType->ItemIndex,editr,prcopt.rb);
	
	strcpy(filopt.satantp,SatPcvFile     ->Text.c_str());
	strcpy(filopt.rcvantp,AntPcvFile     ->Text.c_str());
	strcpy(filopt.stapos, StaPosFile     ->Text.c_str());
	strcpy(filopt.geoid,  GeoidDataFile  ->Text.c_str());
	strcpy(filopt.dcb,    DCBFile        ->Text.c_str());
	strcpy(filopt.iono,   IonoFile       ->Text.c_str());
	strcpy(filopt.geexe,  GoogleEarthFile->Text.c_str());
	
	time2str(utc2gpst(timeget()),s,0);
	sprintf(comment,"rtkpost options (%s, v.%s)",s,VER_RTKLIB);
	setsysopts(&prcopt,&solopt,&filopt);
	if (!saveopts(file.c_str(),"w",comment,sysopts)) return;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::UpdateEnable(void)
{
	int rel=PMODE_DGPS<=PosMode->ItemIndex&&PosMode->ItemIndex<=PMODE_FIXED;
	int rtk=PMODE_KINEMA<=PosMode->ItemIndex&&PosMode->ItemIndex<=PMODE_FIXED;
	int ppp=PosMode->ItemIndex>=PMODE_PPP_KINEMA;
	
	Freq           ->Enabled=rel;
	Solution       ->Enabled=rel||ppp;
	DynamicModel   ->Enabled=rel;
	TideCorr       ->Enabled=rel||ppp;
	//IonoOpt        ->Enabled=!ppp;
	
	AmbRes         ->Enabled=rtk;
	GloAmbRes      ->Enabled=rtk&&AmbRes->ItemIndex>0&&NavSys2->Checked;
	ValidThresAR   ->Enabled=rtk&&AmbRes->ItemIndex>=1;
	LockCntFixAmb  ->Enabled=rtk&&AmbRes->ItemIndex>=1;
	ElMaskAR       ->Enabled=rtk&&AmbRes->ItemIndex>=1;
	OutCntResetAmb ->Enabled=rtk||ppp;
	FixCntHoldAmb  ->Enabled=rtk&&AmbRes->ItemIndex==3;
	ElMaskHold     ->Enabled=rtk&&AmbRes->ItemIndex==3;
	LabelHold      ->Enabled=rtk&&AmbRes->ItemIndex==3;
	SlipThres      ->Enabled=rtk||ppp;
	MaxAgeDiff     ->Enabled=rel;
	RejectThres    ->Enabled=rel||ppp;
	NumIter        ->Enabled=rel||ppp;
	BaselineConst  ->Enabled=PosMode->ItemIndex==PMODE_MOVEB;
	BaselineLen    ->Enabled=BaselineConst->Checked&&PosMode->ItemIndex==PMODE_MOVEB;
	BaselineSig    ->Enabled=BaselineConst->Checked&&PosMode->ItemIndex==PMODE_MOVEB;
	
	OutputHead     ->Enabled=SolFormat->ItemIndex!=3;
	OutputOpt      ->Enabled=SolFormat->ItemIndex!=3;
	TimeFormat     ->Enabled=SolFormat->ItemIndex!=3;
	TimeDecimal    ->Enabled=SolFormat->ItemIndex!=3;
	LatLonFormat   ->Enabled=SolFormat->ItemIndex==0;
	FieldSep       ->Enabled=SolFormat->ItemIndex!=3;
	OutputDatum    ->Enabled=SolFormat->ItemIndex==0;
	OutputHeight   ->Enabled=SolFormat->ItemIndex==0;
	OutputGeoid    ->Enabled=SolFormat->ItemIndex==0&&OutputHeight->ItemIndex==1;
	SolStatic      ->Enabled=PosMode->ItemIndex==PMODE_STATIC||
							 PosMode->ItemIndex==PMODE_PPP_STATIC;
	
	RovAntPcv      ->Enabled=rel||ppp;
	RovAnt         ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntE        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntN        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RovAntU        ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	LabelRovAntD   ->Enabled=(rel||ppp)&&RovAntPcv->Checked;
	RefAntPcv      ->Enabled=rel;
	RefAnt         ->Enabled=rel&&RefAntPcv->Checked;
	RefAntE        ->Enabled=rel&&RefAntPcv->Checked;
	RefAntN        ->Enabled=rel&&RefAntPcv->Checked;
	RefAntU        ->Enabled=rel&&RefAntPcv->Checked;
	LabelRefAntD   ->Enabled=rel&&RefAntPcv->Checked;
	
	RovPosType     ->Enabled=PosMode->ItemIndex==PMODE_FIXED||PosMode->ItemIndex==PMODE_PPP_FIXED;
	RovPos1        ->Enabled=RovPosType->Enabled&&RovPosType->ItemIndex<=2;
	RovPos2        ->Enabled=RovPosType->Enabled&&RovPosType->ItemIndex<=2;
	RovPos3        ->Enabled=RovPosType->Enabled&&RovPosType->ItemIndex<=2;
	BtnRovPos      ->Enabled=RovPosType->Enabled&&RovPosType->ItemIndex<=2;
	
	RefPosType     ->Enabled=rel&&PosMode->ItemIndex!=PMODE_MOVEB;
	RefPos1        ->Enabled=RefPosType->Enabled&&RefPosType->ItemIndex<=2;
	RefPos2        ->Enabled=RefPosType->Enabled&&RefPosType->ItemIndex<=2;
	RefPos3        ->Enabled=RefPosType->Enabled&&RefPosType->ItemIndex<=2;
	BtnRefPos      ->Enabled=RefPosType->Enabled&&RefPosType->ItemIndex<=2;
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::GetPos(int type, TEdit **edit, double *pos)
{
	double p[3]={0},dms1[3]={0},dms2[3]={0};
	
	if (type==1) { /* lat/lon/height dms/m */
		sscanf(edit[0]->Text.c_str(),"%lf %lf %lf",dms1,dms1+1,dms1+2);
		sscanf(edit[1]->Text.c_str(),"%lf %lf %lf",dms2,dms2+1,dms2+2);
		p[0]=(dms1[0]<0?-1:1)*(fabs(dms1[0])+dms1[1]/60+dms1[2]/3600)*D2R;
		p[1]=(dms1[0]<0?-1:1)*(fabs(dms2[0])+dms2[1]/60+dms2[2]/3600)*D2R;
		p[2]=str2dbl(edit[2]->Text);
		pos2ecef(p,pos);
	}
	else if (type==2) { /* x/y/z-ecef */
		pos[0]=str2dbl(edit[0]->Text);
		pos[1]=str2dbl(edit[1]->Text);
		pos[2]=str2dbl(edit[2]->Text);
	}
	else {
		p[0]=str2dbl(edit[0]->Text)*D2R;
		p[1]=str2dbl(edit[1]->Text)*D2R;
		p[2]=str2dbl(edit[2]->Text);
		pos2ecef(p,pos);
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SetPos(int type, TEdit **edit, double *pos)
{
	AnsiString s;
	double p[3],dms1[3],dms2[3],s1,s2;
	
	if (type==1) { /* lat/lon/height dms/m */
		ecef2pos(pos,p); s1=p[0]<0?-1:1; s2=p[1]<0?-1:1;
		p[0]=fabs(p[0])*R2D+1E-12; p[1]=fabs(p[1])*R2D+1E-12;
		dms1[0]=floor(p[0]); p[0]=(p[0]-dms1[0])*60.0;
		dms1[1]=floor(p[0]); dms1[2]=(p[0]-dms1[1])*60.0;
		dms2[0]=floor(p[1]); p[1]=(p[1]-dms2[0])*60.0;
		dms2[1]=floor(p[1]); dms2[2]=(p[1]-dms2[1])*60.0;
		edit[0]->Text=s.sprintf("%.0f %02.0f %09.6f",s1*dms1[0],dms1[1],dms1[2]);
		edit[1]->Text=s.sprintf("%.0f %02.0f %09.6f",s2*dms2[0],dms2[1],dms2[2]);
		edit[2]->Text=s.sprintf("%.4f",p[2]);
	}
	else if (type==2) { /* x/y/z-ecef */
		edit[0]->Text=s.sprintf("%.4f",pos[0]);
		edit[1]->Text=s.sprintf("%.4f",pos[1]);
		edit[2]->Text=s.sprintf("%.4f",pos[2]);
	}
	else {
		ecef2pos(pos,p);
		edit[0]->Text=s.sprintf("%.9f",p[0]*R2D);
		edit[1]->Text=s.sprintf("%.9f",p[1]*R2D);
		edit[2]->Text=s.sprintf("%.4f",p[2]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::ReadAntList(void)
{
	TStringList *list;
	pcvs_t pcvs={0};
	char *p;
	
	if (!readpcv(AntPcvFile->Text.c_str(),&pcvs)) return;
	
	list=new TStringList;
	list->Add("");
	list->Add("*");
	
	for (int i=0;i<pcvs.n;i++) {
		if (pcvs.pcv[i].sat) continue;
		if ((p=strchr(pcvs.pcv[i].type,' '))) *p='\0';
		if (i>0&&!strcmp(pcvs.pcv[i].type,pcvs.pcv[i-1].type)) continue;
		list->Add(pcvs.pcv[i].type);
	}
	RovAnt->Items=list;
	RefAnt->Items=list;
	
	free(pcvs.pcv);
}
//---------------------------------------------------------------------------
void __fastcall TOptDialog::SpeedButton1Click(TObject *Sender)
{
	KeyDialog->Flag=2;
	KeyDialog->Show();
}
//---------------------------------------------------------------------------

