//---------------------------------------------------------------------------
#include <vcl.h>
#include <FileCtrl.hpp>
#pragma hdrstop

#include "rtklib.h"
#include "refdlg.h"
#include "svroptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSvrOptDialog *SvrOptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TSvrOptDialog::TSvrOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::FormShow(TObject *Sender)
{
	double pos[3];
	AnsiString s;
	DataTimeout->Text=s.sprintf("%d",SvrOpt[0]);
	ConnectInterval->Text=s.sprintf("%d",SvrOpt[1]);
	AvePeriodRate->Text=s.sprintf("%d",SvrOpt[2]);
	SvrBuffSize->Text=s.sprintf("%d",SvrOpt[3]);
	SvrCycle->Text=s.sprintf("%d",SvrOpt[4]);
	NmeaCycle->Text=s.sprintf("%d",SvrOpt[5]);
	FileSwapMarginE->Text=s.sprintf("%d",FileSwapMargin);
	if (norm(NmeaPos,3)>0.0) {
		ecef2pos(NmeaPos,pos);
		NmeaPos1->Text=s.sprintf("%.8f",pos[0]*R2D);
		NmeaPos2->Text=s.sprintf("%.8f",pos[1]*R2D);
		NmeaPos3->Text=s.sprintf("%.3f",pos[2]);
	}
	else {
		NmeaPos1->Text="0.00000000";
		NmeaPos2->Text="0.00000000";
		NmeaPos3->Text="0.000";
	}
	TraceLevelS->ItemIndex=TraceLevel;
	NmeaReqT->Checked=NmeaReq;
	LocalDir->Text=LocalDirectory;
	ProxyAddr->Text=ProxyAddress;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnOkClick(TObject *Sender)
{
	double pos[3];
	SvrOpt[0]=DataTimeout->Text.ToInt();
	SvrOpt[1]=ConnectInterval->Text.ToInt();
	SvrOpt[2]=AvePeriodRate->Text.ToInt();
	SvrOpt[3]=SvrBuffSize->Text.ToInt();
	SvrOpt[4]=SvrCycle->Text.ToInt();
	SvrOpt[5]=NmeaCycle->Text.ToInt();
	FileSwapMargin=FileSwapMarginE->Text.ToInt();
	pos[0]=str2dbl(NmeaPos1->Text)*D2R;
	pos[1]=str2dbl(NmeaPos2->Text)*D2R;
	pos[2]=str2dbl(NmeaPos3->Text);
	if (norm(pos,3)>0.0) {
		pos2ecef(pos,NmeaPos);
	}
	else {
		for (int i=0;i<3;i++) NmeaPos[i]=0.0;
	}
	TraceLevel=TraceLevelS->ItemIndex;
	NmeaReq=NmeaReqT->Checked;
	LocalDirectory=LocalDir->Text;
	ProxyAddress=ProxyAddr->Text;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnPosClick(TObject *Sender)
{
	AnsiString s;
	RefDialog->RovPos[0]=str2dbl(NmeaPos1->Text);
	RefDialog->RovPos[1]=str2dbl(NmeaPos2->Text);
	RefDialog->RovPos[2]=str2dbl(NmeaPos3->Text);
	RefDialog->BtnLoad->Enabled=true;
	RefDialog->StaPosFile=StaPosFile;
	if (RefDialog->ShowModal()!=mrOk) return;
	NmeaPos1->Text=s.sprintf("%.8f",RefDialog->Pos[0]);
	NmeaPos2->Text=s.sprintf("%.8f",RefDialog->Pos[1]);
	NmeaPos3->Text=s.sprintf("%.3f",RefDialog->Pos[2]);
	StaPosFile=RefDialog->StaPosFile;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::BtnLocalDirClick(TObject *Sender)
{
	AnsiString dir=LocalDir->Text;
	if (!SelectDirectory("Local Directory for FTP/HTTP","",dir)) return;
	LocalDir->Text=dir;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::UpdateEnable(void)
{
	NmeaCycle->Enabled=NmeaReqT->Checked;
	NmeaPos1->Enabled=NmeaReqT->Checked;
	NmeaPos2->Enabled=NmeaReqT->Checked;
	NmeaPos3->Enabled=NmeaReqT->Checked;
	BtnPos->Enabled=NmeaReqT->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TSvrOptDialog::NmeaReqTClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------

