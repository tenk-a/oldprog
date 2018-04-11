#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <direct.h>
#include <io.h>

#include "subr.h"

/*--------------------------------------------------------------------------*/

int		debugflag;


/*--------------------------------------------------------------------------*/
/* �����񏈗��֌W															*/


char *strncpyZ(char *dst, const char *src, size_t size)
{
	strncpy(dst, src, size);
	dst[size-1] = 0;
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
	p = STREND(s);
	if (p != s && p[-1] == '\n') {
		p[-1] = 0;
	}
	return s;
}


long strtolKM(char *s, char **d, int r)
{
	long l;

	l = strtol(s, &s, r);
	if (*s == 'k' || *s == 'K') {
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
/* �G���[�����t���̕W���֐�													*/

void *mallocE(size_t a)
	/* �G���[������Α�exit�� malloc() */
{
	void *p;

	if (a == 0)
		a = 1;
	p = malloc(a);
	//printf("malloc(0x%x)\n",a);
	if (p == NULL) {
		err_exit("������������Ȃ�(%d byte(s))\n",a);
	}
	return p;
}


void *callocE(size_t a, size_t b)
	/* �G���[������Α�exit�� calloc() */
{
	void *p;

	if (a== 0 || b == 0)
		a = b = 1;
	p = calloc(a,b);
	//printf("calloc(0x%x,0x%x)\n",a,b);
	if (p == NULL) {
		err_exit("������������Ȃ�(%d*%d byte(s))\n",a,b);
	}
	return p;
}


void *reallocE(void *m, size_t a)
	/* �G���[������Α�exit�� calloc() */
{
	void *p;

	if (a == 0)
		a = 1;
	p = realloc(m, a);
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
	p = calloc(1,strlen(s)+8);
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



void *mallocMaE(int sz, int minSz, int aln)
{
	void *p;

	p = mallocMa(sz, minSz, aln);
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
	int  a;

	/* �A���C�����g�p�̃}�X�N�𐶐� */
	if (aln <= 0)
		a = 1;
	else
		a = (1<<aln);
	a = a - 1;

	/* �T�C�Y�𒲐� */
	sz = (sz + a) & ~a;
	minSz = (minSz + a) & ~a;

	p = malloc(sz);
	if (p)
		return p;

	/* �ŏ��T�C�Y���m�ۂł��邩�`�F�b�N */
	if (minSz <= 0)
		return NULL;
	p = malloc(minSz);
	if (p == NULL)
		return NULL;
	free(p);

	/* �m�ۂł���T�C�Y��T��*/
	do {
		sz = (sz/2 + a) & ~a;
		if (sz < minSz)
			sz = minSz;
		p = malloc(sz);
	} while (p == NULL && sz > minSz);

	return p;
}



/*--------------------------------------------------------------------------*/
/* �G���[�����A�f�o�b�O�p�֐�												*/


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
/* �t�@�C���������񏈗�														*/

int  file_sjisFlag = 1;


int	  file_setSjisMode(int sw)
{
	int n;

	n = file_sjisFlag;
	file_sjisFlag = sw;
	return n;
}


char *file_baseName(char *adr)
{
	char *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\')
			adr = p + 1;
		if (FILE_ISKANJI((*(unsigned char *)p)) && *(p+1) )
			p++;
		p++;
	}
	return adr;
}


char *file_extPtr(char *name)
{
	char *p;

	name = file_baseName(name);
	p = strrchr(name, '.');
	if (p) {
		return p+1;
	}
	return STREND(name);
}


char *file_chgExt(char filename[], const char *ext)
{
	char *p;

	p = file_baseName(filename);
	p = strrchr( p, '.');
	if (p == NULL) {
		if (ext) {
			strcat(filename,".");
			strcat( filename, ext);
		}
	} else {
		if (ext == NULL)
			*p = 0;
		else
			strcpy(p+1, ext);
	}
	return filename;
}


char *file_AddExt(char filename[], const char *ext)
{
	if (strrchr(file_baseName(filename), '.') == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	}
	return filename;
}


char *file_delLastDirSep(char *dir)
{
	/* ������̍Ō�� \ �� / ������΍폜 */
	char *p, *s;

	if (dir) {
		s = file_baseName(dir);
		if (strlen(s) > 1) {
			p = STREND(s);
			if (p[-1] == '/') {
				p[-1] = 0;
			} else if (p[-1] == '\\') {
				if (file_sjisFlag == 0) {
					p[-1] = 0;
				} else {
					int f = 0;
					while (*s) {
						f = 0;
						if (ISKANJI(*s) && s[1]) {
							s++;
							f = 1;
						}
						s++;
					}
					if (f == 0)
						p[-1] = 0;
				}
			}
		}
	}
	return dir;
}


char *file_NameUpr(char *s0)
{
	/* �S�p�Q�o�C�g�ڂ��l������ strupr */
	char *s = s0;

	while (*s) {
		if (FILE_ISKANJI(*s) && s[1]) {
			s += 2;
		} else if (ISLOWER(*s)) {
			*s = TOUPPER(*s);
			s++;
		} else {
			s++;
		}
	}
	return s0;
}


char *file_dirNameDupE(char *dir, char *name)
{
	void *m;

	m = file_dirNameDup(dir,name);
	if (m == NULL) {
		err_exit("������������܂���\n");
	}
	return m;
}


char *file_dirNameDup(char *dir, char *name)
{
	// �f�B���N�g�����ƃt�@�C�����������������̂�strdup����
	// �g���q�̕t���ւ����ł���悤�A+5�o�C�g�͗]���Ƀ������͊m�ۂ���
	int l,n;
	void *m;

	l = (dir) ? strlen(dir) : 0;
	n = (name) ? strlen(name) : 0;
	m = calloc(1, l+n+1+7);
	if (m == NULL)
		return NULL;
	if (l) {
		strcpy(m, dir);
		file_delLastDirSep(m);
		strcat(m, "\\");
	}
	if (n) {
		strcat(m, name);
	}
	return m;
}


#if 1
char *file_dirDirNameChgExt(char *onam, const char *dir, const char *mdir, const char *name, const char *chgext)
{
	if (onam == NULL || name == NULL || strcmp(name,".") == 0)
		return NULL;
	onam[0] = 0;
	if (dir && dir[0])
		strcpy(onam, dir);
	if (mdir && mdir[0]) {		// �\�[�X�f�B���N�g�����w�肳��Ă���΁A���̎w�薼�̃f�B���N�g�����t����
		if (onam[0])
			strcat(onam, "\\");
		strcat(onam, mdir);
	}
	/*if (name)*/ {
		if (onam[0])
			strcat(onam, "\\");
		strcat(onam, name);
		file_chgExt(onam, chgext);
	}
	return onam;
}


char *file_dirNameChgExt(char *nam, char *dir, char *name, char *chgext)
{
	if (name == NULL || strcmp(name,".") == 0)
		return NULL;
	if (dir && *dir && name[0] != '\\' && name[1] != ':') {
		sprintf(nam, "%s\\%s", dir, name);
	} else {
		sprintf(nam, "%s", name);
	}
	file_chgExt(nam, chgext);
	//strupr(nam);
	return nam;
}


char *file_dirNameAddExt(char *nam, char *dir, char *name, char *addext)
{
	if (name == NULL || strcmp(name,".") == 0)
		return NULL;
	if (dir && *dir && name[0] != '\\' && name[1] != ':') {
		sprintf(nam, "%s\\%s", dir, name);
	} else {
		sprintf(nam, "%s", name);
	}
	file_AddExt(nam, addext);
	//strupr(nam);
	return nam;
}
#endif


char *file_getMidDir(char mdir[], const char *name)
{
	// name ���̃h���C�u���ƃx�[�X���𔲂����f�B���N�g������mdir[]�ɓ���ĕԂ�
	char *d;

	if (mdir == NULL || name == NULL)
		return NULL;
	if (name[1] == ':')		// �h���C�u���t��������
		name += 2;
	if (name[0] == '\\')	// ���[�g�w�肳��Ă�
		name += 1;
	strcpy(mdir, name);
	d = file_baseName(mdir);
	if (d <= mdir) {
		mdir[0] = 0;
	} else if (d[-1] == '\\') {
		*d = '\0';
	}
	return mdir;
}



int file_Rename(char *oldname, char *newname)
{
	char bak[FILE_NMSZ];

	strcpy(bak, newname);
	file_chgExt(bak, "bak");
	remove(bak);
	rename(newname, bak);
	return rename(oldname, newname);
}


/* ------------------------- */

static int file_getTmpDirSub(char *t, int f);

int file_getTmpDirE(char *t)
{
	return file_getTmpDirSub(t, 1);
}


int file_getTmpDir(char *t)
{
	return file_getTmpDirSub(t, 0);
}


static int file_getTmpDirSub(char *t, int f)
{
	char *p;
	char nm[FILE_NMSZ+2];

	if (*t) {
		strcpy(nm, t);
		p = STREND(nm);
	} else {
		p = getenv("TMP");
		if (p == NULL) {
			p = getenv("TEMP");
			if (p == NULL) {
				if (f == 0)
					p = ".\\";
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
	_fullpath(t, nm, FILE_NMSZ);
	p = file_baseName(t);
	*p = 0;
	if (p[-1] == '\\')
		p[-1] = 0;
	return 0;
}


/*--------------------------------------------------------------------------*/

int file_isWildC(const char *onam)
{
	return (strpbrk(onam, "*?") != NULL);
}


#if 1

static struct _finddatai64_t file_findData;
static char file_findDir[FILE_NMSZ];
static char *file_findBase;
static long  file_findHdl;

char *file_findFirstName(char dst[], const char *src)
{
	file_findHdl = _findfirsti64((char *)src, &file_findData);
	if (file_findHdl == -1) {
		file_findDir[0] = 0;
		return NULL;
	}
	strncpyZ(file_findDir, src, FILE_NMSZ);
	file_findBase   = file_baseName(file_findDir);
	*file_findBase  = 0;
	//

	strcpy(dst, file_findDir);
	strncpyZ(dst+strlen(dst), file_findData.name, FILE_NMSZ-strlen(dst));

	if (file_findData.attrib & _A_SUBDIR) {
		return file_findNextName(dst);
	}
	return dst;
}


char *file_findNextName(char dst[])
{
	if (file_findHdl == -1)
		return NULL;
	do {
		if (_findnexti64(file_findHdl, &file_findData)) {
			_findclose(file_findHdl);
			file_findHdl = -1;
			return NULL;
		}
		strcpy(dst, file_findDir);
		strncpyZ(dst+strlen(dst), file_findData.name, FILE_NMSZ-strlen(dst));
	} while (file_findData.attrib & _A_SUBDIR);
	return dst;
}

#else
static WIN32_FIND_DATA file_findData;
static char file_findDir[FILE_NMSZ];
static char *file_findBase;
static HANDLE file_findHdl;

char *file_findFirstName(char dst[], const char *src)
{
	file_findHdl = FindFirstFile(src, &file_findData);
	if (file_findHdl == INVALID_HANDLE_VALUE) {
		file_findDir[0] = 0;
		return NULL;
	}
	strncpyZ(file_findDir, src, FILE_NMSZ);
	file_findBase   = file_baseName(file_findDir);
	*file_findBase  = 0;

	//
	strcpy(dst, file_findDir);
	strncpyZ(dst+strlen(dst), file_findData.cFileName, FILE_NMSZ-strlen(dst));

	if (file_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return file_findNextName(dst);
	}
	return dst;
}


char *file_findNextName(char dst[])
{
	if (file_findHdl == INVALID_HANDLE_VALUE)
		return NULL;
	do {
		if (FindNextFile(file_findHdl, &file_findData) == 0) {
			FindClose(file_findHdl);
			file_findHdl = INVALID_HANDLE_VALUE;
			return NULL;
		}
		strcpy(dst, file_findDir);
		strncpyZ(dst+strlen(dst), file_findData.cFileName, FILE_NMSZ-strlen(dst));
	} while (file_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	return dst;
}
#endif



#if 1	//

int file_fdateCmp(const char *tgt, const char *src)
{
	// ��̃t�@�C���̓��t�̑召���r����.
	// tgt ���V������� 1(��), �����Ȃ�� 0, tgt ���Â���� -1(��)
	struct _finddatai64_t srcData;
	struct _finddatai64_t tgtData;
	long   srcFindHdl, tgtFindHdl;
	time_t srcTm, tgtTm;

	srcFindHdl = _findfirsti64((char *)src, &srcData);
	srcTm = (srcFindHdl == -1) ? 0 : srcData.time_write;

	tgtFindHdl = _findfirsti64((char *)tgt, &tgtData);
	tgtTm = (tgtFindHdl == -1) ? 0 : tgtData.time_write;

DBG_F(("%x - %x = %d\n", tgtTm, srcTm, tgtTm - srcTm));

	if (tgtTm < srcTm)
		return -1;
	else if (tgtTm > srcTm)
		return 1;
	return 0;
}


#endif



/* ------------------------- */
/*--------------------------------------------------------------------------*/

void file_makeDmyFileE(char *oname, size_t fsiz, int fh)
{
	int c;

	c = file_makeDmyFile(oname, fsiz, fh);
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


int file_makeDmyFile(char *oname, size_t fsiz, int fh)
{
	FILE *fp;
	int  sz, rsz, n;
	char *b;

	sz  = 0x10000;
	n   = fsiz / sz;
	rsz = fsiz % sz;

	b   = malloc(sz);
	if (b == NULL) {
		return -3;
	}
	memset(b, fh, sz);
	fp = fopen(oname, "wb");
	if (fp == NULL)
		return -1;
	while (n--) {
		fwrite(b, 1, sz, fp);
		if (ferror(fp)) {
			fclose(fp);
			if (ferror(fp))
				fclose(fp);
			return -2;
		}
	}
	if (rsz) {
		fwrite(b, 1, rsz, fp);
		if (ferror(fp)) {
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

void *file_saveE(const char *name, void *buf, int size)
{
	void *p;

	p = file_save(name, buf, size);
	if (p == NULL) {
		err_exit("%s�̃Z�[�u�Ɏ��s���܂���\n", name);
	}
	return p;
}



FILE *fopenMD(const char *name, char *mode)
{
	// �f�B���N�g���@��@�\�t����fopen
	FILE *fp;
	char nm[FILE_NMSZ];
	char *s, *e;

	fp = fopen(name, mode);
	if (fp)
		return fp;
	if (strpbrk(mode, "wa") == NULL)	// �ǂݍ��݃��[�h�Ȃ�΃t�H���_���쐬���Ȃ�
		return NULL;
	strcpy(nm, name);
	e = nm + strlen(nm);
	for (;;) {
		s = file_baseName(nm);
		if (s <= nm || s[-1] != '\\')
			return NULL;
		if (mkdir(nm) == 0) {
			s[-1] = 0;
			do {
				s[-1] = '\\';
				s += strlen(s);
				if (s >= e)
					return fopen(name, mode);
			} while (mkdir(nm) == 0);
			return NULL;
		}
	}
}



void *file_save(const char *name, void *buf, int size)
{
	FILE *fp;

	fp = fopenMD(name,"wb");
	if (fp == NULL)
		return NULL;
	setvbuf(fp, NULL, _IOFBF, 1024*1024);
	if (size && buf) {
		fwrite(buf, 1, size, fp);
		if (ferror(fp)) {
			fclose(fp);
			buf = NULL;
		}
	}
	fclose(fp);
	return buf;
}


void *file_loadE(const char *name, void *buf, int bufsz, int *rdszp)
{
	void *p;

	p = file_load(name, buf, bufsz, rdszp);
	if (p == NULL) {
		err_exit("%s�̃��[�h�Ɏ��s���܂���\n", name);
	}
	return p;
}


void *file_load(const char *name, void *buf, int bufsz, int *rdszp)
{
	/* name : �ǂ݂��ރt�@�C�� */
	/* buf  : �ǂ݂��ރ������BNULL���w�肳���� malloc���A16�o�C�g�]���Ɋm�ۂ��� */
	/* bufsz: buf�̃T�C�Y�B0���w�肳���� �t�@�C���T�C�Y�ƂȂ� */
	/* rdszp: NULL�łȂ���΁A�ǂ݂��񂾃t�@�C���T�C�Y�����ĕԂ� */
	/* �߂�l: buf�̃A�h���X��malloc���ꂽ�A�h���X. �G���[����NULL��Ԃ� */
	FILE *fp;
	int  l;

	fp = fopen(name, "rb");
	if (fp == NULL)
		return NULL;
	l = filelength(fileno(fp));
	if (rdszp)
		*rdszp = l;
	if (bufsz == 0)
		bufsz = l;
	if (l > bufsz)
		l = bufsz;
	if (buf == NULL) {
		bufsz = (bufsz + 15 + 0x4000) & ~15;
		buf = calloc(1, bufsz);
		if (buf == NULL)
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
/* �t�@�C���̓��t���擾														*/

#ifdef __BORLANDC__

FILE_TIME file_timeGet(char *srcname)
{
	FILE		*fp;
	int    		f;
	FILE_TIME	fdt;

	fp = fopen(srcname, "rb");
	if (fp == NULL) {
		memset(&fdt, 0, sizeof fdt);
		return fdt;
	}
	f = getftime(fileno(fp), &fdt);
	fclose(fp);
	if (f < 0) {
		memset(&fdt, 0, sizeof fdt);
		return fdt;
	}
	return fdt;
}


int file_timeSet(char *dstname, FILE_TIME fdt)
{
	FILE *fp;
	int    f;

	fp = fopen(dstname, "ab+");
	if (fp == NULL) {
		return -1;
	}
	f = setftime(fileno(fp), &fdt);
	fclose(fp);
	if (f < 0) {
		return -1;
	}
	return 0;
}


int file_timeCmp(FILE_TIME t1, FILE_TIME t2)
{
	int f;

	f = t1.ft_year  - t2.ft_year;	if (f)	return f;
	f = t1.ft_month - t2.ft_month;	if (f)	return f;
	f = t1.ft_day   - t2.ft_day;	if (f)	return f;
	f = t1.ft_hour  - t2.ft_hour;	if (f)	return f;
	f = t1.ft_min   - t2.ft_min;	if (f)	return f;
	f = t1.ft_tsec  - t2.ft_tsec;
	return f;
}

#endif /* __BORLANDC__ */

/* ------------------------------------------------------------------------ */
/* �G���[exit���� �t�@�C���֐�												*/


FILE *fopenE(char *name, char *mod)
{
	/* �G���[������Α�exit�� fopen() */
	FILE *fp;

	fp = fopen(name,mod);
	if (fp == NULL) {
		err_exit("�t�@�C�� %s ���I�[�v���ł��܂���\n",name);
	}
	setvbuf(fp, NULL, _IOFBF, 1024*1024);
	return fp;
}


size_t  fwriteE(void *buf, size_t sz, size_t num, FILE *fp)
{
	/* �G���[������Α�exit�� fwrite() */
	size_t l;

	l = fwrite(buf, sz, num, fp);
	if (ferror(fp)) {
		fcloseE(fp);
		err_exit("�t�@�C�������݂ŃG���[����\n");
	}
	return l;
}


size_t  freadE(void *buf, size_t sz, size_t num, FILE *fp)
{
	/* �G���[������Α�exit�� fread() */
	size_t l;

	l = fread(buf, sz, num, fp);
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
		if (ferror(fp)) {
			fclose(fp);
		}
	}
}


size_t	flength(FILE *fp)
{
	/* �t�@�C���T�C�Y�����߂� */
	/* �ߋ��Ƃ̌݊��̂��߁A�g���Ȃ疼�O�t���ւ��끄���� */

	return filelength(fileno(fp));
}


size_t	file_size(const char *name)
{
	FILE *fp = fopen(name, "rb");
	int sz;
	if (fp == NULL)
		return 0;
	sz = filelength(fileno(fp));
	fclose(fp);
	return sz;
}



/* ------------------------------------------------------------------------ */
/* �G���[exit���� fgetc,fputc�t�@�C���֐�									*/


int fgetcE(FILE *fp)
{
	/* fp��� 1�o�C�g(0..255) �ǂݍ���. �G���[������Α��I�� */
	static uint8_t buf[1];

	freadE(buf, 1, 1, fp);
	return (uint8_t)buf[0];
}


int fgetc2iE(FILE *fp)
{
	/* fp��� ��ٴ��ި�݂� 2�o�C�g�ǂݍ���. �G���[������Α��I�� */
	int c;

	c = fgetcE(fp);
	return (uint16_t)(c + (fgetcE(fp)<<8));
}


int fgetc4iE(FILE *fp)
{
	/* fp��� ��ٴ��ި�݂� 4�o�C�g�ǂݍ���. �G���[������Α��I�� */
	int c;

	c = fgetc2iE(fp);
	return c + (fgetc2iE(fp)<<16);
}


int fgetc2mE(FILE *fp)
{
	/* fp��� big���ި�݂� 2�o�C�g�ǂݍ���. �G���[������Α��I�� */
	int c;

	c = fgetcE(fp);
	return (uint16_t)((c<<8) + fgetcE(fp));
}


int fgetc4mE(FILE *fp)
{
	/* fp��� big���ި�݂� 4�o�C�g�ǂݍ���. �G���[������Α��I�� */
	int c;

	c = fgetc2mE(fp);
	return (c<<16) + fgetc2mE(fp);
}


void fputcE(int c, FILE *fp)
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
	int n;

	n = strlen(s);
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
/* BM�@��p���������o�C�g�f�[�^�̃T�[�`										*/
/* �� C����ɂ��A���S���Y�����T�̂�����									*/

void *memSearch(void *area0, int asiz, const void *ptn0, int ptnlen)
{
	/* area0, asiz : �����J�n�A�h���X�Ƃ��͈̔�     */
	/* ptn0, ptnlen: �����f�[�^�Ƃ��̃T�C�Y(byte��) */
	int skp[256];
	int i, j, k;
	int c, tail;
	unsigned char *area = (unsigned char *)area0;
	const unsigned char *ptn  = (const unsigned char *)ptn0;

	if (ptnlen <= 0 || asiz <= 0) {
		return NULL;
	}
	tail = ptn[ptnlen - 1];				/* �Ō�̕���(1byte) */
	if (ptnlen == 1) {					/* ����1�Ȃ�ȒP! */
		for (i = 0; i < asiz; i++) {	/* �P���ɓ���1byte��������܂Ō��� */
			if (area[i] == tail)
				return area + i;		/* �݂����� */
		}
	} else {							/* ����2�ȏ�̂Ƃ��\���� */
										/* skp[]�ɁA���̕��������������O�̂Ƃ��Ɏ��̌����J�n�ʒu�փX�L�b�v����o�C�g����ݒ肷�� */
		for (i = 0; i < 256; i++)		/* �܂�256�������ׂĕs��v�̏ꍇ�Ƃ��Č����f�[�^�T�C�Y���̃X�L�b�v�l��ݒ� */
			skp[i] = ptnlen;
		for (i = 0; i < ptnlen-1; i++)	/* ���Ɍ����f�[�^���\�����镶���̏ꍇ�́A���̕��������̌����f�[�^�̃o�C�g�������X�L�b�v�ɂ��� */
			skp[ptn[i]] = ptnlen - 1 - i;
		/* i = ptnlen - 1; */			/* for���̌��ʂ����̎��ɓ���������s�v */
		/* �w��͈͂̌����J�n�I */
		while (i < asiz) {
			c = area[i];				/* �����f�[�^�̔�����1byte���`�F�b�N����. */
			if (c == tail) {			/* �����ňႦ�΁A(�ő�)�����f�[�^�o�C�g���́A��r�����X�L�b�v�ł��� */
				j = ptnlen - 1;			/* �����Ȃ�΁A����Ɍ����f�[�^����납���r���� */
				k = i;
				while (ptn[--j] == area[--k]) {
					if (j == 0)			/* �����f�[�^�̐擪�Ȃ�� */
						return area + k;/* �������� */
				}
			}
			i += skp[c];				/* ���̈ʒu�Ń}�b�`���Ȃ������̂ŁA���̈ʒu�܂ŃX�L�b�v */
		}
	}

	/* ������Ȃ����� */
	return NULL;
}


void *memStr(void *mem, const char *ptn, int memSz)
{
	return memSearch(mem, memSz, ptn, strlen(ptn));
}



/* ------------------------------------------------------------------------ */
/* ������̃��X�g���쐬		*/

slist_t *slist_add(slist_t **p0, char *s)
{
	/* ������̃��X�g��ǉ�		*/
	slist_t* p;

	p = *p0;
	if (p == NULL) {
		p = callocE(1, sizeof(slist_t));
		p->s = strdupE(s);
		*p0 = p;
	} else {
		while (p->link != NULL) {
			p = p->link;
		}
		p->link = callocE(1, sizeof(slist_t));
		p = p->link;
		p->s = strdupE(s);
	}
	return p;
}


void slist_free(slist_t **p0)
{
	/* ������̃��X�g���폜	*/
	slist_t *p, *q;

	for (p = *p0; p; p = q) {
		q = p->link;
		freeE(p->s);
		freeE(p);
	}
	*p0 = NULL;
}



/* ------------------------------------------------------------------------ */
/* �t�@�C���� �̎擾 */
#if 0

static flist_mode = 0;
static flist_srcDir[FILE_NMSZ];
static flist_dstDir[FILE_NMSZ];
static flist_srcExt[FILE_NMSZ];
static flist_dstExt[FILE_NMSZ];
static flist_outName[FILE_NMSZ];


void flist_setMode(int mode)
{
	flist_mode = mode;
}

void flist_setSrcDir(const char *src)
{
	strcpy(flist_srcDir, src);
}

void flist_setSrcExt(const char *src)
{
	strcpy(flist_srcExt, src);
}


void flist_setDstDir(const char *dst)
{
	strcpy(flist_dstDir, dst);
}


void flist_setDstExt(const char *dst)
{
	strcpy(flist_dstExt, dst);
}


void flist_setOutName(const char *dst)
{
	strcpy(flist_outName, dst);
	file_AddExt(flist_outName, flist_dstExt);
}



static flist_t *flist_add1(flist_t **p0, const char *nm)
{
	/* ������̃��X�g��ǉ�		*/
	char buf[FILE_NMSZ], mdir[FILE_NMSZ];
	flist_t* p;

	p = *p0;
	if (p == NULL) {
		p = callocE(1, sizeof(flist_t));
		*p0 = p;
	} else {
		while (p->link != NULL) {
			p = p->link;
		}
		p->link = callocE(1, sizeof(flist_t));
		p = p->link;
	}

	file_dirNameAddExt(buf, flist_srcDir, nm, flist_srcExt);
	p->s = strdupE(buf);
	mdir[0] = 0;
	if (flist_srcDir[0])
		file_getMidDir(mdir, nm);
	if (flist_outName[0]) {
		p->d = strdupE(flist_outName);
		flist_outName[0] = 0;
	} else {
		file_dirDirNameChgExt(buf, flist_dstDir, mdir, nm, flist_dstExt);
		p->d = strdupE(buf);
	}
	return p;
}


flist_t *flist_add(flist_t **p0, const char *s)
{
	char nm[FILE_NMSZ];

	if (file_isWildC(s) == 0) {
		return flist_add1(p0, s);
	}
	if (file_findFirstName(nm, s)) {
		do {
			slist_add1(p0, nm);
		} while (file_findNextName(nm));
	}
	return NULL;
}

#endif



/* ------------------------------------------------------------------------ */
/* �\�[�g�ςݕ�����|�C���^�̔z����A��������������Ă��̈ʒu��Ԃ�		*/


static STBL_CMP STBL_cmp = (STBL_CMP)strcmp;


STBL_CMP STBL_SetFncCmp(STBL_CMP cmp)
{
	if (cmp)
		STBL_cmp = cmp;
	return STBL_cmp;
}


int STBL_Add(void *t[], int *tblcnt, void *key)
	/*  t     : ������ւ̃|�C���^�������߂��z��	*/
	/*  tblcnt: �o�^�ό�							*/
	/*  key   : �ǉ����镶����						*/
	/*  ���A�l: 0:�ǉ� -1:���łɓo�^��				*/
{
	int  low, mid, f, hi;

	hi = *tblcnt;
	mid = low = 0;
	while (low < hi) {
		mid = (low + hi - 1) / 2;
		if ((f = STBL_cmp(key, t[mid])) < 0) {
			hi = mid;
		} else if (f > 0) {
			mid++;
			low = mid;
		} else {
			return -1;	/* �������̂��݂������̂Œǉ����Ȃ� */
		}
	}
	(*tblcnt)++;
	for (hi = *tblcnt; --hi > mid;) {
		t[hi] = t[hi-1];
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
	int     low, mid, f;

	low = 0;
	while (low < nn) {
		mid = (low + nn - 1) / 2;
		if ((f = STBL_cmp(key, tbl[mid])) < 0)
			nn = mid;
		else if (f > 0)
			low = mid + 1;
		else
			return mid;
	}
	return -1;
}





/*--------------------------------------------------------------------------*/
/* ��̃e�L�X�g�t�@�C���ǂ݂���											*/

unsigned long	TXT1_line;
char	TXT1_name[FILE_NMSZ];
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


int TXT1_Open(const char *name)
{
	TXT1_fp = fopen(name,"rt");
	if (TXT1_fp == 0)
		return -1;
	strcpy(TXT1_name, name);
	TXT1_line = 0;
	return 0;
}


void TXT1_OpenE(const char *name)
{
	TXT1_fp = fopenE((char*)name,"rt");
	strcpy(TXT1_name, name);
	TXT1_line = 0;
}


char *TXT1_GetsE(char *buf, int sz)
{
	char *p;

	p = fgets(buf, sz, TXT1_fp);
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
/* ��̃e�L�X�g�t�@�C���o��												*/

uint32_t	OTXT1_line;
char	OTXT1_name[FILE_NMSZ];
FILE	*OTXT1_fp;


void OTXT1_Error(char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	fprintf(stdout, "%-12s %5d : ", OTXT1_name, OTXT1_line);
	vfprintf(stdout, fmt, app);
	va_end(app);
	return;
}


void OTXT1_ErrorE(char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	fprintf(stdout, "%-12s %5d : ", OTXT1_name, OTXT1_line);
	vfprintf(stdout, fmt, app);
	va_end(app);
	exit(1);
}


int OTXT1_Open(char *name)
{
	OTXT1_fp = fopenMD(name,"wt");
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
