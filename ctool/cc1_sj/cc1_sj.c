/*
  cc1_sj
  mingw �� cc1 ���s�O�Ƀ\�[�X���̃}���`�o�C�g���� 0x??5C �� \ �΍���{�����b�p�[

  �Y���� cc1.exe(cc1plus.exe) �� cc1_org.exe(cpp1plus_org.exe) ��rename������A
  cc1_sj.exe �� cc1.exe(cc1plus.exe)�̖��ł��̃f�B���N�g���ɃR�s�[���ė��p�B
  �� ���̃v���O�������g��Usage�\���͖����B

[memo]
�@cpp_sj �� gcc v3 �Ή��ŁB
�@gcc v3 ����O�� cpp ���Ă΂�Ă��Ȃ��悤�Ȃ̂ŁA
�@'\'�΍􂵂�C/C++�\�[�X���e���|�����ɍ���Ă���� cc1 �ɓn���悤�ɕύX�B
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <process.h>
#include <time.h>
#if defined(_WIN32)
#  include <windows.h>
#endif

#define STDERR			stderr
#define LBUFSIZE		(64*1024)		/* �s�o�b�t�@�T�C�Y. */

typedef unsigned char uchar;

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <mbstring.h>
#elif defined(__BORLANDC__)
#  include <mbstring.h>
#  define  _spawnve			spawnve
#else		// �V�t�gJIS��p
#  define _ismbblead(c)		((uchar)(c) >= 0x81 && ((uchar)(c) <= 0x9F || ((uchar)(c) >= 0xE0 && (uchar)(c) <= 0xFC)))
#  define _ismbbtrail(c)	((uchar)(c) >= 0x40 && (uchar)(c) <= 0xfc && (c) != 0x7f)
#endif

#define USE_MBC_FNAME					/* �t�@�C�������̑S�p�����ɑΉ�����ꍇ�� �����define */



static char *fgetStr(char *buf, long len, FILE *fp);
static void chkAddYen(const char *inName, const char *outName);
static void erPrintf(const char *fmt, ...);
void *callocE(size_t a, size_t b);
void err_exit(char *fmt, ...);
char *fname_getBase(char *adr);
char *fname_delDotDotDir(char *path);


int main(int argc, char *argv[], char *envp[])
{
	const char *fname=NULL;
	const char *appName;
	char  *exeName;
	char  *ctmpName;
	char  *p;
	char  *b;
	int   cppMode = 0;
	int   dbgFlg  = 0;
	int	  fnameNo = -1;
	int   n, i;

	// cpp0org�̃p�X����ݒ�
  #if defined(_WIN32)		// �ŋ߂̃R���p�C���� argv[0] �Ƀt���p�X�����Ȃ��̂����s�̂悤(T T)
	{
		static char buf[4096+4];
		GetModuleFileName(NULL, buf, (sizeof buf) - 1);
		appName = buf;
	}
  #else
	appName = argv[0];
  #endif

	// �{����exe�����쐬
	exeName = callocE(1, strlen(appName) + 16);
	strcpy(exeName, appName);
	p = strrchr(exeName, '.');
	if (p) {
		memmove(p+4, p, strlen(p)+1);
		memcpy(p, "_org", 4);
	} else {	// �G���[�����̎蔲��
		strcat(exeName, "_org.exe");
	}
	fname_delDotDotDir(exeName);

	// �V�t�gJIS�����ϊ��w�肪���ꂽ���`�F�b�N. �t�@�C�������`�F�b�N
	b = "";
	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			if (strcmp(p, "-D_CPP_SJ") == 0) {
				cppMode = 1;
			} else if (strcmp(p, "-D_CPP_SJ=0") == 0) {
				cppMode = 0;
			} else if (strcmp(p, "-D_CPP_SJ=1") == 0) {
				cppMode = 1;
			} else if (strcmp(p, "-D_CPP_SJ=2") == 0) {	//����͔p�ĂŁA�폜�\��
				cppMode = 2;
			} else if (strcmp(p, "-D_CPP_SJ=-1") == 0) {	// �f�o�b�O�p
				cppMode = 1;
				dbgFlg  = 1;
			}
		} else {
			if (strcmp(b, "-o") == 0) {
			} else if (strcmp(b, "-dumpbase") == 0) {
			} else if (strcmp(b, "-auxbase") == 0) {
			} else if (strcmp(b, "-auxbase-strip") == 0) {
			} else if (strcmp(b, "-aux-info") == 0) {
			} else if (strcmp(b, "-G") == 0) {
			} else if (strcmp(b, "-MQ") == 0) {
			} else if (strcmp(b, "-MT") == 0) {
			} else if (strcmp(b, "-MF") == 0) {
			} else if (strcmp(b, "-I") == 0) {
			} else if (strcmp(b, "-iprefix") == 0) {
			} else if (strcmp(b, "-include") == 0) {
			} else if (strcmp(b, "-imacros") == 0) {
			} else if (strcmp(b, "-iwithprefix") == 0) {
			} else if (strcmp(b, "-iwithprefixbefore") == 0) {
			} else if (strcmp(b, "-isystem") == 0) {
			} else if (strcmp(b, "-idirafter") == 0) {
			//} else if (strcmp(b, "--param") == 0) {
			} else if (fname == NULL) {
				fname = p;
				fnameNo = i;
			} else {
				erPrintf("cc1_sj: too many file names(%s)\n", p);
			}
		}
		b = p;
	}

	if (dbgFlg) {
		printf("exe:%s\n",exeName);
		for (i = 0; i < argc; i++) {
			printf("%s\n", argv[i]);
		}
	}

	if (fname == 0 || fnameNo < 0) {
		err_exit("cc1_sj: no file name\n");
	}

	// �t�@�C�����̏���
	//ctmpName  = _tempnam(".", "cc1sj");
	//if (ctmpName == NULL)
	{
		time_t tim;
		time(&tim);		// �b��...
		ctmpName  = callocE(1, strlen(fname) + 256);
		sprintf(ctmpName, "%s.~ctmp%x", fname, (unsigned)tim);
	}
	if (dbgFlg) {
		printf("%s\n", fname);
		printf("%s\n", ctmpName);
	}

	if (cppMode) {	// �V�t�gJIS�S�p������΍�����Ă��� cpp �����s����
		// ctmpName �̃e�L�X�g���̑S�p�̉��ʃo�C�g�� 0x5c �̕����̒���� \ ��}��
		chkAddYen(fname, ctmpName);
		argv[fnameNo] = ctmpName;
		n = _spawnve(_P_WAIT, exeName, argv, envp);			// ��cc1.exe �̎��s
		if (dbgFlg == 0)
			remove(ctmpName);
		return n;
	} else {		// �V�t�gJIS�S�p������΍�����Ȃ�
		return _spawnve(_P_WAIT, exeName, argv, envp);		// ��cc1.exe �̎��s
	}
}



/*--------------------------------------------------------------------------*/
static const char *src_name;
static int  src_line;
static char	src_lineContinueFlg = 0;


/** tmpName �̃e�L�X�g���̑S�p�̉��ʃo�C�g�� 0x5c �̕����̒���� \ ��}�� */
static void chkAddYen(const char *inName, const char *outName)
{
	FILE	*fp,*ofp;
	char	*p;
	char 	*buf;
	int 	c, k, mode;

	buf = callocE(1, LBUFSIZE*4+4);
	fp = fopen(inName, "rt");
	if (fp == NULL) {
		err_exit("no input file '%s'.\n", inName);
	}
	ofp = fopen(outName, "wt");
	if (ofp == NULL) {
		err_exit("cannot open output file `%s'.\n", outName);
	}

	// ���܂��Ȃ��̈�s���o��
	fprintf(ofp, "# 1 \"%s\"\n", inName);

	src_name = inName;
	src_line = 0;
	src_lineContinueFlg = 0;
	mode = 0;
	for (; ;) {
		++src_line;
		if (src_lineContinueFlg) {
			src_lineContinueFlg = 0;
			--src_line;
		}
		if (fgetStr(buf, LBUFSIZE, fp) == NULL)
			break;
		p = buf;
		for (; ;) {
			c = *p++;
			if (c == '\0') {
				break;
			} else if (c == '"') {
				if (mode == 0)
					mode = '"';
				else if (mode == '"')
					mode = 0;
			} else if (c == '\'') {
				if (mode == 0)
					mode = '\'';
				else if (mode == '\'')
					mode = 0;
			} else if (_ismbblead(c)) {
				k = *p++;
				if (_ismbbtrail(k) == 0) {
					erPrintf("bad MULTI-BYTE-CHAR:%02x%02x\n", c, k);
					//--p;
				}
				if (k == '\0') {
					break;
				}
				if (k == '\\') {
					if (*p == '\n' && (mode == '/' || mode == '*')) {
						// �R�����g���̍s���ɖ�蕶��������ꍇ��'\'�łȂ�'_'�����Ă���.
						// �R�����g����\\�G�X�P�[�v�����͂���Ȃ��ōs�A������������.
						// ��' '��}�������ꍇ�A�폜����Ă��܂��čs�A��������(T T)
						memmove(p+1, p, strlen(p)+1);
						*p++ = '_';
					} else /*if (mode == '\'' || mode == '"')*/ {
						memmove(p+1, p, strlen(p)+1);
						*p++ = '\\';
					}
				}
			} else if (c == '\\') {
				c = *p++;
				if (c == '\0')
					break;
				if (_ismbblead(c))
					--p;
			} else if (mode == '*' && c == '*') {
				if (*p == '/') {
					p++;
					mode = 0;
				}
			} else if (mode == 0 && c == '/') {
				if (*p == '*') {
					p++;
					mode = '*';
				} else if (*p == '/') {
					p++;
					mode = '/';
					//break;
				}
			}
		}
		if (mode == '/')
			mode = 0;
		fputs(buf, ofp);
		if (ferror(ofp)) {
			break;
		}
	}
	free(buf);
	if (ferror(ofp)) {
		fprintf(STDERR, "%s %d : write error.\n", outName, src_line);
	}
	fclose(ofp);
	if (ferror(fp)) {
		erPrintf("read error.\n");
	}
	fclose(fp);
}



/** ��s����. '\0'���������Ă��邩�̃`�F�b�N������ */
static char *fgetStr(char *buf, long len, FILE *fp)
{
	char *p;
	int i, c;

	--len;
	p = buf;
	i = 0;
	do {
		if (i++ == len) {
			src_lineContinueFlg = 1;
			erPrintf("too long line.\n");
			break;
		}
		c = fgetc(fp);
		if (ferror(fp) || feof(fp)) {
			buf[0] = 0;
			return NULL;
		}
		if (c == '\0') {
			erPrintf("found '\\0'.\n");
			c = ' ';
		}
		*p++ = c;
	} while (c != '\n');
	*p = 0;
	return buf;
}



/** �ϊ����ɖ��̂������s��\�����邽�߂�printf */
static void erPrintf(const char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	fprintf(STDERR, "%s %d : ", src_name, src_line);
	vfprintf(STDERR, fmt, app);
	va_end(app);
}



/*--------------------------------------------------------------------------*/

/** �G���[������Α�exit�� calloc() */
void *callocE(size_t a, size_t b)
{
	void *p;

	if (a== 0 || b == 0)
		a = b = 1;
	p = calloc(a,b);
	//printf("calloc(0x%x,0x%x)\n",a,b);
	if (p == NULL) {
		err_exit("not enough memory.(%d*%d byte(s))\n",a,b);
	}
	return p;
}



void err_exit(char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	vfprintf(STDERR, fmt, app);
	va_end(app);
	exit(1);
}



/** �p�X�����̃t�@�C�����ʒu��T��(MS-DOS�ˑ�) */
char *fname_getBase(char *adr)
{
	char *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\')
			adr = p + 1;
	  #ifdef USE_MBC_FNAME
		if (_ismbblead((*(uchar *)p)) && *(p+1) )
			p++;
	  #endif
		p++;
	}
	return adr;
}


/** path ���� ./ �� ../ �̃T�u�f�B���N�g�������[���ɏ]���č폜 (MS-DOS�ˑ�) */
char *fname_delDotDotDir(char *path)
{
	uchar *p = (uchar*)path;
	uchar *d;
	uchar *dir = NULL;
	int c;

	if (*p && p[1] == ':') {		// �Ƃ肠�����A�蔲���łP���������B�l�b�g���[�N�Ƃ��͍l���Ȃ�
		p += 2;
	}
	if (memcmp(p, "//", 2) == 0 || memcmp(p, "\\\\", 2) == 0) {		// �l�b�g���[�N�R���s���[�^�H
		p += 2;
		dir = p;
	}
	d = p;
	while (*p) {
		c = *p++;
		if (c == '/' || c == '\\') {
			c = '\\';
			*d++ = c;
			if (p[0] == '.') {
				if (p[1] == '/' || p[1] == '\\') {
					p += 2;
				} else if (dir && p[1] == '.' && (p[2] == '/' || p[2] == '\\')) {
					p += 3;
					d = dir;
				}
			}
			dir = d;
	  #ifdef USE_MBC_FNAME
		} else if (_ismbblead(c) && *p) {
			*d++ = c;
			*d++ = *p++;
	  #endif
		} else {
			*d++ = c;
		}
	}
	*d = 0;
	return path;
}


