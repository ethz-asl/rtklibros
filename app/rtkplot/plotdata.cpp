//---------------------------------------------------------------------------
// plotdata : rtkplot data functions
//---------------------------------------------------------------------------
#include "rtklib.h"
#include "plotmain.h"
#include "mapdlg.h"

// read solutions -----------------------------------------------------------
void __fastcall TPlot::ReadSol(TStrings *files, int sel)
{
    solbuf_t sol={0};
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i,n=0;
    char *paths[MAXNFILE]={""};
    
    trace(3,"ReadSol: sel=%d\n",sel);
    
    if (files->Count<=0) return;
    
    ReadWaitStart();
    
    for (i=0;i<files->Count&&n<MAXNFILE;i++) {
        paths[n++]=files->Strings[i].c_str();
    }
    TimeSpan(&ts,&te,&tint);
    
    ShowMsg(s.sprintf("reading %s...",paths[0]));
    ShowLegend(NULL);
    
    if (!readsolt(paths,n,ts,te,tint,0,&sol)) {
        ShowMsg(s.sprintf("no solution data : %s...",paths[0]));
        ShowLegend(NULL);
        ReadWaitEnd();
        return;
    }
    freesolbuf(SolData+sel);
    SolData[sel]=sol;
    
    if (SolFiles[sel]!=files) {
        SolFiles[sel]->Assign(files);
    }
    Caption="";
    
    ReadSolStat(files,sel);
    
    for (i=0;i<2;i++) {
        if (SolFiles[i]->Count==0) continue;
        Caption=Caption+SolFiles[i]->Strings[0]+(SolFiles[i]->Count>1?"... ":" ");
    }
    BtnSol12->Down=False;
    if (sel==0) BtnSol1->Down=true;
    else        BtnSol2->Down=true;
    
    if (sel==0||SolData[0].n<=0) {
        time2gpst(SolData[sel].data[0].time,&Week);
        UpdateOrigin();
    }
    SolIndex[0]=SolIndex[1]=ObsIndex=0;
    
    if (PlotType>PLOT_NSAT) {
        UpdateType(PLOT_TRK);
    }
    else {
        UpdatePlotType();
    }
    FitTime();
    if (AutoScale&&PlotType<=PLOT_SOLA) {
        FitRange(1);
    }
    else {
        SetRange(1,YRange);
    }
    UpdateTime();
    UpdatePlot();
    
    ReadWaitEnd();
}
// read solution status -----------------------------------------------------
void __fastcall TPlot::ReadSolStat(TStrings *files, int sel)
{
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i,n=0;
    char *paths[MAXNFILE]={""},id[32];
    
    trace(3,"ReadSolStat\n");
    
    freesolstatbuf(SolStat+sel);
    
    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<files->Count&&n<MAXNFILE;i++) {
        paths[n++]=files->Strings[i].c_str();
    }
    ShowMsg(s.sprintf("reading %s...",paths[0]));
    ShowLegend(NULL);
    
    readsolstatt(paths,n,ts,te,tint,SolStat+sel);
    
    UpdateSatList();
}
// read observation data ----------------------------------------------------
void __fastcall TPlot::ReadObs(TStrings *files)
{
    obs_t obs={0};
    nav_t nav={0};
    sta_t sta={0};
    AnsiString s;
    int i,nobs;
    
    trace(3,"ReadObs\n");
    
    if (files->Count<=0) return;
    
    ReadWaitStart();
    ShowLegend(NULL);
    
    if ((nobs=ReadObsRnx(files,&obs,&nav,&sta))<=0) {
        ReadWaitEnd();
        return;
    }
    freeobs(&Obs);
    freenav(&Nav,0xFF);
    Obs=obs;
    Nav=nav;
    Sta=sta;
    UpdateObs(nobs);
    
    if (ObsFiles!=files) {
        ObsFiles->Assign(files);
    }
    NavFiles->Clear();
    Caption=s.sprintf("%s%s",files->Strings[0].c_str(),files->Count>1?"...":"");
    
    BtnSol1->Down=true;
    time2gpst(Obs.data[0].time,&Week);
    SolIndex[0]=SolIndex[1]=ObsIndex=0;
    
    if (PlotType<PLOT_OBS||PLOT_DOP<PlotType) {
        UpdateType(PLOT_OBS);
    }
    else {
        UpdatePlotType();
    }
    FitTime();
    UpdateTime();
    UpdatePlot();
    
    ReadWaitEnd();
}
// read observation data rinex ----------------------------------------------
int __fastcall TPlot::ReadObsRnx(TStrings *files, obs_t *obs, nav_t *nav,
                                 sta_t *sta)
{
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i;
    char *obsfile,navfile[1024],*p;
    
    trace(3,"ReadObsRnx\n");
    
    TimeSpan(&ts,&te,&tint);
    
    for (i=0;i<files->Count;i++) {
        obsfile=files->Strings[i].c_str();
        
        ShowMsg(s.sprintf("reading obs data... %s",obsfile));
        Application->ProcessMessages();
        
        if (readrnxt(obsfile,1,ts,te,tint,obs,nav,sta)<0) {
            ShowMsg("error: insufficient memory");
            return -1;
        }
    }
    ShowMsg("reading nav data...");
    Application->ProcessMessages();
    
    for (i=0;i<files->Count;i++) {
        strcpy(navfile,files->Strings[i].c_str());
        
        if (!(p=strrchr(navfile,'.'))) continue;
        
        if (!strcmp(p,".obs")||!strcmp(p,".OBS")) {
            strcpy(p,".nav" ); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
            strcpy(p,".gnav"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
            strcpy(p,".hnav"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
            strcpy(p,".qnav"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
        }
        else if (!strcmp(p+3,"o" )||!strcmp(p+3,"d" )||
                 !strcmp(p+3,"O" )||!strcmp(p+3,"D" )) {
            strcpy(p+3,"N"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
            strcpy(p+3,"G"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
            strcpy(p+3,"H"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
            strcpy(p+3,"Q"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
            strcpy(p+3,"P"); readrnxt(navfile,1,ts,te,tint,NULL,nav,NULL);
        }
    }
    if (obs->n<=0) {
        ShowMsg(s.sprintf("no observation data: %s...",files->Strings[0].c_str()));
        freenav(nav,0xFF);
        return 0;
    }
    uniqnav(nav);
    return sortobs(obs);
}
// read navigation data -----------------------------------------------------
void __fastcall TPlot::ReadNav(TStrings *files)
{
    AnsiString s;
    gtime_t ts,te;
    double tint;
    int i;
    
    trace(3,"ReadNav\n");
    
    if (files->Count<=0) return;
    
    ReadWaitStart();
    ShowLegend(NULL);
    
    TimeSpan(&ts,&te,&tint);
    
    freenav(&Nav,0xFF);
    
    ShowMsg("reading nav data...");
    Application->ProcessMessages();
    
    for (i=0;i<files->Count;i++) {
        readrnxt(files->Strings[i].c_str(),1,ts,te,tint,NULL,&Nav,NULL);
    }
    uniqnav(&Nav);
    
    if (Nav.n<=0&&Nav.ng<=0&&Nav.ns<=0) {
        ShowMsg(s.sprintf("no nav message: %s...",files->Strings[0].c_str()));
        ReadWaitEnd();
        return;
    }
    if (NavFiles!=files) {
        NavFiles->Assign(files);
    }
    UpdateObs(NObs);
    UpdatePlot();
    
    ReadWaitEnd();
}
// read elevation mask data -------------------------------------------------
void __fastcall TPlot::ReadElMaskData(AnsiString file)
{
    AnsiString s;
    FILE *fp;
    double az0=0.0,el0=0.0,az1,el1;
    int i,j;
    char buff[256];
    
    trace(3,"ReadElMaskData\n");
    
    for (i=0;i<=360;i++) ElMaskData[i]=0.0;
    
    if (!(fp=fopen(file.c_str(),"r"))) {
        ShowMsg(s.sprintf("no el mask data: %s...",file.c_str()));
        ShowLegend(NULL);
        return;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (buff[0]=='%'||sscanf(buff,"%lf %lf",&az1,&el1)<2) continue;
        
        if (az0<az1&&az1<=360.0&&0.0<=el1&&el1<=90.0) {
            
            for (j=(int)az0;j<=(int)az1;j++) ElMaskData[j]=el0*D2R;
        }
        az0=az1; el0=el1;
    }
    fclose(fp);
    UpdatePlot();
}
// read map image data ------------------------------------------------------
void __fastcall TPlot::ReadMapData(AnsiString file)
{
    TJPEGImage *image=new TJPEGImage;
    AnsiString s;
    
    trace(3,"ReadMapData\n");
    
    try {
        image->LoadFromFile(file);
    }
    catch (Exception &exception) {
        ShowMsg(s.sprintf("map file read error: %s",file));
        ShowLegend(NULL);
        return;
    }
    MapImage->Assign(image);
    MapImageFile=file;
    MapSize[0]=MapImage->Width;
    MapSize[1]=MapImage->Height;
    delete image;
    
    ReadMapTag(file);
    
    BtnShowMap->Down=true;
    
    MapAreaDialog->UpdateField();
    UpdatePlot();
}
// read map tag data --------------------------------------------------------
void __fastcall TPlot::ReadMapTag(AnsiString file)
{
    FILE *fp;
    char buff[1024],*p;
    
    trace(3,"ReadMapTag\n");
    
    file=file+".tag";
    
    if (!(fp=fopen(file.c_str(),"r"))) return;
    
    MapScaleX=MapScaleY=1.0;
    MapScaleEq=0;
    MapLat=MapLon=0.0;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (buff[0]=='\0'||buff[0]=='%'||buff[0]=='#') continue;
        if (!(p=strchr(buff,'='))) continue;
        *p='\0';
        if      (strstr(buff,"scalex" )==buff) sscanf(p+1,"%lf",&MapScaleX );
        else if (strstr(buff,"scaley" )==buff) sscanf(p+1,"%lf",&MapScaleY );
        else if (strstr(buff,"scaleeq")==buff) sscanf(p+1,"%d" ,&MapScaleEq);
        else if (strstr(buff,"lat"    )==buff) sscanf(p+1,"%lf",&MapLat    );
        else if (strstr(buff,"lon"    )==buff) sscanf(p+1,"%lf",&MapLon    );
    }
    fclose(fp);
}
// read map path data -------------------------------------------------------
void __fastcall TPlot::ReadMapPath(AnsiString file)
{
    FILE *fp;
    int n=0;
    char buff[1024];
    double pos[3];
    
    if (!(fp=fopen(file.c_str(),"r"))) return;
    
    while (fgets(buff,sizeof(buff),fp)&&n<MAXMAPPATH) {
        
        if (sscanf(buff,"%lf %lf %lf",pos,pos+1,pos+2)!=3) continue;
        pos[0]*=D2R;
        pos[1]*=D2R;
        pos2ecef(pos,MapPath+n*3);
        n++;
    }
    fclose(fp);
    
    NMapPath=n;
    
    BtnShowPoint->Down=true;
    
    UpdatePlot();
}
// read station position data -----------------------------------------------
void __fastcall TPlot::ReadStaPos(const char *file, const char *sta,
                                  double *rr)
{
    FILE *fp;
    char buff[256],code[256],name[256];
    double pos[3];
    int sinex=0;
    
    if (!(fp=fopen(file,"r"))) return;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (strstr(buff,"%=SNX")==buff) sinex=1;
        if (buff[0]=='%'||buff[1]=='#') continue;
        if (sinex) {
            if (strlen(buff)<68||strncmp(buff+14,sta,4)) continue;
            if (!strncmp(buff+7,"STAX",4)) rr[0]=str2num(buff,47,21);
            if (!strncmp(buff+7,"STAY",4)) rr[1]=str2num(buff,47,21);
            if (!strncmp(buff+7,"STAZ",4)) {
                rr[2]=str2num(buff,47,21);
                break;
            }
        }
        else {
            if (sscanf(buff,"%lf %lf %lf %s",pos,pos+1,pos+2,code)<4) continue;
            if (strcmp(code,sta)) continue;
            pos[0]*=D2R;
            pos[1]*=D2R;
            pos2ecef(pos,rr);
            break;
        }
    }
	fclose(fp);
}
// connect to external sources ----------------------------------------------
void __fastcall TPlot::Connect(void)
{
    AnsiString s;
    char *cmd,*path,buff[MAXSTRPATH],*name[2]={"",""},*p;
    int i,mode=STR_MODE_R;
    
    trace(3,"Connect\n");
    
    if (ConnectState) return;
    
    for (i=0;i<2;i++) {
        if      (RtStream[i]==STR_NONE    ) continue;
        else if (RtStream[i]==STR_SERIAL  ) path=StrPaths[i][0].c_str();
        else if (RtStream[i]==STR_FILE    ) path=StrPaths[i][2].c_str();
        else if (RtStream[i]<=STR_NTRIPCLI) path=StrPaths[i][1].c_str();
        else continue;
        
        if (RtStream[i]==STR_FILE||!SolData[i].cyclic||SolData[i].nmax!=RtBuffSize+1) {
            Clear();
            initsolbuf(SolData+i,1,RtBuffSize+1);
        }
        if (RtStream[i]==STR_SERIAL) mode|=STR_MODE_W;
        
        strcpy(buff,path);
        if ((p=strstr(buff,"::"))) *p='\0';
        if ((p=strstr(buff,"/:"))) *p='\0';
        if ((p=strstr(buff,"@"))) name[i]=p+1; else name[i]=buff;
        
        if (!stropen(Stream+i,RtStream[i],mode,path)) {
            ShowMsg(s.sprintf("connect error: %s",name));
            ShowLegend(NULL);
            trace(1,"stream open error: ch=%d type=%d path=%s\n",i+1,RtStream[i],path);
            continue;
        }
        strsettimeout(Stream+i,RtTimeOutTime,RtReConnTime);
        
        if (StrCmdEna[i][0]) {
            cmd=StrCmds[i][0].c_str();
            strwrite(Stream+i,(unsigned char *)cmd,strlen(cmd));
        }
        ConnectState=1;
    }
    if (!ConnectState) return;
    
    if (Title!="") Caption=Title;
    else Caption=s.sprintf("CONNECT %s %s",name[0],name[1]);
    
    BtnConnect->Down=true;
    BtnSol1   ->Down=*name[0];
    BtnSol2   ->Down=*name[1];
    BtnSol12  ->Down=false;
    UpdateTime();
    UpdatePlot();
}
// disconnect from external sources -----------------------------------------
void __fastcall TPlot::Disconnect(void)
{
    AnsiString s;
    char *cmd;
    int i;
    
    trace(3,"Disconnect\n");
    
    if (!ConnectState) return;
    
    ConnectState=0;
    
    for (i=0;i<2;i++) {
        if (StrCmdEna[i][1]) {
            cmd=StrCmds[i][1].c_str();
            strwrite(Stream+i,(unsigned char *)cmd,strlen(cmd));
        }
        strclose(Stream+i);
    }
    if (strstr(Caption.c_str(),"CONNECT")) {
        Caption=s.sprintf("DISCONNECT%s",Caption.c_str()+7);
    }
    UpdateTime();
    UpdatePlot();
}
// check observation data types ---------------------------------------------
int __fastcall TPlot::CheckObs(AnsiString file)
{
    char *p;
    
    trace(3,"CheckObs\n");
    
    if (!(p=strrchr(file.c_str(),'.'))) return 0;
    if (!strcmp(p,".z")||!strcmp(p,".gz")||!strcmp(p,".zip")||
        !strcmp(p,".Z")||!strcmp(p,".GZ")||!strcmp(p,".ZIP")) {
        return *(p-1)=='o'||*(p-1)=='O'||*(p-1)=='d'||*(p-1)=='D';
    }
    return !strcmp(p,".obs")||!strcmp(p,".OBS")||
           !strcmp(p+3,"o" )||!strcmp(p+3,"O" )||
           !strcmp(p+3,"d" )||!strcmp(p+3,"D" );
}
// update observation data index, azimuth/elevation, satellite list ---------
void __fastcall TPlot::UpdateObs(int nobs)
{
    AnsiString s;
    prcopt_t opt=prcopt_default;
    gtime_t time;
    sol_t sol={0};
    double pos[3],rr[3],e[3],azel[MAXOBS*2]={0},rs[6],dts[2],var;
    int i,j,k,svh,per,per_=-1;
    char msg[128];
    
    trace(3,"UpdateObs\n");
    
    delete [] IndexObs; IndexObs=NULL;
    delete [] Az; Az=NULL;
    delete [] El; El=NULL;
    NObs=0;
    if (nobs<=0) return;
    
    IndexObs=new int[nobs];
    Az=new double[Obs.n];
    El=new double[Obs.n];
    
    opt.err[0]=900.0;
    
    ReadWaitStart();
    ShowLegend(NULL);
    
    for (i=0;i<Obs.n;i=j) {
        time=Obs.data[i].time;
        for (j=i;j<Obs.n;j++) {
            if (timediff(Obs.data[j].time,time)>TTOL) break;
        }
        IndexObs[NObs++]=i;
        
        if (Nav.n<=0&&Nav.ng<=0&&Nav.ns<=0) {
            for (k=0;k<j-i;k++) Az[i+k]=El[i+k]=0.0;
            continue;
        }
        if (RcvPos==0) {
            pntpos(Obs.data+i,j-i,&Nav,&opt,&sol,azel,NULL,msg);
        }
        else {
            if (RcvPos==1) { // lat/lon/height
                for (k=0;k<3;k++) pos[k]=OOPos[k];
                pos2ecef(pos,rr);
            }
            else { // rinex header position
                for (k=0;k<3;k++) rr[k]=Sta.pos[k];
                ecef2pos(rr,pos);
            }
            for (k=0;k<j-i;k++) {
                azel[k*2]=azel[1+k*2]=0.0;
                if (!satpos(time,time,Obs.data[i+k].sat,EPHOPT_BRDC,&Nav,rs,dts,
                            &var,&svh)) continue;
                if (geodist(rs,rr,e)>0.0) satazel(pos,e,azel+k*2);
            }
        }
        for (k=0;k<j-i;k++) {
            Az[i+k]=azel[  k*2];
            El[i+k]=azel[1+k*2];
            if (Az[i+k]<0.0) Az[i+k]+=2.0*PI;
        }
        per=(i+1)*100/Obs.n;
        if (per!=per_) {
            ShowMsg(s.sprintf("updating azimuth/elevation... (%d%%)",(per_=per)));
            Application->ProcessMessages();
        }
    }
    IndexObs[NObs]=Obs.n;
    
    UpdateSatList();
    
    ReadWaitEnd();
}
// set connect path ---------------------------------------------------------
void __fastcall TPlot::ConnectPath(const char *path, int ch)
{
    const char *p;
    
    trace(3,"ConnectPath: path=%s ch=%d\n",path,ch);
    
    RtStream[ch]=STR_NONE;
    
    if (!(p=strstr(path,"://"))) return;
    if      (!strncmp(path,"serial",6)) RtStream[ch]=STR_SERIAL;
    else if (!strncmp(path,"tcpsvr",6)) RtStream[ch]=STR_TCPSVR;
    else if (!strncmp(path,"tcpcli",6)) RtStream[ch]=STR_TCPCLI;
    else if (!strncmp(path,"ntrip", 5)) RtStream[ch]=STR_NTRIPCLI;
    else if (!strncmp(path,"file",  4)) RtStream[ch]=STR_FILE;
    else return;
    
    StrPaths[ch][1]=p+3;
    RtFormat[ch]=SOLF_LLH;
    RtTimeForm=0;
    RtDegForm =0;
    RtFieldSep=" ";
    RtTimeOutTime=0;
    RtReConnTime =10000;
    
    BtnShowTrack->Down=true;
    BtnFixHoriz ->Down=true;
    BtnFixVert  ->Down=true;
}
// clear data ---------------------------------------------------------------
void __fastcall TPlot::Clear(void)
{
    AnsiString s;
    double ep[]={2010,1,1,0,0,0};
    int i;
    
    trace(3,"Clear\n");
    
    Week=NObs=0;
    
    for (i=0;i<2;i++) {
        freesolbuf(SolData+i);
        free(SolStat[i].data);
        SolStat[i].n=0;
        SolStat[i].data=NULL;
    }
    freeobs(&Obs);
    freenav(&Nav,0xFF);
    delete [] IndexObs;
    delete [] Az;
    delete [] El;
    IndexObs=NULL;
    Az=NULL;
    El=NULL;
    SolFiles[0]->Clear();
    SolFiles[1]->Clear();
    ObsFiles->Clear();
    NavFiles->Clear();
    NObs=0;
    SolIndex[0]=SolIndex[1]=ObsIndex=0;
    
    for (i=0;i<3;i++) {
        TimeEna[i]=0;
    }
    TimeStart=TimeEnd=epoch2time(ep);
    BtnAnimate->Down=false;
    
    if (PlotType>PLOT_NSAT) {
        UpdateType(PLOT_TRK);
    }
    if (!ConnectState) {
        initsolbuf(SolData  ,0,0);
        initsolbuf(SolData+1,0,0);
        Caption=Title!=""?Title:s.sprintf("%s ver.%s",PRGNAME,VER_RTKLIB);
    }
    else {
        initsolbuf(SolData  ,1,RtBuffSize+1);
        initsolbuf(SolData+1,1,RtBuffSize+1);
    }
    UpdateTime();
    UpdatePlot();
}
// reload data --------------------------------------------------------------
void __fastcall TPlot::Reload(void)
{
    trace(3,"Reload\n");
    
    ReadObs(ObsFiles);
    ReadNav(NavFiles);
    ReadSol(SolFiles[0],0);
    ReadSol(SolFiles[1],1);
}
// read wait start ----------------------------------------------------------
void __fastcall TPlot::ReadWaitStart(void)
{
    MenuFile->Enabled=false;
    MenuEdit->Enabled=false;
    MenuView->Enabled=false;
    MenuHelp->Enabled=false;
    Panel1->Enabled=false;
    Disp->Enabled=false;
    Screen->Cursor=crHourGlass;
}
// read wait end ------------------------------------------------------------
void __fastcall TPlot::ReadWaitEnd(void)
{
    MenuFile->Enabled=true;
    MenuEdit->Enabled=true;
    MenuView->Enabled=true;
    MenuHelp->Enabled=true;
    Panel1->Enabled=true;
    Disp->Enabled=true;
    Screen->Cursor=crDefault;
}
// --------------------------------------------------------------------------
