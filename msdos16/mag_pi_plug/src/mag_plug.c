/*===========================================================================*/
/*	MAG_PLUG																	 */
/*===========================================================================*/

#include "gf.h"
#include "tofmt1p.c"


/*---------------------------------------------------------------------------*/
/*-----    �w  ��  �v    ----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void G_Usage(void)
{
	puts(
		"mag_plug ver1.00                          by Tenk* 1996\n"
		"usage: mag_plug [-a] <COMMAND> <inputfile> <outputfile>\n"
		" <COMMAND> ... DJ505JC or TO_DJP.\n"
		);
	exit(1);
}



/*---------------------------------------------------------------------------*/
/*-----    �� �� �� �[ �` ��   ----------------------------------------------*/
/*---------------------------------------------------------------------------*/
GF		*gf;				/* ���ۂ̉摜�W�J�Ŏg���郁����(��׸���)*/
void	*mem;				/* ���ۂ̉摜�W�J�Ŏg���郁����		*/

int  G_Open(TOFMT1P *t, char *name)
{
	gf  = NULL;
	mem = _fmalloc(0x20000LU);
	if (mem == NULL) {
		return 1;
	}
	gf = MAG_Open(mem, name);
	if ((((ULONG)gf)&0xffff0000UL) == 0L) {
		static int errNo[] = {
			-1,		/* PRGERR:�摜�o�b�t�@�ւ̃|�C���^�̒l���s�� */
			2,		/* �t�@�C�����I�[�v���ł��Ȃ����� */
			3,		/* �w�b�_ID���Ⴄ */
			3,		/* �w�b�_�ɖ��������� */
		};
		return errNo[ (int)( ((ULONG)gf) & 0xffffU) ];
	}

	t->col  = gf->pln;
	t->xsz  = gf->xsize;
	t->ysz  = gf->ysize;
	t->x0   = gf->xstart;
	t->y0   = gf->ystart;
	t->xasp = gf->aspect1;
	t->yasp = gf->aspect2;
	t->bcol = gf->overlayColor;
	t->comment = gf->comment;
	memcpy(t->artist, gf->artist, sizeof gf->artist);
	memcpy(t->rgb, gf->palette, 3*256);
	return 0;
}


void G_Load(TOFMT1P *t, void (far * far putLine)(void far *, UCHAR far *))
{
	static char *errMsgLoad[] = {
		"read error.\n",
		"Abnormal data.\n",					/* �f�[�^����݂����� */
		"[WIDTH] out of range.\n",			/* ���������������邩�A�܂��͑傫������ */
		"sorry... unkown format.",
		"Programer's error.(incorrect pointer)\n",
	};
	int n;

	n = MAG_Load(gf, putLine, t, t->ysz);
	if (n) {
		printf("%s : %s\n",gf->fileName,errMsgLoad[n]);
	}
}


void G_Close(TOFMT1P *t)
{
	if (gf)
		MAG_Close(gf);
	if (mem)
		_ffree(mem);
}
