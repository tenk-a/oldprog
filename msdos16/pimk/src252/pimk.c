/*
    pimk    PC98用Pi&MAG ローダ

    	    1993-1995	    	Writen By てんかﾐ☆(Masashi Kitamura)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <alloc.h>
#include <signal.h>
#include <conio.h>
#include <dir.h>
#include <io.h>
#include "def.h"
#include "rpal.h"
#include "gf.h"
#include "key.h"
#include "text.h"
#include "ems.h"
#include "rpal.h"

#include "vv.h"
#include "v98.h"
#include "vmp.h"
#include "vwab.h"
#include "wab256.h"
#include "v21.h"
#include "vhf.h"
#include "vfb.h"
#include "vhfb.h"
#include "pim.h"

//#define EX_HF

/*---------------------------------------------------------------------------*/
VVF gVvfunc[N_CNT] = {
  /*{NO,V_SIZE, V_Init, V_Start, V_End, V_GStart, V_GEnd, V_GCls,
     V_ShowOn, VShowOff, V_SetTone, V_GetLines,
     V_RevX, Vv_RevY, V_SetPal,V_PutPx, V_PutPxScrn, V_PutPxPart,
     V_PutPxBCol, V_PutPxLoop,
     Vv_SclLoopMode, V_SclUp, V_SclDw, V_SclLft, V_SclRig, V_GetSclOfs},*/
    //ｱﾅﾛｸﾞ16色
    {N_S,V98_SIZE, V98_Init, Vv_Non, V98_End, V98_GStart, V98_GEnd, V98_GCls,
     V98_ShowOn, V98_ShowOff, V98_SetTone, V98_GetLines, V98_GetLines,
     V98_RevX, Vv_RevY, V98_SetPal, V98_PutPx, V98_PutPxScrn, V98_PutPxPart,
     V98_PutPxBCol, V98_PutPxLoop,
     Vv_SclLoopMode, V98_SclUp, V98_SclDw, V98_SclLft, V98_SclRig,
     V98_GetSclOfs},
    //力技
    {N_WL,VWL_SIZE,VMP_InitWl,VMP_StartWl,VMP_EndWl,VMP_GStartWl,VMP_GEndWl,
     VMP_GCls, VMP_ShowOn, VMP_ShowOff, VMP_SetToneWl,
     VMP_GetLineWl, VMP_GetLineWl,
     VMP_RevX, Vv_RevY, VMP_SetPalWl, VMP_PutPx, VMP_PutPxScrn, VMP_PutPxPart,
     VMP_PutPxBCol, VMP_PutPxLoop,
     Vv_SclLoopMode, VMP_SclUp, VMP_SclDw, VMP_SclLft,
     VMP_SclRig, VMP_GetSclOfs},
    //PC386M/P
    {N_MP,VMP_SIZE, VMP_Init, Vv_Non, VMP_End, VMP_GStart, VMP_GEnd, VMP_GCls,
     VMP_ShowOn, VMP_ShowOff, VMP_SetTone, VMP_GetLines, VMP_GetLines,
     VMP_RevX, Vv_RevY, VMP_SetPal, VMP_PutPx, VMP_PutPxScrn, VMP_PutPxPart,
     VMP_PutPxBCol, VMP_PutPxLoop,
     Vv_SclLoopMode, VMP_SclUp, VMP_SclDw, VMP_SclLft, VMP_SclRig,
     VMP_GetSclOfs},
    //WAB-S
    {N_WAB,VWAB_SIZE, VWAB_Init, VWAB_Start, VWAB_End, VWAB_GStart, VWAB_GEnd,
     VWAB_GCls, VWAB_ShowOn, VWAB_ShowOff, VWAB_SetTone,
     V21_GetLines, Vv_GetLine24,
     VWAB_RevX, Vv_RevY, VWAB_SetPal, VWAB_PutPx,
     VWAB_PutPxScrn, VWAB_PutPxPart, VWAB_PutPxBCol, VWAB_PutPxLoop,
     Vv_SclLoopMode, VWAB_SclUp, VWAB_SclDw, VWAB_SclLft, VWAB_SclRig,
     VWAB_GetSclOfs},
    //PC9821
   #ifndef WAB21
    {N_21,V21_SIZE, V21_Init, V21_Start, V21_End, V21_GStart, V21_GEnd,
     V21_GCls, V21_ShowOn, V21_ShowOff, V21_SetTone,
     V21_GetLines, Vv_GetLine24,
     VWAB_RevX, Vv_RevY, V21_SetPal, V21_PutPx,
     V21_PutPxScrn, V21_PutPxPart, V21_PutPxBCol, V21_PutPxLoop,
     Vv_SclLoopMode, V21_SclUp, V21_SclDw, V21_SclLft, V21_SclRig,
     V21_GetSclOfs},
   #else
    {N_21,V21_SIZE, V21_Init, V21_Start, V21_End, V21_GStart, V21_GEnd,
     VWAB_GCls, V21_ShowOn, V21_ShowOff, VWAB_SetTone,
     V21_GetLines, Vv_GetLine24,
     VWAB_RevX, Vv_RevY, VWAB_SetPal, V21_PutPx,
     V21_PutPxScrn, V21_PutPxPart, V21_PutPxBCol, V21_PutPxLoop,
     Vv_SclLoopMode, V21_SclUp, V21_SclDw, V21_SclLft, V21_SclRig,
     V21_GetSclOfs},
   #endif
    //HF+
   #ifndef WAB21
    {N_HF,VHF_SIZE, VHF_Init, VHF_Start, VHF_End, VHF_GStart, VHF_GEnd,
     VHF_GCls, VHF_ShowOn, VHF_ShowOff, VHF_SetTone,
     V21_GetLines, Vv_GetLine24,
     VWAB_RevX, Vv_RevY, VHF_SetPal, VHF_PutPx,
     VHF_PutPxScrn, VHF_PutPxPart, VHF_PutPxBCol, VHF_PutPxLoop,
     Vv_SclLoopMode, VHF_SclUp, VHF_SclDw, VHF_SclLft, VHF_SclRig,
     VHF_GetSclOfs},
   #else
    {N_HF,VHF_SIZE, VHF_Init, VHF_Start, VHF_End, VHF_GStart, VHF_GEnd,
     VWAB_GCls, VWAB_ShowOn, VWAB_ShowOff, VHF_SetTone,
     V21_GetLines, Vv_GetLine24,
     VWAB_RevX, Vv_RevY, VHF_SetPal, VHF_PutPx,
     VHF_PutPxScrn, VHF_PutPxPart, VHF_PutPxBCol, VHF_PutPxLoop,
     Vv_SclLoopMode, VHF_SclUp, VHF_SclDw, VHF_SclLft, VHF_SclRig,
     VHF_GetSclOfs},
   #endif
    //SF
    {N_SF,VHF_SIZE, VHF_InitSF, VHF_Start, VHF_End, VHF_GStart, VHF_GEnd,
     VHF_GCls, VHF_ShowOn, VHF_ShowOff, VHF_SetTone,
     V21_GetLines, Vv_GetLine24,
     VWAB_RevX, Vv_RevY, VHF_SetPal, VHF_PutPx,
     VHF_PutPxScrn, VHF_PutPxPart, VHF_PutPxBCol, VHF_PutPxLoop,
     Vv_SclLoopMode, Vv_Non/*VHF_SclUp*/, Vv_Non/*VHF_SclDw*/,
     Vv_Non/*VHF_SclLft*/, Vv_Non/*VHF_SclRig*/,
     VHF_GetSclOfs},
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static VVF *gVf = &(gVvfunc[0]);
static VVF *gVf_old = &(gVvfunc[0]);

static int gKeyWaitFlg = 0; 	//キー入力・モード 0:違う 1:そう
static int gKeyWaitMode = 0;	//スクロールでVsyncWaitウェイト 0:無し 1:有り
static int gDebugFlg = 0;   	//debug
static int gClsFlg = 0;     	//画面消去sw
static int gCls2Flg = 0;    	//画面モードが切り替わるとき画面消去sw
static int gLeaveTone=-1/*100*/;//終了時のトーン
static int gTone=-1/*100*/; 	//トーン
static int gYmax=0; 	    	//最大展開行数. デバッグ用
static int gForce256flg = 0;	//強制256表示
static int gDspSclFlg = 1;  	//一度に表示しきれんかった部分の表示方法-z
static int gWaitTime = 0;   	//表示ごとに gWaitTime*0.1 秒wait
static int gC256mode = N_WL;	//256色画像の表示手段 0:16色減色 1:力 2:PC386M
static int gV21mode = 0;    	//PC9821 -2:I/Oﾉﾐ  -1:400強 0:標準 1:480強
static int gVramPage = 0;   	//16色画像の表示ページ
static int gAutoChk16 = 0;  	//256色のとき16色しかつかってないかどうかCHK
static unsigned gEmsSeg,gEmsCnt,gEmsHdl;    //仮想VRAM：EMS管理
static int gEmsMax = 0x7FF0;	    	    //最大EMSページ
static unsigned gMmSeg = 0,gMmSzs = 0;	    //仮想VRAM：メインメモリ
static unsigned gMmSeg0 = 0,gMmSzs0 = 0;    	//仮想VRAM：メインメモリ
static char gPlugInTmpNam[260]; //テンポラリ名

#define ResetMm()   (gMmSeg = gMmSeg0, gMmSzs = gMmSzs0)

void far *GetMm(WORD szs)
{
    void far *p;
    p = (void far *)(((DWORD)gMmSeg)<<16);
    gMmSeg += szs;
    gMmSzs -= szs;
    return p;
}

#define MmTerm()    MEM_Freep(gMmSeg0)

int MmInit(void)
{
    gMmSzs0 = MEM_CoreLeftp();	    // 仮想VRAM
    // 0x400=安全 + 0x20000=展開ルーチン・ワーク + 0xA000=出力ルーチン・ワーク
    if (gMmSzs0 < 0x40+0x2000U+0xA00U)
    	return 1;
    gMmSzs0 -= 0x40;
    gMmSeg0 = MEM_Allocp(gMmSzs0);
    if (gMmSeg0 == 0)
    	return 1;
    return 0;
}

int EmsInit(void)
{
    gEmsCnt = 0;
    gEmsHdl = 0;
    gEmsSeg = 0;
    if (EMS_Exist()) {
    	gEmsSeg = EMS_PageSeg();
    	if (gEmsSeg >= 0xC000) {
    	    gEmsCnt = EMS_FreePageCount();
    	    if (gEmsCnt > gEmsMax) {
    	    	gEmsCnt = gEmsMax;
    	    }
    	    gEmsHdl = EMS_Alloc(gEmsCnt);
    	}else {
    	    gEmsSeg = 0;
    	}
    	//printf("hdl=%x seg=%x cnt=%d\n",gEmsHdl,gEmsSeg,gEmsCnt);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
/*- GF 画像展開関係 ---------------------------------------------------------*/
// 画像展開関係
enum {GF_NIL=0,GF_PI,GF_MAG,GF_DJP,GF_Q0,GF_RGB,GF_BMP,GF_PMT,
#if 0
GF_VR98,
#endif
GF_MAX};

static struct GFT {
    char ext[4];
    WORD size;
    GF far * far (*open)(void far *gf, char far *fileName);
    int far (*close)(void far *gf);
    int far (*load)(void far *gf,
    	 void far (*putLine)
    	    (void far *dat, unsigned char far *lineBuf),
    	 void far *putLinDat,
    	 int ymax
    	 );
} gFt[] = {
    {"*",   0U,      NULL,     NULL,	  NULL,   },
    {"PI",  0x1000U, PI_Open,  PI_Close,  PI_Load },
    {"MAG", 0x2000U, MAG_Open, MAG_Close, MAG_Load},
    {"DJP", 0x1000U, DJP_Open, DJP_Close, DJP_Load},
    {"Q0",  0x1000U, Q0_Open,  Q0_Close,  DJP_Load},
    {"RGB", 0x1000U, Q0_Open,  Q0_Close,  DJP_Load},
    {"BMP", 0x1000U, BMP_Open, BMP_Close, BMP_Load},
    {"PMT", 0x1000U, PMT_Open, DJP_Close, DJP_Load},
    {"PMY", 0x1000U, PMT_Open, DJP_Close, DJP_Load},
  #if 0
    {"]G",  0x100U,  VR98_Open,VR98_Close,VR98_Load},
  #endif
    {"",    0U,      NULL,     NULL,	  NULL,   },
};

#define FMT_PLUG    0x80

#if FMT_PLUG

static int gPlgFlg = 0;     	//プラグインを許可  1:する 0:しない
static int gPlgTmpFlg = 0;  	//プラグイン中かどうか


typedef struct PLUG_T {
    char    ext[4];
    int     tgtFmt;
    char    *cmd;
} PLUG_T;
PLUG_T gPlug[64];
int    gPlugCnt;

int PlugIn(char *nam, char *ext)
    // nam : file name (260 byte)
{
    int fmt;
    char cmd[400];

    gPlgTmpFlg = 0;
    for (fmt = 0; fmt < gPlugCnt; fmt++) {
    	if (strcmp(ext,gPlug[fmt].ext) == 0) {
    	  #if 0
    	    sprintf(cmd, "%s %s %s", gPlug[fmt].cmd, nam, gPlugInTmpNam);
    	  #else
    	    {	int i;
    	    	char *d, *s;
    	    	d = cmd;
    	    	s = gPlug[fmt].cmd;
    	    	for (i = 0; i < 128; i++) {
    	    	    *d = *s++;
    	    	    if (*d == 0)
    	    	    	break;
    	    	    if (*d == '%') {
    	    	    	if (*s == 'i') {
    	    	    	    ++s;
    	    	    	    memcpy(d, nam, strlen(nam));
    	    	    	    d += strlen(nam)-1;
    	    	    	} else if (*s == 'o') {
    	    	    	    ++s;
    	    	    	    memcpy(d,gPlugInTmpNam,strlen(gPlugInTmpNam));
    	    	    	    d += strlen(gPlugInTmpNam)-1;
    	    	    	} else if (*s == '%') {
    	    	    	    ++s;
    	    	    	}
    	    	    }
    	    	    d++;
    	    	}
    	    }
    	  #endif
    	    puts(cmd);
    	    MmTerm();
    	    system(cmd);
    	    if (MmInit()) {
    	    	void Exit(int);
    	    	printf("メモリが足らなくなったの\n");
    	    	Exit(1);
    	    }
    	    //strcpy(nam, gPlugInTmpNam);
    	    gPlgTmpFlg = 1;
    	    return gPlug[fmt].tgtFmt;
    	}
    }
    return 0;
}

#endif


int GF_ChkExt(char *fname)
{
    char *p;
    int fmt;

    fmt = 0;
    p = fname;
    if (*p == '.')
    	p++;
    if (*p == '.')
    	p++;
    p = strrchr(p,'.');
    if (p) {
    	p++;
      #if FMT_PLUG
    	if (gPlgFlg)
    	    fmt = PlugIn(fname, p);
    	if (fmt == 0)
      #endif
    	{
    	    for (fmt = 1; gFt[fmt].ext[0] != '\0'; fmt++) {
    	    	if (stricmp(p,gFt[fmt].ext) == 0)
    	    	    break;
    	    }
    	    if (gFt[fmt].ext[0] == '\0')
    	    	fmt = 0;
    	}
    }
    if (fmt == GF_RGB) {
    	FIL_FIND_T ff;
    	FIL_FindFirst(fname, 0, &ff);
    	fmt = GF_Q0;
    	if (ff.size == 48)
    	    fmt = 0;
  #if 0
    } else if (fmt == GF_VR98) {
    	gVf = &gVvfunc[0];
    	if (gVf_old != gVf) {
    	    if (c256mode > N_MP /*&& gVf_old->no <= N_MP*/) {
    	    	gVf_old->GCls();
    	    	RPAL_SetDfltPal(100);
    	    }
    	    gVf_old->GEnd();
    	}
    	gVf_old = gVf;
    	//printf("4b ");//if (getch() != ' ') {;err = -1; goto RET;}
    	gVf->GStart();
  #endif
    }
  RET:
    return fmt;
}


/*---------------------------------------------------------------------------*/
static int DoOne(int fmt, char *path, char *dspPath,
    	    	int xstt, int ystt, int asp1, int asp2, int bcol,
    	    	int reductFlg, int c256mode, int loopFlg
    	    	)
{
    static BYTE *errMsgPiOpen[] = {
    	"PRGERR:画像バッファへのポインタの値が不正\n",
    	"ファイル・オープンできなかった",
    	"ヘッダIDが違う",
    	"ヘッダに矛盾がある",
    };
    static BYTE *errMsgPiLoad[] = {
    	"読み込みエラー\n",
    	"データを読み込みすぎた\n",
    	"横幅が小さすぎるか、または大きすぎる\n",
    	"ゴメンね. 未対応のフォーマットなの."
    	"PRGERR:画像バッファへのポインタの値が不正\n",
    };
    int ycnt,i,kabeFlg,err,pln;
    //unsigned c256seg=0;
    unsigned vvSeg=0;
    GF far *gf;
    VV far *vvv;

    ResetMm();

    //Piファイル OPEN
    //printf("%s\n",path);
    gf = gFt[fmt].open(GetMm(gFt[fmt].size), path);
    if ((((DWORD)gf)&0xffff0000UL) == 0L) {
    	err = (int)( ((DWORD)gf) & 0xffffU);
    	printf("%s : %s\n",path, errMsgPiOpen[err]);
    	return -1;
    }
    // Piフォーマットで256色ならもう64Kバイト作業バッファを確保
    if (fmt == GF_PI && gf->pln == 8) {
    	GetMm(0x1000U);
    }

    pln = gf->pln;
    if (gAutoChk16 && pln == 8) {
    	int a,i;
    	for (a = 0, i = 48; i < 256*3; i++) {
    	    a |= gf->palette[i];
    	}
    	if (a == 0)
    	    pln = 4;
    }

    //メッセージ表示のための準備
    PriMsgSet(/*gf->fileName*/dspPath, gf->xsize, gf->ysize,
    	    	gf->xstart, gf->ystart,
    	    	gf->pln, gf->aspect1,gf->aspect2,gf->saverName,
    	    	gf->artist, gf->comment);

    Text_Cls(); //TEXT画面消去
    Text_Sw(1);
    PriCmt();	//ファイル名、コメント表示

    //始点
    if (xstt < 0 || ystt < 0) {
    	xstt = gf->xstart;
    	ystt = gf->ystart;
    }
    //アスペクト比
    if (asp1 == 0 || asp2 == 0) {
    	asp1 = gf->aspect1;
    	asp2 = gf->aspect2;
    }

    //256色画像のときの準備
    //c256seg = 0;
    if (/*gf->*/pln == 4) {
    	gVf = &gVvfunc[0];
    	if (gForce256flg) {
    	    gVf = &gVvfunc[c256mode];
    	} else {
    	    c256mode = N_S;
    	}
    } else {
      #if 0
    	if (fmt == GF_PI) {
    	    c256seg = (WORD)((((DWORD)gf)>>16)+0x1000);
    	    if (c256seg == 0) {
    	    	printf("(^.^;)");
    	    	goto ERR;
    	    }
    	}
      #endif
    	gVf = &gVvfunc[c256mode];
    }


    // 透明色モードのとき
    if (bcol == 0) {
    	bcol = gf->overlayColor;
    }
    //loop mode
    if (loopFlg == 0) {
    	loopFlg = gf->loopMode;
    }

    //VVを設定
    ycnt = gf->ysize;
    if (gYmax && ycnt > gYmax) {
    	ycnt = gYmax;
    	//printf("%d 行までを表示します.\n",ycnt);
    }
    // printf("ycnt=%d ",ycnt);

    // PC9821とき
    if (gVf->no == N_21) {
    	if (gV21mode) {
    	    V21_SetMode(gV21mode);
    	} else {
    	    //y:仮想VRAMでの行数
    	    int y = ycnt;
    	    if (reductFlg) {
    	    	y >>= reductFlg;
    	    } else if (asp1 == 2 && asp2 == 2) {
    	    	y <<= 1;
    	    }
    	    if (y >= 400) {
    	    	V21_Sw480((y!=400));
    	    }
    	}
    }

    //各グラフィックモード用初期化
    vvv = GetMm(gVf->memSize>>4);
    vvSeg = (WORD)(((DWORD)vvv) >> 16);
    err = gVf->Init(vvSeg, gMmSeg, gMmSzs, gEmsHdl,gEmsSeg,gEmsCnt,
    	    	    /*gf->*/pln, gf->xsize, ycnt, xstt, ystt,
    	    	    gf->palette, asp1, asp2, bcol,
    	    	    reductFlg,loopFlg
    	    	    );
    if (gDebugFlg) {
    	printf ("InitAddr=%8lx\n",gVf->Init);
    	//printf("cSeg=%04x vvSeg=%04x gMmSeg=%04x gMmSzs=%04x\n",
    	//  c256seg,vvSeg,gMmSeg,gMmSzs);
    	printf("vvSeg=%04x gMmSeg=%04x gMmSzs=%04x\n",vvSeg,gMmSeg,gMmSzs);
    	Vv_PriInfo(vvSeg);
    	if (getch() != ' ') {;err = -1; goto RET;}
    }
    //printf("EMS %d page\n",emsCnt);
    //printf("ycnt = %d , %d\n",err,ycnt);
    //printf("xs,ys=%d,%d pln=%d\n",vvv->xvsize,vvv->yvsize,vvv->pln);
    //if (getch() != ' ') {;err = -1;	goto RET;}
    if (err < ycnt) {
    	ycnt = err;
    	printf("メモリが足りないので %d 行までしか読み込めません.\n",ycnt);
    	if (ycnt == 0) {
    	    printf("(T_T;)");
    	    goto ERR;
    	}
    }
    err = 0;
    if (gForce256flg && c256mode == N_S) {
    	V98_SetDitMode (vvSeg, gForce256flg);
    }

    // 展開
    //printf("2 ");
    if (/*gf->*/pln != 24) {
    	if ((err=gFt[fmt].load( gf , gVf->GetLines, vvv, ycnt)) != 0) {
    	    printf("%s : %s\n",path,errMsgPiLoad[err-1]);
    	}
    } else {
    	if ((err=gFt[fmt].load( gf , gVf->GetLine24, vvv, ycnt)) != 0) {
    	    printf("%s : %s\n",path,errMsgPiLoad[err-1]);
    	}
    }
    //printf("3 ");
    // Piファイル・クローズ
    gFt[fmt].close(gf);

  #if 0
    // 256画像用のバッファを解放
    if (fmt == GF_PI && c256seg) {
    	MEM_Freep(c256seg);
    	PI_SetC256buf(0);
    }
  #endif

  FLOOP:
    //画面準備
    //printf("4 ");//if (getch() != ' ') {;err = -1; goto RET;}
  #if 0
    if (c256mode == N_WAB || c256mode >= N_HF /*&& gVf_old->no <= N_MP*/) {
    	gVvfunc[0].GCls();
    	RPAL_SetDfltPal(100);
    }
  #endif
    if (gVf_old != gVf) {
    	if (gCls2Flg || c256mode > N_MP /*&& gVf_old->no <= N_MP*/) {
    	    gVf_old->GCls();
    	    RPAL_SetDfltPal(100);
    	}
    	gVf_old->GEnd();
    }
    gVf_old = gVf;
    //printf("4b ");//if (getch() != ' ') {;err = -1; goto RET;}
    gVf->GStart();

    //16色なとき
    if (gVf->no == N_S || gVf->no == N_WL) {
    	Grph_DspPage(gVramPage);
    	Grph_ActPage(gVramPage);
    }

    // 画面クリア
    //printf("5 ");//if (getch() != ' ') {;err = -1; goto RET;}
    if (gClsFlg) {
      #if 0
    	if (gVf->no >= N_WAB) {
    	    V98_GCls();
    	}
      #endif
    	gVf->GCls();
    }
    //パレット設定
    //printf("6 ");//if (getch() != ' ') {;err = -1; goto RET;}
    gVf->SetPal(vvSeg,gTone);

    //表示
    //printf("7 ");//if (getch() != ' ') {;err = -1; goto RET;}

    kabeFlg = (vvv->xvsize<=vvv->xgscrn && vvv->yvsize<=vvv->ygscrn);
    if (kabeFlg && loopFlg) {
    	//ループ画像のとき
    	gVf->PutPxLoop(vvSeg);
    } else if (bcol > 0) {
    	//透明色付のとき
    	vvv->xgstart %= vvv->xgscrn;
    	vvv->ygstart %= vvv->ygscrn;
    	gVf->PutPxBCol(vvSeg);
    } else if( (vvv->xgstart||vvv->ygstart)
    	&&(vvv->xgstart < vvv->xgscrn && vvv->ygstart < vvv->ygscrn)
    ){
    	//部分セーブ画像のとき
    	//vvv->xgstart %= vvv->xgscrn;
    	//vvv->ygstart %= vvv->ygscrn;
    	gVf->PutPxPart(vvSeg);
    } else {
    	//普通に表示
    	switch(gDspSclFlg) {
    	case 0://スクロールしない
    	case 1://スクロールする
    	    vvv->xvstart = vvv->yvstart = 0;
    	    vvv->xgstart = vvv->ygstart = 0;
    	    //gVf->PutPxScrn(vvSeg,0,0);
    	    gVf->PutPxPart(vvSeg);
    	    //ﾉｰﾏﾙ16,力技,PC386M,WABS,PC9821,HFで640*400以上の画像ならズリズリ
    	    if (gDspSclFlg && c256mode <= N_HF) {
    	    	// 縦400line以上あるなら縦スクロール
    	    	if (vvv->yvsize > vvv->ygscrn) {
    	    	    i = vvv->yvsize - vvv->ygscrn;
    	    	    do {
    	    	    	gVf->SclDw(vvSeg,1);	//down
    	    	    } while (--i);
    	    	}
    	    	// 横640ドット以上なら横スクロール
    	    	if (vvv->xvsize > vvv->xgscrn) {
    	    	    i = (vvv->xvsize - vvv->xgscrn) >> (1+3);
    	    	    do {
    	    	    	gVf->SclRig(vvSeg,16);	//right
    	    	    } while (--i);
    	    	}
    	    }
    	    break;
    	}
    }
 //printf("8 ");
    // キー入力時
  KKK:
    Text_Sw(1);     //PC9821対策?(T^T)
    err = 0;
    if (gKeyWaitFlg) {
    	err = KeyLoop(vvSeg,gVf,gKeyWaitMode,gTone,loopFlg,kabeFlg);
    } else {
    	int i;
    	if (gWaitTime) {
    	    for (i = 0; i < gWaitTime;i++) {
    	    	int j;
    	    	for (j = 0; j < 12;j++) {
    	    	    if (Key_Shift()&0x01) {
    	    	    	goto KKK2;
    	    	    }
    	    	    delay(8);
    	    	}
    	    }
    	}
    	if (Key_Shift() & 0x01) {
     KKK2:
    	    gKeyWaitFlg = 1;
    	    if (gVf->no < N_HF /*|| gVf->no > N_HFB*/) {
    	    	for (i = 0; i < 6; i++) {
    	    	    gVf->SetTone(50); delay(40); gVf->SetTone(100); delay(40);
    	    	}
    	    }
    	    gVf->SetTone(gTone);
    	    goto KKK;
    	}
    }
    //256画面モードをちょこっと変更
    if (err == 20) {
    	if (c256mode == N_21 && gV21mode >= -1) {
    	    //400Lだったら480Lに、480Lだったら400Lに変更
    	    V21_Sw480((V21_GetYsize() == 400));
    	 J1:
    	    if (/*gf->*/pln == 4) {
    	    	if (gForce256flg) {
    	    	    gVf = &gVvfunc[c256mode];
    	    	}else {
    	    	    c256mode = N_S;
    	    	    gVf = &gVvfunc[0];
    	    	}
    	    } else {
    	    	gVf = &gVvfunc[c256mode];
    	    }
    	    err = gVf->Init(vvSeg, gMmSeg, gMmSzs, gEmsHdl,gEmsSeg,gEmsCnt,
    	    	    	    /*gf->*/pln, gf->xsize, ycnt, xstt, ystt,
    	    	    	    gf->palette, asp1, asp2, bcol,
    	    	    	    reductFlg,loopFlg
    	    	    	    );
    	}
    	goto FLOOP;
    }

  RET: //終了
    return err;
 ERR:
    printf("メモリがたんないよ\n");
    err = -1;
    goto RET;
}


/*---------------------------------------------------------------------------*/
static int gOverlayColor = 0;	    //透明色
static int gXstart = -1,gYstart =-1;//始点
static int gAsp1 = 0, gAsp2 = 0;    //アスペクト比
static int gReductFlg = 0;  	    //縮小モード
static int gMultiDirFlg = 0;	    //Multi Dir Sarch flag
static int gLoopSclFlg = 0; 	    //ループ・スクロール・モード
static int gEnd256mode = 1; 	    //256画像終了の方法 0:強制16色 1:ﾃﾞﾌｫﾙﾄ


void Term(void)
{
    if (gEmsCnt) {
    	EMS_Free(gEmsHdl);
    }
    if (gLeaveTone > 200) {
    	gLeaveTone = 200;
    }
    if (gLeaveTone < 0) {
    	gLeaveTone = 50;
    	if (gTone >= 0 && gTone <= 200) {
    	    gLeaveTone = gTone;
    	}
    }
    if (gC256mode == N_21 && gVf->no != N_21) {
    	//pc9821 は必ず End() を呼んで起動前の状態に戻す
    	V21_End(gLeaveTone);
    	V98_End(gLeaveTone);
    } else {
    	gVf->End(gLeaveTone);
    }
    if (gEnd256mode == 0) { //強制的に16色モードで終了
    	gVf->ShowOff();
    	gVf->GEnd();
    	gVvfunc[0].ShowOff();
    }
    Text_Sw(1);
    Text_CursorSw(1);
    Text_PFKeySw(1);	//ファンクション・キー表示を行なう
}

void Exit(int sig)
    // Stop-Key , ctrl-c 割り込み用
{
    Term();
    sig = 1;
    exit(sig);
}


int Foo(char *path, char *filname, int fmt)
{
    static char fname[514];

    if (FIL_GetDirEnt(fname,path,0x01) == 0) {
    	do {
    	    int err,asp1,asp2,reductFlg,c256mode,f;
    	    char *fn;
    	    f = fmt;
    	    fn = fname;
    	    if (f == 0) {
    	    	f = GF_ChkExt(fname);
    	      #if FMT_PLUG
    	    	if (gPlgTmpFlg) {
    	    	    fn = gPlugInTmpNam;
    	    	}
    	      #endif
    	    }
    	    if (f == 0)
    	    	continue;
    	    asp1 = gAsp1;
    	    asp2 = gAsp2;
    	    reductFlg = gReductFlg;
    	    c256mode = gC256mode;
    	  LOOP:
    	    err = DoOne(f,fn,fname,
    	    	    	gXstart,gYstart, asp1, asp2, gOverlayColor,
    	    	    	reductFlg, c256mode, gLoopSclFlg
    	    	    	);
    	    if (err == 1) {
    	    	asp1 = asp2 = 0;
    	    	reductFlg = 0;
    	    	goto LOOP;
    	    } else if (err == 2) {
    	    	asp1 = asp2 = 2;
    	    	reductFlg = 0;
    	    	goto LOOP;
    	    } else if (err == 3) {
    	    	asp1 = asp2 = 0;
    	    	reductFlg = 1;
    	    	goto LOOP;
    	    } else if (err == 4) {
    	    	asp1 = asp2 = 0;
    	    	reductFlg = 2;
    	    	goto LOOP;
    	    } else if (err >= 10 && err < 20) {
    	    	c256mode = err - 10;
    	    	goto LOOP;
    	    }
    	  #if FMT_PLUG
    	    if (gPlgTmpFlg)
    	    	remove(gPlugInTmpNam);
    	  #endif
    	    if (err)
    	    	return err;
    	} while (FIL_GetDirEnt(fname,NULL,0x01) == 0);
    }
    //multi-directory-sarch
    if (gMultiDirFlg) {
    	BYTE *fptr;
    	FIL_FIND_T fslot;

    	fptr = FIL_BaseName_N(path);
    	strcpy(fptr,"*.*");
    	if (FIL_FindFirst(path,_A_SUBDIR,&fslot) == 0) {
    	    do {
    	    	if ((fslot.attrib & _A_SUBDIR) && fslot.name[0] != '.') {
    	    	    strcpy( stpcpy(fptr,fslot.name) , filname);
    	    	    if (Foo(path, filname, fmt)) {
    	    	    	return -1;
    	    	    	//break;
    	    	    }
    	    	}
    	    } while (FIL_FindNext(&fslot) == 0);
    	}
    }
    return 0;
}


void Usage(VOID)
{
 puts(
 "PC98用Pi&Mag ﾛｰﾀﾞ Pimk[ﾋﾟﾝｸ] v2.52                            by てんかﾐ☆\n"
 "usage: pimk [-opts] [.FMT] [@FILE] filename…\n"
 " .FMT    拡張子無視でﾌｫｰﾏｯﾄを.pi|.mag|.q0|.djp に強制\n"
 " @FILE   FILE より画像ファイル名を取得\n"
 " -k[N]   ｷｰ入力ﾓｰﾄﾞ([HELP]ｷｰで説明)   -a[N,M] ﾄﾞｯﾄの横縦比 N:横比 M:縦比  \n"
 "         N:終了時のﾄｰﾝ[0-200％]       -ab     4倍拡大読込ﾓｰﾄﾞ             \n"
// -kn[N]  -kに同じ.ｽｸﾛｰﾙでのWAITを省く     	(ﾄﾞｯﾄ縦横比は無視)  	    \n"
 " -kk[N]  -kに同じ.ｽｸﾛｰﾙでWAIT挿入             (ﾄﾞｯﾄ縦横比は無視)          \n"
 " -l[X,Y] 指定座標に表示               -at     1/4縮小読込ﾓｰﾄﾞ             \n"
 " -lc[N]  透明色指定. N:0～15(255)     -ay     1/16縮小読込ﾓｰﾄﾞ            \n"
 " -ln     強制256色ﾓｰﾄﾞon -ln off 　   -ll[-]  ｽｸﾛｰﾙでﾙｰﾌﾟ･ﾓｰﾄﾞ on/[-]off  \n"
 " -lv[N]  終了時のﾄｰﾝ[0-200％]         -ly[N]  はみ出した部分をｽｸﾛｰﾙして   \n"
 " -v[N]   ﾊﾟﾚｯﾄ のﾄｰﾝ[0-200％]                 0:表示しない 1:表示する     \n"
 " -b[N]   16色画をN[0|1]ﾍﾟｰｼﾞに表示    -pc[-]  16|256自動判別on/[-]off     \n"
 " -c      表示前に画面消去             -n[STR] 256色画像の表示方法         \n"
 " -e[N]   最大使用EMSをNｷﾛﾊﾞｲﾄに設定           -nS:16色ﾃﾞｨｻﾞ               \n"
 " -m      下位ﾃﾞｨﾚｸﾄﾘを再帰的に検索            -nW:力技(2画面切替)         \n"
 " -w[N]   連続表示で表示毎にN/10秒待つ         -nMP:PC386M/P               \n"
 "         (ﾃﾞﾌｫﾙﾄ5)                            -n21[a|b,0|1|2]:PC9821      \n"
 " -j[-]   ﾌﾟﾗｸﾞｲﾝする/-j-しない（一番最初に-j-を指定すれば.cfg読込しない） \n"
 //" -nWAB[,M,XPOS,YPOS]:WAB	 \n"
 //" -nSF:HF+,SF2   	    	 \n"
 //" -le     標準16色で画面offにして終了 \n"
 "\n"
 "※ キー入力モードでは[HELP]キーで説明が表示されます\n"
 );
 exit(1);
}


int DoOpt(void)
{
    int err;
    err = 1;

    if (gClsFlg) {
    	V98_GCls();
    }
    gVf_old = &(gVvfunc[gC256mode]);
    gVf_old->Start();
    if (gC256mode >= N_WAB) {
    	gVf_old->GStart();
    	err = 0;
    }
    if (gClsFlg) {
    	V98_GCls();
    	gVf_old->GCls();
    	err = 0;
    }
  #if 0
    if (gKeyWaitFlg) {
    	Key_Wait();
    	err = 0;
    }
    if (gLeaveTone >= 0) {
    	gVf_old->SetTone(gLeaveTone);
    	err = 0;
    }
  #endif
    if (gC256mode >= N_WAB) {
    	gVf_old->GEnd();
    }
    gVf_old->End(gLeaveTone);
    return err;
}

void Option(BYTE *p)
{
    BYTE *p0 = p;
    int c;

    p++;
    c = *p++;
    c = toupper(c);
    switch(c) {
    case '?':
    case '\0':	    //ヘルプ
    	Usage();
    	break;
    case 'C':	    //画面消去
    	gClsFlg = 1;
    	if (*p == '-')
    	    gClsFlg = 0;
    	break;
    case 'K':	    //キー入力
    	gKeyWaitFlg = 1;
    	if (*p == 'N' || *p == 'n') {
    	    p++;
    	    gKeyWaitMode = 0;
    	} else if (*p == 'K' || *p == 'k') {
    	    p++;
    	    gKeyWaitMode = 1;
    	}
    	if (*p) {
    	    gLeaveTone = (int)strtol(p,NULL,10);
    	} else {
    	    gLeaveTone = 50/*-1*/;
    	}
    	break;
    case 'B':
    	gVramPage = (int)strtol(p,NULL,10);
    	if (gVramPage != 0 && gVramPage != 1) {
    	    goto OPTERR;
    	}
    	break;
    case 'V':	    //トーン
    	gTone = (int)strtol(p,NULL,10);
    	if (gTone > 200) {
    	    goto OPTERR;
    	}
    	break;
    case 'Z':	    //デバッグ
    	gYmax = (int)strtol(p,NULL,10);
    	gDebugFlg = 1;
    	break;
    case 'W':	    //表示毎のウェイト
    	gWaitTime = 5;
    	if (*p) {
    	    gWaitTime = (int)strtol(p,NULL,10);
    	}
    	break;
    case 'M':	    //マルチ・ディレクトリ
    	gMultiDirFlg = 1;
    	break;
    case 'E':
    	if (*p == '\0') {
    	    gEmsMax = 0x7FF0;
    	} else {
    	    gEmsMax = (int)strtol(p,NULL,10);
    	    if (gEmsMax % 16) {
    	    	gEmsMax += 16;
    	    }
    	    gEmsMax >>= 4;
    	}
    	break;
    case 'N':	    //256画像の表示方法
    	if (*p == '\0') {
    	    gC256mode = N_WL;
    	    gForce256flg = 0;
    	} else if (stricmp(p,"S") == 0) {
    	    gC256mode = N_S;
    	    gForce256flg = 0;
    	} else if (stricmp(p,"W") == 0) {
    	    gC256mode = N_WL;
    	    gForce256flg = 0;
    	} else if (stricmp(p,"MP") == 0) {
    	    gC256mode = N_MP;
    	    gForce256flg = 0;
    	} else if (strnicmp(p,"WAB",3) == 0) {
    	    int mode,hpos,vpos;
    	    gForce256flg = 1;
    	    gC256mode = N_WAB;
    	    gCls2Flg = 1;
    	    p += 3;
    	    mode = -1; hpos = 0; vpos = 0;
    	    if (*p == '2') {
    	    	WAB256_Sw2000(1);
    	    	p++;
    	    }
    	    if (*p == 'l' || *p == 'L') {   //実験ﾙｰﾁﾝ^^;
    	    	int xx,yy;
    	    	++p;
    	    	xx = yy = 0;
    	    	//xx = (int)strtol(p,(void *)(&p),10);
    	    	//if (*p == '*') {
    	    	//  ++p;
    	    	    yy = (int)strtol(p,(void *)(&p),10);
    	    	//}
    	    	VWAB_SetLoadMode(xx,yy);
    	    }
    	    if (*p != '\0') {
    	    	++p;
    	    	mode = (int)strtol(p,(void *)(&p),16);
    	    	if (*p != '\0') {
    	    	    ++p;
    	    	    hpos = (int)strtol(p,(void *)(&p),16);
    	    	    if (*p != '\0') {
    	    	    	++p;
    	    	    	vpos = (int)strtol(p,(void *)(&p),16);
    	    	    }
    	    	}
    	    }
    	    //printf("mode=%02x,hpos=%02x,vpos=%02x\n",mode,hpos,vpos);
    	    VWAB_SetMode(mode,hpos,vpos);
    	} else if (strncmp(p,"21",2) == 0) {
    	    int vflg = 0;

    	    gC256mode = N_21;
    	    gCls2Flg = 1;
    	    gForce256flg = 0;
    	    gKeyWaitMode = 1;
    	    //gDspSclFlg = 0;
    	    p += 2;
    	    c = *(p++); c = toupper(c);
    	    if	    (c == 'Z') gV21mode = -2;
    	    else if (c == 'N') gV21mode = -1;
    	    else if (c == 'A') gV21mode =  0;
    	    else if (c == 'B') gV21mode =  1;
    	    //else if (c == 'D') gV21mode = -3;
    	    else    	       --p,gV21mode =  -1;
    	    if (*p != '\0') {
    	    	p++;
    	    	if (*p >= '0' && *p <= '2') {
    	    	    vflg = *p - '0';
    	    	}
    	    }
    	    V21_SetVflg(vflg);
    	    V21_SetMode(gV21mode);
    	} else if (stricmp(p,"SF") == 0) {
    	    gC256mode = N_SF;
    	    gForce256flg = 0;
    	    gKeyWaitMode = 0;
    	    //gDspSclFlg = 0;
    	} else if (stricmp(p,"HF") == 0) {
    	    gC256mode = N_HF;
    	    gKeyWaitMode = 0;
    	    gForce256flg = 0;
    	    //gDspSclFlg = 0;
    	} else {
    	    gC256mode = (int)strtol(p,NULL,10);
    	}
    	if (gC256mode < 0 || gC256mode >= N_CNT) {
    	    goto OPTERR;
    	}
    	gVf_old = &(gVvfunc[gC256mode]);
    	break;
    case 'A':	//アスペクト比
    	c = *p; c = toupper(c);
    	if (c == '\0') {
    	    gAsp1 = gAsp2 = 1;
    	} else if (c == 'B') {
    	    gReductFlg = 0;
    	    gAsp1 = gAsp2 = 2;
    	} else if (c == 'T') {
    	    gReductFlg = 1;
    	} else if (c == 'Y') {
    	    gReductFlg = 2;
    	} else {
    	    gAsp1 = (int)strtol(p,&((char *)p),10);
    	    if (gAsp1 == 0) {
    	    	gAsp1 = 1;
    	    }
    	    gAsp2 = 1;
    	    if (*p != '\0') {
    	    	++p;
    	    	gAsp2 = (int)strtol(p,&((char *)p),10);
    	    	if (gAsp2 == 0) {
    	    	    gAsp2 = 1;
    	    	}
    	    }
    	    if (gAsp1 < 0 || gAsp1 > 127||gAsp2 < 0||gAsp2 > 127){
    	    	gAsp1 = gAsp2 = 0;
    	    }
    	}
    	break;
    case 'L':
    	c = *p++;
    	c = toupper(c);
    	switch(c) {
    	case 'V':
    	    if (*p) {
    	    	gLeaveTone = (int)strtol(p,NULL,10);
    	    } else {
    	    	gLeaveTone = -1/*50*/;
    	    }
    	    break;
    	case 'C':   //透明色
    	    if (*p == '-') {
    	    	gOverlayColor = -1;
    	    } else {
    	    	gOverlayColor = (int)strtol(p,NULL,10)+1;
    	    	if (gOverlayColor > 255+1) {
    	    	    goto OPTERR;
    	    	}
    	    }
    	    break;
    	case 'L':   //ループモード
    	    gLoopSclFlg = 3;
    	    if (*p == '-' || *p == '0') {
    	    	gLoopSclFlg = 0;
    	    }
    	    break;
    	case 'N':   //強制256モード
    	    gForce256flg = 1;
    	    if (*p == '-' || *p == '0') {
    	    	gForce256flg = 0;
    	    }
    	    break;
    	case 'E':   //256色終了時、強制的に16色にする
    	    gEnd256mode = 0;
    	    if (*p >= '0' && *p <= '1') {
    	    	gEnd256mode = *p - '0';
    	    }
    	    break;
    	case 'Y':   //スクロールの有無
    	    gDspSclFlg = 1;
    	    if (*p) {
    	    	gDspSclFlg = (int)strtol(p,NULL,10);
    	    }
    	    if (gDspSclFlg > 1) {
    	    	gDspSclFlg = 0;
    	    }
    	    break;
    	default://始点
    	    if (isdigit(c)) {
    	    	--p;
    	    	gXstart = (int)strtol(p,&((char *)p),10);
    	    	gYstart = 0;
    	    	if (*p != '\0') {
    	    	    gYstart = (int)strtol(p+1,NULL,10);
    	    	}
    	    } else {
    	    	goto OPTERR;
    	    }
    	}
    	break;
    case 'P':
    	c = *p++;
    	c = toupper(c);
    	if (c == 'C') {
    	    if (*p == '-') {
    	    	gAutoChk16 = 0;
    	    	//DJP_AutoChk16(0);
    	    } else {
    	    	gAutoChk16 = 1;
    	    	//DJP_AutoChk16(1);
    	    }
    	} else {
    	    goto OPTERR;
    	}
    	break;
  #if FMT_PLUG
    case 'J':
    	gPlgFlg = (*p != '-');
    	break;
  #endif
    default:
  OPTERR:
    	printf("オプションがおかしい %s\n",p0);
    	exit(1);
    }
}


int GetCfg(char *argv0)
    // argv0 : .cfg ファイルのパス＆ファイル名
{
    char nm[300];
    int hdl,l,i;
    char *s,*p,*q;

    FIL_ChgExt(strcpy(nm,argv0), "CFG");
    if ((hdl = _open(nm,0)) == -1)
    	return -1;
    FIL_GetLt(hdl);
    l = 0;
    while ((s = (BYTE *)FIL_GetLt(-1)) != NULL) {
    	++l;
    	while (*s && *(BYTE *)s <= 0x20)
    	    s++;
    	if (*s == 0 || *s == ';' || *s == '#')
    	    continue;
    	if (*s == '-') {
    	    s = strtok(s," \t\r");
    	    if (stricmp(s,"-END-") == 0)
    	    	goto RET;
    	    Option(s);
    	    continue;
    	}
    	if (*s == '.')
    	    s++;
    	p = strtok(s," .\t");
    	if (p == NULL)
    	    continue;
    	strupr(p);
    	if (strlen(p) > 3) {
    	    printf("%s %d : 変換元拡張子が3ﾊﾞｲﾄ以上だ(%s)\n",nm,l,p);
    	    goto ERR;
    	}
    	for (i = 0; i < gPlugCnt; i++) {
    	    if (strcmp(p,gPlug[i].ext) == 0) {
    	    	printf("%s %d : %sの変換の指定が複数ある\n",nm,l,p);
    	    	goto ERR;
    	    }
    	}
    	q = strtok(NULL," \t.:");
    	if (q == NULL)
    	    continue;
    	strupr(q);
    	for (i = 1; gFt[i].ext[0] != 0; i++) {
    	    if (strcmp(q,gFt[i].ext) == 0)
    	    	goto J2;
    	}
    	printf("%s %d : 変換できない変換先拡張子名が指定された(%s)\n",nm,l,q);
    	goto ERR;
      J2:
    	strcpy(gPlug[gPlugCnt].ext, p);
    	gPlug[gPlugCnt].tgtFmt = i;
    	gPlug[gPlugCnt].cmd = strdup(strtok(NULL,""));
    	if (gPlug[gPlugCnt].cmd == NULL) {
    	    printf("%s %d : メモリが足りなくなった\n",nm,l);
    	    goto ERR;
    	}
      #if 0
    	printf("%3s -> %3s : %s\n",
    	    gPlug[gPlugCnt].ext,
    	    gFt[gPlug[gPlugCnt].tgtFmt].ext,
    	    gPlug[gPlugCnt].cmd);
      #endif
    	gPlugCnt++;
      J1:;
    }
  RET:
    _close(hdl);
    return 0;
  ERR:
    _close(hdl);
    return -1;
}


#define AT_FIL
#ifdef AT_FIL
char *gAtFil=NULL, gAtFil0[sizeof(char*)];

int GetAtFil(char *nm)
{
    int hdl,c;
    char *s,*p;

    if ((hdl = _open(nm,0)) == -1)
    	return -1;
    FIL_GetLt(hdl);
    gAtFil = gAtFil0;
    while ((s = FIL_GetLt(-1)) != NULL) {
    	for (c = 1;c;) {
    	    while (*s && *(BYTE *)s <= 0x20)	    	//空白をスキップ
    	    	s++;
    	    if (*s == 0 || *s == ';'/*|| *s == '#'*/)	//空行 or コメント
    	    	goto J1;
    	    p = s;
    	    while (*(BYTE *)s > 0x20)
    	    	s++;
    	    c = *s;
    	    *s++ = '\0';
    	    strupr(p);
    	  #if 0
    	    if (*p == '-') {	    	    	    //オプション
    	    	Option(p);
    	    	continue;
    	    }
    	  #endif
    	    gAtFil = *((char**)gAtFil) = malloc(strlen(p)+sizeof(char*)+1);
    	    memset(gAtFil, 0, sizeof(char*));
    	    strcpy(gAtFil+sizeof(char*), p);
    	}
      J1:;
    }
    _close(hdl);
    return 0;
  ERR:
    return -1;
}

#endif

int main (int argc, char *argv[])
{
    //const MT_PARASIZE = 4*1024/16;
    static int brk,err,fmt = 0;
    static char filname[16];
    static char path[514];
    int i,c;
    BYTE *p;

    if (argc < 2) {
    	Usage();
    }
  #if FMT_PLUG
    //テンポラリ・ディレクトリ名を得る
    p = getenv("TMP");
    if (p == NULL)
    	p = getenv("TEMP");
    if (p == NULL)
    	p = ".\\";
    strcpy(gPlugInTmpNam,p);
    p = gPlugInTmpNam + strlen(p);
    if (p[-1] != ':' && p[-1] != '\\' && p[-1] != '/')
    	strcat(gPlugInTmpNam,"\\");
    strcat(gPlugInTmpNam,"pimk_tmp.tmp");
  #endif

  #if FMT_PLUG
    // オプション＆プラグイン定義ファイルを読み込む
    if (stricmp(argv[1],"-J-")) {// 一番最初のﾄｰｸﾝが-J-ならば.CFGは無視
    	GetCfg(argv[0]);
    }
  #endif

    // オプションの処理
    for (c = i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-' && p[1] != '-') {
    	    Option(p);
    	} else if (*p == '.' && p[1] != '.' && p[1] != '/' && p[1] != '\\') {
    	    p++;
    	    for (fmt = 1; gFt[fmt].ext[0] != '\0'; fmt++) {
    	    	if (stricmp(p,gFt[fmt].ext) == 0)
    	    	    break;
    	    }
    	    if (gFt[fmt].ext[0] == '\0')
    	    	fmt = 0;
      #ifdef AT_FIL
    	} else if (*p == '@') {
    	    GetAtFil(p+1);
    	    c = 0;
      #endif
    	} else {
    	    c = 0;
    	}
    }
    if (gLeaveTone < 0) {
    	if (gTone < 0) {
    	    gLeaveTone = 50;
    	} else {
    	    gLeaveTone = gTone;
    	}
    }
    if (gTone < 0) {
    	gTone = 100;
    }
    if (c) {
    	if (DoOpt()) {
    	    printf ("ファイル名が指定されてないよぉ...\n");
    	}
    	Exit(1);
    }
    // 初期化
    signal(SIGINT,Exit);

    // メモリ確保
    if (MmInit())
    	goto MEM_ERR;
    EmsInit();

    Key_Init();     	    	//キー関係開始
    Text_PFKeySw(0);	    	/*ｶｰｿﾙoff;PF表示off*/
    Text_CursorSw(0);

    brk = getcbrk();
    setcbrk(1);

    Grph_Analog();
    Grph_Show(1);

    //ファイル毎に処理
    if (gClsFlg) {//||(gC256mode==N_21&&gForce256flg==1)){ //PC9821初期ﾀｲﾌﾟ対策
    	V98_GCls();
    }
    gVf_old->Start();

    // PC9821 のとき
    if (gC256mode == N_21) {
    	if (V21_scnMode == 0 || V21_scnMode == 0xff){//起動時が16色モードならば
    	    Grph_DspPage(gVramPage);
    	    Grph_ActPage(gVramPage);
    	    gVf_old = &(gVvfunc[N_S]);
    	}
    }

    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (p == NULL || *p == '-' || /* *p == '/' ||*/ *p == '\0')
    	    continue;
    	FIL_FullPath(p, path);
    	p = (path+strlen(path)) - 1;
    	if (*p == ':' || *p == '\\' || *p == '/') {
    	    *++p = '*';
    	    *++p = 0;
    	}
    	FIL_AddExt(path, gFt[fmt].ext);
    	filname[0] = '\\';
    	strcpy(filname+1, FIL_BaseName_N(path));
      //printf("Path=%s fname=%s\n",path,filname);
    	if (Foo(path,filname,fmt))
    	    goto RET0;
    }
  #ifdef AT_FIL
    {
    	char *q;
    	for (q = *(char**)gAtFil0; q; q = *(char**)q) {
    	    FIL_FullPath(q+sizeof(char*), path);
    	    p = (path+strlen(path)) - 1;
    	    if (*p == ':' || *p == '\\' || *p == '/') {
    	    	*++p = '*';
    	    	*++p = 0;
    	    }
    	    FIL_AddExt(path, gFt[fmt].ext);
    	    filname[0] = '\\';
    	    strcpy(filname+1, FIL_BaseName_N(path));
    	    if (Foo(path,filname,fmt))
    	    	goto RET0;
    	}
    }
  #endif
  RET0:
    err = 0;
  RET:
    //終了
    Term();

    setcbrk(brk);
    return err;

  MEM_ERR:
    printf("メモリがたんない...\n");
    err = 1;
    goto RET;
}
