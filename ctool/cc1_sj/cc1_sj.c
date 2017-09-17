/*
  cc1_sj
  mingw で cc1 実行前にソース中のマルチバイト文字 0x??5C の \ 対策を施すラッパー

  該当の cc1.exe(cc1plus.exe) を cc1_org.exe(cpp1plus_org.exe) にrenameした後、
  cc1_sj.exe を cc1.exe(cc1plus.exe)の名でそのディレクトリにコピーして利用。
  ※ このプログラム自身のUsage表示は無し。

[memo]
　cpp_sj の gcc v3 対応版。
　gcc v3 から外部 cpp が呼ばれていないようなので、
　'\'対策したC/C++ソースをテンポラリに作ってそれを cc1 に渡すように変更。
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
#define LBUFSIZE		(64*1024)		/* 行バッファサイズ. */

typedef unsigned char uchar;

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <mbstring.h>
#elif defined(__BORLANDC__)
#  include <mbstring.h>
#  define  _spawnve			spawnve
#else		// シフトJIS専用
#  define _ismbblead(c)		((uchar)(c) >= 0x81 && ((uchar)(c) <= 0x9F || ((uchar)(c) >= 0xE0 && (uchar)(c) <= 0xFC)))
#  define _ismbbtrail(c)	((uchar)(c) >= 0x40 && (uchar)(c) <= 0xfc && (c) != 0x7f)
#endif

#define USE_MBC_FNAME					/* ファイル名中の全角文字に対応する場合は これをdefine */



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

	// cpp0orgのパス名を設定
  #if defined(_WIN32)		// 最近のコンパイラは argv[0] にフルパスを入れないのが流行のよう(T T)
	{
		static char buf[4096+4];
		GetModuleFileName(NULL, buf, (sizeof buf) - 1);
		appName = buf;
	}
  #else
	appName = argv[0];
  #endif

	// 本物のexe名を作成
	exeName = callocE(1, strlen(appName) + 16);
	strcpy(exeName, appName);
	p = strrchr(exeName, '.');
	if (p) {
		memmove(p+4, p, strlen(p)+1);
		memcpy(p, "_org", 4);
	} else {	// エラー処理の手抜き
		strcat(exeName, "_org.exe");
	}
	fname_delDotDotDir(exeName);

	// シフトJIS文字変換指定がされたかチェック. ファイル名をチェック
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
			} else if (strcmp(p, "-D_CPP_SJ=2") == 0) {	//これは廃案で、削除予定
				cppMode = 2;
			} else if (strcmp(p, "-D_CPP_SJ=-1") == 0) {	// デバッグ用
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

	// ファイル名の準備
	//ctmpName  = _tempnam(".", "cc1sj");
	//if (ctmpName == NULL)
	{
		time_t tim;
		time(&tim);		// 暫定...
		ctmpName  = callocE(1, strlen(fname) + 256);
		sprintf(ctmpName, "%s.~ctmp%x", fname, (unsigned)tim);
	}
	if (dbgFlg) {
		printf("%s\n", fname);
		printf("%s\n", ctmpName);
	}

	if (cppMode) {	// シフトJIS全角文字列対策をしてから cpp を実行する
		// ctmpName のテキスト中の全角の下位バイトが 0x5c の文字の直後に \ を挿入
		chkAddYen(fname, ctmpName);
		argv[fnameNo] = ctmpName;
		n = _spawnve(_P_WAIT, exeName, argv, envp);			// 元cc1.exe の実行
		if (dbgFlg == 0)
			remove(ctmpName);
		return n;
	} else {		// シフトJIS全角文字列対策をしない
		return _spawnve(_P_WAIT, exeName, argv, envp);		// 元cc1.exe の実行
	}
}



/*--------------------------------------------------------------------------*/
static const char *src_name;
static int  src_line;
static char	src_lineContinueFlg = 0;


/** tmpName のテキスト中の全角の下位バイトが 0x5c の文字の直後に \ を挿入 */
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

	// おまじないの一行を出力
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
						// コメント中の行末に問題文字がある場合は'\'でなく'_'を補っておく.
						// コメント中は\\エスケープ処理はされないで行連結が発生する.
						// ※' 'を挿入した場合、削除されてしまって行連結が発生(T T)
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



/** 一行入力. '\0'が混ざっているかのチェックをする */
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



/** 変換時に問題のあった行を表示するためのprintf */
static void erPrintf(const char *fmt, ...)
{
	va_list app;

	va_start(app, fmt);
	fprintf(STDERR, "%s %d : ", src_name, src_line);
	vfprintf(STDERR, fmt, app);
	va_end(app);
}



/*--------------------------------------------------------------------------*/

/** エラーがあれば即exitの calloc() */
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



/** パス名中のファイル名位置を探す(MS-DOS依存) */
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


/** path から ./ と ../ のサブディレクトリをルールに従って削除 (MS-DOS依存) */
char *fname_delDotDotDir(char *path)
{
	uchar *p = (uchar*)path;
	uchar *d;
	uchar *dir = NULL;
	int c;

	if (*p && p[1] == ':') {		// とりあえず、手抜きで１文字だけ。ネットワークとかは考えない
		p += 2;
	}
	if (memcmp(p, "//", 2) == 0 || memcmp(p, "\\\\", 2) == 0) {		// ネットワークコンピュータ？
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


