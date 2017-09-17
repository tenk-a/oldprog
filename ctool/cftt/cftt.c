/****************************************************************************
  cftt v2.11

    	C����\�[�X�v���O�����̌ďo�֌W�𒲂ׂ�

 1991/05/?? �ɓ�������� callc ���������Ă����ʂ��̂ɂȂ��Ă����̂ƃ\�[�X��
    	    �o�b�`���̂𐮗����邽�ߍ�蒼���B
 1991�`1994 ����. �^����������Ă����A�����������͖Y�p�̔ޕ��B
 1995/06/17 �傫�ȃv���O��������͂��邽�� djgcc �ōăR���p�C��.
 cftt v2.00 �֐��������łȂ��A�}�N�����A�ϐ����Ƃ��̔��ʂ�����B
    	    ���G�ȏo�͂̎w�����߁A.t01 .t02 .t03 .t04	�ƌŒ�ŏo�́B
    	    ���X�|���X�t�@�C���̏���������ύX.	.cfg��ǂނ悤�ɂ���B
    	    ���̑��������A�����ς��ύX,�ǉ����폜�B
 1996/04/01 �c���[�\���ł̃O���[�v���̒ʂ��ԍ�������Ă����̂��C��.
 cftt v2.10 (�O���[�v���̒ʂ��ԍ����Đ����ȂƂ��s�v���Ȃ�...�j
    	    ���łɕs�v�s���ƂȂ����I�v�V�����̍폜��ύX�B
    	    .t03 .t0 4�o�͂Ń^�O�`���łȂ����O�݂̂̏o�͂��\��.
    	    @root �p�~�B@lib��@group �ɕϖ����Ĉ����ǉ�.@comment �ǉ�.
    	    @if0 �` @endif ��ǉ�. �ȂǂȂǁB
 1997/09/28 Bcc32 v5.2 �ŃR���p�C��������\�������̂Ńf�o�b�O(NULL�΍�)

 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*--------------------------------------------------------------------------*/
typedef	unsigned char	UCHAR;
typedef	unsigned short	USHORT;
typedef	unsigned    	UINT;
typedef	unsigned long	ULONG;

#define	ISKANJI(c)  ((unsigned)((c)^0x20) - 0xa1 < 0x3cU)
#define	ISKANJI2(c) ((UCHAR)(c)	>= 0x40	&& (UCHAR)(c) <= 0xfc && (c) !=	0x7f)


/*--------------------------------------------------------------------------*/

#define	OPTDIV
#define	FINO_OFS 9
#define	FILNUMSZ    12	    	/* �\������t�@�C�����̃T�C�Y	    	*/

#ifdef _M32_
#define	GRPFNAME_MAX	(260+1)	/* @group �Ŏw��ł���̧�ق̐�	    	*/
#define	LIN_SIZ	    248	    	/* �\������1�s�̕�  	    	    	*/
#define	SYM_SIZ	    128	    	/* ���O�̒���	    	    	    	*/
#define	ROOT_MAX    (1024*4+1)	/* -r=<NAME> �̐�   	    	    	*/
#define	MIF_MAX	    40	    	/* #if�l�X�g�̐[��  	    	    	*/
#define	LINBUF_SIZ  (1024*4)	/* linBuf�̃T�C�Y   	    	    	*/
#define	RSV_MAX	    (2048)  	/* �\���̍ő吔   	    	    	*/
#define	SETVBUF_SIZ (0xC000)	/* ̧�ُ����p�o�b�t�@	    	    	*/
#else
#define	GRPFNAME_MAX	(20+1)	/* @group �Ŏw��ł���̧�ق̐�	    	*/
#define	LIN_SIZ	    80	    	/* �\������1�s�̕�  	    	    	*/
#define	SYM_SIZ	    64	    	/* ���O�̒���	    	    	    	*/
#define	ROOT_MAX    (52+1)  	/* -r=<NAME> �̐�   	    	    	*/
#define	MIF_MAX	    20	    	/* #if�l�X�g�̐[��  	    	    	*/
#define	LINBUF_SIZ  256	    	/* linBuf�̃T�C�Y   	    	    	*/
#define	RSV_MAX	    (128)   	/* �\���̍ő吔   	    	    	*/
#define	SETVBUF_SIZ (0x2000)	/* ̧�ُ����p�o�b�t�@	    	    	*/
#endif

typedef	struct REFLIST_T {  	/* �֐��Q�Ƃ̂��߂̃��X�g�̍\����   	*/
    struct TNODE_T  	*nodePnt;
    struct REFLIST_T	*next;
} REFLIST;

typedef	struct MULTDEF_T {
    char *filName;
    int	 defLin;
    struct MULTDEF_T *next;
} MULTDEF;

typedef	struct TNODE_T {    	/*  	-- �葱�����m�[�h --	    	*/
    struct TNODE_T *link[2];	/* 0:�؂̍��ւ̃|�C���^	    1:�E    	*/
    int	    avltFlg;	    	/* avl-tree �̃o�����X	    	    	*/
    char    *symName;	    	/* �֐���   	    	    	    	*/
    char    *filName;	    	/* �֐���`�̂���t�@�C�����ւ��߲��	*/
    short   atr;    	    	/* 0:�֐� 1:�ϐ��� 2:�}�N�� 	    	*/
    int	    bgnLin; 	    	/* �֐���`�͈͂̊J�n�s	    	    	*/
    int	    defLin; 	    	/* �֐���`�̍s�ԍ� 	    	    	*/
    int	    refLin; 	    	/* tree�\���ł̎Q�Ƃ���ԍ� 	    	*/
    short   refGrp; 	    	/* tree�\���ł̎Q�Ƃ����ٰ�ߔԍ�   	*/
    struct REFLIST_T *calls;	/* ���̊֐����Ăяo���֐���ؽĂւ��߲��	*/
    struct REFLIST_T *caller;	/* ���̊֐����Ăяo���֐���ؽĂւ��߲��	*/
    struct MULTDEF_T *multDef;	/* ���d��`�ӏ��ꗗ 	    	    	*/
    char    *cmnt;  	    	/* ���[�U��`�R�����g	    	    	*/
} TNODE;

/*---------------------------------------------------------------------------*/
int 	____dmy____;	    	/* TC4(PowerPack16)�΍�(T T)	    	*/
int 	filNamSiz;  	    	/* �\������t�@�C����(PathList)�̃T�C�Y	*/
int 	level;	    	    	/* {}�̃��x��...0 ����ԊO������킷	*/
int 	funcNo;	    	    	/* main(),DispOyakoFlow()�Ŏg�p(����)	*/
int 	pass;	    	    	/* pass	0:�o�^���� 1:�Q�Ə���	    	*/
char	linBuf[LINBUF_SIZ+1];	/* �o�͍s�����̂��߂̃o�b�t�@	    	*/
int 	SYM_linNo;  	    	/* ���O�̌��������s�ԍ�   	    	*/
int 	SYM_sttNo;  	    	/* ��`�J�n�̍s�ԍ� 	    	    	*/
int 	SYM_atr;    	    	/* ���O����: 1,2:�֐� 3:�ϐ�? 4:�}�N��	*/
char	SYM_name[SYM_SIZ+1];	/* ���O������̂��߂̃o�b�t�@	    	*/
int 	Flg_symMode = 1;    	/* 0=�֐� 1=�Ȃ�ł�	    	    	*/
UCHAR	Flg_dspAllLine;	    	/* -t1��߼�݂��׸�  	    	    	*/
UCHAR	Flg_dspOyakoMode;   	/* -c1��߼�݂��׸�  	    	    	*/
UCHAR	Flg_dspTree = 1;    	/* Tree�\���̗L��   	    	    	*/
UCHAR	Flg_dspOyakoTree;   	/* �e�q�֌W�\����tree�\�������֐��݂̂�?*/
UCHAR	Flg_dspUndef;	    	/* ����]�֐��݂̂̕\����   	    	*/
UCHAR	Flg_dspTrFuncOnly;  	/* tree�ŕ\������̂͊֐��I�����[   	*/
UCHAR	Flg_msg;    	    	/* ���b�Z�[�W(�}��)�t���O   	    	*/
UCHAR	Flg_karagyo = 1;    	/* ��s�o�� 1:on 0:off	    	    	*/
REFLIST	*refl0;	    	    	/* ̧�ْ��̊֐��̏��Ԃ�ؽč쐬�Ɏg�p	*/
TNODE	*TREE_root; 	    	/* MakSmTree(),MakRefLst(),main()�Ŏg�p	*/
TNODE	*TREE_curNode;	    	/* */
int 	ROOT_cnt;   	    	/* tree�\����ٰĂɂ���֐����̐�    	*/
int 	ROOT_grp;   	    	/* tree�\����ٰĂɂ���֐�����group�ԍ�	*/
char	*ROOT_name[ROOT_MAX];	/* tree�\���Ń��[�g�ɂ���֐����̔z��	*/
int 	RSV_cnt;    	    	/* RSV_name�Ǘ��p   	    	    	*/
char	*RSV_name[RSV_MAX]; 	/* �\��ꈵ���̖��O�ꗗ	    	    	*/
char	*GRP_fnam[GRPFNAME_MAX];/* @Groupfile�Ŏw�肷��t�@�C����   	*/
char	*GRP_cmnt[GRPFNAME_MAX];/* group���Ƃ̃R�����g	    	    	*/
int 	GRP_no;	    	    	/* DispTreeFlow()�Ŏg�p(����)	    	*/
int 	GRP_subNo;  	    	/* DispTreeFlow()�Ŏg�p(����)	    	*/
char	*SRC_name;  	    	/* ���ݓ��͒��̃t�@�C�����ւ̃|�C���^	*/
FILE	*SRC_fp;    	    	/* ���ݓ��͒��̃t�@�C���̃X�g���[�� 	*/
FILE	*errFp;	    	    	/* �G���[�o�͗pfp   	    	    	*/
typedef	struct SRCL_T {
    struct SRCL_T   *link;  	/* ���̃��X�g�ւ̃|�C���^   	    	*/
    char    	    *name;  	/* ���͂��鿰��̧�ٖ��ւ��߲���̔z��	*/
    REFLIST 	    *func;  	/* ̧�قɂ���֐���ؽ�	    	    	*/
} SRCL;
SRCL *SRCL_top;	    	    	/* �\�[�X�t�@�C�����X�g�̐擪	    	*/

#define	DEBUG
#ifdef DEBUG
UCHAR	Debug_flg;  	    	/* �f�o�b�O�p�\���̃t���O   	    	*/
#endif

enum {AT_NON=0,AT_FUNC,AT_DECL,AT_VAR,AT_MAC,AT_CMNT};
char AT_str[][2] = {" ","�","�","�","�"};

/*---------------------------------------------------------------------------*/
FILE	*fopenE(char *fname, char *mod)
{
    FILE *fp;

    fp = fopen(fname,mod);
    if (fp == NULL) {
    	fprintf(errFp, "\n�t�@�C�� %s ���I�[�v���ł��܂���\n", fname);
    	exit(errno);
    }
    setvbuf(fp,	NULL, _IOFBF, SETVBUF_SIZ);
    return fp;
}

void *callocE(size_t a,	size_t b)
{
    void *p;
    p =	calloc(a, b);
    if (p == NULL) {
    	fprintf(errFp,"\n������������܂���\n");
    	exit(1);
    }
    return p;
}

char *strdupE(char *s)
{
    char *p;
    p =	strdup(s);
    if (p == NULL) {
    	fprintf(errFp,"\n������������܂���\n");
    	exit(1);
    }
    return p;
}


/*---------------------------------------------------------------------------*/
char	*FIL_BaseName(char *adr)
{
    char *p;

    p =	adr;
    while (*p != '\0') {
    	if (*p == ':' || *p == '/' || *p == '\\')
    	    adr	= p + 1;
    	if (ISKANJI((*(unsigned	char *)p)) && *(p+1) )
    	    p++;
    	p++;
    }
    return adr;
}

char	*FIL_ChgExt(char filename[], char *ext)
{
    char *p;

    p =	FIL_BaseName(filename);
    p =	strrchr( p, '.');
    if (p == NULL) {
    	strcat(filename,".");
    	strcat(	filename, ext);
    } else {
    	strcpy(p+1, ext);
    }
    return filename;
}

char	*FIL_AddExt(char filename[], char *ext)
{
    if (strrchr(FIL_BaseName(filename),	'.') ==	NULL) {
    	strcat(filename,".");
    	strcat(filename, ext);
    }
    return filename;
}



/*---------------------------------------------------------------------------*/
int 	STBL_Add(char *t[], int	*tblcnt, char *key)
   /*
    *  t     : ������ւ̃|�C���^�������߂��z��
    *  tblcnt: �o�^�ό�
    *  key   : �ǉ����镶����
    *  ���A�l: 0:�ǉ� -1:���łɓo�^��
    */
{
    int	 low, mid, f, hi;

    hi = *tblcnt;
    mid	= low =	0;
    while (low < hi) {
    	mid = (low + hi	- 1) / 2;
    	if ((f = strcmp(key, t[mid])) <	0) {
    	    hi = mid;
    	} else if (f > 0) {
    	    mid++;
    	    low	= mid;
    	} else {
    	    return -1;	/* �������̂��݂������̂Œǉ����Ȃ� */
    	}
    }
    (*tblcnt)++;
    for	(hi = *tblcnt; --hi > mid;) {
    	t[hi] =	t[hi-1];
    }
    t[mid] = key;
    return 0;
}


int 	STBL_Search(char *tbl[], int nn, char *key)
   /*
    *  key:������������ւ̃|�C���^
    *  tbl:������ւ̃|�C���^�������߂��z��
    *  nn:�z��̃T�C�Y
    *  ���A�l:��������������̔ԍ�(0���)  �݂���Ȃ������Ƃ�-1
    */
{
    int	    low, mid, f;

    low	= 0;
    while (low < nn) {
    	mid = (low + nn	- 1) / 2;
    	if ((f = strcmp(key, tbl[mid]))	< 0)
    	    nn = mid;
    	else if	(f > 0)
    	    low	= mid +	1;
    	else
    	    return mid;
    }
    return -1;
}

/*---------------------------------------------------------------------------*/


static int  uncharBuf[256]; 	/* 1�����ޯ��Ɏg�p */
    	    	    	    	/* �\���ɍL���͂��Ȃ̂œY���`�F�b�N�� */
static int  *unChar = uncharBuf;

static int  linNo = 1;	    	/* ���ݓǂݍ��ݒ��̍s�ԍ�   	    	*/
static int  Mif_p = 0;	    	/* Mif_Set(),SYM_GetInit() �Ŏg�p   	*/
static char *Mif_Name[]	= { 	/* Mif_Set(),LineTop() �Ŏg�p	    	*/
    "elif", "else", "endif", "if", "ifdef", "ifndef"
};

void	RSV_Init(void)
{
    static char	*rsv[] = {  /* �i����'('������\���̂���j�\���	*/
    	"",
    	"auto",	    "break",	"case",	    "char",
    	"const",    "continue",	"default",  "defined",
    	"do",	    "double",	"else",	    "enum",
    	"extern",   "float",	"for",	    "goto",
    	"if",	    "int",  	"long",	    "register",
    	"return",   "short",	"signed",   "sizeof",
    	"static",   "struct",	"switch",   "typedef",
    	"typeof",   "union",	"unsigned", "void",
    	"volatile", "while",
    };	/* int���̌^�w��q������̂� char (*f)[]�Ƃ����悤�Ȑ錾�����邽�� */
    #define RSVCNT (sizeof rsv / sizeof	rsv[0])
    int	i;

    for	(i = 0;	i < RSVCNT; i++) {
    	RSV_name[i] = rsv[i];
    }
    RSV_cnt = i;
}


static void Err_Src(char *m)
{
    fprintf(errFp, "\n%s %d : �\�[�X�ɃG���[������܂�. %s\n", SRC_name, linNo,	m);
    exit(1);
}

static void Err_Eof(void)
{
    fprintf(errFp, "\n%s %d : �������ȂƂ����EOF�ɂȂ�܂���\n",
    	    SRC_name, linNo);
    exit(1);
}


static void CH_Unget(int c)
    /* CH_Get�œ����P��������͂ɖ߂� */
{
    if (c == '\n')
    	linNo--;
    *++unChar =	c;
}

static int CH_Get(void)
    /* �ꕶ������
     *	 ���A�l	0x01����0xff,����� EOF(�����Ă��� -1)
     * ...0x00�� CH_Unget�ɗ��p�����..���͂̓e�L�X�g�łȂ��Ƃ����Ȃ��I
     */
{
    int	    c;

    if (*unChar	== 0) {
    	c = getc(SRC_fp);
      #if 0
    	if (feof(SRC_fp))
    	    c =	EOF;
      #endif
    } else {	    	    /* CH_Unget���ꂽ�l���Ƃ肾�� */
    	c = *unChar;
    	*unChar-- = '\0';
    }
  #ifdef DEBUG
   #if 0
    if (level == 0) {
    	fprintf(errFp, "%d : %d", linNo, c);
    	if (0x20 <= c && c <= 0x7e)
    	    fprintf(errFp, "  '%c'", c);
    	fprintf(errFp, "\n");
    }
   #endif
  #endif
    if (c == '\n')
    	linNo++;
    return c;
}

static int CH_Get_ChkEof(void)
    /* EOF�Ȃ�G���[�I������CH_Get() */
{
    int	    c;

    c =	CH_Get();
    if (c < 0)	    	    	/* EOF�͕��l */
    	Err_Eof();
    return c;
}


static int CH_Skip(void)
    /* 1�������� .. '\'�G�X�P�[�v�A�����Ȃ�' '(0x20)��Ԃ� */
{
    int	    c, i;

    c =	CH_Get_ChkEof();
    if (c == '\\') {
    	c = CH_Get_ChkEof();
    	if ('0'	<= c &&	c <= '7') {
    	    i =	3;
    	    do {
    	    	c = CH_Get_ChkEof();
    	    	if (c <	'0' || '7' < c)	{
    	    	    CH_Unget(c);
    	    	    break;
    	    	}
    	    } while (--i);
    	    c =	' ';
    	} else if (c ==	'x' || c == 'X') {
    	    do {
    	    	c = CH_Get_ChkEof();
    	    } while (isxdigit(c));
    	    CH_Unget(c);
    	    c =	' ';
    	} else if (strchr("\n\\\?\'\"ntvbrfa", c)) {
    	    c =	' ';
    	}
    } else {
    	if (ISKANJI(c))	{
    	    CH_Get_ChkEof();
    	    c =	' ';
    	}
    }
    return c;
}

static void CH_Unget_CmtSkip(int c)
    /* CH_Get_CmtSkip�œ����P������߂�	*/
{
    CH_Unget(c);
}

static int CH_Get_CmtSkip(void)
    /* �R�����g��ǂ݂Ƃ���1�������� */
{
    int	    c, c1;

    for	(;;) {
    	c = CH_Get();
    	if (c == '\\') {
    	    c1 = CH_Get_ChkEof();
    	    if (c1 == '\n')
    	    	c = ' ';
    	    else
    	    	CH_Unget(c1);
    	    break;
    	}
      #if 1
    	if (c == '*') {	/* �|�C���^��'*'����������(��Z������) */
    	    c =	' ';
    	    break;
    	}
      #endif
    	if (c != '/')
    	    break;
    	c1 = CH_Get_ChkEof();
    	if (c1 == '*') {
    	    for	(;;) {	    	/* �R�����g��ǂݔ�΂�	 */
    	    	c = CH_Skip();
    	    	if (c == '*') {
    	    	    c =	CH_Get_ChkEof();
    	    	    if (c == '/')
    	    	    	break;	/* �R�����g�I��	    */
    	    	    CH_Unget(c);
    	    	}
    	    }
    	    /* break; */
    	} else if (c1 == '/') {
    	    while (CH_Skip() !=	'\n') ;
    	    c =	'\n';
    	    break;
    	} else {
    	    CH_Unget(c1);
    	    break;
    	}
    }/*	end for	*/
    return c;
}

static int CH_Get_CmtSkipEofChk(void)
    /* EOF�ŃG���[�I������CH_Get_CmtSkip() */
{
    int	    c;

    c =	CH_Get_CmtSkip();
    if (c < 0)
    	Err_Eof();
    return c;
}

static int SYM_GetStr(void)
   /* �P�P�����. ���O�������SYM_name�ɂ���ĕԂ� �Ȃ���� ���̂Ƃ��̕���. */
{
    int	    c, i;

    SYM_name[0]	= '\0';
    do {
    	c = CH_Get_CmtSkip();
    } while (c == ' ' || c == '\t');

    if (c > 0 && c <= 0xff) {
    	if (isalpha(c) || c == '_') {
    	    SYM_name[0]	= (char)c;
    	    i =	1;
    	    for	(;;) {
    	    	c = CH_Get_CmtSkipEofChk();
    	    	if (isalnum(c) == 0 && c != '_')
    	    	    break;
    	    	if (i <	SYM_SIZ)
    	    	    SYM_name[i++] = (char)c;
    	    }
    	    SYM_name[i]	= '\0';
    	    CH_Unget_CmtSkip(c);
    	    c =	'A';
    	} else if (isdigit(c)) {
    	    for	(;;) {
    	    	c = CH_Get_CmtSkipEofChk();
    	    	if (isalnum(c) == 0 && c != '_')
    	    	    break;
    	    }
    	    CH_Unget_CmtSkip(c);
    	    c =	'0';
      #if 10
    	} else if (c ==	'"') {	/* �������ǂݔ�΂� */
    	    do {
    	    	c = CH_Skip();
    	    } while (c != '"');
    	    c =	'0';
    	} else if (c ==	'\'') {	/* �����萔��ǂݔ�΂�	 */
    	    CH_Skip();
    	    if ((c = CH_Get_CmtSkipEofChk()) !=	'\'') {
    	    	Err_Src("�i'�萔'�֌W�j");
    	    }
    	    c =	'0';
      #endif
    	}
    }
    return c;
}

static void Mif_Set(int	n)
    /* #if�l�X�g�̏��� */
{
    int	    i;
    static struct MIF {	    	/* #if�l�X�g�����̃X�^�b�N */
    	int 	n;
    	int 	lvl;
    	int 	lno;
    }	mif[MIF_MAX];

  #ifdef DEBUG
    if (Debug_flg != 0 && pass == 0)
    	fprintf(errFp, "   %s %d : #%s ������܂�(���� %d) (Mif_p=%d)\n",
    	    	SRC_name, linNo, Mif_Name[n], level, Mif_p);
  #endif
    if (0 <= n && n <= 2) { 	/* #elif,#else,#endif */
    	i = Mif_p - 1;
    	if (Mif_p == 0)
    	    Err_Src("�i#ϸۊ֌W�j");
    	if (mif[i].lvl != level	&& Flg_msg != 0	&& pass	== 0) {
    	    fprintf(errFp,
    	    	    "   %s %d �` %d : #%s�`#%s �Ԃ�'{'��'}'�̐��������܂���\n",
    	    	    SRC_name, mif[i].lno, linNo, Mif_Name[mif[i].n], Mif_Name[n]);
    	}
    	Mif_p--;
    }
    if (n != 2)	{   	    	/* #elif,#else,#if,#ifdef,#ifndef */
    	if (++Mif_p == MIF_MAX)	{
    	    fprintf(errFp, "   %s %d : #if ����ȽĂ����ق��[�����܂�\n",
    	    	    SRC_name, linNo);
    	    exit(1);
    	}
    	i = Mif_p - 1;
    	mif[i].n = n;
    	mif[i].lvl = level;
    	mif[i].lno = linNo;
    }
}

static int LineTop(void)
    /* #�}�N���̏��� */
{
    static int asmflg =	0;
    int	    c;

    do {
    	c = CH_Get();
    } while (c == ' ' || c == '\t');

    if (c == '#') {
    	c = SYM_GetStr();
    	if (SYM_name[0]	!= '\0') {
    	    if (strcmp("asm", SYM_name)	== 0) {
    	    	while (c != '\n')
    	    	    c =	CH_Get_CmtSkipEofChk();
    	      #ifdef DEBUG
    	    	if (Debug_flg != 0 && pass == 0)
    	    	    fprintf(errFp, "  %s %d : #asm ������܂�\n",
    	    	    	    	    SRC_name, linNo);
    	      #endif
    	    	if (asmflg != 0)
    	    	    Err_Src("�i#asm�֌W�j");
    	    	asmflg = 1;
    	    	do {	    	/* #endasm�܂ł�ǂݔ�΂� */
    	    	    do {
    	    	    	c = CH_Get_ChkEof();
    	    	    } while (c != '\n');
    	    	} while	(LineTop() != 0x7fff);
    	    	asmflg = 0;

    	    } else if (strcmp("endasm",	SYM_name) == 0)	{
    	    	while (c != '\n')
    	    	    c =	CH_Get_CmtSkipEofChk();
    	      #ifdef DEBUG
    	    	if (Debug_flg != 0 && pass == 0)
    	    	    fprintf(errFp, "  %s %d : #endasm ������܂�\n",
    	    	    	    	    SRC_name, linNo);
    	      #endif
    	    	CH_Unget(c);
    	    	return 0x7fff;
    	  #if 1
    	    } else if (strcmp("define",	SYM_name) == 0)	{/*#define�͓��ꏈ��*/
    	    	SYM_name[0] = '\0';
    	    	c = 0xFE;   /* �f�t�@�C����`������킷^^; */
    	    	goto RET;
    	  #endif
    	    } else {
    	    	if ((c = STBL_Search(Mif_Name, 6, SYM_name)) > 0) {
    	    	    Mif_Set(c);
    	    	}
    	    	do {
    	    	    c =	CH_Get_CmtSkipEofChk();
    	    	} while	(c != '\n');
    	    }
    	    c =	'\n';
    	} else {
    	    while (c !=	'\n')
    	    	c = CH_Get_CmtSkipEofChk();
    	}
    }
  RET:
    CH_Unget_CmtSkip(c);
    return 0;
}

static int CH_SpcSkip(void)
    /* �󔒂�ǂ݂Ƃ΂��A�󔒈ȊO�̕�����Ԃ� */
{
    int	    c;

    for	(; ;) {
    	c = CH_Get_CmtSkipEofChk();
    	if (c != ' ' &&	c != '\n' && c != '\t')
    	    break;
    	if (c == '\n')
    	    LineTop();
    }
    return c;
}

void	SYM_GetInit(void)
    /* file��ǂݍ��ނ��Ƃɍs�Ȃ�, SYM_Get���Ăяo���ɂ������Ă̏����� */
{
    SYM_sttNo =	linNo =	1;
    Mif_p = 0;
    unChar = uncharBuf;
    *unChar = 0;
    LineTop();
}


int SYM_GetTypedef(void)
    /* typedef ��`�ɂ����ꂽ�P��i�^���j���A��������P��ꗗ�ɓo�^���� */
{
    int	    i, c;

  #ifdef DEBUG
    if (Debug_flg)
    	fprintf(errFp, "  %s %d : typedef\n", SRC_name,	linNo);
  #endif
    c =	0;
    while (((c = SYM_GetStr()) >= 0)) {
    	/*printf("%c : %s\n",c,	SYM_name);*/
    	if (SYM_name[0]	!= 0) {
    	    char *p;
    	    if (STBL_Search(RSV_name,RSV_cnt,SYM_name) > 0)
    	    	continue;
    	    p =	strdupE(SYM_name);
    	    STBL_Add(RSV_name, &RSV_cnt, p);
    	    c =	' ';

    	} else if (c ==	0xFE) {	/* #define �̂Ƃ� */
    	    SYM_GetStr();
    	    fprintf(errFp,"%s %d : typedef��`�r����#define %s�錾�����邪����.\n", SRC_name, linNo,SYM_name);
    	    do {
    	    	c = CH_Get_CmtSkipEofChk();
    	    } while (c != '\n');
    	    CH_Unget_CmtSkip(c);
    	    c =	' ';

    	} else if (c ==	'[' && level ==	0) {
    	    for	(i = 0;;) { /* [..]��ǂݔ�΂�	*/
    	    	if (c == '[') {
    	    	    ++i;
    	    	} else if (c ==	']') {
    	    	    if ((--i) == 0)
    	    	    	break;
    	    	}
    	    	c = CH_SpcSkip();
    	    }

    	} else if (c ==	'{' && level ==	0) {
    	    for	(i = 0;;) { /* {..}��ǂݔ�΂�	*/
    	    	if (c == '{') {
    	    	    ++i;
    	    	} else if (c ==	'}') {
    	    	    if ((--i) == 0)
    	    	    	break;
    	    	}
    	    	c = CH_SpcSkip();
    	    }
    	} else if (c ==	'\n') {	/* '\n'�Ȃ̂Ő�Ɏ��̍s��#ϸ� */
    	    LineTop();	    	/* �̏���������	    	      */

    	} else if (c ==	';') {
    	    /*CH_Unget_CmtSkip(c);*/
    	    break;

    	} else {
    	    SYM_name[0]	= 0;
    	    c =	'@';
    	}
    }
    if (c < 0)	    	    /* �t�@�C���̏I���Ȃ�NULL��Ԃ� */
    	Err_Eof();
    /*printf("[typedef]end\n");*/
    return c;
}


int SYM_GetStruct(void)
    /* �\���̖����A��������P��ꗗ�ɓo�^���� */
{
    int	    i, c;

  #ifdef DEBUG
    if (Debug_flg)
    	fprintf(errFp, "  %s %d : Struct\n", SRC_name, linNo);
  #endif

    c =	SYM_GetStr();
    if (c < 0)
    	return 0;
    if (SYM_name[0] != 0) {
    	char *p;
    	if (STBL_Search(RSV_name,RSV_cnt,SYM_name) > 0)
    	    return ' ';
    	p = strdupE(SYM_name);
    	STBL_Add(RSV_name, &RSV_cnt, p);
    	c = CH_Get_CmtSkipEofChk();
    }
    while (c ==	0xFE ||	c == '\n') {
    	if (c == 0xFE) {    	    /* #define �̂Ƃ� */
    	    SYM_GetStr();
    	    fprintf(errFp,"%s %d : typedef��`�r����#define %s�錾�����邪����.\n", SRC_name, linNo,SYM_name);
    	    do {
    	    	c = CH_Get_CmtSkipEofChk();
    	    } while (c != '\n');
    	    CH_Unget_CmtSkip(c);
    	    c =	' ';
    	} else {
    	    if (c == '\n') {	/* '\n'�Ȃ̂Ő�Ɏ��̍s��#ϸ� */
    	    	LineTop();  	    /* �̏��������� 	    	  */
    	    }
    	}
    	c = CH_Get_CmtSkipEofChk();
    }
    if (c == '{' && level == 0)	{
    	for (i = 0;;) {	/* {..}��ǂݔ�΂� */
    	    if (c == '{') {
    	    	++i;
    	    } else if (c == '}') {
    	    	if ((--i) == 0)
    	    	    break;
    	    }
    	    c =	CH_SpcSkip();
    	}
    } else {
    	CH_Unget_CmtSkip(c);
    	c = ' ';
    }
    return c;
}



int SYM_Get(void)
   /* �֐����𓾂�
    *	  �֐������݂���� SYM_name �ɂ��̕�����ASYM_linNo �ɂ݂�����
    *	  �s�ԍ����Z�b�g���A�֐��l�Ƃ��ĂP��Ԃ��BEOF�Ȃ� 0��Ԃ��B
    */
{
    int	    i, c,b;
    char    svnam[SYM_SIZ+1];	/* ���O������̂��߂̃o�b�t�@	    	*/

    c =	0;
    while ((b=c,(c = SYM_GetStr()) >= 0)) {
    	/*printf("%x {%s}\n",c,SYM_name);*/
    	SYM_atr	= AT_NON;

    	if (c == 0xFE) {    /* #define �̂Ƃ� */
    	    c =	SYM_GetStr();
    	    strcpy(svnam,SYM_name);
    	    /*printf("%02x %s %d\n",c,svnam,STBL_Search(RSV_name,RSV_cnt,svnam));*/
    	    if (svnam[0]!=0 && STBL_Search(RSV_name,RSV_cnt,svnam) <= 0) {
    	    	SYM_atr	= AT_MAC;
    	    	SYM_linNo = linNo;
    	    	do {
    	    	    c =	CH_Get_CmtSkipEofChk();
    	    	} while	(c != '\n');
    	    	CH_Unget_CmtSkip(c);
    	    	c = 'A';
    	    	strcpy(SYM_name,svnam);
    	      #ifdef DEBUG
    	    	if (Debug_flg != 0 && pass == 0) {
    	    	    fprintf(errFp, "  %s %d : #define %s\n",
    	    	    	    SRC_name, linNo, SYM_name);
    	    	}
    	      #endif
    	    } else {
    	    	do {
    	    	    c =	CH_Get_CmtSkipEofChk();
    	    	} while	(c != '\n');
    	    	CH_Unget_CmtSkip(c);
    	    	c = ' ';
    	    }
    	    break;

    	} else if (SYM_name[0] != 0) {
    	  #if 10
    	    if (pass ==	0) {	/*�^��`�Ő錾���ꂽ�^�͖�������P��ɂ���*/
    	    	if (strcmp(SYM_name,"typedef") == 0) {
    	    	    if ((c = SYM_GetTypedef()) == 0)
    	    	    	break;
    	    	    continue;
    	    	} else if (strcmp(SYM_name,"struct") ==	0) {
    	    	    if ((c = SYM_GetStruct()) == 0)
    	    	    	break;
    	    	    continue;
    	    	} else if (strcmp(SYM_name,"union") == 0) {
    	    	    if ((c = SYM_GetStruct()) == 0)
    	    	    	break;
    	    	    continue;
    	    	} else if (strcmp(SYM_name,"enum") == 0) {
    	    	    if ((c = SYM_GetStruct()) == 0)
    	    	    	break;
    	    	    continue;
    	    	}
    	    }
    	  #endif

    	    if (STBL_Search(RSV_name,RSV_cnt,SYM_name) > 0)
    	    	continue;

    	    SYM_linNo =	linNo;
    	    strcpy(svnam, SYM_name);
    	    c =	CH_SpcSkip();
    	    if (c == '(') {
    	      KKK:;
    	    	SYM_atr	= AT_FUNC;
    	    	if (level == 0)	{
    	    	    for	(i = 0;;) { /* ��������ǂݔ�΂� */
    	    	    	if (c == '(') {
    	    	    	    ++i;
    	    	    	} else if (c ==	')') {
    	    	    	    if ((--i) == 0)
    	    	    	    	break;
    	    	    	}
    	    	    	c = CH_SpcSkip();
    	    	    }
    	    	    c =	CH_SpcSkip();
    	    	    CH_Unget_CmtSkip(c);
    	    	    if (c == ',' || c == ';' ||	c == '(' || c == '=') {
    	    	    	SYM_atr	= AT_DECL;
    	    	    }
    	    	}
    	    } else {	/* �ϐ����Ȃɂ�	*/
    	    	if (b == '(' &&	c == ')') {
    	    	    c =	CH_SpcSkip();
    	    	    if (c == '(')
    	    	    	goto KKK;
    	    	}
    	    	CH_Unget_CmtSkip(c);
    	    	c = 'A';
    	    	SYM_atr	= AT_VAR;
    	    	if (Flg_symMode	== 0) {	/* ����, �o�^���͊֐��ȊO�͖���	*/
    	    	    continue;	    	/* -b �w�莞�́A�֐��ȊO���o�^ */
    	    	}   	    	    	/* �Q�Ə����ł�(..) �̗L���͖��� */
    	    }
    	    strcpy(SYM_name,svnam);
    	    c =	'A';
    	    break;

    	} else if (c ==	'[' && level ==	0) {
    	    for	(i = 0;;) { /* ��������ǂݔ�΂� */
    	    	if (c == '[') {
    	    	    ++i;
    	    	} else if (c ==	']') {
    	    	    if ((--i) == 0)
    	    	    	break;
    	    	}
    	    	c = CH_SpcSkip();
    	    }

      #if 0
    	} else if (c ==	'"') {	/* �������ǂݔ�΂� */
    	    do {
    	    	c = CH_Skip();
    	    } while (c != '"');
    	} else if (c ==	'\'') {	/* �����萔��ǂݔ�΂�	 */
    	    CH_Skip();
    	    if ((c = CH_Get_CmtSkipEofChk()) !=	'\'') {
    	    	Err_Src("�i'�萔'�֌W�j");
    	    }
      #endif
    	} else if (c ==	'\n') {	/* '\n'�Ȃ̂Ő�Ɏ��̍s��#ϸ� */
    	    LineTop();	    	/* �̏���������	    	      */

    	} else if (c ==	'{') {
    	    level++;
    	  #ifdef DEBUG
    	    if (Debug_flg != 0 && pass == 0)
    	    	fprintf(errFp, "  %s %d : '{' level %d\n",
    	    	    	    	SRC_name, linNo, level);
    	  #endif

    	} else if (c ==	'}') {
    	  #ifdef DEBUG
    	    if (Debug_flg != 0 && pass == 0)
    	    	fprintf(errFp, "  %s %d : '}' level %d\n",
    	    	    	    	SRC_name, linNo, level);
    	  #endif
    	    level--;
    	    if (level == 0 && SYM_sttNo	== 0)
    	    	SYM_sttNo = linNo + 1;

    	} else {
    	    SYM_name[0]	= 0;
    	    if (c != '(')
    	    	c = '@';
    	}
    }

  #ifdef DEBUG
    if (Debug_flg != 0 && Flg_msg != 0)
    	if (c >= 0)
    	    fprintf(errFp, "  %s %d : SYM_Get '%s'(size %d) level %d\n",
    	    	    SRC_name, SYM_linNo, SYM_name, strlen(SYM_name), level);
    	else
    	    fprintf(errFp, "  %s %d : EOF\n", SRC_name,	linNo);
  #endif

    if (c < 0) {    	    	/* �t�@�C���̏I���Ȃ�NULL��Ԃ� */
    	if (level != 0)
    	    Err_Eof();
    	SYM_name[0] = '\0';
    	return 0;
    }
    /*printf("[%s]\n",SYM_name);*/

    return 1;
}



/*---------------------------------------------------------------------------*/

static void AddMultDefPos(TNODE	*p, char *fname, int lno)
    /* ���d��`���L�^���� */
{
    MULTDEF *m;
    m =	callocE(1,sizeof (MULTDEF));
    m->next    = p->multDef;
    m->filName = fname;
    m->defLin  = lno;
    p->multDef = m;
}

static int TREE_Add(TNODE *pp, int lrF)
    /* �֐�����o�^ */
{
    REFLIST *q;
    int	l,r,nl,nr;
    int	 b;
    TNODE *p,*lp,*lrp,*lrlp,*lrrp;

    if (pp == NULL)
    	return 0;
    p =	pp->link[lrF];
    if (p == NULL) {
      #ifdef DEBUG
    	if (Debug_flg != 0)
    	    fprintf(errFp, "   TREE_Add: %s\n",	SYM_name);
      #endif
    	TREE_curNode = pp->link[lrF] = p = callocE(1,sizeof(TNODE));
    	p->symName = strdupE(SYM_name);
    	if (level == 0)	{
    	    p->filName = SRC_name;
    	    p->defLin =	(int) SYM_linNo;
    	    p->bgnLin =	(int) SYM_sttNo;
    	    p->atr    =	(short)	SYM_atr;
    	    SYM_sttNo =	0;
    	    q =	callocE(1,sizeof(REFLIST));
    	    q->nodePnt = p;
    	    q->next = NULL;
    	    if (refl0) {
    	    	refl0->next = q;
    	    	refl0 =	q;
    	    }
    	} else {
    	    p->filName = "----";
    	    p->defLin =	0;
    	    p->bgnLin =	0;
    	}
      #if 0
    	p->refGrp = p->refLin =	0;
    	p->calls  = p->caller =	NULL;
    	p->avltFlg = 0;
    	p->link[1]  = p->link[0]   = NULL;
      #endif
    	return 3;
    }
    b =	strcmp(SYM_name, p->symName);
    if (b == 0)	{   /* �������O���������� */
    	TREE_curNode = NULL;
    	if (level == 0 && pass == 0) {
    	    /* ����`�֐��Ȃ�o�^���Ȃ��� */
    	    if (p->defLin == 0 || (p->atr == AT_DECL) || (p->atr == AT_CMNT)) {
    	    	AddMultDefPos(p, p->filName, p->defLin);
    	    	p->filName = SRC_name;
    	    	p->defLin  = SYM_linNo;
    	    	p->atr	   = (short) SYM_atr;
    	  #if 1
    	    } if (SYM_atr == AT_FUNC &&	p->atr != AT_FUNC) {
    	    	AddMultDefPos(p, p->filName, p->defLin);
    	    	p->filName = SRC_name;
    	    	p->defLin  = SYM_linNo;
    	    	p->atr	   = (short) SYM_atr;
    	  #endif
    	    } else {
    	    	fprintf(errFp, "  %s %d : %s���d����`�ł�\n",
    	    	    SRC_name, SYM_linNo, SYM_name);
    	    	AddMultDefPos(p, SRC_name, SYM_linNo);
    	    }
    	}
    	return 0;
    }
    if (b < 0) {/* �� */
    	l = 0;	r = 1;	nl = 1;	nr = 2;
    } else {	/* �E */
    	l = 1;	r = 0;	nl = 2;	nr = 1;
    }

    b =	TREE_Add(p,l);
    if (b == 0)
    	return 0;

    if (p->avltFlg == nr) {
    	p->avltFlg = 0;
    	return 0;
    } else if (p->avltFlg == 0)	{
    	p->avltFlg = nl;
    	return p->avltFlg;
    }
    if (b == nl) {
    	lp  	    = p->link[l];
    	lrp 	    = lp->link[r];
    	p->link[l]  = lrp;
    	p->avltFlg  = 0;
    	lp->link[r] = p;
    	lp->avltFlg = 0;
    	pp->link[lrF] =	lp;
    } else if (b == nr)	{
    	lp  	    = p->link[l];
    	lrp 	    = lp->link[r];
    	lrlp	    = lrp->link[l];
    	lrrp	    = lrp->link[r];
    	pp->link[lrF] =	lrp;
    	p->link[l]  = lrrp;
    	lp->link[r] = lrlp;
    	lrp->link[l]= lp;
    	lrp->link[r]= p;
    	lp->avltFlg = p->avltFlg = 0;
    	if (lrp->avltFlg == nl)
    	    p->avltFlg = nr;
    	else if	(lrp->avltFlg == nr)
    	    lp->avltFlg	= nl;
    	lrp->avltFlg = 0;
    } else {
    	printf("PRGERR:InsertNode avltFlg = 3\n");
    	exit(1);
    }
    return 0;
}

TNODE *TREE_Serch(TNODE	* p)
    /* �؂��֐��������� */
{
    int	    f;

    if (p == NULL) {
      #ifdef DEBUG
    	if (Debug_flg != 0 && Flg_msg != 0)
    	    fprintf(errFp, "  Not Found: %s\n",	SYM_name);
      #endif
    	return NULL;
    }
    f =	strcmp(SYM_name, p->symName);
    return (f <	0) ? TREE_Serch(p->link[0]) : (f > 0) ?	TREE_Serch(p->link[1]) : p;
}


REFLIST	*MakSmTree(int a_flg)
    /* (��`���ꂽ)�֐�����؂ɓo�^ */
{
    REFLIST r;
    TNODE t;

    SRC_fp = fopenE(SRC_name, "r");

    t.link[0] =	TREE_root;
    t.avltFlg =	0;
    pass = level = 0;
    memset(&r, 0, sizeof r);
    refl0 = &r;
    SYM_GetInit();
    while (SYM_Get()) {
    	if ((SYM_name[0] != 0) && (level == 0 || a_flg)) {
    	    TREE_Add(&t, 0);
    	}
    }
    TREE_root =	t.link[0];
    fclose(SRC_fp);
    return r.next;
}

void	MakRefLst(void)
    /* �֐��̎Q�Ƃ̃��X�g���쐻	*/
{
    TNODE *p1;
    TNODE *p2;
    REFLIST *q1;
    REFLIST *q2;

    SRC_fp = fopenE(SRC_name, "r");

    pass = 1;
    level = 0;
    Flg_symMode	= 1;
    SYM_GetInit();
    p1 = p2 = NULL;
    while (SYM_Get()) {
    	if (SYM_name[0]	!= '\0') {
    	    if (level == 0) {
    	    	p1 = TREE_Serch(TREE_root); /* p1�F�Ăяo���葱��    */
    	    	p2 = NULL;
    	    } else {
    	    	p2 = TREE_Serch(TREE_root); /* p2�F�Ăяo�����葱�� */
    	    }
    	}
    	if (p1 != NULL && p2 !=	NULL) {
    	    /* p1�ɂ��p2�̌Ăяo����p1��calls���X�g�ɓo�^ */
    	    q1 = callocE(1,sizeof(REFLIST));
    	  #ifdef DEBUG
    	    if (Debug_flg != 0)	{
    	    	fprintf(errFp, "   %s calls %s\n",
    	    	    	p1->symName, p2->symName);
    	    }
    	  #endif
    	    q1->nodePnt	= p2;
    	    q1->next = NULL;
    	    if (p1->calls == NULL) {
    	    	p1->calls = q1;
    	    } else {
    	    	q2 = p1->calls;
    	    	while (q2->next	!= NULL)
    	    	    q2 = q2->next;
    	    	q2->next = q1;
    	    }
    	    /* p1�ɂ��p2�̌Ăяo����p2��called���X�g�ɓo�^ */
    	  #ifdef DEBUG
    	    if (Debug_flg != 0)
    	    	fprintf(errFp, "   %s is called by %s\n",
    	    	    	p2->symName, p1->symName);
    	  #endif
    	    q1 = callocE(1,sizeof(REFLIST));
    	    q1->nodePnt	= p1;
    	    q1->next = NULL;
    	    if (p2->caller == NULL) {
    	    	p2->caller = q1;
    	    } else {
    	    	q2 = p2->caller;
    	    	while (q2->next	!= NULL)
    	    	    q2 = q2->next;
    	    	q2->next = q1;
    	    }
    	}/* endif */
    }/*	endwhile */
    fclose(SRC_fp);
}


/*----------------------------------------------------------------------------*/
FILE *outFp;

static void PriFiNo(char * files, int no)
    /* �t�@�C����,�s�ԍ��̕\�� */
{
    int	    n;

    if (files == NULL)
    	files =	"";
    n =	filNamSiz - strlen(files);
    /*printf("[%d,%d]",filNamSiz,n);*/
    if (n > 0) {
    	while (--n >= 0)
    	    fprintf(outFp, " ");
    }
    fprintf(outFp,"%s %5d : ", files, no);
}

static int  SchNodePnt(REFLIST * q)
    /* REFLIST_T���X�g�œ����֐������ɂ��邩�ǂ��� */
{
    REFLIST *n;

    n =	q;
    while ((n =	n->next) != NULL) {
    	if (q->nodePnt == n->nodePnt)
    	    return 0;
    }
    return 1;
}


static int  ChkOya(TNODE * pp, TNODE * cp)
    /*'�q'�֐����ďo�����֐��ɐꑮ���邩�ǂ���,'�q'�֐���'�e'�ł����邩�ǂ���*/
{
    REFLIST *q;
    int	    n;

    n =	1;
    q =	cp->caller;
    while (q !=	NULL) {
    	if (q->nodePnt != pp &&	q->nodePnt != cp) {
    	    n =	0;
    	    break;
    	}
    	q = q->next;
    }
    q =	cp->calls;
    while (q !=	NULL) {
    	if (q->nodePnt == pp)
    	    return n + 2;
    	q = q->next;
    }
    return n;
}


static int  ChkGrobalName(TNODE	*p)
{
    MULTDEF *m;
    REFLIST *q;

    for	(m = p->multDef; m != NULL; m =	m->next) {
    	if (p->filName != m->filName)
    	    goto NNN;
    }
    q =	p->caller;
    while (q !=	NULL) {
    	if (SchNodePnt(q)) {
    	    if (p->filName != q->nodePnt->filName)
    	    	goto NNN;
    	}
    	q = q->next;
    }
    return 0;
 NNN:
    return 1;
}


static int  SchRootf(char * s)
    /* �w�肳�ꂽ�֐������܂��\������ĂȂ�tree�\���̃��[�g�ɂȂ�֐������H */
{
    UCHAR   f;
    int	    n, i;
    char    *p;

    for	(n = GRP_no, i = ROOT_grp + 1; i < ROOT_cnt; i++) {
    	p = ROOT_name[i];
    	f = 0;
    	if (*p == '+') {    	/* '+'���֐����̂Ƃ� */
    	    p++;
    	    f =	1;
    	} else {    	    	/* �����łȂ��Ƃ��� �O���|�v�ԍ����X�V */
    	    n++;
    	}
    	if ((*s	== *p) && (strcmp(s, p)	== 0)) { /* �݂������Ƃ� */
    	    if (f == 0)	return n;   	/* �O���[�v�ԍ�	�������� */
    	    else    	return -n;  	/* '+'���֐����Ȃ畉�ɂ��� */
    	}
    }
    return 0;	    	    	/* �݂���Ȃ����� */
}

#ifdef GRPN_ON	/* �o�O���Ă� */
static void CountGrpSubNoSub(REFLIST * q, int f)
{
    int	    n, called;
    TNODE   *p;

    p =	q->nodePnt;
    called = p->refGrp;
    if (p->defLin == 0)
    	called = 1;
    if (called == 0) {
    	if (f == 0 && (n = SchRootf(p->symName)) != 0) {
    	    if (n > 0) {
    	    	p->refGrp = (short) n;
    	    	p->refLin = 0;
    	    } else {
    	    	p->refGrp = (short) (-n);
    	    	p->refLin = -1;
    	    }
    	    called = 1;
    	}
    }
    if (f)
    	called = 0;
    if (called == 0) {	    	/* (p->refGrp == 0) */
    	p->refGrp = (short) GRP_no;
    	p->refLin = (int)   GRP_subNo++;
    	for (q = p->calls; q !=	NULL; q	= q->next) {
    	    if (Flg_dspAllLine || SchNodePnt(q))
    	    	CountGrpSubNoSub(q, 0);
    	}
    }
}

void	CountGrpSubNo(void)
    /* �O���[�v���ł̖��O�� No.	�����肷�� */
{
    REFLIST r;

    GRP_no = 0;
    for	(ROOT_grp = 0; ROOT_grp	< ROOT_cnt; ROOT_grp++)	{
    	if (*(ROOT_name[ROOT_grp]) == '+') {
    	    strncpy(SYM_name, ROOT_name[ROOT_grp]+1, SYM_SIZ);
    	} else {
    	    GRP_subNo =	0;
    	    ++GRP_no;
    	    strncpy(SYM_name, ROOT_name[ROOT_grp], SYM_SIZ);
    	}
    	SYM_name[SYM_SIZ] = '\0';
    	/*    printf("%2d.%04d %s\n", ROOT_grp,	GRP_subNo, SYM_name);*/
    	r.nodePnt = TREE_Serch(TREE_root);
    	if (r.nodePnt != NULL) {
    	    r.next = NULL;
    	    CountGrpSubNoSub(&r, 1);
    	} else {
    	    fprintf(errFp, "\n%s() ������܂���\n", SYM_name);
    	}
    }
}
#endif



static void PriFuNu(char *fncs,	int atr, int grp, int nn, int siz, int g,int h,char *cmnt)
    /* �֐���,tree�\���ł̔ԍ��̕\�� */
{
    #define TAB	16
    int	    n, i;
    static char	s[6];

    fprintf(outFp,"%s %s", AT_str[atr],	fncs);
    if (grp > 0) {
    	if (nn >= 0)
    	    sprintf(s, "%d.%04d", grp, nn);
    	else
    	    sprintf(s, "%d.????", grp);
    	n = strlen(s);
    } else {
    	n = 0;
    }
    n += strlen(fncs);
    i =	TAB - (n + siz)	% TAB;
    if (n + siz	< TAB *	2)
    	i += TAB;
    n =	i;
    if (grp > 0) {
    	while (--n >= 0)
    	    fprintf(outFp," ");
    	fprintf(outFp,"%s", s);
    }
    if (g) {
    	fprintf(outFp," ��");
    	i += 3;
    }
    if (h) {
    	fprintf(outFp," �");
    	i += 3;
    }
    if (cmnt)
    	fprintf(outFp,"\t // %s",cmnt);
    fprintf(outFp,"\n");
}


static void PriFlow(REFLIST * q, char *	s)
   /* �֐�����tree�\��
    * q�̏����Œi�t�����ĕ\������
    * �\���ς݂̎葱���͕\�������s��refLin�ɕۑ����ȉ��̃��x���͕\�����Ȃ�
    */
{
    int	    called;
    int	    n,bflg;
    TNODE   *p;

    p =	q->nodePnt;
    called = p->refGrp;
    if (p->defLin == 0)
    	called = 1;

    /* Flg_dspTrFuncOnly=1�̂Ƃ��A�֐��ȊO��'�q��'�����Ȃ��΂����͕\�����Ȃ� */
    bflg = (Flg_dspTrFuncOnly
    	    && (p->atr==AT_VAR||p->atr==AT_MAC)
    	    && p->calls==NULL);

    if (called == 0) {
    	if (s != linBuf	&& (n =	SchRootf(p->symName)) != 0) {
    	  #ifndef GRPN_ON   /* �o�O���Ă� */
    	    if (n > 0) {
    	    	p->refGrp = (short) n;
    	    	p->refLin = 0;
    	    } else {
    	    	p->refGrp = (short) (-n);
    	    	p->refLin = -1;
    	    }
    	  #endif
    	    called = 1;
    	}
    }
    if (s == linBuf)
    	called = 0;
    if (/*Flg_dspTree &&*/ !bflg)
    	PriFiNo(p->filName, p->defLin);
  #ifndef GRPN_ON   /* �o�O���Ă� */
    if (called == 0) {	    	/* (p->refGrp == 0) */
    	p->refGrp = (short) GRP_no;
    	p->refLin = (int)   GRP_subNo++;
    }
  #endif
    if (/*Flg_dspTree &&*/ !bflg) {
    	if (called == 0)
    	    fprintf(outFp,"%2d.%04d ", p->refGrp, p->refLin);
    	else
    	    fprintf(outFp,"        ");
    	n = 6;
    	if ((s + 4) - linBuf < LINBUF_SIZ) {
    	    strcpy(s, "|--");
    	} else {
    	    fprintf(errFp,"�֐��̌ďo�֌W���[�����܂�. tree�𕪊����Ă�������.\n");
    	    exit(1);
    	}
    	if (s >= linBuf	+ 3) {
    	    fprintf(outFp,linBuf + 3);
    	    n += strlen(linBuf + 3);
    	}
    	*s = '\0';
    	PriFuNu(p->symName, p->atr, (int) (called ? p->refGrp :	0),
    	    	(int)(p->refLin), n, ChkGrobalName(p), (p->multDef!=NULL), p->cmnt);
    }
    if (called)
    	return;
    if (/*Flg_dspTree &&*/ !bflg) {
    	/* �Ăяo���葱���̕\��	*/
    	if (q->next == NULL)
    	    strcat(linBuf, "   ");
    	else
    	    strcat(linBuf, "|  ");
    }
    q =	p->calls;
    while (q !=	NULL) {
    	if (Flg_dspAllLine || SchNodePnt(q))
    	    PriFlow(q, s + 3);
    	q = q->next;
    }
    if (/*Flg_dspTree &&*/ Flg_karagyo && !bflg) {
    	if (*s == '|') {
    	    for	(n = 8 + FINO_OFS + filNamSiz; --n >= 0;)
    	    	fprintf(outFp," ");
    	    fprintf(outFp,"%s\n", linBuf + 3);
    	}
    	*s = '\0';
    }
}

void	DispTreeFlow(void)
   /* �Ăяo���֌W��tree�\������ */
{
    REFLIST r;

    /*if (Flg_dspTree)*/
    	fprintf(outFp,"\n�Ăяo���֌W\n");
    GRP_no = 0;
    for	(ROOT_grp = 0; ROOT_grp	< ROOT_cnt; ROOT_grp++)	{
    	if (*(ROOT_name[ROOT_grp]) == '+') {
    	    /*(ROOT_name[ROOT_grp])++;*/
    	    if (/*Flg_dspTree &&*/ Flg_karagyo)
    	    	fprintf(outFp,"\n");
    	    strncpy(SYM_name, (ROOT_name[ROOT_grp])+1, SYM_SIZ);
    	} else {
    	    GRP_subNo =	0;
    	    /*if (Flg_dspTree)*/
    	    	fprintf(outFp,"\nGroup%4d\t%s\n", GRP_no+1, GRP_cmnt[GRP_no]);
    	    ++GRP_no;
    	    strncpy(SYM_name, ROOT_name[ROOT_grp], SYM_SIZ);
    	}
    	SYM_name[SYM_SIZ] = '\0';
    	r.nodePnt = TREE_Serch(TREE_root);
    	if (r.nodePnt != NULL) {
    	    /* fprintf(outFp,"\n'%s()'\n",ROOT_name[ROOT_grp]);	*/
    	    r.next = NULL;
    	    PriFlow(&r,	linBuf);
    	} else {
    	    fprintf(errFp, "\n%s() ������܂���\n", SYM_name);
    	}
    }
    /*if (Flg_dspTree)*/
    	fprintf(outFp,"\n");
}




static void PriFuNuOyako(char *	fncs, int atr, int grp,	int nn,	/*int sz,*/ int	cf,char	*cmnt)
    /* �֐���,tree�\���ł̔ԍ��̕\�� */
{
  #if 1
    if (grp > 0) {
    	if (nn >= 0)
    	    fprintf(outFp,"%3d.%04d ", grp, nn);
    	else
    	    fprintf(outFp,"%3d.???? ", grp);
    } else if (grp == 0) {
    	    fprintf(outFp,"         ");
    }
    fprintf(outFp,"%s %s %s", cf?"��":"  ", AT_str[atr], fncs?fncs:"");
    if (cmnt)
    	fprintf(outFp,"\t // %s",cmnt);
  #else
    #define TAB	16
    int	    n, i;
    static char	s[6];

    fprintf(outFp,"%s %s", AT_str[atr],	fncs);
    if (grp > 0) {
    	if (nn >= 0)
    	    sprintf(s, "%d.%04d", grp, nn);
    	else
    	    sprintf(s, "%d.????", grp);
    	n = strlen(s);
    } else {
    	n = 0;
    }
    n += strlen(fncs);
    i =	TAB - (n + sz) % TAB;
    if (n + sz < TAB * 2)
    	i += TAB;
    if (grp > 0) {
    	while (--i >= 0)
    	    fprintf(outFp," ");
    	fprintf(outFp,"%s", s);
    }
    if (cf)
    	fprintf(outFp,"static");
  #endif
    fprintf(outFp,"\n");
}



/*----------------------------------------*/
static int priFuncAtr;

static void PriFunc(TNODE * p)
   /* �e�֐��̐e�q�̕\�� sub */
{
    TNODE *z;
    REFLIST *q;
    static char	ch[4][8] = {
    	"      ",
    	"  ����",
    	"    ��",
    	"������",
    };

    if (p == NULL)
    	return;

    PriFunc(p->link[0]);
    if (Flg_dspUndef !=	0) {
    	if (p->defLin != 0)
    	    goto NEXT;
    	else if	(Flg_dspOyakoTree != 0 && p->refGrp == 0)
    	    goto NEXT;
    }

    if (  (priFuncAtr == 0 && p->atr !=	AT_VAR)
    	||(priFuncAtr == 1 && p->atr !=	AT_DECL)
    	||(priFuncAtr == 2 && p->atr !=	AT_FUNC)
    	||(priFuncAtr == 3 && p->atr !=	AT_MAC)	)
    {
    	goto NEXT;
    }

    PriFiNo(p->filName,	p->defLin);
    /* fprintf(outFp,"%5d ", funcNo); */
    PriFuNuOyako(p->symName, p->atr, p->refGrp,	p->refLin, /*6,*/ ChkGrobalName(p),p->cmnt);
  #if 1
    if ((Flg_dspOyakoMode & 0x03) != 3)	{
    	MULTDEF	*m;
    	for (m = p->multDef; m != NULL;	m = m->next) {
    	    if (p->filName) {
    	    	PriFiNo(m->filName, m->defLin);
    	    	fprintf(outFp,"          �d����`?\n");
    	    }
    	}
    }
  #endif
    funcNo++;
    if ((Flg_dspOyakoMode & 0x02) == 0)	{
    	q = p->caller;
    	while (q != NULL) {
    	    z =	q->nodePnt;
    	    if (SchNodePnt(q) && z) {
    	    	PriFiNo(z->filName, z->defLin);
    	    	fprintf(outFp,"        %2s�e ",
    	    	       ((p->filName == q->nodePnt->filName) ? "��" : "��"));
    	    	PriFuNuOyako(z->symName, z->atr, -1, 0,	/*13,*/	ChkGrobalName(z), z->cmnt);
    	    }
    	    q =	q->next;
    	}
    }
    if ((Flg_dspOyakoMode & 0x01) == 0)	{
    	q = p->calls;
    	while (q != NULL) {
    	    z =	q->nodePnt;
    	    if (SchNodePnt(q) && z) {
    	    	PriFiNo(z->filName, z->defLin);
    	    	fprintf(outFp,"    %6s%2s ", ch[ChkOya(p, q->nodePnt)],
    	    	       ((p->filName == q->nodePnt->filName) ? "��" : "�q"));
    	    	PriFuNuOyako(z->symName, z->atr, -1, 0,	/*13,*/	ChkGrobalName(z),z->cmnt);
    	    }
    	    q =	q->next;
    	}
    }
    if ((Flg_dspOyakoMode & 0x03) != 3)
    	fprintf(outFp,"\n");
  NEXT:
    PriFunc(p->link[1]);
}

void	DispOyakoFlow(void)
   /*
    * �e���O�̐e�q�֌W��\��
    */
{
    fprintf(outFp,"\n�ϐ��̐e�q�֌W\n\n");
    funcNo = 1;
    priFuncAtr = 0;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\t�ϐ� %d ��\n",funcNo-1);
    fprintf(outFp,"\n�錾�̂݊֐��̐e�q�֌W\n\n");
    funcNo = 1;
    priFuncAtr = 1;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\t�錾�̂݊֐� %d ��\n",funcNo-1);
    fprintf(outFp,"\n�֐��̐e�q�֌W\n\n");
    funcNo = 1;
    priFuncAtr = 2;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\t�֐� %d ��\n",funcNo-1);
    fprintf(outFp,"\n#define�}�N���̐e�q�֌W\n\n");
    funcNo = 1;
    priFuncAtr = 3;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\t�}�N�� %d ��\n",funcNo-1);
}


static void PriNameOnly(TNODE *p)
   /* �e���O�̐e�q�̕\�� sub */
{
    if (p == NULL)
    	return;
    PriNameOnly(p->link[0]);
    if (Flg_dspUndef !=	0) {
    	if (p->defLin != 0)
    	    goto NEXT;
    	else if	(Flg_dspOyakoTree != 0 && p->refGrp == 0)
    	    goto NEXT;
    }

    if (  (priFuncAtr == 0 && p->atr !=	AT_VAR)
    	||(priFuncAtr == 1 && p->atr !=	AT_DECL)
    	||(priFuncAtr == 2 && p->atr !=	AT_FUNC)
    	||(priFuncAtr == 3 && p->atr !=	AT_MAC)	)
    {
    	goto NEXT;
    }
    funcNo++;
    if (p->cmnt)
    	fprintf(outFp,"%-31s\t%s\n",p->symName,	p->cmnt);
    else
    	fprintf(outFp,"%s\n",p->symName);
  NEXT:
    PriNameOnly(p->link[1]);
}


void	DispNameList(int f)
   /*
    * �ϐ��A�֐��A�}�N���ꗗ��\��
    */
{
    fprintf(outFp,"\n; �ϐ��ꗗ\n");
    funcNo = 1;
    priFuncAtr = 0;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\t�ϐ� %d ��\n",funcNo-1);

    fprintf(outFp,"\n; �錾�݂̂̊֐��ꗗ\n");
    funcNo = 1;
    priFuncAtr = 1;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\t�錾�̂݊֐� %d ��\n",funcNo-1);

    fprintf(outFp,"\n; �֐��ꗗ\n");
    funcNo = 1;
    priFuncAtr = 2;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\t�֐� %d ��\n",funcNo-1);

    fprintf(outFp,"\n; #define�}�N���ꗗ\n");
    funcNo = 1;
    priFuncAtr = 3;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\t�}�N�� %d ��\n",funcNo-1);
}

static void PriFuncSm(TNODE * p)
   /* �e�֐��̐e�q�̕\�� sub */
{
    TNODE *z;
    REFLIST *q;
    static char	ch[4][8] = {
    	"      ",
    	"  ����",
    	"    ��",
    	"������",
    };

    if (p == NULL)
    	return;
    if (Flg_dspUndef !=	0) {
    	if (p->defLin != 0)
    	    goto NEXT;
    	else if	(Flg_dspOyakoTree != 0 && p->refGrp == 0)
    	    goto NEXT;
    }

    if (  (priFuncAtr == 0 && p->atr !=	AT_VAR)
    	||(priFuncAtr == 1 && p->atr !=	AT_DECL)
    	||(priFuncAtr == 2 && p->atr !=	AT_FUNC)
    	||(priFuncAtr == 3 && p->atr !=	AT_MAC)	)
    {
    	goto NEXT;
    }

    PriFiNo(p->filName,	p->defLin);
    /* fprintf(outFp,"%5d ", funcNo); */
    PriFuNuOyako(p->symName, p->atr, p->refGrp,	p->refLin, /*6,*/ ChkGrobalName(p),p->cmnt);
  #if 0
    if ((Flg_dspOyakoMode & 0x03) != 3)	{
    	MULTDEF	*m;
    	for (m = p->multDef; m != NULL;	m = m->next) {
    	    PriFiNo(m->filName,	m->defLin);
    	    fprintf(outFp,"          �d����`?\n");
    	}
    }
  #endif
    funcNo++;
    if ((Flg_dspOyakoMode & 0x02) == 0)	{
    	q = p->caller;
    	while (q != NULL) {
    	    z =	q->nodePnt;
    	    if (SchNodePnt(q)) {
    	    	PriFiNo(z->filName, z->defLin);
    	    	fprintf(outFp,"        %2s�e ",
    	    	       ((p->filName == q->nodePnt->filName) ? "��" : "��"));
    	    	PriFuNuOyako(z->symName, z->atr, -1, 0,	/*13,*/	ChkGrobalName(z),z->cmnt);
    	    }
    	    q =	q->next;
    	}
    }
    if ((Flg_dspOyakoMode & 0x01) == 0)	{
    	q = p->calls;
    	while (q != NULL) {
    	    z =	q->nodePnt;
    	    if (SchNodePnt(q)) {
    	    	PriFiNo(z->filName, z->defLin);
    	    	fprintf(outFp,"    %6s%2s ", ch[ChkOya(p, q->nodePnt)],
    	    	       ((p->filName == q->nodePnt->filName) ? "��" : "�q"));
    	    	PriFuNuOyako(z->symName, z->atr, -1, 0,	/*13,*/	ChkGrobalName(z),z->cmnt);
    	    }
    	    q =	q->next;
    	}
    }
    /* if ((Flg_dspOyakoMode & 0x03) !=	3)
    	fprintf(outFp,"\n");*/
  NEXT:;
}

static void PriNameOnlySm(TNODE	* p)
    /* �e���O�̐e�q�̕\�� sub */
{
    if (p == NULL)
    	return;
    if (Flg_dspUndef !=	0) {
    	if (p->defLin != 0)
    	    goto NEXT;
    	else if	(Flg_dspOyakoTree != 0 && p->refGrp == 0)
    	    goto NEXT;
    }

    if (  (priFuncAtr == 0 && p->atr !=	AT_VAR)
    	||(priFuncAtr == 1 && p->atr !=	AT_DECL)
    	||(priFuncAtr == 2 && p->atr !=	AT_FUNC)
    	||(priFuncAtr == 3 && p->atr !=	AT_MAC)	)
    {
    	goto NEXT;
    }
    if (p->cmnt)
    	fprintf(outFp,"%-31s\t%s\n",p->symName,	p->cmnt);
    else
    	fprintf(outFp,"%s\n",p->symName);
  NEXT:;
}



void	DispFileSmList(int f)
    /* �\�[�X�t�@�C���P�ʂŁA�ϐ��A�֐��A�}�N���ꗗ��\�� */
{
    SRCL    *s;
    REFLIST *q;
    TNODE   *p;

    fprintf(outFp,"\n\n; �t�@�C�����Ƃ̖��O���X�g\n\n");
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	fprintf(outFp,"\n;file %s\n", s->name);
    	priFuncAtr = 0;
    	for (q = s->func; q != NULL; q = q->next) {
    	    p =	q->nodePnt;
    	    if (f) PriNameOnlySm(p); else PriFuncSm(p);
    	}
    	priFuncAtr = 1;
    	for (q = s->func; q != NULL; q = q->next) {
    	    p =	q->nodePnt;
    	    if (f) PriNameOnlySm(p); else PriFuncSm(p);
    	}
    	priFuncAtr = 2;
    	for (q = s->func; q != NULL; q = q->next) {
    	    p =	q->nodePnt;
    	    if (f) PriNameOnlySm(p); else PriFuncSm(p);
    	}
    	priFuncAtr = 3;
    	for (q = s->func; q != NULL; q = q->next) {
    	    p =	q->nodePnt;
    	    if (f) PriNameOnlySm(p); else PriFuncSm(p);
    	}
    }
}

#ifdef OPTDIV

void	DispFileSm(void)
{
    REFLIST *q;
    TNODE   *p;
    SRCL    *s;
    int	    n;

    fprintf(outFp, "\n# �t�@�C�����Ƃ̊֐��̃��X�g\n\n");
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	fprintf(outFp, "\n#file %s\n", s->name);
    	for (q = s->func; q != NULL; q = q->next) {
    	    p =	q->nodePnt;
    	    if (p->filName == s->name) {
    	    	fprintf(outFp, "%6d ", p->bgnLin);
    	    	if (GRP_fnam[p->refGrp]	== NULL)
    	    	    n =	fprintf(outFp, "group%d.c", p->refGrp);
    	    	else
    	    	    n =	fprintf(outFp, "%s.c", GRP_fnam[p->refGrp]);
    	    	n = 16 - n;
    	    	while (--n >= 0)
    	    	    fprintf(outFp, " ");
    	    	fprintf(outFp, " # %s\n", p->symName);
    	    }
    	}
    }
}
#endif



/*---------------------------------------------------------------------------*/

void	Usage(void)
{
    fprintf(errFp,
    	"usage: cftt [-opts] <file>...    //cftt v2.11 C�̖��O�̎Q�Ɗ֌W�\��\n"
    	" -f        �֐��ȊO�̓c���[�\���ŕ\�����Ȃ�\n"
    	" -k        ���Ղ��̂��߂̋�s�o�͂�}�~\n"
    	" -m        �ЂƂ̃��x��(�͈�)�œ����֐��ďo���������Ƃ����ׂĕ\��\n"
    	" -n        .t03 .t04�o�͂Ŗ��O�����ɂ���(�^�O�t�@�C���`���ɂ��Ȃ�)\n"
    	" -v<N>     ̧�ٖ��̌���(N:0-80)\n"
    	" -r<func>  tree�\���̃��[�g�̊֐������w��(�ȗ���:main)\n"
    	" -o<path>  ���ʂ�<path>�ɏo��\n"
    	" -e<path>  �o�߃��b�Z�[�W��<path>�ɏo��\n"
    	/*" -y	      �o�߃��b�Z�[�W�̕\����}��\n"*/
    	" @<path>   ̧�ٖ�,�֐�����<path>������(@�̂ݕW������)\n"
    	" +<path>   ̧�ٖ�,�֐�����<path>������(+�̂ݕW������)\n"
     #if 0  /* ���͂�g���Ȃ��Â��I�v�V����? */
    	" -a        ����`�֐����o�^\n"
    	" -a1       �e�q�\���Ŗ���]�֐��̂ݕ\��\n"
    	" -b        �֐����ȊO�͓o�^���Ȃ�\n"
    	" -c[N]     �e�q�֌W�\���� N=1:�q�����AN=2:�e���AN=3:�e�Ǝq���A�\�����Ȃ�\n"
     #endif
    	);
  #ifdef DEBUG
    /*fprintf(errFp," -d    	���ޯ�ޥӰ��\n");*/
  #endif
    exit(0);
}


/*---------------------------------------------------------------------------*/
static int  Opt_undef;	    	    /* ����`�֐���o�^���邩�ǂ��� 	*/
static int  Opt_dspNameLstFlg =	0;  /* �^�O�t�@�C���������ɕ\��	    	*/
static char outFname[260];  	    /* �o�͂���p�X�������߂�	    	*/
static char errFname[260];  	    /* error�o�͂���p�X��������    	*/
static char cftFname[260];  	    /* cft�t�@�C����	    	    	*/
#ifdef OPTDIV
static int  Opt_div;	    	    /* .T05�o��	    	    	    	*/
#endif


void Opts(char *s)
{
    char *p;
    int	 c;

    p =	s;
    c =	*p++;
    c =	toupper(c);
    switch (c) {
    case 'C':
    	if (*p == 0 || *p == '0')
    	    Flg_dspOyakoMode = 0;
    	else if	(*p == '1')
    	    Flg_dspOyakoMode = 1;   /* '�q(��)'��\�����Ȃ� */
    	else if	(*p == '2')
    	    Flg_dspOyakoMode = 2;   /* �e��\�����Ȃ� */
    	else if	(*p == '3')
    	    Flg_dspOyakoMode = 3;   /* �����̂ݕ\�� */
    	break;
  #ifdef DEBUG
    case 'D':	    	/* �f�o�b�O�E���[�h */
    	Debug_flg = 1;
    	break;
  #endif
    case 'E':	    	/* error�o�͂���p�X���w��    */
    	if (*p == '\0')
    	    goto OPT_ERR;
    	strcpy(errFname,p);
    	break;

    case 'F':
    	Flg_dspTrFuncOnly = 1;	/* �֐��ȊO��Tree�ŕ\�����Ȃ� */
    	break;

    case 'M':
    	Flg_dspAllLine = 1; 	/* ����ꂽ�ďo���֐����ׂĂ�\��*/
    	break;

    case 'N':
    	Opt_dspNameLstFlg = 1;	/* .t03	.t04 �Ŗ��O�����o�� */
    	break;

    case 'K':
    	Flg_karagyo = 0;
    	break;

    case 'O':	    	/* �o�͂���p�X���w��	 */
    	if (*p == '\0')
    	    goto OPT_ERR;
    	strcpy(outFname,p);
    	break;

    case 'R':	    	/* tree�\���ł�ٰĂɂȂ�֐��̎w�� */
    	Flg_dspOyakoTree = 1;
    	if (*p != '\0' && ROOT_cnt < ROOT_MAX)
    	    ROOT_name[ROOT_cnt++] = p;
    	break;

    case 'V':	    	/* �t�@�C�����̕����w��	   */
    	filNamSiz = atoi(p);
    	if (filNamSiz <	0 || filNamSiz > 80)
    	    goto OPT_ERR;
    	break;

    case 'Y':	    	/* ���b�Z�[�W��}�� */
    	Flg_msg	= 0;
    	break;

    case 'T':	/* �̂̃o�b�`�t�@�C���̂��߂Ɏc���Ă���I�v�V����^^; */
    	if (*p == '\0')	{
    	    ;
    	} else if (*p == '1') {
    	    Flg_dspAllLine = 1;	    /* ����ꂽ�ďo���֐����ׂĂ�\��*/
    	} else if (*p == '2') {
    	    Flg_dspTrFuncOnly =	1;  /* �֐��ȊO��Tree�ŕ\�����Ȃ� */
    	} else if (*p == '3') {
    	    Opt_dspNameLstFlg =	1;
    	}
    	break;

  #if 1	/* ���͂�Ӗ����Ȃ��Ȃ��I�v�V���� */
    case 'A':	    	/* ����`�֐����o�^ */
    	Opt_undef = 1;
    	if (*p == '1') {
    	    Flg_dspUndef = 1;
    	}
    	break;
    case 'B':	    	/* �֐��ȊO���o�^(����)	*/
    	Flg_symMode = 0;
    	break;
  #endif
  #ifdef OPTDIV
    case 'G':
    	Opt_div	= 1;
    	break;
  #endif

    case '\0':
    case '?':	    	/* �w���v */
    	Usage();

    default:
  OPT_ERR:
    	fprintf(errFp, "�w�肳�ꂽ�I�v�V��������������(%s)\n",s);
    	exit(1);
    }
}


void SRCL_Add(char *name)
{
    SRCL *s;
    s =	callocE(1,sizeof(SRCL));
    s->name = name;
  #if 0
    s->func = NULL;
    s->link = NULL;
  #endif
    if (SRCL_top == NULL) {
    	SRCL_top = s;
    } else {
    	SRCL *t;
    	for (t = SRCL_top; t->link != NULL; t =	t->link)
    	    ;
    	t->link	= s;
    }
}


void	ReadCft(char *cft_name)
    /* @file�ł́A�w��p�X���t�@�C��������� */
{
    FILE   *fp;	    	    /* -z��߼�݂œ��͂���̧�ق̽�ذ�   */
    char    *s;
    int	    l, i, f, libFlg = 0, if0Flg	= 0;

    if (cft_name == NULL || cft_name[0]	== 0)
    	fp = stdin;
    else
    	fp = fopenE(cft_name, "r");
  #ifdef DEBUG
    if (Debug_flg)
    	printf("CFT:%s\n", cft_name);
  #endif
    f =	0;
    l =	0;
    for	(; ;) {
      NEXT_LINE:
    	l++;
    	s = fgets(linBuf, LINBUF_SIZ, fp);
    	if (s == NULL)
    	    break;
    	while ((s = strtok(s, " \t\n\r")) != NULL && *s	!= '\0') {
    	    if (*s == ';')  {
    	    	goto NEXT_LINE;

    	    } else if (strcmp(s, "@if0") == 0) {
    	    	if (if0Flg == 1) {
    	    	    fprintf(errFp,"%s %d : @if0�̃l�X�g�͂ł��܂���\n",	cft_name, l);
    	    	}
    	    	if0Flg = 1;
    	    	goto NEXT_LINE;

    	    } else if (strcmp(s, "@endif") == 0) {
    	    	if (if0Flg == 0) {
    	    	    fprintf(errFp,"%s %d : @if0�Ȃ�@endif�������ꂽ\n", cft_name, l);
    	    	}
    	    	if0Flg = 0;
    	    	goto NEXT_LINE;
    	    }
    	    if (if0Flg)
    	    	goto NEXT_LINE;

    	    if (strcmp(s, "@file") == 0) {
    	    	f = 0;
    	    	s = NULL;
    	    	continue;

    	    } else if (strcmp(s, "@word") == 0)	{
    	    	f = 1;
    	    	s = NULL;
    	    	continue;

    	  #if 0
    	    } else if (strcmp(s, "@root") == 0)	{
    	    	f = 2;
    	    	s = NULL;
    	    	continue;
    	  #endif

    	    } else if (strcmp(s, "@group") == 0) {
    	    	f = 3;
    	    	libFlg = 1;
    	      #if 1
    	    	GRP_cmnt[GRP_no] = "";
    	    	if ((s = strtok(NULL, " \t\n"))	!= NULL	&& *s != '\0' && *s != ';') {
    	    	    GRP_cmnt[GRP_no] = strdupE(s);
    	    	}
    	      #endif
    	    	GRP_no++;
    	    	/*s = NULL;*/
    	    	goto NEXT_LINE;/* continue;*/

    	    } else if (strcmp(s, "@groupfile") == 0) {
    	    	if ((s = strtok(NULL, " \t\n"))	!= NULL	&& *s != '\0')
    	    	    i =	atoi(s);
    	    	else
    	    	    goto ERR_GRPOPT;
    	    	if (i <= 0 || i	>= GRPFNAME_MAX)
    	    	    goto ERR_GRPOPT;
    	    	if ((s = strtok(NULL, " \t\n"))	!= NULL	&& *s != '\0') {
    	    	    GRP_fnam[i]	= strdupE(s);
    	    	} else {
    	      ERR_GRPOPT:
    	    	    fprintf(errFp,"%s %d : .cft����@groupfile�̎w�肪��������\n", cft_name, l);
    	    	    exit(1);
    	    	}
    	    	goto NEXT_LINE;

    	    } else if (strcmp(s, "@comment") ==	0) {
    	    	f = 4;
    	    	s = NULL;
    	    	continue;

    	    } else if (*s == '-') {
    	    	Opts(s+1);
    	    	s = NULL;
    	    	continue;
    	    }

    	    if (f == 0)	{   	    /* �t�@�C������o�^	*/
    	    	SRCL_Add(strdupE(s));
    	    } else if (f == 1) {    /* ��������P����w�肷�� */
    	    	if (RSV_cnt < RSV_MAX) {
    	    	    char *p;
    	    	    p =	strdupE(s);
    	    	    STBL_Add(RSV_name, &RSV_cnt, p);
    	    	}
    	    } else if (f == 2) {    /* ���[�g�ɂȂ閼�O��o�^ */
    	    	if (ROOT_cnt < ROOT_MAX) {
    	    	    ROOT_name[ROOT_cnt++] = strdupE(s);
    	    	}
    	    } else if (f == 3) {    /* ���C�u�����Ȗ��O��o�^ */
    	    	if (ROOT_cnt < ROOT_MAX) {
    	    	    if (libFlg)	{
    	    	    	libFlg = 0;
    	    	    	ROOT_name[ROOT_cnt++] =	strdupE(s);
    	    	    } else {
    	    	    	char *p;
    	    	    	p = callocE(1, strlen(s)+2);
    	    	    	*p = '+';
    	    	    	strcpy(p+1,s);
    	    	    	ROOT_name[ROOT_cnt++] =	p;
    	    	    }
    	    	}
    	    } else if (f == 4) {
    	    	TNODE t;
    	    	memset(&t, 0, sizeof t);
    	    	strncpy(SYM_name,s,SYM_SIZ);
    	    	SYM_name[SYM_SIZ] = 0;
    	    	if ((s = strtok(NULL, " \t\r\n")) != NULL && *s	!= '\0') {
    	    	    t.link[0] =	TREE_root;
    	    	    SYM_atr = AT_CMNT;
    	    	    pass = level = 0;
    	    	    TREE_Add(&t, 0);
    	    	    if (TREE_curNode)
    	    	    	TREE_curNode->cmnt = strdupE(s);
    	    	    TREE_root =	t.link[0];
    	    	    s =	NULL;
    	    	    goto NEXT_LINE;
    	    	} else {
    	    	    fprintf(errFp,"%s %d : .cft����@comment���̎w�肪��������\n", cft_name, l);
    	    	}
    	    }
    	    s =	NULL;
    	}
    }
    if (ferror(fp))
    	exit(errno);
    if (cft_name != NULL && cft_name[0]	!= 0)
    	fclose(fp);
    return;
}

int 	main(int argc, char * argv[])
{
    int	   i;
    char    *p;
    SRCL    *s;

    filNamSiz =	FILNUMSZ;
    ROOT_cnt = Flg_dspOyakoMode	= Opt_undef =
    Flg_dspAllLine = Flg_dspUndef = Flg_dspOyakoTree = 0;
    Flg_msg = 1;
    RSV_Init();
    errFp = stderr;
    outFp = stdout;
    errFname[0]	= 0;
  #ifdef DEBUG
    Debug_flg =	0;
  #endif

    if (argc < 2)
    	Usage();

    TREE_root =	NULL;
    GRP_no = 0;
    for	(i = 0;	i < GRPFNAME_MAX; i++) {
    	GRP_fnam[i] = NULL;
    	GRP_cmnt[i]  = "";
    }

    strcpy(cftFname,argv[0]);
    FIL_ChgExt(cftFname, "cfg");
    ReadCft(cftFname);
    cftFname[0]	= 0;
    outFname[0]	= 0;
    errFname[0]	= 0;

    /* �I�v�V�����^�t�@�C�����ǂݎ�� */
    for	(i = 1;	i < argc; i++) {
    	p = argv[i];
    	if (*p == '+' || *p == '@') {
    	    strcpy(cftFname, p+1);
    	    if (cftFname[0])
    	    	FIL_AddExt(cftFname, "cft");
    	    ReadCft(cftFname);
    	} else if (*p == '-') {
    	    Opts(p+1);
    	} else {
    	    SRCL_Add(p);    /* �t�@�C������ۑ�	*/
    	}
    }

  #if 0	/*def DEBUG*/
    if (Debug_flg) {
    	for (i = 0; i <	RSV_cnt; i++)
    	    fprintf(errFp,"\t$ %s\n",RSV_name[i]);
    }
  #endif

    /* �`�F�b�N	*/
    if (SRCL_top == NULL) {
    	fprintf(errFp, "���̓t�@�C�������w�肳��Ă��Ȃ�\n");
    	exit(1);
    }
    if (ROOT_cnt == 0) {
    	ROOT_name[ROOT_cnt++] =	strdupE("main");
    } else if (ROOT_cnt	>= ROOT_MAX) {
    	fprintf(errFp,"�֐��̐����������܂�\n");
    	exit(1);
    }

    /* �o�͐�w�� */
    if (errFname[0]) {
    	errFp =	fopenE(errFname, "w");
    } else {
    	errFp =	stdout;
    }

  #ifdef DEBUG
    if (Debug_flg != 0)	{
    	for (s = SRCL_top; s !=	NULL; s	= s->link)
    	    fprintf(errFp, "%2d%s\n", i, s->name);
    }
  #endif

    /* �t�@�C���̒��������߂� */
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	if (filNamSiz <	strlen(s->name))
    	    filNamSiz =	strlen(s->name);
    }

    /* ���O�o�^���� */
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	SRC_name = s->name;
    	if (Flg_msg)
    	    fprintf(errFp, "--- %s �̓o�^�������J�n���܂�\n", SRC_name);
    	s->func	= MakSmTree((int)Opt_undef);
    }
    fprintf(errFp,"\n");

    /* ���O�Q�Ə��� */
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	SRC_name = s->name;
    	if (Flg_msg)
    	    fprintf(errFp, "+++ %s �̎Q�Ə������J�n���܂�\n", SRC_name);
    	MakRefLst();
    }

  #ifdef GRPN_ON    /* �o�O���Ă� */
    /* �O���[�v���̔ԍ�������	*/
    CountGrpSubNo();
  #endif

    /* �ďo�֌W��tree�\������ */
    outFp = stdout;
    i =	outFname[0];
    if (i)  outFp = fopenE(FIL_ChgExt(outFname,	"T01"),	"w");
    DispTreeFlow();
    if (i)  fclose(outFp);

    /* �e�֐��̐e�q�֌W��\�� */
    if (i)  outFp = fopenE(FIL_ChgExt(outFname,	"T02"),	"w");
    Flg_dspOyakoMode = 0;
    DispOyakoFlow();
    if (i)  fclose(outFp);

    /* �ϐ��A�֐��A�}�N�����Ƃ̖��O�ꗗ��\�� */
    if (i) {
    	outFp =	fopenE(FIL_ChgExt(outFname, "T03"), "w");
    	Flg_dspOyakoMode = 3;
    	DispNameList(Opt_dspNameLstFlg);
    	fclose(outFp);
    }


    /* �\�[�X�t�@�C�����̖��O�ꗗ��\��	*/
    if (i) {
    	outFp =	fopenE(FIL_ChgExt(outFname, "T04"), "w");
    	DispFileSmList(Opt_dspNameLstFlg);
    	fclose(outFp);
    }

  #ifdef OPTDIV
    /* �\�[�X���Ƃ̊֐����X�g */
    if (Opt_div) {
    	if (i)	outFp =	fopenE(FIL_ChgExt(outFname, "T05"), "w");
    	DispFileSm();
    	if (i)	fclose(outFp);
    }
  #endif

    /* �I�� */
    return 0;
}

