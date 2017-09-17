/*=============================================================================
 *  VHP Loader Version 1.21T (CG presser)
 *  	640*200,640*400 （8/16(analog) colors）
 *  	    	  PC-9801 , PC-286 or PC88VA
 *
 *  	1989.12 - 1990.03    by マシーンＭ (Ver.1.10 for PC9801)
 *  	1991.08 - 1992.02    modified by M.Kitamura(てんか☆ふん)
 *- 履歴 ----------------------------------------------------------------------
 * v1.00   640x200,640x400(8 colors)
 * v1.10   16(analog) colors 追加
 * v1.20T  1991/09/01 .RGBﾌｧｲﾙの出力(-r). 入力ﾊﾞｯﾌｧの拡大. -v(明度指定).
 *  	    	      88VA対応. Turbo-C(LSI-C)でｺﾝﾊﾟｲﾙ. etc
 * v1.21T  1992/02/06  last modified
 *- コンパイル方法 ------------------------------------------------------------
 * 98:lcc vhl.c vhl_2.c vhl_g98.c vhl_rgbf.c sub.c rpalsrch.c rpalset.c
 *    -lnoexpand.obj -k/stack:15000
 * va:lcc -DPC88VA vhl.c vhl_2.c vhl_gva.c vhl_rgbf.c sub.c va.c va2.c
 *    -lnoexpand.obj -k/stack:15000
 *=============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <process.h>
#include <io.h>
#include <conio.h>
#include <dos.h>
#include "tenkafun.h"
#include "sub.h"
#include "vhp.h"
#define  EXT
#include "vhl.h"

#define puts_exit(s) (puts(s),exit(1))

/* Loader の ビットマップデータの識別子 */
#define VL_PAINT   1
#define VL_PAINTED 2

/*------------------------ 変数宣言 ------------------------------*/

/* パターンテーブル */
static byte Pattb[MAXTBSIZE],
    	    Pattr[MAXTBSIZE],
    	    Pattg[MAXTBSIZE],
    	    Patti[MAXTBSIZE];
static word Pattptr;	    	   /* パターンテーブルの位置　*/

static int  P2tbbit;

/* ハフマン符号化 */
struct HUF {
  #ifndef TST8BIT
    word num;
    word code;
    word codenum;     /* 0-15 */
  #else
    word num;
    word code;
    byte codenum;/* 0-15 */
  #endif
};
static struct HUF hbuf[MAXHUF];

static word Cthbuf = 0;

/* データ入力 */
static int  Mask;
static word Tbb;
static FILE *Fpr;

#ifdef EX_IN_BUF
/* 拡張入力ﾊﾞｯﾌｧ  -- ﾊｰﾄﾞﾃﾞｨｽｸで使用するときはかえって邪魔^^; */
#define B_BUF_SIZ (0xff00)
static BYTE_FP B_buf;
static BYTE_FP B_bufp;
static word B_cnt;
static word B_siz = B_BUF_SIZ;
static long B_fsiz;
static int  B_no;
static byte B_flg = 1;
static byte Closed_flg = OFF;
static byte *Loadname;
#endif


/*---------------------------------------------------------------------------*/
void
error(int err)
{
    byte *p;

    p = NULL;
    switch (err) {
    case ER_AB:
    	p = "Abnormal data.";
    	break;
    case ER_OPEN:
    	p = "Can't open the file.";
    	break;
    case ER_BUG:
    	p = "Programer's error!";
    }
    dspSwitch(ON,OFF);
    if (p)
    	puts(p);
    exit(1);
}


/*---------------------------------------------------------------------------*/
#ifndef EX_IN_BUF
#define b_buf_init()
#define b_getc()       fgetc(Fpr)
#define b_close()      fclose(Fpr)

static void
b_open(byte *fname)
{
    if ((Fpr = fopen(fname, "rb")) == NULL) {
    	puts(fname);
    	error(ER_OPEN);
    }
}
#else

static void
b_buf_init(void)
{
    while (B_flg) {
    	B_buf = farmalloc(B_siz);
    	if (B_buf != NULL)
    	    break;
    	if (B_siz >= 0x1f00) {
    	    B_siz -= 0x1000;
    	    continue;
    	}
    	B_flg = OFF;
    	break;
    }
}

static void
b_buf_read(void)
{
    word cnt;

    B_bufp = B_buf;
    if (B_fsiz > (long) B_siz) {
    	B_cnt = B_siz;
    } else {
    	B_cnt = B_fsiz;
    	Closed_flg = ON;
    }
    if (_dos_read(B_no, B_buf, B_cnt, &cnt)) {
    	puts(Loadname);
    	puts("read error");
    	error(0);
    }
    if (Closed_flg != OFF)
    	fclose(Fpr);

  #if 0
    if (B_cnt != cnt) {
    	puts("b_buf_read");
    	error(ER_BUG);
    }
  #endif
}

static void
b_open(byte *loadname)
{
    Loadname = loadname;
    if ((Fpr = fopen(Loadname, "rb")) == NULL) {
    	puts(Loadname);
    	error(ER_OPEN);
    }
    Closed_flg = OFF;
    if (B_flg) {
    	B_no = fileno(Fpr);
    	B_fsiz = filelength(B_no);
    	if (B_fsiz < 0) {
    	    B_flg = OFF;
    	    return;
    	}
    	b_buf_read();
    }
}


static int
b_getc(void)
{
    if (B_flg == OFF)
    	return getc(Fpr);

    for (;;) {
    	if (B_cnt-- > 0)
    	    return *(B_bufp++);
    	B_fsiz -= (long) B_siz;
    	if (Closed_flg == OFF && B_fsiz > 0L) {
    	    b_buf_read();
    	    continue;
    	} else
    	    return EOF;
    }
}

static void
b_close(void)
{
    if (Closed_flg == OFF)
    	fclose(Fpr);
}
#endif

/* データ入力ルーチン */
static void
initgetbit(void)
{
    Mask = 0;
}

static word
getbit(void)
{
    static int buf;

    if ((Mask >>= 1) == 0) {
    	if ((buf = b_getc()) < 0) {
    	    puts("EOF");
    	    error(ER_AB);
    	}
    	Mask = 0x80;
    }
    return (buf & Mask) ? 1 : 0;
}

word
getbits(int n)
{
    word x;

    x = 0;
    while (n-- > 0)
    	x = (x << 1) + getbit();
    return x;
}


word
getdata(void)
    /*
     *ファイルから入力したハフマン符号を変換する
     */
{
    int  n, p, code;

    n = p = code = 0;
    while (n++ <= 16) {
    	code = (code << 1) | getbit();
    	while (hbuf[p].codenum < n)
    	    ++p;
    	while (hbuf[p].codenum == n) {
    	    if (hbuf[p].code == code)
    	    	return hbuf[p].num;
    	    ++p;
    	}
    }
    puts("unknown huffman bits");
    error(ER_AB);
    return 0;
}


/*---------------------------------------------------------------------------*/
#ifndef TST8BIT     /*ふつうにｺﾝﾊﾟｲﾙしたとき */
  #ifndef getVb
    #define getVb(x,y)	  (Vbuf[y][x])
  #endif
  #define setVb(x,y,d)	(Vbuf[y][x] = (byte)(d))

#else  /* Vbuf サイズを小さくするばあい (8bit機に対応するための下準備？)*/

int
getVb(int x, int y)
{
    int  a;

    a = Vbuf[y][x/4];
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

void
setVb(int x, int y, int data)
{
    word a;

    a = Vbuf[y][x/4];
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
    Vbuf[y][x/4] = a;
    return;
}

#endif


static int
getflg(int x, int y)
{
    switch (Offy) {
    case 1:
    	return getVb(x,y);
    case 2:
    	if (y + 1 >= Ly)
    	    return getVb(x,y);
    	else
    	    return getVb(x,y) & getVb(x,y+1);
    }
    error(ER_BUG);
    return 0;
}

static void
setflg(int x, int y, int data)
{
    setVb(x,y,data);
}


static void
putpat(int x, int y)
{
    setflg(x, y, VL_PAINTED);
    if ((Offy == 2) && (y+1 < Ly))
    	setflg(x, y+1, VL_PAINTED);
    putpat2(x, y);
}


static void
tbsetpat(byte pb, byte pr, byte pg, byte pi)
{
    if (Pattptr >= MAXTBSIZE)
    	return;

    Pattb[Pattptr] = pb;
    Pattr[Pattptr] = pr;
    Pattg[Pattptr] = pg;
    Patti[Pattptr] = pi;
    ++Pattptr;
}

static void
tbgetpat(int n)
{
    if (n >= MAXTBSIZE)
    	error(ER_AB);

    Patb = Pattb[n];
    Patr = Pattr[n];
    Patg = Pattg[n];
    Pati = Patti[n];
}



static void
pass2(void)
{
    int  x, y, w;
 /* int i, j, n;    	  */
 /* int patb, patr, patg; */

     Ucol1 = 0;
     Ucol2 = 1;
    Offy = 1;
    for (x = 0; x < LMAXX; ++x) {
    	for (y = 0; y < Ly; ++y) {
    	    if (getVb(x,y) == VL_PAINTED)
    	    	continue;
    	   /* 4*2の上半分4*1パターンを展開 */
    	    w = 1;
    	    pass22(x, y, w);
    	    Patb1 = Patb << 4;
    	    Patr1 = Patr << 4;
    	    Patg1 = Patg << 4;
    	    Pati1 = Pati << 4;

    	   /* 4*2の下半分4*1パターンを展開 */
    	    w = 0;
    	    pass22(x, y, w);
    	    Patb1 |= Patb;
    	    Patr1 |= Patr;
    	    Patg1 |= Patg;
    	    Pati1 |= Pati;

    	    convpat();
    	    putpat2(x, y);  	/* VRAMにデータ(4*2)を書き込む */
    	}
    }
}


/*---------------------------------------------------------------------------*/
static int
drawright(int x, int y)
{
    ++x;
    if (x == LMAXX)
    	return LMAXX - 1;
    while (getflg(x, y) == VL_PAINT) {	 /* --->(rx) */
    	putpat(x, y);
    	++x;
    	if (x == LMAXX)
    	    return LMAXX - 1;
    }
    return --x;
}

static int
drawleft(int x, int y)
{
    while (getflg(x, y) == VL_PAINT) {	 /* (lx)<--- */
    	putpat(x, y);
    	--x;
    	if (x < 0)
    	    return 0;
    }
    return ++x;
}


static void
paint(int x, int y)
{
    int  rx, lx;

    if (y < 0 || y >= Ly)
    	return;

    if (getflg(x, y) != VL_PAINT)
    	return;

    lx = drawleft(x, y);
    rx = drawright(x, y);

    while (lx <= rx) {
    	paint(lx, y - Offy);
    	paint(lx, y + Offy);
    	++lx;
    }
}


static int
issame42(int x, int y)
    /*
     *	周りにペイントパターンがなければ、真を返す
     *	（独立した4*2のパターンかどうかを調べる）
     */
{
    int  flg;

    flg = 1;
    if (x - 1 >= 0)
    	if (getflg(x - 1, y))
    	    flg = 0;
    if (x + 1 < LMAXX)
    	if (getflg(x + 1, y))
    	    flg = 0;
    if (y - 1 >= 0)
    	if (getflg(x, y - 1))
    	    flg = 0;
    if (y + 1 < Ly)
    	if (getflg(x, y + 1))
    	    flg = 0;

    return flg;
}


/*
 * パターンデータの入力
 * A. 0xxx xxxx     	    	    	    - V_TYPE42 found pattern
 * B. 10bb bbbb bbrr rrrr rrgg gggg gg(i..) - V_TYPE42 not found pattern
 * C. 11    	    	    	    	    - V_TYPE44
 * D.	0x xxxx xx  	    	    	    -	found pattern
 * E.	1b bbbb bbbr rrrr rrrg gggg ggg(i..)-	not found pattern
 * F. xxxx xxx	    	    	    	    - V_SAME42(識別子がなくても判別可)
 *
 *     b - blue  パターン 8bit(4*2)
 *     r - red	 パターン 8bit(4*2)
 *     g - green パターン 8bit(4*2)
 *     x - パターンテーブルの位置 1,2,3,4,5,6 or 7 bit
 *  （１６色の時には gggg gggg のあとに iiii iiii が付く）
 */
static void
pass1(void)
{
    int  x, y, pn;

    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (getVb(x,y) != VL_PAINT)
    	    	continue;
    	    Offy = 1;	    	/* dummy */

    	    if (issame42(x, y))
    	    	continue;

    	    if (getbit() == 0) {
    	    	pn = getbits(Tbb);
    	    	tbgetpat(pn);
    	    	Patb1 = Patb;
    	    	Patr1 = Patr;
    	    	Patg1 = Patg;
    	    	Pati1 = Pati;
    	    	convpat();
    	    	Offy = 1;
    	    	paint(x, y);
    	    } else {
    	    	if (getbit() == 0) {
    	    	    Patb1 = getbits(8);
    	    	    Patr1 = getbits(8);
    	    	    Patg1 = getbits(8);
    	    	    if (Color16)
    	    	    	Pati1 = getbits(8);
    	    	    tbsetpat(Patb1, Patr1, Patg1, Pati1);
    	    	    convpat();
    	    	    Offy = 1;
    	    	    paint(x, y);
    	    	} else {
    	    	    if (getbit() == 0) {
    	    	    	pn = getbits(Tbb);
    	    	    	tbgetpat(pn);
    	    	    	Patb1 = Patb;
    	    	    	Patr1 = Patr;
    	    	    	Patg1 = Patg;
    	    	    	Pati1 = Pati;
    	    	    } else {
    	    	    	Patb1 = getbits(8);
    	    	    	Patr1 = getbits(8);
    	    	    	Patg1 = getbits(8);
    	    	    	if (Color16)
    	    	    	    Pati1 = getbits(8);
    	    	    	tbsetpat(Patb1, Patr1, Patg1, Pati1);
    	    	    }
    	    	    if (getbit() == 0) {
    	    	    	pn = getbits(Tbb);
    	    	    	tbgetpat(pn);
    	    	    	Patb2 = Patb;
    	    	    	Patr2 = Patr;
    	    	    	Patg2 = Patg;
    	    	    	Pati2 = Pati;
    	    	    } else {
    	    	    	Patb2 = getbits(8);
    	    	    	Patr2 = getbits(8);
    	    	    	Patg2 = getbits(8);
    	    	    	if (Color16)
    	    	    	    Pati2 = getbits(8);
    	    	    	tbsetpat(Patb2, Patr2, Patg2, Pati2);
    	    	    }
    	    	    convpat();
    	    	    Offy = 2;
    	    	    paint(x, y);
    	    	}
    	    }
    	}
    }

    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (getVb(x,y) != VL_PAINT)
    	    	continue;

    	    pn = getbits(Tbb);
    	    tbgetpat(pn);
    	    Patb1 = Patb;
    	    Patr1 = Patr;
    	    Patg1 = Patg;
    	    if (Color16)
    	    	Pati1 = Pati;
    	    convpat();
    	    Offy = 1;	    	/* dummy */
    	    putpat(x, y);
    	}
    }
}


/*---------------------------------------------------------------------------*/
static void
loadpass2(void)
{
    int  i, j, n, p;
    struct HUF tmphbuf;

    memset(hbuf, 0, sizeof(hbuf));
    p = 0;
    n = getbits(P2tbbit);
    do {
    	hbuf[p].num = n;
    	hbuf[p].codenum = getbits(4);
    	hbuf[p].code = getbits(hbuf[p].codenum);
    	++p;
    } while ((n = getbits(P2tbbit)) != 0);

    Cthbuf = p;

    for (i = 0; i < Cthbuf - 1; ++i) {
    	for (j = i + 1; j < Cthbuf; ++j) {
    	    if (hbuf[i].codenum <= hbuf[j].codenum)
    	    	continue;
    	    tmphbuf = hbuf[i];
    	    hbuf[i] = hbuf[j];
    	    hbuf[j] = tmphbuf;
    	}
    }
}


static void
loadpass1(void)
{
    byte bt, btbuf[LMAXY][LMAXX / 8];
    word i, j, k;
    int  x, y;
    struct HUF tmphbuf;

    Tbb = getbits(3) + 1;

/*
 *　符号の入力
 */
    for (i = 0; i < 0x10; ++i) {/*  符号長の入力 */
    	hbuf[i].codenum = getbits(4);
    }
    for (i = 0; i < 0x10; ++i) {/* 符号の入力 */
    	hbuf[i].num = i;
    	hbuf[i].code = getbits(hbuf[i].codenum);
    }
    Cthbuf = 0x10;

    for (i = 0; i < Cthbuf - 1; ++i) {
    	for (j = i + 1; j < Cthbuf; ++j) {
    	    if (hbuf[i].codenum <= hbuf[j].codenum)
    	    	continue;
    	    tmphbuf = hbuf[i];
    	    hbuf[i] = hbuf[j];
    	    hbuf[j] = tmphbuf;
    	}
    }
/*
 *　ビットマップデータの入力
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX / 8; ++x) {
    	    btbuf[y][x] = getdata() << 4;
    	    btbuf[y][x] |= getdata();
    	}
    }
/*
 *　テーブルにＥＯＲをかけて、もとにもどす
 */
    for (y = Ly - 2; y >= 0; --y) {
    	for (x = 0; x < LMAXX / 8; ++x) {
    	    btbuf[y][x] = btbuf[y][x] ^ btbuf[y + 1][x];
    	}
    }
/*
 *  ビットマップテーブルの作成
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; x += 8) {
    	    bt = 0x80;
    	    for (k = 0; k < 8; ++k) {
    	    	setVb(x + k, y, ((bt & btbuf[y][x / 8]) ? VL_PAINT : 0));
    	    	bt >>= 1;
    	    }
    	}
    }
}


static byte *
getPalette(byte analog_flg)
{
    static byte pa16[] = {
    	0x00, 0x00, 0x00,
    	0x00, 0x00, 0x0c,
    	0x00, 0x0c, 0x00,
    	0x00, 0x0c, 0x0c,
    	0x0c, 0x00, 0x00,
    	0x0c, 0x00, 0x0c,
    	0x0c, 0x0c, 0x00,
    	0x08, 0x08, 0x08,
    	0x0c, 0x0c, 0x0c,
    	0x00, 0x00, 0x0f,
    	0x00, 0x0f, 0x00,
    	0x00, 0x0f, 0x0f,
    	0x0f, 0x00, 0x00,
    	0x0f, 0x00, 0x0f,
    	0x0f, 0x0f, 0x00,
    	0x0f, 0x0f, 0x0f
    };
    static byte pal8[] = {
    	0x00, 0x00, 0x00,
    	0x00, 0x00, 0x0f,
    	0x00, 0x0f, 0x00,
    	0x00, 0x0f, 0x0f,
    	0x0f, 0x00, 0x00,
    	0x0f, 0x00, 0x0f,
    	0x0f, 0x0f, 0x00,
    	0x0f, 0x0f, 0x0f,
    	0x00, 0x00, 0x00,
    	0x00, 0x00, 0x0f,
    	0x00, 0x0f, 0x00,
    	0x00, 0x0f, 0x0f,
    	0x0f, 0x00, 0x00,
    	0x0f, 0x00, 0x0f,
    	0x0f, 0x0f, 0x00,
    	0x0f, 0x0f, 0x0f
    };
    static byte acol[48];
    int  i;

    if (analog_flg)
    	for (i = 0; i < 16; ++i) {
    	    acol[i*3 + 1] = getbits(8);
    	    acol[i*3 + 0] = getbits(8);
    	    acol[i*3 + 2] = getbits(8);
    	}
    else if (Color16)
    	for (i = 0; i < 48; ++i)
    	    acol[i] = pa16[i];
    else
    	for (i = 0; i < 48; ++i)
    	    acol[i] = pal8[i];

    return acol;
}


/* 展開 */
void
vhload(byte *load_name,word toon,byte rgb_flg)
{
    int  type;
    byte *grb;
    byte analog;

    b_open(load_name);
    initgetbit();

    type = getbits(8);	    	/* get 8 bits */
    Ly = (type & 0x01) ? 200 : 100;
    Color16 = (type & 0x02) ? ON : OFF;
    P2tbbit = (type & 0x02) ? 10 : 9;
    analog  = (type & 0x04) ? ON : OFF;

    memset(Vbuf,  0, sizeof(Vbuf));
    memset(Pattb, 0, sizeof(Pattb));
    memset(Pattr, 0, sizeof(Pattr));
    memset(Pattg, 0, sizeof(Pattg));
    memset(Patti, 0, sizeof(Patti));
    Pattptr = 1;

    grb = getPalette(analog);
    if (rgb_flg) {
    	putPalFile(rgb_flg,grb,load_name);
    	b_close();
    	return;
    }
    GLinit(type & 0x01, Color16, toon, grb);

 #ifdef PC88VA
    acs_vram();
  #if defined(KAIHATU)
    if (vaVramModeFlg)
    	outp(0x510, 0);
  #endif
 #endif

    loadpass1();
    pass1();
    loadpass2();
    pass2();

 #ifdef PC88VA
    acs_text();
 #endif

    b_close();
}


/*---------------------------------------------------------------------------*/
static void
dispusage(void)
{
 #ifndef PC88VA
    puts("VHP Loader Ver.1.21T for PC9801 by ﾏｼｰﾝM(Ver.1.10)");
 #else
    puts("VHP Loader Ver.1.21T for PC88VA by ﾏｼｰﾝM(Ver.1.10 for PC9801)");
 #endif
    puts("                       modified by ﾃﾝｶ*ﾌﾝ");
    puts("Usage: vhload [-{<sw>}] {<pathname>[.VHP] [-{<sw>}] }");
    puts("<sw>: ?, k, r|a, v[N]");
    puts("  ?    ヘルプ表示");
    puts("  k    １画面展開ごとにキー入力待ち");
    puts("  r    .rgb ファイル作成");
    puts("  a    .alg ファイル作成");
    puts("  v[N] 明るさ. N:0から100. N省略時 N=50");
 #if defined(PC88VA)&&defined(KAIHATU)
    puts("  h    ﾏﾙﾁﾌﾟﾚｰﾝﾓｰﾄﾞで表示");
    puts("  i    ｼﾝｸﾞﾙﾌﾟﾚｰﾝﾓｰﾄﾞで表示");
 #endif
    exit(0);
}


void
ctrl_c(void)
{
    dspSwitch(ON,OFF);
    puts("Abort");
    exit(0);
}

static void
keywait(void)
{
    if (getch() == 0x1b) {
    	dspSwitch(ON,ON);
    	exit(0);
    }
}

/*----------------------------*/
int
main(int argc, byte *argv[])
{
    #define FNAM_MAX 130
    byte fname[FNAM_MAX+40],load_name[FNAM_MAX+1];
    int  i;
    byte *p;
    static byte sw_a = 0;
    static byte sw_k = OFF;
    static int	toon = 100;

 #ifdef PC88VA
    vaInit();
 #endif
    if (argc < 2)
    	dispusage();

    for (i = 1; i < argc; ++i) {
    	p = argv[i];
    	if (*p != '-')
    	    continue;
    	while (*(++p) != '\0') {
    	    switch (toupper(*p)) {
    	    case '?':
    	    	dispusage();
    	    	break;
    	    case 'K':
    	    	sw_k = ON;
    	    	break;
    	    case 'A':
    	    	sw_a = 2;
    	    	break;
    	    case 'R':
    	    	sw_a = 1;
    	    	break;
    	    case 'V':
    	    	++p;
    	    	if (isdigit(*p)) {
    	    	    toon = (int) strtoul(p, &p, 10);
    	    	    if (toon > 100)
    	    	    	toon = 100;
    	    	} else
    	    	    toon = 50;
    	    	--p;
    	    	break;
 #if defined(PC88VA) && defined(KAIHATU)
    	    case 'H':
    	    	vaVramModeFlg = 0;/* ﾏﾙﾁﾌﾟﾚｰﾝ */
    	    	break;
    	    case 'I':
    	    	vaVramModeFlg = 4;/* ｼﾝｸﾞﾙﾌﾟﾚｰﾝ */
    	    	    break;
 #endif
    	    default:
    	    	puts_exit("Unknown switches.");
    	    }
    	}
    }

    if (signal(SIGINT, ctrl_c) == SIG_ERR)
    	puts_exit("Can't set SIGINT.");
    b_buf_init();

    for (i = 1; i < argc; ++i) {
    	if (argv[i][0] == '-')
    	    continue;
    	if (strlen(argv[i]) > FNAM_MAX)
    	    puts_exit("Pathname too long.");
    	strcpy(fname,argv[i]);
    	/* jstrrplchr(fname,'/','\\'); */
    	if (set_ext(fname,"VHP"))
    	    puts_exit("Bad, Pathname.");

    	if (getWildFname(load_name, fname) != 0) {
    	    strcat(fname," : Can't open the file");
    	    puts_exit(fname);
    	}
    	vhload(load_name,toon,sw_a);
    	if (sw_k)
    	    keywait();
    	while (getWildFname(load_name,NULL) == 0) {
    	    vhload(load_name,toon,sw_a);
    	    if (sw_k)
    	    	keywait();
    	}
    }
    dspSwitch(ON,ON);
    return 0;
}
