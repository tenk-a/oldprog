/****************************************************************************
  cftt v2.11

    	C言語ソースプログラムの呼出関係を調べる

 1991/05/?? 伊藤誠氏作の callc を改造してたが別ものになってきたのとソースが
    	    バッチいのを整理するため作り直す。
 1991～1994 混沌. 某か手を加えてたが、いつ何したかは忘却の彼方。
 1995/06/17 大きなプログラムを解析するため djgcc で再コンパイル.
 cftt v2.00 関数名だけでなく、マクロ名、変数名とかの判別をする。
    	    複雑な出力の指定をやめ、.t01 .t02 .t03 .t04	と固定で出力。
    	    レスポンスファイルの書きかたを変更.	.cfgを読むようにする。
    	    その他もろもろ、いっぱい変更,追加＆削除。
 1996/04/01 ツリー表示でのグループ内の通し番号がずれていたのを修正.
 cftt v2.10 (グループ内の通し番号って正直なとこ不要だなあ...）
    	    すでに不要不明となったオプションの削除や変更。
    	    .t03 .t0 4出力でタグ形式でない名前のみの出力を可能に.
    	    @root 廃止。@libを@group に変名して引数追加.@comment 追加.
    	    @if0 ～ @endif を追加. などなど。
 1997/09/28 Bcc32 v5.2 でコンパイルしたら暴走したのでデバッグ(NULL対策)

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
#define	FILNUMSZ    12	    	/* 表示するファイル名のサイズ	    	*/

#ifdef _M32_
#define	GRPFNAME_MAX	(260+1)	/* @group で指定できるﾌｧｲﾙの数	    	*/
#define	LIN_SIZ	    248	    	/* 表示する1行の幅  	    	    	*/
#define	SYM_SIZ	    128	    	/* 名前の長さ	    	    	    	*/
#define	ROOT_MAX    (1024*4+1)	/* -r=<NAME> の数   	    	    	*/
#define	MIF_MAX	    40	    	/* #ifネストの深さ  	    	    	*/
#define	LINBUF_SIZ  (1024*4)	/* linBufのサイズ   	    	    	*/
#define	RSV_MAX	    (2048)  	/* 予約語の最大数   	    	    	*/
#define	SETVBUF_SIZ (0xC000)	/* ﾌｧｲﾙ書込用バッファ	    	    	*/
#else
#define	GRPFNAME_MAX	(20+1)	/* @group で指定できるﾌｧｲﾙの数	    	*/
#define	LIN_SIZ	    80	    	/* 表示する1行の幅  	    	    	*/
#define	SYM_SIZ	    64	    	/* 名前の長さ	    	    	    	*/
#define	ROOT_MAX    (52+1)  	/* -r=<NAME> の数   	    	    	*/
#define	MIF_MAX	    20	    	/* #ifネストの深さ  	    	    	*/
#define	LINBUF_SIZ  256	    	/* linBufのサイズ   	    	    	*/
#define	RSV_MAX	    (128)   	/* 予約語の最大数   	    	    	*/
#define	SETVBUF_SIZ (0x2000)	/* ﾌｧｲﾙ書込用バッファ	    	    	*/
#endif

typedef	struct REFLIST_T {  	/* 関数参照のためのリストの構造体   	*/
    struct TNODE_T  	*nodePnt;
    struct REFLIST_T	*next;
} REFLIST;

typedef	struct MULTDEF_T {
    char *filName;
    int	 defLin;
    struct MULTDEF_T *next;
} MULTDEF;

typedef	struct TNODE_T {    	/*  	-- 手続き情報ノード --	    	*/
    struct TNODE_T *link[2];	/* 0:木の左へのポインタ	    1:右    	*/
    int	    avltFlg;	    	/* avl-tree のバランス	    	    	*/
    char    *symName;	    	/* 関数名   	    	    	    	*/
    char    *filName;	    	/* 関数定義のあるファイル名へのﾎﾟｲﾝﾀ	*/
    short   atr;    	    	/* 0:関数 1:変数等 2:マクロ 	    	*/
    int	    bgnLin; 	    	/* 関数定義範囲の開始行	    	    	*/
    int	    defLin; 	    	/* 関数定義の行番号 	    	    	*/
    int	    refLin; 	    	/* tree表示での参照する番号 	    	*/
    short   refGrp; 	    	/* tree表示での参照するｸﾞﾙｰﾌﾟ番号   	*/
    struct REFLIST_T *calls;	/* この関数が呼び出す関数のﾘｽﾄへのﾎﾟｲﾝﾀ	*/
    struct REFLIST_T *caller;	/* この関数を呼び出す関数のﾘｽﾄへのﾎﾟｲﾝﾀ	*/
    struct MULTDEF_T *multDef;	/* 多重定義箇所一覧 	    	    	*/
    char    *cmnt;  	    	/* ユーザ定義コメント	    	    	*/
} TNODE;

/*---------------------------------------------------------------------------*/
int 	____dmy____;	    	/* TC4(PowerPack16)対策(T T)	    	*/
int 	filNamSiz;  	    	/* 表示するファイル名(PathList)のサイズ	*/
int 	level;	    	    	/* {}のレベル...0 が一番外をあらわす	*/
int 	funcNo;	    	    	/* main(),DispOyakoFlow()で使用(ｶｳﾝﾀ)	*/
int 	pass;	    	    	/* pass	0:登録処理 1:参照処理	    	*/
char	linBuf[LINBUF_SIZ+1];	/* 出力行生成のためのバッファ	    	*/
int 	SYM_linNo;  	    	/* 名前の見つかった行番号   	    	*/
int 	SYM_sttNo;  	    	/* 定義開始の行番号 	    	    	*/
int 	SYM_atr;    	    	/* 名前属性: 1,2:関数 3:変数? 4:マクロ	*/
char	SYM_name[SYM_SIZ+1];	/* 名前文字列のためのバッファ	    	*/
int 	Flg_symMode = 1;    	/* 0=関数 1=なんでも	    	    	*/
UCHAR	Flg_dspAllLine;	    	/* -t1ｵﾌﾟｼｮﾝのﾌﾗｸﾞ  	    	    	*/
UCHAR	Flg_dspOyakoMode;   	/* -c1ｵﾌﾟｼｮﾝのﾌﾗｸﾞ  	    	    	*/
UCHAR	Flg_dspTree = 1;    	/* Tree表示の有無   	    	    	*/
UCHAR	Flg_dspOyakoTree;   	/* 親子関係表示でtree表示した関数のみか?*/
UCHAR	Flg_dspUndef;	    	/* 未定犠関数のみの表示か   	    	*/
UCHAR	Flg_dspTrFuncOnly;  	/* treeで表示するのは関数オンリー   	*/
UCHAR	Flg_msg;    	    	/* メッセージ(抑制)フラグ   	    	*/
UCHAR	Flg_karagyo = 1;    	/* 空行出力 1:on 0:off	    	    	*/
REFLIST	*refl0;	    	    	/* ﾌｧｲﾙ中の関数の順番のﾘｽﾄ作成に使用	*/
TNODE	*TREE_root; 	    	/* MakSmTree(),MakRefLst(),main()で使用	*/
TNODE	*TREE_curNode;	    	/* */
int 	ROOT_cnt;   	    	/* tree表示でﾙｰﾄにする関数名の数    	*/
int 	ROOT_grp;   	    	/* tree表示でﾙｰﾄにする関数名のgroup番号	*/
char	*ROOT_name[ROOT_MAX];	/* tree表示でルートにする関数名の配列	*/
int 	RSV_cnt;    	    	/* RSV_name管理用   	    	    	*/
char	*RSV_name[RSV_MAX]; 	/* 予約語扱いの名前一覧	    	    	*/
char	*GRP_fnam[GRPFNAME_MAX];/* @Groupfileで指定するファイル名   	*/
char	*GRP_cmnt[GRPFNAME_MAX];/* groupごとのコメント	    	    	*/
int 	GRP_no;	    	    	/* DispTreeFlow()で使用(ｶｳﾝﾀ)	    	*/
int 	GRP_subNo;  	    	/* DispTreeFlow()で使用(ｶｳﾝﾀ)	    	*/
char	*SRC_name;  	    	/* 現在入力中のファイル名へのポインタ	*/
FILE	*SRC_fp;    	    	/* 現在入力中のファイルのストリーム 	*/
FILE	*errFp;	    	    	/* エラー出力用fp   	    	    	*/
typedef	struct SRCL_T {
    struct SRCL_T   *link;  	/* 次のリストへのポインタ   	    	*/
    char    	    *name;  	/* 入力するｿｰｽ･ﾌｧｲﾙ名へのﾎﾟｲﾝﾀの配列	*/
    REFLIST 	    *func;  	/* ﾌｧｲﾙにある関数のﾘｽﾄ	    	    	*/
} SRCL;
SRCL *SRCL_top;	    	    	/* ソースファイルリストの先頭	    	*/

#define	DEBUG
#ifdef DEBUG
UCHAR	Debug_flg;  	    	/* デバッグ用表示のフラグ   	    	*/
#endif

enum {AT_NON=0,AT_FUNC,AT_DECL,AT_VAR,AT_MAC,AT_CMNT};
char AT_str[][2] = {" ","ﾌ","ﾗ","ﾍ","ﾏ"};

/*---------------------------------------------------------------------------*/
FILE	*fopenE(char *fname, char *mod)
{
    FILE *fp;

    fp = fopen(fname,mod);
    if (fp == NULL) {
    	fprintf(errFp, "\nファイル %s をオープンできません\n", fname);
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
    	fprintf(errFp,"\nメモリが足りません\n");
    	exit(1);
    }
    return p;
}

char *strdupE(char *s)
{
    char *p;
    p =	strdup(s);
    if (p == NULL) {
    	fprintf(errFp,"\nメモリが足りません\n");
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
    *  t     : 文字列へのポインタをおさめた配列
    *  tblcnt: 登録済個数
    *  key   : 追加する文字列
    *  復帰値: 0:追加 -1:すでに登録済
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
    	    return -1;	/* 同じものがみつかったので追加しない */
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
    *  key:さがす文字列へのポインタ
    *  tbl:文字列へのポインタをおさめた配列
    *  nn:配列のサイズ
    *  復帰値:見つかった文字列の番号(0より)  みつからなかったとき-1
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


static int  uncharBuf[256]; 	/* 1文字ﾊﾞｯｸに使用 */
    	    	    	    	/* 十分に広いはずなので添字チェック無 */
static int  *unChar = uncharBuf;

static int  linNo = 1;	    	/* 現在読み込み中の行番号   	    	*/
static int  Mif_p = 0;	    	/* Mif_Set(),SYM_GetInit() で使用   	*/
static char *Mif_Name[]	= { 	/* Mif_Set(),LineTop() で使用	    	*/
    "elif", "else", "endif", "if", "ifdef", "ifndef"
};

void	RSV_Init(void)
{
    static char	*rsv[] = {  /* （後ろに'('がくる可能性のある）予約語	*/
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
    };	/* int等の型指定子があるのは char (*f)[]というような宣言があるため */
    #define RSVCNT (sizeof rsv / sizeof	rsv[0])
    int	i;

    for	(i = 0;	i < RSVCNT; i++) {
    	RSV_name[i] = rsv[i];
    }
    RSV_cnt = i;
}


static void Err_Src(char *m)
{
    fprintf(errFp, "\n%s %d : ソースにエラーがあります. %s\n", SRC_name, linNo,	m);
    exit(1);
}

static void Err_Eof(void)
{
    fprintf(errFp, "\n%s %d : おかしなところでEOFになりました\n",
    	    SRC_name, linNo);
    exit(1);
}


static void CH_Unget(int c)
    /* CH_Getで得た１文字を入力に戻す */
{
    if (c == '\n')
    	linNo--;
    *++unChar =	c;
}

static int CH_Get(void)
    /* 一文字入力
     *	 復帰値	0x01から0xff,および EOF(たいていは -1)
     * ...0x00は CH_Ungetに利用される..入力はテキストでないといけない！
     */
{
    int	    c;

    if (*unChar	== 0) {
    	c = getc(SRC_fp);
      #if 0
    	if (feof(SRC_fp))
    	    c =	EOF;
      #endif
    } else {	    	    /* CH_Ungetされた値をとりだす */
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
    /* EOFならエラー終了するCH_Get() */
{
    int	    c;

    c =	CH_Get();
    if (c < 0)	    	    	/* EOFは負値 */
    	Err_Eof();
    return c;
}


static int CH_Skip(void)
    /* 1文字入力 .. '\'エスケープ、漢字なら' '(0x20)を返す */
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
    /* CH_Get_CmtSkipで得た１文字を戻す	*/
{
    CH_Unget(c);
}

static int CH_Get_CmtSkip(void)
    /* コメントを読みとだす1文字入力 */
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
    	if (c == '*') {	/* ポインタの'*'を消す処理(乗算もだが) */
    	    c =	' ';
    	    break;
    	}
      #endif
    	if (c != '/')
    	    break;
    	c1 = CH_Get_ChkEof();
    	if (c1 == '*') {
    	    for	(;;) {	    	/* コメントを読み飛ばす	 */
    	    	c = CH_Skip();
    	    	if (c == '*') {
    	    	    c =	CH_Get_ChkEof();
    	    	    if (c == '/')
    	    	    	break;	/* コメント終了	    */
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
    /* EOFでエラー終了するCH_Get_CmtSkip() */
{
    int	    c;

    c =	CH_Get_CmtSkip();
    if (c < 0)
    	Err_Eof();
    return c;
}

static int SYM_GetStr(void)
   /* １単語入力. 名前があればSYM_nameにいれて返す なければ そのときの文字. */
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
    	} else if (c ==	'"') {	/* 文字列を読み飛ばす */
    	    do {
    	    	c = CH_Skip();
    	    } while (c != '"');
    	    c =	'0';
    	} else if (c ==	'\'') {	/* 文字定数を読み飛ばす	 */
    	    CH_Skip();
    	    if ((c = CH_Get_CmtSkipEofChk()) !=	'\'') {
    	    	Err_Src("（'定数'関係）");
    	    }
    	    c =	'0';
      #endif
    	}
    }
    return c;
}

static void Mif_Set(int	n)
    /* #ifネストの処理 */
{
    int	    i;
    static struct MIF {	    	/* #ifネスト処理のスタック */
    	int 	n;
    	int 	lvl;
    	int 	lno;
    }	mif[MIF_MAX];

  #ifdef DEBUG
    if (Debug_flg != 0 && pass == 0)
    	fprintf(errFp, "   %s %d : #%s があります(ﾚﾍﾞﾙ %d) (Mif_p=%d)\n",
    	    	SRC_name, linNo, Mif_Name[n], level, Mif_p);
  #endif
    if (0 <= n && n <= 2) { 	/* #elif,#else,#endif */
    	i = Mif_p - 1;
    	if (Mif_p == 0)
    	    Err_Src("（#ﾏｸﾛ関係）");
    	if (mif[i].lvl != level	&& Flg_msg != 0	&& pass	== 0) {
    	    fprintf(errFp,
    	    	    "   %s %d ～ %d : #%s～#%s 間で'{'と'}'の数があいません\n",
    	    	    SRC_name, mif[i].lno, linNo, Mif_Name[mif[i].n], Mif_Name[n]);
    	}
    	Mif_p--;
    }
    if (n != 2)	{   	    	/* #elif,#else,#if,#ifdef,#ifndef */
    	if (++Mif_p == MIF_MAX)	{
    	    fprintf(errFp, "   %s %d : #if 文のﾈｽﾄのﾚﾍﾞﾙが深すぎます\n",
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
    /* #マクロの処理 */
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
    	    	    fprintf(errFp, "  %s %d : #asm があります\n",
    	    	    	    	    SRC_name, linNo);
    	      #endif
    	    	if (asmflg != 0)
    	    	    Err_Src("（#asm関係）");
    	    	asmflg = 1;
    	    	do {	    	/* #endasmまでを読み飛ばす */
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
    	    	    fprintf(errFp, "  %s %d : #endasm があります\n",
    	    	    	    	    SRC_name, linNo);
    	      #endif
    	    	CH_Unget(c);
    	    	return 0x7fff;
    	  #if 1
    	    } else if (strcmp("define",	SYM_name) == 0)	{/*#defineは特殊処理*/
    	    	SYM_name[0] = '\0';
    	    	c = 0xFE;   /* デファイン定義をあらわす^^; */
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
    /* 空白を読みとばし、空白以外の文字を返す */
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
    /* fileを読み込むごとに行なう, SYM_Getを呼び出すにあたっての初期化 */
{
    SYM_sttNo =	linNo =	1;
    Mif_p = 0;
    unChar = uncharBuf;
    *unChar = 0;
    LineTop();
}


int SYM_GetTypedef(void)
    /* typedef 定義にあらわれた単語（型名）を、無視する単語一覧に登録する */
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

    	} else if (c ==	0xFE) {	/* #define のとき */
    	    SYM_GetStr();
    	    fprintf(errFp,"%s %d : typedef定義途中に#define %s宣言があるが無視.\n", SRC_name, linNo,SYM_name);
    	    do {
    	    	c = CH_Get_CmtSkipEofChk();
    	    } while (c != '\n');
    	    CH_Unget_CmtSkip(c);
    	    c =	' ';

    	} else if (c ==	'[' && level ==	0) {
    	    for	(i = 0;;) { /* [..]を読み飛ばす	*/
    	    	if (c == '[') {
    	    	    ++i;
    	    	} else if (c ==	']') {
    	    	    if ((--i) == 0)
    	    	    	break;
    	    	}
    	    	c = CH_SpcSkip();
    	    }

    	} else if (c ==	'{' && level ==	0) {
    	    for	(i = 0;;) { /* {..}を読み飛ばす	*/
    	    	if (c == '{') {
    	    	    ++i;
    	    	} else if (c ==	'}') {
    	    	    if ((--i) == 0)
    	    	    	break;
    	    	}
    	    	c = CH_SpcSkip();
    	    }
    	} else if (c ==	'\n') {	/* '\n'なので先に次の行の#ﾏｸﾛ */
    	    LineTop();	    	/* の処理をする	    	      */

    	} else if (c ==	';') {
    	    /*CH_Unget_CmtSkip(c);*/
    	    break;

    	} else {
    	    SYM_name[0]	= 0;
    	    c =	'@';
    	}
    }
    if (c < 0)	    	    /* ファイルの終わりならNULLを返す */
    	Err_Eof();
    /*printf("[typedef]end\n");*/
    return c;
}


int SYM_GetStruct(void)
    /* 構造体名を、無視する単語一覧に登録する */
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
    	if (c == 0xFE) {    	    /* #define のとき */
    	    SYM_GetStr();
    	    fprintf(errFp,"%s %d : typedef定義途中に#define %s宣言があるが無視.\n", SRC_name, linNo,SYM_name);
    	    do {
    	    	c = CH_Get_CmtSkipEofChk();
    	    } while (c != '\n');
    	    CH_Unget_CmtSkip(c);
    	    c =	' ';
    	} else {
    	    if (c == '\n') {	/* '\n'なので先に次の行の#ﾏｸﾛ */
    	    	LineTop();  	    /* の処理をする 	    	  */
    	    }
    	}
    	c = CH_Get_CmtSkipEofChk();
    }
    if (c == '{' && level == 0)	{
    	for (i = 0;;) {	/* {..}を読み飛ばす */
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
   /* 関数名を得る
    *	  関数名がみつかれば SYM_name にその文字列、SYM_linNo にみつかった
    *	  行番号をセットし、関数値として１を返す。EOFなら 0を返す。
    */
{
    int	    i, c,b;
    char    svnam[SYM_SIZ+1];	/* 名前文字列のためのバッファ	    	*/

    c =	0;
    while ((b=c,(c = SYM_GetStr()) >= 0)) {
    	/*printf("%x {%s}\n",c,SYM_name);*/
    	SYM_atr	= AT_NON;

    	if (c == 0xFE) {    /* #define のとき */
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
    	    if (pass ==	0) {	/*型定義で宣言された型は無視する単語にする*/
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
    	    	    for	(i = 0;;) { /* 引き数を読み飛ばす */
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
    	    } else {	/* 変数かなにか	*/
    	    	if (b == '(' &&	c == ')') {
    	    	    c =	CH_SpcSkip();
    	    	    if (c == '(')
    	    	    	goto KKK;
    	    	}
    	    	CH_Unget_CmtSkip(c);
    	    	c = 'A';
    	    	SYM_atr	= AT_VAR;
    	    	if (Flg_symMode	== 0) {	/* 普通, 登録時は関数以外は無視	*/
    	    	    continue;	    	/* -b 指定時は、関数以外も登録 */
    	    	}   	    	    	/* 参照処理では(..) の有無は無視 */
    	    }
    	    strcpy(SYM_name,svnam);
    	    c =	'A';
    	    break;

    	} else if (c ==	'[' && level ==	0) {
    	    for	(i = 0;;) { /* 引き数を読み飛ばす */
    	    	if (c == '[') {
    	    	    ++i;
    	    	} else if (c ==	']') {
    	    	    if ((--i) == 0)
    	    	    	break;
    	    	}
    	    	c = CH_SpcSkip();
    	    }

      #if 0
    	} else if (c ==	'"') {	/* 文字列を読み飛ばす */
    	    do {
    	    	c = CH_Skip();
    	    } while (c != '"');
    	} else if (c ==	'\'') {	/* 文字定数を読み飛ばす	 */
    	    CH_Skip();
    	    if ((c = CH_Get_CmtSkipEofChk()) !=	'\'') {
    	    	Err_Src("（'定数'関係）");
    	    }
      #endif
    	} else if (c ==	'\n') {	/* '\n'なので先に次の行の#ﾏｸﾛ */
    	    LineTop();	    	/* の処理をする	    	      */

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

    if (c < 0) {    	    	/* ファイルの終わりならNULLを返す */
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
    /* 多重定義を記録する */
{
    MULTDEF *m;
    m =	callocE(1,sizeof (MULTDEF));
    m->next    = p->multDef;
    m->filName = fname;
    m->defLin  = lno;
    p->multDef = m;
}

static int TREE_Add(TNODE *pp, int lrF)
    /* 関数名を登録 */
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
    if (b == 0)	{   /* 同じ名前が見つかった */
    	TREE_curNode = NULL;
    	if (level == 0 && pass == 0) {
    	    /* 未定義関数なら登録しなおす */
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
    	    	fprintf(errFp, "  %s %d : %sが重複定義です\n",
    	    	    SRC_name, SYM_linNo, SYM_name);
    	    	AddMultDefPos(p, SRC_name, SYM_linNo);
    	    }
    	}
    	return 0;
    }
    if (b < 0) {/* 左 */
    	l = 0;	r = 1;	nl = 1;	nr = 2;
    } else {	/* 右 */
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
    /* 木より関数をさがす */
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
    /* (定義された)関数名を木に登録 */
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
    /* 関数の参照のリストを作製	*/
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
    	    	p1 = TREE_Serch(TREE_root); /* p1：呼び出す手続き    */
    	    	p2 = NULL;
    	    } else {
    	    	p2 = TREE_Serch(TREE_root); /* p2：呼び出される手続き */
    	    }
    	}
    	if (p1 != NULL && p2 !=	NULL) {
    	    /* p1によるp2の呼び出しをp1のcallsリストに登録 */
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
    	    /* p1によるp2の呼び出しをp2のcalledリストに登録 */
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
    /* ファイル名,行番号の表示 */
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
    /* REFLIST_Tリストで同じ関数が下にあるかどうか */
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
    /*'子'関数が呼出した関数に専属するかどうか,'子'関数が'親'でもあるかどうか*/
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
    /* 指定された関数名がまだ表示されてないtree表示のルートになる関数名か？ */
{
    UCHAR   f;
    int	    n, i;
    char    *p;

    for	(n = GRP_no, i = ROOT_grp + 1; i < ROOT_cnt; i++) {
    	p = ROOT_name[i];
    	f = 0;
    	if (*p == '+') {    	/* '+'つき関数名のとき */
    	    p++;
    	    f =	1;
    	} else {    	    	/* そうでないときは グル－プ番号を更新 */
    	    n++;
    	}
    	if ((*s	== *p) && (strcmp(s, p)	== 0)) { /* みつかったとき */
    	    if (f == 0)	return n;   	/* グループ番号	をかえす */
    	    else    	return -n;  	/* '+'つき関数名なら負にする */
    	}
    }
    return 0;	    	    	/* みつからなかった */
}

#ifdef GRPN_ON	/* バグってる */
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
    /* グループ毎での名前の No.	を決定する */
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
    	    fprintf(errFp, "\n%s() がありません\n", SYM_name);
    	}
    }
}
#endif



static void PriFuNu(char *fncs,	int atr, int grp, int nn, int siz, int g,int h,char *cmnt)
    /* 関数名,tree表示での番号の表示 */
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
    	fprintf(outFp," ｸﾞ");
    	i += 3;
    }
    if (h) {
    	fprintf(outFp," ﾀ");
    	i += 3;
    }
    if (cmnt)
    	fprintf(outFp,"\t // %s",cmnt);
    fprintf(outFp,"\n");
}


static void PriFlow(REFLIST * q, char *	s)
   /* 関数名のtree表示
    * qの情報をで段付けして表示する
    * 表示済みの手続きは表示した行をrefLinに保存し以下のレベルは表示しない
    */
{
    int	    called;
    int	    n,bflg;
    TNODE   *p;

    p =	q->nodePnt;
    called = p->refGrp;
    if (p->defLin == 0)
    	called = 1;

    /* Flg_dspTrFuncOnly=1のとき、関数以外で'子供'がいないばあいは表示しない */
    bflg = (Flg_dspTrFuncOnly
    	    && (p->atr==AT_VAR||p->atr==AT_MAC)
    	    && p->calls==NULL);

    if (called == 0) {
    	if (s != linBuf	&& (n =	SchRootf(p->symName)) != 0) {
    	  #ifndef GRPN_ON   /* バグってる */
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
  #ifndef GRPN_ON   /* バグってる */
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
    	    fprintf(errFp,"関数の呼出関係が深すぎます. treeを分割してください.\n");
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
    	/* 呼び出す手続きの表示	*/
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
   /* 呼び出し関係をtree表示する */
{
    REFLIST r;

    /*if (Flg_dspTree)*/
    	fprintf(outFp,"\n呼び出し関係\n");
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
    	    fprintf(errFp, "\n%s() がありません\n", SYM_name);
    	}
    }
    /*if (Flg_dspTree)*/
    	fprintf(outFp,"\n");
}




static void PriFuNuOyako(char *	fncs, int atr, int grp,	int nn,	/*int sz,*/ int	cf,char	*cmnt)
    /* 関数名,tree表示での番号の表示 */
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
    fprintf(outFp,"%s %s %s", cf?"ｸﾞ":"  ", AT_str[atr], fncs?fncs:"");
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
   /* 各関数の親子の表示 sub */
{
    TNODE *z;
    REFLIST *q;
    static char	ch[4][8] = {
    	"      ",
    	"  実の",
    	"    ◎",
    	"◎実の",
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
    	    	fprintf(outFp,"          重複定義?\n");
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
    	    	fprintf(outFp,"        %2s親 ",
    	    	       ((p->filName == q->nodePnt->filName) ? "母" : "父"));
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
    	    	       ((p->filName == q->nodePnt->filName) ? "娘" : "子"));
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
    * 各名前の親子関係を表示
    */
{
    fprintf(outFp,"\n変数の親子関係\n\n");
    funcNo = 1;
    priFuncAtr = 0;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\t変数 %d 個\n",funcNo-1);
    fprintf(outFp,"\n宣言のみ関数の親子関係\n\n");
    funcNo = 1;
    priFuncAtr = 1;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\t宣言のみ関数 %d 個\n",funcNo-1);
    fprintf(outFp,"\n関数の親子関係\n\n");
    funcNo = 1;
    priFuncAtr = 2;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\t関数 %d 個\n",funcNo-1);
    fprintf(outFp,"\n#defineマクロの親子関係\n\n");
    funcNo = 1;
    priFuncAtr = 3;
    PriFunc(TREE_root);
    fprintf(outFp,"\t\tマクロ %d 個\n",funcNo-1);
}


static void PriNameOnly(TNODE *p)
   /* 各名前の親子の表示 sub */
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
    * 変数、関数、マクロ一覧を表示
    */
{
    fprintf(outFp,"\n; 変数一覧\n");
    funcNo = 1;
    priFuncAtr = 0;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\t変数 %d 個\n",funcNo-1);

    fprintf(outFp,"\n; 宣言のみの関数一覧\n");
    funcNo = 1;
    priFuncAtr = 1;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\t宣言のみ関数 %d 個\n",funcNo-1);

    fprintf(outFp,"\n; 関数一覧\n");
    funcNo = 1;
    priFuncAtr = 2;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\t関数 %d 個\n",funcNo-1);

    fprintf(outFp,"\n; #defineマクロ一覧\n");
    funcNo = 1;
    priFuncAtr = 3;
    if (f) PriNameOnly(TREE_root); else	PriFunc(TREE_root);
    fprintf(outFp,";\tマクロ %d 個\n",funcNo-1);
}

static void PriFuncSm(TNODE * p)
   /* 各関数の親子の表示 sub */
{
    TNODE *z;
    REFLIST *q;
    static char	ch[4][8] = {
    	"      ",
    	"  実の",
    	"    ◎",
    	"◎実の",
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
    	    fprintf(outFp,"          重複定義?\n");
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
    	    	fprintf(outFp,"        %2s親 ",
    	    	       ((p->filName == q->nodePnt->filName) ? "母" : "父"));
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
    	    	       ((p->filName == q->nodePnt->filName) ? "娘" : "子"));
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
    /* 各名前の親子の表示 sub */
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
    /* ソースファイル単位で、変数、関数、マクロ一覧を表示 */
{
    SRCL    *s;
    REFLIST *q;
    TNODE   *p;

    fprintf(outFp,"\n\n; ファイルごとの名前リスト\n\n");
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

    fprintf(outFp, "\n# ファイルごとの関数のリスト\n\n");
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
    	"usage: cftt [-opts] <file>...    //cftt v2.11 Cの名前の参照関係表示\n"
    	" -f        関数以外はツリー表示で表示しない\n"
    	" -k        見易さのための空行出力を抑止\n"
    	" -m        ひとつのレベル(範囲)で同じ関数呼出があったときすべて表示\n"
    	" -n        .t03 .t04出力で名前だけにする(タグファイル形式にしない)\n"
    	" -v<N>     ﾌｧｲﾙ名の桁数(N:0-80)\n"
    	" -r<func>  tree表示のルートの関数名を指定(省略時:main)\n"
    	" -o<path>  結果を<path>に出力\n"
    	" -e<path>  経過メッセージを<path>に出力\n"
    	/*" -y	      経過メッセージの表示を抑制\n"*/
    	" @<path>   ﾌｧｲﾙ名,関数名を<path>より入力(@のみ標準入力)\n"
    	" +<path>   ﾌｧｲﾙ名,関数名を<path>より入力(+のみ標準入力)\n"
     #if 0  /* もはや使えない古いオプション? */
    	" -a        未定義関数も登録\n"
    	" -a1       親子表示で未定犠関数のみ表示\n"
    	" -b        関数名以外は登録しない\n"
    	" -c[N]     親子関係表示で N=1:子供を、N=2:親を、N=3:親と子を、表示しない\n"
     #endif
    	);
  #ifdef DEBUG
    /*fprintf(errFp," -d    	ﾃﾞﾊﾞｯｸﾞ･ﾓｰﾄﾞ\n");*/
  #endif
    exit(0);
}


/*---------------------------------------------------------------------------*/
static int  Opt_undef;	    	    /* 未定義関数を登録するかどうか 	*/
static int  Opt_dspNameLstFlg =	0;  /* タグファイル化せずに表示	    	*/
static char outFname[260];  	    /* 出力するパス名をしめす	    	*/
static char errFname[260];  	    /* error出力するパス名を示す    	*/
static char cftFname[260];  	    /* cftファイル名	    	    	*/
#ifdef OPTDIV
static int  Opt_div;	    	    /* .T05出力	    	    	    	*/
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
    	    Flg_dspOyakoMode = 1;   /* '子(娘)'を表示しない */
    	else if	(*p == '2')
    	    Flg_dspOyakoMode = 2;   /* 親を表示しない */
    	else if	(*p == '3')
    	    Flg_dspOyakoMode = 3;   /* 自分のみ表示 */
    	break;
  #ifdef DEBUG
    case 'D':	    	/* デバッグ・モード */
    	Debug_flg = 1;
    	break;
  #endif
    case 'E':	    	/* error出力するパスを指定    */
    	if (*p == '\0')
    	    goto OPT_ERR;
    	strcpy(errFname,p);
    	break;

    case 'F':
    	Flg_dspTrFuncOnly = 1;	/* 関数以外はTreeで表示しない */
    	break;

    case 'M':
    	Flg_dspAllLine = 1; 	/* 現われた呼出し関数すべてを表示*/
    	break;

    case 'N':
    	Opt_dspNameLstFlg = 1;	/* .t03	.t04 で名前だけ出力 */
    	break;

    case 'K':
    	Flg_karagyo = 0;
    	break;

    case 'O':	    	/* 出力するパスを指定	 */
    	if (*p == '\0')
    	    goto OPT_ERR;
    	strcpy(outFname,p);
    	break;

    case 'R':	    	/* tree表示でのﾙｰﾄになる関数の指定 */
    	Flg_dspOyakoTree = 1;
    	if (*p != '\0' && ROOT_cnt < ROOT_MAX)
    	    ROOT_name[ROOT_cnt++] = p;
    	break;

    case 'V':	    	/* ファイル名の幅を指定	   */
    	filNamSiz = atoi(p);
    	if (filNamSiz <	0 || filNamSiz > 80)
    	    goto OPT_ERR;
    	break;

    case 'Y':	    	/* メッセージを抑制 */
    	Flg_msg	= 0;
    	break;

    case 'T':	/* 昔のバッチファイルのために残っているオプション^^; */
    	if (*p == '\0')	{
    	    ;
    	} else if (*p == '1') {
    	    Flg_dspAllLine = 1;	    /* 現われた呼出し関数すべてを表示*/
    	} else if (*p == '2') {
    	    Flg_dspTrFuncOnly =	1;  /* 関数以外はTreeで表示しない */
    	} else if (*p == '3') {
    	    Opt_dspNameLstFlg =	1;
    	}
    	break;

  #if 1	/* もはや意味をなさないオプション */
    case 'A':	    	/* 未定義関数も登録 */
    	Opt_undef = 1;
    	if (*p == '1') {
    	    Flg_dspUndef = 1;
    	}
    	break;
    case 'B':	    	/* 関数以外も登録(実験)	*/
    	Flg_symMode = 0;
    	break;
  #endif
  #ifdef OPTDIV
    case 'G':
    	Opt_div	= 1;
    	break;
  #endif

    case '\0':
    case '?':	    	/* ヘルプ */
    	Usage();

    default:
  OPT_ERR:
    	fprintf(errFp, "指定されたオプションがおかしい(%s)\n",s);
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
    /* @fileでの、指定パスよりファイル名を入力 */
{
    FILE   *fp;	    	    /* -zｵﾌﾟｼｮﾝで入力するﾌｧｲﾙのｽﾄﾘｰﾑ   */
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
    	    	    fprintf(errFp,"%s %d : @if0のネストはできません\n",	cft_name, l);
    	    	}
    	    	if0Flg = 1;
    	    	goto NEXT_LINE;

    	    } else if (strcmp(s, "@endif") == 0) {
    	    	if (if0Flg == 0) {
    	    	    fprintf(errFp,"%s %d : @if0なく@endifがあらわれた\n", cft_name, l);
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
    	    	    fprintf(errFp,"%s %d : .cft中の@groupfileの指定がおかしい\n", cft_name, l);
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

    	    if (f == 0)	{   	    /* ファイル名を登録	*/
    	    	SRCL_Add(strdupE(s));
    	    } else if (f == 1) {    /* 無視する単語を指定する */
    	    	if (RSV_cnt < RSV_MAX) {
    	    	    char *p;
    	    	    p =	strdupE(s);
    	    	    STBL_Add(RSV_name, &RSV_cnt, p);
    	    	}
    	    } else if (f == 2) {    /* ルートになる名前を登録 */
    	    	if (ROOT_cnt < ROOT_MAX) {
    	    	    ROOT_name[ROOT_cnt++] = strdupE(s);
    	    	}
    	    } else if (f == 3) {    /* ライブラリな名前を登録 */
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
    	    	    fprintf(errFp,"%s %d : .cft中の@comment部の指定がおかしい\n", cft_name, l);
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

    /* オプション／ファイル名読み取り */
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
    	    SRCL_Add(p);    /* ファイル名を保存	*/
    	}
    }

  #if 0	/*def DEBUG*/
    if (Debug_flg) {
    	for (i = 0; i <	RSV_cnt; i++)
    	    fprintf(errFp,"\t$ %s\n",RSV_name[i]);
    }
  #endif

    /* チェック	*/
    if (SRCL_top == NULL) {
    	fprintf(errFp, "入力ファイル名が指定されていない\n");
    	exit(1);
    }
    if (ROOT_cnt == 0) {
    	ROOT_name[ROOT_cnt++] =	strdupE("main");
    } else if (ROOT_cnt	>= ROOT_MAX) {
    	fprintf(errFp,"関数の数が多すぎます\n");
    	exit(1);
    }

    /* 出力先指定 */
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

    /* ファイルの長さを求める */
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	if (filNamSiz <	strlen(s->name))
    	    filNamSiz =	strlen(s->name);
    }

    /* 名前登録処理 */
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	SRC_name = s->name;
    	if (Flg_msg)
    	    fprintf(errFp, "--- %s の登録処理を開始します\n", SRC_name);
    	s->func	= MakSmTree((int)Opt_undef);
    }
    fprintf(errFp,"\n");

    /* 名前参照処理 */
    for	(s = SRCL_top; s != NULL; s = s->link) {
    	SRC_name = s->name;
    	if (Flg_msg)
    	    fprintf(errFp, "+++ %s の参照処理を開始します\n", SRC_name);
    	MakRefLst();
    }

  #ifdef GRPN_ON    /* バグってる */
    /* グループ毎の番号をつける	*/
    CountGrpSubNo();
  #endif

    /* 呼出関係をtree表示する */
    outFp = stdout;
    i =	outFname[0];
    if (i)  outFp = fopenE(FIL_ChgExt(outFname,	"T01"),	"w");
    DispTreeFlow();
    if (i)  fclose(outFp);

    /* 各関数の親子関係を表示 */
    if (i)  outFp = fopenE(FIL_ChgExt(outFname,	"T02"),	"w");
    Flg_dspOyakoMode = 0;
    DispOyakoFlow();
    if (i)  fclose(outFp);

    /* 変数、関数、マクロごとの名前一覧を表示 */
    if (i) {
    	outFp =	fopenE(FIL_ChgExt(outFname, "T03"), "w");
    	Flg_dspOyakoMode = 3;
    	DispNameList(Opt_dspNameLstFlg);
    	fclose(outFp);
    }


    /* ソースファイル毎の名前一覧を表示	*/
    if (i) {
    	outFp =	fopenE(FIL_ChgExt(outFname, "T04"), "w");
    	DispFileSmList(Opt_dspNameLstFlg);
    	fclose(outFp);
    }

  #ifdef OPTDIV
    /* ソースごとの関数リスト */
    if (Opt_div) {
    	if (i)	outFp =	fopenE(FIL_ChgExt(outFname, "T05"), "w");
    	DispFileSm();
    	if (i)	fclose(outFp);
    }
  #endif

    /* 終了 */
    return 0;
}

