/*=============================================================================
 *  VHP Saver  Version 1.21T (CG presser)
 *  	640*200,640*400 （8/16(analog) colors）
 *  	    	  PC-9801 , PC-286 or PC88VA
 *
 *  	1989.12 - 1990.04    by マシーンＭ (Ver.1.11 for PC9801)
 *  	1991.08 - 1992.02    modified by M.Kitamura(てんか☆ふん)
 *- 履歴 ----------------------------------------------------------------------
 * v1.00   640x200,640x400(8 colors)
 * v1.10   16(analog) colors 追加
 * v1.11   paint()再帰呼びだしでの stack overflow が起こらないようにした
 *  	   buildhuftree()の ちょっとした高速化
 * v1.20T  1991/09/01 .rgbﾊﾟﾚｯﾄﾌｧｲﾙに対応.88VA対応.Turbo-C,LSI-Cでのｺﾝﾊﾟｲﾙ.
 * v1.21T  1992/02/06  last modified
 *- コンパイル方法 ------------------------------------------------------------
 * 98: lcc vhs.c vhs_2.c rpalsrch.c rpalget.c -lnoexpand.obj -k/stack:15000
 * va: lcc -DPC88VA vhs.c vhs_2.c va2.c -lnoexpand.obj -k/stack:15000
 *=============================================================================
 */
#include <stdio.h>
#include <string.h>
#include <dos.h>
/*#include <ctype.h>*/
#include <conio.h>
#include <process.h>
#include "tenkafun.h"
#include "vhp.h"
#ifndef PC88VA
#include "rpal.h"
#endif
#define  EXT
#include "vhs.h"


/* Saver での 現在のpass */
#define PASS1	   1
#define SAVEPASS1  2
#define PASS2	   3
#define SAVEPASS2  4

/* Saver の ビットマップデータの識別子 */
#define V_TYPE42   1
#define V_TYPE44   2
#define V_SAME42   4
#define V_PAINT    (V_TYPE42|V_TYPE44|V_SAME42)
#define V_NON	   8
#define V_PAINTED 16
#define V_TBMASK  32

#define vof(x,y) (LMAXX*(y)+(x))

/*---------------------- 変数宣言 ----------------------*/

/* パターンテーブル */
static byte Pattb[MAXTBSIZE],
    	    Pattr[MAXTBSIZE],
    	    Pattg[MAXTBSIZE],
    	    Patti[MAXTBSIZE];
static word Pattptr = 1;      /* ﾊﾟﾀｰﾝﾃｰﾌﾞﾙの位置（Patt?[0]は黒に固定）*/
static word Tbsize, Tbmask, Tbb;

/* ビットマップテーブル */
static byte Vbuf[LMAXY][LMAXX];

/* RGBI データ */
static byte Npatb, Npatr, Npatg, Npati;
static byte Lpatb, Lpatr, Lpatg, Lpati;
static byte Rpatb, Rpatr, Rpatg, Rpati;
static byte Patb, Patr, Patg, Pati;

/* 最後に使ったパターンの2色を退避(pass2) */
static int  Lcol1, Lcol2;
static int  Ucol1, Ucol2;

/* アナログ */
static int  Analog;

/* ペイント */
static int  Ctnest = 0;
static int  Px, Py;
static int  Offx, Offy;
static int  Ptype, Pvector;
static word P2mask;

/* pass2 各識別子の数 */
static word Ctp2[MAXP2TB];
static word P2tbbit;

/* ハフマン符号化用 */
struct HUF {
    word num, count, treenum, code, codenum;
};
static struct HUF Hbuf[MAXHUF];

static word Cthbuf = 0;

/* データ出力 */
static FILE *Fpw;

/* セーブファイル名 */
static byte Savename[80 + 1];

/* .rgb|.algファイル名 */
static byte Algname[80 + 1];

/* 現在のpass */
static int  Pass;


/*-------------------------------------------------------------------------*/
void
extcat(byte oldfname[], byte *ext, byte newfname[])
    	/* 拡張子を変更する */
{
    byte ch;
    int  i, fnp;

    fnp = 0;
    for (i = 0; oldfname[i] != '\0'; ++i) {
    	ch = oldfname[i];
    	if (ch == '/' || ch == '\\') {
    	    fnp = i;
    	    ch = '\\';
    	} else if (ch >= 'a' && ch <= 'z') {
    	    ch -= 0x20;
    	}
    	newfname[i] = ch;
    }
    newfname[i] = '\0';
    for (i = fnp; newfname[i] != '\0' && newfname[i] != '.'; ++i)
    	;
    newfname[i] = '\0';
    strcat(newfname, ext);
}


/*---------------------------------------------------------------------------*/
void
error(int err)
{
    byte *p;

    p = NULL;
    switch (err) {
    case ER_BUG:
    	p = "Sorry.	Bugs exist in this program.";
    	break;
    case ER_WRITE:
    	p = "Can't write.";
    	break;
    case ER_NTLONG:
    	p = "Pathname too long.";
    	break;
    case ER_SW:
    	p = "Unknown switches.";
    	break;
    }
    if (p)
    	puts(p);
    exit(1);
}


/*--------------------
 * データ出力ルーチン
 *--------------------
 */
void
putbit(int bit)
{
    static word buffer = 0, mask = 0x80;

    if (bit)
    	buffer |= mask;
    if ((mask >>= 1) == 0) {
    	if (putc(buffer, Fpw) == EOF)
    	    error(ER_WRITE);
    	buffer = 0;
    	mask = 0x80;
    }
}

void
putbits(word mask, word x)
{
    do {
    	putbit(x & mask);
    } while (mask >>= 1);
}

void
flushbit(void)
{
    int  i;

    for (i = 0; i < 7; i++)
    	putbit(0);
}


int
tbcmppat(byte pb, byte pr, byte pg, byte pi)
{
    int  i;

    for (i = 0; i < Tbsize; ++i) {
    	if (pb == Pattb[i] && pr == Pattr[i] && pg == Pattg[i] &&
    	    (!Color16 || pi == Patti[i]))
    	    return i;
    }
    return -1;
}


void
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


int
drawrbmt(int x, int y)
{
    ++x;
    if (x == LMAXX)
    	return LMAXX - 1;
    while (Vbuf[y][x] & V_PAINT) {  	/* --->(rx) */
    	Vbuf[y][x] |= P2mask;
    	++x;
    	if (x == LMAXX)
    	    return LMAXX - 1;
    }

    return --x;
}

int
drawlbmt(int x, int y)
{
    while (Vbuf[y][x] & V_PAINT) {  	/* (lx)<--- */
    	Vbuf[y][x] |= P2mask;
    	--x;
    	if (x < 0)
    	    return 0;
    }

    return ++x;
}

void
paintbmt(void)
{
    byte rx, lx;

    if (Py < 0 || Py >= Ly)
    	return;

    if (!(Vbuf[Py][Px] & V_PAINT))
    	return;
    if (Vbuf[Py][Px] & P2mask)
    	return;
    lx = (byte) drawlbmt(Px, Py);
    rx = (byte) drawrbmt(Px, Py);

    while (lx <= rx) {
    	Px = lx;
    	--Py;
    	paintbmt(); 	    	/* paint upper line */

    	Px = lx;
    	Py += 2;
    	paintbmt(); 	    	/* paint lower line */

    	--Py;
    	++lx;
    }
}


/*---------------------------------------------------------------------------*/
void
settype(void)
{
    if ((Patb1 == Patb2) && (Patr1 == Patr2) && (Patg1 == Patg2)
    	&& (!Color16 || (Pati1 == Pati2)))
    {
    	Ptype = V_TYPE42;
    	Offx = 1;
    	Offy = 1;
    } else {
    	Ptype = V_TYPE44;
    	Offx = 1;
    	Offy = 2;
    }
}


int
getflg(int x, int y)
{
    switch (Ptype) {
    case V_TYPE42:
    	return Vbuf[y][x];
    case V_TYPE44:
    	return Vbuf[y][x] | Vbuf[y + 1][x];
    }
    return 0;
}

void
setflg(int x, int y, word data)
{
    switch (Ptype) {
    case V_TYPE42:
    	Vbuf[y][x] = (byte) data;
    	break;
    case V_TYPE44:
    	Vbuf[y][x] = (byte) data;
    	Vbuf[y + 1][x] = (byte) data;
    	break;
    }
}

int
cmppat(int x, int y)
{
    if (getflg(x, y) == V_NON)
    	return 0;

    if (_cmppat1(x,y))
    	if (Ptype == V_TYPE42 || _cmppat2(x,y+1))
    	    return 1;
    return 0;
}


/*-----------------------------------
 *  周りにある同じパターンの数を返す
 *-----------------------------------
 */
int
checkr(int x, int y)
{
    int  ctr;

    ctr = 0;
    if (x - Offx >= 0)
    	if (getflg(x - Offx, y) == 0 && cmppat(x - Offx, y))
    	    ++ctr;
    if (x + Offx < LMAXX)
    	if (getflg(x + Offx, y) == 0 && cmppat(x + Offx, y))
    	    ++ctr;
    if (y - Offy >= 0)
    	if (getflg(x, y - Offy) == 0 && cmppat(x, y - Offy))
    	    ++ctr;
    if (y + Offy < Ly)
    	if (getflg(x, y + Offy) == 0 && cmppat(x, y + Offy))
    	    ++ctr;

    return ctr;
}


int
drawright(int x, int y)
{
    ++x;
    if (x == LMAXX)
    	return LMAXX - 1;
    while (cmppat(x, y)) {  	/* --->(rx) */
    	setflg(x, y, Ptype);
    	++x;
    	if (x == LMAXX)
    	    return LMAXX - 1;
    }
    setflg(x, y, V_NON);

    return --x;
}

int
drawleft(int x, int y)
{
    while (cmppat(x, y)) {  	/* (lx)<--- */
    	setflg(x, y, Ptype);
    	--x;
    	if (x < 0)
    	    return 0;
    }
    setflg(x, y, V_NON);

    return ++x;
}


void
paint(void)
{
    int  rx, lx;

    if (Py < 0 || Py >= Ly) {
    	if (Offy == 2 && Py == -1)
    	    Vbuf[0][Px] = V_NON;
    	return;
    }
  /* すでにペイントされているか、パターンが異なればリターンする */
  /* Ctnest が MAXNEST をこえればリターンする */
    if (getflg(Px, Py) || !cmppat(Px, Py) || (Ctnest > MAXNEST)) {
    	if (Pvector == UP) {
    	    if (Vbuf[Py + Offy - 1][Px] == 0)
    	    	Vbuf[Py + Offy - 1][Px] = V_NON;
    	} else {
    	    if (Vbuf[Py][Px] == 0)
    	    	Vbuf[Py][Px] = V_NON;
    	}
    	return;
    }
    lx = drawleft(Px, Py);
    rx = drawright(Px, Py);

    while (lx <= rx) {
    	++Ctnest;
    	Pvector = UP;
    	Px = lx;
    	Py -= Offy;
    	paint();

    	Pvector = DOWN;
    	Px = lx;
    	Py += Offy + Offy;
    	paint();
    	--Ctnest;

    	Py -= Offy;
    	++lx;
    }
}


void
pass1(void)
{
    word find;
    int  x, y;

    memset(Vbuf, 0, sizeof(Vbuf));  	/* clear bit map table */

/*
 *  周り全て（上下左右）に、同じパターンが存在する時ペイントする（4*2 or 4*4）
 *　（この部分はなくても、かまわない。圧縮率に多少影響を与える。）
 */
    for (y = 1; y < Ly - 1; ++y) {
    	for (x = 1; x < LMAXX - 1; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*　パターンの読み込み　*/
    	    settype();	    	/*　パターンタイプのセット　*/
    	    if (Ptype == V_TYPE44) {
    	    	if (getflg(x, y))
    	    	    continue;	/* check Vbuf[y+1][x] */
    	    }
    	    if (checkr(x, y) == 4) {
    	    	Px = x;
    	    	Py = y;
    	    	paint();    	/*　ペイント開始　*/
    	    }
    	}
    }

/*
 *　周りに一つでも同じパターンが存在する時ペイントする（4*2 or 4*4）
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*　パターンの読み込み　*/
    	    settype();	    	/*　パターンタイプのセット　*/
    	    if (Ptype == V_TYPE44) {
    	    	if (getflg(x, y))
    	    	    continue;	/* check Vbuf[y+1][x] */
    	    	if (y == Ly - 1)
    	    	    continue;	/* (X,Ly)も圧縮してしまうので */
    	    }
    	    if (checkr(x, y) > 0) {
    	    	Px = x;
    	    	Py = y;
    	    	paint();    	/*　ペイント開始　*/
    	    }
    	}
    }

/*
 *　周りに一つでも同じパターンが存在する時ペイントする（4*2）
 *  （塗残しをペイントする）
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*　パターンの読み込み　*/
    	    Ptype = V_TYPE42;
    	    Offx = 1;
    	    Offy = 1;	    	/* パターンタイプを４＊２にセット */
    	    if (checkr(x, y) > 0) {
    	    	Px = x;
    	    	Py = y;
    	    	paint();
    	    }
    	}
    }

/*
 * パターンテーブルの作成
 */
    memset(Pattb, 0, sizeof(Pattb));
    memset(Pattr, 0, sizeof(Pattr));
    memset(Pattg, 0, sizeof(Pattg));
    memset(Patti, 0, sizeof(Patti));
    Tbsize = 256;   	    	/* dummy */
    P2mask = V_TBMASK;
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (!(Vbuf[y][x] & V_PAINT))
    	    	continue;
    	    if (Vbuf[y][x] & V_TBMASK)
    	    	continue;
    	    getpat(x, y);   	/*　パターンの読み込み　*/
    	    find = tbcmppat(Patb1, Patr1, Patg1, Pati1);
    	    if (find == (word) (-1))
    	    	tbsetpat(Patb1, Patr1, Patg1, Pati1);
    	    if (Vbuf[y][x] & V_TYPE44) {
    	    	find = tbcmppat(Patb2, Patr2, Patg2, Pati2);
    	    	if (find == (word) (-1))
    	    	    tbsetpat(Patb2, Patr2, Patg2, Pati2);
    	    }
    	    Px = x;
    	    Py = y;
    	    paintbmt();
    	}
    }
    if (Pattptr <= 2) {
    	Tbsize = 2;
    	Tbmask = 0x01;
    	Tbb = 0;
    } else if (Pattptr <= 4) {
    	Tbsize = 4;
    	Tbmask = 0x02;
    	Tbb = 1;
    } else if (Pattptr <= 8) {
    	Tbsize = 8;
    	Tbmask = 0x04;
    	Tbb = 2;
    } else if (Pattptr <= 16) {
    	Tbsize = 16;
    	Tbmask = 0x08;
    	Tbb = 3;
    } else if (Pattptr <= 32) {
    	Tbsize = 32;
    	Tbmask = 0x10;
    	Tbb = 4;
    } else if (Pattptr <= 64) {
    	Tbsize = 64;
    	Tbmask = 0x20;
    	Tbb = 5;
    } else if (Pattptr <= 128) {
    	Tbsize = 128;
    	Tbmask = 0x40;
    	Tbb = 6;
    } else if (Pattptr <= 256) {
    	Tbsize = 256;
    	Tbmask = 0x80;
    	Tbb = 7;
    }
/*
 *  独立している（周りに同じパターンがない）4*2パターンで、
 *　パターンテーブルにそのパターンが存在する時
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*　パターンの読み込み　*/
    	    find = tbcmppat(Patb1, Patr1, Patg1, Pati1);
    	    if (find != (word) (-1)) {
    	    	Vbuf[y][x] = (byte) V_SAME42;
    	    	if (x - 1 >= 0)
    	    	    Vbuf[y][x - 1] = V_NON;
    	    	if (x + 1 < LMAXX)
    	    	    Vbuf[y][x + 1] = V_NON;
    	    	if (y - 1 >= 0)
    	    	    Vbuf[y - 1][x] = V_NON;
    	    	if (y + 1 < Ly)
    	    	    Vbuf[y + 1][x] = V_NON;
    	    }
    	}
    }
}


/*---------------------------------------------------------------------------*/
/*-------------------
 * ハフマン符号化
 *-------------------
 */
void
inithuf(void)
{
    Cthbuf = 0;
}

void
sethuf(word num, word count)
{
    if (Cthbuf == MAXHUF)
    	error(ER_BUG);

    Hbuf[Cthbuf].num	 = num;
    Hbuf[Cthbuf].count	 = count;
    Hbuf[Cthbuf].treenum = 0;
    Hbuf[Cthbuf].code	 = 0;
    Hbuf[Cthbuf].codenum = 0;

    ++Cthbuf;
}

void
sethufbit(int i, int dt)
{
    Hbuf[i].code |= dt << (Hbuf[i].codenum++);
}


void
buildhufnode(int k, word tn, int dt)
{
    int  i;

    for (i = k; i < Cthbuf; ++i) {
    	if (Hbuf[i].treenum == tn)
    	    sethufbit(i, dt);
    }
}


void
sorthufnum(void)
{
    struct HUF tmphbuf;
    int  i, j;

    for (i = 0; i < Cthbuf - 1; ++i) {
    	for (j = i + 1; j < Cthbuf; ++j) {
    	    if (Hbuf[i].num <= Hbuf[j].num)
    	    	continue;
    	    tmphbuf = Hbuf[i];
    	    Hbuf[i] = Hbuf[j];
    	    Hbuf[j] = tmphbuf;
    	}
    }
}


void
buildhuftree(void)
{
    struct HUF tmphbuf;
    word tnum, tn1, tn2;
    int  i, j, k;

    tnum = 1;
    for (i = 0; i < Cthbuf - 1; ++i) {
    	for (j = i + 1; j < Cthbuf; ++j) {
    	    if (Hbuf[i].count >= Hbuf[j].count)
    	    	continue;
    	    tmphbuf = Hbuf[i];
    	    Hbuf[i] = Hbuf[j];
    	    Hbuf[j] = tmphbuf;
    	}
    }
    for (k = Cthbuf - 1; k > 0; --k) {
    	sethufbit(k - 1, 0);
    	sethufbit(k, 1);
    	if (Hbuf[k - 1].treenum)
    	    buildhufnode(k + 1, Hbuf[k - 1].treenum, 0);
    	if (Hbuf[k].treenum)
    	    buildhufnode(k + 1, Hbuf[k].treenum, 1);

       /* ツリー番号セット */
    	tn1 = Hbuf[k - 1].treenum;
    	tn2 = Hbuf[k].treenum;
    	if (tn1 == 0 && tn2 == 0) {
    	    Hbuf[k - 1].treenum = tnum;
    	    Hbuf[k].treenum = tnum;
    	    ++tnum;
    	} else if (tn1 > tn2) {
    	    Hbuf[k].treenum = tn1;  	/* ツリー番号の更新 */
    	    for (i = k + 1; i < Cthbuf; ++i) {
    	    	if (tn2 == Hbuf[i].treenum)
    	    	    Hbuf[i].treenum = tn1;
    	    }
    	} else {
    	    Hbuf[k - 1].treenum = tn2;	/* ツリー番号の更新 */
    	    for (i = k + 1; i < Cthbuf; ++i) {
    	    	if (tn1 == Hbuf[i].treenum)
    	    	    Hbuf[i].treenum = tn2;
    	    }
    	}
    	Hbuf[k - 1].count += Hbuf[k].count;

    	i = k - 1;
    	while (i != 0 && Hbuf[i].count >= Hbuf[i - 1].count) {	/* > demoii */
    	    tmphbuf = Hbuf[i];
    	    Hbuf[i] = Hbuf[i - 1];
    	    Hbuf[i - 1] = tmphbuf;
    	    --i;
    	}
    }
}

void
savepass1(void)
{
    byte data, bt, btbuf[LMAXY][LMAXX / 8];
    word i, x, y, k, h, l, t, ctbtbuf[0x10];
    int  find;

    putbits(0x04, Tbb); /* パターンテーブルの最大オフセットビット長 出力 */

    memset(ctbtbuf, 0, sizeof(ctbtbuf));
/*
 * ビットマップテーブルデータを４ビット(0-0xf)ごとに区切る
 *  （ハフマン符号化のため）
 */
    i = 0;
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; x += 8) {
    	    data = 0;
    	    bt = 0x80;
    	    for (k = 0; k < 8; ++k) {
    	    	if (Vbuf[y][x + k] & V_PAINT) {
    	    	    data |= bt;
    	    	}
    	    	bt >>= 1;
    	    }
    	    btbuf[y][x / 8] = data;
    	}
    }
/*
 *　テーブルにＥＯＲをかける
 */
    for (y = 0; y < Ly - 1; ++y) {
    	for (x = 0; x < LMAXX / 8; ++x) {
    	    btbuf[y][x] = btbuf[y][x] ^ btbuf[y + 1][x];
    	}
    }
/*
 *  ハフマン符号化
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX / 8; ++x) {
    	    ++ctbtbuf[btbuf[y][x] & 0xf];
    	    ++ctbtbuf[btbuf[y][x] >> 4];
    	}
    }
    inithuf();
    t = 0;
    for (i = 0; i < 0x10; ++i) {
    	t += ctbtbuf[i];
    	sethuf(i, ctbtbuf[i]);
    }
    buildhuftree();
    sorthufnum();


/*
 *　符号のセーブ
 */
    for (i = 0; i < 0x10; ++i) {/*  符号長を出力 */
    	putbits(0x08, Hbuf[i].codenum);
    }
    for (i = 0; i < 0x10; ++i) {/*  符号を出力 */
    	putbits(1 << (Hbuf[i].codenum - 1), Hbuf[i].code);
    }
/*
 *　ビットマップデータのセーブ
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX / 8; ++x) {
    	    h = btbuf[y][x] >> 4;
    	    l = btbuf[y][x] & 0xf;
    	    putbits(1 << (Hbuf[h].codenum - 1), Hbuf[h].code);
    	    putbits(1 << (Hbuf[l].codenum - 1), Hbuf[l].code);
    	}
    }
/*
 *  パターンデータのセーブ
 *  A. 0xxx xxxx    	    	    	  - V_TYPE42 found pattern
 *  B. 10bb bbbb bbrr rrrr rrgg gggg gg   - V_TYPE42 not found pattern
 *  C. 11   	    	    	    	  - V_TYPE44
 *  D.	 0x xxxx xx 	    	    	  -   found pattern
 *  E.	 1b bbbb bbbr rrrr rrrg gggg ggg  -   not found pattern
 *  F. xxxx xxx     	    	    	  - V_SAME42 (識別子がなくても判別可能)
 *
 *     b - blue  パターン 8bit(4*2)
 *     r - red	 パターン 8bit(4*2)
 *     g - green パターン 8bit(4*2)
 *     x - パターンテーブルの位置 1-7bit（7bit固定ではない）
 */
    P2mask = V_PAINTED;     	/* paint2 でのマスクビットの設定 */
    Pattptr = 1;    	    	/* パターンテーブルの位置を初期化　*/
    memset(Pattb, 0, sizeof(Pattb));
    memset(Pattr, 0, sizeof(Pattr));
    memset(Pattg, 0, sizeof(Pattg));
    memset(Patti, 0, sizeof(Patti));
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x] & V_PAINTED)
    	    	continue;

    	    if (Vbuf[y][x] & V_TYPE42) {
    	    	getpat(x, y);
    	    	find = tbcmppat(Patb1, Patr1, Patg1, Pati1);
    	    	if (find != -1) {
    	    	   /*　4*2で、テーブルにそのパターンがある時　*/
    	    	    putbit(0);	/* 識別子出力 */
    	    	    putbits(Tbmask, find);
    	    	} else {
    	    	   /*　4*2で、テーブルにそのパターンがない時　*/
    	    	    tbsetpat(Patb1, Patr1, Patg1, Pati1);
    	    	    putbit(1);
    	    	    putbit(0);	/* 識別子出力 */
    	    	    putbits(0x80, Patb1);
    	    	    putbits(0x80, Patr1);
    	    	    putbits(0x80, Patg1);
    	    	    if (Color16)
    	    	    	putbits(0x80, Pati1);
    	    	}
    	    	Px = x;
    	    	Py = y;
    	    	paintbmt(); 	/* ビットマップテーブル上をペイントする　*/

    	    } else if (Vbuf[y][x] & V_TYPE44) {
    	    	putbit(1);
    	    	putbit(1);  	/* 識別子出力 */
    	    	getpat(x, y);
    	    	find = tbcmppat(Patb1, Patr1, Patg1, Pati1);
    	    	if (find != -1) {
    	    	   /*　4*4の上半分 4*2で、テーブルにそのパターンがある時　*/
    	    	    putbit(0);	    	   /* 識別子出力 */
    	    	    putbits(Tbmask, find); /* テーブルの位置出力 */
    	    	} else {
    	    	   /*  4*4の上半分 4*2で、テーブルにそのパターンがない時  */
    	    	    tbsetpat(Patb1, Patr1, Patg1, Pati1); /*ﾊﾟﾀｰﾝをﾃｰﾌﾞﾙにｾｯﾄ*/
    	    	    putbit(1);	    	   /* 識別子出力 */
    	    	    putbits(0x80, Patb1);  /* パターン出力 */
    	    	    putbits(0x80, Patr1);
    	    	    putbits(0x80, Patg1);
    	    	    if (Color16)
    	    	    	putbits(0x80, Pati1);
    	    	}

    	    	find = tbcmppat(Patb2, Patr2, Patg2, Pati2);
    	    	if (find != -1) {
    	    	   /*　4*4の下半分 4*2で、テーブルにそのパターンがある時　*/
    	    	    putbit(0);	/* 識別子出力 */
    	    	    putbits(Tbmask, find);  	/* テーブルの位置出力 */
    	    	} else {
    	    	   /*　4*4の下半分 4*2で、テーブルにそのパターンがない時　*/
    	    	    tbsetpat(Patb2, Patr2, Patg2, Pati2); /*ﾊﾟﾀｰﾝをﾃｰﾌﾞﾙにｾｯﾄ*/
    	    	    putbit(1);	/* 識別子出力 */
    	    	    putbits(0x80, Patb2);
    	    	    putbits(0x80, Patr2);
    	    	    putbits(0x80, Patg2);
    	    	    if (Color16)
    	    	    	putbits(0x80, Pati2);
    	    	}
    	    	Px = x;
    	    	Py = y;
    	    	paintbmt();
    	    }
    	}
    }

    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x] & V_SAME42) {
    	    	getpat(x, y);
    	    	find = tbcmppat(Patb1, Patr1, Patg1, Pati1);
    	    	if (find == -1) {
    	    	    puts("pass1");
    	    	    error(ER_BUG);
    	    	}
    	    	putbits(Tbmask, find);
    	    	Px = x;
    	    	Py = y;
    	    	paintbmt();
    	    }
    	}
    }
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------
 * 上下左右の最初に見つかるペイントパターンをサーチ
 *---------------------------------------------------
 * in : x,y 現在の座標
 *  	w   high(=1) or low(=0)  4 bits
 *  	v   サーチ方向 (UP, DOWN, LEFT, RIGHT)
 * out: return value = 0 - パターンが見つからなかった
 *  	    	       1 - パターンを見つけた
 *  	    	    	   (Patb,Patr,Patg)
 */
int
findpat(int x, int y, int w, int v)
{
    int  findflg;

    findflg = 0;
    switch (v) {
    case UP:
    	while (--y >= 0)
    	    if (Vbuf[y][x] & V_PAINTED) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    case DOWN:
    	while (++y < Ly)
    	    if (Vbuf[y][x] & V_PAINTED) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    case LEFT:
    	while (--x >= 0)
    	    if (Vbuf[y][x] & V_PAINTED) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    case RIGHT:
    	while (++x < LMAXX)
    	    if (Vbuf[y][x] & V_PAINTED) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    }
    if (findflg) {
    	get4bitpat(x, y, w, &Patb, &Patr, &Patg, &Pati);
    }
    return findflg;
}


int
pp_l2col()
{
    static int code[] = {
    	0, P_2COL1, P_2COL2, P_2COL3, P_2COL4, P_2COL5, P_2COL6, P_2COL7,
    	P_2COL8, P_2COL9, P_2COLA, P_2COLB, P_2COLC, P_2COLD, P_2COLE, 0
    };
    word pat, colorcode;
    int  i;

    pat = 0;
    for (i = 0x8; i != 0; i >>= 1) {
    	if (i & Npatb)
    	    colorcode = 1;
    	else
    	    colorcode = 0;
    	if (i & Npatr)
    	    colorcode |= 2;
    	if (i & Npatg)
    	    colorcode |= 4;
    	if (i & Npati && Color16)
    	    colorcode |= 8;

    	if (Lcol2 == colorcode)
    	    pat |= i;
    	else if (Lcol1 != colorcode)
    	    return 0;
    }

    if ((Pass == SAVEPASS2 && Ctp2[code[pat]] != 0) || Pass == PASS2) {
    	Ucol1 = Lcol1;
    	Ucol2 = Lcol2;
    }
    return code[pat];
}


/*------------------------------
 * 上下左右で同じパターンを探す
 *------------------------------
 */
int
pp_same(int x, int y, int w)
{
    if (findpat(x, y, w, DOWN)) {
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg
    	    && (!Color16 || Npati == Pati)) {
    	    return P_SAMEDOWN;
    	}
    }
    if (findpat(x, y, w, UP)) {
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg
    	    && (!Color16 || Npati == Pati)) {
    	    return P_SAMEUP;
    	}
    }
    if (findpat(x, y, w, RIGHT)) {
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg
    	    && (!Color16 || Npati == Pati)) {
    	    return P_SAMERIGHT;
    	}
    }
    if (findpat(x, y, w, LEFT)) {
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg
    	    && (!Color16 || Npati == Pati)) {
    	    return P_SAMELEFT;
    	}
    }
    return 0;
}

/*------------------------------
 * 一つ上のパターンと比べる
 *------------------------------
 */
int
pp_same2(int x, int y, int w)
{
    if (y == 0)
    	return 0;

    get4bitpat(x, y - 1, w, &Patb, &Patr, &Patg, &Pati);
    if (Npatb == Patb && Npatr == Patr && Npatg == Patg
    	&& (!Color16 || Npati == Pati)) {
    	return P_SAME1UP;
    }
    return 0;
}

/*--------------------------------------------------------------------
 * 現在のパターンとPat?のパターンで、異なる色が一色かどうかを確かめる
 *--------------------------------------------------------------------
 * return value  = 0  一色ではない
 *  	    	!= 0  一色(XXXX XXiX  Xgrb pppp)
 *  	    	    	igrb 色
 *  	    	    	pppp パターン
 */
int
cmp1c(void)
{
    byte pat, patb, patr, patg, pati;
    int  retval;

   /* パターンと異なる色のビットを抽出 */
    pat = (Patb ^ Npatb) | (Patr ^ Npatr) | (Patg ^ Npatg);
    if (Color16)
    	pat |= (Pati ^ Npati);

    if (pat == 0 || pat == 0xf)
    	return 0;

    patb = pat & Npatb;     	/* プレーンごとに異なる色を抽出 */
    patr = pat & Npatr;
    patg = pat & Npatg;
    pati = pat & Npati;

   /* patb,r,g は、すべてpatと同じ、または 0でなければならない */
    if (patb != pat && patb != 0)
    	return 0;
    if (patr != pat && patr != 0)
    	return 0;
    if (patg != pat && patg != 0)
    	return 0;
    if (pati != pat && pati != 0 && Color16)
    	return 0;

    retval = pat;
    if (patb)
    	retval |= 0x10;
    if (patr)
    	retval |= 0x20;
    if (patg)
    	retval |= 0x40;
    if (pati && Color16)
    	retval |= 0x200;

    return retval;
}

/*------------------------------
 * 左のパターンと他の１カラー
 *------------------------------
 */
int
pp_l1c(int x, int y, int w)
{
    int  i;

    if (!findpat(x, y, w, LEFT))
    	return 0;
    i = cmp1c();

    return i ? P_LC + i : 0;
}

/*------------------------------
 * 右のパターンと他の１カラー
 *------------------------------
 */
int
pp_r1c(int x, int y, int w)
{
    int  i;

    if (!findpat(x, y, w, RIGHT))
    	return 0;
    i = cmp1c();

    return i ? P_RC + i : 0;
}

/*------------------------------
 * 上のパターンと他の１カラー
 *------------------------------
 */
int
pp_u1c(int x, int y, int w)
{
    int  i;

    if (!findpat(x, y, w, UP))
    	return 0;
    i = cmp1c();

    return i ? P_UC + i : 0;
}

/*------------------------------
 * 下のパターンと他の１カラー
 *------------------------------
 */
int
pp_d1c(int x, int y, int w)
{
    int  i;

    if (!findpat(x, y, w, DOWN))
    	return 0;
    i = cmp1c();

    return i ? P_DC + i : 0;
}


/*-------------------
 * 一色のブロック
 *-------------------
 */
int
pp_col(void)
{
    static int code[] = {
    	P_COL0, P_COL1, P_COL2, P_COL3, P_COL4, P_COL5, P_COL6, P_COL7,
    	P_COL8, P_COL9, P_COL10, P_COL11, P_COL12, P_COL13, P_COL14, P_COL15
    };
    int  n;

    if (Npatb != 0 && Npatb != 0x0f)
    	return 0;
    if (Npatr != 0 && Npatr != 0x0f)
    	return 0;
    if (Npatg != 0 && Npatg != 0x0f)
    	return 0;
    if (Npati != 0 && Npati != 0x0f && Color16)
    	return 0;

    n = (Npatg & 4) | (Npatr & 2) | (Npatb & 1);
    if (Color16)
    	n |= Npati & 0x8;

    return code[n];
}


/*---------------------------------------------------------
 *　二つのパターンに はさまれているパターンかどうかを調べる
 *---------------------------------------------------------
 */
int
cmplr(void)
{   	    	       /* 0  1	2  3  4  5  6  7  8  9	a  b  c  d  e  f */
    static int code[] = { 0, 0, 0, 0, 0, 1, 2, 0, 0, 3, 4, 0, 0, 0, 0, 0};
    byte pat, mbit;
    int  i, llen, rlen;

    pat = (Lpatb ^ Npatb) | (Lpatr ^ Npatr) | (Lpatg ^ Npatg);
    if (Color16)
    	pat |= (Lpati ^ Npati);

    mbit = 0x8;     	    	/* mask bit */
    for (i = 0; i < 4; ++i) {
    	if (pat & mbit)
    	    break;
    	mbit >>= 1;
    }	    	    	    	/* i = 0,1,2,3,4 */
    llen = i;

    pat = (Rpatb ^ Npatb) | (Rpatr ^ Npatr) | (Rpatg ^ Npatg);
    if (Color16)
    	pat |= (Rpati ^ Npati);

    mbit = 0x1;     	    	/* mask bit */
    for (i = 0; i < 4; ++i) {
    	if (pat & mbit)
    	    break;
    	mbit <<= 1;
    }	    	    	    	/* i = 0,1,2,3,4 */
    rlen = (llen + i > 4) ? 4 - llen : i;

/* 左右と異なるパターンを抽出 */
    pat = Npatb & (0xf >> llen) & (0xf << rlen);
    pat |= Npatr & (0xf >> llen) & (0xf << rlen);
    pat |= Npatg & (0xf >> llen) & (0xf << rlen);
    if (Color16)
    	pat |= Npati & (0xf >> llen) & (0xf << rlen);

    if (pat)
    	return 0;   	    	/* 黒以外のパターンは無視 */

    return code[(llen << 2) | (rlen)];	/* return value = 1,2,3,4 */
}

/*--------------------------------
 * 左－右　のパターンを組み合わせる
 *--------------------------------
 */
int
pp_lr(int x, int y, int w)
{
    static int code[] = {0, P_LR11, P_LR13, P_LR31, P_LR22};

    if (!findpat(x, y, w, LEFT))
    	return 0;   	    	/* 左のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, RIGHT))
    	return 0;   	    	/* 右のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * 左－上　のパターンを組み合わせる
 *--------------------------------
 */
int
pp_lu(int x, int y, int w)
{
    static int code[] = {0, P_LU11, P_LU13, P_LU31, P_LU22};

    if (!findpat(x, y, w, LEFT))
    	return 0;   	    	/* 左のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* 上のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * 左－下　のパターンを組み合わせる
 *--------------------------------
 */
int
pp_ld(int x, int y, int w)
{
    static int code[] = {0, P_LD11, P_LD13, P_LD31, P_LD22};

    if (!findpat(x, y, w, LEFT))
    	return 0;   	    	/* 左のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* 下のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * 上－右　のパターンを組み合わせる
 *--------------------------------
 */
int
pp_ur(int x, int y, int w)
{
    static int code[] = {0, P_UR11, P_UR13, P_UR31, P_UR22};

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* 上のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, RIGHT))
    	return 0;   	    	/* 右のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * 下－右　のパターンを組み合わせる
 *--------------------------------
 */
int
pp_dr(int x, int y, int w)
{
    static int code[] = {0, P_DR11, P_DR13, P_DR31, P_DR22};

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* 下のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, RIGHT))
    	return 0;   	    	/* 右のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}


/*-------------------------------
 * 上－下　のパターンを組み合わせる
 *-------------------------------
 */
int
pp_ud(int x, int y, int w)
{
    static int code[] = {0, P_UD11, P_UD13, P_UD31, P_UD22};

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* 上のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* 下のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * 下－上　のパターンを組み合わせる
 *--------------------------------
 */
int
pp_du(int x, int y, int w)
{
    static int code[] = {0, P_DU11, P_DU13, P_DU31, P_DU22};

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* 下のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* 上のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * 座標 x-1 のパターンと比較
 *--------------------------------
 */
int
pp_sxm1(int x, int y, int w)
{
    if (x == 0)
    	return 0;
/*  （Ｘ－１，Ｙ） */
    get4bitpat(x - 1, y, w, &Patb, &Patr, &Patg, &Pati);
    if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	(!Color16 || Npati == Pati)) {
    	return P_S0;
    }
/*  （Ｘ－１，Ｙ－１） */
    if (y > 0) {
    	get4bitpat(x - 1, y - 1, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SU1;
    	}
    }
/*  （Ｘ－１，Ｙ－２） */
    if (y > 1) {
    	get4bitpat(x - 1, y - 2, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SU2;
    	}
    }
/*  （Ｘ－１，Ｙ－３） */
    if (y > 3) {
    	get4bitpat(x - 1, y - 3, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SU3;
    	}
    }
/*  （Ｘ－１，Ｙ＋１） */
    if (y < Ly - 1) {
    	get4bitpat(x - 1, y + 1, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SD1;
    	}
    }
/*  （Ｘ－１，Ｙ＋２） */
    if (y < Ly - 2) {
    	get4bitpat(x - 1, y + 2, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SD2;
    	}
    }
/*  （Ｘ－１，Ｙ＋３） */
    if (y < Ly - 3) {
    	get4bitpat(x - 1, y + 3, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SD3;
    	}
    }
    return 0;
}


/*
 *  最初に見つけた２色を返す
 */
int
get2color(void)
{
    word i, colorcode, c1;

    c1 = 0xff;
    for (i = 0x8; i != 0; i >>= 1) {
    	if (i & Npatb)
    	    colorcode = 1;
    	else
    	    colorcode = 0;
    	if (i & Npatr)
    	    colorcode |= 2;
    	if (i & Npatg)
    	    colorcode |= 4;
    	if ((i & Npati) && Color16)
    	    colorcode |= 8;

    	if (c1 != colorcode) {
    	    if (c1 != 0xff)
    	    	return (c1 << 4) | colorcode;
    	    c1 = colorcode;
    	}
    }
    return (Ucol1<<4) | Ucol2;/* 2色以上でなければ、最後に使用した2色を返す */
}


/*-----------------
 * 圧縮関数
 *-----------------
 */
int
pass22(int x, int y, int w)
{
    int  i, col;

    	 if ((i = pp_same2(x, y, w)) != 0);
    else if ((i = pp_same(x, y, w) ) != 0);
    else if ((i = pp_col()  	   ) != 0);
    else if ((i = pp_l2col()	   ) != 0);
    else if ((i = pp_l1c(x, y, w)  ) != 0);
    else if ((i = pp_r1c(x, y, w)  ) != 0);
    else if ((i = pp_lr(x, y, w)   ) != 0);
    else if ((i = pp_sxm1(x, y, w) ) != 0); 	/* x-1に同じパターンがあるか?*/
    else if ((i = pp_u1c(x, y, w)  ) != 0);
    else if ((i = pp_d1c(x, y, w)  ) != 0);
    else if ((i = pp_lu(x, y, w)   ) != 0);
    else if ((i = pp_ld(x, y, w)   ) != 0);
    else if ((i = pp_ur(x, y, w)   ) != 0);
    else if ((i = pp_dr(x, y, w)   ) != 0);
    else if ((i = pp_ud(x, y, w)   ) != 0);
    else if ((i = pp_du(x, y, w)   ) != 0);

    col = get2color();	    	/* 現在のパターンのうち2色を覚えておく */
    Lcol1 = col >> 4;
    Lcol2 = col & 0xf;

    return i;
}


void
pass2(void)
{
    int  x, y, w, i, j;
    long nct;

    memset(Ctp2, 0, sizeof(Ctp2));

    Lcol1 = Ucol1 = 0;
    Lcol2 = Ucol2 = 1;
    nct = 0;
    for (x = 0; x < LMAXX; ++x) {
    	for (y = 0; y < Ly; ++y) {
    	    if (Vbuf[y][x] != V_NON && Vbuf[y][x] != 0)
    	    	continue;
    	    for (w = 1; w >= 0; --w) {
    	    	++nct;
    	    	get4bitpat(x, y, w, &Npatb, &Npatr, &Npatg, &Npati);
    	    	++Ctp2[pass22(x, y, w)];
    	    }
    	}
    }

    inithuf();
    for (i = 1; i < MAXP2TB; ++i) {/* "1"から始める("0"の非圧縮部分は除く) */
    	if (Ctp2[i] >= 6) {
    	    sethuf(i, (word) Ctp2[i]);
    	} else {
    	    Ctp2[0] += Ctp2[i]; /*  圧縮できなかった数はP_NONに加える　*/
    	    Ctp2[i] = 0;
    	}
    }
    sethuf(0, (word) Ctp2[0]);	/* 非圧縮部分をセット */

    buildhuftree();
    sorthufnum();

   /*  符号情報のセーブ */
    for (i = 0; i < Cthbuf; ++i) {
    	putbits(1 << (P2tbbit - 1), Hbuf[i].num);
    	putbits(0x8, Hbuf[i].codenum);
    	putbits(1 << (Hbuf[i].codenum - 1), Hbuf[i].code);
    }
    putbits(1 << (P2tbbit - 1), 0); 	/* エンドコード */

    for (i = Cthbuf - 1; i >= 0; --i) { /*　符号変換配列の並べ代え */
    	j = Hbuf[i].num;
    	Hbuf[j] = Hbuf[i];
    }
}


/*---------------------------------------------------------------------------*/
void
savepass2(void)
{
    int  x, y, w, n;

    Lcol1 = Ucol1 = 0;
    Lcol2 = Ucol2 = 1;
    for (x = 0; x < LMAXX; ++x) {
    	for (y = 0; y < Ly; ++y) {
    	    if (Vbuf[y][x] != V_NON && Vbuf[y][x] != 0)
    	    	continue;
    	    for (w = 1; w >= 0; --w) {
    	    	get4bitpat(x, y, w, &Npatb, &Npatr, &Npatg, &Npati);

    	    	n = pass22(x, y, w);
    	    	if (Ctp2[n] == 0)
    	    	    n = P_NON;

    	    	putbits(1 << (Hbuf[n].codenum - 1), Hbuf[n].code);

    	    	if (n == P_NON) {
    	    	    putbits(0x8, Npatb);
    	    	    putbits(0x8, Npatr);
    	    	    putbits(0x8, Npatg);
    	    	    if (Color16)
    	    	    	putbits(0x8, Npati);
    	    	}
    	    }
    	}
    }
}


/*--------------
 * hsv -> RGB
 *--------------
 * in : h  - 色相     (0-360)
 *  	s  - 色彩     (0-100)
 *  	v  - 最大明度 (0-100)
 *
 * out: rgb[0]	- R (0-15)
 *  	rgb[1]	- G (0-15)
 *  	rgb[2]	- B (0-15)
 */
void
hsv_to_rgb(int h, int s, int v, int rgb[])
{
    int  a[7];
    int  i, f, r, g, b;

    i = h / 60;
    f = (h % 60) * 100 / 60;

    a[1] = a[2] = v;
    a[3] = v * (100 - s * f / 100) / 100;
    a[4] = a[5] = v * (100 - s) / 100;
    a[6] = v * (100 - (s * (100 - f)) / 100) / 100;

    r = a[i = (i > 4) ? i - 4 : i + 2] * 16 / 100;
    b = a[i = (i > 4) ? i - 4 : i + 2] * 16 / 100;
    g = a[i = (i > 4) ? i - 4 : i + 2] * 16 / 100;

    rgb[0] = (r >= 16) ? 15 : r;
    rgb[1] = (g >= 16) ? 15 : g;
    rgb[2] = (b >= 16) ? 15 : b;
}


int
savealg(void)
{
    FILE *fpr;
    static int cv[] = {0, 9, 10, 11, 12, 13, 14, 15, 8, 1, 2, 3, 4, 5, 6, 7};
    int  a, b, i, h, s, v, hsv[48], rgb[3];

    if ((fpr = fopen(Algname, "rb")) == NULL)
    	return -1;
    for (i = 0; i < 48; ++i) {
    	hsv[i] = getc(fpr);
    	hsv[i] += getc(fpr) * 256;
    }
    for (i = 0; i < 16; ++i) {
    	a = cv[i] / 8;
    	b = cv[i] % 8;
    	h = hsv[a * 24 + b];
    	s = hsv[a * 24 + b + 8];
    	v = hsv[a * 24 + b + 16];
    	hsv_to_rgb(h, s, v, rgb);

    	putbits(0x80, rgb[0]);
    	putbits(0x80, rgb[1]);
    	putbits(0x80, rgb[2]);
    }
    fclose(fpr);
    return 0;
}


int
savergb(void)
{
    FILE *fpr;
    int  i, d;

    if ((fpr = fopen(Algname, "rb")) == NULL)
    	return -1;
    for (i = 0; i < 16 * 3; ++i) {
    	d = getc(fpr);
    	if (d < 0)
    	    return -1;
    	if (d > 15)
    	    d = 15;
    	putbits(0x80, d);
    }
    fclose(fpr);
    return 0;
}

#ifdef _RPAL_H_
int
saveRpal(void)
{
    int  toon,i;
    byte grb[16][3];

    if (RPal_Get(&toon,(byte *)(grb)))
    	return -1;
    for (i = 0; i < 16; ++i) {
    	putbits(0x80, grb[i][1]);
    	putbits(0x80, grb[i][0]);
    	putbits(0x80, grb[i][2]);
    }
    return 0;
}
#endif

void
vhsave(void)
{
    int  attr;

    if ((Fpw = fopen(Savename, "wb")) == NULL) {
    	puts("Can't	open.");
    	puts(Savename);
    	error(ER_EXIT);
    }
    if (Algname[0] != '\0') {
    	Analog = ON;
    	Color16 = ON;
    	P2tbbit = 10;
    } else if (Analog) {
    	extcat(Savename, ".RGB", Algname);
    }
    attr  = (Ly == 100)? AT_200    : AT_400;
    attr |= (Color16)  ? AT_C16    : AT_C8;
    attr |= (Analog)   ? AT_ANALOG : AT_DIGITAL;

    putbits(0x80, attr);    	/* 属性出力 */

 #ifndef PC88VA
    if (Color16)    /* for pc98 */
    	outp(0x6a, 1);
    else
    	outp(0x6a, 0);
 #endif

    if (Analog) {
    	if (savergb()) {
    	    extcat(Algname, ".ALG", Algname);
    	    if (savealg())
 #ifdef _RPAL_H_
    	    	if (saveRpal())
 #endif
    	    	{
    	    	    puts("Can't	open.");
    	    	    puts(Algname);
    	    	    error(ER_EXIT);
    	    	}

    	}
    }

#ifdef PC88VA
    acs_vram();
#endif
    swapvram1();

    Pass = PASS1;
    pass1();
    Pass = SAVEPASS1;
    savepass1();
    Pass = PASS2;
    pass2();
    Pass = SAVEPASS2;
    savepass2();

    swapvram2();
#ifdef PC88VA
    acs_text();
#endif

    flushbit();
    fclose(Fpw);
}

/*---------------------------------------------------------------------------*/
void
init(void)
{
    Savename[0] = Algname[0] = '\0';

    Ly	    = 100;  	    	/* 100 - 200 line , 200 - 400 line */
    P2tbbit = 9;    	    	/*   9 - 8 color  ,  10 - 16 color */
    Color16 = OFF;  	    	/* OFF - 8 color  ,  ON - 16 color */
    Analog  = OFF;  	    	/* OFF - digital  ,  ON - analog   */
}


void
dispusage(void)
{
#ifndef PC88VA
    puts("VHP Saver	Ver.1.21T for PC9801  by ﾏｼｰﾝM(Ver.1.11)");
#else
    puts("VHP Saver	Ver.1.21T for PC88VA  by ﾏｼｰﾝM(Ver.1.11	for	PC9801)");
#endif
    puts("                       Modified by ﾃﾝｶ*ﾌﾝ");
    puts("Usage: vhsave	[-{<sw>}] <pathname1>[.VHP]	[<pathname2>[.RGB|.ALG]] [-{<sw>}]");
    puts("<sw>:	?, 2|4,	6|8|a");
    puts("  ?    ヘルプ表示");
    puts("  2    ２００ライン （ﾃﾞﾌｫﾙﾄ）");
    puts("  4    ４００ライン");
    puts("  6    １６色デジタル");
    puts("  8      ８色デジタル （ﾃﾞﾌｫﾙﾄ）");
    puts("  a    １６色アナログ （<pathname1>.RGB|.ALG が必要です）\n");
    puts(" <pathname2>[.RGB|.ALG]を指定すると１６色アナログになります");
    puts("                          （<sw> 6|8|a 指定は無視されます）");

    exit(0);
}

void
checksw(byte *p)
{
    while (*(++p) != '\0') {
    	switch (*p) {
    	case '?':
    	    dispusage();
    	    break;
    	case '2':
    	    Ly = 100;
    	    break;
    	case '4':
    	    Ly = 200;
    	    break;
    	case '6':
    	    Color16 = ON;
    	    P2tbbit = 10;
    	    Analog = OFF;
    	    break;
    	case '8':
    	    Color16 = OFF;
    	    P2tbbit = 9;
    	    Analog = OFF;
    	    break;
    	case 'A':
    	case 'a':
    	    Color16 = ON;
    	    P2tbbit = 10;
    	    Analog = ON;
    	    break;
    	default:
    	    error(ER_SW);
    	}
    }
}

void
checkname(byte *p)
{
    if (strlen(p) >= sizeof(Savename) - 4)
    	error(ER_NTLONG);

    if (Savename[0] == '\0') {
    	extcat(p, ".VHP", Savename);
    } else if (Algname[0] == '\0') {
    	extcat(p, ".RGB", Algname);
    } else {
    	dispusage();
    }
}

/*-------------
 * メイン関数
 *-------------
 */
void
main(int argc, byte *argv[])
{
    int  i;

    if (argc < 2)
    	dispusage();
    init();
 #ifdef PC88VA
    va_init();
 #endif

    for (i = 1; i < argc; ++i) {
    	if (*argv[i] == '-')
    	    checksw(argv[i]);
    }
    for (i = 1; i < argc; ++i) {
    	if (*argv[i] == '-')
    	    continue;
    	else
    	    checkname(argv[i]);
    }
    vhsave();
}
