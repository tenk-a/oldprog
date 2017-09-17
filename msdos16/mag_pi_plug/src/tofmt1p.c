/*===========================================================================*/
/*  	DJP�ȃv���O�C���d�l�́@?->DJP,PMT �R���o�[�^ ���ʃ��[�`��   	     */
/*  	far ���f���ŃR���p�C�����邱��!!    	    	    	    	     */
/*===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include <malloc.h>
#define VBUFSIZE    	0x2000

typedef unsigned char	UCHAR;
typedef unsigned int	UINT;
typedef unsigned short	USHORT;
typedef unsigned long	ULONG;

#define isdigit(c)  (((unsigned)(c) - '0') < 10U)
#define iskanji(c)  ((unsigned)((c)^0x20) - 0xa1 < 0x3cU)
#define islower(c)  (((unsigned)(c)-'a') < 26U)
#define toupper(c)  (islower(c) ? (c) - 0x20 : (c) )

#ifdef DEBUG
#define D(a)	a
#else
#define D(a)	{}
#endif



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

typedef struct TOFMT1P {
    int     xsz, ysz;	    	/* �摜�T�C�Y	    	    	    	*/
    int     x0, y0; 	    	/* �n�_     	    	    	    	*/
    int     xasp, yasp;     	/* �A�X�y�N�g��     	    	    	*/
    int     col;    	    	/* �F��bit 4:16�F  8:256�F  	    	*/
    int     bcol;   	    	/* �����F 0:���� 1�`:����(bcol-1)   	*/
    UINT    dat;    	    	/* �t�@�C�����t     	    	    	*/
    UINT    tim;    	    	/* �t�@�C������     	    	    	*/
    char    *comment;	    	/* �R�����g�ւ̃|�C���^     	    	*/
    char    artist[20];     	/* ��Җ�   	    	    	    	*/
    UCHAR   rgb[256*3];     	/* �p���b�g (r,g,b)*256 r,g,b�͊e8�ޯ�	*/
} TOFMT1P;


void G_Usage(void);
int  G_Open(TOFMT1P *t, char *name);
void G_Load(TOFMT1P *t, void (far * far putLine)(void far *, UCHAR far *));
void G_Close(TOFMT1P *t);



/*---------------------------------------------------------------------------*/
/*-----    �o �́@�� �[ �` ��	---------------------------------------------*/
/*---------------------------------------------------------------------------*/
static FILE *TO_fp;

#define TO_Err()    (ferror(TO_fp))

#define TO_PutB(b)  ( fputc((UCHAR)(b),TO_fp) )

static void TO_PutW(UINT w)
{
    fputc((UCHAR)(w),TO_fp);
    fputc((UCHAR)((w)>>8),TO_fp);
}

static void TO_PutD(ULONG d)
{
    fputc((UCHAR)(d),TO_fp);
    fputc((UCHAR)((d)>>8),TO_fp);
    fputc((UCHAR)((ULONG)(d)>>16),TO_fp);
    fputc((UCHAR)((ULONG)(d)>>24),TO_fp);
}

static void TO_Write(void *buf, unsigned size)
{
    fwrite(buf, 1, size, TO_fp);
}

static void _loadds /*_saveregs*/ far TO_PutLine(void far *t, UCHAR far *b)
{
    TO_Write(b, ((TOFMT1P*)t)->xsz);
}

static void _loadds /*_saveregs*/ far TO_PutLineL2(void far *t, UCHAR far *b)
{
    TO_Write(b, ((TOFMT1P*)t)->xsz);
    TO_Write(b, ((TOFMT1P*)t)->xsz);
}


static int TO_Djp(TOFMT1P *t, char *name, int aspFlg)
{
    aspFlg = (aspFlg && t->xasp && t->yasp && (t->yasp*10 / t->xasp) >= 18);

    TO_fp = fopen(name, "wb");	    	    	    if (TO_fp==NULL) return 2;
    if( setvbuf(TO_fp, NULL, _IOFBF, VBUFSIZE) )    {fclose(TO_fp); return 1; }

    /* ID */
    TO_Write("DJ505J",6);   	    	    	    if (TO_Err())   goto ERR_W;
    /* �摜�T�C�Y */
    TO_PutW(t->xsz);	    	    	    	    if (TO_Err())   goto ERR_W;
    TO_PutW((aspFlg) ? (t->ysz+t->ysz) : t->ysz);   if (TO_Err())   goto ERR_W;
    /* 8�r�b�g�F�摜 */
    TO_PutW(0);     	    	    	    	    if (TO_Err())   goto ERR_W;
    /* �p���b�g */
    TO_Write(t->rgb, 256*3);	    	    	    if (TO_Err())   goto ERR_W;
    /* �s�N�Z���E�f�[�^ */
    if (aspFlg)
    	G_Load(t, TO_PutLineL2);
    else
    	G_Load(t, TO_PutLine);
    /* �I�� */
    fclose(TO_fp);
    return 0;

  ERR_W:
    fclose(TO_fp);
    return 3;
}



static int TO_Pmt(TOFMT1P *t, char *name, int aspFlg)
    /* �ٍ� mg.exe �̍��̧��̫�ϯ� PMT �Ή�... �قƂ�ǂ̐l�ɖ��֌W^^; */
{
    int  i;

    aspFlg = (aspFlg && t->xasp && t->yasp && (t->yasp*10 / t->xasp) >= 18);

    TO_fp = fopen(name, "wb");
    if (TO_fp == NULL)
    	return 2;
    if ( setvbuf(TO_fp, NULL, _IOFBF, VBUFSIZE) ) {
    	fclose(TO_fp);	return 1;
    }

    TO_Write("Pm",2);	    	    	    	    	    	/* ID	    */
    if (TO_Err())
    	goto ERR_W;
    TO_PutB(t->col);	    	    	    	    	    	/* 16|256�F�摜 */
    TO_PutB(0x00);  	    	    	    	    	    	/* flags    */
    TO_PutW(t->xsz);	    	    	    	    	    	/* x size   */
    TO_PutW((aspFlg)?(t->ysz*2):t->ysz);    	    	    	/* y size   */
    TO_PutW(t->x0); 	    	    	    	    	    	/* x start  */
    TO_PutW((aspFlg)?(t->y0*2):t->y0);	    	    	    	/* y start  */
    TO_PutW(t->bcol);	    	    	    	    	    	/* �����F   */
    TO_PutW(0x00);  	    	    	    	    	    	/* rsv.     */
    TO_PutW((aspFlg) ? 1 : (t->xasp?t->xasp:1));    	    	/* x aspect.*/
    TO_PutW((aspFlg) ? 1 : (t->yasp?t->yasp:1));    	    	/* y aspect.*/

    if (t->comment && strlen(t->comment))   	    	    	/* comment  */
    	TO_PutD(64 + 256*3 + (ULONG)t->xsz*(ULONG)t->ysz);
    else
    	TO_PutD(0L);

    /* ��Җ� */
    if (strlen(t->artist) == 0) {
    	memset(t->artist, 0 , sizeof t->artist);
    }
    TO_Write(t->artist, 18);
    TO_PutB(0);

    /* �t�@�C������,���t */
    TO_PutB(0);
    TO_PutW(t->tim);
    TO_PutW(t->dat);

    /* �\��̈� */
    for(i = 0; i < 16; i++)
    	TO_PutB(0);

    if (TO_Err())
    	goto ERR_W;

    /* �p���b�g */
    TO_Write(t->rgb, 256*3);
    if (TO_Err())
    	goto ERR_W;

    /* �s�N�Z���E�f�[�^ */
    if (aspFlg) {
    	G_Load(t, TO_PutLineL2);
    } else {
    	G_Load(t, TO_PutLine);
    }

    /* �R�����g */
    if (t->comment && strlen(t->comment)) {
    	char *p;
    	for (p = t->comment, i = strlen(t->comment); i-- ; p++)
    	    TO_PutB(*p);
    	TO_PutB(0);
    	if (TO_Err())
    	    goto ERR_W;
    }

    fclose(TO_fp);
    return 0;

  ERR_W:
    fclose(TO_fp);
    return 3;
}



/*---------------------------------------------------------------------------*/
/*-----    ���@�C�@���@�� �[ �` ��   ----------------------------------------*/
/*---------------------------------------------------------------------------*/


int cdecl main(int argc, char *argv[])
{
    enum {FMT_NON, FMT_DJP, FMT_PMT} fmt;
    static TOFMT1P t;
    int i, c, aflg;
    char *p, *srcname, *dstname;

    fmt = FMT_NON;
    aflg = 0;
    if (argc < 2)
    	G_Usage();

    /* �I�v�V�����̃`�F�b�N */
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p != '-')
    	    break;
    	++p;
    	c = *p++, c = toupper(c);
    	switch(c) {
    	case 'A':
    	    aflg = 1;
    	    if (*p == '-')
    	    	aflg = 0;
    	    break;
    	case '?':
    	case '\0':
    	    G_Usage();
    	default:
    	    printf("Incorrect command-line option. : %s\n", argv[i]), exit(1);
    	}
    }
    if (argc-i < 3) {
    	printf("Too few command-line arguments.\n");	exit(1);
    }
    srcname = argv[i+1];
    dstname = argv[i+2];

    /* �o�̓t�H�[�}�b�g�w��̃`�F�b�N */
    strupr(p);
    if	    (strcmp(p,"DJ505JC") == 0) fmt = FMT_DJP,	aflg = 1;
    else if (strcmp(p,"DJ505JM") == 0) fmt = FMT_DJP,	aflg = 1;
    else if (strcmp(p, "TO_DJP") == 0) fmt = FMT_DJP;
    else if (strcmp(p, "TO_PMT") == 0) fmt = FMT_PMT;
    else {
    	printf("Bad '<FORMAT>' name. : %s\n",p), exit(1);
    }

    /* �ϊ����b�Z�[�W */
    printf("%s : (%s) %s -> %s\n", argv[0], argv[i], argv[i+1], argv[i+2]);

    /* �摜��W�J���邽�߂̏����� */
    memset(&t, 0, sizeof t);
    switch(G_Open(&t,srcname)) {
    case 0: break;
    case 1: printf("Not enough memory for input-buffer.\n");	return 1;
    case 2: printf("Unable to open file '%s'\n", srcname);  	return 1;
    case 3: printf("Incorrect header.\n");  	    	    	return 1;
    default:printf("Programer's error 'G_Open'\n"); 	    	return 1;
    }

    /* �W�J���o�� */
    switch (fmt) {
    case FMT_PMT: c = TO_Pmt(&t, dstname, aflg); break;
    case FMT_DJP: c = TO_Djp(&t, dstname, aflg); break;
    default:	  printf("Programer's error.\n"); return 1;
    }
    /* �I�� */
    G_Close(&t);
    switch (c) {
    case 1: printf("Not enough memory for output-buffer.\n");	return 1;
    case 2: printf("Cannot create file '%s'\n",dstname);    	return 1;
    case 3: printf("Cannot write output.\n");	    	    	return 1;
    }
    return 0;
}
