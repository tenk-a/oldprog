/*=============================================================================
 *  VHP Saver  Version 1.21T (CG presser)
 *  	640*200,640*400 �i8/16(analog) colors�j
 *  	    	  PC-9801 , PC-286 or PC88VA
 *
 *  	1989.12 - 1990.04    by �}�V�[���l (Ver.1.11 for PC9801)
 *  	1991.08 - 1992.02    modified by M.Kitamura(�Ă񂩁��ӂ�)
 *- ���� ----------------------------------------------------------------------
 * v1.00   640x200,640x400(8 colors)
 * v1.10   16(analog) colors �ǉ�
 * v1.11   paint()�ċA�Ăт����ł� stack overflow ���N����Ȃ��悤�ɂ���
 *  	   buildhuftree()�� ������Ƃ���������
 * v1.20T  1991/09/01 .rgb��گ�̧�قɑΉ�.88VA�Ή�.Turbo-C,LSI-C�ł̺��߲�.
 * v1.21T  1992/02/06  last modified
 *- �R���p�C�����@ ------------------------------------------------------------
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


/* Saver �ł� ���݂�pass */
#define PASS1	   1
#define SAVEPASS1  2
#define PASS2	   3
#define SAVEPASS2  4

/* Saver �� �r�b�g�}�b�v�f�[�^�̎��ʎq */
#define V_TYPE42   1
#define V_TYPE44   2
#define V_SAME42   4
#define V_PAINT    (V_TYPE42|V_TYPE44|V_SAME42)
#define V_NON	   8
#define V_PAINTED 16
#define V_TBMASK  32

#define vof(x,y) (LMAXX*(y)+(x))

/*---------------------- �ϐ��錾 ----------------------*/

/* �p�^�[���e�[�u�� */
static byte Pattb[MAXTBSIZE],
    	    Pattr[MAXTBSIZE],
    	    Pattg[MAXTBSIZE],
    	    Patti[MAXTBSIZE];
static word Pattptr = 1;      /* �����ð��ق̈ʒu�iPatt?[0]�͍��ɌŒ�j*/
static word Tbsize, Tbmask, Tbb;

/* �r�b�g�}�b�v�e�[�u�� */
static byte Vbuf[LMAXY][LMAXX];

/* RGBI �f�[�^ */
static byte Npatb, Npatr, Npatg, Npati;
static byte Lpatb, Lpatr, Lpatg, Lpati;
static byte Rpatb, Rpatr, Rpatg, Rpati;
static byte Patb, Patr, Patg, Pati;

/* �Ō�Ɏg�����p�^�[����2�F��ޔ�(pass2) */
static int  Lcol1, Lcol2;
static int  Ucol1, Ucol2;

/* �A�i���O */
static int  Analog;

/* �y�C���g */
static int  Ctnest = 0;
static int  Px, Py;
static int  Offx, Offy;
static int  Ptype, Pvector;
static word P2mask;

/* pass2 �e���ʎq�̐� */
static word Ctp2[MAXP2TB];
static word P2tbbit;

/* �n�t�}���������p */
struct HUF {
    word num, count, treenum, code, codenum;
};
static struct HUF Hbuf[MAXHUF];

static word Cthbuf = 0;

/* �f�[�^�o�� */
static FILE *Fpw;

/* �Z�[�u�t�@�C���� */
static byte Savename[80 + 1];

/* .rgb|.alg�t�@�C���� */
static byte Algname[80 + 1];

/* ���݂�pass */
static int  Pass;


/*-------------------------------------------------------------------------*/
void
extcat(byte oldfname[], byte *ext, byte newfname[])
    	/* �g���q��ύX���� */
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
 * �f�[�^�o�̓��[�`��
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
 *  ����ɂ��铯���p�^�[���̐���Ԃ�
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
  /* ���łɃy�C���g����Ă��邩�A�p�^�[�����قȂ�΃��^�[������ */
  /* Ctnest �� MAXNEST ��������΃��^�[������ */
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
 *  ����S�āi�㉺���E�j�ɁA�����p�^�[�������݂��鎞�y�C���g����i4*2 or 4*4�j
 *�@�i���̕����͂Ȃ��Ă��A���܂�Ȃ��B���k���ɑ����e����^����B�j
 */
    for (y = 1; y < Ly - 1; ++y) {
    	for (x = 1; x < LMAXX - 1; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*�@�p�^�[���̓ǂݍ��݁@*/
    	    settype();	    	/*�@�p�^�[���^�C�v�̃Z�b�g�@*/
    	    if (Ptype == V_TYPE44) {
    	    	if (getflg(x, y))
    	    	    continue;	/* check Vbuf[y+1][x] */
    	    }
    	    if (checkr(x, y) == 4) {
    	    	Px = x;
    	    	Py = y;
    	    	paint();    	/*�@�y�C���g�J�n�@*/
    	    }
    	}
    }

/*
 *�@����Ɉ�ł������p�^�[�������݂��鎞�y�C���g����i4*2 or 4*4�j
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*�@�p�^�[���̓ǂݍ��݁@*/
    	    settype();	    	/*�@�p�^�[���^�C�v�̃Z�b�g�@*/
    	    if (Ptype == V_TYPE44) {
    	    	if (getflg(x, y))
    	    	    continue;	/* check Vbuf[y+1][x] */
    	    	if (y == Ly - 1)
    	    	    continue;	/* (X,Ly)�����k���Ă��܂��̂� */
    	    }
    	    if (checkr(x, y) > 0) {
    	    	Px = x;
    	    	Py = y;
    	    	paint();    	/*�@�y�C���g�J�n�@*/
    	    }
    	}
    }

/*
 *�@����Ɉ�ł������p�^�[�������݂��鎞�y�C���g����i4*2�j
 *  �i�h�c�����y�C���g����j
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*�@�p�^�[���̓ǂݍ��݁@*/
    	    Ptype = V_TYPE42;
    	    Offx = 1;
    	    Offy = 1;	    	/* �p�^�[���^�C�v���S���Q�ɃZ�b�g */
    	    if (checkr(x, y) > 0) {
    	    	Px = x;
    	    	Py = y;
    	    	paint();
    	    }
    	}
    }

/*
 * �p�^�[���e�[�u���̍쐬
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
    	    getpat(x, y);   	/*�@�p�^�[���̓ǂݍ��݁@*/
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
 *  �Ɨ����Ă���i����ɓ����p�^�[�����Ȃ��j4*2�p�^�[���ŁA
 *�@�p�^�[���e�[�u���ɂ��̃p�^�[�������݂��鎞
 */
    for (y = 0; y < Ly; ++y) {
    	for (x = 0; x < LMAXX; ++x) {
    	    if (Vbuf[y][x])
    	    	continue;
    	    getpat(x, y);   	/*�@�p�^�[���̓ǂݍ��݁@*/
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
 * �n�t�}��������
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

       /* �c���[�ԍ��Z�b�g */
    	tn1 = Hbuf[k - 1].treenum;
    	tn2 = Hbuf[k].treenum;
    	if (tn1 == 0 && tn2 == 0) {
    	    Hbuf[k - 1].treenum = tnum;
    	    Hbuf[k].treenum = tnum;
    	    ++tnum;
    	} else if (tn1 > tn2) {
    	    Hbuf[k].treenum = tn1;  	/* �c���[�ԍ��̍X�V */
    	    for (i = k + 1; i < Cthbuf; ++i) {
    	    	if (tn2 == Hbuf[i].treenum)
    	    	    Hbuf[i].treenum = tn1;
    	    }
    	} else {
    	    Hbuf[k - 1].treenum = tn2;	/* �c���[�ԍ��̍X�V */
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

    putbits(0x04, Tbb); /* �p�^�[���e�[�u���̍ő�I�t�Z�b�g�r�b�g�� �o�� */

    memset(ctbtbuf, 0, sizeof(ctbtbuf));
/*
 * �r�b�g�}�b�v�e�[�u���f�[�^���S�r�b�g(0-0xf)���Ƃɋ�؂�
 *  �i�n�t�}���������̂��߁j
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
 *�@�e�[�u���ɂd�n�q��������
 */
    for (y = 0; y < Ly - 1; ++y) {
    	for (x = 0; x < LMAXX / 8; ++x) {
    	    btbuf[y][x] = btbuf[y][x] ^ btbuf[y + 1][x];
    	}
    }
/*
 *  �n�t�}��������
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
 *�@�����̃Z�[�u
 */
    for (i = 0; i < 0x10; ++i) {/*  ���������o�� */
    	putbits(0x08, Hbuf[i].codenum);
    }
    for (i = 0; i < 0x10; ++i) {/*  �������o�� */
    	putbits(1 << (Hbuf[i].codenum - 1), Hbuf[i].code);
    }
/*
 *�@�r�b�g�}�b�v�f�[�^�̃Z�[�u
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
 *  �p�^�[���f�[�^�̃Z�[�u
 *  A. 0xxx xxxx    	    	    	  - V_TYPE42 found pattern
 *  B. 10bb bbbb bbrr rrrr rrgg gggg gg   - V_TYPE42 not found pattern
 *  C. 11   	    	    	    	  - V_TYPE44
 *  D.	 0x xxxx xx 	    	    	  -   found pattern
 *  E.	 1b bbbb bbbr rrrr rrrg gggg ggg  -   not found pattern
 *  F. xxxx xxx     	    	    	  - V_SAME42 (���ʎq���Ȃ��Ă����ʉ\)
 *
 *     b - blue  �p�^�[�� 8bit(4*2)
 *     r - red	 �p�^�[�� 8bit(4*2)
 *     g - green �p�^�[�� 8bit(4*2)
 *     x - �p�^�[���e�[�u���̈ʒu 1-7bit�i7bit�Œ�ł͂Ȃ��j
 */
    P2mask = V_PAINTED;     	/* paint2 �ł̃}�X�N�r�b�g�̐ݒ� */
    Pattptr = 1;    	    	/* �p�^�[���e�[�u���̈ʒu���������@*/
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
    	    	   /*�@4*2�ŁA�e�[�u���ɂ��̃p�^�[�������鎞�@*/
    	    	    putbit(0);	/* ���ʎq�o�� */
    	    	    putbits(Tbmask, find);
    	    	} else {
    	    	   /*�@4*2�ŁA�e�[�u���ɂ��̃p�^�[�����Ȃ����@*/
    	    	    tbsetpat(Patb1, Patr1, Patg1, Pati1);
    	    	    putbit(1);
    	    	    putbit(0);	/* ���ʎq�o�� */
    	    	    putbits(0x80, Patb1);
    	    	    putbits(0x80, Patr1);
    	    	    putbits(0x80, Patg1);
    	    	    if (Color16)
    	    	    	putbits(0x80, Pati1);
    	    	}
    	    	Px = x;
    	    	Py = y;
    	    	paintbmt(); 	/* �r�b�g�}�b�v�e�[�u������y�C���g����@*/

    	    } else if (Vbuf[y][x] & V_TYPE44) {
    	    	putbit(1);
    	    	putbit(1);  	/* ���ʎq�o�� */
    	    	getpat(x, y);
    	    	find = tbcmppat(Patb1, Patr1, Patg1, Pati1);
    	    	if (find != -1) {
    	    	   /*�@4*4�̏㔼�� 4*2�ŁA�e�[�u���ɂ��̃p�^�[�������鎞�@*/
    	    	    putbit(0);	    	   /* ���ʎq�o�� */
    	    	    putbits(Tbmask, find); /* �e�[�u���̈ʒu�o�� */
    	    	} else {
    	    	   /*  4*4�̏㔼�� 4*2�ŁA�e�[�u���ɂ��̃p�^�[�����Ȃ���  */
    	    	    tbsetpat(Patb1, Patr1, Patg1, Pati1); /*����݂�ð��قɾ��*/
    	    	    putbit(1);	    	   /* ���ʎq�o�� */
    	    	    putbits(0x80, Patb1);  /* �p�^�[���o�� */
    	    	    putbits(0x80, Patr1);
    	    	    putbits(0x80, Patg1);
    	    	    if (Color16)
    	    	    	putbits(0x80, Pati1);
    	    	}

    	    	find = tbcmppat(Patb2, Patr2, Patg2, Pati2);
    	    	if (find != -1) {
    	    	   /*�@4*4�̉����� 4*2�ŁA�e�[�u���ɂ��̃p�^�[�������鎞�@*/
    	    	    putbit(0);	/* ���ʎq�o�� */
    	    	    putbits(Tbmask, find);  	/* �e�[�u���̈ʒu�o�� */
    	    	} else {
    	    	   /*�@4*4�̉����� 4*2�ŁA�e�[�u���ɂ��̃p�^�[�����Ȃ����@*/
    	    	    tbsetpat(Patb2, Patr2, Patg2, Pati2); /*����݂�ð��قɾ��*/
    	    	    putbit(1);	/* ���ʎq�o�� */
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
 * �㉺���E�̍ŏ��Ɍ�����y�C���g�p�^�[�����T�[�`
 *---------------------------------------------------
 * in : x,y ���݂̍��W
 *  	w   high(=1) or low(=0)  4 bits
 *  	v   �T�[�`���� (UP, DOWN, LEFT, RIGHT)
 * out: return value = 0 - �p�^�[����������Ȃ�����
 *  	    	       1 - �p�^�[����������
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
 * �㉺���E�œ����p�^�[����T��
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
 * ���̃p�^�[���Ɣ�ׂ�
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
 * ���݂̃p�^�[����Pat?�̃p�^�[���ŁA�قȂ�F����F���ǂ������m���߂�
 *--------------------------------------------------------------------
 * return value  = 0  ��F�ł͂Ȃ�
 *  	    	!= 0  ��F(XXXX XXiX  Xgrb pppp)
 *  	    	    	igrb �F
 *  	    	    	pppp �p�^�[��
 */
int
cmp1c(void)
{
    byte pat, patb, patr, patg, pati;
    int  retval;

   /* �p�^�[���ƈقȂ�F�̃r�b�g�𒊏o */
    pat = (Patb ^ Npatb) | (Patr ^ Npatr) | (Patg ^ Npatg);
    if (Color16)
    	pat |= (Pati ^ Npati);

    if (pat == 0 || pat == 0xf)
    	return 0;

    patb = pat & Npatb;     	/* �v���[�����ƂɈقȂ�F�𒊏o */
    patr = pat & Npatr;
    patg = pat & Npatg;
    pati = pat & Npati;

   /* patb,r,g �́A���ׂ�pat�Ɠ����A�܂��� 0�łȂ���΂Ȃ�Ȃ� */
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
 * ���̃p�^�[���Ƒ��̂P�J���[
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
 * �E�̃p�^�[���Ƒ��̂P�J���[
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
 * ��̃p�^�[���Ƒ��̂P�J���[
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
 * ���̃p�^�[���Ƒ��̂P�J���[
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
 * ��F�̃u���b�N
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
 *�@��̃p�^�[���� �͂��܂�Ă���p�^�[�����ǂ����𒲂ׂ�
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

/* ���E�ƈقȂ�p�^�[���𒊏o */
    pat = Npatb & (0xf >> llen) & (0xf << rlen);
    pat |= Npatr & (0xf >> llen) & (0xf << rlen);
    pat |= Npatg & (0xf >> llen) & (0xf << rlen);
    if (Color16)
    	pat |= Npati & (0xf >> llen) & (0xf << rlen);

    if (pat)
    	return 0;   	    	/* ���ȊO�̃p�^�[���͖��� */

    return code[(llen << 2) | (rlen)];	/* return value = 1,2,3,4 */
}

/*--------------------------------
 * ���|�E�@�̃p�^�[����g�ݍ��킹��
 *--------------------------------
 */
int
pp_lr(int x, int y, int w)
{
    static int code[] = {0, P_LR11, P_LR13, P_LR31, P_LR22};

    if (!findpat(x, y, w, LEFT))
    	return 0;   	    	/* ���̃p�^�[���ǂݍ��� */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, RIGHT))
    	return 0;   	    	/* �E�̃p�^�[���ǂݍ��� */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * ���|��@�̃p�^�[����g�ݍ��킹��
 *--------------------------------
 */
int
pp_lu(int x, int y, int w)
{
    static int code[] = {0, P_LU11, P_LU13, P_LU31, P_LU22};

    if (!findpat(x, y, w, LEFT))
    	return 0;   	    	/* ���̃p�^�[���ǂݍ��� */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* ��̃p�^�[���ǂݍ��� */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * ���|���@�̃p�^�[����g�ݍ��킹��
 *--------------------------------
 */
int
pp_ld(int x, int y, int w)
{
    static int code[] = {0, P_LD11, P_LD13, P_LD31, P_LD22};

    if (!findpat(x, y, w, LEFT))
    	return 0;   	    	/* ���̃p�^�[���ǂݍ��� */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* ���̃p�^�[���ǂݍ��� */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * ��|�E�@�̃p�^�[����g�ݍ��킹��
 *--------------------------------
 */
int
pp_ur(int x, int y, int w)
{
    static int code[] = {0, P_UR11, P_UR13, P_UR31, P_UR22};

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* ��̃p�^�[���ǂݍ��� */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, RIGHT))
    	return 0;   	    	/* �E�̃p�^�[���ǂݍ��� */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * ���|�E�@�̃p�^�[����g�ݍ��킹��
 *--------------------------------
 */
int
pp_dr(int x, int y, int w)
{
    static int code[] = {0, P_DR11, P_DR13, P_DR31, P_DR22};

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* ���̃p�^�[���ǂݍ��� */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, RIGHT))
    	return 0;   	    	/* �E�̃p�^�[���ǂݍ��� */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}


/*-------------------------------
 * ��|���@�̃p�^�[����g�ݍ��킹��
 *-------------------------------
 */
int
pp_ud(int x, int y, int w)
{
    static int code[] = {0, P_UD11, P_UD13, P_UD31, P_UD22};

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* ��̃p�^�[���ǂݍ��� */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* ���̃p�^�[���ǂݍ��� */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * ���|��@�̃p�^�[����g�ݍ��킹��
 *--------------------------------
 */
int
pp_du(int x, int y, int w)
{
    static int code[] = {0, P_DU11, P_DU13, P_DU31, P_DU22};

    if (!findpat(x, y, w, DOWN))
    	return 0;   	    	/* ���̃p�^�[���ǂݍ��� */
    Lpatb = Patb;
    Lpatr = Patr;
    Lpatg = Patg;
    Lpati = Pati;

    if (!findpat(x, y, w, UP))
    	return 0;   	    	/* ��̃p�^�[���ǂݍ��� */
    Rpatb = Patb;
    Rpatr = Patr;
    Rpatg = Patg;
    Rpati = Pati;

    return code[cmplr()];
}

/*--------------------------------
 * ���W x-1 �̃p�^�[���Ɣ�r
 *--------------------------------
 */
int
pp_sxm1(int x, int y, int w)
{
    if (x == 0)
    	return 0;
/*  �i�w�|�P�C�x�j */
    get4bitpat(x - 1, y, w, &Patb, &Patr, &Patg, &Pati);
    if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	(!Color16 || Npati == Pati)) {
    	return P_S0;
    }
/*  �i�w�|�P�C�x�|�P�j */
    if (y > 0) {
    	get4bitpat(x - 1, y - 1, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SU1;
    	}
    }
/*  �i�w�|�P�C�x�|�Q�j */
    if (y > 1) {
    	get4bitpat(x - 1, y - 2, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SU2;
    	}
    }
/*  �i�w�|�P�C�x�|�R�j */
    if (y > 3) {
    	get4bitpat(x - 1, y - 3, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SU3;
    	}
    }
/*  �i�w�|�P�C�x�{�P�j */
    if (y < Ly - 1) {
    	get4bitpat(x - 1, y + 1, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SD1;
    	}
    }
/*  �i�w�|�P�C�x�{�Q�j */
    if (y < Ly - 2) {
    	get4bitpat(x - 1, y + 2, w, &Patb, &Patr, &Patg, &Pati);
    	if (Npatb == Patb && Npatr == Patr && Npatg == Patg &&
    	    (!Color16 || Npati == Pati)) {
    	    return P_SD2;
    	}
    }
/*  �i�w�|�P�C�x�{�R�j */
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
 *  �ŏ��Ɍ������Q�F��Ԃ�
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
    return (Ucol1<<4) | Ucol2;/* 2�F�ȏ�łȂ���΁A�Ō�Ɏg�p����2�F��Ԃ� */
}


/*-----------------
 * ���k�֐�
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
    else if ((i = pp_sxm1(x, y, w) ) != 0); 	/* x-1�ɓ����p�^�[�������邩?*/
    else if ((i = pp_u1c(x, y, w)  ) != 0);
    else if ((i = pp_d1c(x, y, w)  ) != 0);
    else if ((i = pp_lu(x, y, w)   ) != 0);
    else if ((i = pp_ld(x, y, w)   ) != 0);
    else if ((i = pp_ur(x, y, w)   ) != 0);
    else if ((i = pp_dr(x, y, w)   ) != 0);
    else if ((i = pp_ud(x, y, w)   ) != 0);
    else if ((i = pp_du(x, y, w)   ) != 0);

    col = get2color();	    	/* ���݂̃p�^�[���̂���2�F���o���Ă��� */
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
    for (i = 1; i < MAXP2TB; ++i) {/* "1"����n�߂�("0"�̔񈳏k�����͏���) */
    	if (Ctp2[i] >= 6) {
    	    sethuf(i, (word) Ctp2[i]);
    	} else {
    	    Ctp2[0] += Ctp2[i]; /*  ���k�ł��Ȃ���������P_NON�ɉ�����@*/
    	    Ctp2[i] = 0;
    	}
    }
    sethuf(0, (word) Ctp2[0]);	/* �񈳏k�������Z�b�g */

    buildhuftree();
    sorthufnum();

   /*  �������̃Z�[�u */
    for (i = 0; i < Cthbuf; ++i) {
    	putbits(1 << (P2tbbit - 1), Hbuf[i].num);
    	putbits(0x8, Hbuf[i].codenum);
    	putbits(1 << (Hbuf[i].codenum - 1), Hbuf[i].code);
    }
    putbits(1 << (P2tbbit - 1), 0); 	/* �G���h�R�[�h */

    for (i = Cthbuf - 1; i >= 0; --i) { /*�@�����ϊ��z��̕��בウ */
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
 * in : h  - �F��     (0-360)
 *  	s  - �F��     (0-100)
 *  	v  - �ő喾�x (0-100)
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

    putbits(0x80, attr);    	/* �����o�� */

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
    puts("VHP Saver	Ver.1.21T for PC9801  by ϼ��M(Ver.1.11)");
#else
    puts("VHP Saver	Ver.1.21T for PC88VA  by ϼ��M(Ver.1.11	for	PC9801)");
#endif
    puts("                       Modified by �ݶ*��");
    puts("Usage: vhsave	[-{<sw>}] <pathname1>[.VHP]	[<pathname2>[.RGB|.ALG]] [-{<sw>}]");
    puts("<sw>:	?, 2|4,	6|8|a");
    puts("  ?    �w���v�\��");
    puts("  2    �Q�O�O���C�� �i��̫�āj");
    puts("  4    �S�O�O���C��");
    puts("  6    �P�U�F�f�W�^��");
    puts("  8      �W�F�f�W�^�� �i��̫�āj");
    puts("  a    �P�U�F�A�i���O �i<pathname1>.RGB|.ALG ���K�v�ł��j\n");
    puts(" <pathname2>[.RGB|.ALG]���w�肷��ƂP�U�F�A�i���O�ɂȂ�܂�");
    puts("                          �i<sw> 6|8|a �w��͖�������܂��j");

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
 * ���C���֐�
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
