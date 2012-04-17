//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "convmain.h"
#include "convopt.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConvOptDialog *ConvOptDialog;
//---------------------------------------------------------------------------
static double str2dbl(AnsiString str)
{
	double val=0.0;
	sscanf(str.c_str(),"%lf",&val);
	return val;
}
//---------------------------------------------------------------------------
__fastcall TConvOptDialog::TConvOptDialog(TComponent* Owner)
	: TForm(Owner)
{
	AnsiString s;
	int glo=MAXPRNGLO,gal=MAXPRNGAL,qzs=MAXPRNQZS,cmp=MAXPRNCMP,nf=NFREQ;
	if (glo<=0) Nav2->Enabled=false;
	if (gal<=0) Nav3->Enabled=false;
	if (qzs<=0) Nav4->Enabled=false;
	if (cmp<=0) Nav6->Enabled=false;
	if (nf<3) Freq3->Enabled=false;
	if (nf<4) Freq4->Enabled=false;
	if (nf<5) Freq5->Enabled=false;
	if (nf<6) Freq6->Enabled=false;
	RnxVer->Clear();
	RnxVer->AddItem(s.sprintf("%.2f",RNX2VER),NULL);
	RnxVer->AddItem(s.sprintf("%.2f",RNX3VER),NULL);
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::FormShow(TObject *Sender)
{
	AnsiString s;
	RnxVer->ItemIndex=MainWindow->RnxVer;
	RnxFile->Checked=MainWindow->RnxFile;
	RnxCode->Text=MainWindow->RnxCode;
	RunBy->Text=MainWindow->RunBy;
	Marker->Text=MainWindow->Marker;
	MarkerNo->Text=MainWindow->MarkerNo;
	MarkerType->Text=MainWindow->MarkerType;
	Name0->Text=MainWindow->Name[0];
	Name1->Text=MainWindow->Name[1];
	Rec0->Text=MainWindow->Rec[0];
	Rec1->Text=MainWindow->Rec[1];
	Rec2->Text=MainWindow->Rec[2];
	Ant0->Text=MainWindow->Ant[0];
	Ant1->Text=MainWindow->Ant[1];
	Ant2->Text=MainWindow->Ant[2];
	AppPos0->Text=s.sprintf("%.4f",MainWindow->AppPos[0]);
	AppPos1->Text=s.sprintf("%.4f",MainWindow->AppPos[1]);
	AppPos2->Text=s.sprintf("%.4f",MainWindow->AppPos[2]);
	AntDel0->Text=s.sprintf("%.4f",MainWindow->AntDel[0]);
	AntDel1->Text=s.sprintf("%.4f",MainWindow->AntDel[1]);
	AntDel2->Text=s.sprintf("%.4f",MainWindow->AntDel[2]);
	Comment0->Text=MainWindow->Comment[0];
	Comment1->Text=MainWindow->Comment[1];
	RcvOption->Text=MainWindow->RcvOption;
	
	Nav1->Checked=MainWindow->NavSys&SYS_GPS;
	Nav2->Checked=MainWindow->NavSys&SYS_GLO;
	Nav3->Checked=MainWindow->NavSys&SYS_GAL;
	Nav4->Checked=MainWindow->NavSys&SYS_QZS;
	Nav5->Checked=MainWindow->NavSys&SYS_SBS;
	Nav6->Checked=MainWindow->NavSys&SYS_CMP;
	Obs1->Checked=MainWindow->ObsType&OBSTYPE_PR;
	Obs2->Checked=MainWindow->ObsType&OBSTYPE_CP;
	Obs3->Checked=MainWindow->ObsType&OBSTYPE_DOP;
	Obs4->Checked=MainWindow->ObsType&OBSTYPE_SNR;
	Freq1->Checked=MainWindow->FreqType&FREQTYPE_L1;
	Freq2->Checked=MainWindow->FreqType&FREQTYPE_L2;
	Freq3->Checked=MainWindow->FreqType&FREQTYPE_L5;
	Freq4->Checked=MainWindow->FreqType&FREQTYPE_L6;
	Freq5->Checked=MainWindow->FreqType&FREQTYPE_L7;
	Freq6->Checked=MainWindow->FreqType&FREQTYPE_L8;
	ExSats->Text=MainWindow->ExSats;
	TraceLevel->ItemIndex=MainWindow->TraceLevel;
	
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::BtnOkClick(TObject *Sender)
{
	MainWindow->RnxVer=RnxVer->ItemIndex;
	MainWindow->RnxFile=RnxFile->Checked;
	MainWindow->RnxCode=RnxCode->Text;
	MainWindow->RunBy=RunBy->Text;
	MainWindow->Marker=Marker->Text;
	MainWindow->MarkerNo=MarkerNo->Text;
	MainWindow->MarkerType=MarkerType->Text;
	MainWindow->Name[0]=Name0->Text;
	MainWindow->Name[1]=Name1->Text;
	MainWindow->Rec[0]=Rec0->Text;
	MainWindow->Rec[1]=Rec1->Text;
	MainWindow->Rec[2]=Rec2->Text;
	MainWindow->Ant[0]=Ant0->Text;
	MainWindow->Ant[1]=Ant1->Text;
	MainWindow->Ant[2]=Ant2->Text;
	MainWindow->AppPos[0]=str2dbl(AppPos0->Text);
	MainWindow->AppPos[1]=str2dbl(AppPos1->Text);
	MainWindow->AppPos[2]=str2dbl(AppPos2->Text);
	MainWindow->AntDel[0]=str2dbl(AntDel0->Text);
	MainWindow->AntDel[1]=str2dbl(AntDel1->Text);
	MainWindow->AntDel[2]=str2dbl(AntDel2->Text);
	MainWindow->Comment[0]=Comment0->Text;
	MainWindow->Comment[1]=Comment1->Text;
	MainWindow->RcvOption=RcvOption->Text;
	int navsys=0,obstype=0,freqtype=0;
	if (Nav1->Checked) navsys|=SYS_GPS;
	if (Nav2->Checked) navsys|=SYS_GLO;
	if (Nav3->Checked) navsys|=SYS_GAL;
	if (Nav4->Checked) navsys|=SYS_QZS;
	if (Nav5->Checked) navsys|=SYS_SBS;
	if (Nav6->Checked) navsys|=SYS_CMP;
	if (Obs1->Checked) obstype|=OBSTYPE_PR;
	if (Obs2->Checked) obstype|=OBSTYPE_CP;
	if (Obs3->Checked) obstype|=OBSTYPE_DOP;
	if (Obs4->Checked) obstype|=OBSTYPE_SNR;
	if (Freq1->Checked) freqtype|=FREQTYPE_L1;
	if (Freq2->Checked) freqtype|=FREQTYPE_L2;
	if (Freq3->Checked) freqtype|=FREQTYPE_L5;
	if (Freq4->Checked) freqtype|=FREQTYPE_L6;
	if (Freq5->Checked) freqtype|=FREQTYPE_L7;
	if (Freq6->Checked) freqtype|=FREQTYPE_L8;
	MainWindow->NavSys=navsys;
	MainWindow->ObsType=obstype;
	MainWindow->FreqType=freqtype;
	MainWindow->ExSats=ExSats->Text;
	MainWindow->TraceLevel=TraceLevel->ItemIndex;
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::RnxFileClick(TObject *Sender)
{
	UpdateEnable();	
}
//---------------------------------------------------------------------------
void __fastcall TConvOptDialog::UpdateEnable(void)
{
//	RnxCode->Enabled=RnxFile->Checked;
//	Label12->Enabled=RnxFile->Checked;
}
//---------------------------------------------------------------------------

