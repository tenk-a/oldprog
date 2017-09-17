/*
    DoPI.exe  Pi画像ローダ
    ver 1.11
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include <conio.h>
#include <alloc.h>
#include <signal.h>
#include <pc98.h>
#include "dopi.h"
#define pr(s)	printf(s)

    byte *gNameVer = "DoPI v1.11";  //β版";

#define ATMO	1

//#define PILD4C    1
#define DEB_DMP 1
#ifdef DEB_DMP
_S byte gDmpFlg;    	    //デバッグ用. 展開データをDUMPするかどうか
#endif
#ifdef PILD4C	    	    //もはやこのルーチンは使えなくなっている
_S byte gPiLd4cFlg = 0;     //Cで書かれた展開ルーチンを使用するかどうか
#endif
#define DEBSCL
byte gDebSclFlg;    	    //スクロールのデバッグ用...


#define DFLT_TOON   50
byte oDfltTone = DFLT_TOON; //-v,-lvでの省略時のトーン

byte *gPrgName;     	    //起動したときのコマンド名
byte gChk;  	    	    //デバック用フラグ
byte gChkPiLd4a;    	    //PiLd4a,PiLd4bのデバッグ用フラグ
word gPalTone;	    	    //パレットのトーン
long PiLd_filSiz;   	    //ロードするPi画像のファイルサイズ
_S int g256mode;    	    //color 256
#ifdef	HF
//int  gFBmode;     	    //
_S byte gHfMode = 0;	    //HyPER-FRAME+ を使う
_S byte gEx256Flg = 0;	    //1=前回の表示はＨＦに表示した. 2=PC386M(P)
#endif
_S int	gBx,gBy;    	    //-o での基点
_S int	gSx,gSy;    	    //画像の表示位置
_S int	gL200flg;   	    //200ライン画像かどうか
_S int	gFx,gFy;    	    //スクロールできるかどうかのフラグ
_S word gEndPalTone;	    //終了時のパレットのトーン
_S byte gEndPalToneFlg;     // -lv が指定されたかどうか
_S byte gEndGrphOffFlg;     // -lvf が指定されたかどうか
_S byte gKeyWaitFlg;	    //キー入力モードにするかどうか
_S byte gClsFlg;    	    //表示毎に画面消去するかどうか
_S byte gBxByMode;  	    //-o が指定されたかどうか, -oaかどうか
_S byte gOvrLdFlg;  	    //画面合成するかどうか
_S byte gOvrLdCol;  	    //画面合成での透明色
_S byte gOvrLdFlg2; 	    //Piヘッダに埋め込まれた透明色を無視するかどうか
_S byte gUraUseMode;	    //裏VRAM に書き込みを行なうかどうか
_S byte gUraWrtMode;	    //-lb[N] の N.
_S byte gVRamGetNo; 	    //-kg[N] の N.
_S byte gPalFlg;    	    //-v が指定されたかどうか
_S byte gUraLoadFlg;	    //-b で裏VRAMにロードするかどうか
_S byte gGdcSclFlg; 	    //GDCスクロールかソフトウェア転送か
_S byte gSclBakFlg; 	    //スクロールでバックして始点に戻るか
_S byte gSclLoopFlg;	    //上下左右で端まできたとき反対側に行くか止まるか
_S byte gSmlFlg;    	    //縮小表示するかどうか
_S byte gTxtFlg;    	    //-t:ロードファイル名、コメント表示をするかどうか
_S byte gInfoFlg;   	    //-i:ヘッダを表示するか
_S int	gMieMode;   	    //-a:見栄張り表示 1,2
_S byte gMie2Mode;  	    //-a:見栄張り表示 3,4
_S byte gRdBufVRamFlg;	    //入力バッファを裏VRAMにとるかどうか
_S byte gToneDownFlg;	    //画面消去でフィードアウトするかどうか
_S byte gToneUpFlg; 	    //-a:見栄張り表示 5  フィードイン
_S byte gI8086Flg;  	    //CPUが8086(V30)か186以上か
_S int	gVsWaitCnt; 	    //VSYNCの信号をみて一定時間に何回ループできたか
_S int	gVsWaitCnt0;	    //-w[N] での N
_S int	gVsWaitCnt1;	    //-ws[N,M] での N
_S int	gVsWaitCnt2;	    //-ws[N,M] での M

_S byte g256col16pal[48] = {
     0, 0, 0,
     0, 0,15,
    15, 0, 0,
    15, 0,15,
     0, 5, 0,
     0, 5,15,
    15, 5, 0,
    15, 5,15,
     0,10, 0,
     0,10,15,
    15,10, 0,
    15,10,15,
     0,15, 0,
     0,15,15,
    15,15, 0,
    15,15,15
};
/*---------------------------------------------------------------------------*/
void TCls(void)
    // テキスト画面クリア
{
    if (gChk)
    	return;
    printf ("\x1b[2J\x1b[>5h\x1b[>1h");/*ﾃｷｽﾄ画面ｸﾘｱ;ｶｰｿﾙoff;PF表示off*/
}

void Exit(int n)
    // プログラム終了
{
    if (Gdc_ofsX || Gdc_ofsY) { //ＶRAM表示がGDCをいじったままなら連続化
    	int j;
    	j = 50;
    	while (--j > 0)
    	    WaitVsync();
    	Gdc_SclInit();
    }
    ActGPage(0);
    DspGPage(0);
    printf("\x1b[>5l\x1b[>1l");/*ｶｰｿﾙon;PF表示on*/
    exit(n);
}

void DosError(int n)
    // デバッグ用の表示&終了
{
    if (n >= 0)
    	printf("%s : Dos Function Error #%d\n",gPrgName,n);
    else
    	n = 1;
    Exit(n);
}

void DbgPutWord(int n)
    // デバッグ用の表示
{
    printf(" %d(%x) ",n,n);
}

void PriMsg(byte *s)
    // メッセージ表示
{
    printf("%s : %s",gPrgName,s);
}

void PriExit(byte *s)
    // メッセージを表示して終了(1)
{
    printf("%s : %s",gPrgName,s);
    Exit(1);
}

void DmyProc2(void)
    // 意味がないかもしれないが、空ループ用のダミー関数
{
    ;
}

void DmyProc(void)
    // 意味がないかもしれないが、空ループ用のダミー関数
{
    DmyProc2();
}

#if 1
#define VsyncWait(n)	delay(n)
#else
void VsyncWait(int n)
    // -w , @w での時間待ち
{
    long l;
    Key_BufClr();
    while (n) {
    	l = Key_Get();
    	if (l & 0xffff0000L) {
    	    l &= 0xFFFF;
    	    if (l == 0x001b||(l & 0xff) == 0x03)
    	    	Exit(1);
    	    else if (l == 0x1c0d || l == 0x3420)
    	    	return;
    	}
    	WaitVsync();
    	n--;
    }
}
#endif

void VsWaitInit(void)
    // -ws の時間待ちようルーチンの初期化
{
    gVsWaitCnt = (int)CntVsync();
    gVsWaitCnt2 = gVsWaitCnt1 = 0;
  #if 0
    if (gVsWaitCnt < 20)
    	PriExit("PrgErr:CntVsyncがおかしい\n");
    /*else if (gVsWaitCnt > 120)
    	gVsWaitCnt1 = 1;*/
  #endif
}

void VsWait(int n)
    // -wsN,M の N の値の時間待ち
{
    volatile long l;
    l = (long)gVsWaitCnt * (long)n;
    while (l) {
    	--l;
    	DmyProc();
    }
}

void VsWait2(void)
    // -wsN,M の M の値の時間待ち
{
    volatile long l;
    l = gVsWaitCnt * gVsWaitCnt2;
    l >>= 2;
    while (l) {
    	--l;
    	DmyProc();
    }
}

/*---------------------------------------------------------------------------*/

void GInit(void)
    // VRAMの状態の初期化
{
    union   REGS    regs;

    ActGPage(0);
    regs.h.ah = 0x42;
    if (gL200flg==1
    || (gL200flg==0&& PiLd_dotX && PiLd_dotY && (PiLd_dotX/PiLd_dotY >= 2))) {
    	regs.h.ch = 0x80;
    	Gv_btmLine = 200;
    	if (gSmlFlg)
    	    regs.h.ch = 0xC0;
    } else {
    	regs.h.ch = 0xC0;
    	Gv_btmLine = 400;
    }
    int86( 0x18, &regs, &regs);

    regs.h.ah = 0x40;
    int86( 0x18, &regs, &regs);
    outp( 0x6a, 1); //analog mode
    Gdc_SclInit();
}

void ToneDown(int n)
    // -cp での画面をトーンダウン（フィードアウト）
{
    int i;

    if (gPalTone > n) {
    	for (i = gPalTone; i >= n; i--) {
    	    RPal_ChgTone(i);
    	    if (i & 0x01)
    	    	WaitVsync();
    	}
    } else {
    	RPal_ChgTone(n);
    }
}

void ToneUp(int n)
    // -a5 での画面をトーンアップ（フィードイン）
{
    int i;

    for (i = 0; i <= n; i++) {
    	RPal_ChgTone(i);
    	if (i & 0x01)
    	    WaitVsync();
    }
}

void GCls(void)
    // グラフィック画面消去
{
    Gv_Cls();
    if (gUraUseMode) {
    	ActGPage(1);
    	Gv_Cls();
    	ActGPage(0);
    }
  #ifdef HF
    if (gHfMode) {
    	/*gHfMode =*/ Hf_Init();
    	Hf_Cls();
    	Hf_Term();
    }
  #endif
}

#if 0
void PriColTbl(void)
{
    if (gChk) {
    	byte *p;
    	int  l;
    	p = PiLd_colTbl;
    	for (l = 0;l < 256;l++) {
    	    if ((l % 16) == 0) {
    	    	printf("\n: ");
    	    }
    	    printf("%02x ",*p++);
    	}
    	printf("\n");
    }
}
#endif

#ifdef DEB_DMP
_S void MemDmp(void far *adr, dword len)
    // デバッグ用。指定アドレスのダンプを行なう
{
    byte huge *dp;
    dword l;

    dp = adr;
    dp++;dp--;
    for (l = 0;l < len;l++) {
    	if ((l % 20) == 0) {
    	    if ((l % 320) == 0) {
    	    	printf("\n");
    	    	if (gKeyWaitFlg) {
    	    	    if (getch() == 0x1b)
    	    	    	return;
    	    	}
    	    }
    	    printf("\n%08x ",l);
    	}
    	printf("%02x ",*dp++);
    }
    printf("\n");
}
#endif

void PriHdr(byte *name,int cf)
    // ヘッダ情報の表示
{
    typedef int (*pri_t)(char *, ...);
    pri_t pri[2] = {(pri_t)printf, (pri_t)cprintf};
    pri_t prf;
    static byte *crlf[2] = {"\n","\r\n"};
    int i,n;

    prf = pri[cf];
    prf("%s\r\n",name);
    prf("  ドット比: %d:%d",PiLd_dotX,PiLd_dotY);
    prf("  サイズ %d*%d",PiLd_sizX,PiLd_sizY0);
    prf("  省略時座標(%d,%d)-(%d,%d)",
    	PiLd_topX,PiLd_topY,PiLd_topX+PiLd_sizX-1,PiLd_topY+PiLd_sizY0-1);
    prf(crlf[cf]);
    prf("  セーバ名:%s",PiLd_macName);
    prf("  %d色(%dプレーン)", ((PiLd_plnCnt == 8) ? 256 : 16), PiLd_plnCnt);
    prf("  パレットモード:%02x",PiLd_palMode);
    prf(crlf[cf]);
    n = 16;
    if (cf == 0 && PiLd_plnCnt == 8)
    	n = 256;
    for (i=0; i < n; i++) {
    	prf("\t%02x: %02x %02x %02x",
    	    i,PiLd_pal256[i*3],PiLd_pal256[i*3+1],PiLd_pal256[i*3+2]);
    	if (i % (4) == 3)
    	    prf(crlf[cf]);
    }
}

void HdrErrMsg(int f)
{
    byte *msg[] = {
    	"Ｐｉファイルでない",
    	"256色画像は -lbu を指定しないと表示できない",
    	"この画像サイズには対応していない",
    	"画像が大きすぎる(メモリが足りない)",
    };
    PriMsg(msg[f-1]);
}

void Disp(int sx,int sy,int ovrf)
    // 画像を表示.
    // 実際の表示は Gv_Dsp???の関数が行なう
{
    int f;

    sx = -sx;
    sy = -sy;
    Gdc_SclInit();
    f = 0;
    if (gUraUseMode || gUraWrtMode) {
    	if (Gv_btmLine == 200)
    	    f = 1;
    	Gv_btmLine = 400;
    }
    if (ovrf>0) {
    	Gv_DspOvrLd(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy,gOvrLdCol);
    } else if (gMieMode) {
    	Gv_DspP(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy,gMieMode);
    } else if (PiLd_sizX == 640 && sx == 0 && sy == 0) {
    	Gv_Dsp640(PiLd_wkBuf2,PiLd_sizY);
    } else {
    	Gv_Dsp(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy);
    }
    if (gUraUseMode && sx == 0 && sy == 0) {
    	ActGPage(1);
    	if (f == 0)
    	    DspGPage(1);
    	if(gUraUseMode == 1)
    	    sy -= 400;
    	else if (gUraUseMode == 2)
    	    sx -= 640;
    	if (ovrf>0) {
    	    Gv_DspOvrLd(PiLd_wkBuf2,
    	    	PiLd_sizX,PiLd_sizY,sx,sy,gOvrLdCol);
    	} else if (gMieMode) {
    	    Gv_DspP(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy,gMieMode);
    	} else {
    	    Gv_Dsp(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy);
    	}
    	ActGPage(0);
    	if (f == 0)
    	    DspGPage(0);
    }
    if (f)
    	Gv_btmLine = 200;
}

void DispL(int sx,int sy)
    // 画像の表示。スクロールで不連続になったVRAM状態を初期化して
    // 指定された位置の画像を表示
{
    int f,sizx;

    if (PiLd_sizX < 640 && PiLd_sizY < Gv_btmLine) {
    	Disp(sx<<3,sy,0);
    	return;
    }
    Gdc_SclInit();
    if (sx < 0 || sy < 0)
    	sx = sy = 0;
    f = 0;
    if (gUraUseMode || gUraWrtMode) {
    	if (Gv_btmLine == 200)
    	    f = 1;
    	Gv_btmLine = 400;
    }
    sizx=PiLd_sizX>>3;
    Gv_WkCpy(PiLd_wkBuf2,sizx, PiLd_sizY,sx, sy,
    	80,400,0,0);
    if (gUraUseMode) {
    	ActGPage(1);
    	if (f == 0)
    	    DspGPage(1);
    	if(gUraUseMode == 1)
    	    sy += 400;
    	else if (gUraUseMode == 2)
    	    sx += 80;
    	Gv_WkCpy(PiLd_wkBuf2, sizx, PiLd_sizY,sx%sizx, sy%PiLd_sizY,
    	    80,400,0,0);
    	ActGPage(0);
    	if (f == 0)
    	    DspGPage(0);
    }
    if (f)
    	Gv_btmLine = 200;
}


/*---------------------------------------------------------------------------*/
void KeyUsage(int cf)
{
    byte *keyUsageMsg[] = {
    	"ESC      DoPI を終了",
    	"HELP     この説明(消すには もう一度HELPを押すか TAB を押す)",
    	"TAB =    ヘッダ情報､ｺﾒﾝﾄ の表示",
    	"CR SPC   次の画像",
    	"HOME/CLR 始点を(0,0)にして再表示(-lbの影響あり)",
    	"↑↓←→ 画面に表示できなかった部分を表示(スクロール)",
    	"         スクロールは画面範囲を越える画像で始点が(0,0)のときのみ可",
    	"0～6     スクロールの移動量",
    	"         0:1ﾄﾞｯﾄ 1:2ﾄﾞｯﾄ 2:4ﾄﾞｯﾄ 3:8ﾄﾞｯﾄ 4:16ﾄﾞｯﾄ 5:32ﾄﾞｯﾄ  6:64ﾄﾞｯﾄ",
    	"INS      パレットのトーンを10％アップ(0-200%)",
    	"DEL      パレットのトーンを10％ダウン(0-200%)",
    	"/        画面丁度か範囲を越える画像なら、スクロールでループする",
    	"-        そのままの座標で再表示(-lb指定時のみ. 裏画面を描き直す)",
    	"[        上下反転（メインメモリ上のデータに対して行う）",
    	"]        左右反転（　　　　　　　〃                  ）",
    	"\\        スクロール方法の切替(GDC利用/ｿﾌﾄｳｪｱ転送)",
    	",        縮小表示(1/4). 何かキーを押せば戻る",
    	"^        画面が400lineの時、裏画面を見る. 何かキーを押せば戻る",
    	"@        現在のVRAMを内部バッファに取り込む",
    	NULL
    };
    typedef int (*pri_t)(char *, ...);
    pri_t pri[2] = {(pri_t)printf, (pri_t)cprintf};
    pri_t prf;
    byte *crlf[2] = {"\n","\r\n"};
    byte **p;

    prf = pri[cf];
    prf("%s でキー入力時に使えるキー",gNameVer);
    prf(crlf[cf]);
    for (p = keyUsageMsg; *p != NULL; p++) {
    	prf(*p);
    	prf(crlf[cf]);
    }
}

int KeyMode(byte *name,int sx, int sy)
    // キー入力
{
    int  i,f,g,xx,yy,mode,xsiz,tone;
    word c;
    //int  sav_cbrk;
    static int ddy = 16;
    static byte *keyGmsg[] = {
    	"640*400","640*800","1280*400","400*640","800*640","400*1280"
    };

    //sav_cbrk = getcbrk();
    //setcbrk(0);
    gVRamGetNo = 0;
    tone = gPalTone;
    xsiz = PiLd_sizX>>3;
    sx >>= 3;
    f = (gFx && (xsiz - sx >= 80));
    g = (gFy && (PiLd_sizY - sy >= Gv_btmLine));
    xx = (xsiz >= 80) ? 80 : xsiz;
    yy = (PiLd_sizY >= Gv_btmLine) ? Gv_btmLine : PiLd_sizY;
    mode = 0;
    if ((sx && g && !f) || (sy && f && !g))
    	f = g = 0;
    for (;;) {
    	if (mode) {
    	    gotoxy(1,25);
    	    cprintf("(%3d,%3d) Skip:%d  tone:%3d%%   %s    ",
    	    	sx*8,sy,ddy,tone,(gSclLoopFlg) ? "loop":"    ");
    	}

    	Gv_ofsFlg = 0;
    	if (gGdcSclFlg && Gv_btmLine != 200) {	//GDCスクロール
    	    Key_BufClr();
    	    c = pc98key(0x04,0x07);
    	    if (c & 0x04) { //UP
    	    	if (g == 0)
    	    	    continue;
    	    	i = ddy;
    	    	do {
    	    	    if (--sy < 0) {
    	    	    	if (gSclLoopFlg == 0) {
    	    	    	    sy = 0;
    	    	    	    goto JJ0;
    	    	    	}
    	    	    	sy += PiLd_sizY;
    	    	    }
    	    	    Gdc_Scl(0,-1);
    	    	    Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx, sy,xx, 1,
    	    	    	Gdc_ofsX, Gdc_ofsY);
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    } else if (c & 0x20) { //DOWN
    	    	if (g == 0)
    	    	    continue;
    	    	i = ddy;
    	    	do {
    	    	    if (sy + Gv_btmLine >= PiLd_sizY && gSclLoopFlg == 0) {
    	    	    	sy = PiLd_sizY - Gv_btmLine;
    	    	    	goto JJ0;
    	    	    }
    	    	    Gdc_Scl(0,1);
    	    	    Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx,
    	    	    	(sy+Gv_btmLine)%PiLd_sizY,  xx, 1,
    	    	    	Gdc_ofsX, (Gdc_ofsY + Gv_btmLine - 1)%400);
    	    	    if (++sy >= PiLd_sizY)
    	    	    	sy -= PiLd_sizY;
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    } else if (c & 0x10) { //Right
    	    	if (f == 0)
    	    	    goto JJ0;
    	    	i = ddy >> 3;
    	    	if (i < 2||((sx & 0x01) == 0))
    	    	    i = 2;
    	    	do {
    	    	    if (sx + 80 >= xsiz && gSclLoopFlg == 0) {
    	    	    	sx = xsiz - 80;
    	    	    	goto JJ0;
    	    	    }
    	    	    Gdc_Scl(1,0);
    	    	    Gv_ofsFlg = (Gdc_ofsX != 0);
    	    	    Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,
    	    	    	(sx + 80)%xsiz, sy, 1, yy,
    	    	    	(Gdc_ofsX + 80 - 1)%80, Gdc_ofsY);
    	    	    Gv_ofsFlg = 0;
    	    	    sx += 1;
    	    	    if (sx >= xsiz)
    	    	    	sx -= xsiz;
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    } else if (c & 0x08) { //Left
    	    	if (f == 0)
    	    	    continue;
    	    	i = ddy >> 3;
    	    	if (i < 2||((sx & 0x01) == 0))
    	    	    i = 2;
    	    	do {
    	    	    sx -= 1;
    	    	    if (sx < 0) {
    	    	    	if (gSclLoopFlg == 0) {
    	    	    	    sx = 0;
    	    	    	    goto JJ0;
    	    	    	}
    	    	    	sx += xsiz;
    	    	    }
    	    	    Gdc_Scl(-1,0);
    	    	    Gv_WkCpy(PiLd_wkBuf2,xsiz,PiLd_sizY,
    	    	    	sx,sy,1,yy,Gdc_ofsX,Gdc_ofsY);
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    JJ0:
    	    	continue;
    	    }
    	    c = Key_GetWait();
    	} else {    //ソフトウェア転送
    	    int dd,ddx;

    	    Key_BufClr();
    	    c = Key_GetWait();
    	    switch (c >> 8) {
    	    case 0x3A://UP
    	    	if (g == 0)
    	    	    break;
    	    	if (gSclLoopFlg) {
    	    	    dd = ddy;
    	    	} else if (sy > 0) {
    	    	    dd = (sy > ddy) ? ddy : sy;
    	    	} else
    	    	    break;
    	    	sy -= dd;
    	    	if (sy < 0)
    	    	    sy += PiLd_sizY;
    	    	Gv_GDwn(dd);
    	    	Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx, sy, xx, dd,
    	    	    Gdc_ofsX, Gdc_ofsY);
    	    	break;
    	    case 0x3D://Dwn
    	    	if (g == 0)
    	    	    break;
    	    	if (gSclLoopFlg) {
    	    	    dd = ddy;
    	    	} else {
    	    	    dd = PiLd_sizY - (sy + Gv_btmLine);
    	    	    if (dd < 0)
    	    	    	break;
    	    	    if (dd > ddy)
    	    	    	dd = ddy;
    	    	}
    	    	Gv_GUp(dd);
    	    	Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx,
    	    	    (sy+Gv_btmLine)%PiLd_sizY, xx, dd,
    	    	    Gdc_ofsX, (Gdc_ofsY + Gv_btmLine - dd)%400);
    	    	sy += dd;
    	    	if (sy >= PiLd_sizY)
    	    	    sy -= PiLd_sizY;
    	    	break;
    	    case 0x3C://Rgt
    	    	if (f == 0)
    	    	    break;
    	    	ddx = (ddy < 8) ? 1 : (ddy >> 3);
    	    	if (gSclLoopFlg) {
    	    	    dd = ddx;
    	    	} else if (sx + 80 < xsiz) {
    	    	    dd = xsiz - (sx + 80);
    	    	    if (dd > ddx)
    	    	    	dd = ddx;
    	    	} else
    	    	    break;
    	    	Gv_GLft(dd);
    	    	Gv_ofsFlg = (Gdc_ofsX != 0);
    	    	Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,
    	    	    (sx + 80)%xsiz, sy, dd, yy,
    	    	    (Gdc_ofsX + 80 - dd)%80, Gdc_ofsY);
    	    	sx += dd;
    	    	if (sx >= xsiz)
    	    	    sx -= xsiz;
    	    	break;
    	    case 0x3B://Lft
    	    	if (f == 0)
    	    	    break;
    	    	ddx = (ddy < 8) ? 1 : (ddy >> 3);
    	    	if (gSclLoopFlg) {
    	    	    dd = ddx;
    	    	} else if (sx > 0) {
    	    	    dd = sx;
    	    	    if (dd > ddx)
    	    	    	dd = ddx;
    	    	} else
    	    	    break;
    	    	sx -= dd;
    	    	if (sx < 0)
    	    	    sx += xsiz;
    	    	Gv_GRgt(dd);
    	    	Gv_WkCpy(PiLd_wkBuf2,xsiz,PiLd_sizY,
    	    	    sx,sy,dd,yy,Gdc_ofsX,Gdc_ofsY);
    	    	break;
    	    default:
    	    	goto JJ2;
    	    }
    	    continue;
    	}

    JJ2:
    	switch (c >> 8) {
    	case 0x00://ESC 00:1B
    	    //if (Gdc_ofsY||Gdc_ofsX) {
    	    //	DispL(sx,sy);
    	    //}
    	    c = -1;
    	    goto LOOP_OUT;
    	case 0x1C://CR
    	case 0x34://SPC
    	    //if (Gdc_ofsY||Gdc_ofsX) {
    	    //	DispL(sx,sy);
    	    //}
    	    c = 0;
    	    goto LOOP_OUT;
    	case 0x3E://CLR 3E:00 (1A)
    	    sx=sy=0;
    	    GCls();
    	    Disp(0,0,0);
    	    f = (gFx && (xsiz - sx >= 80));
    	    g = (gFy && (PiLd_sizY - sy >= Gv_btmLine));
    	    break;
    	case 0x3F://HELP
    	    if (mode != 3) {
    	    	mode = 3;
    	    	RPal_ChgTone(50);
    	    	//RPal_Set(50,PiLd_pal);
    	    	TCls();
    	    	KeyUsage(1);
    	    	break;
    	    }
    	case 0x0F://TAB
    	TAB:
    	    ++mode;
    	    if (mode >= 3)
    	    	mode = 0;
    	    switch (mode) {
    	    case 0:
    	    	RPal_Set(tone,PiLd_pal);
    	    	TCls();
    	    	break;
    	    case 1:
    	    	RPal_ChgTone(50);
    	    	//RPal_Set(50,PiLd_pal);
    	    	TCls();
    	    	PriHdr(name,1);
    	    	if (*PiLd_commentBuf) {
    	    	    cprintf("\r\nコメントあり");
    	    	} else {
    	    	    cprintf("\r\nコメントなし");
    	    	    ++mode;
    	    	}
    	    	break;
    	    case 2:
    	    	if (*PiLd_commentBuf) {
    	    	    RPal_ChgTone(50);
    	    	    //RPal_Set(50,PiLd_pal);
    	    	    TCls();
    	    	    printf("%Fs",PiLd_commentBuf);
    	    	}
    	    }
    	    break;
    	case 0x38://INS
    	    tone += 10;
    	    if (tone > 200)
    	    	tone = 200;
    	    if (mode == 0)
    	    	RPal_ChgTone(tone);
    	    break;
    	case 0x39://DEL
    	    tone -= 10;
    	    if (tone < 0)
    	    	tone = 0;
    	    if (mode == 0)
    	    	RPal_ChgTone(tone);
    	    break;
    	default:
    	    goto JJ4;
    	}
    	continue;

    JJ4:
    	c = c & 0xff;
    	c = toupper(c);
    	switch(c) {
    	case '0':ddy = 1;   break;
    	case '1':ddy = 2;   break;
    	case '2':ddy = 4;   break;
    	case '3':ddy = 8;   break;
    	case '4':ddy = 16;  break;
    	case '5':ddy = 32;  break;
    	case '6':ddy = 64;  break;
    	case '=':
    	    goto TAB;
    	case '-':
    	    DispL(sx,sy);
    	    break;
    	case '/':
    	    if (f == 0 && g == 0)
    	    	break;
    	    ++gSclLoopFlg;
    	    gSclLoopFlg &= 0x01;
    	    if (gSclLoopFlg == 0
    	    	&& (sx + 80 >= xsiz ||sy+Gv_btmLine > PiLd_sizY)) {
    	    	sx=sy=0;
    	    	Disp(0,0,0);
    	    }
    	    break;
    	case '\\':
    	    if (f == 0 && g == 0)
    	    	break;
    	    ++gGdcSclFlg;
    	    gGdcSclFlg &= 0x01;
    	    //sx = sy = 0;
    	    if (sx & 0x01) {
    	    	if (sx > 0)
    	    	    --sx;
    	    	else
    	    	    ++sx;
    	    }
    	    DispL(sx,sy);
    	    break;
    	case '['://ﾀﾃ反転
    	    Gv_RevY(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY);
    	    sx = sy = 0;
    	    DispL(sx,sy);
    	    break;
    	case ']'://ﾖｺ反転
    	    Gv_RevX(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY);
    	    sx = sy = 0;
    	    DispL(sx,sy);
    	    break;
    	/*case '':
    	    Gv_RePos(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,sx,sy);
    	    sx = sy = 0;
    	    DispL(sx,sy);
    	    break;*/
    	case ','://縮小表示
    	    gSmlFlg = 1;
    	    GInit();
    	    GCls();
    	    Gv_DspSml(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY);
    	    Key_GetWait();
    	    gSmlFlg = 0;
    	    GInit();
    	    DispL(sx,sy);
    	    break;
    	case '^'://裏vramを覗く
    	    if (Gv_btmLine != 200) {
    	    	ActGPage(1);
    	    	DspGPage(1);
    	    	Key_GetWait();
    	    	ActGPage(0);
    	    	DspGPage(0);
    	    }
    	    break;
    	case '@':
    	    RPal_ChgTone(50);
    	    TCls();
    	    cprintf("取り込むサイズを番号1～6で指定."\
    	    	    " キャンセルは他のキー. (4～6は90度右に回転)\r\n");
    	    cprintf("    1:640*400  2:640*800  3:1280*400  4:400*640"\
    	    	    "  5:800*640  6:400*1280\r\n");
    	    c = 0xff & Key_GetWait();
    	    TCls();
    	    if (c >= '1' && c <= '6') {
    	    	DispL(sx,sy);
    	    	cprintf("%s",keyGmsg[c-'1']);
    	    	gVRamGetNo = c - '0';
    	    	c = 0;
    	    	goto LOOP_OUT;
    	    } else {
    	    	RPal_ChgTone(tone);
    	    }
    	    break;
    	}/* end of switch */
    } /* InKey Loop */

 LOOP_OUT:
  #if 1
    if ((Gdc_ofsY||Gdc_ofsX)) {
    	DispL(sx,sy);
    }
  #endif
    //setcbrk(sav_cbrk);
    return c;
}

/*---------------------------------------------------------------------------*/

void DispInit(void)
    // 画像ごとの画面の初期化. 座標、範囲の設定.
{
    GInit();
    RPal_Set(gPalTone,PiLd_pal);
    gUraUseMode = 0;
    if (gUraWrtMode && gSmlFlg == 0) {
    	if (gUraWrtMode > 1 && PiLd_sizX > 640)
    	    gUraUseMode = 2;
    	else if (PiLd_sizY > 400)
    	    gUraUseMode = 1;
    }
  #ifdef MP
    if (g256mode == 386 && PiLd_plnCnt == 8)
    	gUraUseMode = 3;
  #endif
    switch (gBxByMode) {
    case 1:
    	gSx = -gBx;
    	gSy = -gBy;
    	break;
    case 2:
    	gSx = -(PiLd_topX+gBx);
    	gSy = -(PiLd_topY+gBy);
    default:
    	gSx = -PiLd_topX;
    	gSy = -PiLd_topY;
    }
    gFx = (PiLd_sizX > 640) ? 2 : (PiLd_sizX == 640) ? 1 : 0;
    gFy = (PiLd_sizY > Gv_btmLine) ? 2 : (PiLd_sizY == Gv_btmLine) ? 1 : 0;
    if (PiLd_sizX - gSx < 0)
    	gSx = PiLd_sizX - 640;	//gSx = (gFx) ? (PiLd_sizX - 640):0;
    if (PiLd_sizY - gSy < 0)
    	gSy = PiLd_sizY-Gv_btmLine;//gSy = (gFy) ? (PiLd_sizY - Gv_btmLine):0;
}

void MemErr(void)
{
    printf("メモリが足りません\n");
    Exit(1);
}

void VramGetDisp(int n)
    // -kq[N] での画像取込み＆表示
{
    static word dmy;
    static int	xx[7] = {0,640,640,1280,400,800, 400};
    static int	yy[7] = {0,400,800, 400,640,640,1280};
    static int	ura[7]= {0,  0,  1,   2,  0,  1,   2};

  J0:
    gUraWrtMode = ura[n];
    PiLd_wkBuf2 = PiLd_wkBuf + xx[n];
    PiLd_sizX = xx[n];
    PiLd_sizY = yy[n];
    if ( (dword)PiLd_sizX*PiLd_sizY/2 +(dword)PiLd_sizX*2 > PiLd_wkBufMaxS*16L)
    	MemErr();
    PiLd_topX = PiLd_topY = 0;
    PiLd_plnCnt = 4;
    PiLd_palMode = 0;
    PiLd_dotX = PiLd_dotY = 0;
    strcpy(PiLd_macName,"DoPI");
    if (RPal_Get(&dmy,PiLd_pal)) {
    	PriExit("常駐パレットが組み込まれていません");
    }
    if (n < 4)
    	Gv_GetVRam(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,n);
    else
    	Gv_GetVRamQ(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY>>3,n - 4);
    DispInit();
    Disp(gSx,gSy,0);
    TCls();
    gVRamGetNo = 0;
    KeyMode("画面取り込み",gSx,gSy);
    gUraLoadFlg = gOvrLdFlg = 0;
    if (gClsFlg == 1)
    	gClsFlg = 0;
    if (gKeyWaitFlg == 1)
    	gKeyWaitFlg = 0;
    if (gVRamGetNo) {
    	n = gVRamGetNo;
    	goto J0;
    }
}

int KeyWaitSml(void)
{
    if (gKeyWaitFlg) {
    	int c;
    	do {
    	    c = getch();
    	    if (c == 0x1b)
    	    	return 1;
    	} while (c != ' ' && c != 0x0d);
    } else {
    	if (gVsWaitCnt0)
    	    VsyncWait(gVsWaitCnt0);
    }
    return 0;
}

#if defined(HF)||defined(MP)
int KeyWaitHF(void)
{
    static ddy = 64;

    //cprintf("*");
    if (gKeyWaitFlg) {
    	word c;
    	for (; ;) {
    	    Key_BufClr();
    	    c = Key_GetWait();
    	    //cprintf("%04x ",c);
    	    switch(c >> 8) {
    	    case 0x3a://UP
    	    	gSy -= ddy;
    	    	if (gSy < 0)
    	    	    gSy = 0;
    	    	return -1;
    	    case 0x3d://DOWN
    	    	if (PiLd_sizY > 400) {
    	    	    gSy += ddy;
    	    	    if (gSy > PiLd_sizY - 400)
    	    	    	gSy = PiLd_sizY - 400;
    	    	}
    	    	return -1;
    	    case 0x3c://RIGHT
    	    	if (PiLd_sizX > 640) {
    	    	    gSx += ddy;
    	    	    if (gSx > PiLd_sizX - 640)
    	    	    	gSx = PiLd_sizX - 640;
    	    	}
    	    	return -1;
    	    case 0x3b://LEFT
    	    	gSx -= ddy;
    	    	if (gSx < 0)
    	    	    gSx = 0;
    	    	return -1;
    	    case 0x00://ESC 00:1B
    	    	return 1;
    	    case 0x3E://CLR 3E:00 (1A)
    	    	GCls();
    	    	return -2;
    	    case 0x1C://CR
    	    case 0x34://SPC
    	    	return 0;
    	    }
    	    switch(c&0xff) {
    	    case '0':ddy = 1;	break;
    	    case '1':ddy = 2;	break;
    	    case '2':ddy = 4;	break;
    	    case '3':ddy = 8;	break;
    	    case '4':ddy = 16;	break;
    	    case '5':ddy = 32;	break;
    	    case '6':ddy = 64;	break;
    	    case '7':ddy = 128; break;
    	    case '8':ddy = 256; break;
    	    }
    	}
    } else {
    	if (gVsWaitCnt0)
    	    VsyncWait(gVsWaitCnt0);
    }
    return 0;
}
#endif

int  PiDisp(byte *loadName)
    // 画面の表示
{
    int xx;

    xx = PiLd_sizX>>3;
    xx = (xx >= 80) ? 80 : xx;
  #ifdef HF
    if (gEx256Flg == 1) {
    	Hf_Term();
    }
  #endif
  #ifdef MP
    if (gEx256Flg == 2) {
    	Mp_Term();
    }
  #endif
  #if defined(HF)||defined(MP)
    gEx256Flg = 0;
  #endif
    if (gUraLoadFlg == 0) { // 表画面に表示する
    	if (gToneDownFlg)
    	    ToneDown(0);
    	if (gClsFlg)
    	    GCls();
    	DispInit();
    	if (gSmlFlg) {//縮小表示が指定されているとき
    	    if (gToneUpFlg)
    	    	RPal_ChgTone(0);
    	    Gv_DspSml(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY);
    	    TCls();
    	    if (gToneUpFlg)
    	    	ToneUp(gPalTone);
    	    if (KeyWaitSml())
    	    	return 1;
  #ifdef    HF
    	} else if (PiLd_plnCnt == 8 && gHfMode) {
    	    int c,sx,sy;
    	    TCls();
    	    Gv_Cls();
    	    if (gUraUseMode) {
    	    	ActGPage(1);
    	    	Gv_Cls();
    	    	ActGPage(0);
    	    }
    	    /*gHfMode =*/ Hf_Init();
    	    gEx256Flg = 1;
    	    sx = sy = 0x7fff;
    	    c = -2;
    	    do {
    	    	if (c != -1 ||(sx != gSx || sy != gSy)) {
    	    	    Hf_Dsp(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, gSx, gSy);
    	    	    sx = gSx;sy = gSy;
    	    	}
    	    } while ((c = KeyWaitHF()) < 0);
    	    return c;
  #endif
  #ifdef    MP
    	} else if (PiLd_plnCnt == 8 && g256mode == 386) {
    	    int     c,bx,by,yy,sx,sy;
    	    TCls();
    	    Mp_Init();
    	    Mp_SetPal(PiLd_pal256);
    	    if (gSx >= 0) {
    	    	bx = 0;
    	    } else {
    	    	bx = -(gSx>>3),gSx = 0;
    	    }
    	    if (gSy >= 0) {
    	    	by = 0;
    	    } else {
    	    	by = -gSy,gSy = 0;
    	    }
    	    gEx256Flg = 2;
    	    sx = sy = 0x7fff;
    	    c = -2;
    	    GCls();
    	    do {
    	    	if (c != -1 || (sx != gSx || sy != gSy)) {
    	    	    yy = (PiLd_sizY >= 400) ? 400 : PiLd_sizY;
    	    	    Mp_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,
    	    	    	gSx>>3, gSy, xx,/*400*/yy, bx%80,by%Gv_btmLine);
    	    	    sx = gSx;sy = gSy;
    	    	    bx = by = 0;
    	    	}
    	    } while ((c = KeyWaitHF()) < 0);
    	    return c;
  #endif
    	} else {
    	    if (gMie2Mode && gSx == 0 && gSy == 0 && gUraWrtMode == 0
    	    	&& gOvrLdFlg <= 0 && gGdcSclFlg && Gv_btmLine != 200
    	    	&& PiLd_sizY >= Gv_btmLine) {//見栄張りモードときのスクロール
    	    	int i;
    	    	if (gMie2Mode == 1) {
    	    	    for (i = 0; i < Gv_btmLine; i++) {
    	    	    	Gdc_Scl(0,1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }
    	    	} else {
    	    	    for (i = PiLd_sizY; --i >= 0/*Gv_btmLine*/;) {
    	    	    	Gdc_Scl(0,-1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i % Gv_btmLine);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }
    	    	    /*for (i = PiLd_sizY-Gv_btmLine; --i >= 0;) {
    	    	    	Gdc_Scl(0,-1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }*/
    	    	}
    	    } else {//普通に表示
    	    	if (gToneUpFlg)
    	    	    RPal_ChgTone(0);
    	    	Disp(gSx,gSy,gOvrLdFlg);
    	    	if (gToneUpFlg)
    	    	    ToneUp(gPalTone);
    	    }
    	    if (/*gKeyWaitFlg == 0 &&*/ gSx == 0 && gSy == 0 && gUraWrtMode==0
    	    	&& gOvrLdFlg <= 0 && PiLd_sizY > Gv_btmLine && gGdcSclFlg
    	    	&& Gv_btmLine != 200 && gMie2Mode != 2) {
    	    	    //400ライン以上の画像のときのスクロール
    	    	int i;
    	    	for (i = Gv_btmLine; i < PiLd_sizY; i++) {
    	    	    Gdc_Scl(0,1);
    	    	    Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	xx, 1, 0, (i - Gv_btmLine)%Gv_btmLine);
    	    	    	//xx, 1, Gdc_ofsX, Gdc_ofsY);
    	    	    VsWait(gVsWaitCnt1);
    	    	}
    	    	if (gSclBakFlg) {
    	    	    for (i = 0;i < 6;i++)
    	    	    	WaitVsync();
    	    	    for (i = PiLd_sizY-Gv_btmLine; --i >= 0;) {
    	    	    	Gdc_Scl(0,-1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i % Gv_btmLine);
    	    	    	    //xx, 1, Gdc_ofsX, Gdc_ofsY);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }
    	    	} else {
    	    	    gSy = Gv_btmLine;
    	    	}
    	    }
    	    TCls();
    	    if (gKeyWaitFlg) {	//キー入力モード
    	    	if (KeyMode(loadName,gSx,gSy))
    	    	    return 1;
    	    } else {
    	    	if (gVsWaitCnt0)    //表示後の時間待ち
    	    	    VsyncWait(gVsWaitCnt0);
    	    }
    	}
    } else {	// -b filename での裏VRAMへのロード
    	DispInit();
    	DspGPage(1);
    	ActGPage(1);
    	if (gClsFlg)
    	    Gv_Cls();
    	if (gSmlFlg)
    	    Gv_DspSml(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY);
    	else
    	    Disp(gSx,gSy,gOvrLdFlg);
    	TCls();
    	DspGPage(0);
    	ActGPage(0);
    }
    gUraLoadFlg = gOvrLdFlg = 0;
    if (gClsFlg == 1)
    	gClsFlg = 0;
    if (gKeyWaitFlg == 1)
    	gKeyWaitFlg = 0;
    return 0;
}

int PiLoad(byte *name)
{
    FILE *fp;
    int f;
    int c256mode;

    fp = fopen(name,"rb");
    if (fp == NULL) {
    	printf("%s はオープンできませんでした\n",name);
    	return 1;
    }
    if (gRdBufVRamFlg)
    	ActGPage(1);
    PiLd_RdFirst(fileno(fp));
    PiLd_filSiz = filelength(PiLd_ph);
    PiLd_ColTblInit(); /* PriColTbl();*/
    f = PiLd_ReadHdr(); // ヘッダを読み込み設定
    c256mode = 0;
    if (PiLd_plnCnt == 8) {
    	if (gRdBufVRamFlg) {
    	    c256mode = g256mode+1;
  #ifdef HF
    	    if (gHfMode)
    	    	c256mode = -1;
  #endif
    	} else
    	    f = 2;
    }
    if (PiLd_bcol && gOvrLdFlg2 == 0) {
    	gOvrLdFlg = 1;
    	gOvrLdCol = PiLd_bcol - 1;
    }
    if (f != 1 && gInfoFlg) {	// -i でのヘッダ情報の表示のみのとき
    	PriHdr(name,0);
    	if (*PiLd_commentBuf)
    	    printf("%Fs\n\n",PiLd_commentBuf);
    	else
    	    printf("\n");
    	fclose(fp);
    	return 1;
    }
    if (f) {
    	HdrErrMsg(f);
    	return 1;
    }
    TCls();
    if (gTxtFlg) {
    	printf("%s",name);
    	if (*PiLd_commentBuf) {
    	    if (strncmp(PiLd_commentBuf, "User:", 5) == 0) {
    	    	printf (" : %Fs\n", &PiLd_commentBuf[5]);
    	    } else {
    	    	printf ("\n%Fs\n", PiLd_commentBuf);
    	    }
    	} else {
    	    printf("\n");
    	}
    }
    if (gChk) {
    	PriHdr(name,0);
    }
    if (PiLd_sizY != PiLd_sizY0) {
    	printf("\nメモリが確保できないので %d 行までしか表示できません\n",PiLd_sizY);
    }
  #ifdef PILD4C
    if (gPiLd4cFlg)
    	PiLd_Load4c();
    else
  #else
    // Piデータを展開
    if (c256mode == 0) {
    	PiLd_Load4a();
    } else {
    	PiLd_Load8();
    }
    if (gChk)
    	printf("PiLd end\n");
  #endif
    fclose(fp);
  #ifdef DEB_DMP
    if (gDmpFlg) {
    	MemDmp(PiLd_wkBuf, 1024);
    	MemDmp(PiLd_wkBuf2, 1024);
    	//MemDmp(PiLd_wkBuf, PiLd_wkSiz);
    }
  #endif
    // 展開したデータをさらに表示しやすいように変換
    switch(c256mode) {
    case 0:
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4a(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY);
    	break;
    case 1:
    	Decol_Dit16(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, PiLd_pal256);
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4b(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	memcpy(PiLd_pal,g256col16pal,16*3);
    	break;
    case 2:
    case 3:
    	Decol_Mono16(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, PiLd_pal256,
    	    	    PiLd_wkBuf, PiLd_sizX * 2, g256mode-1);
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4b(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	break;
    case 4:
    	Decol_Col16(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, PiLd_pal256,
    	    	    (void far *)0xA8000000L, PiLd_sizX * 2 * 4 * 3);
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4b(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	memcpy(PiLd_pal,g256col16pal,16*3);
    	break;
  #ifdef    MP
    case 386+1: //PC386M&P
    	Mp_Cnv(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	break;
  #endif
    default:
    	;   	//HF(SF)
    }

  #ifdef DEB_DMP
    if (gDmpFlg)
    	MemDmp(PiLd_wkBuf, PiLd_wkSiz);
  #endif
    if (gChk)
    	printf("Gv_Cnv4 end\n");
    return 0;
}

/*---------------------------------------------------------------------------*/
_S void OptsRngErr(byte *s)
{
    printf("-%s オプションで指定できない値が指定されている\n",s);
    Exit(1);
}

_S void OptsErr(byte *s)
{
    printf("%s オプションの指定がおかしい\n",s);
    Exit(1);
}


_S void Usage(void)
{
    printf("use: dopi [-opts] file(s) ／ %s By てんか☆  Pi作者:やなぎさわ氏\n",gNameVer);
    pr("・コマンドラインの先頭から順番に実行. オプション、ファイルの順番に注意.\n");
    pr("・裏VRAMを作業用メモリとして使用するので注意.\n");
    pr("・キー入力待ちでHELPキーを押すと、使えるキーの説明を表示.\n");
    pr("@FILE    FILE よりオプション・ファイル名を収得\n");
    //pr("-??	   その他のオプション(-a,-cp,-i,-kg,-lbu,-ll,-ly,-w,-ws,-wh)のヘルプ\n");
    pr("-?k      キー入力待ちの時に使えるキーの表示\n");
    pr("-k[0|1]  表示毎にキー入力待ち. -k1 なら１度だけ. -k0 でオフ\n");
    pr("-c[0|1]  表示に先だって毎回画面消去. -c1 ならそのとき１度だけ. -c0でオフ\n");
    pr("-t[1]    ファイル名、コメントを表示しない. -t1 で表示する\n");
    pr("-v[N]    パレットのトーン. N:0～200%%\n");
    pr("-lv[N]   終了時のパレットのトーン. N:0～200%%\n");
    pr("-lvf     終了時に画面表示オフ\n");
    pr("-i       ヘッダ情報を表示(画像表示無)\n");
    pr("-b  　　 直後のファイルを裏画面に表示\n");
    pr("-lb[N]   巨大画像のとき裏画面に表示. N:= 0:しない 1:縦を優先 2:横を優先\n");
    pr("-lr[0]   縮小表示(１／４). 始点無視. -lr0 が指定されるとオフ\n");
    pr("-lm[0-3] 256色画像の表示方法. 0:ﾃﾞｨｻﾞ 1:手抜ﾓﾉｸﾛ 2:誤差拡散ﾓﾉｸﾛ 3:誤差拡散ｶﾗｰ\n");
    pr("-o[X,Y]  画面の表示開始位置. 画像データの始点情報との合計\n");
    pr("-oa[X,Y] 画面の表示開始位置. 画像データの始点情報は無視\n");
    pr("         X:-1300位～632 かつ 8ドット単位. Y:-1300位～399\n");
    pr("         負の値のばあい、負の部分は表示されず 0からの部分を表示\n");
    pr("         座標値の後ろに'o'を付けるとバイト単位で指定(例:-o11o,4 なら -o88,4)\n");
    {
    	pr("[more]");
    	getch();
    	pr("\b\b\b\b\b\b\      \b\b\b\b\b\b");
    }
    pr("-a[0-5]  見栄表示(^_^?; -a0:オフ 1-5:試してね\n");
    pr("-cp      -c と同じだが、消去前にパレットをトーン・ダウン\n");
    pr("-kg[N]   現在のVRAMをバッファに取り込んでキー入力待ち\n");
    pr("         N= 1:640*400 2:640*800 3:1280*400 4:400*640 5:800*640 6:400*1280\n");
    pr("-lbu0    裏VRAMを作業バッファに使用しない（256色画像をロードできなくなる）\n");
    pr("-lc[C]   直後のファイルの色C を透明色扱い.C:0～15.(16色画像のみ)\n");
    pr("         -lc- 省略時透明色を強制的に禁止\n");
    pr("-ll[0]   スクロールで範囲を越えたとき反対側へ続く. -ll0 でオフ\n");
    pr("-ly<0-2> スクロール方法. 0:ソフトウェア転送 1:ＧＤＣ利用(Back) 2:ＧＤＣ利用\n");
    pr("         -ly1 は、400ラインを越える画像は下まで表示後、上に戻る\n");
    pr("-w[N]    キー入力待ちでないとき、画面表示後、Nだけ待つ\n");
    pr("-ws[N,M] スクロールのウェイト. N:キー待ちでないとき  M:キー待ちのとき\n");
    pr("         -w,-ws でのN,Mの値は機種ごとに違うので、実際に指定して調整してnね\n");
    pr("         ただし、V30 では -w の値は半分に、-ws の値は無視する\n");
    pr("-wh      V30でも -w,-ws の指定をそのまま処理する\n");
  #ifdef HF
    pr("-h       256色画像のとき、HF(ﾊｲﾊﾟｰﾌﾚｰﾑ)に表示(安易)\n");
  #endif
  #ifdef MP
    pr("-lmm     256色画像のとき、PC386M,Pなら256色画面で表示(安易)\n");
  #endif
    //pr(" -z	  無視してね（デバッグ用）\n");
    Exit(0);
}

#if 0
_S void Usage2(void)
{
    printf("%s の -? で表示しきれなかったオプション\n",gNameVer);
    Exit(0);
}
#endif

_S void Option(byte *p)
    //オプションを収得
{
    static byte *pp;
    int  c;

    c = *p++;
    c = toupper(c);
    switch (c) {
    case 'B':
    	gUraLoadFlg = 1;
    	break;
    case 'I':
    	gInfoFlg = 1;
    	break;
    case 'C':
    	gClsFlg = -1;
    	if (*p == '1')
    	    gClsFlg = 1;
    	else if (*p == '-' || *p == '0')
    	    gClsFlg = 0;
    	else if (*p == 'P' || *p == 'p') {
    	    gToneDownFlg = 1;
    	    if (*p == '-' || *p == '0')
    	    	gToneDownFlg = 0;
    	} else if (*p && *p != ' ')
    	    OptsErr("-c");
    	break;
    case 'K':
    	gKeyWaitFlg = -1;
    	c = *p++;
    	c = toupper(c);
    	if (c == '?') {
    	    KeyUsage(0);
    	    Exit(0);
    	} else if (c == '-'||c=='0') {
    	    gKeyWaitFlg = 0;
    	} else if (c == '1') {
    	    gKeyWaitFlg = 1;
    	} else if (c == 'G') {
    	    gKeyWaitFlg = 1;
    	    c = *p++;
    	    c = (c=='-') ? '0' : (c==' '||c=='\0') ? '1' : c;
    	    if (c >= '0' && c <= '6')
    	    	gVRamGetNo = c - '0';
    	    else
    	    	OptsRngErr("kg");
    	} else if (c && c != ' ')
    	    OptsErr("-k");
    	break;
    case 'L':
    	c = *p++;
    	c = toupper(c);
    	if (c == 'V') {
    	    if (*p == 'F' || *p == 'f') {
    	    	gEndGrphOffFlg = 1;
    	    }else{
    	    	gEndPalToneFlg = 1;
    	    	gEndPalTone = oDfltTone;
    	    	if (*p && *p != ' ') {
    	    	    gEndPalTone = (word)strtoul(p,&(char *)pp,0);
    	    	    if (gEndPalTone > 200)
    	    	    	OptsRngErr("lv");
    	    	    p = pp;
    	    	}
    	    	if (*p == ',') {
    	    	    p++;
    	    	    oDfltTone = (word)strtoul(p,&(char *)pp,0);
    	    	    if (oDfltTone > 200)
    	    	    	OptsRngErr("lv");
    	    	}
    	    }
    	} else if (c == 'B') {
    	    c = *p++;
    	    c = toupper(c);
    	    c = (c == '\0'||c==' ') ? '1' : (c=='-') ? '0' : c;
    	    if (c >= '0' && c <= '2') {
    	    	gUraWrtMode = c - '0';
    	    } else if (c == 'U') {
    	    	;
    	    } else {
    	    	OptsRngErr("lb");
    	    }
    	} else if (c == 'C') {
    	    if (*p == '\0' || *p == ' ') {
    	    	gOvrLdFlg = 1;
    	    	gOvrLdCol = 0;
    	    } else if (*p == '-') {
    	    	gOvrLdFlg2 = 1;
    	    	gOvrLdCol = 0;
    	    } else {
    	    	gOvrLdFlg = 1;
    	    	gOvrLdCol = (int)strtol(p,NULL,0);
    	    }
    	    if (gOvrLdCol < 0 || gOvrLdCol > 15)
    	    	OptsRngErr("lc");
    	} else if (c == 'Y') {
    	    switch (*p) {
    	    case '0':
    	    	gGdcSclFlg = 0;
    	    	break;
    	    case ' ':
    	    case '\0':
    	    case '1':
    	    	gSclBakFlg = 1;
    	    	gGdcSclFlg = 1;
    	    	break;
    	    case '2':
    	    	gSclBakFlg = 0;
    	    	gGdcSclFlg = 1;
    	    	break;
    	    default:
    	    	OptsErr("-ly");
    	    }
    	} else if (c == 'M') {
    	    if (*p == '\0' || *p == ' ')
    	    	g256mode = 0;
  #ifdef    MP
    	    else if (*p == 'M'||*p == 'm')
    	    	g256mode = 386;
  #endif
    	    else {
    	    	g256mode = (int)strtol(p,NULL,0);
    	    	if (g256mode < 0 || g256mode > 3)
    	    	    OptsRngErr("d");
    	    }
    	} else if (c == 'L') {
    	    gSclLoopFlg = (*p == '-'||*p == '0') ? 0 : 1;
    	} else if (c == 'R') {
    	    gSmlFlg = (*p == '-'||*p == '0') ? 0 : 1;
    	} else if (c && c != ' ') {
    	    OptsErr("-l");
    	}
    	break;
    /*case 'A':
    	gBxByMode = 1;
    	goto OPT_OA:*/
    case 'O':
    	gBxByMode = 2;
    //OPT_OA:
    	gBx = gBy = 0;
    	if (*p == 'a' || *p=='A')
    	    p++,gBxByMode = 1;
    	if (*p == '\0'||*p == ' ')
    	    break;
    	if (*p == '_') {
    	    gBx = -0x7fff;
    	    ++p;
    	} else if (*p != ',') {
    	    gBx = (word)strtol(p,&(char *)pp,0);
    	    p = pp;
    	    if (*p == 'o' || *p == 'O')
    	    	p++,gBx *= 8;
    	    if (gBx > 79*8 || gBx < -1900 || gBx &0x07)
    	    	OptsRngErr("o");
    	}
    	if (*p++ != ',')
    	    break;
    	if (*p == '_') {
    	    gBy = -0x7fff;
    	    ++p;
    	} else {
    	    gBy = (word)strtol(p,&(char *)pp,0);
    	    p = pp;
    	    if (*p == 'o' || *p == 'O')
    	    	p++,gBy *= 8;
    	    if (gBy > 399 || gBy < -1900)
    	    	OptsRngErr("o");
    	}
    	break;
    case 'T':
    	gTxtFlg = 0;
    	if (*p == '1')
    	    gTxtFlg = 1;
    	break;
    case 'V':
    	gPalFlg = 1;
    	gPalTone = oDfltTone;
    	if (*p == '\0' || *p == ' ')
    	    break;
    	gPalTone = (word)strtoul(p,&(char *)pp,0);
    	if (gPalTone > 200)
    	    OptsRngErr("v");
    	break;
    case 'A':
    	gToneUpFlg = gMieMode = gMie2Mode = 0;
    	switch (*p) {
    	case '1':   gMieMode = 2;   break;
    	case '2':   gMieMode = 4;   break;
    	case '3':   gMie2Mode = 1;  break;
    	case '4':   gMie2Mode = 2;  break;
    	case '5':   gToneUpFlg = 1; break;
    	}
    	break;
    case 'W':
    	if (*p == '\0' || *p == ' ') {
    	    gVsWaitCnt0 = 0;
    	} else if (*p >= '0' && *p <= '9') {
    	    gVsWaitCnt0 = (int)strtoul(p,NULL,0);
    	    if (gI8086Flg)
    	    	gVsWaitCnt0 >>= 1;
    	} else if (*p == 'S' || *p == 's') {
    	    p++;
    	    gVsWaitCnt2 = 0;
    	    gVsWaitCnt1 = (int)strtoul(p,&(char *)pp,0);
    	    p = pp;
    	    if (*p == ',') {
    	    	p++;
    	    	gVsWaitCnt2 = (int)strtoul(p,NULL,0);
    	    }
    	    if (gI8086Flg)
    	    	gVsWaitCnt1 = gVsWaitCnt2 = 0;
    	} else if (*p == 'H' || *p == 'h') {
    	    gI8086Flg = 0;
    	    if (*++p == '-')
    	    	gI8086Flg = 1;
    	} else {
    	    OptsErr("-ws");
    	}
    	break;
  #ifdef    HF
    case 'H':
    	gHfMode = 1;	//HyPER-FRAME+ mode
    	//gFBmode = 0;
    	break;
    //case 'F':
    //	gHfMode = 1;	//HyPER-FRAME+ mode
    //	gFBmode = 1;
    //	break;
  #endif
    case 'Z':
    	break;
    case '?':
    	/*if (*p == 'k' || *p == 'K') {
    	    KeyUsage(0);
    	    Exit(0);
    	}*/
    case '\0':
    	Usage();
    default:
    	printf("-%c は知らないオプションだ."
    	    "- か -? で説明を見てね\n",c);
    	Exit(1);
    }
}

_S void Option0(byte *p)
    // -? や -lbu のオプションを処理
{
    int  c;

    c = *p++;
    c = toupper(c);
    switch (c) {
    case 'Z':
    	c = *p++;
    	c = toupper(c);
    	if (c == 'S')
    	    gDebSclFlg = 1;
    	else if (c == 'Z')
    	    gChkPiLd4a = 1;
      #ifdef PILD4C
    	else if (c == 'C')
    	    gPiLd4cFlg = 1;
      #endif
      #ifdef DEB_DMP
    	else if (c == 'D')
    	    gDmpFlg = 1;
      #endif
    	else {
    	    gChk = 1;
    	}
    	break;
    case 'L':
    	if (*p == 'b' || *p == 'B'){
    	    ++p;
    	    if(*p == 'u' || *p == 'U') {
    	    	gRdBufVRamFlg = 1;
    	    	++p;
    	    	if (*p == '-' || *p == '0')
    	    	    gRdBufVRamFlg = 0;
    	    }
    	}
    	break;
    case 'K':
    	if (*p == '?') {
    	    KeyUsage(0);
    	    Exit(0);
    	}
    	break;
    case '?':
    	if (*p == '?') {
    	    //Usage2();
    	} else if (*p == 'k' || *p == 'K') {
    	    KeyUsage(0);
    	    Exit(0);
    	}
    case '\0':
    	Usage();
    }
}

byte *FGetTok(byte **wk, FILE *fp)
    // ファイルよりトークンを得る
{
    static byte buf[300+40];
    byte *p,*s;

    if (*wk == NULL) {
  JJJ:
    	*wk = p = fgets(buf,300,fp);
    	if (p == NULL)
    	    return NULL;
    } else
    	p = *wk;
    for (;;) {
    	if (*p == '\0' || *p == '#') {
    	    goto JJJ;
    	} else if (*p > ' '/* && *p != ','*/) {
    	    break;
    	}
    	++p;
    }
    s = p;
    for (;;) {
    	if (*p == '\0') {
    	    *wk = NULL;
    	    return s;
    	} else if (*p <= ' ') {
    	    *p = '\0';
    	    *wk = p+1;
    	    return s;
    	}
    	++p;
    }
}

#ifdef ATMO
void AtMkOdr(byte *p,byte **ap,FILE *fp)
    // @命令の処理
{
    static byte col[8] = {0,1,4,5,2,3,6,7};
    int x,y,c;

    c = *p;
    switch (c) {
    case 'k':
    	getch();
    	break;
    case 'i':
    	TCls();
    	break;
    case 'p':
    case 'd':
    	p = *ap;
    	if (*p == '"') {
    	    p++; (*ap)++; x = '"';
    	} else if (*p == '\'') {
    	    p++; (*ap)++; x = '\'';
    	} else
    	    x = '\n';
    	while (*p != '\0') {
    	    if (*p == x) {
    	    	*p++ = '\0';
    	    	break;
    	    }
    	    p++;
    	}
    	cprintf ("%s",*ap);
    	*ap = p;
    	if (*p == '\0')
    	    *ap = NULL;
    	if (c == 'p')
    	    cprintf("\r\n");
    	break;
    case 'l':
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@l");
    	x = (int)strtol(p,NULL,0);
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@l");
    	y = (int)strtol(p,NULL,0);
    	gotoxy(x,y);
    	break;
    case 'c':
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@c");
    	x = (int)strtol(p,NULL,0);
    	y = x & 0x0f;
    	x &= 0xff00;
    	textattr(x+col[y]);
    	break;
    case 'w':
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@w");
    	x = (int)strtol(p,NULL,0);
    	VsyncWait(x);
    	break;
    default:
    	OptsErr("@");
    }
}
#endif

int FilOpt(byte *name)
    // ファイルよりオプション、ファイル名を入力して実行
{
    FILE *fp;
    byte *p;
    static byte *aFGetTokWk = NULL;
    static byte tmpName[140];

    fp = fopen(name,"r");
    if (fp == NULL) {
    	printf("%s はオープンできませんでした\n",name);
    	return 1;
    }
    while ((p = FGetTok(&aFGetTokWk,fp)) != NULL) {
    	if (*p == '-') {
    	    Option(p+1);
    	    if (gVRamGetNo)
    	    	VramGetDisp(gVRamGetNo);
    	    gVRamGetNo = 0;
    #ifdef ATMO
    	} else if (*p == '@') {
    	    AtMkOdr(p+1,&aFGetTokWk,fp);
    #endif
    	} else {
    	    ChgExt(tmpName,p,"Pi");
    	    ActGPage(0);
    	    if (PiLoad(tmpName) == 0) {
    	    	if (PiDisp(tmpName))
    	    	    return 1;
    	    	if (gVRamGetNo) {
    	    	    VramGetDisp(gVRamGetNo);
    	    	    gVRamGetNo = 0;
    	    	}
    	    }
    	}
    }
    fclose(fp);
    return 0;
}

void KeyBrk(int sig)
    // Stop-Key , ctrl-c 割り込み用
{
    if (gEndPalToneFlg)
    	RPal_Set(gEndPalTone,PiLd_pal);
    sig = 1;
    Exit(sig);
}

void main(int argc,byte *argv[])
{
    byte *p;
    int  i,n;
    static byte tmpName[140];
    static byte loadName[150];
    static word dmy;
    word  freememszs,inbufszs,ldbufszs;

    Gv_Init();
    signal(SIGINT,KeyBrk);
    gPrgName = argv[0];
    strlwr(gPrgName);
    gPalTone = 100;
    //gUraWrtMode = 0;
    gGdcSclFlg = 1;
    gTxtFlg = 1;
    gSclBakFlg = 1;
    g256mode = 0;   	//-lm0
    gRdBufVRamFlg = 1;	//-lbu
    if (argc < 2)
    	Usage();
    VsWaitInit();
    gI8086Flg = (Chk86()) ? 0 : 1;

    //環境変数 DOPI よりオプション収得
    p = getenv("DOPI");
    if (p) {
    	while (*p) {
    	    //printf("DOPI=%s\n",p);
    	    while(*p == ' ' || *p == '\t')
    	    	p++;
    	    if (*(p++) == '-') {
    	    	Option0(p);
    	    	Option(p);
    	    }
    	    while(*p && *p != ' ' && *p != '\t')
    	    	p++;
    	}
    }

    // -? や -lbu 用にオプションを解析
    for (n = i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p != '-') {
    	    n = 0;
    	} else {
    	    Option0(p+1);
    	}
    }

    // コマンドラインにオプションしか指定されていないとき
    if (n) {
    	for (i = 1; i < argc; i++) {
    	    p = argv[i];
    	    if (*p == '-') {
    	    	Option(p + 1);
    	    }
    	}
    	if (gPalFlg) {
    	    RPal_ChgTone(gPalTone);
    	}
      #if defined(MP)
    	if (g256mode == 386) {
    	    Mp_Init();
    	    gUraWrtMode = 3;
    	}
      #endif
    	if (gClsFlg) {
    	    gUraUseMode = gUraWrtMode;
    	    if (gToneDownFlg)
    	    	ToneDown(0);
    	    GCls();
    	}
    	if (gVRamGetNo == 0)
    	    Exit(0);
    }

    // 画像展開用のバッファの確保
    freememszs = allocmem(0xffff,&dmy);
    if (freememszs < 0xA00) {
    	MemErr();
    } else if (freememszs < 0x2A00) {
    	inbufszs = 0x400;   //0x400*0x10 = 0x4000 = 16K
    	ldbufszs = freememszs - 0x400 - 0x80;
    } else if (freememszs < 0x3700) {
    	inbufszs = freememszs - 0x2800 - 0x80;
    	ldbufszs = 0x2800;
    } else {
    	inbufszs = 0xD80;
    	ldbufszs = freememszs - 0xD80 - 0x80;
    }
    if (gRdBufVRamFlg) {
    	inbufszs = 0;
    	ldbufszs = freememszs - 0x80;
    }
    if (gChk) {
    	printf("mem:%04x0(%ld)  入力buf:%04x0(%ld)  画像buf:%04x0(%ld)\n",
    	    freememszs,freememszs*16L, inbufszs,inbufszs*16L,ldbufszs,
    	    ldbufszs*16L);
    	getch();
    }
    if (gRdBufVRamFlg) {
    	PiLd_RdInitVRam();
    } else {
    	if ((PiLd_RdInit(inbufszs*16)) != 0)
    	    MemErr();
    }

    if ((PiLd_LdInit(ldbufszs)) != 0)
    	MemErr();

    // コマンドラインにファイル名なしで -kg[N]が指定されたとき
    if (gVRamGetNo) {
    	VramGetDisp(gVRamGetNo);
    	goto EXIT_MAIN;
    }

    // ファイル、オプションの処理
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-') {    	    // オプション
    	    Option(p + 1);
    	    if (gVRamGetNo)
    	    	VramGetDisp(gVRamGetNo);
    	    gVRamGetNo = 0;
    	} else if (*p == '@') {     // @FILE
    	    if (FilOpt(p+1))
    	    	goto EXIT_MAIN;
    	} else {    	    	    // ファイル名
    	    ChgExt(tmpName,p,"pi");
    	    n = DirEntryGet(loadName,tmpName,0);
    	    if (n == 0) {
    	    	do {
    	    	    ActGPage(0);
    	    	    if (PiLoad(loadName) == 0) {
    	    	    	if (PiDisp(loadName))
    	    	    	    goto EXIT_MAIN;
    	    	    	if (gVRamGetNo) {
    	    	    	    VramGetDisp(gVRamGetNo);
    	    	    	    gVRamGetNo = 0;
    	    	    	}
    	    	    }
    	    	} while ((n = DirEntryGet(loadName,NULL,0)) == 0);
    	    } else {
    	    	printf(" %s が見つからないです\n",tmpName);
    	    }
    	}
    }
  EXIT_MAIN:
    if (gEndPalToneFlg) {
    	if (gEx256Flg && gEndPalTone < 100) {
    	    gEndGrphOffFlg = 1;
    	} else {
    	    if (gToneDownFlg)
    	    	ToneDown(gEndPalTone);
    	    else
    	    	RPal_Set(gEndPalTone,PiLd_pal);
    	}
    }
    if (gEndGrphOffFlg) {
      #ifdef HF
    	if (gHfMode) {
    	    Hf_DspOff();
    	    outp( 0x6a, 1); //analog mode
    	}
      #endif
    	_AX = 0x4100;
    	geninterrupt(0x18);
    	//outp( 0x6a, 1);   //analog mode
    }
    Exit(0);
}
