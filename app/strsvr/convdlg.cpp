//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "rtklib.h"
#include "convdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConvDialog *ConvDialog;
//---------------------------------------------------------------------------
__fastcall TConvDialog::TConvDialog(TComponent* Owner)
	: TForm(Owner)
{
	int i;
	for (i=0;i<=MAXRCVFMT;i++) {
		InFormat->Items->Add(formatstrs[i]);
	}
	InFormat->ItemIndex=0;
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::FormShow(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::BtnOkClick(TObject *Sender)
{
	;	
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::ConversionClick(TObject *Sender)
{
	UpdateEnable();
}
//---------------------------------------------------------------------------
void __fastcall TConvDialog::UpdateEnable(void)
{
	InFormat ->Enabled=Conversion->Checked;
	OutFormat->Enabled=Conversion->Checked;
	LabelConv->Enabled=Conversion->Checked;
	GroupBox1->Enabled=Conversion->Checked;
	GroupBox2->Enabled=Conversion->Checked;
}
//---------------------------------------------------------------------------

