//---------------------------------------------------------------------------
#ifndef dlmainH
#define dlmainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <FileCtrl.hpp>

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:
	TPanel *Panel1;
	TLabel *Label1;
	TComboBox *DataTypeS;
	TListBox *DataListS;
	TPanel *Panel2;
	TEdit *AddressE;
	TLabel *Label2;
	TLabel *Label4;
	TEdit *LoginUser;
	TLabel *Label5;
	TEdit *Passwd;
	TLabel *Label6;
	TEdit *LocalDir;
	TButton *BtnLocalDir;
	TPanel *Panel3;
	TButton *BtnStations;
	TButton *BtnLog;
	TButton *BtnDownload;
	TButton *BtnExit;
	TPanel *Panel4;
	TLabel *Label7;
	TMemo *Memo1;
	TGroupBox *GroupBox1;
	TCheckBox *UnzipC;
	TCheckBox *SkipExist;
	TCheckBox *AbortOnErr;
	TCheckBox *ProxyEna;
	TEdit *Proxy;
	TButton *Button1;
	TGroupBox *GroupBox2;
	TEdit *TimeY1;
	TUpDown *TimeY1UD;
	TEdit *TimeH1;
	TUpDown *TimeH1UD;
	TSpeedButton *BtnTime1;
	TLabel *Label9;
	TEdit *TimeY2;
	TUpDown *TimeY2UD;
	TEdit *TimeH2;
	TUpDown *TimeH2UD;
	TSpeedButton *BtnTime2;
	void __fastcall BtnExitClick(TObject *Sender);
	void __fastcall TimeY1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeH1UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeY2UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall TimeH2UDChangingEx(TObject *Sender, bool &AllowChange,
          short NewValue, TUpDownDirection Direction);
	void __fastcall BtnLocalDirClick(TObject *Sender);
	void __fastcall BtnStationsClick(TObject *Sender);
	void __fastcall BtnLogClick(TObject *Sender);
	void __fastcall BtnDownloadClick(TObject *Sender);
	void __fastcall DataTypeSChange(TObject *Sender);
private:
public:
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
