//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "rtklib.h"
#include "tcpoptdlg.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TTcpOptDialog *TcpOptDialog;

#define NTRIP_TIMEOUT	10000				// response timeout (ms)
#define NTRIP_CYCLE		50					// processing cycle (ms)
#define MAXSRCTBL		512000				// max source table size (bytes)
#define ENDSRCTBL		"ENDSOURCETABLE"	// end marker of table
#define MAXLINE			1024				// max line size (byte)

static char buff[MAXSRCTBL];

/* get source table -------------------------------------------------------*/
static char *getsrctbl(const char *path)
{
	static int lock=0;
	AnsiString s;
	stream_t str;
	char *p=buff,msg[MAXSTRMSG];
	int ns,stat,len=strlen(ENDSRCTBL);
	unsigned int tick=tickget();
	
	if (lock) return NULL; else lock=1;
	
	strinit(&str);
	if (!stropen(&str,STR_NTRIPCLI,STR_MODE_R,path)) {
		lock=0; 
		return NULL;
	}
	while(p<buff+MAXSRCTBL-1) {
		ns=strread(&str,p,buff+MAXSRCTBL-p-1); *(p+ns)='\0';
		if (p-len-3>buff&&strstr(p-len-3,ENDSRCTBL)) break;
		p+=ns;
		Sleep(NTRIP_CYCLE);
		stat=strstat(&str,msg);
		if (stat<0) break;
		if ((int)(tickget()-tick)>NTRIP_TIMEOUT) {
			break;
		}
	}
	strclose(&str);
	lock=0;
	return buff;
}
//---------------------------------------------------------------------------
__fastcall TTcpOptDialog::TTcpOptDialog(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::FormShow(TObject *Sender)
{
	char buff[2048],*p,*q;
	char *addr,*port="",*mntpnt="",*user="",*passwd="",*str="";
	char *ti[]={"TCP Server Options ","TCP Client Options",
			    "NTRIP Server Options","NTRIP Client Options"};
	
	strcpy(buff,Path.c_str());
	if ((p=strchr(buff,'/'))) {
		if ((q=strchr(p+1,':'))) {
			*q='\0'; str=q+1;
		}
		*p='\0'; mntpnt=p+1;
	}
	if ((p=strchr(buff,'@'))) {
		*p++='\0';
		if ((q=strchr(buff,':'))) {
			*q='\0'; passwd=q+1;
		}
		*q='\0'; user=buff;
	}
	else p=buff;
	if ((q=strchr(p,':'))) {
		*q='\0'; port=q+1;
	}
	addr=p;
	Addr->Text=addr;
	Port->Text=port;
	MntPnt->Text=mntpnt;
	User->Text=user;
	Passwd->Text=passwd;
	Str->Text=str;
	Addr->Enabled=Opt>=1;
	MntPnt->Enabled=Opt>=2;
	User->Enabled=Opt==3;
	Passwd->Enabled=Opt>=2;
	Str->Enabled=Opt==2;
	LabelAddr->Caption=Opt>=2?"NTRIP Caster Host":"TCP Server Address";
	LabelAddr->Enabled=Opt>=1;
	LabelMntPnt->Enabled=Opt>=2;
	LabelUser->Enabled=Opt==3;
	LabelPasswd->Enabled=Opt>=2;
	LabelStr->Enabled=Opt==2;
	Caption=ti[Opt];
	Addr->Items->Clear();
	MntPnt->Items->Clear();
	
	for (int i=0;i<MAXHIST;i++) {
		if (History[i]!="") Addr->Items->Add(History[i]);
	}
	for (int i=0;i<MAXHIST;i++) {
		if (MntpHist[i]!="") MntPnt->Items->Add(MntpHist[i]);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::BtnOkClick(TObject *Sender)
{
	AnsiString s;
	Path=s.sprintf("%s:%s@%s:%s/%s:%s",User->Text.c_str(),Passwd->Text.c_str(),
			Addr->Text.c_str(),Port->Text.c_str(),MntPnt->Text.c_str(),
			Str->Text.c_str());
	AddHist(Addr,History);
	AddHist(MntPnt,MntpHist);
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::BtnGetListClick(TObject *Sender)
{
	AnsiString addr,text,item[3];
	char buff[MAXLINE],*p,*q,*r,*srctbl;
	int i,n;
	
	if (Addr->Text!="") return;
	addr=Addr->Text+":"+Port->Text;
	
	if (!(srctbl=getsrctbl(addr.c_str()))) return;
	
	text=MntPnt->Text; MntPnt->Clear(); MntPnt->Text=text;
	for (p=srctbl;*p;p=q+1) {
		if (!(q=strchr(p,'\n'))) break;
		n=q-p<MAXLINE-1?q-p:MAXLINE-1;
		strncpy(buff,p,n); buff[n]='\0';
		if (strncmp(buff,"STR",3)) continue;
		for (i=0,r=strtok(buff,";");i<3&&p;i++,r=strtok(NULL,";")) item[i]=r;
		MntPnt->AddItem(item[1],NULL);
	}
}
//---------------------------------------------------------------------------
void __fastcall TTcpOptDialog::AddHist(TComboBox *list, AnsiString *hist)
{
	for (int i=0;i<MAXHIST;i++) {
		if (list->Text!=hist[i]) continue;
		for (int j=i+1;j<MAXHIST;j++) hist[j-1]=hist[j];
		hist[MAXHIST-1]="";
	}
	for (int i=MAXHIST-1;i>0;i--) hist[i]=hist[i-1];
	hist[0]=list->Text;
	
	list->Clear();
	for (int i=0;i<MAXHIST;i++) {
		if (hist[i]!="") list->Items->Add(hist[i]);
	}
}
//---------------------------------------------------------------------------

