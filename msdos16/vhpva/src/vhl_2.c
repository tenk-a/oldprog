#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vhp.h"
#include "tenkafun.h"
#include "vhl.h"

/* RGBI データ */
static byte Lpatb, Lpatr, Lpatg, Lpati;
/* 最後に使ったパターンの2色を退避(pass2) */
static byte Lpb, Lpr, Lpg, Lpi;

/*---------------------------------------------------------------------------*/

static int
findpat(int x, int y, int w, int v)
    /*
     * 上下左右の最初に見つかるペイントパターンをサーチ
     * in : x,y 現在の座標
     *	    w	high(=1) or low(=0)  4 bits
     *	    v	サーチ方向 (UP, DOWN, LEFT, RIGHT)
     * out: return value = 0 - パターンが見つからなかった
     *	    	    	   1 - パターンを見つけた
     *	    	    	       (Patb,Patr,Patg,Pati)
     */
{
    int  findflg;

    findflg = 0;
    switch (v) {
    case UP:
    	while (--y >= 0)
    	    if (getVb(x,y)) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    case DOWN:
    	while (++y < Ly)
    	    if (getVb(x,y)) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    case LEFT:
    	while (--x >= 0)
    	    if (getVb(x,y)) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    case RIGHT:
    	while (++x < LMAXX)
    	    if (getVb(x,y)) {
    	    	findflg = 1;
    	    	break;
    	    }
    	break;
    }
    if (!findflg) {
    	puts("painted pattern not found");
    	error(ER_AB);
    }
    get4bitpat(x, y, w, &Patb, &Patr, &Patg, &Pati);

    return findflg;
}


static int
get2color(void)
    /*
     *	パターンLp? で使用している２色を返す
     */
{
    word i, colorcode, c1;

    c1 = 0xff;
    for (i = 0x8; i != 0; i >>= 1) {
    	if (i & Lpb)
    	    colorcode = 1;
    	else
    	    colorcode = 0;
    	if (i & Lpr)
    	    colorcode |= 2;
    	if (i & Lpg)
    	    colorcode |= 4;
    	if ((i & Lpi) && Color16)
    	    colorcode |= 8;

    	if (c1 != colorcode) {
    	    if (c1 != 0xff)
    	    	return (c1 << 4) | colorcode;
    	    c1 = colorcode;
    	}
    }
    return (Ucol1<<4) | Ucol2; /* 2色以上でなければ、最後に使用した2色を返す */
}

static void
pp_l2col(int x, int y, int w, int n)
{
    byte pat;
    word lcol12, lcol1, lcol2;

    pat = ((n - P_2COL1) >> 3) + 1;
    lcol12 = get2color();
    lcol1 = lcol12 >> 4;
    lcol2 = lcol12 & 0xf;

    if (lcol1 & 1)
    	Patb = ~pat & 0xf;
    else
    	Patb = 0;
    if (lcol1 & 2)
    	Patr = ~pat & 0xf;
    else
    	Patr = 0;
    if (lcol1 & 4)
    	Patg = ~pat & 0xf;
    else
    	Patg = 0;
    if (lcol1 & 8)
    	Pati = ~pat & 0xf;
    else
    	Pati = 0;

    if (lcol2 & 1)
    	Patb |= pat;
    if (lcol2 & 2)
    	Patr |= pat;
    if (lcol2 & 4)
    	Patg |= pat;
    if (lcol2 & 8)
    	Pati |= pat;

    Ucol1 = lcol1;
    Ucol2 = lcol2;
}


static void
pp_non(int x, int y, int w, int n)
{
    Patb = getbits(4);
    Patr = getbits(4);
    Patg = getbits(4);
    if (Color16)
    	Pati = getbits(4);
}

static void
pp_same(int x, int y, int w, int n)
    /*
     * 上下左右で同じパターンを探す
     */
{
    switch (n) {
    case P_SAMEDOWN:
    	findpat(x, y, w, DOWN);
    	break;
    case P_SAMEUP:
    	findpat(x, y, w, UP);
    	break;
    case P_SAMERIGHT:
    	findpat(x, y, w, RIGHT);
    	break;
    case P_SAMELEFT:
    	findpat(x, y, w, LEFT);
    	break;
    }
}

static void
pp_same2(int x, int y, int w, int n)
    /*
     * 一つ上のパターン
     */
{
    get4bitpat(x, y - 1, w, &Patb, &Patr, &Patg, &Pati);
}


static void
pp_mkp1c(int n)
{
    Patb = (Patb & (~(n & 0xf))) | (n & 0x10 ? (n & 0xf) : 0);
    Patr = (Patr & (~(n & 0xf))) | (n & 0x20 ? (n & 0xf) : 0);
    Patg = (Patg & (~(n & 0xf))) | (n & 0x40 ? (n & 0xf) : 0);
    if (!Color16)
    	return;
    Pati = (Pati & (~(n & 0xf))) | (n & 0x200 ? (n & 0xf) : 0);
}

static void
pp_l1c(int x, int y, int w, int n)
    /*
     * 左のパターンと他の１カラー
     */
{
    findpat(x, y, w, LEFT);
    pp_mkp1c(n);
}

static void
pp_r1c(int x, int y, int w, int n)
    /*
     * 右のパターンと他の１カラー
     */
{
    findpat(x, y, w, RIGHT);
    pp_mkp1c(n);
}

static void
pp_u1c(int x, int y, int w, int n)
    /*
     * 上のパターンと他の１カラー
     */
{
    findpat(x, y, w, UP);
    pp_mkp1c(n);
}

static void
pp_d1c(int x, int y, int w, int n)
    /*
     * 下のパターンと他の１カラー
     */
{
    findpat(x, y, w, DOWN);
    pp_mkp1c(n);
}


/*=============================*/
static void
pp_col(int x, int y, int w, int n)
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
    Patb = ccode[i][0];
    Patr = ccode[i][1];
    Patg = ccode[i][2];
    Pati = ccode[i][3];
}

static void
setlr(int n)
{
    switch (n) {
    case 5:
    	Patb = Lpatb & 0x08 | Rpatb & 0x01;
    	Patr = Lpatr & 0x08 | Rpatr & 0x01;
    	Patg = Lpatg & 0x08 | Rpatg & 0x01;
    	Pati = Lpati & 0x08 | Rpati & 0x01;
    	break;

    case 6:
    	Patb = Lpatb & 0x08 | Rpatb & 0x03;
    	Patr = Lpatr & 0x08 | Rpatr & 0x03;
    	Patg = Lpatg & 0x08 | Rpatg & 0x03;
    	Pati = Lpati & 0x08 | Rpati & 0x03;
    	break;

    case 9:
    	Patb = Lpatb & 0x0c | Rpatb & 0x01;
    	Patr = Lpatr & 0x0c | Rpatr & 0x01;
    	Patg = Lpatg & 0x0c | Rpatg & 0x01;
    	Pati = Lpati & 0x0c | Rpati & 0x01;
    	break;

    case 0xa:
    	Patb = Lpatb & 0x0c | Rpatb & 0x03;
    	Patr = Lpatr & 0x0c | Rpatr & 0x03;
    	Patg = Lpatg & 0x0c | Rpatg & 0x03;
    	Pati = Lpati & 0x0c | Rpati & 0x03;
    	break;

    default:
    	error(ER_AB);
    }
}

static void
pp_lr(int x, int y, int w, int n)
    /*
     * 左右のパターンを組み合わせる
     */
{
    findpat(x, y, w, LEFT); 	/* 左のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    findpat(x, y, w, RIGHT);	/* 右のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    setlr(n);
}

static void
pp_lu(int x, int y, int w, int n)
    /*
     * 左上のパターンを組み合わせる
     */
{
    findpat(x, y, w, LEFT); 	/* 左のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    findpat(x, y, w, UP);   	/* 上のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    setlr(n);
}

static void
pp_ld(int x, int y, int w, int n)
    /*
     * 左下のパターンを組み合わせる
     */
{
    findpat(x, y, w, LEFT); 	/* 左のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    findpat(x, y, w, DOWN); 	/* 下のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    setlr(n);
}

static void
pp_ur(int x, int y, int w, int n)
    /*
     * 上右のパターンを組み合わせる
     */
{
    findpat(x, y, w, UP);   	/* 上のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    findpat(x, y, w, RIGHT);	/* 右のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    setlr(n);
}

static void
pp_dr(int x, int y, int w, int n)
    /*
     * 下右のパターンを組み合わせる
     */
{
    findpat(x, y, w, DOWN); 	/* 下のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    findpat(x, y, w, RIGHT);	/* 右のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    setlr(n);
}


static void
pp_ud(int x, int y, int w, int n)
    /*
     * 上下のパターンを組み合わせる
     */
{
    findpat(x, y, w, UP);   	/* 上のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    findpat(x, y, w, DOWN); 	/* 下のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    setlr(n);
}

static void
pp_du(int x, int y, int w, int n)
    /*
     * 下上のパターンを組み合わせる
     */
{
    findpat(x, y, w, DOWN); 	/* 下のパターン読み込み */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    findpat(x, y, w, UP);   	/* 上のパターン読み込み */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    setlr(n);
}

static void
pp_sxm1(int x, int y, int w, int n)
    /*
     * x-1のパターンと比較
     */
{
    switch (n) {
    case P_S0:
    	get4bitpat(x - 1, y, w, &Patb, &Patr, &Patg, &Pati);
    	break;
    case P_SU1:
    	get4bitpat(x - 1, y - 1, w, &Patb, &Patr, &Patg, &Pati);
    	break;
    case P_SU2:
    	get4bitpat(x - 1, y - 2, w, &Patb, &Patr, &Patg, &Pati);
    	break;
    case P_SU3:
    	get4bitpat(x - 1, y - 3, w, &Patb, &Patr, &Patg, &Pati);
    	break;
    case P_SD1:
    	get4bitpat(x - 1, y + 1, w, &Patb, &Patr, &Patg, &Pati);
    	break;
    case P_SD2:
    	get4bitpat(x - 1, y + 2, w, &Patb, &Patr, &Patg, &Pati);
    	break;
    case P_SD3:
    	get4bitpat(x - 1, y + 3, w, &Patb, &Patr, &Patg, &Pati);
    	break;
    default:
    	error(ER_AB);
    }
}


#if 0 /*******************/
void
pass22(int x, int y, int w)
{
    int  n;

    n = getdata();
    switch (n) {
    case P_NON:
    	pp_non(x, y, w, n);
    	break;

    case P_SAMEUP:
    case P_SAMEDOWN:
    case P_SAMELEFT:
    case P_SAMERIGHT:
    	pp_same(x, y, w, n);
    	break;

    case P_SAME1UP:
    	pp_same2(x, y, w, n);
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
    	pp_col(x, y, w, n);
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
    	pp_l2col(x, y, w, n);
    	break;

    case P_LR11:
    	pp_lr(x, y, w, 5);
    	break;
    case P_LR13:
    	pp_lr(x, y, w, 6);
    	break;
    case P_LR31:
    	pp_lr(x, y, w, 9);
    	break;
    case P_LR22:
    	pp_lr(x, y, w, 0xa);
    	break;

    case P_LU11:
    	pp_lu(x, y, w, 5);
    	break;
    case P_LU13:
    	pp_lu(x, y, w, 6);
    	break;
    case P_LU31:
    	pp_lu(x, y, w, 9);
    	break;
    case P_LU22:
    	pp_lu(x, y, w, 0xa);
    	break;

    case P_LD11:
    	pp_ld(x, y, w, 5);
    	break;
    case P_LD13:
    	pp_ld(x, y, w, 6);
    	break;
    case P_LD31:
    	pp_ld(x, y, w, 9);
    	break;
    case P_LD22:
    	pp_ld(x, y, w, 0xa);
    	break;

    case P_UR11:
    	pp_ur(x, y, w, 5);
    	break;
    case P_UR13:
    	pp_ur(x, y, w, 6);
    	break;
    case P_UR31:
    	pp_ur(x, y, w, 9);
    	break;
    case P_UR22:
    	pp_ur(x, y, w, 0xa);
    	break;

    case P_DR11:
    	pp_dr(x, y, w, 5);
    	break;
    case P_DR13:
    	pp_dr(x, y, w, 6);
    	break;
    case P_DR31:
    	pp_dr(x, y, w, 9);
    	break;
    case P_DR22:
    	pp_dr(x, y, w, 0xa);
    	break;

    case P_UD11:
    	pp_ud(x, y, w, 5);
    	break;
    case P_UD13:
    	pp_ud(x, y, w, 6);
    	break;
    case P_UD31:
    	pp_ud(x, y, w, 9);
    	break;
    case P_UD22:
    	pp_ud(x, y, w, 0xa);
    	break;

    case P_DU11:
    	pp_du(x, y, w, 5);
    	break;
    case P_DU13:
    	pp_du(x, y, w, 6);
    	break;
    case P_DU31:
    	pp_du(x, y, w, 9);
    	break;
    case P_DU22:
    	pp_du(x, y, w, 0xa);
    	break;

    case P_S0:
    case P_SU1:
    case P_SU2:
    case P_SU3:
    case P_SD1:
    case P_SD2:
    case P_SD3:
    	pp_sxm1(x, y, w, n);
    	break;

    default:
    	if (n < P_RC)
    	    pp_l1c(x, y, w, n);
    	else if (n < P_UC)
    	    pp_r1c(x, y, w, n);
    	else if (n < P_DC)
    	    pp_u1c(x, y, w, n);
    	else if (n < P_LC + 0x200)
    	    pp_d1c(x, y, w, n);
    	else if (n < P_RC + 0x200)
    	    pp_l1c(x, y, w, n);
    	else if (n < P_UC + 0x200)
    	    pp_r1c(x, y, w, n);
    	else if (n < P_DC + 0x200)
    	    pp_u1c(x, y, w, n);
    	else
    	    pp_d1c(x, y, w, n);
    }
    Lpb = Patb;
    Lpr = Patr;
    Lpg = Patg;
    Lpi = Pati;
}

#else /*******************/

void
pass22(int x, int y, int w)
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
    static void (*func[]) () = {
    	error,
    	pp_non,
    	pp_same,
    	pp_same2,
    	pp_col,
    	pp_lr,
    	pp_lu,
    	pp_ld,
    	pp_ur,
    	pp_dr,
    	pp_ud,
    	pp_du,
    	pp_sxm1,
    	pp_l2col
    };

    int  n, fn, arg, off;

    n = getdata();

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
    	    pp_l1c(x, y, w, n);
    	else if (n < P_UC)
    	    pp_r1c(x, y, w, n);
    	else if (n < P_DC)
    	    pp_u1c(x, y, w, n);
    	else if (n < P_LC + 0x200)
    	    pp_d1c(x, y, w, n);
    	else if (n < P_RC + 0x200)
    	    pp_l1c(x, y, w, n);
    	else if (n < P_UC + 0x200)
    	    pp_r1c(x, y, w, n);
    	else if (n < P_DC + 0x200)
    	    pp_u1c(x, y, w, n);
    	else
    	    pp_d1c(x, y, w, n);
    }
    Lpb = Patb;
    Lpr = Patr;
    Lpg = Patg;
    Lpi = Pati;
}
#endif /******/
