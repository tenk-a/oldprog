/*
    PICld (pic2tif改名)
    	    	4,8,12,15,16,24ﾋﾞｯﾄ色pic-> tiff,bmp,rgb,q0 コンバータ

    	    	12,15,16,24ﾋﾞｯﾄ色 -> 24ﾋﾞｯﾄ色TIF,24ﾋﾞｯﾄ色BMP,RGB,Q0
    	    	256色pic -> 256色tif,256色bmp,RGB,Q0
    	    	16色pic  -> 16色tif,16色bmp,RGB,Q0

    	コンパイルは far dataモデル(ｺﾝﾊﾟｸﾄ､ﾗｰｼﾞ､ﾋｭｰｼﾞ)で行なうこと
*/

/*---------------------------------------------------------------------------*/
/*  	    	    共　通　	    	    	    	    	    	     */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#if 1
    #include <signal.h>
    #define KEYBRK  /* CTRL-C で abort 可にする...  	    	    	    */
    	    	    /* TCでｺﾝﾊﾟｲﾙしてみた範囲では意味なかった(T^T)  	    */
#endif
/*  #define SML */  /* SML が設定されると、連鎖を記録するバッファの使用量を */
    	    	    /* 半分にできる。が、展開速度が一層遅くなる。   	    */
    #define VA256PIC/* VA256色PIC関係のルーチン. VA256色PICなんか大ｯｯ嫌い   */
    #define DITHER  /* 減色(ディザ)関係...これは必ず定義(未定儀にできない)  */
    	    	    /* ｴﾃﾞｨﾀでﾃﾞｨｻﾞ関係のﾙｰﾁﾝを探すためのｷｰﾜｰﾄﾞ     	    */
    #define ASPX68K /* X68Kなｱｽﾍﾟｸﾄ比のとき横幅調整する場合のﾙｰﾁﾝを探すため */
    	    	    /* のｷｰﾜｰﾄﾞ. かならず定義(未定儀にできない)     	    */
    	    	    /* 横幅調整は、画面表示向けな機能	    	    	    */

#ifdef TCC /*__TURBOC__*//* TC(++),BC++用かな... MSC,QCは? */
    #define PC98    /* PC9801のVRAMに表示するオプションを付加. デバッグ用   */
    #define DIRENTRY/* ﾜｲﾙﾄﾞ･ｶｰﾄﾞ対応のﾌｧｲﾙ名取得ﾙｰﾁﾝを使う.	    	    */
    #define FDATEGET/* ファイルの日付を保存するオプションを付加     	    */
#else /*#endif*/
    #define far
    #define huge
#endif

#ifdef __GO32__
    #define cdecl
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#ifndef toascii /* ctype.h がincludeされていないとき */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif
#ifndef CT_KJ1	/* jctype.h がincludeされていないとき */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif

#define FNAMESIZE   128
#define WRTBUFSIZE  0x6000

typedef DWORD PIXEL;	/* 1ピクセルを収める型 */
    	    	    	/* (MSB) aaaaaaaaBBBBBBBBGGGGGGGGRRRRRRRR (LSB) */
    	    	    	/* 普通 aaaaaaaa は 00000000	    	    	*/
    	    	    	/* 256(16)色のときはaaaaaaaa(bit31-24)の位置に	*/
    	    	    	/* 元のﾋﾟｸｾﾙの値を入れます  	    	    	*/
    	    	    	/* （構造体にしたほうがいいのかもしれないけど	*/
    	    	    	/*   整数で受け渡しするほうが楽なので^^;）  	*/


/*- 日付保持のために使用 ----------------------------------------------------*/
#ifdef FDATEGET /*MS-C/QC系の手続き名^^;*/
#ifdef TCC
#include <dos.h>
#if __TURBOC__ <= 0x0300    /* BCや MSなC にはある関数 */

unsigned _dos_setftime(int hno, unsigned dat, unsigned tim)
{
    union REGS reg;

    reg.x.ax = 0x5701;
    reg.x.bx = hno;
    reg.x.cx = tim;
    reg.x.dx = dat;
    intdos(&reg, &reg);
    if (reg.x.flags & 1) {/* reg.x.flags はTCに依存 */
    	return (unsigned)(errno = reg.x.ax);
    }
  /*printf("\nhno=%x fdate=%4x ftime=%4x\n",hno,dat,tim);*/
    return 0;
}

unsigned _dos_getftime(int hno, unsigned *dat, unsigned *tim)
{
    union REGS reg;

    reg.x.ax = 0x5700;
    reg.x.bx = hno;
    intdos(&reg, &reg);
    if (reg.x.flags & 1) {  /* reg.x.flags はTCに依存 */
    	return (unsigned)(errno = reg.x.ax);
    }
    *tim = reg.x.cx;
    *dat = reg.x.dx;
  /*printf("\nhno=%x fdate=%4x ftime=%4x\n",hno,*dat,*tim);*/
    return 0;
}
#endif
#endif
#endif


/*---------------------------------------------------------------------------*/

#ifdef X68K
    /* なじぇXCのmallocってMAX 64Kbytesなのさ...ms-dosより質悪う(T T) */
    #define far
    #define huge
    #define cdecl
    #define malloc(sz)	Malloc(sz)
    #define calloc(a,b) Calloc(a,b)
    #define free(p) 	Free(p)
    extern void *MALLOC(size_t sz);
    extern void MFREE(void *p);

static void *Malloc(size_t sz)
{
    void *p;

    p = MALLOC(sz);
    if ((unsigned long)p >= 0x81000000U)
    	p = NULL;
    return p;
}

static void *Calloc(size_t sz, size_t n)
{
    void *p;

    p = Malloc(sz * n);
    memset(p, 0, sz * n);
    return p;
}


static void Free(void *p)
{

    MFREE(p);
}
#endif


/*---------------------------------------------------------------------------*/

char *FIL_BaseName(char *adr)
{
    char *p;

    p = adr;
    while (*p != '\0') {
    	if (*p == ':' || *p == '/' || *p == '\\') {
    	    adr = p + 1;
    	}
    	if (iskanji((*(unsigned char *)p)) && *(p+1) ) {
    	    p++;
    	}
    	p++;
    }
    return adr;
}

char *FIL_ChgExt(char filename[], char *ext)
{
    char *p;

    if (filename[0] == '.') {
    	filename ++;
    }
    if (filename[0] == '.') {
    	filename ++;
    }
    p = strrchr(filename, '/');
    if ( p == NULL) {
    	p = filename;
    }
    p = strrchr(p, '\\');
    if ( p == NULL) {
    	p = filename;
    }
    p = strrchr( p, '.');
    if (p == NULL) {
    	strcat(filename,".");
    	strcat( filename, ext);
    } else {
    	strcpy(p+1, ext);
    }
    return filename;
}

char *FIL_AddExt(char filename[], char *ext)
{
    char *p;

    if (filename[0] == '.') {
    	filename ++;
    }
    if (filename[0] == '.') {
    	filename ++;
    }
    p = strrchr(filename, '/');
    if ( p == NULL) {
    	p = filename;
    }
    p = strrchr(p, '\\');
    if ( p == NULL) {
    	p = filename;
    }
    if ( strrchr( p, '.') == NULL) {
    	strcat(filename,".");
    	strcat(filename, ext);
    }
    return filename;
}

void *calloc_m(size_t nobj, size_t sz)
{
    void *p;
    p = calloc(nobj,sz);
    if(p == NULL) {
    	printf("必要なメモリ(%d*%d)を確保できないよお(T^T)\n",nobj,sz);
    }
    return p;
}

FILE *TmpCreate(char *name, char **tmpname)
    /*name にﾃﾞｨﾚｸﾄﾘ名付ﾌｧｲﾙ名を入れて呼び出すと、そのﾃﾞｨﾚｸﾄﾘにﾃﾝﾎﾟﾗﾘな     */
    /*名前でﾌｧｲﾙを書き込みopenし、成功すればそのﾌｧｲﾙﾎﾟｲﾝﾀを返す。   	    */
    /*失敗すれば名前を少し変名して再度openをためし、数回試しみて駄目ならNULL*/
    /*を返す. 復帰時にはtmpnameに実際に生成したﾌｧｲﾙ名を入れて返す.  */
    /*tmpnameはmallocで確保される*/
{
    char *p;
    FILE *fp = NULL;
    int n;

    *tmpname = calloc_m(strlen(name)+13,1);
    	if (*tmpname == NULL) {
    	    return NULL;
    	}
    	strcpy(*tmpname, name);
    p = FIL_BaseName(*tmpname);
    for (n = 0; n < 10; n++) {
    	sprintf(p,"p2t$$%03d.bak",n);
    	fp = fopen(*tmpname,"wb");
    	if (fp) {
    	    break;
    	}
    }
    return fp;
}

void Rename_b(char *oldname, char *newname)
    /* ﾌｧｲﾙ名をrenameする。newnameがすでに存在したときはそれを".bak"しておく*/
{
    /*if (oldname && newname) {*/
    	char nbuf[FNAMESIZE+20];
    	 /*出力名と同じ名前があればrename*/
    	 strcpy(nbuf,newname);
    	 FIL_ChgExt(nbuf,"bak");
    	 remove(nbuf);
    	 rename(newname,nbuf);
    	 /*仮名をちゃんとした名前に変名*/
    	 rename(oldname, newname);
    /*}*/
}

FILE *fopen_m(char *name, char *mode)
{
    FILE *fp;

    fp = fopen(name,mode);
    if (fp == NULL) {
    	printf("%s をオープンできないよお(T^T)\n",name);
    }
    return fp;
}

size_t fwrite_e(const void *p, size_t siz, size_t n, FILE *fp)
{
    size_t nn;
    nn = fwrite(p,siz,n,fp);
    if (nn < n) {
    	printf("ファイル書き込みでエラー\n");
    	exit(1);
    }
    return nn;
}

/*---------------------------------------------------------------------------*/
/*  	    	    	    P	I   C	入  力	    	    	    	     */
/*---------------------------------------------------------------------------*/
/*module PIC*/
/*  export PIC, PIC_FUNC_PUTLINE, PIC_Open, PIC_CloseR, PIC_PutLines	*/
/*  	    ,PIC_rdDebFlg, PIC_DIT_CNT;     	    	    	    	*/

#define PIC_RDBUFSIZ	0x6000	    /* PIC入力バッファSIZE(展開有り用) */
#define PIC_HDRRDBUFSIZ 0x800	    /* PIC入力バッファSIZE(展開無し用) */
#define PIC_COMMENTSIZ	0x1000	    /* PICコメント・バッファ・サイズ */

typedef void (*PIC_FUNC_PUTLINE)(void *C, PIXEL *buf);
    	    	/* PIC_PutLinesにわたす１行出力を行う関数(へのポインタ)の型 */

int PIC_rdDebFlg = 0;	/* !0 ときデバッグ   1=counter	2=msg*/

/* PIC-TYPE */
#define PIC_X68K    0
#define PIC_88VA    1
#define PIC_TOWNS   2
#define PIC_MAC     3
#define PIC_EX	    15


/*-- 128個分の色キャッシュ関係 --*/
typedef struct PIC_C7TBL_T {
    PIXEL col;
    int  lft;
    int  rig;
} PIC_C7TBL;

typedef struct PIC_T {
    FILE *fp;	    	/* ファイル・ポインタ	    	    	    	    */
    char *name;     	/* ファイル名	    	    	    	    	    */
    char *tmpname;  	/* 仮ファイル名 ... 使うこと無し^^; 	    	    */
    int  colbit;    	/* 色ビット数	    	    	    	    	    */
    int  xsize,ysize;	/* サイズ　横幅,縦幅	    	    	    	    */
    int  xstart,ystart; /* 基点座標 	    	    	    	    	    */
    int  asp1, asp2;	/* アスペクト比. 横,縦	    	    	    	    */
    int  macNo;     	/* 機種番号: 0=X68K 1=88VA 2=TOWNS 3=MAC 15=拡張ﾍｯﾀﾞ*/
    	    	    	/*  	     (廃案 内部処理用:$101=MSX(/MMﾍｯﾀﾞ))    */
    int  macMode;   	/* 機種依存モード   	    	    	    	    */
    WORD typ;	    	/* macMode*0x10 + macNo     	    	    	    */
    char *comment;  	/* コメント 	    	    	    	    	    */
    char *comment2; 	/* /MM形式の拡張ヘッダのときの、実際のコメント	    */
    char artist[20];	/* 作者名 18バイト  	    	    	    	    */
    int  palbit;    	/* パレット１つのビット数   	    	    	    */
    BYTE *rgb;	    	/* rgbパレット	    	    	    	    	    */

/* private: */
    /* 入力関係 */
    size_t rdpos;   	/* 入力バッファでの現在の位置	    	    	     */
    size_t rdlen;   	/* 読込まれたデータのバイト数	    	    	     */
    size_t rdbufsiz;	/* 入力バッファのサイズ     	    	    	     */
    BYTE rdmsk;     	/* ビット入力でのマスク     	    	    	     */
    BYTE rddat;     	/* ビット入力での１バイト   	    	    	     */
    BYTE *rdbuf;    	/* 入力バッファへのポインタ 	    	    	     */

    /* 変換関係 */
    int sizeXo,sizeYo;	/* 画像サイズ	    	    	    	    	     */
    int asp1o,asp2o;	/* 出力側のアスペクト比     	    	    	     */
    BYTE **cmap;    	/* 連鎖を記録するためのバッファ     	    	     */
    PIXEL *plin[2]; 	/* ピクセル・データ２行分を収める   	    	     */
    int c7p;	    	/* 色キャッシュの最新の位置 	    	    	     */
    PIC_C7TBL *c7t; 	/* 色キャッシュ・テーブル   	    	    	     */
    PIXEL (*funcGetCol)(struct PIC_T *);    /* 色読込みﾙｰﾁﾝへのポインタ      */

  #ifdef VA256PIC
    PIXEL *va256buf;
  #endif

  #ifdef DITHER
    /* 減色(ディザ) */
    int ditherMode; 	/* 減色  0:行わない  3:８色  4:16色  8:256色	     */
    void (*ditherFunc)(struct PIC_T *pic, PIXEL *buf);
    	    	    	/* 実際に減色を行うﾙｰﾁﾝへのﾎﾟｲﾝﾀ    	    	     */
    BYTE *ditRGB;   	/* 減色でのパレット 	    	    	    	     */
    int  ditY;	    	/* 現在の行番号     	    	    	    	     */
  #endif

    /* X68Kな比率なときの横幅調整関係 */
  #ifdef ASPX68K
    int  aspX68k;   	/* x68k なアスペクト比のときon(1)   	    	     */
    PIXEL *aspX68kBuf;
  #endif

} PIC;


static int  PIC_ReadBuf(PIC *pic)
    /* バッファに読込む */
{
    pic->rdlen = fread(pic->rdbuf, 1, pic->rdbufsiz, pic->fp);
    if (ferror(pic->fp)) {
    	printf("読み込みでエラー発生？\n");
    }
    pic->rdpos = 0;
    return 0;
}

static int  PIC_RdByte(PIC *pic)
{
    int c;

    if (pic->rdpos >= pic->rdlen) {
    	PIC_ReadBuf(pic);
    	if (pic->rdlen == 0) {
    	  #if 1
    	    static int flg = 0;
    	    printf("ファイルサイズ以上に読込もうとした...？！\n");
    	    *(DWORD *)(pic->rdbuf) = 0;
    	    pic->rdlen = 4;
    	    if (flg) {
    	    	exit (1);
    	    }
    	    flg = 1;
    	  #else
    	    printf("ファイルサイズ以上に読込もうとした...？！\n");
    	    exit (1);
    	  #endif
    	}
    }
    c = pic->rdbuf[pic->rdpos++];
    return c;
}

static WORD PIC_RdWord(PIC *pic)
{
    WORD h;
    h = PIC_RdByte(pic);
    return h * 0x100 + PIC_RdByte(pic);
}

static int PIC_RdBit(PIC *pic)
{
    int f;

    if (pic->rdmsk == 0x00) {
    	pic->rddat = PIC_RdByte(pic);
    	pic->rdmsk = 0x80;
    }
    f = (pic->rddat & pic->rdmsk) ? 1 : 0;
    pic->rdmsk >>= 1;
    return f;
}

static DWORD PIC_RdBits(PIC *pic, int n)
    /*	n = 1..32 */
{
    DWORD r;

    r = 0;
    while (--n >= 0) {
    	if (pic->rdmsk == 0x00) {
    	    pic->rddat = PIC_RdByte(pic);
    	    pic->rdmsk = 0x80;
    	}
    	r <<= 1;
    	if (pic->rddat & pic->rdmsk){
    	    r |= 0x01;
    	}
    	pic->rdmsk >>= 1;
    }
    return r;
}

static long PIC_RdLen(PIC *pic)
{
    int n;

    n = 1;
    while (PIC_RdBit(pic)) {
    	n++;
    }
    return PIC_RdBits(pic, n) + (1L << n) - 1;
}

static void PIC_Free(PIC *pic)
    /* PIC読込みで確保したメモリを解放*/
{
    if (pic->fp) {
    	fclose(pic->fp);
    }
    if (pic->name) {
    	free(pic->name);
    }
  #if 0
    if (pic->tmpname) {
    	free(pic->tmpname);
    }
  #endif
    if (pic->rgb) {
    	free(pic->rgb);
    }
    if (pic->rdbuf) {
    	free(pic->rdbuf);
    }
    if (pic->comment) {
    	free(pic->comment);
    }
    if (pic->plin[0]) {
    	free(pic->plin[0]);
    }
    if (pic->plin[1]) {
    	free(pic->plin[1]);
    }
    if (pic->cmap) {
    	int y;
    	for (y = pic->sizeYo;--y >= 0;) {
    	    if (pic->cmap[y]) {
    	    	free(pic->cmap[y]);
    	    }
    	}
    	free(pic->cmap);
    }
    if (pic->c7t) {
    	free(pic->c7t);
    }
  #ifdef DITHER
    if (pic->ditRGB) {
    	free(pic->ditRGB);
    }
  #endif
  #ifdef ASPX68K
    if (pic->aspX68kBuf) {
    	free(pic->aspX68kBuf);
    }
  #endif
  #ifdef VA256PIC
    if (pic->va256buf) {
    	free(pic->va256buf);
    }
  #endif
    free(pic);
}

void PIC_CloseR(PIC *pic)
{
    PIC_Free(pic);
}

static void PIC_GetMM(PIC *pic)
    /* コメントより/MMヘッダ情報を読み込む */
    /* X68K系のオプションのみサポート. MSX系のはほとんど無視.	*/
    /* （/XY,/XSN,/XSS,/XFH,/XFL,/MYのみ）  	    	    	*/
{
    char *s ,*st;
    int mk,my,mhy,xs,xf,ms;

    if (PIC_rdDebFlg == 2) {
    	printf("/MM形式ヘッダ\n");
    }
    mk = my = mhy = xs = xf = 0;
    ms = 8;
    st = s = strdup(pic->comment/*+4*/);
    if (s == NULL) {
    	printf("メモリが足りないよン\n");
    	return;
    }
    s += 4;
    for (s = strtok(s, "/"); s && *s && *s != ':'; s = strtok(NULL,"/")) {
    	if (PIC_rdDebFlg == 2) {
    	    printf("/%s\n",s);
    	}
    	if (strncmp(s,"XY",2) == 0) {
    	    s += 2;
    	    pic->xstart = (s[0]-'0')*1000 + (s[1]-'0')*100
    	    	    	+(s[2]-'0')*10 + (s[3]-'0');
    	    s += 4;
    	    pic->ystart = (s[0]-'0')*1000 + (s[1]-'0')*100
    	    	    	+(s[2]-'0')*10 + (s[3]-'0');
    	} else if (strncmp(s,"AU",2) == 0) {/* 作者名 */
    	    int i;
    	    s += 2;
    	    for (i = 0; i < 18; i++) {
    	    	if (s[i] == 0 || s[i] == '/' || s[i] == '\x1a')
    	    	    break;
    	    	pic->artist[i] = s[i];
    	    }
    	    /* pic->artist[i] = '\0';	*/  /* callocしてるから不要^^; */
    	} else if (strcmp(s,"XSN") == 0) {/* X68K:512*512 mode */
    	    xs = 1;
    	} else if (strcmp(s,"XSS") == 0) {/* X68K:768*512 mode */
    	    xs = 2;
    	} else if (strcmp(s,"XFH") == 0) {/* X68K:31KHz mode */
    	    xf = 1;
    	} else if (strcmp(s,"XFL") == 0) {/* X68K:15KHz mode */
    	    xf = 2;
    	} else if (strcmp(s,"MK") == 0) {/*縦横２倍...無視^^;*/
    	    mk = 1;
    	} else if (strcmp(s,"MY") == 0) {/*縦２倍...縦比率を倍*/
    	    my = 1;
    	} else if (strcmp(s,"MHY") == 0) {/*X68Kとき縦を512/424倍...無視*/
    	    mhy = 1;
    	} else if (strncmp(s,"MS",2) == 0) {/*msxの画面モード...無視*/
    	    ms = (int)strtol(s+2,NULL,16);
    	}
    }
    if (s && *s) {
    	if (*s == ':')
    	    s++;
    	pic->comment2 = pic->comment + (s - st);
    }
    free(st);
    if (xs == 1) {
    	pic->asp1 = 13;
    	pic->asp2 = 9;
    	if (xf == 2) {
    	    pic->asp1 = 32;/* 5 */
    	    pic->asp2 = 25;/* 4 */
    	}
    } else if (xs == 2) {
    	pic->asp1 = 1;
    	pic->asp2 = 1;
    }
    if (my) {
    	pic->asp2 <<= 1;
    }
    if (PIC_rdDebFlg == 2) {
    	printf("  xs=%d xf=%d my=%d mk=%d mhy=%d ms=%d\n",
    	    xs,xf,my,mk,mhy,ms);
    }
}


BYTE *PIC_SetDiRGB256(void)
    /* 256色固定ﾊﾟﾚｯﾄを生成...TOWNSのﾃﾞﾌｫﾙﾄ256色,VAの256色, と同じ */
{
    int r,g,b;
    BYTE *p,*rgb;

    rgb = p = calloc_m(256, 3);
    if (rgb == NULL) {
    	exit(1);
    }
    for (g = 0; g < 256; g += 32) {
    	for (r = 0; r < 256; r += 32) {
    	    for (b = 0; b < 256; b += 64) {
    	    	*p++ = (r)? r|0x1f : 0;
    	    	*p++ = (g)? g|0x1f : 0;
    	    	*p++ = (b)? b|0x2f : 0;
    	    }
    	}
    }
    return rgb;
}


PIC *PIC_Open(char *name, int flg)
    /* char *name   	picファイル名	    	    	    */
    /* int  flg     	0:展開行う  	1:ヘッダ読込みのみ  */
{
    char *p;
    int  c,i;
    PIC *pic;

    /* */
    pic = calloc_m(1, sizeof(PIC));
    if (pic == NULL) {
    	return NULL;
    }

    /* PICファイルopen */
    pic->name = strdup(name);
    pic->fp = fopen_m(pic->name, "rb");
    if (pic->fp == NULL) {
    	goto ERR;
    }

    /* PIC読込みバッファの初期化 */
    pic->rdbufsiz = (flg == 0) ? PIC_RDBUFSIZ : PIC_HDRRDBUFSIZ;
    pic->rdbuf = calloc(pic->rdbufsiz,1);
    if (pic->rdbuf == NULL) {
    	goto ERR;
    }
    /*PIC_ReadBuf(pic);*/

    /* ID 読込み&確認 */
    if (PIC_RdByte(pic)!= 'P'|| PIC_RdByte(pic)!= 'I'|| PIC_RdByte(pic)!= 'C'){
    	printf("ファイルの先頭(ID)が'PIC'でない\n");
    	goto ERR;
    }

    /* コメント読込み */
    pic->comment = p = malloc(PIC_COMMENTSIZ);
    if (p == NULL) {
    	goto ERR;
    }
    i = 0;
    for (;;) {
    	c = PIC_RdByte(pic);
    	if (c == 0x1a) { /* EOF ならコメント終了 */
    	    break;
    	} else if (c < 0) {
    	    goto ERR;
    	} else if (c && i < PIC_COMMENTSIZ-2) {
    	    *p++ = (char)c;
    	    i++;
    	}
    }
    *p = '\0';
    realloc(pic->comment, i+1); /*ｺﾒﾝﾄﾊﾞｯﾌｧを実際に読込んだサイズにする*/
    pic->comment2 = pic->comment;

    /* check 0x00 */
    if (PIC_RdByte(pic) != 0x00) {
    	printf("PICヘッダがおかしい\n");
    	goto ERR;
    }
    /* PIC-TYPE & MODE 読込み */
    pic->typ = c = PIC_RdWord(pic);
    pic->macNo = c & 0x0f;
    pic->macMode = (c>>4) & 0x0f;
    /* 色ビット数 */
    pic->colbit = PIC_RdWord(pic);

    /* 画像サイズ */
    pic->sizeXo = pic->xsize = PIC_RdWord(pic);
    pic->sizeYo = pic->ysize = PIC_RdWord(pic);

    switch (pic->macNo) {
    case PIC_X68K/*0*/:
    	pic->asp1 = 13;
    	pic->asp2 = 9;
    	if (pic->colbit == 4) {
    	    pic->asp1 = 1;
    	    pic->asp2 = 1;
    	}
    	if (pic->colbit <= 8) {
    	    int col;
    	    col = 0x01 << pic->colbit;
    	    pic->rgb = calloc_m(col, 3);
    	    if (pic->rgb == NULL) {
    	    	goto ERR;
    	    }
    	    for (i = 0; i < col*3; i+=3) {
    	    	pic->rgb[i+1] = (BYTE)(PIC_RdBits(pic, 5)<<3);
    	    	pic->rgb[i+0] = (BYTE)(PIC_RdBits(pic, 5)<<3);
    	    	pic->rgb[i+2] = (BYTE)(PIC_RdBits(pic, 5)<<3);
    	    	if (PIC_RdBit(pic)) {
    	    	    pic->rgb[i+0] |= 0x04;
    	    	    pic->rgb[i+1] |= 0x04;
    	    	    pic->rgb[i+2] |= 0x04;
    	    	}
    	    }
    	}
    	break;

    case PIC_88VA/*1*/:
    	if (pic->macMode == 2) {
    	    pic->rgb = PIC_SetDiRGB256();
    	    pic->colbit = 8;
    	}
    	pic->asp1 = 1;	/* 5 */
    	pic->asp2 = 1;	/* 6 */
    	if (pic->macMode == 0) {
    	    if (pic->xsize == 640 && pic->ysize >= 200) {
    	    	pic->asp1 = 1;
    	    	pic->asp2 = 2;
    	    } else if (pic->xsize == 320 && pic->ysize >= 400) {
    	    	pic->asp1 = 2;
    	    	pic->asp2 = 1;
    	    }
    	}
    	break;

    case PIC_TOWNS/*2*/:
    	pic->asp1 = 1;
    	pic->asp2 = 1;
    	break;

    case PIC_MAC/*3*/:
    	pic->asp1 = 1;
    	pic->asp2 = 1;
    	break;

    case PIC_EX/*0x0f*/:    /* 拡張PIC */
    	pic->xstart = PIC_RdWord(pic);
    	if (pic->xstart == -1) {
    	    pic->xstart = 0;
    	}
    	pic->ystart = PIC_RdWord(pic);
    	if (pic->ystart == -1) {
    	    pic->ystart = 0;
    	}
    	pic->asp1 = PIC_RdByte(pic);
    	pic->asp2 = PIC_RdByte(pic);
    	if (pic->asp1 == 0 || pic->asp2 == 0) {
    	    pic->asp1 = pic->asp2 = 1;
    	}
    	if (pic->colbit <= 8) {
    	    pic->palbit = PIC_RdByte(pic);
    	    if (pic->palbit > 8) {
    	    	printf("このパレットには対応していません(パレット１つのサイズが8ビット以上あります)\n");
    	    	exit(1);
    	    }
    	    {
    	    	int col,n;
    	    	col = 0x01 << pic->colbit;
    	    	pic->rgb = calloc_m(col, 3);
    	    	if (pic->rgb == NULL) {
    	    	    goto ERR;
    	    	}
    	    	n = 8 - pic->palbit;
    	    	for (i = 0; i < col*3; i+=3) {
    	    	    pic->rgb[i+1] = (BYTE)(PIC_RdBits(pic, pic->palbit)<<n);
    	    	    pic->rgb[i+0] = (BYTE)(PIC_RdBits(pic, pic->palbit)<<n);
    	    	    pic->rgb[i+2] = (BYTE)(PIC_RdBits(pic, pic->palbit)<<n);
    	    	}
    	    }
    	}
    	break;

    default:
    	printf("このプログラムのしらない、PIC-TYPEです\n");
    }

    /* /mfged5形式ヘッダのとき */
    if (/*pic->macNo != 0x0f &&*/ strncmp(pic->comment,"/mfged5/",8) == 0) {
    	char *s;
    	s = pic->comment + 8;
    	pic->xstart = (s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+(s[3]-'0');
    	s += 4;
    	pic->ystart = (s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+(s[3]-'0');
    	pic->comment2 = pic->comment + 8 + 8;
    }

    /* /MM形式ヘッダのとき */
    if (/*pic->macNo != 0x0f &&*/ strncmp(pic->comment,"/MM/",4) == 0) {
    	PIC_GetMM(pic);
    }

    pic->asp1o = pic->asp1;
    pic->asp2o = pic->asp2;
    return pic;

  ERR:
    PIC_Free(pic);
    return NULL;
}

/*-------------------------------------*/
/*-------------------------------------*/

static void PIC_InitC7t(PIC *pic)
    /* 128個の色キャッシュ・テーブルの初期化 */
{
    int i;

    pic->c7t = calloc_m(128, sizeof(PIC_C7TBL));
    if (pic->c7t == NULL){
    	exit(1);
    }
    for (i = 0; i < 127; i++) {
    	pic->c7t[i].col = 0;
    	pic->c7t[i+1].lft = i;
    	pic->c7t[i].rig = i+1;
    }
    pic->c7t[0].lft = 127;
    pic->c7t[127].rig = 0;
    pic->c7p = 0;
    return;
}

static void PIC_SetC7(PIC *pic, PIXEL col)
    /* 色キャッシュテーブルに color を登録 */
{
    pic->c7p = pic->c7t[pic->c7p].rig;
    pic->c7t[pic->c7p].col = col;
}

static PIXEL PIC_GetC7(PIC *pic, int i)
    /* 色キャッシュ・テーブルから色を取りだし、その色の登録の順番を最新にする*/
{
    if (i != pic->c7p) {
    	pic->c7t[pic->c7t[i].rig].lft = pic->c7t[i].lft;
    	pic->c7t[pic->c7t[i].lft].rig = pic->c7t[i].rig;
    	pic->c7t[pic->c7t[pic->c7p].rig].lft = i;
    	pic->c7t[i].rig = pic->c7t[pic->c7p].rig;
    	pic->c7t[pic->c7p].rig = i;
    	pic->c7t[i].lft = pic->c7p;
    	pic->c7p = i;
    }
    return pic->c7t[i].col;
}

/*------ 色(ピクセル)を読込むルーチン -----*/

static PIXEL PIC_GetCol4(PIC *pic)
    /* 色データを得る */
    /* チェックしてません！　正しいか嘘か妖しいぞ^^; */
{
    PIXEL col;
    int n;

    n = (int)PIC_RdBits(pic, 4);
    col = ((DWORD)n<<24)
    	| ((DWORD)pic->rgb[n*3+0])  	/* R */
    	| ((DWORD)pic->rgb[n*3+1]<<8)	/* G */
    	| ((DWORD)pic->rgb[n*3+2]<<16); /* B */
    if (PIC_rdDebFlg == 2) {
    	printf("p4[%06lx]\n",(long)col);
    }
    return col;
}

static PIXEL PIC_GetCol8(PIC *pic)
    /* 色データを得る */
{
    PIXEL col;
    int n;

    n = (int)PIC_RdBits(pic, 8);
    col = ((DWORD)n<<24)
    	| ((DWORD)pic->rgb[n*3+0])  	/* R */
    	| ((DWORD)pic->rgb[n*3+1]<<8)	/* G */
    	| ((DWORD)pic->rgb[n*3+2]<<16); /* B */
    if (PIC_rdDebFlg == 2) {
    	printf("p8[%06lx]\n",(long)col);
    }
    return col;
}

static PIXEL PIC_GetCol12(PIC *pic)
    /* 色データを得る（読込むor色キャッシュ・テーブル) */
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	int r,g,b;
    	g = (int)PIC_RdBits(pic, 4); g = (g<<4)|g;  /* G */
    	r = (int)PIC_RdBits(pic, 4); r = (r<<4)|r;  /* R */
    	b = (int)PIC_RdBits(pic, 4); b = (b<<4)|b;  /* B */
    	col = ((long)b << 16) | ((long)g<<8) | (long)r;
    	PIC_SetC7(pic, col);	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("P12[%06lx]\n",(long)col);
    	}
    }
    return col;
}

static PIXEL PIC_GetCol15(PIC *pic)
    /* 色データを得る（読込むor色キャッシュ・テーブル) */
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	int r,g,b;
    	g = (int)PIC_RdBits(pic, 5); g = (g<<3)/*|(g>>2)*/; 	/* G */
    	r = (int)PIC_RdBits(pic, 5); r = (r<<3)/*|(r>>2)*/; 	/* R */
    	b = (int)PIC_RdBits(pic, 5); b = (b<<3)/*|(b>>2)*/; 	/* B */
    	col = ((long)b << 16) | ((long)g<<8) | (long)r;
    	PIC_SetC7(pic, col);	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("P15[%06lx]\n",(long)col);
    	}
    }
    return col;
}

static PIXEL PIC_GetCol16(PIC *pic)
    /* 色データを得る（読込むor色キャッシュ・テーブル) */
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	col  = PIC_RdBits(pic, 5) <<(3 +8); 	/* G */
    	col |= PIC_RdBits(pic, 5) <<(3);    	/* R */
    	col |= PIC_RdBits(pic, 5) <<(3 +8+8);	/* B */
    	if(PIC_RdBit(pic)) {
    	    col |= 0x040404L/*0x070707L*/;
    	}
    	PIC_SetC7(pic, col);	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("P16[%06lx]\n",(long)col);
    	}
    }
    return col;
}

static PIXEL PIC_GetCol24(PIC *pic)
    /* 色データを得る（読込むor色キャッシュ・テーブル) */
    /* ぜんぜんチェックしてません！　正しいか嘘か妖しいぞ^^; */
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	col  = PIC_RdBits(pic, 8) << 8;     /* G */
    	col |= PIC_RdBits(pic, 8);  	    /* R */
    	col |= PIC_RdBits(pic, 8) << (8+8); /* B */
    	PIC_SetC7(pic, col);	    	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("P24[%06lx]\n",(long)col);
    	}
    }
    return col;
}

#ifdef PIC32COL
static PIXEL PIC_GetCol32(PIC *pic)
    /* 色データを得る（読込むor色キャッシュ・テーブル) */
    /* ぜんぜんチェックしてません！　正しいか嘘か妖しいぞ^^; */
    /* 実は32ﾋﾞｯﾄ色のGRBIの配置をしらんのな。しかたないのでとりあえず、 */
    /* 32 ビット色は R,G,B,A 順で各 8ビット、A はｱﾄﾋﾞｭﾚｰﾄ(?…輝度?)とかってに*/
    /* 解釈. 出力では A を無視して R,G,B を出力するようにしてる。32ﾋﾞｯﾄ色な*/
    /* 画像が出現するか不明だけど、駄目だったら、そのとき(誰かが)修正、と^^;*/
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	col  = PIC_RdBits(pic, 8) << 8;     /* G */
    	col |= PIC_RdBits(pic, 8);  	    /* R */
    	col |= PIC_RdBits(pic, 8) << (8+8); /* B */
    	col |= PIC_RdBits(pic, 8) <<(8+8+8);/* I */
    	PIC_SetC7(pic, col);	    	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("P32[%06lx]\n",(long)col);
    	}
    }
    return col;
}
#endif

static PIXEL PIC_GetCol15mac(PIC *pic)
    /* 色データを得る（mac-pic用) */
    /* ぜんぜんチェックしてません！　正しいか嘘か妖しいぞ^^; */
    /* ピクセルは (0)BBBBBRRRRRGGGGG らしい(T^T)　*/
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	int r,g,b;
      #if 1 /*これはいらないけど...*/
    	if (pic->colbit == 16) {
    	    PIC_RdBit(pic); /* 0 */
    	}
      #endif
    	r = (int)PIC_RdBits(pic, 5); r = (r<<3)/*|(r>>2)*/; 	/* R */
    	g = (int)PIC_RdBits(pic, 5); g = (g<<3)/*|(g>>2)*/; 	/* G */
    	b = (int)PIC_RdBits(pic, 5); b = (b<<3)/*|(b>>2)*/; 	/* B */
    	col = ((long)b << 16) | ((long)g<<8) | (long)r;
    	PIC_SetC7(pic, col);	    	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("Pmac[%06lx]\n",(long)col);
    	}
    }
    return col;
}

static PIXEL PIC_GetCol16va(PIC *pic)
    /* PC88VA-64K-PIC:色データを得る（読込むor色キャッシュ・テーブル) */
    /* 色データを得る（読込むor色キャッシュ・テーブル) */
    /* GGGGGGRRRRRBBBBB */
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	int r,g,b;
    	g = (int)PIC_RdBits(pic, 6) << 2; /*if (g) {g |= 0x3;}*/
    	r = (int)PIC_RdBits(pic, 5) << 3; if (r) {r |= 0x4/*0x7*/;}
    	b = (int)PIC_RdBits(pic, 5) << 3; if (b) {b |= 0x4/*0x7*/;}
    	col = ((long)b << 16) | ((long)g<<8) | (long)r;
    	PIC_SetC7(pic, col);	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("P16[%06lx]\n",(long)col);
    	}
    }
    return col;
}

#ifdef VA256PIC

static PIXEL PIC_GetCol8va(PIC *pic)
    /* PC88VA-256-PIC:色データを得る（読込むor色キャッシュ・テーブル) */
{
    PIXEL col;

    if (PIC_RdBit(pic)) {   /* 128個の色テーブルから色を得る */
    	col = PIC_GetC7(pic, (int)PIC_RdBits(pic, 7));
    	if (PIC_rdDebFlg == 2) {
    	    printf("p[%06lx]\n",(long)col);
    	}
    } else {	    /* 色を読込む */
    	col = PIC_RdBits(pic, 16);
    	PIC_SetC7(pic, col);	    /* 色キャッシュ・テーブルに登録 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("P8va[%06lx]\n",(long)col);
    	}
    }
    return col;
}

static PIXEL *PIC_LineVA256(PIC *pic, PIXEL *p)
{
    PIXEL *q;
    int i,n;

    q = pic->va256buf;
    for (i = 0; i < pic->xsize; i++) {
    	n = (int)*p & 0xff;
    	*q++ =((DWORD)n<<24)
    	    | ((DWORD)pic->rgb[n*3+0])	    /* R */
    	    | ((DWORD)pic->rgb[n*3+1]<<8)   /* G */
    	    | ((DWORD)pic->rgb[n*3+2]<<16); /* B */
    	n = ((int)*p >> 8) & 0xff;
    	*q++ =((DWORD)n<<24)
    	    | ((DWORD)pic->rgb[n*3+0])	    /* R */
    	    | ((DWORD)pic->rgb[n*3+1]<<8)   /* G */
    	    | ((DWORD)pic->rgb[n*3+2]<<16); /* B */
    	p++;
    }
    return pic->va256buf;
}

#endif	/* VA256PIC */

static void PIC_InitGetCol(PIC *pic)
    /* 色数,PIC-TYPEに従って色読込みルーチンを選ぶ */
{
    /* */
    switch(pic->colbit) {
    case 4:
    	pic->funcGetCol = PIC_GetCol4;
    	/*printf("16色PICは誤変換の可能性大^^;\n");*/
    	break;
    case 8:
    	pic->funcGetCol = PIC_GetCol8;
    	break;
    case 12:
    	pic->funcGetCol = PIC_GetCol12;
    	break;
    case 15:
    	pic->funcGetCol = PIC_GetCol15;
    	break;
    case 16:
    	pic->funcGetCol = PIC_GetCol16;
    	break;
    case 24:
    	pic->funcGetCol = PIC_GetCol24;
    	break;
    case 32:
       #ifdef PIC32COL
    	pic->funcGetCol = PIC_GetCol32;
    	break;
       #else
    	printf("このプログラム作成時には、32bit色は仕様が決定していないので存在していません\n");
    	exit(1);
       #endif
    default:
    	printf("4,8,12,15,16,24,32ビット色以外のPICはないはずです\n");
    	exit(1);
    }

    switch (pic->macNo) {
    case PIC_X68K/*0*/:
    	break;
    case PIC_88VA/*1*/:
    	if (pic->macMode == 2) {
    	 #ifdef VA256PIC
    	    pic->colbit = 8;
    	    pic->funcGetCol = PIC_GetCol8va;
    	    pic->va256buf = calloc_m(pic->xsize*2+1,sizeof(PIXEL));
    	  #else
    	    printf("VA256色PICには未対応\n");
    	    exit(1);
    	  #endif
    	} else if (pic->colbit == 16) {
    	    	pic->funcGetCol = PIC_GetCol16va;
    	/*} else if (pic->colbit == 12) {
    	    pic->funcGetCol = PIC_GetCol12;*/
    	}
    	break;
    case PIC_TOWNS/*2*/:
    	/*printf("TOWNS-PICは未確認（変換した方は成功したかどうか連絡くださるとうれしいのな）\n");*/
    	break;

    case PIC_MAC/*3*/:
    	if (pic->colbit == 15 || pic->colbit == 16) {
    	    pic->funcGetCol = PIC_GetCol15mac;
    	}
    	/*printf("MAC-PICは未確認（変換した方は成功したかどうか連絡くださるとうれしいな）\n");*/
    	/*exit(1);*/
    	break;
    case PIC_EX/*0x0f*/:    /* 拡張PIC */
    	break;
    default:
    	printf("このプログラムのしらない、PIC-TYPEです\n");
    }
}

#ifdef DITHER
/*-------------------  ディザによる減色   -----------------*/
#define PIC_DIT_CNT (sizeof PIC_dit / sizeof PIC_dit[0])
static BYTE PIC_dit1[64],PIC_dit2[64],PIC_dit3[64];

static BYTE PIC_dit[][64] = {
   {0, 32,8, 40, 2, 34,10,42,/*Bayer 8*8*/
    48,16,56,24, 50,18,58,26,
    12,44,4, 36, 14,46,6, 38,
    60,28,52,20, 62,30,54,22,
    3, 35,11,43, 1, 33,9, 41,
    51,19,59,27, 49,17,57,25,
    15,47,7, 39, 13,45,5, 36,
    63,31,55,23, 61,29,53,21},
    /* 4*4 */
   {0, 32,8, 40, 0, 32,8, 40,/*Bayer*/
    48,16,56,24, 48,16,56,24,
    12,44,4, 36, 12,44,4, 36,
    60,28,52,20, 60,28,52,20,
    0, 32,8, 40, 0, 32,8, 40,
    48,16,56,24, 48,16,56,24,
    12,44,4, 36, 12,44,4, 36,
    60,28,52,20, 60,28,52,20},
   {40,16,24,32, 40,16,24,32, /* Half Tone */
    48,0, 8, 56, 48,0, 8, 56,
    28,36,44,20, 28,36,44,20,
    12,60,52,4,  12,60,52,4,
    40,16,24,32, 40,16,24,32,
    48,0, 8, 56, 48,0, 8, 56,
    28,36,44,20, 28,36,44,20,
    12,60,52,4,  12,60,52,4},
   {48,16,32,56, 48,16,32,56,/* Screw...*/
    44,0, 8, 24, 44,0, 8, 24,
    28,12,4, 40, 28,12,4, 40,
    60,36,20,52, 60,36,20,52,
    48,16,32,56, 48,16,32,56,
    44,0, 8, 24, 44,0, 8, 24,
    28,12,4, 40, 28,12,4, 40,
    60,36,20,52, 60,36,20,52},
   {0, 4, 32,36, 0, 4, 32,36,/*Dot dispersed*/
    15,20,48,52, 15,20,48,52,
    40,44,8, 12, 40,44,8, 12,
    52,60,48,28, 52,60,48,28,
    0, 4, 32,36, 0, 4, 32,36,
    15,20,48,52, 15,20,48,52,
    40,44,8, 12, 40,44,8, 12,
    52,60,48,28, 52,60,48,28},
};



static void PIC_Dither3(PIC *pic, PIXEL *buf)
    /* ﾋﾟｸｾﾙ･ﾃﾞｰﾀ1行を8 色に減色。出力でのﾋﾟｸｾﾙ･ﾃﾞｰﾀ自体は１６色画像 */
{
    int x, dy;
    PIXEL col;
    int dyx, r,g,b/*,n*/;

    dy = ((pic->ditY++) & 7)<<3;
    for (x = 0; x < pic->sizeXo; x++) {
    	dyx = (dy|(x&7));
    	col = buf[x];
    	r = (BYTE)((col >> 0) & 0xff);	r += PIC_dit1[dyx]; r >>= 8;
    	g = (BYTE)((col >> 8) & 0xff);	g += PIC_dit1[dyx]; g >>= 8;
    	b = (BYTE)((col >>16) & 0xff);	b += PIC_dit1[dyx]; b >>= 8;
    	/*n = (int)(col >> 24);*/
    	col &= 0xffffffL;
    	col |= ((DWORD)((g << 2) + (r<<1) + b) << 24)/* | (n << 28)*/;
    	buf[x] = col;
    }
    return;
}

static void PIC_Dither4(PIC *pic, PIXEL *buf)
    /* ﾋﾟｸｾﾙ･ﾃﾞｰﾀ 1行を16色に減色 */
{
    int x, dy;
    PIXEL col;
    int dyx, r,g,b/*,n*/;

    dy = ((pic->ditY++) & 7)<<3;
    for (x = 0; x < pic->sizeXo; x++) {
    	dyx = (dy|(x&7));
    	col = buf[x];
    	b = (BYTE)((col >>16) & 0xff);	b += PIC_dit1[dyx]; b >>= 8;
    	r = (BYTE)((col >> 0) & 0xff);	r += PIC_dit1[dyx]; r >>= 8;
    	g = (BYTE)((col >> 8) & 0xff);	g += PIC_dit2[dyx];
    	g *= 3; g >>= 8;
    	/*n = (int)(col >> 24)*/;
    	col &= 0xffffffL;
    	col |= ((DWORD)((g << 2) + (r<<1) + b) << 24) /*|(n << 28)*/;
    	buf[x] = col;
    }
    return;
}

static void PIC_Dither8(PIC *pic, PIXEL *buf)
    /* ﾋﾟｸｾﾙ･ﾃﾞｰﾀ 1行を256色に減色 */
{
    int x, dy;
    PIXEL col;
    int dyx, r,g,b;

    dy = ((pic->ditY++) & 7)<<3;
    for (x = 0; x < pic->sizeXo; x++) {
    	dyx = (dy | (x & 7));
    	col = buf[x];
    	r = (BYTE)((col >> 0) & 0xff);	r += PIC_dit3[dyx]; r *= 7; r >>= 8;
    	g = (BYTE)((col >> 8) & 0xff);	g += PIC_dit3[dyx]; g *= 7; g >>= 8;
    	b = (BYTE)((col >>16) & 0xff);	b += PIC_dit2[dyx]; b *= 3; b >>= 8;
    	col &= 0xffffffL;
    	col |= ((DWORD)((g << 5) + (r<<2) + b) << 24);
    	buf[x] = col;
    }
    return;
}

BYTE *PIC_DitherMode(PIC *pic, int ditherMode)
    /* ディザによる減色の準備 */
{
    static BYTE rgb3[48] = {	/* 8色用の固定ﾊﾟﾚｯﾄ */
    	0x00,0x00,0x00, 0x00,0x00,0xff, 0xff,0x00,0x00, 0xff,0x00,0xff,
    	0x00,0xff,0x00, 0x00,0xff,0xff, 0xff,0xff,0x00, 0xff,0xff,0xff,
    	0x00,0x00,0x00, 0x00,0x00,0xff, 0xff,0x00,0x00, 0xff,0x00,0xff,
    	0x00,0xff,0x00, 0x00,0xff,0xff, 0xff,0xff,0x00, 0xff,0xff,0xff
    };
    static BYTE rgb4[48] = {	/* 16色用の固定ﾊﾟﾚｯﾄ */
    	0x00,0x00,0x00, 0x00,0x00,0xff, 0xff,0x00,0x00, 0xff,0x00,0xff,
    	0x00,0x55,0x00, 0x00,0x55,0xff, 0xff,0x55,0x00, 0xff,0x55,0xff,
    	0x00,0xaa,0x00, 0x00,0xaa,0xff, 0xff,0xaa,0x00, 0xff,0xaa,0xff,
    	0x00,0xff,0x00, 0x00,0xff,0xff, 0xff,0xff,0x00, 0xff,0xff,0xff
    };
    int i,n;

    n = ditherMode >> 8;
    for (i = 0; i < 64; i++) {
    	PIC_dit1[i] = PIC_dit[n][i]*2;
    	PIC_dit2[i] = PIC_dit[n][i];
    	PIC_dit3[i] = PIC_dit[n][i]>>1;
    }
    pic->ditherMode = ditherMode;
    ditherMode &= 0xff;
    if (ditherMode == 3) {  	    /* 8色に減色するとき */
    	pic->ditherFunc = PIC_Dither3;
    	/*memcpy(rgb, rgb3,48);*/
    	return rgb3;
    } else if (ditherMode == 4) {   /* 16色に減色するとき */
    	pic->ditherFunc = PIC_Dither4;
    	/*memcpy(rgb, rgb4,48);*/
    	return rgb4;
    } else {	    	    	    /* 256色に減色するとき */
    	pic->ditherFunc = PIC_Dither8;
    	pic->ditRGB = PIC_SetDiRGB256();
    	return pic->ditRGB;
    }
}
#endif	/* DITHER */

/*-------------------------------------*/

#ifdef ASPX68K
    /* x68k なアスペクト比のとき、１行(横方向)を 4/3 倍する */
    /* 3ドットごとに 1 ドット挿入(4/3だけでなく5/4,3,2も可) */
    /* 多色ならその挿入する１ドットは隣合うピクセルの平均   */
    /* 16色,256色PICなら左隣を複写  	    	    	    */

static PIXEL *PIC_AspX68kLine(PIC *pic, PIXEL *p)
{
    int c,i,x;

    for (c = i = x = 0; i < pic->xsize-1; i++, p++) {
    	pic->aspX68kBuf[x++] = *p;
    	if (++c == pic->aspX68k) {
    	    int n,r,g,b;

    	    n = (int)(*p >> 24);
    	    r = (BYTE)(((BYTE)*p + (BYTE)p[1]) >> 1);
    	    g = (BYTE)(((BYTE)(*p >> 8) + (BYTE)(p[1]>> 8)) >> 1);
    	    b = (BYTE)(((BYTE)(*p >>16) + (BYTE)(p[1]>>16)) >> 1);
    	    pic->aspX68kBuf[x++] = ((DWORD)n << 24)
    	    	    	    	    + ((DWORD)b << 16)
    	    	    	    	    + ((DWORD)g << 8)
    	    	    	    	    + (DWORD)r;
    	    c = 0;
    	}
    }
    pic->aspX68kBuf[x] = *p;
    return pic->aspX68kBuf;
}

void PIC_InitAspX68k(PIC *pic,int dot)
    /* アスペクト比がX68Kのものだったときで、横幅調整を行うばあいの設定 */
    /* VA-PIC の 横:縦が 1:2のものや 2:1のものにも対応 */
{
    pic->aspX68k = 0;
    if (dot == 0) {
    	return;
    }
  #if 0
    if (pic->typ == 0x1f) {
    	pic->aspX68k = dot;
    }
  #endif
    if (pic->asp2) {
    	int n;
    	n = (int)((long)pic->asp1 * 100L / pic->asp2);
    	if (n >= 125 && n <= 150) {
    	    pic->aspX68k = dot;
    	} else if (n >= 180 && n <= 230) {  /* 2:1 ... VA-PIC...*/
    	    pic->aspX68k = dot = 1;
    	} else if (n >= 40 && n <= 60) {    /* 1:2 ... VA-PIC,MSX-PIC...*/
    	    pic->aspX68k = dot = -1;
    	}
    }
    if (pic->aspX68k > 0) {
    	int x;
    	x = pic->xsize * (dot+1);
    	if (dot > 1 && x % dot) {
    	    x += dot;
    	}
    	x /= dot;
    	pic->aspX68kBuf = calloc_m(x + dot + 1, sizeof(PIXEL));
    	pic->sizeXo = x;
    	pic->asp1o = 1;
    	pic->asp2o = 1;
    }
    return;
}

#endif	/* ASPX68K */

/*-------------------------------------*/

#ifndef SML
#define PIC_SetCmap(pic,x,y,c)	(pic->cmap[y][x] = c)/*cmap[y][x]に連鎖を記録*/
#define PIC_GetCmap(pic,x,y)	(pic->cmap[y][x])    /*cmap[y][x]の連鎖を読む*/
#else

static void PIC_SetCmap(PIC *pic, int x, int y, int c)
{
    if (x & 0x01) { /* 奇数座標のとき */
    	x >>= 1;
    	pic->cmap[y][x] &= 0xf0;
    	pic->cmap[y][x] |= c;
    } else {	    /* 偶数 */
    	x >>= 1;
    	pic->cmap[y][x] &= 0x0f;
    	pic->cmap[y][x] |= c << 4;
    }
}

static int PIC_GetCmap(PIC *pic, int x, int y)
{
    if (x & 0x01) { /* 奇数座標のとき */
    	return pic->cmap[y][x>>1] & 0x0f;
    } else {	    /* 偶数 */
    	return (pic->cmap[y][x>>1] >> 4);
    }
}
#endif



static void PIC_RdChain(PIC *pic, int xx, int yy)
    /* 変化点の連鎖を pic->cmap に展開	    	    	    */
    /*	 連鎖の値は -2,-1,0,1,-2 の5つだけど、	    	    */
    /*	 連鎖無しを表す値を０にするために、連鎖の値に+4して */
    /*	 pic->cmap 上には記録する   */
{
    int i;

    /*if (PIC_rdDebFlg == 2) {
    	printf("(%d,%d)",xx,yy);
    }*/
    for(i=0;;i++) {
    	++yy;
    	if (PIC_RdBit(pic)) {
    	    if (PIC_RdBit(pic)) {   	/*11 (+1,+1) */
    	    	++xx;/* if (xx >= pic->xsize) {xx = 0;}*/
    	    	if (yy < pic->sizeYo) {
    	    	    /*pic->cmap[yy][xx] = -1 + 4;*/
    	    	    PIC_SetCmap(pic, xx,yy, -1 + 4);
    	    	}
    	    } else {	    	    	/*11 (+0,+1) */
    	    	if (yy < pic->sizeYo) {
    	    	    /*pic->cmap[yy][xx] = 0 + 4;*/
    	    	    PIC_SetCmap(pic, xx,yy, 0 + 4);
    	    	}
    	    }
    	} else {
    	    if (PIC_RdBit(pic)) {   	/*01 (-1,+1) */
    	    	--xx;/* if (xx < 0) {xx += pic->xsize;} */
    	    	if (yy < pic->sizeYo) {
    	    	    /*pic->cmap[yy][xx] = +1 + 4;*/
    	    	    PIC_SetCmap(pic, xx,yy, +1 + 4);
    	    	}
    	    } else {
    	    	if (PIC_RdBit(pic)) {
    	    	    if (PIC_RdBit(pic)) {/*0011 (+2,+1) */
    	    	    	xx += 2;/*if (xx >= pic->xsize) {xx -= pic->xsize;}*/
    	    	    	if (yy < pic->sizeYo) {
    	    	    	    /*pic->cmap[yy][xx] = -2 + 4;*/
    	    	    	    PIC_SetCmap(pic, xx,yy, -2 + 4);
    	    	    	}
    	    	    } else {	    	/*0010 (-2,+1);*/
    	    	    	xx -= 2;/*if (xx < 0) {xx += pic->xsize;}*/
    	    	    	if (yy < pic->sizeYo) {
    	    	    	    /*pic->cmap[yy][xx] = +2 + 4;*/
    	    	    	    PIC_SetCmap(pic, xx,yy, +2 + 4);
    	    	    	}
    	    	    }
    	    	} else {
    	    	    if (PIC_rdDebFlg == 2) {
    	    	    	printf("\nchainLine %d\n",i);
    	    	    }
    	    	    return;
    	    	}
    	    }
    	}
    	if (PIC_rdDebFlg == 2) {
    	    printf("-(%d,%d)",xx,yy);
    	}
    } /* end for */
}

int PIC_PutLines(PIC *pic, void *o, PIC_FUNC_PUTLINE funcPutLine)
    /* PICを展開... 1行展開するごとに 1行出力関数(funcPutLine)を行う */
{
    long len;
    PIXEL col;
    int  x,y;

    /* 変換に必要なメモリを確保 */
    /* ピクセル・データを収めるバッファ... ２行分 */
    x = pic->xsize;
    pic->plin[0] = calloc_m(x,sizeof(PIXEL));
    if(pic->plin[0]==NULL) {
    	goto ERR;
    }
    pic->plin[1] = calloc_m(x,sizeof(PIXEL));
    if(pic->plin[1]==NULL) {
    	goto ERR;
    }
    /* 9ビット色以上(12,15,16,24,32)のPICで使う128色のキャッシュを初期化*/
    PIC_InitC7t(pic);

    /* color func 初期化*/
    PIC_InitGetCol(pic);

    /* 連鎖情報を記録するためのバッファを確保 */
    pic->cmap = calloc_m(pic->ysize, sizeof(BYTE*));
    if(pic->cmap == NULL) {
    	goto ERR;
    }
  #ifdef SML
    x = (x & 0x01) ? (x/2)+1 : x / 2;
  #endif
    for (y = 0; y < pic->sizeYo;y++) {
    	pic->cmap[y] = calloc(x, 1);
    	if (pic->cmap[y] == NULL) {
    	    if (y == 0) {
    	    	goto ERR;
    	    }
    	    printf("メモリが足りないので%d行分だけ変換してみますけど...\n",y);
    	    pic->sizeYo = y;
    	    break;
    	}
    }

    /* 一番最初の'長さ'を読みとばす */
    x = y = 0;
    len = PIC_RdLen(pic);
    if (len != 1) { 	/* おそらく、必ず１であろう */
    	if (PIC_rdDebFlg == 2) {
    	    printf("len=%ld\n",(long)len);
    	}
    	printf("一番最初の'次の変化点までの長さ'が1でない(%d)\n",len);
    	len -= 1;
    	col = 0;
    	goto J1;
    }

    /* 実際の展開を行う */
    for (; ;) {
    	col = (*pic->funcGetCol)(pic);	/* 色を読込む */
    	if (PIC_RdBit(pic)) {	    	/* 1 なら */
    	    PIC_RdChain(pic, x,y);  	/* 連鎖を pic->cmap に記録する */
    	}
    	len = PIC_RdLen(pic);	    /* 長さを読込む...len >= 1 */
    	if (PIC_rdDebFlg == 2) {
    	    printf("col=%06lx\nlen=%ld\n", (long)col, (long)len);
    	}

  J1:
    	/* 長さの分だけ繰り返す */
    	do {
    	    if (PIC_GetCmap(pic, x,y)) {    /* 書込む点が変更点の連鎖のとき */
    	    	/* 対応する連鎖より１行前のピクセル・データから点(色)を得り */
    	    	/* だし、現在の色にする */
    	    	int xx,yy;

    	    	xx = x + PIC_GetCmap(pic, x,y) - 4;
    	    	yy = y;
    	    	col = pic->plin[(yy-1) & 0x0001][xx];
    	    	if (PIC_rdDebFlg == 2) {
    	    	    printf("q[%06lx] (%d,%d)\n",(long)col,xx,yy);
    	    	}
    	    }

    	    /* 点(色)を設定 */
    	    pic->plin[y&0x0001][x] = col;

    	    /* 一行分のバッファが埋まったら出力 */
    	    if (++x >= pic->xsize) {
    	    	PIXEL *p;
    	      #ifdef VA256PIC
    	    	int vflg = 0;
    	      #endif
    	      #ifdef ASPX68K
    	    	int acnt = 0;
    	      #endif

    	    	x = 0;
    	    	p = pic->plin[y & 0x0001];

    	      #ifdef VA256PIC
    	    	if (pic->typ == 0x21) {
    	    	    vflg = 1;
    	    	    p = PIC_LineVA256(pic,p);
    	    	}
    	VA256PIC_LOOP:
    	      #endif

    	      #ifdef ASPX68K
    	    	if (pic->aspX68k) {
    	    	    if (pic->aspX68k > 0) {
    	    	    	p = PIC_AspX68kLine(pic,p);
    	    	    } else {
    	    	    	acnt = -pic->aspX68k;
    	    	    }
    	    	}
    	ASP_1_2_LOOP:
    	      #endif

    	      #ifdef DITHER
    	    	if (pic->ditherMode) {	/* ディザで減色するばあい */
    	    	    (*pic->ditherFunc)(pic, p); /* 減色を実行 */
    	    	}
    	      #endif

    	    	(*funcPutLine)(o, p);	/* 1行出力 */
    	    	if(PIC_rdDebFlg) {
    	    	    if (PIC_rdDebFlg == 1)
    	    	    	printf("[%d]\r",y);
    	    	    else
    	    	    	printf("[%d]\n",y);
    	    	}

    	      #ifdef ASPX68K
    	    	if (pic->aspX68k < 0 && acnt) {
    	    	    --acnt;
    	    	    goto ASP_1_2_LOOP;
    	    	}
    	      #endif

    	      #ifdef VA256PIC
    	    	if (pic->typ == 0x21 && vflg) {
    	    	    p = pic->va256buf + pic->xsize;
    	    	    vflg = 0;
    	    	    goto VA256PIC_LOOP;
    	    	}
    	      #endif

    	    	if (++y >= pic->sizeYo) {   /* 行番号更新 */
    	    	    goto EXIT_LOOP;
    	    	}
    	    }
    	} while (--len);
    }
  EXIT_LOOP:;

    /* 変換で確保したメモリを解放 */
    free (pic->c7t);
    pic->c7t = NULL;
    for (y = pic->sizeYo;--y >= 0;) {
    	free(pic->cmap[y]);
    	pic->cmap[y] = NULL;
    }
    free(pic->cmap);
    pic->cmap = NULL;
    free(pic->plin[1]);
    pic->plin[1] = NULL;
    free(pic->plin[0]);
    pic->plin[0] = NULL;
    return 0;

  ERR:
    printf("変換に必要なメモリを確保できなかったよお\n");
    return 1;
}

/*end PIC*/

/*---------------------------------------------------------------------------*/
/*  	    	    	    T I F F 出 力   	    	    	    	     */
/*---------------------------------------------------------------------------*/
/*module TIF*/
    /*export TIF, TIF_Create, TIF_CloseW, TIF_PutLine */

#define TIF__BYTE   1
#define TIF__ASCIIZ 2
#define TIF__WORD   3
#define TIF__DWORD  4
#define TIF__RAT    5

typedef struct TIF_T {
    FILE *fp;
    char *name;
    char *tmpname;
    int  colbit;
    int  xsize, ysize;
    int  xstart, ystart;
    long xresol, yresol;
    BYTE *rgb;
/* private: */
    BYTE *linebuf;
} TIF;

static void TIF_Free(TIF *tf)
{
    if (tf->fp) {
    	fclose(tf->fp);
    }
    if (tf->name) {
    	free(tf->name);
    }
    if (tf->tmpname) {
    	free(tf->tmpname);
    }
  #if 0
    if (tf->rgb) {
    	free(tf->rgb);
    }
  #endif
    if (tf->linebuf) {
    	free(tf->linebuf);
    }
    free(tf);
}

void TIF_CloseW(TIF *tf)
{
    if (tf->tmpname) {
    	fclose(tf->fp); tf->fp = NULL;
    	Rename_b(tf->tmpname,tf->name);
    }
    TIF_Free(tf);
}

static int TIF_ocnt = 0, TIF_id = 0;

static void TIF_InitPut(int tifID)
{
    TIF_ocnt = 0;
    TIF_id = tifID;
}

static void TIF_PutB(TIF* tf, int ch)
{
    fputc(ch & 0xff, tf->fp);
    ++TIF_ocnt;
}

static void TIF_PutW(TIF* tf, WORD w)
{
    if (TIF_id == 0) {
    	TIF_PutB(tf, w & 0xff);
    	TIF_PutB(tf, w >> 8);
    } else {
    	TIF_PutB(tf, w >> 8);
    	TIF_PutB(tf, w & 0xff);
    }
}

static void TIF_PutD(TIF* tf, DWORD d)
{
    if (TIF_id == 0) {
    	TIF_PutW(tf, (WORD)(d & 0xffff));
    	TIF_PutW(tf, (WORD)(d >> 16));
    } else {
    	TIF_PutW(tf, (WORD)(d >> 16));
    	TIF_PutW(tf, (WORD)(d & 0xffff));
    }
}

static void TIF_PutTag(TIF* tf, int tag, int typ, long cnt, DWORD dat)
{
    TIF_PutW(tf, tag);
    TIF_PutW(tf, typ);
    TIF_PutD(tf, cnt);
    switch (typ) {
    case TIF__BYTE:
    case TIF__ASCIIZ:
    	if (cnt == 1) {
    	    TIF_PutB(tf, (BYTE)dat&0xff);TIF_PutB(tf, 0);TIF_PutW(tf,0);
    	} else if (cnt == 2) {
    	    TIF_PutB(tf,(BYTE)(dat>>8)&0xff);TIF_PutB(tf,(BYTE)(dat)&0xff);
    	    TIF_PutW(tf,0);
    	} else if (cnt == 3) {
    	    TIF_PutB(tf,(BYTE)(dat>>16)&0xff);TIF_PutB(tf,(BYTE)(dat>>8)&0xff);
    	    TIF_PutB(tf,(BYTE)(dat)&0xff);TIF_PutB(tf,0);
    	} else if (cnt == 4 && typ != TIF__ASCIIZ) {
    	    TIF_PutB(tf,(BYTE)(dat>>24)&0xff);TIF_PutB(tf,(BYTE)(dat>>16)&255);
    	    TIF_PutB(tf,(BYTE)(dat>>8)&0xff);TIF_PutB(tf,(BYTE)dat&0xff);
    	} else {
    	    TIF_PutD(tf,dat);
    	}
    	break;
    case TIF__WORD:
    	if (cnt == 1) {
    	    TIF_PutW(tf,(WORD)dat&0xffff); TIF_PutW(tf, 0);
    	} else if (cnt == 2) {
    	    TIF_PutW(tf,(WORD)(dat>>16)&0xffff); TIF_PutW(tf,(WORD)dat&0xffff);
    	} else {
    	    TIF_PutD(tf,dat);
    	}
    	break;
    default:
    	TIF_PutD(tf, dat);
    }
}

TIF *TIF_Create(char *name, int colbit, int szX, int szY,
    	    	 long xresol, long yresol, BYTE *rgb, int tifID)
    /* tifID 0=II(ｲﾝﾃﾙ)  1=MM(ﾓﾄﾛｰﾗ) */
{
    TIF *tf;
    int i,townsFlg;

    tf = calloc_m(1,sizeof(TIF));
    if (tf == NULL) {
    	return NULL;
    }

    tf->name = strdup(name);
    tf->colbit= colbit;
    if (colbit == 4) {
      #if 1
    	if (szX & 0x01) {
    	    printf("16色TIFFで横幅が奇数なので誤変換します(横幅を強制的に偶数化)\n");
    	    szX++;
    	}
      #endif
    }
    tf->xsize = szX;
    tf->ysize = szY;
    tf->xresol = xresol;
    tf->yresol = yresol;
    tf->rgb   = rgb;

    tf->fp = TmpCreate(name, &tf->tmpname);
    if (tf->fp == NULL) {
    	goto ERR;
    }
    setvbuf(tf->fp,NULL, _IOFBF, WRTBUFSIZE);
    TIF_InitPut(tifID);

    if (tifID == 0) {
    	TIF_PutW(tf, 'I'*0x100+'I');
    } else {
    	TIF_PutW(tf, 'M'*0x100+'M');
    }
    TIF_PutW(tf, 42);
    TIF_PutD(tf, 8);

    townsFlg = 0;
    if (tifID == 0 && (colbit == 8 || colbit == 24)){
    	townsFlg = 1;
    }
    TIF_PutW( tf, townsFlg + 8 + 6 + ((colbit<24 && rgb) ? 1 : 0) );
    if (townsFlg) {
    	TIF_PutTag(tf, 0x0FE, TIF__DWORD, 1, 0);
    }

    if (colbit < 24) {
    	TIF_PutTag(tf, 0x100, TIF__WORD, 1, szX);   /* Image Width TAG*/
    	TIF_PutTag(tf, 0x101, TIF__WORD, 1, szY);   /* Image Length TAG*/
    	TIF_PutTag(tf, 0x102, TIF__WORD, 1, colbit);/*+ Bits Per Sample TAG*/
    	TIF_PutTag(tf, 0x103, TIF__WORD, 1, 1);     /* compression TAG*/
    	if (rgb == NULL) {
    	    TIF_PutTag(tf, 0x106, TIF__WORD, 1, 1); /*+ color */
    	} else {
    	    TIF_PutTag(tf, 0x106, TIF__WORD, 1, 3); /*+ palette */
    	}
    	TIF_PutTag(tf, 0x10A, TIF__WORD, 1, 1);     /* Fill Order. TAG*/
    	if (rgb == NULL) {  	    	    	    /* Photo.Interp. TAG*/
    	    TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x100); /*+ Strip Offset */
    	} else {
    	    if (colbit == 4) {
    	    	TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x200); /*+ Strip Offset*/
    	    } else {
    	    	TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x700); /*+ Strip Offset*/
    	    }
    	}
    	TIF_PutTag(tf, 0x115, TIF__WORD, 1, 1);     /*+ Sample Per Pixel TAG*/
    	TIF_PutTag(tf, 0x118, TIF__WORD,1,0);	    /*+MinSampleValue*/
    	TIF_PutTag(tf, 0x119, TIF__WORD,1,(0x01<<colbit)-1);/*+MaxSampleValue*/
    	TIF_PutTag(tf, 0x11A, TIF__RAT, 1, 0xF0);   /* X Resolution TAG*/
    	TIF_PutTag(tf, 0x11B, TIF__RAT, 1, 0xF8);   /* Y Resolution TAG*/
    	TIF_PutTag(tf, 0x11C, TIF__WORD, 1, 1);     /* Planer Config. TAG*/
    	TIF_PutTag(tf, 0x128, TIF__WORD, 1, 2);     /* Resolution Unit TAG*/
    	if (rgb && colbit <= 256) {
    	    int col;
    	    col = 0x01 << colbit;
    	    TIF_PutTag(tf, 0x140, TIF__WORD, col*3, 0x100); /*+ pal. ofs. */
    	}
    	TIF_PutD(tf, 0L);

    	if (TIF_ocnt > 0xf0) {
    	    printf("PRGERR:TIFF-HEADER\n");
    	}
    	for (i = TIF_ocnt; i < 0xF0; i++) {
    	    TIF_PutB(tf,0);
    	}
    	TIF_PutD(tf,75);TIF_PutD(tf,1);
    	TIF_PutD(tf,75);TIF_PutD(tf,1);
    	if (rgb) {
    	    if (colbit == 4) {
    	    	for (i = 0; i < 16*3; i+=3) {
    	    	    TIF_PutW(tf, rgb[i]*0x100+rgb[i]);
    	    	}
    	    	for (i = 0; i < 16*3; i+=3) {
    	    	    TIF_PutW(tf, rgb[i+1]*0x100+rgb[i+1]);
    	    	}
    	    	for (i = 0; i < 16*3; i+=3) {
    	    	    TIF_PutW(tf, rgb[i+2]*0x100+rgb[i+2]);
    	    	}
    	    	for (i = 16*2*3;i < 0x100;i++) {
    	    	    TIF_PutB(tf, 0x00);
    	    	}
    	    } else if (colbit == 8) {
    	    	for (i = 0; i < 256*3; i+=3) {
    	    	    TIF_PutW(tf, rgb[i]*0x100+rgb[i]);
    	    	}
    	    	for (i = 0; i < 256*3; i+=3) {
    	    	    TIF_PutW(tf, rgb[i+1]*0x100+rgb[i+1]);
    	    	}
    	    	for (i = 0; i < 256*3; i+=3) {
    	    	    TIF_PutW(tf, rgb[i+2]*0x100+rgb[i+2]);
    	    	}
    	    }
    	}
    	/* 変換用バッファ確保 */
    	tf->linebuf = calloc_m(tf->xsize, 1);
    	if (tf->linebuf == NULL) {
    	    goto ERR;
    	}
    } else {	/* full color */
    	TIF_PutTag(tf, 0x100, TIF__WORD, 1, szX);   /* Image Width TAG*/
    	TIF_PutTag(tf, 0x101, TIF__WORD, 1, szY);   /* Image Length TAG*/
    	TIF_PutTag(tf, 0x102, TIF__WORD, 3, 0xDE);  /*+ Bits Per Sample TAG*/
    	TIF_PutTag(tf, 0x103, TIF__WORD, 1, 1);     /* compression TAG*/
    	TIF_PutTag(tf, 0x106, TIF__WORD, 1, 2);     /*+ full color */
    	TIF_PutTag(tf, 0x10A, TIF__WORD, 1, 1);     /* Fill Order. TAG*/
    	TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x100);/* Strip Offset */
    	TIF_PutTag(tf, 0x115, TIF__WORD, 1, 3);     /*+ Sample Per Pixel TAG*/
    	TIF_PutTag(tf, 0x118, TIF__WORD, 3, 0xE4);  /*+MinSampleValue*/
    	TIF_PutTag(tf, 0x119, TIF__WORD, 3, 0xEA);  /*+ Max Sample Value */
    	TIF_PutTag(tf, 0x11A, TIF__RAT, 1, 0xF0);   /* X Resolution TAG*/
    	TIF_PutTag(tf, 0x11B, TIF__RAT, 1, 0xF8);   /* Y Resolution TAG*/
    	TIF_PutTag(tf, 0x11C, TIF__WORD, 1, 1);     /* Planer Config. TAG*/
    	TIF_PutTag(tf, 0x128, TIF__WORD, 1, 2);     /* Resolution Unit TAG*/
    	TIF_PutD(tf, 0L);

    	if (TIF_ocnt > 0xDE) {
    	    printf("PRGERR:TIFF-HEADER\n");
    	}
    	for (i = TIF_ocnt; i < 0xDE; i++) {
    	    TIF_PutB(tf,0);
    	}
    	TIF_PutW(tf,8);TIF_PutW(tf,8);TIF_PutW(tf,8);	/* Bits Per Sample */
    	TIF_PutW(tf,0x00);TIF_PutW(tf,0x00);TIF_PutW(tf,0x00);/* Min.Smpl.Val*/
    	TIF_PutW(tf,0xFF);TIF_PutW(tf,0xFF);TIF_PutW(tf,0xFF);/* Max.Smpl.Val*/
    	TIF_PutD(tf,xresol);TIF_PutD(tf,1); 	    	/* X Resolution */
    	TIF_PutD(tf,yresol);TIF_PutD(tf,1); 	    	/* Y Resolution */
    	/* 変換用バッファ確保 */
    	tf->linebuf = calloc_m(tf->xsize, 3);
    	if (tf->linebuf == NULL) {
    	    goto ERR;
    	}
    }
    return tf;

  ERR:
    TIF_Free(tf);
    return NULL;
}

/*------------------------*/

void TIF_PutLine(TIF *tf, PIXEL *buf)
    /* PIC_PutLines から呼ばれることになる関数 */
{
    int i;
    PIXEL col;
    BYTE *p;

    p = tf->linebuf;
    if (tf->colbit == 4) {
    	for (i = 0; i < tf->xsize; i+=2) {
    	    *p++ = (BYTE)( ((buf[i] >> 20)&0xF0) | ((buf[i+1] >> 24)&0x0f) );
    	}
    	fwrite_e(tf->linebuf, 1, tf->xsize / 2, tf->fp);
    } else if (tf->colbit <= 8) {
    	for (i = 0; i < tf->xsize; i++) {
    	    *p++ = (BYTE)(buf[i] >> 24);
    	}
    	fwrite_e(tf->linebuf, 1, tf->xsize, tf->fp);
    } else {
    	for (i = 0; i < tf->xsize; i++) {
    	    col = buf[i];
    	    *p++ = (BYTE)((col	    ) & 0xFF);	/* R */
    	    *p++ = (BYTE)((col >>  8) & 0xFF);	/* G */
    	    *p++ = (BYTE)((col >> 16) & 0xFF);	/* B */
    	}
    	fwrite_e(tf->linebuf, 3, tf->xsize, tf->fp);
    }
}

/*end TIF*/

/*---------------------------------------------------------------------------*/
/*  	    	    	    	B M P 出 力 	    	    	    	     */
/*---------------------------------------------------------------------------*/
/* module BMP */
/* export BMP, BMP_Create, BMP_CloseW, BMP_PutLine */

/*
    整数の格納方法はインテル方式

 ｻｲｽﾞ:型 ｵﾌｾｯﾄ	内容
    	    //ファイル・ヘッダ部
    2 w +0  ID	0x42,0x4d('BM')
    4 d +2  ファイル・サイズ
    4	+6  ＜予約＞
    4 d +10 実際のデータへのオフセット

    	    //情報ヘッダ部（Windows版）
    4 d +14 情報ヘッダのサイズ（現状は 40）
    4 d +18 横幅（ドット数）
    4 d +22 縦幅（ドット数）
    2 w +26 プレーン数(=1)
    2 w +28 1ﾋﾟｸｾﾙのﾋﾞｯﾄ数 1:mono 4:16色 8:256色 24:ﾌﾙｶﾗｰ
    4 d +30 圧縮モード	 0:無圧縮   1:RLE8方式	2:RLE4方式
    4 d +34 圧縮時の画像データ本体のサイズ：無圧縮時０
    4 d +38 １メートル当たりの横ドット数
    4 d +42 １メートル当たりの縦ドット数
    4 d +46 色の数（カラーテーブルの数）。1->2,4->16,8->256,24->0(ﾊﾟﾚｯﾄ無)
    4 d +50 重要な色の数？
  （4 d +54 ﾌﾙｶﾗｰ以外のとき、パレット・データ  色数(2,16,256)*4
    	    B G R Aの順  Aは予約(ｱﾄﾋﾞｭﾚｰﾄ?）	    	    	）

    	:
    	:
    	    ピクセル・データ（無圧縮フルカラー：B G R の繰り返し）
*/

typedef struct BMP_T {
    FILE *fp;
    char *name;
    char *tmpname;
    int  colbit;
    int  xsize,ysize;
    int  xstart,ystart;
    long xresol, yresol;
    BYTE *rgb;
/* private: */
    BYTE *linebuf;
    int  lineSize;
    long pdataOfs;
} BMP;

#define BMPHDR_SIZE (14+40)

static void BMP_Free(BMP *bp)
{
    if (bp->fp) {
    	fclose(bp->fp);
    }
    if (bp->name) {
    	free(bp->name);
    }
    if (bp->tmpname) {
    	free(bp->tmpname);
    }
  #if 0
    if (bp->rgb) {
    	free(bp->rgb);
    }
  #endif
    if (bp->linebuf) {
    	free(bp->linebuf);
    }
    free(bp);
}

void BMP_CloseW(BMP *bp)
{
    if (bp->tmpname) {
    	fclose(bp->fp); bp->fp = NULL;
    	Rename_b(bp->tmpname, bp->name);
    }
    BMP_Free(bp);
}

static void BMP_PutB(BMP* bp, int ch)
{
    fputc((char)ch, bp->fp);
}

static void BMP_PutW(BMP* bp, WORD w)
{
    BMP_PutB(bp, w & 0xff);
    BMP_PutB(bp, w >> 8);
}

static void BMP_PutD(BMP* bp, DWORD d)
{
    BMP_PutW(bp, (WORD)(d & 0xffff));
    BMP_PutW(bp, (WORD)(d >> 16));
}

BMP *BMP_Create(char *name, int colbit, int xsize, int ysize,
    	    	long xresol, long yresol, BYTE *rgb)
{
    BMP *bp;
    int col,i, lineSize;

    bp = calloc_m(1,sizeof(BMP));
    if (bp == NULL) {
    	return NULL;
    }

    if (colbit == 4) {
    	lineSize = (xsize+1) >> 1;
    } else if (colbit == 8) {
    	lineSize = xsize;
    } else {
    	lineSize = xsize * 3;
    }
    if (lineSize % 4) {
    	lineSize >>= 2;
    	lineSize++;
    	lineSize <<= 2;
    }
    bp->lineSize = lineSize;

    bp->xsize = xsize;
    bp->ysize = ysize;
    bp->xresol = xresol;
    bp->yresol = yresol;
    bp->colbit = colbit;
    bp->name = strdup(name);
    bp->rgb  = rgb;
    if (colbit <= 8) {
    	col = 0x0001 << colbit;
    } else {
    	col = 0;
    }
    bp->pdataOfs = BMPHDR_SIZE + col * 4;

    /* 変換用バッファ確保 */
    bp->linebuf = calloc_m(1, lineSize);
    if (bp->linebuf == NULL) {
    	goto ERR;
    }

    /* create bmp file */
    bp->fp = TmpCreate(name, &bp->tmpname);
    if (bp->fp == NULL) {
    	goto ERR;
    }
    setvbuf(bp->fp,NULL, _IOFBF, WRTBUFSIZE);

    BMP_PutB(bp, 'B');	BMP_PutB(bp, 'M');  	    	    	/* ID	     */
    BMP_PutD(bp, (long)lineSize * ysize + bp->pdataOfs);    	/* file size */
    BMP_PutD(bp, 0L);	    	    	    	    	    	/* rsv	     */
    BMP_PutD(bp, bp->pdataOfs);     	    	    	    	/* pdata ofs */
    BMP_PutD(bp, 40L);	    	    	    	    	    	/* hdr size  */
    BMP_PutD(bp, xsize);    	    	    	    	    	/* x size    */
    BMP_PutD(bp, ysize);    	    	    	    	    	/* y size    */
    BMP_PutW(bp, 1);	    	    	    	    	    	/* plane     */
    BMP_PutW(bp, colbit);   	    	    	    	    	/* colorBits */
    BMP_PutD(bp, 0L);	    	    	    	    	    	/* comp Mode */
    BMP_PutD(bp, 0L);	    	    	    	    	    	/* compPdatSz*/
    BMP_PutD(bp, xresol);   	    	    	    	    	/* x resolu. */
    BMP_PutD(bp, yresol);   	    	    	    	    	/* y resolu. */
    BMP_PutD(bp, col);	    	    	    	    	    	/* paletteCnt*/
    BMP_PutD(bp, col);	    	    	    	    	    	/* col. impo.*/
    /* palette */
    if (col) {
    	for (i = 0; i < col; i++) {
    	    BMP_PutB(bp, rgb[i*3+2]);	/* B */
    	    BMP_PutB(bp, rgb[i*3+1]);	/* G */
    	    BMP_PutB(bp, rgb[i*3+0]);	/* R */
    	    BMP_PutB(bp, 0);	    	/* A */
    	}
    }
    fflush(bp->fp);
    if (fseek(bp->fp, bp->pdataOfs + (long)lineSize * ysize, SEEK_SET)) {
    	printf("BMP出力で書込みエラー\n");
    	goto ERR;
    }
    fseek(bp->fp, -lineSize, SEEK_CUR);
    return bp;

  ERR:
    BMP_Free(bp);
    return NULL;
}

/*------------------------------*/

void BMP_PutLine(BMP *bp, PIXEL *buf)
    /* PIC_PutLines から呼ばれることになる関数 */
{
    int i;
    PIXEL col;
    BYTE *p;

    p = bp->linebuf;
    if (bp->colbit == 4) {
    	for (i = 0; i < (bp->xsize); i+=2) {
    	    *p++ = (BYTE)( ((buf[i] >> 20)&0xF0) | ((buf[i+1] >> 24)&0x0f) );
    	}
    } else if (bp->colbit <= 8) {
    	for (i = 0; i < bp->xsize; i++) {
    	    *p++ = (BYTE)(buf[i] >> 24);
    	}
    } else {
    	for (i = 0; i < bp->xsize; i++) {
    	    col = buf[i];
    	    *p++ = (BYTE)((col >> 16) & 0xFF);	/* B */
    	    *p++ = (BYTE)((col >>  8) & 0xFF);	/* G */
    	    *p++ = (BYTE)((col	    ) & 0xFF);	/* R */
    	}
    }
    fwrite_e(bp->linebuf, 1, bp->lineSize, bp->fp);
    fseek(bp->fp, -(bp->lineSize*2), SEEK_CUR);
}

/* end BMP */

/*---------------------------------------------------------------------------*/
/*  	    	    	    	R G B (Q0) 出 力    	    	    	     */
/*---------------------------------------------------------------------------*/
/*module Q0*/
/* export Q0, Q0_Create, Q0_CloseW, Q0_PutLine */

typedef struct Q0_T {
    FILE *fp;
    char *name;
    char *tmpname;
    int  colbit;
    int  xsize,ysize;
    int  xstart,ystart;
    int  asp1, asp2;
    BYTE *rgb;
/* private: */
    BYTE *linebuf;
} Q0;

static void Q0_Free(Q0 *q0p)
{
    if (q0p->fp) {
    	fclose (q0p->fp);
    }
    if (q0p->name) {
    	free(q0p->name);
    }
    if (q0p->tmpname) {
    	free(q0p->tmpname);
    }
    if (q0p->linebuf) {
    	free(q0p->linebuf);
    }
    free(q0p);
}

void Q0_CloseW(Q0 *q0p)
{
    if (q0p->tmpname) {
    	fclose(q0p->fp); q0p->fp = NULL;
    	Rename_b(q0p->tmpname, q0p->name);
    }
    Q0_Free(q0p);
}

Q0 *Q0_Create(char *name, int colbit, int szX, int szY, int xstart,int ystart,
    	    	int asp1, int asp2, BYTE *rgb, int iprFlg)
{
    FILE *fp;
    char buf[FNAMESIZE+2];
    Q0 *q0p;

    q0p = calloc_m(1,sizeof(Q0));
    if (q0p == NULL) {
    	return NULL;
    }
    q0p->colbit= colbit;
    q0p->xsize = szX;
    q0p->ysize = szY;
    q0p->xstart = q0p->xstart;
    q0p->ystart = q0p->ystart;
    q0p->asp1  = asp1;
    q0p->asp2  = asp2;
    q0p->rgb   = rgb;

    /*falファイル出力 */
    strcpy(buf,name);
    if (iprFlg == 0) {
    	FIL_ChgExt(buf,"FAL");
    } else {
    	FIL_ChgExt(buf,"IPR");
    }
    fp = fopen_m(buf,"w");
    if (fp == NULL) {
    	goto ERR;
    }
    if (iprFlg == 0) {
    	fprintf(fp,"F_ALL(V1.00)\n");
    	fprintf(fp,"%d\t%d\t%d\t%d\n",szX,szY,xstart,ystart);
    	fprintf(fp,"1.00\t255\t255\t255\t0\n");
    } else {
    	fprintf(fp,"  %d  %d  %d  %d\n",szX,szY,xstart,ystart);
    }
    fclose(fp);

    /*q0ファイル作成オープン*/
    q0p->name = strdup(name);
    q0p->fp = TmpCreate(name, &q0p->tmpname);
    if (q0p->fp == NULL) {
    	goto ERR;
    }
    setvbuf(q0p->fp,NULL, _IOFBF, WRTBUFSIZE);
    q0p->linebuf = calloc_m(q0p->xsize,3);
    if (q0p->linebuf == NULL) {
    	goto ERR;
    }
    return q0p;

  ERR:
    Q0_Free(q0p);
    return NULL;
}

/*----------------------------------*/

void Q0_PutLine(Q0 *q0p, PIXEL *buf)
    /* PIC_PutLines から呼ばれることになる関数 */
{
    int i;
    PIXEL col;
    BYTE *p;

    p = q0p->linebuf;
    for (i = 0; i < q0p->xsize; i++) {
    	col = buf[i];
    	*p++ = (BYTE)((col  	) & 0xFF);  /* R */
    	*p++ = (BYTE)((col >>  8) & 0xFF);  /* G */
    	*p++ = (BYTE)((col >> 16) & 0xFF);  /* B */
    }
    fwrite_e(q0p->linebuf, 3, q0p->xsize, q0p->fp);
}

/*end Q0*/

#if 1
/*---------------------------------------------------------------------------*/
/*  	    	    	    	PMT 	    	    	    	    	     */
/*---------------------------------------------------------------------------*/

/*  PMT フォーマット

  +++
　　拙作 mg の前身のmag2pi,dopisv等のさらに前身に pmt2pi というのがありま
　して、主にpi,magを基準にヘッダを劣化させない作業ファイルを用意してそれ
　を介して各種ｺﾝﾊﾞｰﾄを行なおうと考えて作りかけたのですが, ま、実際には
　速度のこともあるしルーチン揃っちゃうと作業ﾌｧｲﾙ用いる方がめんどくさいこと
　もあって現在の mg.exe のようになり、自作作業ﾌｧｲﾙpmtはおくらいりしました.

    しかし mg の拡張は容易でなく、他のフォーマットを変換しようとすると
  やっぱし作業ファイルがほしいもので、もちろんそのためもあって
  tif,bmp,djp 等に対応していますが、どうしても 始点 と 透明色 も一緒に
  含んだ作業ファイルが必要になり、pmtを復活されることにしました（といいつつ、
  以前のﾌｫｰﾏｯﾄとはまったく別ものですが... 単に名前が気に入っていた、と:-）

  　必要が生じたときに作ったものは、実のとこ今のヘッダの先頭 16 バイトのみ
  だったのですが、公開版に含めるに当り、PICld,PICsv と mg.exe との併用
  を考えて、このようになりました。

    他の（自作とか:-）フォーマットのコンバータを作ってmgやpicld&picsvを
  利用しようって方、利用してやってくださいませ（爆）


　+++
    pmt のヘッダ構造は、構造体 PMT をみてください。

    ヘッダ ID は 'Pm' です。

    colbit の値により、２種類の方式にわかれます。
    ８以下ならば 8ビット(256)色画像（１ドット１バイト）、それ以上ならば 24
　ビット色（１ドット３バイト）画像となります.

    4ビット色だからといって１バイト２ドットにするとか、15,16ビット色なら
  １ドット２バイトで、というような処理はしません。

    なお、colbit は 1～8(,12),15,16(,18),24が指定できます。
　colbit=16は18ﾋﾞｯﾄ色扱いで、変換元が x68やVA の 16ﾋﾞｯﾄ色だった場合を想定し
　ています。PIC生成では 5+5+5+1ﾋﾞｯﾄで +1 ﾋﾞｯﾄはGの上から6ﾋﾞｯﾄ目を使います。

    8ﾋﾞｯﾄ色以下ならばパレットは必ず 3 * 256 バイト有ります.
    24ビット色画像ではパレットは有りません。
    これも colbit=4のとき 16(4ﾋﾞｯﾄ)色画像だからといって 3*16となるわけでは
  ありません。

　　透明色ですが、8ﾋﾞｯﾄ色以下のとき意味があり、透明色を使わないのならば
　bcol = 0で、透明色があるばあい、bcol には透明色番号に１足した値を設定
　します。24ﾋﾞｯﾄ色ではどうするか未定でたえず０にします。

    ピクセル・データは、べた（無圧縮）です。8ﾋﾞｯﾄ色画像ならば１ピクセル
　１バイト、24ﾋﾞｯﾄ色画像ならば、１ピクセル３バイト(R,G,B順)です。
　　データの配置は横幅にかかわらず詰めます。奇数個なら奇数個、そのままです。
  BMPのように横４バイト単位になるようにパティングするとかは無しです。

    コメントは、ピクセルデータの直後に付きます。ヘッダには commentOfs に
　ファイル先頭からのコメント先頭へのオフセット値を設定しておき、読み込むと
　きは commentOfs を用いて読み込みます。
　　なお、コメントの終りは 0x00 です。必ずあるものとして扱ってます。

　  作者名は18バイト以内で、18バイトに満たないときは 後ろを 0で埋めます.
  ゴミは残さないように、と。

　　ファイルの拡張子についてですが、基本的にpmtとしてます。
　　以前の版(v0.80)では、8ﾋﾞｯﾄ色のときは .pmt、24ビット色のときは.pmy
　としてましたが、分ける必要がなさそなので pmy は無しということにして
  24ビット色でも .pmtです.

*/

typedef struct PMT {	    	/* PMT ヘッダ --------------------------*/
    WORD    	    id;     	/* ID = 'Pm'	    	    	    	*/
    BYTE    	    colbit; 	/* 1ﾄﾞｯﾄ当りのﾋﾞｯﾄ数.	    	    	*/
    	    	    	    	/* 8以下:8ﾋﾞｯﾄ色扱い 9以上:24ﾋﾞｯﾄ色扱い */
    BYTE    	    flags;  	/* フラグ:今のとこ 0	    	    	*/
    short   	    xsize;  	/* 横幅 (ドット単位)	    	    	*/
    short   	    ysize;  	/* 縦幅 (ドット単位)	    	    	*/
    short   	    x0;     	/* 始点 (ドット単位) -1だと始点無し 	*/
    short   	    y0;     	/* 始点 (ドット単位) -1だと始点無し 	*/
    WORD    	    bcol;   	/* 0:無 1～256:透明色番号+1 	    	*/
    BYTE    	    rsv[2]; 	/* 今のとこ 0	    	    	    	*/
    short   	    xasp;   	/* 横アスペクト     	    	    	*/
    short   	    yasp;   	/* 縦アスペクト     	    	    	*/
    DWORD   	    commentOfs; /* ｺﾒﾝﾄまでのﾌｧｲﾙ先頭からのｵﾌｾｯﾄ    	*/
    	    	    	    	/* なおｺﾒﾝﾄの終端文字は\0   	    	*/
    char    	    artist[18]; /* 作者名. 後ろは \0 で埋める.	    	*/
    char    	    artistTerm; /* 作者名が18ﾊﾞｲﾄきっちりのときのための終端\0*/
    BYTE    	    timeSec1;	/* bit7:time に入り切らない秒の下１ﾋﾞｯﾄ */
    	    	    	    	/* 普通DOSは 0 で問題無し.  	    	*/
    WORD    	    time;   	/* MS-DOSファイルの時間     	    	*/
    WORD    	    date;   	/* MS-DOSファイルの日付     	    	*/
    	    	    	    	/* date=0のとき日付日時は無効	    	*/
    BYTE    	    rsv2[16];	/* 予約. 今のとこ all 0     	    	*/
    BYTE    	    rgb[256*3]; /* パレット (r,g,b)*256 r,g,bは各8ﾋﾞｯﾄ	*/
    	    	    	    	/* colbit<=8のときのみ. たえず 256個有	*/
    	    	    	    	/* colbit>8の24ﾋﾞｯﾄ色扱いではﾊﾟﾚｯﾄ無し	*/
    /*------- 以上 64(+256*3)バイトが PMTのヘッダ ----------------------*/
    FILE    	    *fp;
    BYTE    	    *linebuf;
    char    	    *comment;
} PMT;


static void PMT_Free(PMT *pm)
{
    if (pm->fp) {
    	fclose(pm->fp);
    }
    if (pm->linebuf) {
    	free(pm->linebuf);
    }
    free(pm);

}

void PMT_Close(PMT *pm)
{
    /* コメントがあるとき */
    if (pm->comment && strlen(pm->comment)) {
    	fwrite(pm->comment, 1, strlen(pm->comment)+1, pm->fp);
    }
    PMT_Free(pm);
}

void PMT_WrtWd(void *w, WORD n)
{
    ((BYTE *)w)[0] = (BYTE)n;
    ((BYTE *)w)[1] = (BYTE)(n >> 8);
}

void PMT_WrtDw(void *w, DWORD n)
{
    ((BYTE *)w)[0] = (BYTE)n;
    ((BYTE *)w)[1] = (BYTE)(n >> 8);
    ((BYTE *)w)[2] = (BYTE)(n >> 16);
    ((BYTE *)w)[3] = (BYTE)(n >> 24);
}

PMT *PMT_Create(char *name, int colbit, int szX, int szY,int x0,int y0,
    	    	int xasp, int yasp, int bc,
    	    	BYTE *rgb, char *comment,int fflg, char *artist
    	      #ifdef FDATEGET
    	    	,WORD ftim, WORD fdat
    	      #endif
){
    PMT *pm;

    pm = calloc_m(1,sizeof(PMT));
    if (pm == NULL) {
    	return NULL;
    }

    if (fflg == 0 && colbit <= 8) {
    	pm->colbit = colbit;
    	memcpy(pm->rgb, rgb, 256*3);
    	/* 変換用バッファ確保 */
    	pm->linebuf = calloc_m(szX + 4, 1);
    	if (pm->linebuf == NULL) {
    	    goto ERR;
    	}
    } else {
    	if (colbit <= 8) {
    	    pm->colbit = 24;
    	} else {
    	    pm->colbit = colbit;
    	}
    	rgb = NULL;
    	/* 変換用バッファ確保 */
    	pm->linebuf = calloc_m(szX*3 + 4, 1);
    	if (pm->linebuf == NULL) {
    	    goto ERR;
    	}
    }
    /*pm->flags = 0;*/	/* フラグ ... callocしているから初期化ずみ^^;*/

    PMT_WrtWd(&pm->id, 'P' + 'm'*0x100);
    PMT_WrtWd(&pm->xsize, szX);
    PMT_WrtWd(&pm->ysize, szY);
    PMT_WrtWd(&pm->x0,	x0);
    PMT_WrtWd(&pm->y0,	y0);
    PMT_WrtWd(&pm->bcol,bc);
  #ifdef FDATEGET
    PMT_WrtWd(&pm->date, fdat);
    PMT_WrtWd(&pm->time, ftim);
  #endif
    PMT_WrtWd(&pm->xasp, xasp);
    PMT_WrtWd(&pm->yasp, yasp);

    /* 作者名 */
    if (artist && *artist) {
    	strncpy(pm->artist, artist, 18);
    }
    /*pm->artistTerm = 0;*/ /* callocしているから初期化ずみ^^;*/

    /* コメントがあるとき */
    if (comment && strlen(comment)) {
    	pm->comment = comment;
    	if (pm->colbit <= 8) {
    	    pm->commentOfs = 64 + 256*3 + (DWORD)szY * (DWORD)szX;
    	} else {
    	    pm->commentOfs = 64 + (DWORD)szY * (DWORD)szX * 3L;
    	}
    	PMT_WrtDw(&pm->commentOfs, pm->commentOfs);
    }

    pm->fp = fopen_m(name, "wb");
    if (pm->fp == NULL) {
    	goto ERR;
    }
    setvbuf(pm->fp,NULL, _IOFBF, WRTBUFSIZE);

    if (rgb) {
    	fwrite(pm,1,64+256*3,pm->fp);
    } else {
    	fwrite(pm,1,64,pm->fp);
    }

    return pm;

  ERR:
    PMT_Free(pm);
    return NULL;
}

void PMT_PutLine(PMT *pm, PIXEL *buf)
    /* PIC_PutLines から呼ばれることになる関数 */
{
    int i;
    PIXEL col;
    BYTE *p;

    p = pm->linebuf;
    if (pm->colbit <= 8) {
    	for (i = 0; i < pm->xsize; i++) {
    	    *p++ = (BYTE)(buf[i] >> 24);
    	}
    	fwrite_e(pm->linebuf, 1, pm->xsize, pm->fp);
    } else {
    	for (i = 0; i < pm->xsize; i++) {
    	    col = buf[i];
    	    *p++ = (BYTE)((col	    ) & 0xFF);	/* R */
    	    *p++ = (BYTE)((col >>  8) & 0xFF);	/* G */
    	    *p++ = (BYTE)((col >> 16) & 0xFF);	/* B */
    	}
    	fwrite_e(pm->linebuf, 3, pm->xsize, pm->fp);
    }
}

/*end PMT */
#endif


#if 1
/*---------------------------------------------------------------------------*/
/*  	    	    	    	DJP 	    	    	    	    	     */
/*---------------------------------------------------------------------------*/

typedef struct DJP {	    	/* DJP ヘッダ --------------------------*/
    BYTE    	    id[6];  	/* ID = 'Pm'	    	    	    	*/
    short   	    xsize;  	/* 横幅 (ドット単位)	    	    	*/
    short   	    ysize;  	/* 縦幅 (ドット単位)	    	    	*/
    short   	    colbit; 	/* 0:ﾊﾟﾚｯﾄ付256色 8:ﾊﾟﾚｯﾄ無256色 24:ﾌﾙｶﾗｰ*/

    BYTE    	    rgb[256*3]; /* パレット (r,g,b)*256 r,g,bは各8ﾋﾞｯﾄ	*/
    FILE    	    *fp;
    BYTE    	    *linebuf;
} DJP;


static void DJP_Free(DJP *dj)
{
    if (dj->fp) {
    	fclose(dj->fp);
    }
    if (dj->linebuf) {
    	free(dj->linebuf);
    }
    free(dj);

}

void DJP_Close(DJP *dj)
{
    DJP_Free(dj);
}

void DJP_WrtWd(void *w, WORD n)
{
    ((BYTE *)w)[0] = (BYTE)n;
    ((BYTE *)w)[1] = (BYTE)(n >> 8);
}

DJP *DJP_Create(char *name, int colbit, int szX, int szY, BYTE *rgb)
{
    DJP *dj;

    dj = calloc_m(1,sizeof(DJP));
    if (dj == NULL) {
    	return NULL;
    }

    if (colbit <= 8) {
    	dj->colbit = 0;
    	memcpy(dj->rgb, rgb, 256*3);
    	/* 変換用バッファ確保 */
    	dj->linebuf = calloc_m(szX + 4, 1);
    	if (dj->linebuf == NULL) {
    	    goto ERR;
    	}
    } else {
    	dj->colbit = 24;
    	rgb = NULL;
    	/* 変換用バッファ確保 */
    	dj->linebuf = calloc_m(szX*3 + 4, 1);
    	if (dj->linebuf == NULL) {
    	    goto ERR;
    	}
    }
    memcpy(dj->id, "DJ505J", 6);
    DJP_WrtWd(&dj->xsize, szX);
    DJP_WrtWd(&dj->ysize, szY);
    DJP_WrtWd(&dj->colbit, dj->colbit);

    dj->fp = fopen_m(name, "wb");
    if (dj->fp == NULL) {
    	goto ERR;
    }
    setvbuf(dj->fp,NULL, _IOFBF, WRTBUFSIZE);

    if (rgb) {
    	fwrite(dj,1,12+256*3,dj->fp);
    } else {
    	fwrite(dj,1,12,dj->fp);
    }

    return dj;

  ERR:
    DJP_Free(dj);
    return NULL;
}

void DJP_PutLine(DJP *dj, PIXEL *buf)
    /* PIC_PutLines から呼ばれることになる関数 */
{
    int i;
    PIXEL col;
    BYTE *p;

    p = dj->linebuf;
    if (dj->colbit == 0) {
    	for (i = 0; i < dj->xsize; i++) {
    	    *p++ = (BYTE)(buf[i] >> 24);
    	}
    	fwrite_e(dj->linebuf, 1, dj->xsize, dj->fp);
    } else {
    	for (i = 0; i < dj->xsize; i++) {
    	    col = buf[i];
    	    *p++ = (BYTE)((col	    ) & 0xFF);	/* R */
    	    *p++ = (BYTE)((col >>  8) & 0xFF);	/* G */
    	    *p++ = (BYTE)((col >> 16) & 0xFF);	/* B */
    	}
    	fwrite_e(dj->linebuf, 3, dj->xsize, dj->fp);
    }
}

/*end DJP */
#endif


/*---------------------------------------------------------------------------*/
/*  	    	    	    	PC9801表示  	    	    	    	     */
/*---------------------------------------------------------------------------*/
#ifdef PC98
    /* PC9801表示はオマケです... ディザのﾁｪｯｸのために付加したものなので */
    /* ローダとしての性能を求めないでね^^;  	    	    	    	*/
    /* DOS汎用ｺﾝﾊﾞｰﾀといいつつコンナモン付けるのは邪道ですね^^;     	*/

#include <dos.h>
#include <conio.h>  	/* getch()のため */

/*--------------------------------------*/
/* module RPAL */
    /* export RPAL_SetRGB, RPAL_ChgTone */

struct RPAL_T {
    BYTE id[10];
    BYTE tone;
    BYTE rsv[5];
    BYTE grb[16][3];
};

struct MCB_T {
    BYTE  flag;
    WORD owner;
    WORD blksiz;
    BYTE reserve[11];
};

static struct MCB_T far *_dos_mcbfirst(void)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.ah = 0x52;
    intdosx(&reg,&reg,&sreg);
    return MK_FP(*(WORD far *)MK_FP(sreg.es, reg.x.bx - 2), 0);
}

static int far_strcmp(char far *s1, char far *s2)
{
    while (*s1 == *s2) {
    	if (*s1 == '\0') {
    	    return 0;
    	} else {
    	    ++s1;
    	    ++s2;
    	}
    }
    if (*s1 > *s2) {
    	return 1;
    } else {
    	return -1;
    }
}

static struct RPAL_T far *RPAL_Search(void)
{
    struct MCB_T far *p;
    WORD seg;

    p = _dos_mcbfirst();
    seg = FP_SEG(p);
    for (;;) {
    	if (p->owner != NULL) {
    	    if (far_strcmp((char far *)p + 0x10, "pal98 grb") == 0) {
    	    	return (struct RPAL_T far *)((BYTE far *)p + 0x10);
    	    }
    	}
    	if (p->flag == 'Z') {
    	    break;
    	}
    	seg += p->blksiz + 1;
    	p = MK_FP(seg,0);
    }
    return NULL;
}

void RPAL_ChgTone(int tone)
{
    struct RPAL_T far *p;
    int  i;

    if ((p = RPAL_Search()) == NULL) {
    	return;
    }
    p->tone = tone;
    for (i = 0; i < 16; ++i) {
    	int r,g,b;
    	g = p->grb[i][0] * tone / 100; g = (g > 15) ? 15 : g;
    	r = p->grb[i][1] * tone / 100; r = (r > 15) ? 15 : r;
    	b = p->grb[i][2] * tone / 100; b = (b > 15) ? 15 : b;
    	outp(0xa8, i);
    	outp(0xaa, g);
    	outp(0xac, r);
    	outp(0xae, b);
    }
}

void RPAL_SetRGB(BYTE *rgb, int tone)
{
    struct RPAL_T far *p;
    int  i;

    if ((p = RPAL_Search()) != NULL) {
    	p->tone = tone;
    	for (i = 0; i < 16; ++i) {
    	    p->grb[i][0] = rgb[i*3 + 1]>>4;
    	    p->grb[i][1] = rgb[i*3 + 0]>>4;
    	    p->grb[i][2] = rgb[i*3 + 2]>>4;
    	}
    }
    for (i = 0; i < 16; ++i) {
    	int r,g,b;
    	r = (rgb[i*3 + 0]>>4) * tone / 100; r = (r > 15) ? 15 : r;
    	g = (rgb[i*3 + 1]>>4) * tone / 100; g = (g > 15) ? 15 : g;
    	b = (rgb[i*3 + 2]>>4) * tone / 100; b = (b > 15) ? 15 : b;
    	outp(0xa8, i);
    	outp(0xaa, g);
    	outp(0xac, r);
    	outp(0xae, b);
    }
}

#if 0
int RPal_Get(BYTE *rgb, int *tone)
{
    struct RPAL_T far *p;
    int  i;

    if ((p = RPAL_Search()) == NULL) {
    	return -1;
    }
    if (tone) {
    	*tone = p->tone;
    }
    for (i = 0; i < 16; ++i) {
    	rgb[i * 3 + 0] = p->grb[i][1];
    	rgb[i * 3 + 1] = p->grb[i][0];
    	rgb[i * 3 + 2] = p->grb[i][2];
    }
    return 0;
}
#endif

/*end RPAL*/

/*---------------------------------------*/
/*module D98*/
/*export D98, D98_Create, D98_Close, D98_PutLine, D98_KeyMode */

typedef struct D98_T {
    FILE *fp;
    char *name;
    int  colbit;
    int  xsize,ysize;
    int  xstart,ystart;
    BYTE *rgb;
/* private: */
    int  curY;
    int  maxX;
    BYTE far *bp, far *rp, far *gp, far *ep;
    int  actPage;
} D98;

static int D98_keyFlg = 0;

void D98_KeyMode(int n)
{
    D98_keyFlg = n;
}

static void D98_Free(D98 *d98)
{
    if (d98->name) {
    	free(d98->name);
    }
    free(d98);
}

static void D98_GrphPage(int n)
    /* グラフィック画面の描写&表示ヘージの選択 */
{
    union REGS regs;

    outp(0xa6,n);   /* active-page 1 */
    regs.h.ah = 0x42;
    if (n == 0) {
    	regs.h.ch = 0xC0;
    } else {
    	regs.h.ch = 0xD0;
    }
    int86( 0x18, &regs, &regs); /* 400line. 16color. display-page 0*/
    regs.h.ah = 0x40;
    int86( 0x18, &regs, &regs);
}

static void D98_TextSw(int n)
    /* テキスト表示の on(1)/off(0) */
{
    union REGS regs;
    if (n) {
    	regs.h.ah = 0x0C;
    } else {
    	regs.h.ah = 0x0D;
    }
    int86( 0x18, &regs, &regs);
}

void D98_CloseW(D98 *d98)
{
    D98_Free(d98);
    D98_GrphPage(0);
    if (D98_keyFlg) {
    	int c,page,txtSw;
    	page = txtSw = 0;
    	D98_TextSw(0);
    	for (;;) {
    	    c = getch();
    	    switch(c) {
    	    case 0x1b:
    	    	D98_GrphPage(0);
    	    	RPAL_ChgTone(50);
    	    	D98_TextSw(1);
    	    	exit(1);
    	    case 0x0d:
    	    	D98_GrphPage(0);
    	    	RPAL_ChgTone(50);
    	    	D98_TextSw(1);
    	    	return;
    	    case '\t':
    	    case ' ':
    	    	page = (page + 1) & 0x01;
    	    	D98_GrphPage(page);
    	    	break;
    	    case 'T':
    	    case 't':
    	    	txtSw = (txtSw + 1) & 0x01;
    	    	D98_TextSw(txtSw);
    	    	break;
    	    }
    	}
    }
}

D98 *D98_Create(char *name, int colbit, int xsize, int ysize, BYTE *rgb)
{
    D98 *d98;

    d98 = calloc_m(1,sizeof(D98));
    if (d98 == NULL) {
    	return NULL;
    }
    d98->maxX = d98->xsize = xsize;
    d98->ysize = ysize;
    d98->colbit = colbit;
    d98->name = strdup(name);
    d98->rgb  = rgb;
    if (d98->maxX > 640) {
    	d98->maxX = 640;
    }
    d98->bp = (BYTE far *)0xA8000000L;
    d98->rp = (BYTE far *)0xB0000000L;
    d98->gp = (BYTE far *)0xB8000000L;
    d98->ep = (BYTE far *)0xE0000000L;
    d98->curY = 0;

    /* */
    D98_GrphPage(0);	/* 400line. 16color. display-page 0*/
    outp( 0x6a, 1); /*analog mode*/
    RPAL_SetRGB(rgb, 100);
    return d98;
}

void D98_PutLine(D98 *d98, PIXEL *buf)
{
    int x, xx;
    BYTE msk,bd,rd,gd,ed;
    BYTE far *gp, far *rp, far *bp, far *ep;

    if (d98->curY < 0 || d98->curY >= 400) {
    	if (d98->curY == 400 && d98->actPage == 0) {
    	    d98->actPage = 1;
    	    d98->curY -= 400;
    	    D98_GrphPage(1);	/* display-page 1 */
    	} else {
    	    return;
    	}
    }
    bp = d98->bp + d98->curY*80;
    rp = d98->rp + d98->curY*80;
    gp = d98->gp + d98->curY*80;
    ep = d98->ep + d98->curY*80;

    msk = 0x80;
    bd = rd = gd = ed = 0x00;
    xx = 0;
    for (x = 0; x < d98->maxX; x++) {
    	int n;
    	n = (BYTE)(buf[x] >> 24);
    	if (n & 0x08) {
    	    ed |= msk;
    	}
    	if (n & 0x04) {
    	    gd |= msk;
    	}
    	if (n & 0x02) {
    	    rd |= msk;
    	}
    	if (n & 0x01) {
    	    bd |= msk;
    	}
    	msk >>= 1;
    	if (msk == 0) {
    	    msk = 0x80;
    	    ep[xx] = ed;
    	    gp[xx] = gd;
    	    rp[xx] = rd;
    	    bp[xx] = bd;
    	    xx++;
    	    bd = rd = gd = ed = 0;
    	}
    }
    if (msk != 0x80) {
    	ep[xx] = ed;
    	gp[xx] = gd;
    	rp[xx] = rd;
    	bp[xx] = bd;
    }
    d98->curY++;
    return;
}
/*end D98 */

#endif	/* PC98 */


/*---------------------------------------------------------------------------*/
/*  	    	    	め  い	ん  	    	    	    	    	     */
/*---------------------------------------------------------------------------*/
#define WF_TIF	1
#define WF_BMP	2
#define WF_RGB	3
#define WF_Q0	4
#define WF_PMT	5
#define WF_PMY	6
#define WF_DJP	7
#define WF_98	8

char gWrtExt[][4] = {"","TIF","BMP","RGB","Q0","PMT","PMY","DJP",""};

void Asp2Resol(int asp1, int asp2, long *xresol, long *yresol)
    /* PIC のアスペクト比を元にTIFFの解像度を決める */
    /* TIFF(BMP)で、PICのアスペクト比を保存するのを目的にしているので */
    /* 解像度としては不当な値になるかも^^; */
{
  #if 0
    int f;
  #endif

    *xresol = *yresol = 100;
    while (((asp1 & 0x01) == 0) && ((asp2 & 0x01) == 0)) {
    	asp1 >>= 1;
    	asp2 >>= 1;
    }
    if (asp1 == 0 || asp2 == 0) {
    	printf("アスペクト比がおかしい\n");
    	return;
    }
    if (asp1 == asp2) {
    	return;
    }
  #if 0
    f = 0;
    if (asp1 < asp2) {
    	f = asp1;
    	asp1 = asp2;
    	asp2 = f;
    	f = 1;
    }
  #endif

    if (asp1 <= 5 && asp2 <= 5) {
    	asp1 *= 50;
    	asp2 *= 50;
    } else if (asp1 <= 10 && asp2 <= 10) {
    	asp1 *= 25;
    	asp2 *= 25;
    } else if (asp1 <= 50 && asp2 <= 50) {
    	asp1 *= 10;
    	asp2 *= 10;
    }

  #if 1
    *xresol = asp2;
    *yresol = asp1;
  #else
    if (f == 0) {
    	*xresol = asp2;
    	*yresol = asp1;
    } else {
    	*xresol = asp1;
    	*yresol = asp2;
    }
  #endif
}

void DspPicInfo(PIC *pic, int fmtNo, long xresol, long yresol)
{
    printf("           ｻｲｽﾞ%4d*%-4d %2dﾋﾞｯﾄ色  Aspect=%d:%-2d Type=%02x  (%d,%d)-(%d,%d)\n",
    	pic->xsize,pic->ysize,pic->colbit,
    	pic->asp1, pic->asp2, pic->typ,
    	pic->xstart,pic->ystart,pic->xstart+pic->xsize-1,pic->ystart+pic->ysize-1);
    if (fmtNo == 0) {
    	printf("           TIFF解像度(横,縦)=(%lddpi,%lddpi)\n",xresol,yresol);
    } else if (fmtNo == 1) {
    	printf("           BMP 解像度(横,縦)=(%lddpm,%lddpm)\n",xresol*40,yresol*40);
    }
    if (pic->comment && pic->comment[0]) {
    	printf("           ｺﾒﾝﾄ:%s\n",pic->comment);
    }
}

int ConvPdata (
    char *srcName,
    char *dstName,
    int  wrtFmt,
    int  asp1,
    int  asp2,
  #ifdef DITHER
    int  ditherMode,
  #endif
  #ifdef ASPX68K
    int  aspX68kDot,
  #endif
  #ifdef FDATEGET
    int  fdateFlg,
  #endif
    int  tifID,
    int  iprFlg,
    int  mmFlg
)
{
  #ifdef FDATEGET
    static unsigned fdat, ftim;
  #endif
    static long xresol, yresol;
    static BYTE *rgb;
    PIC *pic;
    int colbit,sizeYo;

    /* PICファイル(ヘッダ)読込みオープン */
    pic = PIC_Open(srcName,0);
    if (pic == NULL) {
    	return 1;
    }
  #ifdef FDATEGET
    /* ファイル日付取得 */
    fdat = ftim = 0;
    if (fdateFlg) {
    	_dos_getftime(fileno(pic->fp), &fdat, &ftim);
    }
  #endif

    colbit = /*(pic->colbit > 8) ? 24 :*/ pic->colbit;
    	    /* 8ﾋﾞｯﾄ色以上の色は24ﾋﾞｯﾄ色で出力 */
    rgb = pic->rgb;
    sizeYo = pic->sizeYo;
  #ifdef VA256PIC
    if (pic->typ == 0x21) {
    	sizeYo <<= 1;
    }
  #endif

    /* */
    if (asp1) {
    	pic->asp1 = asp1;
    }
    if (asp2) {
    	pic->asp2 = asp2;
    }
  #ifdef ASPX68K
    /* X68Kなアスペクト比なら横幅を調整 */
    if (aspX68kDot) {
    	PIC_InitAspX68k(pic, aspX68kDot);
    }
  #endif

    /* アスペクト比を解像度に変換 */
    Asp2Resol(pic->asp1o, pic->asp2o, &xresol, &yresol);

  #ifdef PC98
    if (wrtFmt == WF_98 && colbit > 4
    	&& ((ditherMode&0xff) >= 8 || (ditherMode&0xff) == 0)){
    	    /* PC9801表示で元絵が256色以上で、ﾃﾞｨｻﾞ8色変換でないなら、	*/
    	    /* ﾃﾞｨｻﾞ16色変換を行う  	    	    	    	    	*/
    	ditherMode &= 0xff00;
    	ditherMode |= 4;
    }
  #endif

    /* 変換メッセージ */
    printf("[PIC->%-3s] %s -> %s\n",gWrtExt[wrtFmt], srcName, dstName);
    DspPicInfo(pic, wrtFmt, xresol, yresol);
  #ifdef ASPX68K
    if (pic->aspX68k > 0) {
    	printf("           ｱｽﾍﾟｸﾄ比がX68Kな画像なので横幅を %d/%d 倍(横幅:%d->%dﾄﾞｯﾄ)\n",
    	    pic->aspX68k+1, pic->aspX68k, pic->xsize, pic->sizeXo);
    }
  #endif
  #ifdef DITHER
    if (ditherMode) {
    	if (wrtFmt != WF_RGB && wrtFmt != WF_Q0) {
    	    /* 減色時のﾒｯｾｰｼﾞ. RGB,Q0では減色しないので表示しない */
    	    printf("           %dﾋﾞｯﾄ色に減色(ディザ)\n", ditherMode&0xff);
    	}
    	rgb = PIC_DitherMode(pic, ditherMode);
    	colbit = ((ditherMode&0xff) == 8) ? 8 : 4;
    }
  #endif

    /* 出力ﾌｫｰﾏｯﾄごとに分岐 */
    switch (wrtFmt) {
    case WF_TIF:    	    /* TIFF出力のとき */
    	{
    	    TIF *tif;
    	    /* TIFファイル(ヘッダ作成) */
    	    colbit = (colbit > 8) ? 24 : colbit;
    	    tif = TIF_Create(dstName, colbit, pic->sizeXo, sizeYo,
    	    	xresol, yresol, rgb, tifID);
    	    if (tif == NULL) {
    	    	return 1;
    	    }
    	    /* ピクセル・データをコンバート */
    	    PIC_PutLines(pic, (void *)tif, (PIC_FUNC_PUTLINE)TIF_PutLine);
    	  #ifdef FDATEGET
    	    if (fdateFlg) {
    	    	fseek(tif->fp, 0, SEEK_SET);
    	    	    /*このfseekはおまじない...これしないと日付複写ができない.*/
    	    	    /*このﾌﾟﾛｸﾞﾗﾑのﾊﾞｸﾞなのか, TC++v1.01のﾗｲﾌﾞﾗﾘの性なのか...*/
    	    	    /* まあ自前でmsdosﾌｧﾝｸｼｮﾝ実行するのがいけないのだろうと */
    	    	_dos_setftime(fileno(tif->fp), fdat, ftim);
    	    }
    	  #endif
    	    TIF_CloseW(tif);
    	}
    	break;

    case WF_BMP:    	    /* BMP出力のとき */
    	{
    	    BMP *bmp;
    	    /* BMPファイル(ヘッダ作成) */
    	    colbit = (colbit > 8) ? 24 : colbit;
    	    bmp = BMP_Create(dstName, colbit, pic->sizeXo, sizeYo,
    	    	xresol*40, yresol*40, rgb);
    	    if (bmp == NULL) {
    	    	return 1;
    	    }
    	    /* ピクセル・データをコンバート */
    	    PIC_PutLines(pic, (void *)bmp, (PIC_FUNC_PUTLINE)BMP_PutLine);
    	  #ifdef FDATEGET
    	    if (fdateFlg) {
    	    	_dos_setftime(fileno(bmp->fp), fdat, ftim);
    	    }
    	  #endif
    	    BMP_CloseW(bmp);
    	}
    	break;

    case WF_RGB:    	    /* RGB出力のとき */
    case WF_Q0:     	    /* Q0 出力のとき */
    	{
    	    Q0	*q0p;
    	    /* Q0ファイル(ヘッダ)作成 */
    	    colbit = (colbit > 8) ? 24 : colbit;
    	    q0p = Q0_Create(dstName, pic->colbit, pic->sizeXo, sizeYo,
    	    	    	    pic->xstart, pic->ystart,
    	    	    	    pic->asp1o, pic->asp2o, rgb, iprFlg);
    	    if (q0p == NULL) {
    	    	return 1;
    	    }
    	    /* ピクセル・データをコンバート */
    	    PIC_PutLines(pic, (void *)q0p, (PIC_FUNC_PUTLINE)Q0_PutLine);
    	  #ifdef FDATEGET
    	    if (fdateFlg) {
    	    	fseek(q0p->fp, 0, SEEK_SET);
    	    	    /*このfseekはおまじない...これしないと日付複写ができない.*/
    	    	    /*このﾌﾟﾛｸﾞﾗﾑのﾊﾞｸﾞなのか, TC++v1.01のﾗｲﾌﾞﾗﾘの性なのか...*/
    	    	    /* まあ自前でmsdosﾌｧﾝｸｼｮﾝ実行するのがいけないのだろうと */
    	    	_dos_setftime(fileno(q0p->fp), fdat, ftim);
    	    }
    	  #endif
    	    Q0_CloseW(q0p);
    	}
    	break;

  #ifdef PC98
    case WF_98:     	    /* PC9801 VRAM 出力のとき */
    	{
    	    D98 *d98;
    	    colbit = (colbit > 8) ? 24 : colbit;
    	    d98 = D98_Create(dstName, colbit, pic->sizeXo, sizeYo, rgb);
    	    if (d98 == NULL) {
    	    	return 1;
    	    }
    	    /* ピクセル・データをコンバート */
    	    PIC_PutLines(pic, (void *)d98, (PIC_FUNC_PUTLINE)D98_PutLine);
    	    D98_CloseW(d98);
    	}
    	break;
  #endif
    case WF_PMY:
    case WF_PMT:    	    	/* PMT 出力のとき */
    	{
    	    PMT *pmt;
    	  #if 0
    	    if (wrtFmt == WF_PMT && colbit > 8) {
    	      #if 1
    	    	int l;
    	    	l = strlen(dstName);
    	    	if (l > 3 && stricmp(dstName+l-3,"PMT") == 0) {
    	    	    dstName[l-1] = 'Y';
    	    	}
    	      #else
    	    	printf("16 or 256色以外はpmtに変換できません\n");
    	    	exit(1);
    	      #endif
    	    }
    	  #endif
    	    /* PMTファイル(ヘッダ作成) */
    	    pmt = PMT_Create(dstName, colbit, pic->sizeXo, sizeYo,
    	    	    pic->xstart,pic->ystart, pic->asp1, pic->asp2, 0, rgb,
    	    	    ((mmFlg)?pic->comment:pic->comment2),(wrtFmt==WF_PMY),
    	    	    pic->artist
    	    	 #ifdef FDATEGET
    	    	    ,ftim,fdat
    	    	 #endif
    	    	);
    	    if (pmt == NULL) {
    	    	return 1;
    	    }
    	    /* ピクセル・データをコンバート */
    	    PIC_PutLines(pic, (void *)pmt, (PIC_FUNC_PUTLINE)PMT_PutLine);
    	  #ifdef FDATEGET
    	    if (fdateFlg) {
    	    	_dos_setftime(fileno(pmt->fp), fdat, ftim);
    	    }
    	  #endif
    	    PMT_Close(pmt);
    	}
    	break;

    case WF_DJP:    	    	/* DJP 出力のとき */
    	{
    	    DJP *dj;
    	    /* DJPファイル(ヘッダ作成) */
    	    dj = DJP_Create(dstName, colbit, pic->sizeXo, sizeYo, rgb);
    	    if (dj == NULL) {
    	    	return 1;
    	    }
    	    /* ピクセル・データをコンバート */
    	    PIC_PutLines(pic, (void *)dj, (PIC_FUNC_PUTLINE)DJP_PutLine);
    	  #ifdef FDATEGET
    	    if (fdateFlg) {
    	    	_dos_setftime(fileno(dj->fp), fdat, ftim);
    	    }
    	  #endif
    	    DJP_Close(dj);
    	}
    	break;

    default:
    	printf("PRGERR:出力ﾌｫｰﾏｯﾄ番号がおかしい\n");
    	exit(1);
    }

    PIC_CloseR(pic);
    return 0;
}


int DispInfo(char *srcName)
{
    PIC *pic;
    static long xresol, yresol;

    /* PICファイル(ヘッダ)読込みオープン */
    pic = PIC_Open(srcName,1);
    if (pic == NULL) {
    	return 1;
    }

    Asp2Resol(pic->asp1, pic->asp2, &xresol, &yresol);

    printf("%s\n", srcName);
    DspPicInfo(pic, -1, xresol, yresol);
    PIC_CloseR(pic);
    return 0;
}

void Usage(void)
{
  puts(
    "PICld v0.82    PIC -> 無圧縮 BMP,TIFF,RGB,Q0 コンバータ\n"
    "usage: PICld [.FMT] [-opts] picfile(s)[.pic]\n"
    " .FMT       出力フォーマット指定 .tif .bmp .rgb .q0 .djp .pmt\n"
    " -o<FILE>   出力ファイルを FILE にする（１ファイルのみ）\n"
    " -i         ヘッダ情報のみ表示\n"
    " -ipr       RGB,Q0 出力で一緒に生成する情報ﾌｧｲﾙを.FALでなく .IPRにする\n"
    " -ti        .TIF 指定時、インテル(II)なTIFFで出力\n"
    " -tm        .TIF 指定時、モトローラ(MM)なTIFFで出力\n"
    " -a[M:N]    入力したPICのｱｽﾍﾟｸﾄ比を強制的に M:N とする\n"
    " -mm        コメントから MM ヘッダを取り除かないようにする\n"
  #ifdef DITHER
    " -m<N>[:M]  ﾃﾞｨｻﾞで減色. -m3:8色(16色)  -m4:16色  -m8:256色\n"
    "            M:ﾃﾞｨｻﾞ･ﾊﾟﾀｰﾝを別のものに変更(0=8x8 他は4x4)\n"
  #endif
  #ifdef FDATEGET
    " -d[-]      入力ファイルの日付も複写. -d- で日付の複写を禁止\n"
  #endif
  #ifdef ASPX68K
    " -x[N]      X68Kなｱｽﾍﾟｸﾄ比のとき, 横幅を(N+1)/N 倍する. N:2～5 (省略 3)\n"
  #endif
  #ifdef PC98
    " -98        PC9801 のVRAMに表示(手抜き:-) 同時に-x4を指定したことになる.\n"
    " -k         PC9801表示(-98)のとき、表示ごとにキー入力待ち\n"
    "            ESC:中止 CR:次 SPC,TAB:画面表裏切換 T,t:ﾃｷｽﾄ on/off\n"
  #endif
    "\n"
    "picfileを入力してpicを .FMT で指定したフォーマットにして出力します\n"
    "ファイル名は複数指定できます(-f無指定時）\n"
    "デフォルトで .q0(q0出力) -d(日付複写) オプションが設定されています\n"
  );
  exit(0);
}


#ifdef KEYBRK
void cdecl KeyBrk(int sig)
    /* Stop-Key , ctrl-c 割り込み用*/
{
    printf("Abort.\n");
    sig = 1;
    exit(sig);
}
#endif

int main(int argc, char *argv[])
{
    static char srcName[FNAMESIZE+2];
    static char dstName[FNAMESIZE+2];
    static char *p;
  #ifdef DIRENTRY
    int DirEntryGet(char far *fname, char far *wildname, int fmode);
    static char nambuf[FNAMESIZE+2];
  #endif
  #ifdef DITHER
    static int	ditherMode = 0; /* 減色  0:しない  3:8色  4:16色  8:256色 */
  #endif
  #ifdef ASPX68K
    static int	aspX68kDot = 0; /* ｱｽﾍﾟｸﾄ比がx68kなとき横幅調整 */
  #endif
  #ifdef FDATEGET
    static int	fdateFlg = 1;	/* 元ﾌｧｲﾙの日付もコピー */
  #endif
    int  wrtFmt;    	    	/* 出力フォーマット */
    int  tifID;     	    	/* 0:ｲﾝﾃﾙなtiff  1:ﾓﾄﾛｰﾗなtiff */
    int  iprFlg;    	    	/* 0:.fal  1:.ipr */
    int  asp1,asp2; 	    	/* ｱｽﾍﾟｸﾄ比 */
    int  sw_oneFile;	    	/* -oで指定された１ファイルのみ変換 */
    int  sw_info;   	    	/* -iのときのPICヘッダのみの表示 */
    int  mmFlg;     	    	/* PMTのコメントに MM ヘッダを残すようにする*/
    int  i,c;

    mmFlg = asp1 = asp2 = iprFlg = sw_oneFile = sw_info = tifID = 0;
    wrtFmt = WF_Q0;
    srcName[0] = dstName[0] = '\0';
  #ifdef KEYBRK
    signal(SIGINT,KeyBrk);
  #endif
    if (argc < 2) {
    	Usage();
    }
    /* オプション読み取り */
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '.' && p[1] != '.' && p[1] != '/' && p[1] != '\\') {
    	    p++;
    	    for (wrtFmt = 1; gWrtExt[wrtFmt][0] != '\0'; wrtFmt++) {
    	    	if (stricmp(p,gWrtExt[wrtFmt]) == 0) {
    	    	    break;
    	    	}
    	    }
    	    if (gWrtExt[wrtFmt][0] == '\0') {
    	    	printf("知らないフォーマットだ\n",argv[i]),exit(1);
    	    }
    	} else if (*p == '-') {
    	    p++; c = toupper(*p); p++;
    	    switch (c) {
    	    case '?':
    	    case '\0':
    	    	Usage();
    	    	break;
    	    case 'Z':
    	    	PIC_rdDebFlg = 1;
    	    	c = *p++;
    	    	if (c == '2') {
    	    	    PIC_rdDebFlg = 2;
    	    	}
    	    	break;
    	    case 'O':
    	    	if (*p == 0) {
    	    	    goto OPTERR;
    	    	}
    	    	strncpy(dstName,p,FNAMESIZE);
    	    	dstName[FNAMESIZE] = 0;
    	    	sw_oneFile = 1;
    	    	break;
    	    case 'I':
    	    	if (*p == '\0') {
    	    	    sw_info = 1;
    	    	} else if ((*p == 'P' || *p == 'p')
    	    	    &&(*(p+1) == 'R' || *(p+1) == 'r')) {
    	    	    iprFlg = 1;
    	    	}
    	    	break;
    	    case 'F':
    	    	if ((*p == 'A' || *p == 'a')
    	    	    &&(*(p+1) == 'L' || *(p+1) == 'l'))
    	    	{
    	    	    iprFlg = 0;
    	    	}
    	    	break;
    	    case 'T':
    	    	if (*p == 'I' || *p == 'i') {
    	    	    tifID = 0;
    	    	} else if (*p == 'M' || *p == 'm') {
    	    	    tifID = 1;
    	    	}
    	    	break;
    	    case 'A':
    	    	if (*p == 0) {
    	    	    asp1 = asp2 = 1;
    	    	} else {
    	    	    asp1 = (int)strtol(p,&p, 0);
    	    	    if (*p/* == ','*/) {
    	    	    	asp2 = (int)strtol(p+1,NULL,0);
    	    	    }
    	    	    if (asp1 > 255 || asp2 > 255) {
    	    	    	printf("アスペクト比の値が大きすぎる\n");
    	    	    	exit(1);
    	    	    }
    	    	}
    	    	break;
  #ifdef DITHER
    	    case 'M':
    	    	if (*p == 'M' || *p == 'm') {
    	    	    mmFlg = 1;
    	    	    break;
    	    	}
    	    	c = *p - '0';
    	    	if (c == 3 || c == 4 || c == 8) {
    	    	    ditherMode = c;
    	    	} else {
    	    	    ditherMode = 4;
    	    	}
    	    	if (*++p/* == ','*/) {
    	    	    c = *++p - '0';
    	    	    if (c >= 0 && c < PIC_DIT_CNT) {
    	    	    	ditherMode |= c*0x0100;
    	    	    }
    	    	}
    	    	break;
  #endif
  #ifdef ASPX68K
    	    case 'X':
    	    	aspX68kDot = 3;
    	    	if (*p >= '1' && *p <= '5') {
    	    	    aspX68kDot = *p - '0';
    	    	}
    	    	break;
  #endif
  #ifdef FDATEGET
    	    case 'D':
    	    	fdateFlg = 1;
    	    	if (*p == '-' || *p == '0') {
    	    	    fdateFlg = 0;
    	    	}
    	    	break;
  #endif
  #ifdef PC98
    	    case '9':
    	    	if (*p != '8') {
    	    	    goto OPTERR;
    	    	}
    	    	wrtFmt = WF_98;
    	    	aspX68kDot = 4;
    	    	break;
    	    case 'K':
    	    	D98_KeyMode(1);
    	    	break;
  #endif
    	    default:
      OPTERR:
    	    	printf("オプション指定がおかしい[ -%s ]\n",p-1);
    	    	exit(1);
    	    }
    	}
    }

    /* ファイルごとの処理 */
    c = 0;
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-' || *p == '.') {	/* オプション文字列は無視 */
    	    continue;
    	}
    	strcpy(srcName,p);
    	FIL_AddExt(srcName,"PIC");/*入力ﾌｧｲﾙに拡張子がなければ.pic付加*/
      #ifdef DIRENTRY
    	if (DirEntryGet(nambuf,srcName,0) == 0) {
    	    strcpy(srcName,nambuf);
    	    do {
      #endif
    	    	if (sw_info) {	    	    /* -i :PICヘッダ表示 */
    	    	    c = DispInfo(srcName);
    	    	} else {
    	    	    if (sw_oneFile == 0) {/* 複数ファイル変換(デフォルト) */
    	    	    	strcpy(dstName,srcName);
    	    	    	FIL_ChgExt(dstName, gWrtExt[wrtFmt]);
    	    	    } else {
    	    	    	FIL_AddExt(dstName, gWrtExt[wrtFmt]);
    	    	    }
    	    	    c = ConvPdata(
    	    	    	    srcName,
    	    	    	    dstName,
    	    	    	    wrtFmt,
    	    	    	    asp1,
    	    	    	    asp2,
    	    	    	  #ifdef DITHER
    	    	    	    ditherMode,
    	    	    	  #endif
    	    	    	  #ifdef ASPX68K
    	    	    	    aspX68kDot,
    	    	    	  #endif
    	    	    	  #ifdef FDATEGET
    	    	    	    fdateFlg,
    	    	    	  #endif
    	    	    	    tifID,
    	    	    	    iprFlg,
    	    	    	    mmFlg
    	    	    	);
    	    	    if (sw_oneFile) {
    	    	    	return c;
    	    	    }
    	    	}
      #ifdef DIRENTRY
    	    } while(DirEntryGet(srcName,NULL,0) == 0);
    	}
      #endif
    }
    return c;
}

/*
    MS-DOS以上のメモリがあり、整数は 2の補数表現なｲﾝﾃﾙなかﾓﾄﾛｰﾗなかで、
    int 16ﾋﾞｯﾄ以上 long 32ﾋﾞｯﾄ以上で unsigned char,unsigned long があって、
    関数はANSI-Cな宣言ができて、ANSI-Cな標準ﾗｲﾌﾞﾗﾘで...
*/
/*
    作りはじめた当初は無謀にも PIC 展開のサンプル的なものをめざし、簡潔な
    代物で、ファイル分割しなくていいサイズをめざしてたのだけど...
    あれよあれよというまにでっかくなってしまった... PC98出力,ディザ,横幅調整,
    256色VA-PIC対応は, はっきりいって邪魔ですね^^;。
    いいかげん、ファイル分割しなきゃなあ、と思うのだけど...思うけど
*/
