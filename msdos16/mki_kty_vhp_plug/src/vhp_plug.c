/*===========================================================================*/
/*	VHP_PLUG																 */
/*===========================================================================*/

#include "tofmt4p.c"


/*---------------------------------------------------------------------------*/
/*-----    ヘ  ル  プ    ----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Usage(void)
{
	puts(
		"vhp_plug ver1.00                           by Tenka*        1995\n"
		"   * base program: VHPLOAD v1.10 for PC98  by Machine M  1989,90\n"
		"usage: vhp_plug [-a] <COMMAND> <inputfile> <outputfile>\n"
		" <COMMAND> .. DJ505JC, TO_DJP or TO_BMP.\n"
		"              DJ505JCはDJP生成で、640*200画像は640*400画像に変換\n"
		"              それ以外の場合は、ドット比の調整は行いません\n"
		" -a ......... DJ505JC以外でも 640*200画像を 640*400 に変換します\n"
		);
	exit(1);
}


/*---------------------------------------------------------------------------*/
/*-----    入 力 ル ー チ ン   ----------------------------------------------*/
/*---------------------------------------------------------------------------*/

#define XLMAX 160
#define YLMAX 200

/* エラー */
#define ER_EXIT   0
#define ER_BUG    1
#define ER_WRITE  2 /* Sa */
#define ER_NTLONG 3 /* Sa */
#define ER_SW     4 /* Sa */
#define ER_AB     5 /* Lo */
#define ER_OPEN   6 /* Lo */

/* ハフマン符号化 */
#define MAXHUF 1024

/* パターン検索方向 */
#define UP     1
#define DOWN   2
#define LEFT   3
#define RIGHT  4

/* ペイント時の最大ネスト数 */
#define MAXNEST 500

/* パターンテーブルの大きさ */
#define MAXTBSIZE 256

/* ｐａｓｓ２パターンテーブルの大きさ */
#define MAXP2TB 1024

/* 画像データの属性 */
#define AT_200     0x00
#define AT_400     0x01
#define AT_C8      0x00
#define AT_C16     0x02
#define AT_DIGITAL 0x00
#define AT_ANALOG  0x04

/* Pass2 の識別子 */
#define P_NON        0x00
#define P_SAMEUP     0x1ff    /* ０は非圧縮に使用する */
#define P_SAMEDOWN   0x0f
#define P_SAMELEFT   0x10
#define P_SAMERIGHT  0x1f
#define P_SAME1UP    0x20

#define P_COL0       0x2f
#define P_COL1       0x30
#define P_COL2       0x3f
#define P_COL3       0x40
#define P_COL4       0x4f
#define P_COL5       0x50
#define P_COL6       0x5f
#define P_COL7       0x60

#define P_COL8       0x200
#define P_COL9       0x20f
#define P_COL10      0x210
#define P_COL11      0x21f
#define P_COL12      0x220
#define P_COL13      0x22f
#define P_COL14      0x230
#define P_COL15      0x23f

#define P_LR11       0x6f
#define P_LR13       0x70
#define P_LR31       0x7f
#define P_LR22       0x80

#define P_LU11       0x8f
#define P_LU13       0x90
#define P_LU31       0x9f
#define P_LU22       0xa0

#define P_LD11       0xaf
#define P_LD13       0xb0
#define P_LD31       0xbf
#define P_LD22       0xc0

#define P_UR11       0xcf
#define P_UR13       0xd0
#define P_UR31       0xdf
#define P_UR22       0xe0

#define P_DR11       0xef
#define P_DR13       0xf0
#define P_DR31       0xff
#define P_DR22       0x100

#define P_UD11       0x10f
#define P_UD13       0x110
#define P_UD31       0x11f
#define P_UD22       0x120

#define P_DU11       0x12f
#define P_DU13       0x130
#define P_DU31       0x13f
#define P_DU22       0x140

#define P_S0         0x14f
#define P_SU1        0x150
#define P_SU2        0x15f
#define P_SU3        0x160
#define P_SD1        0x16f
#define P_SD2        0x170
#define P_SD3        0x17f

#define P_2COL1      0x180
#define P_2COL2      0x18f
#define P_2COL3      0x190
#define P_2COL4      0x19f
#define P_2COL5      0x1a0
#define P_2COL6      0x1af
#define P_2COL7      0x1b0
#define P_2COL8      0x1bf
#define P_2COL9      0x1c0
#define P_2COLA      0x1cf
#define P_2COLB      0x1d0
#define P_2COLC      0x1df
#define P_2COLD      0x1e0
#define P_2COLE      0x1ef

#define P_LC         0x00
#define P_RC         0x80
#define P_UC         0x100
#define P_DC         0x180


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/* RGBI データ */
static UCHAR patB1, patR1, patG1, patI1;
static UCHAR patB2, patR2, patG2, patI2;
static UCHAR patB,  patR,  patG,  patI;
static UCHAR rpatB, rpatR, rpatG, rpatI;
/* 最後に使ったパターンの2色を退避(Pass2) */
static int  unCol1, unCol2;
/* 8色 or 16色 */
static int  col16Flg;
/* ペイント */
static int  yoffset;
/* ビットマップテーブル */
static int  ymax;
#ifdef TST8BIT  /* tiny modelでのｺﾝﾊﾟｲﾙを試してみるとき */
  /* vbufは実際には下2bitsしか使われていないので、それをつめてvbufのサイズを
	小さく(1/4)する。ただしvbufに対するｱｸｾｽが余計に時間がかかることになる。*/
  static UCHAR vbuf[YLMAX][XLMAX/4];
  static int  GetVbuf(int x,int y);
#else
  static UCHAR vbuf[YLMAX][XLMAX];               /* 4*2 ビットマップバッファ */
  #define GetVbuf(x,y)    (vbuf[y][x])
#endif


/*----------------------------------------------*/

/* Loader の ビットマップデータの識別子 */
#define VL_PAINT   1
#define VL_PAINTED 2

/* パターンテーブル */
static UCHAR patTblB[MAXTBSIZE],
			patTblR[MAXTBSIZE],
			patTblG[MAXTBSIZE],
			patTblI[MAXTBSIZE];
static USHORT patTblPtr;               /* パターンテーブルの位置　*/

static int  p2tbBit;

/* ハフマン符号化 */
struct HUF {
  #ifndef TST8BIT
	USHORT num;
	USHORT code;
	USHORT codenum;     /* 0-15 */
  #else
	USHORT num;
	USHORT code;
	UCHAR codenum;		/* 0-15 */
  #endif
};
static struct HUF hufBuf[MAXHUF];

static USHORT cthbuf = 0;

/*---------------------------------------------------------------------------*/

static void Error(void)
{
	puts("[VHP-DECODER] Abnormal program termination.\n");
	exit(1);
}

/*---------------------------------------------------------------------------*/
static FILE *FL_fp;
#define FL_Init()
#define FL_GetC()       fgetc(FL_fp)
#define FL_Close()      fclose(FL_fp)

static void FL_Open(char *fname)
{
	if ((FL_fp = fopen(fname, "rb")) == NULL) {
		printf("[VHP-DECODER] Unable to open file %s\n",fname); exit(1);
	}
	if( setvbuf(FL_fp, NULL, _IOFBF, VBUFSIZE) ) {
		puts("[VHP-DECODER] Out of memory.\n"); exit(1);
	}
}


/*---------------------------------------------------------------------------*/
/* データ入力ルーチン */
static int  getBit_mask;

static void InitGetBit(void)
{
	getBit_mask = 0;
}

static USHORT Get1Bit(void)
{
	static int buf;

	if ((getBit_mask >>= 1) == 0) {
		if ((buf = FL_GetC()) < 0) {
			puts("[VHP-DECODER] Unexpected end-of-file."); exit(1);
		}
		getBit_mask = 0x80;
	}
	return (buf & getBit_mask) ? 1 : 0;
}

static USHORT GetBits(int n)
{
	USHORT x;

	x = 0;
	while (n-- > 0)
		x = (x << 1) + Get1Bit();
	return x;
}


static USHORT GetHfData(void)
	/*
	 *ファイルから入力したハフマン符号を変換する
	 */
{
	int  n, p, code;

	n = p = code = 0;
	while (n++ <= 16) {
		code = (code << 1) | Get1Bit();
		while (hufBuf[p].codenum < n)
			++p;
		while (hufBuf[p].codenum == n) {
			if (hufBuf[p].code == code)
				return hufBuf[p].num;
			++p;
		}
	}
	puts("[VHP-DECODER] unknown huffman bits"); exit(1);
	return 0;
}


/*----- VRAMの読み書き　-----------------------------------------------------*/
/* VRAM */
static UCHAR FAR *v_b, FAR *v_r, FAR *v_g, FAR *v_i;

#if 0
static void clrVram(void)
{
	int i;

	for (i = 0; i < 80*400;i++) {
		v_b[i] = v_r[i] = v_g[i] = v_i[i] = 0;
	}
}
#endif

/*-------------------------------------*/
#define VOF(x,y) (XLMAX*(y)+(x))

/* RGBI データ */
static UCHAR patB1u, patR1u, patG1u, patI1u;
static UCHAR patB1l, patR1l, patG1l, patI1l;
static UCHAR patB2u, patR2u, patG2u, patI2u;
static UCHAR patB2l, patR2l, patG2l, patI2l;
static UCHAR patB3u, patR3u, patG3u, patI3u;
static UCHAR patB3l, patR3l, patG3l, patI3l;
static UCHAR patB4u, patR4u, patG4u, patI4u;
static UCHAR patB4l, patR4l, patG4l, patI4l;


static void Get4bitPat(int x,int y,int w,UCHAR *b,UCHAR *r,UCHAR *g,UCHAR *i)
{
	USHORT v;

	v = VOF( ((x >> 1) + ((w == 1) ? 0 : 80)) , y);
	if (x & 1) {
		*b = (UCHAR) (*(v_b + v) & 0xf);
		*r = (UCHAR) (*(v_r + v) & 0xf);
		*g = (UCHAR) (*(v_g + v) & 0xf);
		*i = (UCHAR) (*(v_i + v) & 0xf);
	} else {
		*b = (UCHAR) ((*(v_b + v) >> 4) & 0xf);
		*r = (UCHAR) ((*(v_r + v) >> 4) & 0xf);
		*g = (UCHAR) ((*(v_g + v) >> 4) & 0xf);
		*i = (UCHAR) ((*(v_i + v) >> 4) & 0xf);
	}
}

static void ConvPat(void)
{
	patB1u = patB1;
	patR1u = patR1;
	patG1u = patG1;

	patB1l = patB1u >> 4;
	patR1l = patR1u >> 4;
	patG1l = patG1u >> 4;

	patB2u = patB1u << 4;
	patR2u = patR1u << 4;
	patG2u = patG1u << 4;

	patB2l = patB1u & 0x0f;
	patR2l = patR1u & 0x0f;
	patG2l = patG1u & 0x0f;

	patB1u &= 0xf0;
	patR1u &= 0xf0;
	patG1u &= 0xf0;

	patB3u = patB2;
	patR3u = patR2;
	patG3u = patG2;

	patB3l = patB3u >> 4;
	patR3l = patR3u >> 4;
	patG3l = patG3u >> 4;

	patB4u = patB3u << 4;
	patR4u = patR3u << 4;
	patG4u = patG3u << 4;

	patB4l = patB3u & 0x0f;
	patR4l = patR3u & 0x0f;
	patG4l = patG3u & 0x0f;

	patB3u &= 0xf0;
	patR3u &= 0xf0;
	patG3u &= 0xf0;

	if (!col16Flg)
		return;

	patI1u = patI1;
	patI1l = patI1u >> 4;

	patI2u = patI1u << 4;
	patI2l = patI1u & 0x0f;

	patI1u &= 0xf0;

	patI3u = patI2;
	patI3l = patI3u >> 4;

	patI4u = patI3u << 4;
	patI4l = patI3u & 0x0f;

	patI3u &= 0xf0;
}

static void PutPat2(int x, int y)
{
	USHORT v,w;

	if (x & 1) {
		x >>= 1;
		v = VOF(x,y);
		w = v + 80;/* = VOF(x+80,y); */
		v_b[v] = (UCHAR)((v_b[v] & 0xf0) | patB1l);
		v_r[v] = (UCHAR)((v_r[v] & 0xf0) | patR1l);
		v_g[v] = (UCHAR)((v_g[v] & 0xf0) | patG1l);
		v_b[w] = (UCHAR)((v_b[w] & 0xf0) | patB2l);
		v_r[w] = (UCHAR)((v_r[w] & 0xf0) | patR2l);
		v_g[w] = (UCHAR)((v_g[w] & 0xf0) | patG2l);
		if (col16Flg) {
			v_i[v] = (UCHAR)((v_i[v] & 0xf0) | patI1l);
			v_i[w] = (UCHAR)((v_i[w] & 0xf0) | patI2l);
		}

		if (yoffset == 1)
			return;

		v += XLMAX;/* v = VOF(x,y+1);    */
		w += XLMAX;/* w = VOF(x+80,y+1); */
		v_b[v] = (UCHAR)((v_b[v] & 0xf0) | patB3l);
		v_r[v] = (UCHAR)((v_r[v] & 0xf0) | patR3l);
		v_g[v] = (UCHAR)((v_g[v] & 0xf0) | patG3l);
		v_b[w] = (UCHAR)((v_b[w] & 0xf0) | patB4l);
		v_r[w] = (UCHAR)((v_r[w] & 0xf0) | patR4l);
		v_g[w] = (UCHAR)((v_g[w] & 0xf0) | patG4l);
		if (col16Flg) {
			v_i[v] = (UCHAR)((v_i[v] & 0xf0) | patI3l);
			v_i[w] = (UCHAR)((v_i[w] & 0xf0) | patI4l);
		}

	} else {
		x >>= 1;
		v = VOF(x,y);
		w = v + 80;/* = VOF(x+80,y); */
		v_b[v] = (UCHAR)((v_b[v] & 0x0f) | patB1u);
		v_r[v] = (UCHAR)((v_r[v] & 0x0f) | patR1u);
		v_g[v] = (UCHAR)((v_g[v] & 0x0f) | patG1u);
		v_b[w] = (UCHAR)((v_b[w] & 0x0f) | patB2u);
		v_r[w] = (UCHAR)((v_r[w] & 0x0f) | patR2u);
		v_g[w] = (UCHAR)((v_g[w] & 0x0f) | patG2u);
		if (col16Flg) {
			v_i[v] = (UCHAR)((v_i[v] & 0x0f) | patI1u);
			v_i[w] = (UCHAR)((v_i[w] & 0x0f) | patI2u);
		}

		if (yoffset == 1)
			return;

		v += XLMAX;/* v = VOF(x,y+1);    */
		w += XLMAX;/* w = VOF(x+80,y+1); */
		v_b[v] = (UCHAR)((v_b[v] & 0x0f) | patB3u);
		v_r[v] = (UCHAR)((v_r[v] & 0x0f) | patR3u);
		v_g[v] = (UCHAR)((v_g[v] & 0x0f) | patG3u);
		v_b[w] = (UCHAR)((v_b[w] & 0x0f) | patB4u);
		v_r[w] = (UCHAR)((v_r[w] & 0x0f) | patR4u);
		v_g[w] = (UCHAR)((v_g[w] & 0x0f) | patG4u);
		if (col16Flg) {
			v_i[v] = (UCHAR)((v_i[v] & 0x0f) | patI3u);
			v_i[w] = (UCHAR)((v_i[w] & 0x0f) | patI4u);
		}
	}
}

/*---------------------------------------------------------------------------*/
#ifndef TST8BIT     /*ふつうにｺﾝﾊﾟｲﾙしたとき */
  #ifndef GetVbuf
	#define GetVbuf(x,y)    (vbuf[y][x])
  #endif
  #define SetVbuf(x,y,d)  (vbuf[y][x] = (UCHAR)(d))

#else  /* vbuf サイズを小さくするばあい (8bit機に対応するための下準備？)*/

static int GetVbuf(int x, int y)
{
	int  a;

	a = vbuf[y][x/4];
	switch(x % 4) {
	case 0:
		return (a & 0xc0) >> 6;
	case 1:
		return (a & 0x30) >> 4;
	case 2:
		return (a & 0x0c) >> 2;
	}
	return a & 0x03;
}

static void SetVbuf(int x, int y, int data)
{
	USHORT a;

	a = vbuf[y][x/4];
	data &= 0x03;
	switch(x % 4) {
	case 0:
		a &= 0x3f;
		a |= (data << 6);
		break;
	case 1:
		a &= 0xcf;
		a |= (data << 4);
		break;
	case 2:
		a &= 0xf3;
		a |= (data << 2);
		break;
	default:
		a &= 0xfc;
		a |= data;
	}
	vbuf[y][x/4] = a;
	return;
}

#endif


static int GetFlg(int x, int y)
{
	switch (yoffset) {
	case 1:
		return GetVbuf(x,y);
	case 2:
		if (y + 1 >= ymax)
			return GetVbuf(x,y);
		else
			return GetVbuf(x,y) & GetVbuf(x,y+1);
	}
	Error();
	return 0;
}

static void SetFlg(int x, int y, int data)
{
	SetVbuf(x,y,data);
}


static void PutPat(int x, int y)
{
	SetFlg(x, y, VL_PAINTED);
	if ((yoffset == 2) && (y+1 < ymax))
		SetFlg(x, y+1, VL_PAINTED);
	PutPat2(x, y);
}


static void TbSetPat(UCHAR pb, UCHAR pr, UCHAR pg, UCHAR pi)
{
	if (patTblPtr >= MAXTBSIZE)
		return;

	patTblB[patTblPtr] = pb;
	patTblR[patTblPtr] = pr;
	patTblG[patTblPtr] = pg;
	patTblI[patTblPtr] = pi;
	++patTblPtr;
}

static void TbGetPat(int n)
{
	if (n >= MAXTBSIZE)
		Error();

	patB = patTblB[n];
	patR = patTblR[n];
	patG = patTblG[n];
	patI = patTblI[n];
}



/*---------------------------------------------------------------------------*/
static int DrawRight(int x, int y)
{
	++x;
	if (x == XLMAX)
		return XLMAX - 1;
	while (GetFlg(x, y) == VL_PAINT) {   /* --->(rx) */
		PutPat(x, y);
		++x;
		if (x == XLMAX)
			return XLMAX - 1;
	}
	return --x;
}

static int DrawLeft(int x, int y)
{
	while (GetFlg(x, y) == VL_PAINT) {   /* (lx)<--- */
		PutPat(x, y);
		--x;
		if (x < 0)
			return 0;
	}
	return ++x;
}


static void Paint(int x, int y)
{
	int  rx, lx;

	if (y < 0 || y >= ymax)
		return;

	if (GetFlg(x, y) != VL_PAINT)
		return;

	lx = DrawLeft(x, y);
	rx = DrawRight(x, y);

	while (lx <= rx) {
		Paint(lx, y - yoffset);
		Paint(lx, y + yoffset);
		++lx;
	}
}


static int IsSame42(int x, int y)
	/*
	 *  周りにペイントパターンがなければ、真を返す
	 *  （独立した4*2のパターンかどうかを調べる）
	 */
{
	int  flg;

	flg = 1;
	if (x - 1 >= 0)
		if (GetFlg(x - 1, y))
			flg = 0;
	if (x + 1 < XLMAX)
		if (GetFlg(x + 1, y))
			flg = 0;
	if (y - 1 >= 0)
		if (GetFlg(x, y - 1))
			flg = 0;
	if (y + 1 < ymax)
		if (GetFlg(x, y + 1))
			flg = 0;

	return flg;
}


/* データ */
static USHORT tbb;

/*
 * パターンデータの入力
 * A. 0xxx xxxx                             - V_TYPE42 found pattern
 * B. 10bb bbbb bbrr rrrr rrgg gggg gg(i..) - V_TYPE42 not found pattern
 * C. 11                                    - V_TYPE44
 * D.   0x xxxx xx                          -   found pattern
 * E.   1b bbbb bbbr rrrr rrrg gggg ggg(i..)-   not found pattern
 * F. xxxx xxx                              - V_SAME42(識別子がなくても判別可)
 *
 *     b - blue  パターン 8bit(4*2)
 *     r - red   パターン 8bit(4*2)
 *     g - green パターン 8bit(4*2)
 *     x - パターンテーブルの位置 1,2,3,4,5,6 or 7 bit
 *  （１６色の時には gggg gggg のあとに iiii iiii が付く）
 */
static void Pass1(void)
{
	int  x, y, pn;

	for (y = 0; y < ymax; ++y) {
		for (x = 0; x < XLMAX; ++x) {
			if (GetVbuf(x,y) != VL_PAINT)
				continue;
			yoffset = 1;           /* dummy */

			if (IsSame42(x, y))
				continue;

			if (Get1Bit() == 0) {
				pn = GetBits(tbb);
				TbGetPat(pn);
				patB1 = patB;
				patR1 = patR;
				patG1 = patG;
				patI1 = patI;
				ConvPat();
				yoffset = 1;
				Paint(x, y);
			} else {
				if (Get1Bit() == 0) {
					patB1 = GetBits(8);
					patR1 = GetBits(8);
					patG1 = GetBits(8);
					if (col16Flg)
						patI1 = GetBits(8);
					TbSetPat(patB1, patR1, patG1, patI1);
					ConvPat();
					yoffset = 1;
					Paint(x, y);
				} else {
					if (Get1Bit() == 0) {
						pn = GetBits(tbb);
						TbGetPat(pn);
						patB1 = patB;
						patR1 = patR;
						patG1 = patG;
						patI1 = patI;
					} else {
						patB1 = GetBits(8);
						patR1 = GetBits(8);
						patG1 = GetBits(8);
						if (col16Flg)
							patI1 = GetBits(8);
						TbSetPat(patB1, patR1, patG1, patI1);
					}
					if (Get1Bit() == 0) {
						pn = GetBits(tbb);
						TbGetPat(pn);
						patB2 = patB;
						patR2 = patR;
						patG2 = patG;
						patI2 = patI;
					} else {
						patB2 = GetBits(8);
						patR2 = GetBits(8);
						patG2 = GetBits(8);
						if (col16Flg)
							patI2 = GetBits(8);
						TbSetPat(patB2, patR2, patG2, patI2);
					}
					ConvPat();
					yoffset = 2;
					Paint(x, y);
				}
			}
		}
	}

	for (y = 0; y < ymax; ++y) {
		for (x = 0; x < XLMAX; ++x) {
			if (GetVbuf(x,y) != VL_PAINT)
				continue;

			pn = GetBits(tbb);
			TbGetPat(pn);
			patB1 = patB;
			patR1 = patR;
			patG1 = patG;
			if (col16Flg)
				patI1 = patI;
			ConvPat();
			yoffset = 1;           /* dummy */
			PutPat(x, y);
		}
	}
}


/*---------------------------------------------------------------------------*/
/* RGBI データ */
static UCHAR lpatB, lpatR, lpatG, lpatI;
/* 最後に使ったパターンの2色を退避(Pass2) */
static UCHAR lpB, lpR, lpG, lpI;


static int FindPat(int x, int y, int w, int v)
	/*
	 * 上下左右の最初に見つかるペイントパターンをサーチ
	 * in : x,y 現在の座標
	 *      w   high(=1) or low(=0)  4 bits
	 *      v   サーチ方向 (UP, DOWN, LEFT, RIGHT)
	 * out: return value = 0 - パターンが見つからなかった
	 *                     1 - パターンを見つけた
	 *                         (patB,patR,patG,patI)
	 */
{
	int  findflg;

	findflg = 0;
	switch (v) {
	case UP:
		while (--y >= 0)
			if (GetVbuf(x,y)) {
				findflg = 1;
				break;
			}
		break;
	case DOWN:
		while (++y < ymax)
			if (GetVbuf(x,y)) {
				findflg = 1;
				break;
			}
		break;
	case LEFT:
		while (--x >= 0)
			if (GetVbuf(x,y)) {
				findflg = 1;
				break;
			}
		break;
	case RIGHT:
		while (++x < XLMAX)
			if (GetVbuf(x,y)) {
				findflg = 1;
				break;
			}
		break;
	}
	if (!findflg) {
		puts("[VHP-DECODER] painted pattern not found."); exit(1);
	}
	Get4bitPat(x, y, w, &patB, &patR, &patG, &patI);

	return findflg;
}


static int Get2Color(void)
	/*
	 *  パターンlp? で使用している２色を返す
	 */
{
	USHORT i, colorcode, c1;

	c1 = 0xff;
	for (i = 0x8; i != 0; i >>= 1) {
		if (i & lpB)
			colorcode = 1;
		else
			colorcode = 0;
		if (i & lpR)
			colorcode |= 2;
		if (i & lpG)
			colorcode |= 4;
		if ((i & lpI) && col16Flg)
			colorcode |= 8;

		if (c1 != colorcode) {
			if (c1 != 0xff)
				return (c1 << 4) | colorcode;
			c1 = colorcode;
		}
	}
	return (unCol1<<4)|unCol2; /* 2色以上でなければ、最後に使用した2色を返す */
}

static void Pp_L2col(int x, int y, int w, int n)
{
	UCHAR pat;
	USHORT lcol12, lcol1, lcol2;

	pat = ((n - P_2COL1) >> 3) + 1;
	lcol12 = Get2Color();
	lcol1 = lcol12 >> 4;
	lcol2 = lcol12 & 0xf;

	if (lcol1 & 1)
		patB = ~pat & 0xf;
	else
		patB = 0;
	if (lcol1 & 2)
		patR = ~pat & 0xf;
	else
		patR = 0;
	if (lcol1 & 4)
		patG = ~pat & 0xf;
	else
		patG = 0;
	if (lcol1 & 8)
		patI = ~pat & 0xf;
	else
		patI = 0;

	if (lcol2 & 1)
		patB |= pat;
	if (lcol2 & 2)
		patR |= pat;
	if (lcol2 & 4)
		patG |= pat;
	if (lcol2 & 8)
		patI |= pat;

	unCol1 = lcol1;
	unCol2 = lcol2;
}


static void Pp_Non(int x, int y, int w, int n)
{
	patB = GetBits(4);
	patR = GetBits(4);
	patG = GetBits(4);
	if (col16Flg)
		patI = GetBits(4);
}

static void Pp_Same(int x, int y, int w, int n)
	/*
	 * 上下左右で同じパターンを探す
	 */
{
	switch (n) {
	case P_SAMEDOWN:
		FindPat(x, y, w, DOWN);
		break;
	case P_SAMEUP:
		FindPat(x, y, w, UP);
		break;
	case P_SAMERIGHT:
		FindPat(x, y, w, RIGHT);
		break;
	case P_SAMELEFT:
		FindPat(x, y, w, LEFT);
		break;
	}
}

static void Pp_Same2(int x, int y, int w, int n)
	/*
	 * 一つ上のパターン
	 */
{
	Get4bitPat(x, y - 1, w, &patB, &patR, &patG, &patI);
}


static void Pp_Mkp1c(int n)
{
	patB = (patB & (~(n & 0xf))) | (n & 0x10 ? (n & 0xf) : 0);
	patR = (patR & (~(n & 0xf))) | (n & 0x20 ? (n & 0xf) : 0);
	patG = (patG & (~(n & 0xf))) | (n & 0x40 ? (n & 0xf) : 0);
	if (!col16Flg)
		return;
	patI = (patI & (~(n & 0xf))) | (n & 0x200 ? (n & 0xf) : 0);
}

static void Pp_L1c(int x, int y, int w, int n)
	/*
	 * 左のパターンと他の１カラー
	 */
{
	FindPat(x, y, w, LEFT);
	Pp_Mkp1c(n);
}

static void Pp_R1c(int x, int y, int w, int n)
	/*
	 * 右のパターンと他の１カラー
	 */
{
	FindPat(x, y, w, RIGHT);
	Pp_Mkp1c(n);
}

static void Pp_U1c(int x, int y, int w, int n)
	/*
	 * 上のパターンと他の１カラー
	 */
{
	FindPat(x, y, w, UP);
	Pp_Mkp1c(n);
}

static void Pp_D1c(int x, int y, int w, int n)
	/*
	 * 下のパターンと他の１カラー
	 */
{
	FindPat(x, y, w, DOWN);
	Pp_Mkp1c(n);
}


/*=============================*/
static void Pp_Col(int x, int y, int w, int n)
	/*
	 * 一色のブロック
	 */
{
	static int ccode[][4] = {
		{0x0, 0x0, 0x0, 0x0},
		{0xf, 0x0, 0x0, 0x0},
		{0x0, 0xf, 0x0, 0x0},
		{0xf, 0xf, 0x0, 0x0},
		{0x0, 0x0, 0xf, 0x0},
		{0xf, 0x0, 0xf, 0x0},
		{0x0, 0xf, 0xf, 0x0},
		{0xf, 0xf, 0xf, 0x0},
		{0x0, 0x0, 0x0, 0xf},
		{0xf, 0x0, 0x0, 0xf},
		{0x0, 0xf, 0x0, 0xf},
		{0xf, 0xf, 0x0, 0xf},
		{0x0, 0x0, 0xf, 0xf},
		{0xf, 0x0, 0xf, 0xf},
		{0x0, 0xf, 0xf, 0xf},
		{0xf, 0xf, 0xf, 0xf}
	};
	int  i;

	switch (n) {
	case P_COL0:
		i = 0;
		break;
	case P_COL1:
		i = 1;
		break;
	case P_COL2:
		i = 2;
		break;
	case P_COL3:
		i = 3;
		break;
	case P_COL4:
		i = 4;
		break;
	case P_COL5:
		i = 5;
		break;
	case P_COL6:
		i = 6;
		break;
	case P_COL7:
		i = 7;
		break;
	case P_COL8:
		i = 8;
		break;
	case P_COL9:
		i = 9;
		break;
	case P_COL10:
		i = 10;
		break;
	case P_COL11:
		i = 11;
		break;
	case P_COL12:
		i = 12;
		break;
	case P_COL13:
		i = 13;
		break;
	case P_COL14:
		i = 14;
		break;
	case P_COL15:
		i = 15;
		break;
	}
	patB = ccode[i][0];
	patR = ccode[i][1];
	patG = ccode[i][2];
	patI = ccode[i][3];
}

static void SetLR(int n)
{
	switch (n) {
	case 5:
		patB = (UCHAR)((lpatB & 0x08) | (rpatB & 0x01));
		patR = (UCHAR)((lpatR & 0x08) | (rpatR & 0x01));
		patG = (UCHAR)((lpatG & 0x08) | (rpatG & 0x01));
		patI = (UCHAR)((lpatI & 0x08) | (rpatI & 0x01));
		break;

	case 6:
		patB = (UCHAR)((lpatB & 0x08) | (rpatB & 0x03));
		patR = (UCHAR)((lpatR & 0x08) | (rpatR & 0x03));
		patG = (UCHAR)((lpatG & 0x08) | (rpatG & 0x03));
		patI = (UCHAR)((lpatI & 0x08) | (rpatI & 0x03));
		break;

	case 9:
		patB = (UCHAR)((lpatB & 0x0c) | (rpatB & 0x01));
		patR = (UCHAR)((lpatR & 0x0c) | (rpatR & 0x01));
		patG = (UCHAR)((lpatG & 0x0c) | (rpatG & 0x01));
		patI = (UCHAR)((lpatI & 0x0c) | (rpatI & 0x01));
		break;

	case 0xa:
		patB = (UCHAR)((lpatB & 0x0c) | (rpatB & 0x03));
		patR = (UCHAR)((lpatR & 0x0c) | (rpatR & 0x03));
		patG = (UCHAR)((lpatG & 0x0c) | (rpatG & 0x03));
		patI = (UCHAR)((lpatI & 0x0c) | (rpatI & 0x03));
		break;

	default:
		Error();
	}
}

static void Pp_LR(int x, int y, int w, int n)
	/*
	 * 左右のパターンを組み合わせる
	 */
{
	FindPat(x, y, w, LEFT);     /* 左のパターン読み込み */
	lpatB = patB;
	lpatR = patR;
	lpatG = patG;
	lpatI = patI;

	FindPat(x, y, w, RIGHT);    /* 右のパターン読み込み */
	rpatB = patB;
	rpatR = patR;
	rpatG = patG;
	rpatI = patI;

	SetLR(n);
}

static void Pp_LU(int x, int y, int w, int n)
	/*
	 * 左上のパターンを組み合わせる
	 */
{
	FindPat(x, y, w, LEFT);     /* 左のパターン読み込み */
	lpatB = patB;
	lpatR = patR;
	lpatG = patG;
	lpatI = patI;

	FindPat(x, y, w, UP);       /* 上のパターン読み込み */
	rpatB = patB;
	rpatR = patR;
	rpatG = patG;
	rpatI = patI;

	SetLR(n);
}

static void Pp_LD(int x, int y, int w, int n)
	/*
	 * 左下のパターンを組み合わせる
	 */
{
	FindPat(x, y, w, LEFT);     /* 左のパターン読み込み */
	lpatB = patB;
	lpatR = patR;
	lpatG = patG;
	lpatI = patI;

	FindPat(x, y, w, DOWN);     /* 下のパターン読み込み */
	rpatB = patB;
	rpatR = patR;
	rpatG = patG;
	rpatI = patI;

	SetLR(n);
}

static void Pp_UR(int x, int y, int w, int n)
	/*
	 * 上右のパターンを組み合わせる
	 */
{
	FindPat(x, y, w, UP);       /* 上のパターン読み込み */
	lpatB = patB;
	lpatR = patR;
	lpatG = patG;
	lpatI = patI;

	FindPat(x, y, w, RIGHT);    /* 右のパターン読み込み */
	rpatB = patB;
	rpatR = patR;
	rpatG = patG;
	rpatI = patI;

	SetLR(n);
}

static void Pp_DR(int x, int y, int w, int n)
	/*
	 * 下右のパターンを組み合わせる
	 */
{
	FindPat(x, y, w, DOWN);     /* 下のパターン読み込み */
	lpatB = patB;
	lpatR = patR;
	lpatG = patG;
	lpatI = patI;

	FindPat(x, y, w, RIGHT);    /* 右のパターン読み込み */
	rpatB = patB;
	rpatR = patR;
	rpatG = patG;
	rpatI = patI;

	SetLR(n);
}


static void Pp_UD(int x, int y, int w, int n)
	/*
	 * 上下のパターンを組み合わせる
	 */
{
	FindPat(x, y, w, UP);       /* 上のパターン読み込み */
	lpatB = patB;
	lpatR = patR;
	lpatG = patG;
	lpatI = patI;

	FindPat(x, y, w, DOWN);     /* 下のパターン読み込み */
	rpatB = patB;
	rpatR = patR;
	rpatG = patG;
	rpatI = patI;

	SetLR(n);
}

static void Pp_DU(int x, int y, int w, int n)
	/*
	 * 下上のパターンを組み合わせる
	 */
{
	FindPat(x, y, w, DOWN);     /* 下のパターン読み込み */
	lpatB = patB;
	lpatR = patR;
	lpatG = patG;
	lpatI = patI;

	FindPat(x, y, w, UP);       /* 上のパターン読み込み */
	rpatB = patB;
	rpatR = patR;
	rpatG = patG;
	rpatI = patI;

	SetLR(n);
}

static void Pp_Sxm1(int x, int y, int w, int n)
	/*
	 * x-1のパターンと比較
	 */
{
	switch (n) {
	case P_S0:
		Get4bitPat(x - 1, y, w, &patB, &patR, &patG, &patI);
		break;
	case P_SU1:
		Get4bitPat(x - 1, y - 1, w, &patB, &patR, &patG, &patI);
		break;
	case P_SU2:
		Get4bitPat(x - 1, y - 2, w, &patB, &patR, &patG, &patI);
		break;
	case P_SU3:
		Get4bitPat(x - 1, y - 3, w, &patB, &patR, &patG, &patI);
		break;
	case P_SD1:
		Get4bitPat(x - 1, y + 1, w, &patB, &patR, &patG, &patI);
		break;
	case P_SD2:
		Get4bitPat(x - 1, y + 2, w, &patB, &patR, &patG, &patI);
		break;
	case P_SD3:
		Get4bitPat(x - 1, y + 3, w, &patB, &patR, &patG, &patI);
		break;
	default:
		Error();
	}
}


#if 0 /*******************/
static void Pass22(int x, int y, int w)
{
	int  n;

	n = GetHfData();
	switch (n) {
	case P_NON:
		Pp_Non(x, y, w, n);
		break;

	case P_SAMEUP:
	case P_SAMEDOWN:
	case P_SAMELEFT:
	case P_SAMERIGHT:
		Pp_Same(x, y, w, n);
		break;

	case P_SAME1UP:
		Pp_Same2(x, y, w, n);
		break;

	case P_COL0:
	case P_COL1:
	case P_COL2:
	case P_COL3:
	case P_COL4:
	case P_COL5:
	case P_COL6:
	case P_COL7:
	case P_COL8:
	case P_COL9:
	case P_COL10:
	case P_COL11:
	case P_COL12:
	case P_COL13:
	case P_COL14:
	case P_COL15:
		Pp_Col(x, y, w, n);
		break;

	case P_2COL1:
	case P_2COL2:
	case P_2COL3:
	case P_2COL4:
	case P_2COL5:
	case P_2COL6:
	case P_2COL7:
	case P_2COL8:
	case P_2COL9:
	case P_2COLA:
	case P_2COLB:
	case P_2COLC:
	case P_2COLD:
	case P_2COLE:
		Pp_L2col(x, y, w, n);
		break;

	case P_LR11:
		Pp_LR(x, y, w, 5);
		break;
	case P_LR13:
		Pp_LR(x, y, w, 6);
		break;
	case P_LR31:
		Pp_LR(x, y, w, 9);
		break;
	case P_LR22:
		Pp_LR(x, y, w, 0xa);
		break;

	case P_LU11:
		Pp_LU(x, y, w, 5);
		break;
	case P_LU13:
		Pp_LU(x, y, w, 6);
		break;
	case P_LU31:
		Pp_LU(x, y, w, 9);
		break;
	case P_LU22:
		Pp_LU(x, y, w, 0xa);
		break;

	case P_LD11:
		Pp_LD(x, y, w, 5);
		break;
	case P_LD13:
		Pp_LD(x, y, w, 6);
		break;
	case P_LD31:
		Pp_LD(x, y, w, 9);
		break;
	case P_LD22:
		Pp_LD(x, y, w, 0xa);
		break;

	case P_UR11:
		Pp_UR(x, y, w, 5);
		break;
	case P_UR13:
		Pp_UR(x, y, w, 6);
		break;
	case P_UR31:
		Pp_UR(x, y, w, 9);
		break;
	case P_UR22:
		Pp_UR(x, y, w, 0xa);
		break;

	case P_DR11:
		Pp_DR(x, y, w, 5);
		break;
	case P_DR13:
		Pp_DR(x, y, w, 6);
		break;
	case P_DR31:
		Pp_DR(x, y, w, 9);
		break;
	case P_DR22:
		Pp_DR(x, y, w, 0xa);
		break;

	case P_UD11:
		Pp_UD(x, y, w, 5);
		break;
	case P_UD13:
		Pp_UD(x, y, w, 6);
		break;
	case P_UD31:
		Pp_UD(x, y, w, 9);
		break;
	case P_UD22:
		Pp_UD(x, y, w, 0xa);
		break;

	case P_DU11:
		Pp_DU(x, y, w, 5);
		break;
	case P_DU13:
		Pp_DU(x, y, w, 6);
		break;
	case P_DU31:
		Pp_DU(x, y, w, 9);
		break;
	case P_DU22:
		Pp_DU(x, y, w, 0xa);
		break;

	case P_S0:
	case P_SU1:
	case P_SU2:
	case P_SU3:
	case P_SD1:
	case P_SD2:
	case P_SD3:
		Pp_Sxm1(x, y, w, n);
		break;

	default:
		if (n < P_RC)
			Pp_L1c(x, y, w, n);
		else if (n < P_UC)
			Pp_R1c(x, y, w, n);
		else if (n < P_DC)
			Pp_U1c(x, y, w, n);
		else if (n < P_LC + 0x200)
			Pp_D1c(x, y, w, n);
		else if (n < P_RC + 0x200)
			Pp_L1c(x, y, w, n);
		else if (n < P_UC + 0x200)
			Pp_R1c(x, y, w, n);
		else if (n < P_DC + 0x200)
			Pp_U1c(x, y, w, n);
		else
			Pp_D1c(x, y, w, n);
	}
	lpB = patB;
	lpR = patR;
	lpG = patG;
	lpI = patI;
}

#else /*******************/

static void Pp_Error(int x, int y, int w, int n)
{
	Error();
}


static void Pass22(int x, int y, int w)
{
	static int tid[] = {
		0x01, 0x02, 0x02, 0x02, 0x03, 0x04, 0x04, 0x04,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x55, 0x65, 0x95,
		0xa5, 0x56, 0x66, 0x96, 0xa6, 0x57, 0x67, 0x97,
		0xa7, 0x58, 0x68, 0x98, 0xa8, 0x59, 0x69, 0x99,
		0xa9, 0x5a, 0x6a, 0x9a, 0xaa, 0x5b, 0x6b, 0x9b,
		0xab, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
		0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
		0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x00, 0x02,

		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	static void (*func[]) (int x, int y, int w, int n) = {
		Pp_Error,
		Pp_Non,
		Pp_Same,
		Pp_Same2,
		Pp_Col,
		Pp_LR,
		Pp_LU,
		Pp_LD,
		Pp_UR,
		Pp_DR,
		Pp_UD,
		Pp_DU,
		Pp_Sxm1,
		Pp_L2col,
	};

	int  n, fn, arg, off;

	n = GetHfData();

	if (((n & 0xf) == 0) || ((n & 0xf) == 0xf)) {
		off = n >> 3;
		fn = tid[off] & 0xf;
		arg = tid[off] >> 4;
		if (arg == 0) {
			(*func[fn]) (x, y, w, n);
		} else {
			(*func[fn]) (x, y, w, arg);
		}
	} else {
		if (n < P_RC)
			Pp_L1c(x, y, w, n);
		else if (n < P_UC)
			Pp_R1c(x, y, w, n);
		else if (n < P_DC)
			Pp_U1c(x, y, w, n);
		else if (n < P_LC + 0x200)
			Pp_D1c(x, y, w, n);
		else if (n < P_RC + 0x200)
			Pp_L1c(x, y, w, n);
		else if (n < P_UC + 0x200)
			Pp_R1c(x, y, w, n);
		else if (n < P_DC + 0x200)
			Pp_U1c(x, y, w, n);
		else
			Pp_D1c(x, y, w, n);
	}
	lpB = patB;
	lpR = patR;
	lpG = patG;
	lpI = patI;
}
#endif /******/


static void Pass2(void)
{
	int  x, y, w;
 /* int i, j, n;          */
 /* int patb, patr, patg; */

	unCol1 = 0;
	unCol2 = 1;
	yoffset = 1;
	for (x = 0; x < XLMAX; ++x) {
		for (y = 0; y < ymax; ++y) {
			if (GetVbuf(x,y) == VL_PAINTED)
				continue;
		   /* 4*2の上半分4*1パターンを展開 */
			w = 1;
			Pass22(x, y, w);
			patB1 = patB << 4;
			patR1 = patR << 4;
			patG1 = patG << 4;
			patI1 = patI << 4;

		   /* 4*2の下半分4*1パターンを展開 */
			w = 0;
			Pass22(x, y, w);
			patB1 |= patB;
			patR1 |= patR;
			patG1 |= patG;
			patI1 |= patI;

			ConvPat();
			PutPat2(x, y);      /* VRAMにデータ(4*2)を書き込む */
		}
	}
}



/*---------------------------------------------------------------------------*/
static void LoadPass2(void)
{
	int  i, j, n, p;
	struct HUF tmpHufBuf;

	memset(hufBuf, 0, sizeof(hufBuf));
	p = 0;
	n = GetBits(p2tbBit);
	do {
		hufBuf[p].num = n;
		hufBuf[p].codenum = GetBits(4);
		hufBuf[p].code = GetBits(hufBuf[p].codenum);
		++p;
	} while ((n = GetBits(p2tbBit)) != 0);

	cthbuf = p;

	for (i = 0; i < cthbuf - 1; ++i) {
		for (j = i + 1; j < cthbuf; ++j) {
			if (hufBuf[i].codenum <= hufBuf[j].codenum)
				continue;
			tmpHufBuf = hufBuf[i];
			hufBuf[i] = hufBuf[j];
			hufBuf[j] = tmpHufBuf;
		}
	}
}


static void LoadPass1(void)
{
	UCHAR bt, btbuf[YLMAX][XLMAX / 8];
	USHORT i, j, k;
	int  x, y;
	struct HUF tmpHufBuf;

	tbb = GetBits(3) + 1;

 	/* 符号の入力 */
	for (i = 0; i < 0x10; ++i) {/*  符号長の入力 */
		hufBuf[i].codenum = GetBits(4);
	}
	for (i = 0; i < 0x10; ++i) {/* 符号の入力 */
		hufBuf[i].num = i;
		hufBuf[i].code = GetBits(hufBuf[i].codenum);
	}
	cthbuf = 0x10;

	for (i = 0; i < cthbuf - 1; ++i) {
		for (j = i + 1; j < cthbuf; ++j) {
			if (hufBuf[i].codenum <= hufBuf[j].codenum)
				continue;
			tmpHufBuf = hufBuf[i];
			hufBuf[i] = hufBuf[j];
			hufBuf[j] = tmpHufBuf;
		}
	}

	/* ビットマップデータの入力 */
	for (y = 0; y < ymax; ++y) {
		for (x = 0; x < XLMAX / 8; ++x) {
			btbuf[y][x] = GetHfData() << 4;
			btbuf[y][x] |= GetHfData();
		}
	}

	/* テーブルにＥＯＲをかけて、もとにもどす */
	for (y = ymax - 2; y >= 0; --y) {
		for (x = 0; x < XLMAX / 8; ++x) {
			btbuf[y][x] = btbuf[y][x] ^ btbuf[y + 1][x];
		}
	}

	/* ビットマップテーブルの作成 */
	for (y = 0; y < ymax; ++y) {
		for (x = 0; x < XLMAX; x += 8) {
			bt = 0x80;
			for (k = 0; k < 8; ++k) {
				SetVbuf(x + k, y, ((bt & btbuf[y][x / 8]) ? VL_PAINT : 0));
				bt >>= 1;
			}
		}
	}
}


static void VHP_GetPal(UCHAR *rgb, UCHAR analog_flg)
{
	static UCHAR pa16[] = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x0c,
		0x0c, 0x00, 0x00,
		0x0c, 0x00, 0x0c,
		0x00, 0x0c, 0x00,
		0x00, 0x0c, 0x0c,
		0x0c, 0x0c, 0x00,
		0x08, 0x08, 0x08,
		0x0c, 0x0c, 0x0c,
		0x00, 0x00, 0x0f,
		0x0f, 0x00, 0x00,
		0x0f, 0x00, 0x0f,
		0x00, 0x0f, 0x00,
		0x00, 0x0f, 0x0f,
		0x0f, 0x0f, 0x00,
		0x0f, 0x0f, 0x0f
	};
	static UCHAR pal8[] = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x0f,
		0x0f, 0x00, 0x00,
		0x0f, 0x00, 0x0f,
		0x00, 0x0f, 0x00,
		0x00, 0x0f, 0x0f,
		0x0f, 0x0f, 0x00,
		0x0f, 0x0f, 0x0f,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x0f,
		0x0f, 0x00, 0x00,
		0x0f, 0x00, 0x0f,
		0x00, 0x0f, 0x00,
		0x00, 0x0f, 0x0f,
		0x0f, 0x0f, 0x00,
		0x0f, 0x0f, 0x0f
	};
	int  i;

	if (analog_flg) {
		for (i = 0; i < 16; ++i) {
			rgb[i*3 + 0] = GetBits(8);
			rgb[i*3 + 1] = GetBits(8);
			rgb[i*3 + 2] = GetBits(8);
		}
	} else if (col16Flg) {
		memcpy(rgb, pa16, 16*3);
	} else {
		memcpy(rgb, pal8, 16*3);
	}
	for (i = 0; i < 16*3; i++)
		rgb[i] <<= 4;
}


static void VHP_Load(char *name, UCHAR *rgb, int *typ)
{
	int  type;
	UCHAR analog;

	FL_Open(name);
	InitGetBit();

	*typ = type = GetBits(8);          /* get 8 bits */
	ymax = (type & 0x01) ? 200 : 100;
	col16Flg = (type & 0x02) ? 1  : 0;
	p2tbBit = (type & 0x02) ? 10 : 9;
	analog  = (type & 0x04) ? 1  : 0;

	memset(vbuf,  0, sizeof(vbuf));
	memset(patTblB, 0, sizeof(patTblB));
	memset(patTblR, 0, sizeof(patTblR));
	memset(patTblG, 0, sizeof(patTblG));
	memset(patTblI, 0, sizeof(patTblI));
	patTblPtr = 1;

	VHP_GetPal(rgb, analog);
	LoadPass1();
	Pass1();
	LoadPass2();
	Pass2();

	FL_Close();
}


int  GLoad(TOFMT4P *t, char *name)
{
	static int typ;

	v_b = t->v[0];
	v_r = t->v[1];
	v_g = t->v[2];
	v_i = t->v[3];
	/*printf("%08lx %08lx %08lx %08lx\n", v_b, v_r, v_g, v_i);*/

	VHP_Load(name, t->rgb, &typ);

	t->xsz = 640;
	if (typ & 0x01) {
		t->ysz = 400;
	} else {
		t->ysz = 200;
		t->xasp = 1;
		t->yasp = 2;
	}
	return 0;
}
