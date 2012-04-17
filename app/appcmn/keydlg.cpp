//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "keydlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TKeyDialog *KeyDialog;
//---------------------------------------------------------------------------
__fastcall TKeyDialog::TKeyDialog(TComponent* Owner)
	: TForm(Owner)
{
	Flag=0;
}
//---------------------------------------------------------------------------
void __fastcall TKeyDialog::FormShow(TObject *Sender)
{
	Label29->Visible=Flag>=1;
	Label30->Visible=Flag>=1;
	Label31->Visible=Flag>=2;
	Label32->Visible=Flag>=2;
}
//---------------------------------------------------------------------------
void __fastcall TKeyDialog::BtnOkClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

