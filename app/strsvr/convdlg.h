//---------------------------------------------------------------------------
#ifndef convdlgH
#define convdlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TConvDialog : public TForm
{
__published:
	TPanel *Panel1;
	TLabel *Label1;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	TGroupBox *GroupBox1;
	TCheckBox *SatSys1;
	TCheckBox *SatSys2;
	TCheckBox *SatSys3;
	TCheckBox *SatSys4;
	TCheckBox *SatSys5;
	TCheckBox *SatSys6;
	TEdit *StationId;
	TEdit *AntPos1;
	TEdit *AntPos2;
	TEdit *AntPos3;
	TEdit *AntOff1;
	TEdit *AntOff2;
	TEdit *AntOff3;
	TEdit *AntType;
	TGroupBox *GroupBox2;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TComboBox *IntevalObs;
	TComboBox *IntervalEph;
	TComboBox *IntervalAnt;
	TButton *BtnOk;
	TButton *BtnCancel;
	TCheckBox *Conversion;
	TComboBox *InFormat;
	TLabel *LabelConv;
	TComboBox *OutFormat;
	TButton *BtnPos;
	void __fastcall BtnOkClick(TObject *Sender);
	void __fastcall ConversionClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:
	void __fastcall UpdateEnable(void);
public:
	__fastcall TConvDialog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TConvDialog *ConvDialog;
//---------------------------------------------------------------------------
#endif
