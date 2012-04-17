/*------------------------------------------------------------------------------
* preceph.c : precise ephemeris and clock functions
*
*          Copyright (C) 2007-2010 by T.TAKASU, All rights reserved.
*
* references :
*     [1] S.Hilla, The Extended Standard Product 3 Orbit Format (SP3-c),
*         12 February, 2007
*     [2] J.Ray, W.Gurtner, RINEX Extensions to Handle Clock Information,
*         27 August, 1998
*     [3] D.D.McCarthy, IERS Technical Note 21, IERS Conventions 1996, July 1996
*     [4] D.A.Vallado, Fundamentals of Astrodynamics and Applications 2nd ed,
*         Space Technology Library, 2004
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:48:06 $
* history : 2009/01/18 1.0  new
*           2009/01/31 1.1  fix bug on numerical error to read sp3a ephemeris
*           2009/05/15 1.2  support glonass,galileo,qzs
*           2009/12/11 1.3  support wild-card expansion of file path
*           2010/07/21 1.4  added api:
*                               eci2ecef(),sunmoonpos(),peph2pos(),satantoff(),
*                               readdcb()
*                           changed api:
*                               readsp3()
*                           deleted api:
*                               eph2posp()
*           2010/09/09 1.5  fix problem when precise clock outage
*           2011/01/23 1.6  support qzss satellite code
*-----------------------------------------------------------------------------*/
#include "rtklib.h"

static const char rcsid[]="$Id:$";

#define SQR(x)      ((x)*(x))

#define AU          149597870691.0  /* 1 AU (m) */
#define AS2R        (D2R/3600.0)    /* arc sec to radian */

#define NMAX        10              /* order of polynomial interpolation */
#define MAXDTE      900.0           /* max time difference to ephem time (s) */
#define EXTERR_CLK  1E-3            /* extrapolation error for clock (m/s) */
#define EXTERR_EPH  5E-7            /* extrapolation error for ephem (m/s^2) */

/* satellite code to satellite system ----------------------------------------*/
static int code2sys(char code)
{
    if (code=='G'||code==' ') return SYS_GPS;
    if (code=='R') return SYS_GLO;
    if (code=='E') return SYS_GAL; /* extension to sp3-c */
    if (code=='J') return SYS_QZS; /* extension to sp3-c */
    if (code=='C') return SYS_CMP; /* extension to sp3-c */
    return SYS_NONE;
}
/* time to day and sec -------------------------------------------------------*/
static double time2sec(gtime_t time, gtime_t *day)
{
    double ep[6],sec;
    time2epoch(time,ep);
    sec=ep[3]*3600.0+ep[4]*60.0+ep[5];
    ep[3]=ep[4]=ep[5]=0.0;
    *day=epoch2time(ep);
    return sec;
}
/* coordinate rotation matrix ------------------------------------------------*/
#define Rx(t,X) do { \
    (X)[0]=1.0; (X)[1]=(X)[2]=(X)[3]=(X)[6]=0.0; \
    (X)[4]=(X)[8]=cos(t); (X)[7]=sin(t); (X)[5]=-(X)[7]; \
} while (0)

#define Ry(t,X) do { \
    (X)[4]=1.0; (X)[1]=(X)[3]=(X)[5]=(X)[7]=0.0; \
    (X)[0]=(X)[8]=cos(t); (X)[2]=sin(t); (X)[6]=-(X)[2]; \
} while (0)

#define Rz(t,X) do { \
    (X)[8]=1.0; (X)[2]=(X)[5]=(X)[6]=(X)[7]=0.0; \
    (X)[0]=(X)[4]=cos(t); (X)[3]=sin(t); (X)[1]=-(X)[3]; \
} while (0)

/* astronomical arguments: f={l,l',F,D,OMG} (rad) ----------------------------*/
static void ast_args(double t, double *f)
{
    static const double fc[][5]={ /* coefficients for iau 1980 nutation */
        { 134.96340251, 1717915923.2178,  31.8792,  0.051635, -0.00024470},
        { 357.52910918,  129596581.0481,  -0.5532,  0.000136, -0.00001149},
        {  93.27209062, 1739527262.8478, -12.7512, -0.001037,  0.00000417},
        { 297.85019547, 1602961601.2090,  -6.3706,  0.006593, -0.00003169},
        { 125.04455501,   -6962890.2665,   7.4722,  0.007702  -0.00005939}
    };
    double tt[4];
    int i,j;
    
    for (tt[0]=t,i=1;i<4;i++) tt[i]=tt[i-1]*t;
    for (i=0;i<5;i++) {
        f[i]=fc[i][0]*3600.0;
        for (j=0;j<4;j++) f[i]+=fc[i][j+1]*tt[j];
        f[i]=fmod(f[i]*AS2R,2.0*PI);
    }
}
/* iau 1980 nutation ---------------------------------------------------------*/
static void nut_iau1980(double t, const double *f, double *dpsi, double *deps)
{
    static const double nut[106][10]={
        {   0,   0,   0,   0,   1, -6798.4, -171996, -174.2, 92025,   8.9},
        {   0,   0,   2,  -2,   2,   182.6,  -13187,   -1.6,  5736,  -3.1},
        {   0,   0,   2,   0,   2,    13.7,   -2274,   -0.2,   977,  -0.5},
        {   0,   0,   0,   0,   2, -3399.2,    2062,    0.2,  -895,   0.5},
        {   0,  -1,   0,   0,   0,  -365.3,   -1426,    3.4,    54,  -0.1},
        {   1,   0,   0,   0,   0,    27.6,     712,    0.1,    -7,   0.0},
        {   0,   1,   2,  -2,   2,   121.7,    -517,    1.2,   224,  -0.6},
        {   0,   0,   2,   0,   1,    13.6,    -386,   -0.4,   200,   0.0},
        {   1,   0,   2,   0,   2,     9.1,    -301,    0.0,   129,  -0.1},
        {   0,  -1,   2,  -2,   2,   365.2,     217,   -0.5,   -95,   0.3},
        {  -1,   0,   0,   2,   0,    31.8,     158,    0.0,    -1,   0.0},
        {   0,   0,   2,  -2,   1,   177.8,     129,    0.1,   -70,   0.0},
        {  -1,   0,   2,   0,   2,    27.1,     123,    0.0,   -53,   0.0},
        {   1,   0,   0,   0,   1,    27.7,      63,    0.1,   -33,   0.0},
        {   0,   0,   0,   2,   0,    14.8,      63,    0.0,    -2,   0.0},
        {  -1,   0,   2,   2,   2,     9.6,     -59,    0.0,    26,   0.0},
        {  -1,   0,   0,   0,   1,   -27.4,     -58,   -0.1,    32,   0.0},
        {   1,   0,   2,   0,   1,     9.1,     -51,    0.0,    27,   0.0},
        {  -2,   0,   0,   2,   0,  -205.9,     -48,    0.0,     1,   0.0},
        {  -2,   0,   2,   0,   1,  1305.5,      46,    0.0,   -24,   0.0},
        {   0,   0,   2,   2,   2,     7.1,     -38,    0.0,    16,   0.0},
        {   2,   0,   2,   0,   2,     6.9,     -31,    0.0,    13,   0.0},
        {   2,   0,   0,   0,   0,    13.8,      29,    0.0,    -1,   0.0},
        {   1,   0,   2,  -2,   2,    23.9,      29,    0.0,   -12,   0.0},
        {   0,   0,   2,   0,   0,    13.6,      26,    0.0,    -1,   0.0},
        {   0,   0,   2,  -2,   0,   173.3,     -22,    0.0,     0,   0.0},
        {  -1,   0,   2,   0,   1,    27.0,      21,    0.0,   -10,   0.0},
        {   0,   2,   0,   0,   0,   182.6,      17,   -0.1,     0,   0.0},
        {   0,   2,   2,  -2,   2,    91.3,     -16,    0.1,     7,   0.0},
        {  -1,   0,   0,   2,   1,    32.0,      16,    0.0,    -8,   0.0},
        {   0,   1,   0,   0,   1,   386.0,     -15,    0.0,     9,   0.0},
        {   1,   0,   0,  -2,   1,   -31.7,     -13,    0.0,     7,   0.0},
        {   0,  -1,   0,   0,   1,  -346.6,     -12,    0.0,     6,   0.0},
        {   2,   0,  -2,   0,   0, -1095.2,      11,    0.0,     0,   0.0},
        {  -1,   0,   2,   2,   1,     9.5,     -10,    0.0,     5,   0.0},
        {   1,   0,   2,   2,   2,     5.6,      -8,    0.0,     3,   0.0},
        {   0,  -1,   2,   0,   2,    14.2,      -7,    0.0,     3,   0.0},
        {   0,   0,   2,   2,   1,     7.1,      -7,    0.0,     3,   0.0},
        {   1,   1,   0,  -2,   0,   -34.8,      -7,    0.0,     0,   0.0},
        {   0,   1,   2,   0,   2,    13.2,       7,    0.0,    -3,   0.0},
        {  -2,   0,   0,   2,   1,  -199.8,      -6,    0.0,     3,   0.0},
        {   0,   0,   0,   2,   1,    14.8,      -6,    0.0,     3,   0.0},
        {   2,   0,   2,  -2,   2,    12.8,       6,    0.0,    -3,   0.0},
        {   1,   0,   0,   2,   0,     9.6,       6,    0.0,     0,   0.0},
        {   1,   0,   2,  -2,   1,    23.9,       6,    0.0,    -3,   0.0},
        {   0,   0,   0,  -2,   1,   -14.7,      -5,    0.0,     3,   0.0},
        {   0,  -1,   2,  -2,   1,   346.6,      -5,    0.0,     3,   0.0},
        {   2,   0,   2,   0,   1,     6.9,      -5,    0.0,     3,   0.0},
        {   1,  -1,   0,   0,   0,    29.8,       5,    0.0,     0,   0.0},
        {   1,   0,   0,  -1,   0,   411.8,      -4,    0.0,     0,   0.0},
        {   0,   0,   0,   1,   0,    29.5,      -4,    0.0,     0,   0.0},
        {   0,   1,   0,  -2,   0,   -15.4,      -4,    0.0,     0,   0.0},
        {   1,   0,  -2,   0,   0,   -26.9,       4,    0.0,     0,   0.0},
        {   2,   0,   0,  -2,   1,   212.3,       4,    0.0,    -2,   0.0},
        {   0,   1,   2,  -2,   1,   119.6,       4,    0.0,    -2,   0.0},
        {   1,   1,   0,   0,   0,    25.6,      -3,    0.0,     0,   0.0},
        {   1,  -1,   0,  -1,   0, -3232.9,      -3,    0.0,     0,   0.0},
        {  -1,  -1,   2,   2,   2,     9.8,      -3,    0.0,     1,   0.0},
        {   0,  -1,   2,   2,   2,     7.2,      -3,    0.0,     1,   0.0},
        {   1,  -1,   2,   0,   2,     9.4,      -3,    0.0,     1,   0.0},
        {   3,   0,   2,   0,   2,     5.5,      -3,    0.0,     1,   0.0},
        {  -2,   0,   2,   0,   2,  1615.7,      -3,    0.0,     1,   0.0},
        {   1,   0,   2,   0,   0,     9.1,       3,    0.0,     0,   0.0},
        {  -1,   0,   2,   4,   2,     5.8,      -2,    0.0,     1,   0.0},
        {   1,   0,   0,   0,   2,    27.8,      -2,    0.0,     1,   0.0},
        {  -1,   0,   2,  -2,   1,   -32.6,      -2,    0.0,     1,   0.0},
        {   0,  -2,   2,  -2,   1,  6786.3,      -2,    0.0,     1,   0.0},
        {  -2,   0,   0,   0,   1,   -13.7,      -2,    0.0,     1,   0.0},
        {   2,   0,   0,   0,   1,    13.8,       2,    0.0,    -1,   0.0},
        {   3,   0,   0,   0,   0,     9.2,       2,    0.0,     0,   0.0},
        {   1,   1,   2,   0,   2,     8.9,       2,    0.0,    -1,   0.0},
        {   0,   0,   2,   1,   2,     9.3,       2,    0.0,    -1,   0.0},
        {   1,   0,   0,   2,   1,     9.6,      -1,    0.0,     0,   0.0},
        {   1,   0,   2,   2,   1,     5.6,      -1,    0.0,     1,   0.0},
        {   1,   1,   0,  -2,   1,   -34.7,      -1,    0.0,     0,   0.0},
        {   0,   1,   0,   2,   0,    14.2,      -1,    0.0,     0,   0.0},
        {   0,   1,   2,  -2,   0,   117.5,      -1,    0.0,     0,   0.0},
        {   0,   1,  -2,   2,   0,  -329.8,      -1,    0.0,     0,   0.0},
        {   1,   0,  -2,   2,   0,    23.8,      -1,    0.0,     0,   0.0},
        {   1,   0,  -2,  -2,   0,    -9.5,      -1,    0.0,     0,   0.0},
        {   1,   0,   2,  -2,   0,    32.8,      -1,    0.0,     0,   0.0},
        {   1,   0,   0,  -4,   0,   -10.1,      -1,    0.0,     0,   0.0},
        {   2,   0,   0,  -4,   0,   -15.9,      -1,    0.0,     0,   0.0},
        {   0,   0,   2,   4,   2,     4.8,      -1,    0.0,     0,   0.0},
        {   0,   0,   2,  -1,   2,    25.4,      -1,    0.0,     0,   0.0},
        {  -2,   0,   2,   4,   2,     7.3,      -1,    0.0,     1,   0.0},
        {   2,   0,   2,   2,   2,     4.7,      -1,    0.0,     0,   0.0},
        {   0,  -1,   2,   0,   1,    14.2,      -1,    0.0,     0,   0.0},
        {   0,   0,  -2,   0,   1,   -13.6,      -1,    0.0,     0,   0.0},
        {   0,   0,   4,  -2,   2,    12.7,       1,    0.0,     0,   0.0},
        {   0,   1,   0,   0,   2,   409.2,       1,    0.0,     0,   0.0},
        {   1,   1,   2,  -2,   2,    22.5,       1,    0.0,    -1,   0.0},
        {   3,   0,   2,  -2,   2,     8.7,       1,    0.0,     0,   0.0},
        {  -2,   0,   2,   2,   2,    14.6,       1,    0.0,    -1,   0.0},
        {  -1,   0,   0,   0,   2,   -27.3,       1,    0.0,    -1,   0.0},
        {   0,   0,  -2,   2,   1,  -169.0,       1,    0.0,     0,   0.0},
        {   0,   1,   2,   0,   1,    13.1,       1,    0.0,     0,   0.0},
        {  -1,   0,   4,   0,   2,     9.1,       1,    0.0,     0,   0.0},
        {   2,   1,   0,  -2,   0,   131.7,       1,    0.0,     0,   0.0},
        {   2,   0,   0,   2,   0,     7.1,       1,    0.0,     0,   0.0},
        {   2,   0,   2,  -2,   1,    12.8,       1,    0.0,    -1,   0.0},
        {   2,   0,  -2,   0,   1,  -943.2,       1,    0.0,     0,   0.0},
        {   1,  -1,   0,  -2,   0,   -29.3,       1,    0.0,     0,   0.0},
        {  -1,   0,   0,   1,   1,  -388.3,       1,    0.0,     0,   0.0},
        {  -1,  -1,   0,   2,   1,    35.0,       1,    0.0,     0,   0.0},
        {   0,   1,   0,   1,   0,    27.3,       1,    0.0,     0,   0.0}
    };
    double ang;
    int i,j;
    
    *dpsi=*deps=0.0;
    
    for (i=0;i<106;i++) {
        ang=0.0;
        for (j=0;j<5;j++) ang+=nut[i][j]*f[j];
        *dpsi+=(nut[i][6]+nut[i][7]*t)*sin(ang);
        *deps+=(nut[i][8]+nut[i][9]*t)*cos(ang);
    }
    *dpsi*=1E-4*AS2R; /* 0.1 mas -> rad */
    *deps*=1E-4*AS2R;
}
/* eci to ecef transformation matrix -------------------------------------------
* compute eci to ecef transformation matrix
* args   : gtime_t tutc     I   time in utc
*          erp_t *erp       I   earth rotation parameters (NULL: not used)
*          double *U        O   eci to ecef transformation matrix (3 x 3)
*          double *gmst     IO  greenwich mean sidereal time (rad)
*                               (NULL: no output)
* return : none
* note   : see ref [3] chap 5
*          not thread-safe
*-----------------------------------------------------------------------------*/
extern void eci2ecef(gtime_t tutc, const erp_t *erp, double *U, double *gmst)
{
    const double ep2000[]={2000,1,1,12,0,0};
    static gtime_t tutc_;
    static double U_[9],gmst_;
    gtime_t tgps,tut,tut0;
    erp_t erp_={0};
    double t,t2,t3,eps,ze,th,z,dpsi,deps,ut,gmst0,gast,f[5];
    double R1[9],R2[9],R3[9],R[9],W[9],N[9],P[9],NP[9];
    int i;
    
    trace(3,"eci2ecef: tutc=%s\n",time_str(tutc,3));
    
    if (fabs(timediff(tutc,tutc_))<0.01) { /* read cache */
        for (i=0;i<9;i++) U[i]=U_[i];
        if (gmst) *gmst=gmst_; 
        return;
    }
    tutc_=tutc;
    if (erp) erp_=*erp;
    
    /* terrestrial time */
    tgps=utc2gpst(tutc_);
    t=(timediff(tgps,epoch2time(ep2000))+19.0+32.184)/86400.0/36525.0;
    t2=t*t; t3=t2*t;
    
    /* astronomical arguments */
    ast_args(t,f);
    
    /* iau 1976 precession */
    ze=(2306.2181*t+0.30188*t2+0.017998*t3)*AS2R;
    th=(2004.3109*t-0.42665*t2-0.041833*t3)*AS2R;
    z =(2306.2181*t+1.09468*t2+0.018203*t3)*AS2R;
    eps=(84381.448-46.8150*t-0.00059*t2+0.001813*t3)*AS2R;
    Rz(-z,R1); Ry(th,R2); Rz(-ze,R3);
    matmul("NN",3,3,3,1.0,R1,R2,0.0,R);
    matmul("NN",3,3,3,1.0,R, R3,0.0,P); /* P=Rz(-z)*Ry(th)*Rz(-ze) */
    
    /* iau 1980 nutation */
    nut_iau1980(t,f,&dpsi,&deps);
    Rx(-eps-deps-erp_.ddeps,R1); Rz(-dpsi-erp_.ddpsi,R2); Rx(eps,R3);
    matmul("NN",3,3,3,1.0,R1,R2,0.0,R);
    matmul("NN",3,3,3,1.0,R ,R3,0.0,N); /* N=Rx(-eps)*Rz(-dspi)*Rx(eps) */
    
    /* greenwich mean/aparent sidereal time (rad) */
    tut=timeadd(tutc_,erp_.ut1_utc);
    ut=time2sec(tut,&tut0);
    t=timediff(tut0,epoch2time(ep2000))/86400.0/36525.0;
    t2=t*t; t3=t2*t;
    gmst0=24110.54841+8640184.812866*t+0.093104*t2-6.2E-6*t3;
    gmst_=gmst0+1.002737909350795*ut;
    gmst_=fmod(gmst_,86400.0)*PI/43200.0;
    gast=gmst_+dpsi*cos(eps);
    gast+=(0.00264*sin(f[4])+0.000063*sin(2.0*f[4]))*AS2R;
    
    /* eci to ecef transformation matrix */
    Ry(-erp_.xp,R1); Rx(-erp_.yp,R2); Rz(gast,R3);
    matmul("NN",3,3,3,1.0,R1,R2,0.0,W );
    matmul("NN",3,3,3,1.0,W ,R3,0.0,R ); /* W=Ry(-xp)*Rx(-yp) */
    matmul("NN",3,3,3,1.0,N ,P ,0.0,NP);
    matmul("NN",3,3,3,1.0,R ,NP,0.0,U_); /* U=W*Rz(gast)*N*P */
    
    for (i=0;i<9;i++) U[i]=U_[i];
    if (gmst) *gmst=gmst_; 
    
    trace(5,"gmst=%.12f gast=%.12f\n",gmst_,gast);
    trace(5,"P=\n"); tracemat(5,P,3,3,15,12);
    trace(5,"N=\n"); tracemat(5,N,3,3,15,12);
    trace(5,"W=\n"); tracemat(5,W,3,3,15,12);
    trace(5,"U=\n"); tracemat(5,U,3,3,15,12);
}
/* sun and moon position in eci (ref [4] 5.1.1, 5.2.1) -----------------------*/
static void sunmoonpos_eci(gtime_t tut, double *rsun, double *rmoon)
{
    const double ep2000[]={2000,1,1,12,0,0};
    double t,f[5],eps,Ms,ls,rs,lm,pm,rm,sine,cose,sinp,cosp,sinl,cosl;
    
    trace(3,"sunmoonpos_eci: tut=%s\n",time_str(tut,3));
    
    t=timediff(tut,epoch2time(ep2000))/86400.0/36525.0;
    
    /* astronomical arguments */
    ast_args(t,f);
    
    /* obliquity of the ecliptic */
    eps=23.439291-0.0130042*t;
    sine=sin(eps*D2R); cose=cos(eps*D2R);
    
    /* sun position in eci */
    if (rsun) {
        Ms=357.5277233+35999.05034*t;
        ls=280.460+36000.770*t+1.914666471*sin(Ms*D2R)+0.019994643*sin(2.0*Ms*D2R);
        rs=AU*(1.000140612-0.016708617*cos(Ms*D2R)-0.000139589*cos(2.0*Ms*D2R));
        sinl=sin(ls*D2R); cosl=cos(ls*D2R);
        rsun[0]=rs*cosl;
        rsun[1]=rs*cose*sinl;
        rsun[2]=rs*sine*sinl;
        
        trace(5,"rsun =%.3f %.3f %.3f\n",rsun[0],rsun[1],rsun[2]);
    }
    /* moon position in eci */
    if (rmoon) {
        lm=218.32+481267.883*t+6.29*sin(f[0])-1.27*sin(f[0]-2.0*f[3])+
           0.66*sin(2.0*f[3])+0.21*sin(2.0*f[0])-0.19*sin(f[1])-0.11*sin(2.0*f[2]);
        pm=5.13*sin(f[2])+0.28*sin(f[0]+f[2])-0.28*sin(f[2]-f[0])-
           0.17*sin(f[2]-2.0*f[3]);
        rm=RE_WGS84/sin((0.9508+0.0518*cos(f[0])+0.0095*cos(f[0]-2.0*f[3])+
                   0.0078*cos(2.0*f[3])+0.0028*cos(2.0*f[0]))*D2R);
        sinl=sin(lm*D2R); cosl=cos(lm*D2R);
        sinp=sin(pm*D2R); cosp=cos(pm*D2R);
        rmoon[0]=rm*cosp*cosl;
        rmoon[1]=rm*(cose*cosp*sinl-sine*sinp);
        rmoon[2]=rm*(sine*cosp*sinl+cose*sinp);
        
        trace(5,"rmoon=%.3f %.3f %.3f\n",rmoon[0],rmoon[1],rmoon[2]);
    }
}
/* sun and moon position -------------------------------------------------------
* get sun and moon position in ecef
* args   : gtime_t tut      I   time in ut1
*          erp_t  *erp      I   earth rotation parameters (NULL: not used)
*          double *rsun     O   sun position in ecef (m)
*          double *rmoon    O   moon position in ecef (m)
*          double *gmst     O   gmst (rad)
* return : none
*-----------------------------------------------------------------------------*/
extern void sunmoonpos(gtime_t tutc, const erp_t *erp, double *rsun,
                       double *rmoon, double *gmst)
{
    gtime_t tut;
    double rs[3],rm[3],U[9];
    
    trace(3,"sunmoonpos: tutc=%s\n",time_str(tutc,3));
    
    tut=erp?timeadd(tutc,erp->ut1_utc):tutc; /* utc -> ut1 */
    
    /* sun and moon position in eci */
    sunmoonpos_eci(tut,rsun?rs:NULL,rmoon?rm:NULL);
    
    /* eci to ecef transformation matrix */
    eci2ecef(tutc,erp,U,gmst);
    
    /* sun and moon postion in ecef */
    if (rsun ) matmul("NN",3,1,3,1.0,U,rs,0.0,rsun );
    if (rmoon) matmul("NN",3,1,3,1.0,U,rm,0.0,rmoon);
}
/* read sp3 header -----------------------------------------------------------*/
static int readsp3h(FILE *fp, gtime_t *time, char *type, int *sats,
                    double *bfact, char *tsys)
{
    int i,j,k=0,ns=0,sys,prn;
    char buff[1024];
    
    trace(3,"readsp3h:\n");
    
    for (i=0;i<22;i++) {
        if (!fgets(buff,sizeof(buff),fp)) break;
        
        if (i==0) {
            *type=buff[2];
            if (str2time(buff,3,28,time)) return 0;
        }
        else if (2<=i&&i<=6) {
            if (i==2) {
                ns=(int)str2num(buff,4,2);
            }
            for (j=0;j<17&&k<ns;j++) {
                sys=code2sys(buff[9+3*j]);
                prn=(int)str2num(buff,10+3*j,2);
                if (k<MAXSAT) sats[k++]=satno(sys,prn);
            }
        }
        else if (i==12) {
            strncpy(tsys,buff+9,3); tsys[3]='\0';
        }
        else if (i==14) {
            bfact[0]=str2num(buff, 3,10);
            bfact[1]=str2num(buff,14,12);
        }
    }
    return ns;
}
/* add precise ephemeris -----------------------------------------------------*/
static int addpeph(nav_t *nav, peph_t *peph)
{
    peph_t *nav_peph;
    
    if (nav->ne>=nav->nemax) {
        nav->nemax+=256;
        if (!(nav_peph=(peph_t *)realloc(nav->peph,sizeof(peph_t)*nav->nemax))) {
            trace(1,"readsp3b malloc error n=%d\n",nav->nemax);
            free(nav->peph); nav->peph=NULL; nav->ne=nav->nemax=0;
            return 0;
        }
        nav->peph=nav_peph;
    }
    nav->peph[nav->ne++]=*peph;
    return 1;
}
/* read sp3 body -------------------------------------------------------------*/
static void readsp3b(FILE *fp, char type, int *sats, int ns, double *bfact,
                     char *tsys, int index, nav_t *nav)
{
    peph_t peph;
    gtime_t time;
    double val,std,base;
    int i,j,sat,sys,prn,n=ns*(type=='P'?1:2);
    char buff[1024];
    
    trace(3,"readsp3b: type=%c ns=%d index=%d\n",type,ns,index);
    
    while (fgets(buff,sizeof(buff),fp)) {
        
        if (!strncmp(buff,"EOF",3)) break;
        
        if (buff[0]!='*'||str2time(buff,3,28,&time)) {
            trace(2,"sp3 invalid epoch %31.31s\n",buff);
            continue;
        }
        if (!strcmp(tsys,"UTC")) time=utc2gpst(time); /* utc->gpst */
        peph.time =time;
        peph.index=index;
        
        for (i=0;i<MAXSAT;i++) for (j=0;j<4;j++) {
            peph.pos[i][j]=0.0;
            peph.std[i][j]=0.0f;
        }
        for (i=0;i<n&&fgets(buff,sizeof(buff),fp);i++) {
            
            if (strlen(buff)<4||buff[0]!='P') continue;
            
            sys=buff[1]==' '?SYS_GPS:code2sys(buff[1]);
            prn=(int)str2num(buff,2,2);
            if      (sys==SYS_SBS) prn+=100;
            else if (sys==SYS_QZS) prn+=192; /* extension to sp3-c */
            
            if (!(sat=satno(sys,prn))) continue;
            
            for (j=0;j<4;j++) {
                val=str2num(buff, 4+j*14,14);
                std=str2num(buff,61+j* 3,j<3?2:3);
                
                if (val!=0.0&&fabs(val-999999.999999)>=1E-6) {
                    peph.pos[sat-1][j]=val*(j<3?1000.0:1E-6);
                }
                if ((base=bfact[j<3?0:1])>0.0) {
                    peph.std[sat-1][j]=(float)(pow(base,std)*(j<3?1E-3:1E-12));
                }
            }
        }
        if (!addpeph(nav,&peph)) return;
    }
}
/* compare precise ephemeris -------------------------------------------------*/
static int cmppeph(const void *p1, const void *p2)
{
    peph_t *q1=(peph_t *)p1,*q2=(peph_t *)p2;
    double tt=timediff(q1->time,q2->time);
    return tt<-1E-9?-1:(tt>1E-9?1:q1->index-q2->index);
}
/* combine precise ephemeris -------------------------------------------------*/
static void combpeph(nav_t *nav)
{
    int i,j,k,m;
    
    trace(3,"combpeph: ne=%d\n",nav->ne);
    
    qsort(nav->peph,nav->ne,sizeof(peph_t),cmppeph);
    
    for (i=0,j=1;j<nav->ne;j++) {
        
        if (fabs(timediff(nav->peph[i].time,nav->peph[j].time))<1E-9) {
            
            for (k=0;k<MAXSAT;k++) {
                if (norm(nav->peph[j].pos[k],4)<=0.0) continue;
                for (m=0;m<4;m++) nav->peph[i].pos[k][m]=nav->peph[j].pos[k][m];
                for (m=0;m<4;m++) nav->peph[i].std[k][m]=nav->peph[j].std[k][m];
            }
        }
        else if (++i<j) nav->peph[i]=nav->peph[j];
    }
    nav->ne=i+1;
    
    trace(4,"combpeph: ne=%d\n",nav->ne);
}
/* read sp3 precise ephemeris file ---------------------------------------------
* read sp3 precise ephemeris/clock files and set them to navigation data
* args   : char   *file       I   sp3-c precise ephemeris file
*                                 (wind-card * is expanded)
*          nav_t  *nav        IO  navigation data
* return : none
* notes  : see ref [1]
*          precise ephemeris is appended and combined
*          nav->peph and nav->ne must by properly initialized before calling the
*          function
*          only files with extensions of .sp3, .SP3, .eph* and .EPH* are read
*-----------------------------------------------------------------------------*/
extern void readsp3(const char *file, nav_t *nav)
{
    FILE *fp;
    gtime_t time={0};
    double bfact[2]={0};
    int i,j,n,ns,sats[MAXSAT]={0};
    char *efiles[MAXEXFILE],*ext,type=' ',tsys[4]="";
    
    trace(3,"readpephs: file=%s\n",file);
    
    for (i=0;i<MAXEXFILE;i++) {
        if (!(efiles[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(efiles[i]);
            return;
        }
    }
    /* expand wild card in file path */
    n=expath(file,efiles,MAXEXFILE);
    
    for (i=j=0;i<n;i++) {
        if (!(ext=strrchr(efiles[i],'.'))) continue;
        
        if (strcmp (ext,".sp3"  )&&strcmp (ext,".SP3"  )&&
            strncmp(ext,".eph",4)&&strncmp(ext,".EPH",4)) continue;
        
        if (!(fp=fopen(efiles[i],"r"))) {
            trace(2,"sp3 file open error %s\n",efiles[i]);
            continue;
        }
        /* read sp3 header */
        ns=readsp3h(fp,&time,&type,sats,bfact,tsys);
        
        /* read sp3 body */
        readsp3b(fp,type,sats,ns,bfact,tsys,j++,nav);
        
        fclose(fp);
    }
    for (i=0;i<MAXEXFILE;i++) free(efiles[i]);
    
    /* combine precise ephemeris */
    if (nav->ne>0) combpeph(nav);
}
/* read satellite antenna parameters -------------------------------------------
* read satellite antenna parameters
* args   : char   *file       I   antenna parameter file
*          gtime_t time       I   time
*          nav_t  *nav        IO  navigation data
* return : status (1:ok,0:error)
* notes  : only support antex format for the antenna parameter file
*-----------------------------------------------------------------------------*/
extern int readsap(const char *file, gtime_t time, nav_t *nav)
{
    pcvs_t pcvs={0};
    pcv_t pcv0={0},*pcv;
    int i;
    
    trace(3,"readsap : file=%s time=%s\n",file,time_str(time,0));
    
    if (!readpcv(file,&pcvs)) return 0;
    
    for (i=0;i<MAXSAT;i++) {
        pcv=searchpcv(i+1,"",time,&pcvs);
        nav->pcvs[i]=pcv?*pcv:pcv0;
    }
    free(pcvs.pcv);
    return 1;
}
/* read dcb parameters file --------------------------------------------------*/
static int readdcbf(const char *file, nav_t *nav)
{
    FILE *fp;
    double cbias;
    int sat,type=0;
    char buff[256];
    
    trace(3,"readdcbf: file=%s\n",file);
    
    if (!(fp=fopen(file,"r"))) {
        trace(2,"dcb parameters file open error: %s\n",file);
        return 0;
    }
    while (fgets(buff,sizeof(buff),fp)) {
        
        if      (strstr(buff,"DIFFERENTIAL (P1-P2) CODE BIASES")) type=1;
        else if (strstr(buff,"DIFFERENTIAL (P1-C1) CODE BIASES")) type=2;
        else if (strstr(buff,"DIFFERENTIAL (P2-C2) CODE BIASES")) type=3;
        
        if (!type) continue;
        
        if (!(sat=satid2no(buff))||(cbias=str2num(buff,26,9))==0.0) continue;
        
        nav->cbias[sat-1][type-1]=cbias*1E-9*CLIGHT; /* ns -> m */
    }
    fclose(fp);
    
    return 1;
}
/* read dcb parameters ---------------------------------------------------------
* read differential code bias (dcb) parameters
* args   : char   *file       I   dcb parameters file (wild-card * expanded)
*          nav_t  *nav        IO  navigation data
* return : status (1:ok,0:error)
* notes  : currently only p1-c1 bias of code *.dcb file
*-----------------------------------------------------------------------------*/
extern int readdcb(const char *file, nav_t *nav)
{
    int i,j,n;
    char *efiles[MAXEXFILE]={0};
    
    trace(3,"readdcb : file=%s\n",file);
    
    for (i=0;i<MAXSAT;i++) for (j=0;j<3;j++) {
        nav->cbias[i][j]=0.0;
    }
    for (i=0;i<MAXEXFILE;i++) {
        if (!(efiles[i]=(char *)malloc(1024))) {
            for (i--;i>=0;i--) free(efiles[i]);
            return 0;
        }
    }
    n=expath(file,efiles,MAXEXFILE);
    
    for (i=0;i<n;i++) {
        readdcbf(efiles[i],nav);
    }
    for (i=0;i<MAXEXFILE;i++) free(efiles[i]);
    
    return 1;
}
/* polynomial interpolation by Neville's algorithm ---------------------------*/
static double interppol(const double *x, double *y, int n)
{
    int i,j;
    
    for (j=1;j<n;j++) {
        for (i=0;i<n-j;i++) {
            y[i]=(x[i+j]*y[i]-x[i]*y[i+1])/(x[i+j]-x[i]);
        }
    }
    return y[0];
}
/* satellite position by precise ephemeris -----------------------------------*/
static int pephpos(gtime_t time, int sat, const nav_t *nav, double *rs,
                   double *dts, double *vare, double *varc)
{
    double t[NMAX+1],p[3][NMAX+1],c[2],*pos,std=0.0,s[3],sinl,cosl;
    int i,j,k,index;
    
    trace(4,"pephpos : time=%s sat=%2d\n",time_str(time,3),sat);
    
    rs[0]=rs[1]=rs[2]=dts[0]=0.0;
    
    if (nav->ne<NMAX+1||
        timediff(time,nav->peph[0].time)<-MAXDTE||
        timediff(time,nav->peph[nav->ne-1].time)>MAXDTE) {
        trace(2,"no prec ephem %s sat=%2d\n",time_str(time,0),sat);
        return 0;
    }
    /* binary search */
    for (i=0,j=nav->ne-1;i<j;) {
        k=(i+j)/2;
        if (timediff(nav->peph[k].time,time)<0.0) i=k+1; else j=k;
    }
    index=i<=0?0:i-1;
    
    /* polynomial interpolation for orbit */
    i=index-(NMAX+1)/2;
    if (i<0) i=0; else if (i+NMAX>=nav->ne) i=nav->ne-NMAX-1;
    
    for (j=0;j<=NMAX;j++) {
        t[j]=timediff(nav->peph[i+j].time,time);
        if (norm(nav->peph[i+j].pos[sat-1],3)<=0.0) {
            trace(2,"prec ephem outage %s sat=%2d\n",time_str(time,0),sat);
            return 0;
        }
    }
    for (j=0;j<=NMAX;j++) {
        pos=nav->peph[i+j].pos[sat-1];
#if 0
        p[0][j]=pos[0];
        p[1][j]=pos[1];
#else
        /* correciton for earh rotation ver.2.4.0 */
        sinl=sin(OMGE*t[j]);
        cosl=cos(OMGE*t[j]);
        p[0][j]=cosl*pos[0]-sinl*pos[1];
        p[1][j]=sinl*pos[0]+cosl*pos[1];
#endif
        p[2][j]=pos[2];
    }
    for (i=0;i<3;i++) {
        rs[i]=interppol(t,p[i],NMAX+1);
    }
    if (vare) {
        for (i=0;i<3;i++) s[i]=nav->peph[index].std[sat-1][i];
        std=norm(s,3);
        
        /* extrapolation error for orbit */
        if      (t[0   ]>0.0) std+=EXTERR_EPH*SQR(t[0   ])/2.0;
        else if (t[NMAX]<0.0) std+=EXTERR_EPH*SQR(t[NMAX])/2.0;
        *vare=SQR(std);
    }
    /* linear interpolation for clock */
    t[0]=timediff(time,nav->peph[index  ].time);
    t[1]=timediff(time,nav->peph[index+1].time);
    c[0]=nav->peph[index  ].pos[sat-1][3];
    c[1]=nav->peph[index+1].pos[sat-1][3];
    
    if (t[0]<=0.0) {
        if ((dts[0]=c[0])!=0.0) {
            std=nav->peph[index].std[sat-1][3]*CLIGHT-EXTERR_CLK*t[0];
        }
    }
    else if (t[1]>=0.0) {
        if ((dts[0]=c[1])!=0.0) {
            std=nav->peph[index+1].std[sat-1][3]*CLIGHT+EXTERR_CLK*t[1];
        }
    }
    else if (c[0]!=0.0&&c[1]!=0.0) {
        dts[0]=(c[1]*t[0]-c[0]*t[1])/(t[0]-t[1]);
        i=t[0]<-t[1]?0:1;
        std=nav->peph[index+i].std[sat-1][3]+EXTERR_CLK*fabs(t[i]);
    }
    else {
        dts[0]=0.0;
    }
    if (varc) *varc=SQR(std);
    return 1;
}
/* satellite clock by precise clock ------------------------------------------*/
static int pephclk(gtime_t time, int sat, const nav_t *nav, double *dts,
                   double *varc)
{
    double t[2],c[2],std;
    int i,j,k,index;
    
    trace(4,"pephclk : time=%s sat=%2d\n",time_str(time,3),sat);
    
    if (nav->nc<2||
        timediff(time,nav->pclk[0].time)<-MAXDTE||
        timediff(time,nav->pclk[nav->nc-1].time)>MAXDTE) {
        trace(3,"no prec clock %s sat=%2d\n",time_str(time,0),sat);
        return 1;
    }
    /* binary search */
    for (i=0,j=nav->nc-1;i<j;) {
        k=(i+j)/2;
        if (timediff(nav->pclk[k].time,time)<0.0) i=k+1; else j=k;
    }
    index=i<=0?0:i-1;
    
    /* linear interpolation for clock */
    t[0]=timediff(time,nav->pclk[index  ].time);
    t[1]=timediff(time,nav->pclk[index+1].time);
    c[0]=nav->pclk[index  ].clk[sat-1][0];
    c[1]=nav->pclk[index+1].clk[sat-1][0];
    
    if (t[0]<=0.0) {
        if ((dts[0]=c[0])==0.0) return 0;
        std=nav->pclk[index].std[sat-1][3]*CLIGHT-EXTERR_CLK*t[0];
    }
    else if (t[1]>=0.0) {
        if ((dts[0]=c[1])==0.0) return 0;
        std=nav->pclk[index+1].std[sat-1][3]*CLIGHT+EXTERR_CLK*t[1];
    }
    else if (c[0]!=0.0&&c[1]!=0.0) {
        dts[0]=(c[1]*t[0]-c[0]*t[1])/(t[0]-t[1]);
        i=t[0]<-t[1]?0:1;
        std=nav->pclk[index+i].std[sat-1][3]+EXTERR_CLK*fabs(t[i]);
    }
    else {
        trace(3,"prec clock outage %s sat=%2d\n",time_str(time,0),sat);
        return 0;
    }
    if (varc) *varc=SQR(std);
    return 1;
}
/* satellite antenna phase center offset ---------------------------------------
* compute satellite antenna phase center offset in ecef
* args   : gtime_t time       I   time (gpst)
*          double *rs         I   satellite position and velocity (ecef)
*                                 {x,y,z,vx,vy,vz} (m|m/s)
*          pcv_t  *pcv        I   satellite antenna parameter
*          double *dant       I   satellite antenna phase center offset (ecef)
*                                 {dx,dy,dz} (m)
* return : none
*-----------------------------------------------------------------------------*/
extern void satantoff(gtime_t time, const double *rs, const pcv_t *pcv,
                      double *dant)
{
    double ex[3],ey[3],ez[3],es[3],r[3],rsun[3],gmst;
    int i;
    
    trace(4,"satantoff: time=%s\n",time_str(time,3));
    
    /* sun position in ecef */
    sunmoonpos(gpst2utc(time),NULL,rsun,NULL,&gmst);
    
    /* unit vectors of satellite fixed coordinates */
    for (i=0;i<3;i++) r[i]=-rs[i];
    if (!normv3(r,ez)) return;
    for (i=0;i<3;i++) r[i]=rsun[i]-rs[i];
    if (!normv3(r,es)) return;
    cross3(ez,es,r);
    if (!normv3(r,ey)) return;
    cross3(ey,ez,ex);
    
    for (i=0;i<3;i++) { /* use L1 value */
        dant[i]=pcv->off[0][0]*ex[i]+pcv->off[0][1]*ey[i]+pcv->off[0][2]*ez[i];
    }
}
/* satellite position/clock by precise ephemeris/clock -------------------------
* compute satellite position/clock with precise ephemeris/clock
* args   : gtime_t time       I   time (gpst)
*          int    sat         I   satellite number
*          nav_t  *nav        I   navigation data
*          int    opt         I   sat postion option
*                                 (0: center of mass, 1: antenna phase center)
*          double *rs         O   sat position and velocity (ecef)
*                                 {x,y,z,vx,vy,vz} (m|m/s)
*          double *dts        O   sat clock {bias,drift} (s|s/s)
*          double *var        IO  sat position and clock error variance (m)
*                                 (NULL: no output)
* return : status (1:ok,0:error or data outage)
* notes  : clock includes relativistic correction but does not contain code bias
*          before calling the function, nav->peph, nav->ne, nav->pclk and
*          nav->nc must be set by calling readsp3(), readrnx() or readrnxt()
*          if precise clocks are not set, clocks in sp3 are used instead
*-----------------------------------------------------------------------------*/
extern int peph2pos(gtime_t time, int sat, const nav_t *nav, int opt,
                    double *rs, double *dts, double *var)
{
    double rss[3],rst[3],dtss[1],dtst[1],dant[3]={0},vare=0.0,varc=0.0,tt=1E-3;
    int i;
    
    trace(4,"peph2pos: time=%s sat=%2d opt=%d\n",time_str(time,3),sat,opt);
    
    if (sat<=0||MAXSAT<sat) return 0;
    
    /* satellite position and clock bias */
    if (!pephpos(time,sat,nav,rss,dtss,&vare,&varc)||
        !pephclk(time,sat,nav,dtss,&varc)) return 0;
    
    time=timeadd(time,tt);
    if (!pephpos(time,sat,nav,rst,dtst,NULL,NULL)||
        !pephclk(time,sat,nav,dtst,NULL)) return 0;
    
    /* satellite antenna offset correction */
    if (opt) {
        satantoff(time,rss,nav->pcvs+sat-1,dant);
    }
    for (i=0;i<3;i++) {
        rs[i  ]=rss[i]+dant[i];
        rs[i+3]=(rst[i]-rss[i])/tt;
    }
    /* relativistic effect correction */
    dts[0]=dtss[0]-2.0*dot(rs,rs+3,3)/CLIGHT/CLIGHT;
    dts[1]=(dtst[0]-dtss[0])/tt;
    
    if (var) *var=vare+varc;
    
    return 1;
}
