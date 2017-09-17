/*
  cpp_sj
  mingw で cpp 実行後に シフトJIS 全角文字 0x??5C の \ 対策を施すラッパー

  該当の cpp.exe(cpp0.exe) を cpp_org.exe(cpp0_org.exe) に rename した後、
  cpp_sj.exe を cpp.exe(cpp0.exe)の名でそのディレクトリにコピーして利用。
  ※ このプログラム自身のUsage表示は無し。

[memo]
　手元の gcc version 2.95.3-5 で gcc -v tst.c として cpp0.exe の引数をみて、
　引数の最後の２つが必ず入力ファイルと出力ファイルのペアだろう、
　それ以外の引数はオプションだろう、と解釈して、拙作 czenyen を改造して
　やっつけた代物。
　動作は、Cソースを \ 処理をしたテンポラリファイルを作成してそれをcppに渡し
　実行、できたテキストの#行末のソースファイル名を調整しなおす。
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

#define USE_SJIS_FNAME					/* ファイル名中の全角文字に対応する場合は これをdefine*/



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

	// cpp0orgのパス名を設定
  #if defined(_WIN32)		// 最近のコンパイラは argv[0] にフルパスを入れないの流行のよう(T T)
	{
		static char buf[4096+4];
		GetModuleFileName(NULL, buf, (sizeof buf) - 1);
		appName = buf;
	}
  #else
	appName = argv[0];
  #endif

	// 本物のexe名を作成
	cppName = callocE(1, strlen(appName) + 16);
	strcpy(cppName, appName);
	p = strrchr(cppName, '.');
	if (p) {
		memmove(p+4, p, strlen(p)+1);
		memcpy(p, "_org", 4);
	} else {	// エラー処理の手抜き
		strcat(cppName, "_org.exe");
	}
	fname_delDotDotDir(cppName);

	// 引数の数が少なければ(3つなければ)そのまま cpp_org.exe を実行
	if (argc < 4)
		return _spawnve(_P_WAIT, cppName, argv, envp);

	// シフトJIS文字変換指定がされたかチェック
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
		} else if (strcmp(p, "-D_CPP_SJ=-1") == 0) {	// デバッグ用
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

	// ファイル名の準備
	if (srcNo >= 0 && dstNo >= 0) {
		time_t tim;
		time(&tim);		// 暫定...
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
	case 1:		// シフトJIS全角文字列対策をしてから cpp を実行する
		// ctmpName のテキスト中の全角の下位バイトが 0x5c の文字の直後に \ を挿入
		chkAddYen(srcName, ctmpName);
		argv[srcNo] = ctmpName;
		argv[dstNo] = itmpName;
		n = _spawnve(_P_WAIT, cppName, argv, envp);			// 元cpp.exe の実行
		if (dbgFlg == 0)
			remove(ctmpName);
		if (n != 0)
			return n;
		// # 行番号 "ファイル名" のファイル名を元のCソース名に変換
		n = chgShpLnFlNm(ctmpName, srcName, itmpName, dstName);
		if (dbgFlg == 0)
			remove(itmpName);
		return n;

	case 2:		// cpp を実行後にシフトJIS全角文字列対策をする
		argv[dstNo] = ctmpName;
		n = _spawnve(_P_WAIT, cppName, argv, envp);			// 元cpp.exe の実行
		if (n != 0)
			return n;
		// ctmpName のテキスト中の全角の下位バイトが 0x5c の文字の直後に \ を挿入
		n = chkAddYen(ctmpName, dstName);
		if (dbgFlg == 0)
			remove(ctmpName);
		return n;

	default:	// シフトJIS全角文字列対策をしない
		return _spawnve(_P_WAIT, cppName, argv, envp);		// 元cpp.exe の実行
	}
}



/*--------------------------------------------------------------------------*/
static const char *src_name;
static int  src_line;
static char	src_lineContinueFlg = 0;


/** inName のテキスト中の全角の下位バイトが 0x5c の文字の直後に \ を挿入 */
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



/** inNameのファイル中の #line のファイル名がtmpNameのものをsrcNameに置き換えて outNameのファイルに出力 */
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

	// エラー表示用のソースファイル名と行番号の初期化
	src_name = inName;
	src_line = 0;
	src_lineContinueFlg = 0;

	// 置換するファイル名の準備. ディレクトリセパレータの \ を \\ に変換する必要がある
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
				if (memcmp(p, tname, tl) == 0 && p[tl] == '"') {	// 見つかった
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


/** path 中の \ を \\ に変換する.また 0x80以上は\oooと８進表現に変換. */
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

void *callocE(size_t a, size_t b)
{
	/* エラーがあれば即exitの calloc() */
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
	  #ifdef USE_SJIS_FNAME
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
	czenyenを元にやっつける
  2001-11-23 v1.00
  	cppの後に\処理をするのでなく、cppの前に行うモードを付加（それを基本に）。
  	引数の -D_CPP_SJ,-D_CPP_SJ=2 を見て変換モードを設定、無しなら対策をしない。
  2001-12-01 v1.01
  	・#行のファイル名変換で、全角文字が \202\253 のように変換されていたのに対処。
  	・' "の対やコメントに関わらず \ を挿入するように変更。これで、#if 0中の直接
  	　かかれたコメントを気にしなくてもよい^^; このため -D_CPP_SJ=2 はもう不要。
  2001-12-03 v1.10
	・呼び出す 元cpp を cpp0org.exe でなく 元名に "_org" を付けたファイルとする。
	・#include "～.h" の "の場合に不具合があったので 元cppへの中間ファイルを、
	  cソースと同じディレクトリにし、また、中間ファイル名のつけ方を変更。
  2001-12-05 v1.10
	ソースファイル頭の説明の修正^^;
  2003-12    v1.11
	gcc v3 対応しようとするも、外部cppが呼ばれなくなっていて意味を成さない。
  2003-13	 v1.12
	gcc v3 対策で別途 cc1_sj を作成したときの変更をフィードバック。
	・コンパイルを vc で行うことにし、シフトJIS以外のマルチバイト文字に対応
	　できる可能性を増やす。このためエラーメッセージを英語ぽくする。また、
	　コメント中の\在り文字が行末にあるとき追加する文字を全角空白から半角 _
	　に変更。
  2003-15
	実は mingw にも mbstring.h があったので、mingwでも mbstring.hを使うように.
  2003-17
	テンポラリファイル名に念のため時間を元にした文字列も付加...暫定
	ちゃんとした名前づけはどうするんだったか...スレッドIDとか?

[memo]
　手元の gcc version 2.95.3-5 で gcc -v tst.c として cpp0.exe の引数をみて、
　引数の最後の２つが必ず入力ファイルと出力ファイルのペアだろう、
　それ以外の引数はオプションだろう、と解釈して、拙作 czenyen を改造して
　やっつけた代物。
　動作は、Cソースを \ 処理をしたテンポラリファイルを作成してそれをcppに渡すか、
　先に cpp.exe を実行しテンポラリに吐き出した後￥処理をしたファイルを生成。
*/

