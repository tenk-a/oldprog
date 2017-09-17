/*===========================================================================*/
/*  KTY_PLUG	    	    	    	    	    	    	    	     */
/*===========================================================================*/

#include "tofmt4p.c"


/*---------------------------------------------------------------------------*/
/*-----    �w  ��  �v	 ----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Usage(void)
{
    puts(
    	"kty_plug ver1.00                                  by Tenka* 1995\n"
    	"usage: kty_plug [-a] <COMMAND> <inputfile> <outputfile>\n"
    	" <COMMAND> .. DJ505JC, TO_DJP or TO_BMP.\n"
    	"              DJ505JC��DJP�����ŁA640*200�摜��640*400�摜�ɕϊ�\n"
    	"              ����ȊO�̏ꍇ�́A�h�b�g��̒����͍s���܂���\n"
    	" -a ......... DJ505JC�ȊO�ł� 640*200�摜�� 640*400 �ɕϊ����܂�\n"
    	);
    exit(1);
}



/*---------------------------------------------------------------------------*/
/*-----    �� �� �� �[ �` ��   ----------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
    �j�s�x�t�H�[�}�b�g�o������


    Kitty �t�H�[�}�b�g�́A�f�W�^���W�F�A�T�C�Y 640*200 �܂��� 640*400 �̉摜
    �ł��B
    �t�H�[�}�b�g��҂́ARabbit ��.
 */

#define VB  0
#define VR  1
#define VG  2
#define VE  3
static UCHAR FAR *KTY_g[4]; 	    	/* VRAM 4��ڰ݂ւ��߲��     	     */
static UCHAR KTY_t[4][4];   	    	/* 4*2 or 4*4 ����٥����݂��i�[      */
    	    	    	    	    	/* ���ʒu�΍�ŏ�4bit��4bit�͓����l*/
static UCHAR KTY_l4flg = 0; 	    	/* 0:200L�摜 �ȊO:400L�摜 	     */
static FILE  *KTY_fp;	    	    	/* ���͂�FILE Pointer	    	     */
static UCHAR KTY_markTbl[100][160]; 	/* �\���ψʒu��ϰ����邽�߂�ð���    */
#define KTY_SetMark(x,y)    (KTY_markTbl[y][x] = 1)
#define KTY_GetMark(x,y)    (KTY_markTbl[y][x])

static UCHAR KTY_rgb[16*3] = {
    	    0x00, 0x00, 0x00,
    	    0x00, 0x00, 0xF0,
    	    0xF0, 0x00, 0x00,
    	    0xF0, 0x00, 0xF0,
    	    0x00, 0xF0, 0x00,
    	    0x00, 0xF0, 0xF0,
    	    0xF0, 0xF0, 0x00,
    	    0xF0, 0xF0, 0xF0,
    	    0x00, 0x00, 0x00,
    	    0x00, 0x00, 0xF0,
    	    0xF0, 0x00, 0x00,
    	    0xF0, 0x00, 0xF0,
    	    0x00, 0xF0, 0x00,
    	    0x00, 0xF0, 0xF0,
    	    0xF0, 0xF0, 0x00,
    	    0xF0, 0xF0, 0xF0,
    	};


static UINT KTY_GetB(void)
{
    return (UINT)fgetc(KTY_fp);
}

static void KTY_GetTile(void)
    /* 4*2 �܂��� 4*4 �̃^�C�����t�@�C�������� */
{
    UCHAR a;

    a = (UCHAR)KTY_GetB();
    KTY_t[VB][2] = KTY_t[VB][0] = (a & 0xF0)|(a>>4);
    KTY_t[VB][3] = KTY_t[VB][1] = (a & 0x0F)|(a<<4);
    a = (UCHAR)KTY_GetB();
    KTY_t[VR][2] = KTY_t[VR][0] = (a & 0xF0)|(a>>4);
    KTY_t[VR][3] = KTY_t[VR][1] = (a & 0x0F)|(a<<4);
    a = (UCHAR)KTY_GetB();
    KTY_t[VG][2] = KTY_t[VG][0] = (a & 0xF0)|(a>>4);
    KTY_t[VG][3] = KTY_t[VG][1] = (a & 0x0F)|(a<<4);
    /* KTY_t[VE][2] = KTY_t[VE][0] = 0; */
    /* KTY_t[VE][3] = KTY_t[VE][1] = 0; */

    if (KTY_l4flg > 1) {
    	a = (UCHAR)KTY_GetB();
    	KTY_t[VB][2] = (a & 0xF0)|(a>>4);
    	KTY_t[VB][3] = (a & 0x0F)|(a<<4);
    	a = (UCHAR)KTY_GetB();
    	KTY_t[VR][2] = (a & 0xF0)|(a>>4);
    	KTY_t[VR][3] = (a & 0x0F)|(a<<4);
    	a = (UCHAR)KTY_GetB();
    	KTY_t[VG][2] = (a & 0xF0)|(a>>4);
    	KTY_t[VG][3] = (a & 0x0F)|(a<<4);
    	/* KTY_t[VE][2] = KTY_t[VE][0] = 0; */
    	/* KTY_t[VE][3] = KTY_t[VE][1] = 0; */
    }
}

static void KTY_PutTile(int x, int y)
    /* ���݂̃^�C���E�p�^�[���� (x,y)[x:0�`159,y:0�`99]�ɕ\�� */
{
    UINT o;

    o = y*80*2;
    if (KTY_l4flg)
    	o *= 2;
    o += (x >> 1);
    if ((x & 1) == 0) {     /* �����̂Ƃ� */
    	KTY_g[VB][o] = (KTY_t[VB][0] & 0xF0) | (KTY_g[VB][o] & 0x0F);
    	KTY_g[VR][o] = (KTY_t[VR][0] & 0xF0) | (KTY_g[VR][o] & 0x0F);
    	KTY_g[VG][o] = (KTY_t[VG][0] & 0xF0) | (KTY_g[VG][o] & 0x0F);
    	o += 80;
    	KTY_g[VB][o] = (KTY_t[VB][1] & 0xF0) | (KTY_g[VB][o] & 0x0F);
    	KTY_g[VR][o] = (KTY_t[VR][1] & 0xF0) | (KTY_g[VR][o] & 0x0F);
    	KTY_g[VG][o] = (KTY_t[VG][1] & 0xF0) | (KTY_g[VG][o] & 0x0F);
    	if (KTY_l4flg) {
    	    o += 80;
    	    KTY_g[VB][o] = (KTY_t[VB][2] & 0xF0) | (KTY_g[VB][o] & 0x0F);
    	    KTY_g[VR][o] = (KTY_t[VR][2] & 0xF0) | (KTY_g[VR][o] & 0x0F);
    	    KTY_g[VG][o] = (KTY_t[VG][2] & 0xF0) | (KTY_g[VG][o] & 0x0F);
    	    o += 80;
    	    KTY_g[VB][o] = (KTY_t[VB][3] & 0xF0) | (KTY_g[VB][o] & 0x0F);
    	    KTY_g[VR][o] = (KTY_t[VR][3] & 0xF0) | (KTY_g[VR][o] & 0x0F);
    	    KTY_g[VG][o] = (KTY_t[VG][3] & 0xF0) | (KTY_g[VG][o] & 0x0F);
    	}
    } else {	    	/* ��̂Ƃ� */
    	KTY_g[VB][o] = (KTY_t[VB][0] & 0x0F) | (KTY_g[VB][o] & 0xF0);
    	KTY_g[VR][o] = (KTY_t[VR][0] & 0x0F) | (KTY_g[VR][o] & 0xF0);
    	KTY_g[VG][o] = (KTY_t[VG][0] & 0x0F) | (KTY_g[VG][o] & 0xF0);
    	o += 80;
    	KTY_g[VB][o] = (KTY_t[VB][1] & 0x0F) | (KTY_g[VB][o] & 0xF0);
    	KTY_g[VR][o] = (KTY_t[VR][1] & 0x0F) | (KTY_g[VR][o] & 0xF0);
    	KTY_g[VG][o] = (KTY_t[VG][1] & 0x0F) | (KTY_g[VG][o] & 0xF0);
    	if (KTY_l4flg) {
    	    o += 80;
    	    KTY_g[VB][o] = (KTY_t[VB][2] & 0x0F) | (KTY_g[VB][o] & 0xF0);
    	    KTY_g[VR][o] = (KTY_t[VR][2] & 0x0F) | (KTY_g[VR][o] & 0xF0);
    	    KTY_g[VG][o] = (KTY_t[VG][2] & 0x0F) | (KTY_g[VG][o] & 0xF0);
    	    o += 80;
    	    KTY_g[VB][o] = (KTY_t[VB][3] & 0x0F) | (KTY_g[VB][o] & 0xF0);
    	    KTY_g[VR][o] = (KTY_t[VR][3] & 0x0F) | (KTY_g[VR][o] & 0xF0);
    	    KTY_g[VG][o] = (KTY_t[VG][3] & 0x0F) | (KTY_g[VG][o] & 0xF0);
    	}
    }
}


static void KTY_Decode(void)
{
    UINT  d,x,y;
    UCHAR b;

    /* init. */
    memset(KTY_markTbl, 0, sizeof(KTY_markTbl));

    for (; ;) {
    	b = (UCHAR)KTY_GetB();
    	if (b == 255)
    	    break;
    	KTY_l4flg = b;
    	    	    	    	    D((b>3)?printf("����:f=%d\n",KTY_l4flg):0);
    	KTY_GetTile();
    	for (; ;) {
    	    UINT  xs,ys,xe,ye;
    	    b = (UCHAR)KTY_GetB();
    	    if (b == 255)
    	    	break;
    	    d = (UINT)b << 8;
    	    d |= KTY_GetB();
    	    d &= 0x3fff;
    	    ys = d / 160;
    	    xs = d % 160;
    	    	    	    	    D ((ys>99)?printf("����:ys=%d\n",ys):0);
    	    if (b & 0x80) {
    	    	xe = xs;
    	    	ye = KTY_GetB();    D ((ye>99)?printf("����:ye=%d\n",ye):0);
    	    } else if (b & 0x40) {
    	    	ye = ys;
    	    	xe = KTY_GetB();    D ((xe>159)?printf("����:xe=%d\n",xe):0);
    	    } else {
    	    	xe = KTY_GetB();    D ((xe>159)?printf("����:xe2=%d\n",xe):0);
    	    	ye = KTY_GetB();    D ((ye>99)?printf("����:ye2=%d\n",ye):0);
    	    }
    	    for (y = ys; y <= ye; y++) {
    	    	for (x = xs; x <= xe; x++) {
    	    	    KTY_SetMark(x,y);
    	    	    KTY_PutTile(x,y);
    	    	}
    	    }
    	}
    	for (; ;) {
    	    x = KTY_GetB();
    	    if (x == 255)
    	    	break;
    	    	    	    	    D ((x>159)?printf("DATA����x=%d\n",x):0);
    	    y = KTY_GetB(); 	    D ((y>99)?printf("DATA����y=%d\n",y):0);
    	    KTY_SetMark(x,y);
    	    KTY_PutTile(x,y);
    	}
    }

    if (KTY_l4flg)
    	KTY_l4flg = 2;
    for (y = 0; y < 100; y++) {
    	for (x = 0; x < 160; x++) {
    	    if (KTY_GetMark(x,y) == 0) {
    	    	KTY_GetTile();
    	    	KTY_PutTile(x,y);
    	    }
    	}
    }
}


int  GLoad(TOFMT4P *t, char *name)
{
    KTY_g[VB] = t->v[0];
    KTY_g[VR] = t->v[1];
    KTY_g[VG] = t->v[2];
    KTY_g[VE] = t->v[3];
    KTY_fp = fopen(name,"rb");
    if (KTY_fp == NULL) {
    	printf("[KTY-DECODER] Unable to open file %s\n",name); return -1;
    }
    if( setvbuf(KTY_fp, NULL, _IOFBF, VBUFSIZE) ) {
    	printf("[KTY-DECODER] Out of memory.\n"); return -1;
    }

    KTY_Decode();

    fclose(KTY_fp);

    t->xsz = 640;
    if (KTY_l4flg) {
    	t->ysz = 400;
    } else {
    	t->ysz = 200;
    	t->xasp = 1;
    	t->yasp = 2;
    }
    memcpy(t->rgb, KTY_rgb, 16*3);
    return 0;
}
