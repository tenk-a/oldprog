/*
	手抜きログ・カッター

	v1.00	1993/01/24	writen by M.Kitamura
    v1.01   1993/02/08  .cfg 指令'd'増設
	v1.02	1993/03/10  .cfg 出力ファイル指定 '-' 追加
 	v1.03	1993/10/24	'*'出力のﾊﾞｯﾌｧﾘﾝｸﾞを自前で行なうように変更
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jstring.h>
#include <signal.h>
#include "tree.h"

/* データ領域が far のとき(ｺﾝﾊﾟｸﾄ､ﾗｰｼﾞ､ﾋｭｰｼﾞ) */
/*#define	FBUF	*/

/* TCのみ. WILDCを定義されるとワイルド・カード展開ができる */
/*#define	WILDC	*/

/*---------------------------------------------------------------------------*/
typedef unsigned WORD;
typedef unsigned char BYTE;
typedef unsigned long DWORD;

#ifndef toascii /* ctype.h がincludeされていないとき */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif
#ifndef CT_KJ1	/* jctype.h がincludeされていないとき */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif

#ifdef WILDC
extern int dirEntryGet(BYTE far *,BYTE far *,WORD);
#endif
/*---------------------------------------------------------------------------*/

void priExit(BYTE *p)
{
	printf("%s\n",p);
	exit(1);
}

FILE *fopen_e(BYTE *Name, BYTE *mode)
{
	FILE *fp;

	fp = fopen(Name,mode);
	if (fp == NULL) {
		printf("\n %s をオープンできません\n",Name);
		exit(errno);
	}
	return fp;
}

#if 0
FILE *freopen_e(BYTE *Name, BYTE *mode,FILE *fp)
{
	fp = freopen(Name,mode,fp);
	if (fp == NULL) {
		printf("\n %s をオープンできません\n",Name);
		exit(errno);
	}
	return fp;
}
#endif

void setvbuf_e(FILE *fp,char *buf, int mode, size_t size)
{
	if (setvbuf(fp,buf,mode,size)) {
		priExit("メモリが足りない(setvbuf)\n");
	}
}

void *malloc_e(size_t siz)
{
	void *p;
	
	p = malloc(siz);
	if (p == NULL) {
		priExit("メモリが足りないよお\n");
	}
	return p;
}

/*---------------------------------------------------------------------------*/

BYTE *getDirName(BYTE dst[], BYTE *s)
{
	BYTE *q;
	BYTE *p;
	int  c;

	q = NULL;
	p = dst;
	while (*s) {
		c = *s++;
		if (c == '\\' || c == '/' || c == ':') {
			if (c == '/') {
				c = '\\';
			}
			q = p + 1;
		}
		*p++ = c;
	}
	if (q == NULL) {
		q = dst;
		*q++ = '.';
	}
	*q = '\0';
	return dst;
}

BYTE *addDirSep(BYTE dir[])
{
	BYTE *s;
	int  c,f;

	s = dir;
	f = 0;
	if (*s == '\0') {
		*s++ = '.';
		*s	 = '\0';
	}
	while (*s) {
		c = *s;
		if (c == '\\' || c == '/' || c == ':') {
			if (c == '/') {
				*s = '\\';
			}
			f = 1;
		} else {
			f = 0;
		}
		s++;
	}
	if (f == 0) {
		*s++ = '\\';
		*s = '\0';
	}
	return dir;
}

BYTE *getFnamePtr(BYTE *s)
{
	BYTE *p;
	int c;
	p = s;
	while (*s) {
		c = *s++;
		if (c == '\\' || c == '/' || c == ':') {
			p = s;
		}
	}
	return p;
}

BYTE *chgExt(BYTE filename[], BYTE *ext)
{
	BYTE *p;

	p = strrchr(filename, '/');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr(p, '\\');	/*全角ﾌｧｲﾙ名相手だとやばいが、しかたあるまい*/
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr( p, '.');
	if (p == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	} else {
		strcpy(p+1, ext);
	}
	return filename;
}

BYTE *stpSkipSpc(BYTE *p)
{
	while (*p == ' ' || *p == '\t') {
		++p;
	}
	return p;
}

BYTE *stpCpyTok(BYTE d[], BYTE *s)
{
	s = stpSkipSpc(s);
	while (*s) {
		*d = *s;
		if (*s == ' ' || *s == '\t' || *s == '\n') {
			break;
		}
		d++;
		s++;
	}
	*d = '\0';
	return s;
}

BYTE *strDelEol(BYTE str[])
{
	BYTE *s;

	s = str;
	while (*s) {
		s++;
	}
	if (str != s) {
		if (*--s == '\n') {
			*s = '\0';
		}
	}
	return str;
}

BYTE *strCpyEsc(BYTE dst[], register BYTE *s)
{
	BYTE *d;
	int mode;
	WORD c;

	d = dst;
	mode = 0;
	while (*s) {
		c = *s++;
		if (c == '"') {
			mode += 1;
			mode &= 0x01;
			continue;
		} else if (mode && c == '\\') {
			c = *s++;
			if (c == '\0') {
				break;
			} else if (c == 'n') {
				c = '\n';
			} else if (c == 't') {
				c = '\t';
			} else if (c == 'r') {
				c = '\r';
			} else if (c == 'f') {
				c = '\f';
			} else if (c == 'a') {
				c = '\a';
			} else if (c == 'b') {
				c = '\b';
			} else if (c == 'e'||c == '[') {
				c = 0x1b;
			} else if (c == 'v') {
				c = '\v';
			} else if (c == 'x') {
				char *ss;

				if (*s == '\0') {
					break;
				}
				c = (int)strtoul(s,&ss,16);
				s = ss;
				if (c > 0x100) {
					*(d++) = c / 0x100;
				}
				c &= 0xff;
			}
		}
		*d++ = c;
	}
	*d = '\0';
	return dst;
}

int linCmp(BYTE *p, BYTE *q)
{
	int a;

	a = 0;
	/*printf("%s * %s ==> ",p,q);*/
	while (*q) {
		if (*p != *q) {
			a = *p - *q;
			break;
		}
		++p;
		++q;
	}
	/*printf("%d\n",a);*/
	return a;
}

/*---------------------------------------------------------------------------*/
#ifdef FBUF
#define FBUF_SIZ (31*1024U)
#define FBUF_SIZ2 (12*1024U)
#else
#define FBUF_SIZ (10*1024U)
#define FBUF_SIZ2 (4*1024U)
#endif

#define FNAME_SIZ	128
#define LIN_SIZ 	1024
int  PriInFilFlg,PriOutFilFlg;
int  DebFlg;
BYTE ExeDir[FNAME_SIZ+2];
BYTE OutDir[FNAME_SIZ+2];
BYTE OutEtcName[FNAME_SIZ+2];
BYTE CfgName[FNAME_SIZ+2];
BYTE LinBuf[LIN_SIZ+2];
BYTE OutHdrStr[LIN_SIZ+2];

typedef struct ST {
	BYTE *Str;
	BYTE *Fname;
	int  Mode;	/* 0:指令m  1:m+  2:d */
} ST;

TREE *StTree;

ST *stObj(BYTE *Str, BYTE *Fname, int Mode)
	/* 木を検索するときのための、一時的な要素の作成 */
{
	static ST dmy;

	dmy.Str   = Str;
	dmy.Fname = Fname;
	dmy.Mode  = Mode;
	return &dmy;
}

int stCmp(ST *p1, ST *p2)
	/* 木を検索するときの要素の比較 (tree*から呼ばれる)*/
{
	return linCmp(p1->Str, p2->Str);
}

ST *stNew(ST *sp)
	/* 木に挿入するときの要素の作成(tree*から呼ばれる) */
{
	ST *p;

	p = malloc_e(sizeof(ST));
	/*memset(p,0,sizeof(ST));*/
	p->Str	 = strdup(sp->Str);
	if (sp->Fname) {
		p->Fname = strdup(sp->Fname);
	} else {
		p->Fname = NULL;
	}
	p->Mode  = sp->Mode;
	return p;
}

/*-------------------------------------*/
static BYTE *bfdBuf;
static int  bfdPos;
static int  bfdSiz;
static FILE *bfdFp;

void initBfdWrt(FILE *fp, size_t siz)
{
	bfdFp = fp;
	bfdSiz = siz;
	bfdPos = 0;
	bfdBuf = malloc_e(siz);
}

void bfdWrt(void *mem, size_t siz)
	/* siz: bfdSiz より必ず小さい値 */
{
	if ((long)bfdSiz - (long)bfdPos >= (long)siz) {
		memcpy(&(bfdBuf[bfdPos]), mem, siz);
		bfdPos += siz;
	} else {
		fwrite(bfdBuf, bfdPos, 1, bfdFp);
		memcpy(bfdBuf, mem, siz);
		bfdPos = siz;
	}
}

void termBfdWrt(void)
{
	if (bfdPos) {
		fwrite(bfdBuf, bfdPos, 1, bfdFp);
	}
	free(bfdBuf);
	bfdPos = bfdSiz = 0;
	bfdBuf = NULL;
	bfdFp = NULL;
}

/*-------------------------------------*/

BYTE *outFname(BYTE *p)
{
	static BYTE fnam[512];

	fnam[0] = 0;
	if (!(p[1] == ':' || *p == '\\' || *p == '/' || *p == '.')) {
		strcpy(fnam,OutDir);
	}
	return strcat(fnam,p);
}

void one(BYTE *fnam)
{
	FILE *ifp,*ofp,*ofp0;
	char *p;
	ST *sp;
	int flg;

	if (PriInFilFlg) {
		printf("[%s]\n",fnam);
	}
	ifp = fopen_e(fnam,"r");
	setvbuf_e(ifp,NULL,_IOFBF,FBUF_SIZ);
	fnam = OutEtcName;
	if (PriOutFilFlg) {
		printf("\t[%s]\n",fnam);
	}
	fnam = NULL;
	ofp = NULL;
	ofp0 = fopen_e(outFname(OutEtcName),"a");
	initBfdWrt(ofp0, FBUF_SIZ2);
	flg = 1;	/* 0:指定ﾌｧｲﾙに出力  1:'*' */
	for(;;) {
		p = fgets(LinBuf,LIN_SIZ,ifp);
		/*printf(LinBuf);*/
		if (p == 0) {
			break;
		}
		sp = treeSearch(StTree, stObj(LinBuf,NULL,0));
		if (sp) {
			BYTE *fnm;
			/*printf(">%s  %p\n",sp->Str,fnam);*/
			if (sp->Fname == NULL) {	/* 出力ﾌｧｲﾙが'*'のとき */
				fnm = OutEtcName;
				flg = 1;
			} else if (strcmp(sp->Fname,"?") == 0) {/*出力ﾌｧｲﾙが'-'のとき*/
				fnm = NULL;
			} else if (strcmp(fnam,sp->Fname)) {
				flg = 0;
				fnam = fnm = sp->Fname;
				if (ofp) {
					fclose(ofp);
				}
				ofp = fopen_e(outFname(sp->Fname),"a");
				setvbuf_e(ofp,NULL,_IOFBF,FBUF_SIZ2);
			} else {
				flg = 0;
				fnm = NULL;
			}
			if (PriOutFilFlg && fnm) {
				printf("\t[%s]\n",fnm);
			}
		}
		if (sp->Mode == 1 && OutHdrStr[0]) {	/* 指定m+ */
			fputs(OutHdrStr,ofp);
		} else if (sp->Mode == 2) {				/* 指令d */
			continue;
		}
		if (flg) {
			bfdWrt(LinBuf,strlen(LinBuf));
		} else {
			fputs(LinBuf,ofp);
		}
	}
	fclose(ifp);
	termBfdWrt();
	fclose(ofp0);
	if (ofp) {
		fclose(ofp);
	}
}


#if 1
void priSt(ST *sp)
{
	BYTE *fname;

	fname = sp->Fname;
	if (fname == NULL) {
		fname = "*";
	}
	printf("%-13s #%d  %s\n",fname, sp->Mode, sp->Str);
}
#endif

void cfgErr(int lin, BYTE *msg)
{
	printf("%s %d : %s\n",CfgName,lin,msg);
	exit(1);
}

void getCfg(BYTE *Name)
{
	FILE *fp;
	static BYTE buf[LIN_SIZ];
	BYTE nmbuf[FNAME_SIZ+2];
	int  c;
	int  lin;
	BYTE *p;

	StTree = treeMake((TREENEW)stNew, NULL, (TREECMP)stCmp);
	strcpy(CfgName,Name);
	chgExt(CfgName,"tlc");
	fp = fopen(CfgName,"r");
	if (fp == NULL) {
		strcpy(CfgName,ExeDir);
		strcat(CfgName,getFnamePtr(Name));
		chgExt(CfgName,"tlc");
		fp = fopen_e(CfgName,"r");
	}
	for(lin = 1;;lin++) {
		p = fgets(buf,LIN_SIZ,fp);
		if (p == NULL) {
			break;
		}
		p = stpSkipSpc(p);
		c = *p++;
		/*c = tolower(c);*/
		if (c == '#' || c == '\n') {
			continue;
		}
		if (c == 'e') {
			if (linCmp(p,"nd") == 0) {
				break;
			}
			goto ERR;
		} else if (c == '*') {
			stpCpyTok(OutEtcName,p);
		} else if (c == '+') {
			p = stpSkipSpc(p);
			strDelEol(p);
			strCpyEsc(OutHdrStr,p);
		} else if (c == 'o') {
			if (OutDir[0] == '\0') {
				stpCpyTok(OutDir,p);
				addDirSep(OutDir);
			}
		} else if (c == 'd') {
			c = 2;
			goto J1;
		} else if (c == 'm') {
			BYTE *nam;

			c = 0;
			if (*p == '+') {
				c = 1;
				++p;
			}
		J1:
			p = stpSkipSpc(p);
			p = stpCpyTok(LinBuf,p);
			p = stpSkipSpc(p);
			if (strcmp(LinBuf,"*") == 0) {
				nam = NULL;
			} else if (strcmp(LinBuf,"-") == 0) {
				nam = "?";
			} else {
				nam = strncpy(nmbuf,LinBuf,FNAME_SIZ);
			}
			strDelEol(p);
			strCpyEsc(LinBuf,p);
			if (LinBuf[0] == '\0') {
				cfgErr(lin,"d または m で、判定用文字列が指定されていない");
			}
			treeInsert(StTree, stObj(LinBuf, nam, c));
		} else {
  ERR:
			cfgErr(lin,"行頭の指定がおかしい");
		}
	}
	fclose(fp);
  #if 1
	if (DebFlg) {
		treeDoAll(StTree, (void (*)(void *))priSt);
	}
  #endif
}

/*---------------------------------------------------------------------------*/

void dispUsage(void)
{
	printf(
		"tlcut v1.03 : 手抜き(ﾀｲﾆｰ)ログ・カッター\n"
		"  定義ファイル tlcfile[.tlc] に従って指定されたログファイルを分割\n"
		"usage> tlcut [-opts] tlcfile[.tlc] logfile(s)\n"
		" tlcfile  ログ分割のための定義ファイル. 拡張子は.tlc\n"
		"          ｶﾚﾝﾄ･ﾃﾞｨﾚｸﾄﾘにないばあいは, tlcut.exe のあるﾃﾞｨﾚｸﾄﾘより探す\n"
		" logfile  ログ・ファイル\n"
		"option:\n"
		" -oDIR    出力先ディレクトリ をDIR にする. このとき tlcfile内の命令 o は無視\n"
		" -v[w]    処理中の入力ファイル名を表示. -vwだと出力先も表示\n"
		"\n"
		"tlcflle を必ず指定。logfileは１つ以上指定する。\n"
		"オプションはファイル名の前後にあってもよく、ファイル入力前に設定する。\n"
		"ファイル名はワイルド・カード指定ができ、その指定に一致するファイル名はソート\n"
		"して順に行なう\n"
	);
	exit(0);
}

void getOption(BYTE *p)
{
	int c;

	c = *p++;
	c = toupper(c);
	switch(c) {
	case '?':
	case '\0':
		dispUsage();
	case 'O':
		if (*p == '=') {
			++p;
		}
		strcpy(OutDir,p);
		addDirSep(OutDir);
		break;
	case 'V':
		PriInFilFlg = 1;
		if (*p == 'W' || *p == 'w') {
			PriOutFilFlg = 1;
		}
		break;
	case 'Z':
		DebFlg = 1;
		break;
	default:
		priExit("知らないオプションだ");
	}
}

void main(int argc, BYTE *argv[])
{
	BYTE *p;
	int  i,n;

	if (argc < 2) {
		dispUsage();
	}
	for (n = 0, i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			getOption(p+1);
		} else {
			n++;
		}
	}
	if (n < 2) {
		priExit("ファイル名が指定されていない");
	}
	getDirName(ExeDir, argv[0]);
	/*printf("%s\n",ExeDir);*/
	i = 1;
	while (i < argc) {
		p = argv[i++];
		if (*p != '-') {
			getCfg(p);
			break;
		}
	}
  #ifndef WILDC
	while ( i < argc) {
		p = argv[i++];
		if (*p == '-') {
			continue;
		}
		one(p);
	}
  #else
	while (i < argc) {
		static BYTE fnambuf[140];
		p = argv[i++];
		if (*p == '-') {
			continue;
		}
		n = 0;
		if (dirEntryGet(fnambuf,p,0x01) == 0) {
			TREE *dir;

			dir = treeMake((TREENEW)strdup, (TREEDEL)free, (TREECMP)strcmp);
			do {
				/*printf("%s\n",fnambuf);*/
				if (treeInsert(dir, fnambuf) == NULL) {
					priExit("メモリが足りません\n");
				}
				++n;
			} while (dirEntryGet(fnambuf,NULL,0x01) == 0);
			tree2dlist(dir);
			p = dlistFirst(dir);
			if (p) {
				do {
					one(p);
				} while((p = dlistNext(dir)) != NULL);
			}
			dlistClear(dir);
		}
		if (n == 0) {
			printf("%s がありません\n",p);
		}
	}
  #endif
	exit(0);
}
