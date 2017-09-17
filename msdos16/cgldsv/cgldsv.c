/*
	CgLdSv	�����Z�[�u�p�摜�R���o�[�g�x���v���O����
	ver. 1.00  92/08/26  writen by �Ă񂩁�
	ver. 1.01  92/09/14  �w�i�F�łȂ�����ʂƔ�r���郂�[�h��t��
	ver. 1.02  92/10/04  pihdr �𗘗p���� mag->pi ��ǉ�(-m)
						 -lm,-lp,-lq,-lg �ŁA�g���q���Ȃ��Ďw��ł���悤�ɂ���
	ver. 1.10  92/10/04  640*800(-wy),1280*400(-wx) �Ή�
	ver. 1.11  92/10/08  -v:�Z�[�o�����[�_���N�������A�o�b�`�E�t�@�C���𐶐�.
    ver. 1.12  92/10/17  -l-OPT -s-OPT ��t��.
	ver. 1.13  92/10/27  ���C���h�J�[�h�@�\���� ^c ���O��. q4۰�ނ�hsx ��.
	                     @FILE �ł̃I�v�V�����A�t�@�C�����ǂݍ���
    ver. 1.13b 93/06/04  -sp �� pi�Z�[�o�� pi.exe ��.
	ver. 1.14  93/07/25  -d��̧�ٓ��t������. ��̫�Ă𕔕����ނłȂ�1��ʾ��ނ�.
*/

#define CGLDSV  	"�b���k���r��  v1.14"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <signal.h>
#include <conio.h>

#if 0
	#define NOWILDC
	/*TC�ȊO�͂�����`���邱��... �ł��Aqc ,msc �͈���������������������
	  �������̂ł킴�킴������`����K�v�Ȃ��Ǝv��... �v��������^^; */
#endif
#if	0	/* ���C�u������ stpcpy ���Ȃ��Ƃ��A�Z�b�g... MSC,QC �������H */
	#define STPCPY
#endif

#define PILD		"DoPI"
#define PILD_URAOPT "-lb"
#define Q4LD		"hsx"
#define PISV		"pi"
#define PISV_L2OPT	"-2"
#define PIHDR   	"PiHdr"
#if 1	/* mag */
#define MAGLD		"mag"
#define MAGLD_URAOPT "-l8"
#define MAGSV		"mag"
#define MAGSV_L2OPT "-2"
#else   /* magd */
#define MAGLD		"Magd"
#define MAGLD_URAOPT "-lw"
#define MAGSV		"Magdsave"
#define MAGSV_L2OPT "-f2"
#endif

/*---------------------------------------------------------------------------*/
#define _S static
typedef unsigned char byte;
typedef byte far *byte_fp;
typedef unsigned		word;
typedef unsigned long	dword;
#ifndef	toascii	/* ctype.h ��include����Ă��Ȃ��Ƃ� */
  #define toupper(c)  (	((c) >=	'a'	&& (c) <= 'z') ? (c) - 0x20	: (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <=	'9')
#endif
#define pr(s)	printf(s)

/*---------------------------------------------------------------------------*/
extern DirEntryGet(byte far *,byte far *,unsigned);

int gBtmLine = 399;
int gDifFlg,gWidFlg,gBatFlg;
int gBakCol,gPxSiz,gChk,gClsCol,gMsgFlg,gPiHdrFlg;
word gFdateFlg,gFdate,gFtime;
byte *gLoader = "";
byte *gLExt = NULL;
byte *gLUraOpt = "";
byte *gSaver = PISV;
byte *gSExt = NULL;
byte *gL2Opt = PISV_L2OPT;
FILE *gBatFp;
#define LDOPTSCNT	10
#define SVOPTSCNT	10
int  gLdOptsCnt,gSvOptsCnt;
byte *gLdOpts[LDOPTSCNT];
byte *gSvOpts[SVOPTSCNT];

byte_fp gVp[] = {
	(byte_fp)0xA8000000L,
	(byte_fp)0xB0000000L,
	(byte_fp)0xB8000000L,
	(byte_fp)0xE0000000L
};
int gCol[16][4] = {
	{0x00,0x00,0x00,0x00},
	{0xff,0x00,0x00,0x00},
	{0x00,0xff,0x00,0x00},
	{0xff,0xff,0x00,0x00},
	{0x00,0x00,0xff,0x00},
	{0xff,0x00,0xff,0x00},
	{0x00,0xff,0xff,0x00},
	{0xff,0xff,0xff,0x00},
	{0x00,0x00,0x00,0xff},
	{0xff,0x00,0x00,0xff},
	{0x00,0xff,0x00,0xff},
	{0xff,0xff,0x00,0xff},
	{0x00,0x00,0xff,0xff},
	{0xff,0x00,0xff,0xff},
	{0x00,0xff,0xff,0xff},
	{0xff,0xff,0xff,0xff},
};


/*- ���t�ێ��̂��߂Ɏg�p ----------------------------------------------------*/
#ifdef __TURBOC__

unsigned _dos_setftime(int hno, unsigned dat, unsigned tim)
{
	union REGS reg;

	reg.x.ax = 0x5701;
	reg.x.bx = hno;
	reg.x.cx = tim;
	reg.x.dx = dat;
	intdos(&reg, &reg);
	if (reg.x.flags & 1) {/* reg.x.flags ��TC�Ɉˑ� */
		return (unsigned)(errno = reg.x.ax);
	}
	return 0;
}

unsigned _dos_getftime(int hno, unsigned *dat, unsigned *tim)
{
	union REGS reg;

	reg.x.ax = 0x5700;
	reg.x.bx = hno;
	intdos(&reg, &reg);
	if (reg.x.flags & 1) {	/* reg.x.flags ��TC�Ɉˑ� */
		return (unsigned)(errno = reg.x.ax);
	}
	*tim = reg.x.cx;
	*dat = reg.x.dx;
	return 0;
}
#endif

/*---------------------------------------------------------------------------*/
#ifdef STPCPY
char *stpcpy(char *p,char *q)
{
	while (*q) {
		*p++ = *q++;
	}
	*p = '\0';
	return p;
}
#endif

void ChgExt(byte *onam, byte *inam, byte *ext)
{
	byte *p;

	p = strncpy(onam,inam,80);
	if (*p == '.')
		p++;
	if (*p == '.')
		p++;
	if ((p = strrchr(p,'.')) == NULL) {
		p = onam;
		while(*p)
			p++;
	}
	if (ext == NULL||*ext == '\0') {
		*p = '\0';
	} else {
		*p++ = '.';
		strcpy(p,ext);
	}
}

byte *DirExtCut(byte *adr)
{
	byte *p;
	_S byte buf[130];

	adr = p = strncpy(buf,adr,128);
	if (*p == '.')
		++p;
	if (*p == '.')
		++p;
	while (*p != '.' && *p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\')
			adr = p + 1;
		p++;
	}
	*p = '\0';
	return adr;
}

FILE* fopen_e(byte *name, byte *mode)
{
	FILE *fp;

	fp = fopen(name, mode);
	if (fp == NULL) {
		printf("%s �̓I�[�v���ł��܂���ł���\n",name);
		exit(1);
	}
	return fp;
}

/*---------------------------------------------------------------------------*/
#define ActGPage(n)	outp(0xa6,n)
#define GdcWait() while(inp(0xa0)&0x02)

void Gdc_SclInit(void)
{
	int t;

	t = (inp(0x31) & 0x80) ? 0 : 0x40;
	GdcWait();
	outp(0xa2,0x70);
	GdcWait();
	outp(0xa0,0x00);
	GdcWait();
	outp(0xa0,0x00);
	GdcWait();
	outp(0xa0,(400&0x0f) << 4);
	GdcWait();
	outp(0xa0,(400>>4)+t);
	GdcWait();
	outp(0xa0,0x00);
	GdcWait();
	outp(0xa0,0x00);
	GdcWait();
	outp(0xa0,0x00);
	GdcWait();
	outp(0xa0,0x00+t);
}

void GCls2(int c)
{
	int far *p;
	int i;

	p = (int far *)0xa8000000L;
	outp(0x7c,0x80);
	outp(0x7e,gCol[c][0]);
	outp(0x7e,gCol[c][1]);
	outp(0x7e,gCol[c][2]);
	outp(0x7e,gCol[c][3]);
	i = 80*400/2;
	do {
		*p++ = 0;
	} while (--i);
	outp(0x7c,0x00);
}

void GCls(int c)
{
	ActGPage(0);
	GCls2(c);
	if (gWidFlg) {
		ActGPage(1);
		GCls2(c);
		ActGPage(0);
	}
}

void GInit(void)
{
	union	REGS	regs;

	ActGPage(0);
	regs.h.ah = 0x42;
	regs.h.ch = 0xC0;
	int86( 0x18, &regs, &regs);

	regs.h.ah = 0x40;
	int86( 0x18, &regs, &regs);
	outp( 0x6a, 1);
	Gdc_SclInit();
}

void GetRng(int *x1,int *y1,int *x2,int *y2,int c)
{
	int cb,cr,cg,ci;
	int x,y;
	byte_fp bp,rp,gp,ip;

	bp = gVp[0];
	rp = gVp[1];
	gp = gVp[2];
	ip = gVp[3];
	if (gChk < 0)
		printf("%lx,%lx,%lx,%lx\n",bp,rp,gp,ip);
	cb = gCol[c][0];
	cr = gCol[c][1];
	cg = gCol[c][2];
	ci = gCol[c][3];
	if (gChk < 0)
		printf("%02x %02x %02x %02x\n",cb,cr,cg,ci);
	*x1 = *y1 = 0x7fff;
	*x2 = *y2 = 0;
	for (y = 0; y <= gBtmLine; y++) {
		for (x = 0; x < 80; x++) {
			int b,r,g,i;
			if (gChk < 0)
				printf("(%3d,%3d) %02x %02x %02x %02x\n",x,y,*bp,*rp,*gp,*ip);
			b = *(bp++);
			r = *(rp++);
			g = *(gp++);
			i = *(ip++);
			if (b == cb && r == cr && g == cg && i == ci) {
				;
			} else {
				if (x < *x1)
					*x1 = x;
				if (x > *x2)
					*x2 = x;
				if (y < *y1)
					*y1 = y;
				if (y > *y2)
					*y2 = y;
			}
		}
	}
  #if 0
	if (*x1 == 0x7fff)
		*x1 = 0;
	if (*y1 == 0x7fff)
		*y1 = 0;
	if (*x2 == 0)
		*x2 = 79;
	if (*y2 == 0)
		*y2 = gBtmLine;
  #endif
}

void GetRngW(int *x1,int *y1,int *x2,int *y2,int c)
{
	static int xx1,yy1,xx2,yy2;

	ActGPage(0);
	GetRng(x1,y1,x2,y2,c);
	if (*x1 == 0x7fff)
		*x1 = 0;
	if (*y1 == 0x7fff)
		*y1 = 0;
	if (*x2 == 0)
		*x2 = 79;
	if (*y2 == 0)
		*y2 = gBtmLine;
	if (gWidFlg) {
		ActGPage(1);
		GetRng(&xx1,&yy1,&xx2,&yy2,c);
		ActGPage(0);
		if (!(xx1 == 0x7fff && yy1 == 0x7fff)) {
			if (xx1 == 0x7fff)
				xx1 = 0;
			if (yy1 == 0x7fff)
				yy1 = 0;
			if (gWidFlg == 1) {
				if (xx1 < *x1)
					*x1 = xx1;
				if (xx2 > *x2)
					*x2 = xx2;
				if (yy2)
					*y2 = 400 + yy2;
			} else {
				if (yy1 < *y1)
					*y1 = yy1;
				if (yy2 > *y2)
					*y2 = yy2;
				if (xx2)
					*x2 = 80 + xx2;
			}
		}
	}
}

void GetDiff(int *x1,int *y1,int *x2,int *y2)
{
	int x,y;
	byte_fp bp,rp,gp,ip;

	bp = gVp[0];
	rp = gVp[1];
	gp = gVp[2];
	ip = gVp[3];
	if (gChk < 0)
		printf("%lx,%lx,%lx,%lx\n",bp,rp,gp,ip);
	*x1 = *y1 = 0x7fff;
	*x2 = *y2 = 0;
	for (y = 0; y <= gBtmLine; y++) {
		for (x = 0; x < 80; x++) {
			int b,r,g,i;
			int cb,cr,cg,ci;

			ActGPage(1);
			cb = *bp;
			cr = *rp;
			cg = *gp;
			ci = *ip;
			if (gChk < 0)
				printf("%02x %02x %02x %02x\n",cb,cr,cg,ci);
			ActGPage(0);
			if (gChk < 0)
				printf("(%3d,%3d) %02x %02x %02x %02x\n",x,y,*bp,*rp,*gp,*ip);
			b = *(bp++);
			r = *(rp++);
			g = *(gp++);
			i = *(ip++);
			if (b == cb && r == cr && g == cg && i == ci) {
				;
			} else {
				if (x < *x1)
					*x1 = x;
				if (x > *x2)
					*x2 = x;
				if (y < *y1)
					*y1 = y;
				if (y > *y2)
					*y2 = y;
			}
		}
	}
	if (*x1 == 0x7fff)
		*x1 = 0;
	if (*y1 == 0x7fff)
		*y1 = 0;
	if (*x2 == 0)
		*x2 = 79;
	if (*y2 == 0)
		*y2 = gBtmLine;
}

void Foo(byte *name)
{
	static int x1,x2,y1,y2;
	static byte buf[300];
	int i;
	byte *p;

	if (gLoader[0] != '\0') {
		if (gClsCol >= 0) {
			GInit();
			GCls(gClsCol);
		}
		p = stpcpy(buf,gLoader);
		for (i = 0; i < gLdOptsCnt; i++) {
			*p++ = ' ';
			p = stpcpy(p,gLdOpts[i]);
		}
		sprintf(p," %s %s", gLUraOpt, name);
		if (gMsgFlg)
			printf("%s\n",buf);
		if (gBatFp)
			fprintf(gBatFp,"%s\n",buf);
		if (gFdateFlg && gSExt) {
			FILE *fp;
			fp = fopen(name,"rb");
			if (fp) {
				_dos_getftime(fileno(fp), &gFdate, &gFtime);
			}
			fclose(fp);
		}
		if (gBatFlg >= 0)
			system(buf);
	}
	if (gBakCol >= 0) {
		if (gDifFlg)
			GetDiff(&x1,&y1,&x2,&y2);
		else
			GetRngW(&x1,&y1,&x2,&y2,gBakCol);
	} else {
		x1 = y1 = 0;
		x2 = 79;
		y2 = gBtmLine;
		if (gWidFlg == 1)
			y2 = 799;
		else if (gWidFlg == 2)
			x2 = 80*2-1;
	}
	p = stpcpy(buf,gSaver);
	if (gBtmLine == 199) {
		*p++ = ' ';
		p = stpcpy(p,gL2Opt);
	}
	for (i = 0; i < gSvOptsCnt; i++) {
		*p++ = ' ';
		p = stpcpy(p,gSvOpts[i]);
	}
	if (gPxSiz == 0) {
		sprintf(p," -s%d,%d,%d,%d %s",x1*8,y1,(x2+1)*8-1,y2,DirExtCut(name));
	} else {
		sprintf(p,"%s %s -s%d,%d,%d,%d %s",x1,y1,x2,y2,DirExtCut(name));
	}
	if (gSaver[0] != '\0') {
		if (gMsgFlg)
			printf("%s\n",buf);
		if (gBatFp)
			fprintf(gBatFp,"%s\n",buf);
		if (gChk == 0 && gBatFlg == 0)
			system(buf);
	} else {
		printf("%s\n",buf);
		if (gBatFp)
			fprintf(gBatFp,"%s\n",buf);
	}
	if (gPiHdrFlg) {
		sprintf(buf,"pihdr -b -z0 -m%s %s",name,DirExtCut(name));
		if (gMsgFlg)
			printf("%s\n",buf);
		if (gBatFp)
			fprintf(gBatFp,"%s\n",buf);
		if (gChk == 0 && gBatFlg == 0)
			system(buf);
	}
	if (gFdateFlg && gSExt) {
		FILE *fp;
		sprintf(buf,"%s.%s",DirExtCut(name),gSExt);
		fp = fopen(buf,"rb+");
		if (fp) {
			_dos_setftime(fileno(fp), gFdate, gFtime);
		}
		fclose(fp);
	}
}

/*---------------------------------------------------------------------------*/
_S void Usage(void)
{
	pr("usage: cgldsv [-opts] file(s)...   �^�@" CGLDSV "  by �Ă񂩁�\n");
	pr("���[�_�ŉ摜��\�����A�w�i�F�ȊO�̕��������܂�l�p�`�̍��W(��8�h�b�g�Ԋu)��\n"
	   "�Z�[�o�ɓn���ĕ����Z�[�u����.(�J�����g�E�f�B���N�g���ɍ쐬)\n");
	pr("-b[N]     �w�i�F�� N(0-15) �ɂ���. -b- ���ƍ��W��T������ʑS�̂��w��\n");
	pr("-c[N]     �摜�����[�h����O�ɉ�ʂ�F�ԍ� N(0-15) �Ŗ��߂�. -c-�Ŏ��s���Ȃ�\n");
	pr("-d        �t�@�C���̓��t������\n");
	pr("-i        �w�i�F�̑���ɗ�VRAM �Ɣ�r���č��W�����߂�\n");
    pr("-wx[-OPT] ��VRAM���Ȃ���1280*400�͈̔͂ō��W��T��. OPT�̓��[�_�ɓn����߼��\n");
    pr("-wy[-OPT] ��VRAM���Ȃ��� 640*800�͈̔͂ō��W��T��. OPT�̓��[�_�ɓn����߼��\n");
	pr("-l=NAME   ���[�_�����w��. -l=�݂̂��ƃ��[�_���g�p���Ȃ�\n");
	pr("-l-OPT    ���[�_�ɃI�v�V���� -OPT ���킽��. -l-OPT ��10�܂ŉ�\n");
	pr("-lm       ���[�_�Ƃ��� " MAGLD " ���g�p\n");
	pr("-lp       ���[�_�Ƃ��� " PILD " ���g�p\n");
	pr("-lq       ���[�_�Ƃ��� " Q4LD " ���g�p\n");
  #if 1
	pr("-lg       ���[�_�Ƃ��� gpcload ���g�p\n");
  #endif
	pr("-s=NAME   �Z�[�o�����w��. -s=�݂̂��ƃZ�[�o���g�p���Ȃ�\n");
	pr("-s-OPT    �Z�[�o�ɃI�v�V���� -OPT ���킽��. -s-OPT ��10�܂ŉ�\n");
	pr("-sm       �Z�[�o�Ƃ��� " MAGSV " ���g�p\n");
	pr("-sp       �Z�[�o�Ƃ��� " PISV " ���g�p\n");
	pr("-2        �Z�[�o��200���C�����[�h�ɂ���\n");
	pr("-v[0][=FILE] �Z�[�u���s��Ȃ�. -v0 �Ȃ烍�[�h���s��Ȃ�. FILE�Ɍo�߂��o��\n");
	pr("-m        mag �� Pi �ɕϊ�. -lm -sp �Ŏ��s��, PiHdr �� mag �̃w�b�_���R�s�[\n");
  #if 0
	pr("-v[-]     ���s���̖��߂�\��. -v- �ŃI�t\n");
  #endif
    pr("@FILE     FILE ���A�I�v�V�����A�t�@�C����������\n");
  #if 1
	{
		pr("[more]");
		getch();
		pr("\b\b\b\b\b\b\      \b\b\b\b\b\b");
	}
  #endif
	pr("\n");
	pr("�ȗ����� -b0 -c0 -v -l= -sp���w�肳���. �w�肷��t�@�C���̓��[�h����t�@�C��\n");
	pr("��. �p���b�g�͊֒m���Ȃ��̂ŁA���[�_�ƃZ�[�o�̓s�����e���Œ������Ă�.\n");
  #if 1
	pr("�Z�[�o�ɓn���t�@�C�����́A�f�B���N�g����(�h���C�u��)�Ɗg���q���폜����܂�.\n");
	pr("�Z�[�o�⃍�[�_�ɓn���I�v�V������" MAGSV ",pi98��O��ɂ��Ă���̂ŁA���̂��̂��g\n");
	pr("���΂����͏C�����čăR���p�C�������ق�����������.\n");
  #endif
	exit(0);
}

_S void OptsRngErr(byte *s)
{
	printf("-%s �I�v�V�����Ŏw��ł��Ȃ��l���w�肳��Ă���\n",s);
	exit(1);
}

_S void OptsErr(byte *s)
{
	printf("-%s �I�v�V�����̎w�肪��������\n",s);
	exit(1);
}


_S void Option(byte *p)
{
	int c;
	
	c = *p++;
	c = toupper(c);
	switch (c) {
	case 'B':
		if (*p == '-')
			gBakCol = -1;
		else
			gBakCol = (int)strtoul(p,NULL,0);
		if (gBakCol > 15)
			OptsRngErr("c");
		break;
	case 'I':
		gDifFlg = (*p == '-') ? 0 : 1;
		break;
	case 'W':
		c = *p++;
		c = toupper(c);
		if (c == 'X')
			gWidFlg = 2;
		else
			gWidFlg = 1;
		if (*p == '-' || *p == '/')
			gLUraOpt = p;
		break;
	case 'C':
		gClsCol = 0;
		if (*p == '-')
			gClsCol = -1;
		else
			gClsCol = (int)strtoul(p,NULL,0);
		if (gClsCol > 15)
			OptsRngErr("r");
		break;
	case 'V':
		gBatFlg = 1;
		if (*p == '0') {
			gBatFlg = -1;
			++p;
		}
		if (*p == '=') {
			p++;
			if (gBatFp == NULL) {
				gBatFp = fopen_e(p,"w");
			}
		}
		break;
	case 'L':
		c = *p++;
		c = toupper(c);
		if (c == '=') {
			gLoader = p;
			gLExt = NULL;
			gLUraOpt = "";
		} else if (c == 'M') {
			gLoader = MAGLD;
			gLExt = "mag";
			gLUraOpt = MAGLD_URAOPT;
		} else if (c == 'D') {
			gLoader = "Magd";
			gLExt = "mag";
			gLUraOpt = "-lw";
		} else if (c == 'P') {
			gLoader = PILD;
			gLExt = "Pi";
			gLUraOpt = PILD_URAOPT;
		} else if (c == 'I') {
			gLoader = "Pi98";
			gLExt = "Pi";
			gLUraOpt = "";
		} else if (c == 'G') {
			gLoader = "gpcload";
			gLExt = "gpc";
			gLUraOpt = "";
		} else if (c == 'Q') {
			gLoader = Q4LD;
			gLExt = "q4";
			gLUraOpt = "";
		} else if (c == '-' || c == '/') {
			if (gLdOptsCnt >= LDOPTSCNT) {
				printf("-l-OPTS �̐�����������\n");
				exit(1);
			}
			gLdOpts[gLdOptsCnt++] = p - 1;
		} else
			OptsErr("l");
		break;
	case 'M':
		gLoader = MAGLD;
		gLExt = "mag";
		gLUraOpt = MAGLD_URAOPT;
		gSaver  = PISV;
		gPiHdrFlg = 1;
		break;
	case '2':
		gBtmLine = 199;
		break;
	case 'S':
		c = *p++;
		c = toupper(c);
		if (c == '=') {
			gSaver = p;
			gSExt = NULL;
		} else if (c == '8') {
			gPxSiz = 8;
		} else if (c == 'M') {
			gSaver = MAGSV;
			gL2Opt = MAGSV_L2OPT;
			gSExt  = "MAG";
		} else if (c == 'D') {
			gSaver = "MagdSave";
			gL2Opt = "-f2";
			gSExt  = "MAG";
		} else if (c == 'P') {
			gSaver = PISV;
			gSExt  = "PI";
		} else if (c == 'I') {
			gSaver = "Pi98";
			gSExt  = "PI";
		} else if (c == '-' || c == '/') {
			if (gSvOptsCnt >= SVOPTSCNT) {
				printf("-s-OPTS �̐�����������\n");
				exit(1);
			}
			gSvOpts[gSvOptsCnt++] = p - 1;
		} else
			OptsErr("s");
		break;
	case 'D':
		gFdateFlg = 1;
		break;
	case 'Z':
		gChk = 1;
		if (*p == 'D' || *p == 'd')
			gChk = -1;
		break;
	case '?':
	case '\0':
		Usage();
	default:
		printf("-%c �͒m��Ȃ��I�v�V������."
			"- �� -? �Ő��������Ă�\n",c);
		exit(1);
	}
}

void OptiErr(byte *s1,byte *s2)
{
	printf("%s �� %s �͈ꏏ�ɂ͎w��ł��܂���\n",s1,s2);
	exit(1);
}

byte *FGetTok(byte **wk, FILE *fp)
{
	static byte buf[200];
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

void FilOpt(byte *name)
{
	FILE *fp;
	byte *p;
	static byte *aFGetTokWk = NULL;

	fp = fopen_e(name,"r");
	while ((p = FGetTok(&aFGetTokWk,fp)) != NULL) {
		if (*p == '-') {
			Option(p+1);
		}
		/* else {
			printf("@FILE���ł̓t�@�C�����͎w��ł��܂���\n");
			exit(1);
		}*/
	}
	fclose(fp);
}

void FilFil(byte *name,byte *key)
{
	FILE *fp;
	byte *p;
	static byte *aFGetTokWk = NULL;

	fp = fopen_e(name,"r");
	while ((p = FGetTok(&aFGetTokWk,fp)) != NULL) {
		if (*p == '-')
			continue;
  		if (gLExt == NULL)
  			strncpy(key,p,128);
  		else
  			ChgExt(key,p,gLExt);
		Foo(key);
	}
	fclose(fp);
}

void KeyBrk(int sig)
{
	sig = 1;
	exit(sig);
}

void main(int argc,byte *argv[])
{
	int i;
	byte *p;
	static byte name[130];
	static byte key[130];

	if (argc < 2)
		Usage();
	signal(SIGINT,KeyBrk);
	gMsgFlg = 1;
	gClsCol = 0;
	gBakCol = -1/*0*/;
	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			Option(p+1);
		} else if (*p == '@') {
			FilOpt(p+1);
		}
	}
	if (gBatFlg)
		gMsgFlg = 1;
	if (gBatFlg < 0)
		gBakCol = -1;
	if (gWidFlg && gDifFlg)
		OptiErr("-wx,-wy", "-i");
	if (gWidFlg && gBtmLine == 199)
		OptiErr("-wx,-wy", "-2");
	/*if (gWidFlg && gBakCol < 0)
		OptiErr("-wx,-wy", "-b-");*/
	if (!gWidFlg)
		gLUraOpt = "";
	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			continue;
  		} else if (*p == '@') {
  			FilFil(p+1,key);
			continue;
		} else {
			if (gLExt == NULL)
				strncpy(key,p,128);
			else
				ChgExt(key,p,gLExt);
			p = key;
	  #ifdef NOWILDC
			Foo(p);
	  #else
			if (*gLoader == '\0') {
				Foo(p);
			} else if(DirEntryGet(name,p,0) == 0) {
				do {
					Foo(name);
				} while (DirEntryGet(name,NULL,0) == 0);
			} else {
				printf("%s �͂���܂���\n",p);
			}
  	  #endif
		}
	}
}
