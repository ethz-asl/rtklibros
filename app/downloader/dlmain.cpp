//---------------------------------------------------------------------------
#include <vcl.h>
#include <vcl\inifiles.hpp>
#pragma hdrstop

#include "rtklib.h"
#include "dlmain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnExitClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY1UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY1->Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY1->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY1->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH1UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    int hms[3]={0},sec,p=TimeH1->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH1->Text.c_str(),"%d:%d",hms,hms+1);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH1->Text=s.sprintf("%02d:%02d",sec/3600,(sec%3600)/60);
    TimeH1->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeY2UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    double ep[]={2000,1,1,0,0,0};
    int p=TimeY2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeY2->Text.c_str(),"%lf/%lf/%lf",ep,ep+1,ep+2);
    if (4<p&&p<8) {
        ep[1]+=ud;
        if (ep[1]<=0) {ep[0]--; ep[1]+=12;}
        else if (ep[1]>12) {ep[0]++; ep[1]-=12;}
    }
    else if (p>7||p==0) ep[2]+=ud; else ep[0]+=ud;
    time2epoch(epoch2time(ep),ep);
    TimeY2->Text=s.sprintf("%04.0f/%02.0f/%02.0f",ep[0],ep[1],ep[2]);
    TimeY2->SelStart=p>7||p==0?10:(p>4?7:4);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::TimeH2UDChangingEx(TObject *Sender, bool &AllowChange,
      short NewValue, TUpDownDirection Direction)
{
    AnsiString s;
    int hms[3]={0},sec,p=TimeH2->SelStart,ud=Direction==updUp?1:-1;
    
    sscanf(TimeH2->Text.c_str(),"%d:%d",hms,hms+1);
    if (p>5||p==0) hms[2]+=ud; else if (p>2) hms[1]+=ud; else hms[0]+=ud;
    sec=hms[0]*3600+hms[1]*60+hms[2];
    if (sec<0) sec+=86400; else if (sec>=86400) sec-=86400;
    TimeH2->Text=s.sprintf("%02d:%02d",sec/3600,(sec%3600)/60);
    TimeH2->SelStart=p>5||p==0?8:(p>2?5:2);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnLocalDirClick(TObject *Sender)
{
	AnsiString dir=LocalDir->Text;
	if (!SelectDirectory("Local Directory","",dir)) return;
	LocalDir->Text=dir;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnStationsClick(TObject *Sender)
{
	;	
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnLogClick(TObject *Sender)
{
	;	
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::BtnDownloadClick(TObject *Sender)
{
	;	
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DataTypeSChange(TObject *Sender)
{
	;	
}
//---------------------------------------------------------------------------

