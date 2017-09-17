/*
  cpp_sj
  mingw �� cpp ���s��� �V�t�gJIS �S�p���� 0x??5C �� \ �΍���{�����b�p�[

  �Y���� cpp.exe(cpp0.exe) �� cpp_org.exe(cpp0_org.exe) �� rename ������A
  cpp_sj.exe �� cpp.exe(cpp0.exe)�̖��ł��̃f�B���N�g���ɃR�s�[���ė��p�B
  �� ���̃v���O�������g��Usage�\���͖����B

[memo]
�@�茳�� gcc version 2.95.3-5 �� gcc -v tst.c �Ƃ��� cpp0.exe �̈������݂āA
�@�����̍Ō�̂Q���K�����̓t�@�C���Əo�̓t�@�C���̃y�A���낤�A
�@����ȊO�̈����̓I�v�V�������낤�A�Ɖ��߂��āA�ٍ� czenyen ����������
�@��������㕨�B
�@����́AC�\�[�X�� \ �����������e���|�����t�@�C�����쐬���Ă����cpp�ɓn��
�@���s�A�ł����e�L�X�g��#�s���̃\�[�X�t�@�C�����𒲐����Ȃ����B
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <process.h>
#include <time.h>

#if defined(_WIN32)
# include <windows.h>
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

#define USE_SJIS_FNAME					/* �t�@�C�������̑S�p�����ɑΉ�����ꍇ�� �����define*/



static char *fgetStr(char *buf, long len, FILE *fp);
static int chkAddYen(const char *inName, const char *outName);
static int chgShpLnFlNm(const char *tmpName, const char *srcName, const char *inName, const char *outName);
static char *fname_dupDirSep(char *dst, const char *src);
static void erPrintf(const char *fmt, ...);
void *callocE(size_t a, size_t b);
void err_exit(char *fmt, ...);
char *fname_getBase(char *adr);
char *fname_delDotDotDir(char *path);


int main(int argc, char *argv[], char *envp[])
{
	const char *srcName=NULL, *dstName=NULL;
	const char *appName;
	char  *cppName, *ctmpName=NULL, *itmpName=NULL;
	char  *p;
	int   cppMode = 0, dbgFlg = 0;
  #ifdef GCC_V3	// for gcc v3
	int   srcNo=-1, dstNo=-1;
  #else // for gcc v2
	int   srcNo = argc-2, dstNo = argc-1;
  #endif
	int   n, i;

	// cpp0org�̃p�X����ݒ�
  #if defined(_WIN32)		// �ŋ߂̃R���p�C���� argv[0] �Ƀt���p�X�����Ȃ��̗��s�̂悤(T T)
	{
		static char buf[4096+4];
		GetModuleFileName(NULL, buf, (sizeof buf) - 1);
		appName = buf;
	}
  #else
	appName = argv[0];
  #endif

	// �{����exe�����쐬
	cppName = callocE(1, strlen(appName) + 16);
	strcpy(cppName, appName);
	p = strrchr(cppName, '.');
	if (p) {
		memmove(p+4, p, strlen(p)+1);
		memcpy(p, "_org", 4);
	} else {	// �G���[�����̎蔲��
		strcat(cppName, "_org.exe");
	}
	fname_delDotDotDir(cppName);

	// �����̐������Ȃ����(3�Ȃ����)���̂܂� cpp_org.exe �����s
	if (argc < 4)
		return _spawnve(_P_WAIT, cppName, argv, envp);

	// �V�t�gJIS�����ϊ��w�肪���ꂽ���`�F�b�N
	for (i = 1; i < argc; i++) {
		p = argv[i];
		//printf("%d %s\n", i, p);
		if (strcmp(p, "-D_CPP_SJ") == 0) {
			cppMode = 1;
		} else if (strcmp(p, "-D_CPP_SJ=0") == 0) {
			cppMode = 0;
		} else if (strcmp(p, "-D_CPP_SJ=1") == 0) {
			cppMode = 1;
		} else if (strcmp(p, "-D_CPP_SJ=2") == 0) {
			cppMode = 2;
		} else if (strcmp(p, "-D_CPP_SJ=-1") == 0) {	// �f�o�b�O�p
			cppMode = 1;
			dbgFlg  = 0;
	  #ifdef GCC_V3 // for gcc v3
		} else if (*p != '-' && strcmp(argv[i-1], "-iprefix") != 0) {
			srcNo   = dstNo;
			dstNo   = i;
	  #endif
		}
	}

	if (dbgFlg) {
		printf("exe:%s\n",appName);
		for (i = 0; i < argc; i++) {
			printf("%s\n", argv[i]);
		}
	}

	// �t�@�C�����̏���
	if (srcNo >= 0 && dstNo >= 0) {
		time_t tim;
		time(&tim);		// �b��...
		srcName = argv[srcNo];
		dstName = argv[dstNo];
		ctmpName  = callocE(1, strlen(srcName) + 256);
		sprintf(ctmpName, "%s.~ctmp%x", srcName, (unsigned)tim);
		itmpName = callocE(1, strlen(dstName) + 256);
		sprintf(itmpName, "%s.~itmp%x", dstName, (unsigned)tim);
		if (dbgFlg)
			printf("%s %s %s %s\n", srcName, dstName, ctmpName, itmpName);
	} else {
		cppMode = 0;
	}

	switch (cppMode) {
	case 1:		// �V�t�gJIS�S�p������΍�����Ă��� cpp �����s����
		// ctmpName �̃e�L�X�g���̑S�p�̉��ʃo�C�g�� 0x5c �̕����̒���� \ ��}��
		chkAddYen(srcName, ctmpName);
		argv[srcNo] = ctmpName;
		argv[dstNo] = itmpName;
		n = _spawnve(_P_WAIT, cppName, argv, envp);			// ��cpp.exe �̎��s
		if (dbgFlg == 0)
			remove(ctmpName);
		if (n != 0)
			return n;
		// # �s�ԍ� "�t�@�C����" �̃t�@�C����������C�\�[�X���ɕϊ�
		n = chgShpLnFlNm(ctmpName, srcName, itmpName, dstName);
		if (dbgFlg == 0)
			remove(itmpName);
		return n;

	case 2:		// cpp �����s��ɃV�t�gJIS�S�p������΍������
		argv[dstNo] = ctmpName;
		n = _spawnve(_P_WAIT, cppName, argv, envp);			// ��cpp.exe �̎��s
		if (n != 0)
			return n;
		// ctmpName �̃e�L�X�g���̑S�p�̉��ʃo�C�g�� 0x5c �̕����̒���� \ ��}��
		n = chkAddYen(ctmpName, dstName);
		if (dbgFlg == 0)
			remove(ctmpName);
		return n;

	default:	// �V�t�gJIS�S�p������΍�����Ȃ�
		return _spawnve(_P_WAIT, cppName, argv, envp);		// ��cpp.exe �̎��s
	}
}



/*--------------------------------------------------------------------------*/
static const char *src_name;
static int  src_line;
static char	src_lineContinueFlg = 0;


/** inName �̃e�L�X�g���̑S�p�̉��ʃo�C�g�� 0x5c �̕����̒���� \ ��}�� */
static int chkAddYen(const char *inName, const char *outName)
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
	c = 0;
	free(buf);
	if (ferror(ofp)) {
		fprintf(STDERR, "%s %d : write error.\n", outName, src_line);
		c = 1;
	}
	fclose(ofp);
	if (ferror(fp)) {
		erPrintf("read error.\n");
		c = 1;
	}
	fclose(fp);
	return c;
}



/** inName�̃t�@�C������ #line �̃t�@�C������tmpName�̂��̂�srcName�ɒu�������� outName�̃t�@�C���ɏo�� */
static int chgShpLnFlNm(const char *tmpName, const char *srcName, const char *inName, const char *outName)
{
	FILE	*fp,*ofp;
	char	*p, *buf, *tname, *sname;
	int 	c, tl, sl;

	buf = callocE(1, LBUFSIZE*4+4);
	fp = fopen(inName, "rt");
	if (fp == NULL) {
		err_exit("no input file '%s'.\n", inName);
	}

	ofp = fopen(outName, "wt");
	if (ofp == NULL) {
		err_exit("cannot open output file `%s'.\n", outName);
	}

	// �G���[�\���p�̃\�[�X�t�@�C�����ƍs�ԍ��̏�����
	src_name = inName;
	src_line = 0;
	src_lineContinueFlg = 0;

	// �u������t�@�C�����̏���. �f�B���N�g���Z�p���[�^�� \ �� \\ �ɕϊ�����K�v������
	tl = strlen(tmpName);
	tname = callocE(1, tl*4+2);
	fname_dupDirSep(tname, tmpName);
	tl = strlen(tname);
	sl = strlen(srcName);
	sname = callocE(1, sl*4+2);
	fname_dupDirSep(sname, srcName);
	sl = strlen(sname);

	for (; ;) {
		++src_line;
		if (src_lineContinueFlg) {
			src_lineContinueFlg = 0;
			--src_line;
		}
		if (fgetStr(buf, LBUFSIZE, fp) == NULL)
			break;
		p = buf;
		if (*p == '#' && p[1] == ' ' && isdigit(p[2])) {
			p = strchr(buf+3, '"');
			if (p) {
				p++;
				if (memcmp(p, tname, tl) == 0 && p[tl] == '"') {	// ��������
					memmove(p+sl, p+tl, strlen(p+tl)+1);
					memcpy(p, sname, sl);
				}
			}
		}
		fputs(buf, ofp);
		if (ferror(ofp))
			break;
	}
	free(buf);
	free(sname);
	free(tname);
	c = 0;
	if (ferror(ofp)) {
		fprintf(STDERR, "%s %d : write error.\n", outName, src_line);
		c = -1;
	}
	fclose(ofp);
	if (ferror(fp)) {
		erPrintf("read error.\n");
		c = -1;
	}
	fclose(fp);
	return c;
}


/** path ���� \ �� \\ �ɕϊ�����.�܂� 0x80�ȏ��\ooo�ƂW�i�\���ɕϊ�. */
static char *fname_dupDirSep(char *dst, const char *src)
{
	const uchar *s = (const uchar*)src;
	uchar *d = (uchar*)dst;
	int   c;

	while (*s) {
		c    = *s++;
		if (c == '\\') {
			*d++ = c;
			*d++ = c;
		} else if (c >= 0x7F) {
			*d++ = '\\';
			*d++ = ((c >> 6) & 3)+'0';
			*d++ = ((c >> 3) & 3)+'0';
			*d++ = ((c     ) & 3)+'0';
		} else {
			*d++ = c;
		}
	}
	*d = 0;
	return dst;
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

void *callocE(size_t a, size_t b)
{
	/* �G���[������Α�exit�� calloc() */
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
	  #ifdef USE_SJIS_FNAME
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
	  #ifdef USE_SJIS_FNAME
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



/*
[history]
  2001-11-18 v0.50 by tenk*
	czenyen�����ɂ������
  2001-11-23 v1.00
  	cpp�̌��\����������̂łȂ��Acpp�̑O�ɍs�����[�h��t���i�������{�Ɂj�B
  	������ -D_CPP_SJ,-D_CPP_SJ=2 �����ĕϊ����[�h��ݒ�A�����Ȃ�΍�����Ȃ��B
  2001-12-01 v1.01
  	�E#�s�̃t�@�C�����ϊ��ŁA�S�p������ \202\253 �̂悤�ɕϊ�����Ă����̂ɑΏ��B
  	�E' "�̑΂�R�����g�Ɋւ�炸 \ ��}������悤�ɕύX�B����ŁA#if 0���̒���
  	�@�����ꂽ�R�����g���C�ɂ��Ȃ��Ă��悢^^; ���̂��� -D_CPP_SJ=2 �͂����s�v�B
  2001-12-03 v1.10
	�E�Ăяo�� ��cpp �� cpp0org.exe �łȂ� ������ "_org" ��t�����t�@�C���Ƃ���B
	�E#include "�`.h" �� "�̏ꍇ�ɕs����������̂� ��cpp�ւ̒��ԃt�@�C�����A
	  c�\�[�X�Ɠ����f�B���N�g���ɂ��A�܂��A���ԃt�@�C�����̂�����ύX�B
  2001-12-05 v1.10
	�\�[�X�t�@�C�����̐����̏C��^^;
  2003-12    v1.11
	gcc v3 �Ή����悤�Ƃ�����A�O��cpp���Ă΂�Ȃ��Ȃ��Ă��ĈӖ��𐬂��Ȃ��B
  2003-13	 v1.12
	gcc v3 �΍�ŕʓr cc1_sj ���쐬�����Ƃ��̕ύX���t�B�[�h�o�b�N�B
	�E�R���p�C���� vc �ōs�����Ƃɂ��A�V�t�gJIS�ȊO�̃}���`�o�C�g�����ɑΉ�
	�@�ł���\���𑝂₷�B���̂��߃G���[���b�Z�[�W���p��ۂ�����B�܂��A
	�@�R�����g����\�݂蕶�����s���ɂ���Ƃ��ǉ����镶����S�p�󔒂��甼�p _
	�@�ɕύX�B
  2003-15
	���� mingw �ɂ� mbstring.h ���������̂ŁAmingw�ł� mbstring.h���g���悤��.
  2003-17
	�e���|�����t�@�C�����ɔO�̂��ߎ��Ԃ����ɂ�����������t��...�b��
	�����Ƃ������O�Â��͂ǂ�����񂾂�����...�X���b�hID�Ƃ�?

[memo]
�@�茳�� gcc version 2.95.3-5 �� gcc -v tst.c �Ƃ��� cpp0.exe �̈������݂āA
�@�����̍Ō�̂Q���K�����̓t�@�C���Əo�̓t�@�C���̃y�A���낤�A
�@����ȊO�̈����̓I�v�V�������낤�A�Ɖ��߂��āA�ٍ� czenyen ����������
�@��������㕨�B
�@����́AC�\�[�X�� \ �����������e���|�����t�@�C�����쐬���Ă����cpp�ɓn�����A
�@��� cpp.exe �����s���e���|�����ɓf���o�����く�����������t�@�C���𐶐��B
*/

