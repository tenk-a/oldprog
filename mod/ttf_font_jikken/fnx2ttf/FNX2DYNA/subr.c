#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <io.h>

#include "subr.h"

/*--------------------------------------------------------------------------*/

int 	debugflag;


/*--------------------------------------------------------------------------*/
/* �����񏈗��֌W   	    	    	    	    	    	    	    */


char *strncpyZ(char *dst, char *src, size_t size)
{
    strncpy(dst, src, size);
    dst[size-1]	= 0;
    return dst;
}


char *StrSkipSpc(char *s)
{
    while ((*s && *(unsigned char *)s <= ' ') || *s == 0x7f) {
    	s++;
    }
    return s;
}


char *StrDelLf(char *s)
{
    char *p;
    p =	STREND(s);
    if (p != s && p[-1]	== '\n') {
    	p[-1] =	0;
    }
    return s;
}


long strtolKM(char *s, char **d, int r)
{
    long l;

    l =	strtol(s, &s, r);
    if (*s == 'k' || *s	== 'K')	{
    	s++;
    	l *= 1024;
    } else if (*s == 'm' || *s == 'M') {
    	s++;
    	l *= 1024*1024;
    } else if (*s == 'g' || *s == 'G') {
    	s++;
    	l *= 1024*1024*1024;
    }
    *d = s;
    return l;
}




/*--------------------------------------------------------------------------*/
/* �G���[�����t���̕W���֐� 	    	    	    	    	    	    */

void *mallocE(size_t a)
    /* �G���[������Α�exit�� malloc() */
{
    void *p;

    if (a == 0)
    	a = 1;
    p =	malloc(a);
    //printf("malloc(0x%x)\n",a);
    if (p == NULL) {
    	err_exit("������������Ȃ�(%d byte(s))\n",a);
    }
    return p;
}


void *callocE(size_t a,	size_t b)
    /* �G���[������Α�exit�� calloc() */
{
    void *p;

    if (a== 0 || b == 0)
    	a = b =	1;
    p =	calloc(a,b);
    //printf("calloc(0x%x,0x%x)\n",a,b);
    if (p == NULL) {
    	err_exit("������������Ȃ�(%d*%d byte(s))\n",a,b);
    }
    return p;
}


void *reallocE(void *m,	size_t a)
    /* �G���[������Α�exit�� calloc() */
{
    void *p;

    if (a == 0)
    	a = 1;
    p =	realloc(m, a);
    //printf("realloc(0x%x,0x%x)\n",m,a);
    if (p == NULL) {
    	err_exit("������������Ȃ�(%d byte(s))\n",a);
    }
    return p;
}


char *strdupE(char *s)
    /* �G���[������Α�exit�� strdup() */
{
    char *p;

    //printf("strdup('%s')\n",s);
    if (s == NULL)
    	return callocE(1,1);
    p =	calloc(1,strlen(s)+8);
    if (p)
    	strcpy(p, s);
    if (p == NULL) {
    	err_exit("������������Ȃ�(����%d+1)\n",strlen(s));
    }
    return p;
}


int freeE(void *p)
{
    if (p)
    	free(p);
    return 0;
}



void *mallocMaE(int sz,	int minSz, int aln)
{
    void *p;

    p =	mallocMa(sz, minSz, aln);
    if (p == NULL) {
    	err_exit("������������Ȃ�(%d byte(s))\n",sz);
    }
    return p;
}


void *mallocMa(int sz, int minSz, int aln)
{
    /* �w�肵��sz ��malloc ���Ď��s������AminSz �`sz �͈̔͂œK���Ɏ����ă��������m�ۂ��� */
    /* aln �� 0�ȏ�ł���� 2��aln��ŃT�C�Y���A���C�����g���܂� */
    void *p;
    int	 a;

    /* �A���C�����g�p�̃}�X�N�𐶐� */
    if (aln <= 0)
    	a = 1;
    else
    	a = (1<<aln);
    a =	a - 1;

    /* �T�C�Y�𒲐� */
    sz = (sz + a) & ~a;
    minSz = (minSz + a)	& ~a;

    p =	malloc(sz);
    if (p)
    	return p;

    /* �ŏ��T�C�Y���m�ۂł��邩�`�F�b�N	*/
    if (minSz <= 0)
    	return NULL;
    p =	malloc(minSz);
    if (p == NULL)
    	return NULL;
    free(p);

    /* �m�ۂł���T�C�Y��T��*/
    do {
    	sz = (sz/2 + a)	& ~a;
    	if (sz < minSz)
    	    sz = minSz;
    	p = malloc(sz);
    } while (p == NULL && sz > minSz);

    return p;
}



/*--------------------------------------------------------------------------*/
/* �G���[�����A�f�o�b�O�p�֐�	    	    	    	    	    	    */


/*volatile*/void err_exit(char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
    vfprintf(stdout, fmt, app);
    va_end(app);
    exit(1);
}


void dbg_printf(char *fmt, ...)
{
 #ifndef NDEBUG
  #ifdef DBG_FILE_NAME
    va_list app;
    FILE *fp;

    if (debugflag == 0)
    	return;
    fp = fopen(DBG_FILE_NAME, "at");
    if (fp) {
    	va_start(app, fmt);
    	vfprintf(fp, fmt, app);
    	va_end(app);
    	fclose(fp);
    }
  #else
    va_list app;
    if (debugflag == 0)
    	return;
    va_start(app, fmt);
    vfprintf(stdout, fmt, app);
    va_end(app);
  #endif
 #endif
}

/*--------------------------------------------------------------------------*/
/* �t�@�C���������񏈗�	    	    	    	    	    	    	    */

int  FIL_sjisFlag = 1;


int   FIL_SetSjisMode(int sw)
{
    int	n;

    n =	FIL_sjisFlag;
    FIL_sjisFlag = sw;
    return n;
}


char *FIL_BaseName(char	*adr)
{
    char *p;

    p =	adr;
    while (*p != '\0') {
    	if (*p == ':' || *p == '/' || *p == '\\')
    	    adr	= p + 1;
    	if (FIL_ISKANJI((*(unsigned char *)p)) && *(p+1) )
    	    p++;
    	p++;
    }
    return adr;
}


char *FIL_GetExt(char *name)
{
    char *p;

    name = FIL_BaseName(name);
    p =	strrchr(name, '.');
    if (p) {
    	return p+1;
    }
    return STREND(name);
}


char *FIL_ChgExt(char filename[], char *ext)
{
    char *p;

    p =	FIL_BaseName(filename);
    p =	strrchr( p, '.');
    if (p == NULL) {
    	if (ext) {
    	    strcat(filename,".");
    	    strcat( filename, ext);
    	}
    } else {
    	if (ext	== NULL)
    	    *p = 0;
    	else
    	    strcpy(p+1,	ext);
    }
    return filename;
}


char *FIL_AddExt(char filename[], char *ext)
{
    if (strrchr(FIL_BaseName(filename),	'.') ==	NULL) {
    	strcat(filename,".");
    	strcat(filename, ext);
    }
    return filename;
}


char *FIL_DelLastDirSep(char *dir)
{
    /* ������̍Ō�� \	�� / ������΍폜 */
    char *p, *s;

    if (dir) {
    	s = FIL_BaseName(dir);
    	if (strlen(s) >	1) {
    	    p =	STREND(s);
    	    if (p[-1] == '/') {
    	    	p[-1] =	0;
    	    } else if (p[-1] ==	'\\') {
    	    	if (FIL_sjisFlag == 0) {
    	    	    p[-1] = 0;
    	    	} else {
    	    	    int	f = 0;
    	    	    while (*s) {
    	    	    	f = 0;
    	    	    	if (ISKANJI(*s)	&& s[1]) {
    	    	    	    s++;
    	    	    	    f =	1;
    	    	    	}
    	    	    	s++;
    	    	    }
    	    	    if (f == 0)
    	    	    	p[-1] =	0;
    	    	}
    	    }
    	}
    }
    return dir;
}


char *FIL_NameUpr(char *s0)
{
    /* �S�p�Q�o�C�g�ڂ��l������	strupr */
    char *s = s0;

    while (*s) {
    	if (FIL_ISKANJI(*s) && s[1]) {
    	    s += 2;
    	} else if (ISLOWER(*s))	{
    	    *s = TOUPPER(*s);
    	    s++;
    	} else {
    	    s++;
    	}
    }
    return s0;
}


char *FIL_DirNameDupE(char *dir, char *name)
{
    void *m;

    m =	FIL_DirNameDup(dir,name);
    if (m == NULL) {
    	err_exit("������������܂���\n");
    }
    return m;
}


char *FIL_DirNameDup(char *dir,	char *name)
{
    // �f�B���N�g�����ƃt�@�C�����������������̂�strdup����
    // �g���q�̕t���ւ����ł���悤�A+5�o�C�g�͗]���Ƀ������͊m�ۂ���
    int	l,n;
    void *m;

    l =	(dir) ?	strlen(dir) : 0;
    n =	(name) ? strlen(name) :	0;
    m =	calloc(1, l+n+1+7);
    if (m == NULL)
    	return NULL;
    if (l) {
    	strcpy(m, dir);
    	FIL_DelLastDirSep(m);
    	strcat(m, "\\");
    }
    if (n) {
    	strcat(m, name);
    }
    return m;
}


#if 1
char *FIL_DirNameChgExt(char *nam, char	*dir, char *name, char *chgext)
{
    if (name ==	NULL ||	strcmp(name,".") == 0)
    	return NULL;
    if (dir && *dir && name[0] != '\\' && name[1] != ':') {
    	sprintf(nam, "%s\\%s", dir, name);
    } else {
    	sprintf(nam, "%s", name);
    }
    FIL_ChgExt(nam, chgext);
    //strupr(nam);
    return nam;
}


char *FIL_DirNameAddExt(char *nam, char	*dir, char *name, char *addext)
{
    if (name ==	NULL ||	strcmp(name,".") == 0)
    	return NULL;
    if (dir && *dir && name[0] != '\\' && name[1] != ':') {
    	sprintf(nam, "%s\\%s", dir, name);
    } else {
    	sprintf(nam, "%s", name);
    }
    FIL_AddExt(nam, addext);
    //strupr(nam);
    return nam;
}
#endif



int FIL_Rename(char *oldname, char *newname)
{
    char bak[FIL_NMSZ];

    strcpy(bak,	newname);
    FIL_ChgExt(bak, "bak");
    remove(bak);
    rename(newname, bak);
    return rename(oldname, newname);
}


/* ------------------------- */

static int FIL_GetTmpDirSub(char *t, int f);

int FIL_GetTmpDirE(char	*t)
{
    return FIL_GetTmpDirSub(t, 1);
}


int FIL_GetTmpDir(char *t)
{
    return FIL_GetTmpDirSub(t, 0);
}


static int FIL_GetTmpDirSub(char *t, int f)
{
    char *p;
    char nm[FIL_NMSZ+2];

    if (*t) {
    	strcpy(nm, t);
    	p = STREND(nm);
    } else {
    	p = getenv("TMP");
    	if (p == NULL) {
    	    p =	getenv("TEMP");
    	    if (p == NULL) {
    	    	if (f == 0)
    	    	    p =	".\\";
    	    	else
    	    	    err_exit("���ϐ�TMP��TEMP�Ńe���|�����E�f�B���N�g�����w�肵�Ă�������\n");
    	    }
    	}
    	strcpy(nm, p);
    	p = STREND(nm);
    }
    if (p[-1] != '\\' && p[-1] != ':' && p[-1] != '/')
    	strcat(nm,"\\");
    strcat(nm,"*.*");
    _fullpath(t, nm, FIL_NMSZ);
    p =	FIL_BaseName(t);
    *p = 0;
    if (p[-1] == '\\')
    	p[-1] =	0;
    return 0;
}



/* ------------------------- */
/*--------------------------------------------------------------------------*/

void FIL_MakeDmyFileE(char *oname, size_t fsiz,	int fh)
{
    int	c;

    c =	FIL_MakeDmyFile(oname, fsiz, fh);
    if (c < 0) {
    	switch (c) {
    	case -1:
    	    err_exit("�t�@�C�� %s ���I�[�v���ł��܂���ł���\n", oname);
    	    break;
    	case -2:
    	    err_exit("�t�@�C�� %s �̏������ݒ��ɃG���[����\n", oname);
    	    break;
    	case -3:
    	    err_exit("�t�@�C�� %s �̍쐬���A���������s������.\n", oname);
    	    break;
    	default:
    	    assert(0);
    	}
    }
}


int FIL_MakeDmyFile(char *oname, size_t	fsiz, int fh)
{
    FILE *fp;
    int	 sz, rsz, n;
    char *b;

    sz	= 0x10000;
    n	= fsiz / sz;
    rsz	= fsiz % sz;

    b	= malloc(sz);
    if (b == NULL) {
    	return -3;
    }
    memset(b, fh, sz);
    fp = fopen(oname, "wb");
    if (fp == NULL)
    	return -1;
    while (n--)	{
    	fwrite(b, 1, sz, fp);
    	if (ferror(fp))	{
    	    fclose(fp);
    	    if (ferror(fp))
    	    	fclose(fp);
    	    return -2;
    	}
    }
    if (rsz) {
    	fwrite(b, 1, rsz, fp);
    	if (ferror(fp))	{
    	    fclose(fp);
    	    if (ferror(fp))
    	    	fclose(fp);
    	    return -2;
    	}
    }
    free(b);
    fclose(fp);
    return 0;
}



/* ------------------------------------------------------------------------ */

void *FIL_SaveE(char *name, void *buf, int size)
{
    void *p;

    p =	FIL_Save(name, buf, size);
    if (p == NULL) {
    	err_exit("%s�̃��[�h�Ɏ��s���܂���\n", name);
    }
    return p;
}


void *FIL_Save(char *name, void	*buf, int size)
{
    FILE *fp;

    fp = fopen(name,"wb");
    setvbuf(fp,	NULL, _IOFBF, 1024*1024);
    if (fp == NULL)
    	return NULL;
    if (size &&	buf) {
    	fwrite(buf, 1, size, fp);
    	if (ferror(fp))	{
    	    fclose(fp);
    	    buf	= NULL;
    	}
    }
    fclose(fp);
    return buf;
}


void *FIL_LoadE(char *name, void *buf, int bufsz, int *rdszp)
{
    void *p;

    p =	FIL_Load(name, buf, bufsz, rdszp);
    if (p == NULL) {
    	err_exit("%s�̃��[�h�Ɏ��s���܂���\n", name);
    }
    return p;
}


void *FIL_Load(char *name, void	*buf, int bufsz, int *rdszp)
{
    /* name : �ǂ݂��ރt�@�C�� */
    /* buf  : �ǂ݂��ރ������BNULL���w�肳���� malloc���A16�o�C�g�]���Ɋm�ۂ��� */
    /* bufsz: buf�̃T�C�Y�B0���w�肳���� �t�@�C���T�C�Y�ƂȂ�	*/
    /* rdszp: NULL�łȂ���΁A�ǂ݂��񂾃t�@�C���T�C�Y�����ĕԂ� */
    /* �߂�l: buf�̃A�h���X��malloc���ꂽ�A�h���X. �G���[����NULL��Ԃ� */
    FILE *fp;
    int	 l;

    fp = fopen(name, "rb");
    if (fp == NULL)
    	return NULL;
    l =	filelength(fileno(fp));
    if (rdszp)
    	*rdszp = l;
    if (bufsz == 0)
    	bufsz =	l;
    if (l > bufsz)
    	l = bufsz;
    if (buf == NULL) {
    	bufsz =	(bufsz + 15 + 16) & ~15;
    	buf = calloc(1,	bufsz);
    	if (buf	== NULL)
    	    return NULL;
    }
    fread(buf, 1, l, fp);
    if (ferror(fp)) {
    	fclose(fp);
    	buf = NULL;
    }
    fclose(fp);
    return buf;
}

/*--------------------------------------------------------------------------*/
/* �t�@�C���̓��t���擾	    	    	    	    	    	    	    */

#ifdef __BORLANDC__

FIL_TIME FIL_TimeGet(char *srcname)
{
    FILE    	*fp;
    int	    	f;
    FIL_TIME	fdt;

    fp = fopen(srcname,	"rb");
    if (fp == NULL) {
    	memset(&fdt, 0,	sizeof fdt);
    	return fdt;
    }
    f =	getftime(fileno(fp), &fdt);
    fclose(fp);
    if (f < 0) {
    	memset(&fdt, 0,	sizeof fdt);
    	return fdt;
    }
    return fdt;
}


int FIL_TimeSet(char *dstname, FIL_TIME	fdt)
{
    FILE *fp;
    int	   f;

    fp = fopen(dstname,	"ab+");
    if (fp == NULL) {
    	return -1;
    }
    f =	setftime(fileno(fp), &fdt);
    fclose(fp);
    if (f < 0) {
    	return -1;
    }
    return 0;
}


int FIL_TimeCmp(FIL_TIME t1, FIL_TIME t2)
{
    int	f;

    f =	t1.ft_year  - t2.ft_year;   if (f)  return f;
    f =	t1.ft_month - t2.ft_month;  if (f)  return f;
    f =	t1.ft_day   - t2.ft_day;    if (f)  return f;
    f =	t1.ft_hour  - t2.ft_hour;   if (f)  return f;
    f =	t1.ft_min   - t2.ft_min;    if (f)  return f;
    f =	t1.ft_tsec  - t2.ft_tsec;
    return f;
}

#endif /* __BORLANDC__ */

/* ------------------------------------------------------------------------ */
/* �G���[exit���� �t�@�C���֐�	    	    	    	    	    	    */


FILE *fopenE(char *name, char *mod)
{
    /* �G���[������Α�exit�� fopen() */
    FILE *fp;

    fp = fopen(name,mod);
    if (fp == NULL) {
    	err_exit("�t�@�C�� %s ���I�[�v���ł��܂���\n",name);
    }
    setvbuf(fp,	NULL, _IOFBF, 1024*1024);
    return fp;
}


size_t	fwriteE(void *buf, size_t sz, size_t num, FILE *fp)
{
    /* �G���[������Α�exit�� fwrite() */
    size_t l;

    l =	fwrite(buf, sz,	num, fp);
    if (ferror(fp)) {
    	fcloseE(fp);
    	err_exit("�t�@�C�������݂ŃG���[����\n");
    }
    return l;
}


size_t	freadE(void *buf, size_t sz, size_t num, FILE *fp)
{
    /* �G���[������Α�exit�� fread() */
    size_t l;

    l =	fread(buf, sz, num, fp);
    if (ferror(fp)) {
    	fcloseE(fp);
    	err_exit("�t�@�C���Ǎ��݂ŃG���[����\n");
    }
    return l;
}


void fcloseE(FILE *fp)
{
    if (fp) {
    	fclose(fp);
    	if (ferror(fp))	{
    	    fclose(fp);
    	}
    }
}


size_t	flength(FILE *fp)
{
    /* �t�@�C���T�C�Y�����߂� */
    /* �ߋ��Ƃ̌݊��̂��߁A�g���Ȃ疼�O�t���ւ��끄����	*/

    return filelength(fileno(fp));
}

/* ------------------------------------------------------------------------ */
/* �G���[exit���� fgetc,fputc�t�@�C���֐�   	    	    	    	    */


int fgetcE(FILE	*fp)
{
    /* fp��� 1�o�C�g(0..255) �ǂݍ���.	�G���[������Α��I�� */
    static uint8_t buf[1];

    freadE(buf,	1, 1, fp);
    return (uint8_t)buf[0];
}


int fgetc2iE(FILE *fp)
{
    /* fp��� ��ٴ��ި�݂� 2�o�C�g�ǂݍ���. �G���[������Α��I�� */
    int	c;

    c =	fgetcE(fp);
    return (uint16_t)(c	+ (fgetcE(fp)<<8));
}


int fgetc4iE(FILE *fp)
{
    /* fp��� ��ٴ��ި�݂� 4�o�C�g�ǂݍ���. �G���[������Α��I�� */
    int	c;

    c =	fgetc2iE(fp);
    return c + (fgetc2iE(fp)<<16);
}


int fgetc2mE(FILE *fp)
{
    /* fp��� big���ި�݂� 2�o�C�g�ǂݍ���. �G���[������Α��I�� */
    int	c;

    c =	fgetcE(fp);
    return (uint16_t)((c<<8) + fgetcE(fp));
}


int fgetc4mE(FILE *fp)
{
    /* fp��� big���ި�݂� 4�o�C�g�ǂݍ���. �G���[������Α��I�� */
    int	c;

    c =	fgetc2mE(fp);
    return (c<<16) + fgetc2mE(fp);
}


void fputcE(int	c, FILE	*fp)
{
    /* fp�� 1�o�C�g(0..255) ��������. �G���[������Α��I�� */
    static uint8_t buf[1];

    buf[0] = (uint8_t)c;
    fwriteE(buf, 1, 1, fp);
}


void fputc2mE(int c, FILE *fp)
{
    /* fp�� �ޯ�޴��ި�݂� 2�o�C�g��������. �G���[������Α��I�� */
    static uint8_t buf[4];

    buf[0] = (uint8_t)(c>> 8);
    buf[1] = (uint8_t)(c);
    fwriteE(buf, 1, 2, fp);
}


void fputc4mE(int c, FILE *fp)
{
    /* fp�� �ޯ�޴��ި�݂� 4�o�C�g��������. �G���[������Α��I�� */
    static uint8_t buf[4];

    buf[0] = (uint8_t)(c>>24);
    buf[1] = (uint8_t)(c>>16);
    buf[2] = (uint8_t)(c>> 8);
    buf[3] = (uint8_t)(c);
    fwriteE(buf, 1, 4, fp);
}


void *fputsE(char *s, FILE *fp)
{
    int	n;

    n =	strlen(s);
    fwriteE(s, 1, n, fp);
    return s;
}


void fputc2iE(int c, FILE *fp)
{
    /* fp�� ��ٴ��ި�݂� 2�o�C�g��������. �G���[������Α��I�� */
    static uint8_t buf[4];

    buf[0] = (uint8_t)(c);
    buf[1] = (uint8_t)(c>> 8);
    fwriteE(buf, 1, 2, fp);
}


void fputc4iE(int c, FILE *fp)
{
    /* fp�� ��ٴ��ި�݂� 4�o�C�g��������. �G���[������Α��I�� */
    static uint8_t buf[4];

    buf[0] = (uint8_t)(c);
    buf[1] = (uint8_t)(c>> 8);
    buf[2] = (uint8_t)(c>> 16);
    buf[3] = (uint8_t)(c>> 24);
    fwriteE(buf, 1, 4, fp);
}



/*--------------------------------------------------------------------------*/
/*  CCITT��32bit CRC�v�Z���[�`�� (C����ɂ��A���S���Y�����T��藬�p)	    */

/* typedef unsigned char uint8_t; */
/* typedef unsigned long uint32_t; */

static uint32_t	crctable[256] =	{
    0x00000000,	0x77073096, 0xee0e612c,	0x990951ba, 0x076dc419,	0x706af48f, 0xe963a535,	0x9e6495a3, 0x0edb8832,	0x79dcb8a4, 0xe0d5e91e,	0x97d2d988, 0x09b64c2b,	0x7eb17cbd, 0xe7b82d07,	0x90bf1d91,
    0x1db71064,	0x6ab020f2, 0xf3b97148,	0x84be41de, 0x1adad47d,	0x6ddde4eb, 0xf4d4b551,	0x83d385c7, 0x136c9856,	0x646ba8c0, 0xfd62f97a,	0x8a65c9ec, 0x14015c4f,	0x63066cd9, 0xfa0f3d63,	0x8d080df5,
    0x3b6e20c8,	0x4c69105e, 0xd56041e4,	0xa2677172, 0x3c03e4d1,	0x4b04d447, 0xd20d85fd,	0xa50ab56b, 0x35b5a8fa,	0x42b2986c, 0xdbbbc9d6,	0xacbcf940, 0x32d86ce3,	0x45df5c75, 0xdcd60dcf,	0xabd13d59,
    0x26d930ac,	0x51de003a, 0xc8d75180,	0xbfd06116, 0x21b4f4b5,	0x56b3c423, 0xcfba9599,	0xb8bda50f, 0x2802b89e,	0x5f058808, 0xc60cd9b2,	0xb10be924, 0x2f6f7c87,	0x58684c11, 0xc1611dab,	0xb6662d3d,
    0x76dc4190,	0x01db7106, 0x98d220bc,	0xefd5102a, 0x71b18589,	0x06b6b51f, 0x9fbfe4a5,	0xe8b8d433, 0x7807c9a2,	0x0f00f934, 0x9609a88e,	0xe10e9818, 0x7f6a0dbb,	0x086d3d2d, 0x91646c97,	0xe6635c01,
    0x6b6b51f4,	0x1c6c6162, 0x856530d8,	0xf262004e, 0x6c0695ed,	0x1b01a57b, 0x8208f4c1,	0xf50fc457, 0x65b0d9c6,	0x12b7e950, 0x8bbeb8ea,	0xfcb9887c, 0x62dd1ddf,	0x15da2d49, 0x8cd37cf3,	0xfbd44c65,
    0x4db26158,	0x3ab551ce, 0xa3bc0074,	0xd4bb30e2, 0x4adfa541,	0x3dd895d7, 0xa4d1c46d,	0xd3d6f4fb, 0x4369e96a,	0x346ed9fc, 0xad678846,	0xda60b8d0, 0x44042d73,	0x33031de5, 0xaa0a4c5f,	0xdd0d7cc9,
    0x5005713c,	0x270241aa, 0xbe0b1010,	0xc90c2086, 0x5768b525,	0x206f85b3, 0xb966d409,	0xce61e49f, 0x5edef90e,	0x29d9c998, 0xb0d09822,	0xc7d7a8b4, 0x59b33d17,	0x2eb40d81, 0xb7bd5c3b,	0xc0ba6cad,
    0xedb88320,	0x9abfb3b6, 0x03b6e20c,	0x74b1d29a, 0xead54739,	0x9dd277af, 0x04db2615,	0x73dc1683, 0xe3630b12,	0x94643b84, 0x0d6d6a3e,	0x7a6a5aa8, 0xe40ecf0b,	0x9309ff9d, 0x0a00ae27,	0x7d079eb1,
    0xf00f9344,	0x8708a3d2, 0x1e01f268,	0x6906c2fe, 0xf762575d,	0x806567cb, 0x196c3671,	0x6e6b06e7, 0xfed41b76,	0x89d32be0, 0x10da7a5a,	0x67dd4acc, 0xf9b9df6f,	0x8ebeeff9, 0x17b7be43,	0x60b08ed5,
    0xd6d6a3e8,	0xa1d1937e, 0x38d8c2c4,	0x4fdff252, 0xd1bb67f1,	0xa6bc5767, 0x3fb506dd,	0x48b2364b, 0xd80d2bda,	0xaf0a1b4c, 0x36034af6,	0x41047a60, 0xdf60efc3,	0xa867df55, 0x316e8eef,	0x4669be79,
    0xcb61b38c,	0xbc66831a, 0x256fd2a0,	0x5268e236, 0xcc0c7795,	0xbb0b4703, 0x220216b9,	0x5505262f, 0xc5ba3bbe,	0xb2bd0b28, 0x2bb45a92,	0x5cb36a04, 0xc2d7ffa7,	0xb5d0cf31, 0x2cd99e8b,	0x5bdeae1d,
    0x9b64c2b0,	0xec63f226, 0x756aa39c,	0x026d930a, 0x9c0906a9,	0xeb0e363f, 0x72076785,	0x05005713, 0x95bf4a82,	0xe2b87a14, 0x7bb12bae,	0x0cb61b38, 0x92d28e9b,	0xe5d5be0d, 0x7cdcefb7,	0x0bdbdf21,
    0x86d3d2d4,	0xf1d4e242, 0x68ddb3f8,	0x1fda836e, 0x81be16cd,	0xf6b9265b, 0x6fb077e1,	0x18b74777, 0x88085ae6,	0xff0f6a70, 0x66063bca,	0x11010b5c, 0x8f659eff,	0xf862ae69, 0x616bffd3,	0x166ccf45,
    0xa00ae278,	0xd70dd2ee, 0x4e048354,	0x3903b3c2, 0xa7672661,	0xd06016f7, 0x4969474d,	0x3e6e77db, 0xaed16a4a,	0xd9d65adc, 0x40df0b66,	0x37d83bf0, 0xa9bcae53,	0xdebb9ec5, 0x47b2cf7f,	0x30b5ffe9,
    0xbdbdf21c,	0xcabac28a, 0x53b39330,	0x24b4a3a6, 0xbad03605,	0xcdd70693, 0x54de5729,	0x23d967bf, 0xb3667a2e,	0xc4614ab8, 0x5d681b02,	0x2a6f2b94, 0xb40bbe37,	0xc30c8ea1, 0x5a05df1b,	0x2d02ef8d,
};



uint32_t MemCrc32(void *dat, int siz)
{
    uint8_t *s = (uint8_t*)dat;
    uint32_t r;

    r =	0xFFFFFFFFUL;
    while (--siz >= 0)
    	r = (r >> 8) ^ crctable[(uint8_t)r ^ *s++];
    return r;	/* �ꍇ�ɂ���Ă� r ^ 0xFFFFFFFFUL */
}



/*--------------------------------------------------------------------------*/
/* BM�@��p���������o�C�g�f�[�^�̃T�[�`	    	    	    	    	    */
/* �� C����ɂ��A���S���Y�����T�̂�����   	    	    	    	    */

void *MemSearch(void *area0, int asiz, void *ptn0, int ptnlen)
{
    /* area0, asiz : �����J�n�A�h���X�Ƃ��͈̔�	    */
    /* ptn0, ptnlen: �����f�[�^�Ƃ��̃T�C�Y(byte��) */
    int	skp[256];
    int	i, j, k;
    int	c, tail;
    unsigned char *area	= area0, *ptn =	ptn0;

    if (ptnlen <= 0 || asiz <= 0) {
    	return NULL;
    }
    tail = ptn[ptnlen -	1]; 	    	/* �Ō�̕���(1byte) */
    if (ptnlen == 1) {	    	    	/* ����1�Ȃ�ȒP! */
    	for (i = 0; i <	asiz; i++) {	/* �P���ɓ���1byte��������܂Ō��� */
    	    if (area[i]	== tail)
    	    	return area + i;    	/* �݂����� */
    	}
    } else {	    	    	    	/* ����2�ȏ�̂Ƃ��\���� */
    	    	    	    	    	/* skp[]�ɁA���̕��������������O�̂Ƃ��Ɏ��̌����J�n�ʒu�փX�L�b�v����o�C�g����ݒ肷�� */
    	for (i = 0; i <	256; i++)   	/* �܂�256�������ׂĕs��v�̏ꍇ�Ƃ��Č����f�[�^�T�C�Y���̃X�L�b�v�l��ݒ� */
    	    skp[i] = ptnlen;
    	for (i = 0; i <	ptnlen-1; i++)	/* ���Ɍ����f�[�^���\�����镶���̏ꍇ�́A���̕��������̌����f�[�^�̃o�C�g�������X�L�b�v�ɂ��� */
    	    skp[ptn[i]]	= ptnlen - 1 - i;
    	/* i = ptnlen -	1; */	    	/* for���̌��ʂ����̎��ɓ���������s�v */
    	/* �w��͈͂̌����J�n�I	*/
    	while (i < asiz) {
    	    c =	area[i];    	    	/* �����f�[�^�̔�����1byte���`�F�b�N����. */
    	    if (c == tail) {	    	/* �����ňႦ�΁A(�ő�)�����f�[�^�o�C�g���́A��r�����X�L�b�v�ł��� */
    	    	j = ptnlen - 1;	    	/* �����Ȃ�΁A����Ɍ����f�[�^����납���r���� */
    	    	k = i;
    	    	while (ptn[--j]	== area[--k]) {
    	    	    if (j == 0)	    	/* �����f�[�^�̐擪�Ȃ�� */
    	    	    	return area + k;/* �������� */
    	    	}
    	    }
    	    i += skp[c];    	    	/* ���̈ʒu�Ń}�b�`���Ȃ������̂ŁA���̈ʒu�܂ŃX�L�b�v	*/
    	}
    }

    /* ������Ȃ�����	*/
    return NULL;
}

/* ------------------------------------------------------------------------ */
/* ������̃��X�g���쐬	    */

slist_t	*slist_add(slist_t **p0, char *s)
{
    /* ������̃��X�g��ǉ� 	*/
    slist_t* p;

    p =	*p0;
    if (p == NULL) {
    	p = callocE(1, sizeof(slist_t));
    	p->s = strdupE(s);
    	*p0 = p;
    } else {
    	while (p->link != NULL)	{
    	    p =	p->link;
    	}
    	p->link	= callocE(1, sizeof(slist_t));
    	p = p->link;
    	p->s = strdupE(s);
    }
    return p;
}


void slist_free(slist_t	**p0)
{
    /* ������̃��X�g���폜 */
    slist_t *p,	*q;

    for	(p = *p0; p; p = q) {
    	q = p->link;
    	freeE(p->s);
    	freeE(p);
    }
}



/* ------------------------------------------------------------------------ */
/* �\�[�g�ςݕ�����|�C���^�̔z����A��������������Ă��̈ʒu��Ԃ�	    */


static STBL_CMP	STBL_cmp = (STBL_CMP)strcmp;


STBL_CMP STBL_SetFncCmp(STBL_CMP cmp)
{
    if (cmp)
    	STBL_cmp = cmp;
    return STBL_cmp;
}


int STBL_Add(void *t[],	int *tblcnt, void *key)
    /*	t     :	������ւ̃|�C���^�������߂��z��    */
    /*	tblcnt:	�o�^�ό�  	    	    	    */
    /*	key   :	�ǉ����镶����	    	    	    */
    /*	���A�l:	0:�ǉ� -1:���łɓo�^��	    	    */
{
    int	 low, mid, f, hi;

    hi = *tblcnt;
    mid	= low =	0;
    while (low < hi) {
    	mid = (low + hi	- 1) / 2;
    	if ((f = STBL_cmp(key, t[mid]))	< 0) {
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


int STBL_Search(void *tbl[], int nn, void *key)
{
   /*
    *  key:������������ւ̃|�C���^
    *  tbl:������ւ̃|�C���^�������߂��z��
    *  nn:�z��̃T�C�Y
    *  ���A�l:��������������̔ԍ�(0���)  �݂���Ȃ������Ƃ�-1
    */
    int	    low, mid, f;

    low	= 0;
    while (low < nn) {
    	mid = (low + nn	- 1) / 2;
    	if ((f = STBL_cmp(key, tbl[mid])) < 0)
    	    nn = mid;
    	else if	(f > 0)
    	    low	= mid +	1;
    	else
    	    return mid;
    }
    return -1;
}



/* ------------------------------------------------------------------------ */
/* ��̃e�L�X�g�t�@�C���ǂ݂���   	    	    	    	    	    */

unsigned long	TXT1_line;
char	TXT1_name[FIL_NMSZ];
FILE	*TXT1_fp;


void TXT1_Error(char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
    fprintf(stdout, "%-12s %5d : ", TXT1_name, TXT1_line);
    vfprintf(stdout, fmt, app);
    va_end(app);
    return;
}


void TXT1_ErrorE(char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
    fprintf(stdout, "%-12s %5d : ", TXT1_name, TXT1_line);
    vfprintf(stdout, fmt, app);
    va_end(app);
    exit(1);
}


int TXT1_Open(char *name)
{
    TXT1_fp = fopen(name,"rt");
    if (TXT1_fp	== 0)
    	return -1;
    strcpy(TXT1_name, name);
    TXT1_line =	0;
    return 0;
}


void TXT1_OpenE(char *name)
{
    TXT1_fp = fopenE(name,"rt");
    strcpy(TXT1_name, name);
    TXT1_line =	0;
}


char *TXT1_GetsE(char *buf, int	sz)
{
    char *p;

    p =	fgets(buf, sz, TXT1_fp);
    if (ferror(TXT1_fp)) {
    	TXT1_Error("file read error\n");
    	exit(1);
    }
    TXT1_line++;
    return p;
}


void TXT1_Close(void)
{
    fcloseE(TXT1_fp);
}



/* ------------------------------------------------------------------------ */
/* ��̃e�L�X�g�t�@�C���o��	    	    	    	    	    	    */

uint32_t    OTXT1_line;
char	OTXT1_name[FIL_NMSZ];
FILE	*OTXT1_fp;


void OTXT1_Error(char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
    fprintf(stdout, "%-12s %5d : ", OTXT1_name,	OTXT1_line);
    vfprintf(stdout, fmt, app);
    va_end(app);
    return;
}


void OTXT1_ErrorE(char *fmt, ...)
{
    va_list app;

    va_start(app, fmt);
    fprintf(stdout, "%-12s %5d : ", OTXT1_name,	OTXT1_line);
    vfprintf(stdout, fmt, app);
    va_end(app);
    exit(1);
}


int OTXT1_Open(char *name)
{
    OTXT1_fp = fopen(name,"wt");
    if (OTXT1_fp == 0)
    	return -1;
    strcpy(OTXT1_name, name);
    OTXT1_line = 0;
    return 0;
}


void OTXT1_OpenE(char *name)
{
    OTXT1_fp = fopenE(name,"rt");
    strcpy(OTXT1_name, name);
    OTXT1_line = 0;
}


int OTXT1_PutsE(char *buf)
{
    OTXT1_line++;
    fputs(buf, OTXT1_fp);
    if (ferror(OTXT1_fp)) {
    	OTXT1_Error("file write error\n");
    	exit(1);
    }
    return 0;
}


void OTXT1_Close(void)
{
    fcloseE(OTXT1_fp);
}





/*--------------------------------------------------------------------------*/
