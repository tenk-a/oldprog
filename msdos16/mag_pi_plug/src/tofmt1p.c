/*===========================================================================*/
/*  	DJPなプラグイン仕様の　?->DJP,PMT コンバータ 共通ルーチン   	     */
/*  	far モデルでコンパイルすること!!    	    	    	    	     */
/*===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include <malloc.h>
#define VBUFSIZE    	0x2000

typedef unsigned char	UCHAR;
typedef unsigned int	UINT;
typedef unsigned short	USHORT;
typedef unsigned long	ULONG;

#define isdigit(c)  (((unsigned)(c) - '0') < 10U)
#define iskanji(c)  ((unsigned)((c)^0x20) - 0xa1 < 0x3cU)
#define islower(c)  (((unsigned)(c)-'a') < 26U)
#define toupper(c)  (islower(c) ? (c) - 0x20 : (c) )

#ifdef DEBUG
#define D(a)	a
#else
#define D(a)	{}
#endif



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

typedef struct TOFMT1P {
    int     xsz, ysz;	    	/* 画像サイズ	    	    	    	*/
    int     x0, y0; 	    	/* 始点     	    	    	    	*/
    int     xasp, yasp;     	/* アスペクト比     	    	    	*/
    int     col;    	    	/* 色数bit 4:16色  8:256色  	    	*/
    int     bcol;   	    	/* 透明色 0:無し 1～:あり(bcol-1)   	*/
    UINT    dat;    	    	/* ファイル日付     	    	    	*/
    UINT    tim;    	    	/* ファイル時間     	    	    	*/
    char    *comment;	    	/* コメントへのポインタ     	    	*/
    char    artist[20];     	/* 作者名   	    	    	    	*/
    UCHAR   rgb[256*3];     	/* パレット (r,g,b)*256 r,g,bは各8ﾋﾞｯﾄ	*/
} TOFMT1P;


void G_Usage(void);
int  G_Open(TOFMT1P *t, char *name);
void G_Load(TOFMT1P *t, void (far * far putLine)(void far *, UCHAR far *));
void G_Close(TOFMT1P *t);



/*---------------------------------------------------------------------------*/
/*-----    出 力　ル ー チ ン	---------------------------------------------*/
/*---------------------------------------------------------------------------*/
static FILE *TO_fp;

#define TO_Err()    (ferror(TO_fp))

#define TO_PutB(b)  ( fputc((UCHAR)(b),TO_fp) )

static void TO_PutW(UINT w)
{
    fputc((UCHAR)(w),TO_fp);
    fputc((UCHAR)((w)>>8),TO_fp);
}

static void TO_PutD(ULONG d)
{
    fputc((UCHAR)(d),TO_fp);
    fputc((UCHAR)((d)>>8),TO_fp);
    fputc((UCHAR)((ULONG)(d)>>16),TO_fp);
    fputc((UCHAR)((ULONG)(d)>>24),TO_fp);
}

static void TO_Write(void *buf, unsigned size)
{
    fwrite(buf, 1, size, TO_fp);
}

static void _loadds /*_saveregs*/ far TO_PutLine(void far *t, UCHAR far *b)
{
    TO_Write(b, ((TOFMT1P*)t)->xsz);
}

static void _loadds /*_saveregs*/ far TO_PutLineL2(void far *t, UCHAR far *b)
{
    TO_Write(b, ((TOFMT1P*)t)->xsz);
    TO_Write(b, ((TOFMT1P*)t)->xsz);
}


static int TO_Djp(TOFMT1P *t, char *name, int aspFlg)
{
    aspFlg = (aspFlg && t->xasp && t->yasp && (t->yasp*10 / t->xasp) >= 18);

    TO_fp = fopen(name, "wb");	    	    	    if (TO_fp==NULL) return 2;
    if( setvbuf(TO_fp, NULL, _IOFBF, VBUFSIZE) )    {fclose(TO_fp); return 1; }

    /* ID */
    TO_Write("DJ505J",6);   	    	    	    if (TO_Err())   goto ERR_W;
    /* 画像サイズ */
    TO_PutW(t->xsz);	    	    	    	    if (TO_Err())   goto ERR_W;
    TO_PutW((aspFlg) ? (t->ysz+t->ysz) : t->ysz);   if (TO_Err())   goto ERR_W;
    /* 8ビット色画像 */
    TO_PutW(0);     	    	    	    	    if (TO_Err())   goto ERR_W;
    /* パレット */
    TO_Write(t->rgb, 256*3);	    	    	    if (TO_Err())   goto ERR_W;
    /* ピクセル・データ */
    if (aspFlg)
    	G_Load(t, TO_PutLineL2);
    else
    	G_Load(t, TO_PutLine);
    /* 終了 */
    fclose(TO_fp);
    return 0;

  ERR_W:
    fclose(TO_fp);
    return 3;
}



static int TO_Pmt(TOFMT1P *t, char *name, int aspFlg)
    /* 拙作 mg.exe の作業ﾌｧｲﾙﾌｫｰﾏｯﾄ PMT 対応... ほとんどの人に無関係^^; */
{
    int  i;

    aspFlg = (aspFlg && t->xasp && t->yasp && (t->yasp*10 / t->xasp) >= 18);

    TO_fp = fopen(name, "wb");
    if (TO_fp == NULL)
    	return 2;
    if ( setvbuf(TO_fp, NULL, _IOFBF, VBUFSIZE) ) {
    	fclose(TO_fp);	return 1;
    }

    TO_Write("Pm",2);	    	    	    	    	    	/* ID	    */
    if (TO_Err())
    	goto ERR_W;
    TO_PutB(t->col);	    	    	    	    	    	/* 16|256色画像 */
    TO_PutB(0x00);  	    	    	    	    	    	/* flags    */
    TO_PutW(t->xsz);	    	    	    	    	    	/* x size   */
    TO_PutW((aspFlg)?(t->ysz*2):t->ysz);    	    	    	/* y size   */
    TO_PutW(t->x0); 	    	    	    	    	    	/* x start  */
    TO_PutW((aspFlg)?(t->y0*2):t->y0);	    	    	    	/* y start  */
    TO_PutW(t->bcol);	    	    	    	    	    	/* 透明色   */
    TO_PutW(0x00);  	    	    	    	    	    	/* rsv.     */
    TO_PutW((aspFlg) ? 1 : (t->xasp?t->xasp:1));    	    	/* x aspect.*/
    TO_PutW((aspFlg) ? 1 : (t->yasp?t->yasp:1));    	    	/* y aspect.*/

    if (t->comment && strlen(t->comment))   	    	    	/* comment  */
    	TO_PutD(64 + 256*3 + (ULONG)t->xsz*(ULONG)t->ysz);
    else
    	TO_PutD(0L);

    /* 作者名 */
    if (strlen(t->artist) == 0) {
    	memset(t->artist, 0 , sizeof t->artist);
    }
    TO_Write(t->artist, 18);
    TO_PutB(0);

    /* ファイル時間,日付 */
    TO_PutB(0);
    TO_PutW(t->tim);
    TO_PutW(t->dat);

    /* 予約領域 */
    for(i = 0; i < 16; i++)
    	TO_PutB(0);

    if (TO_Err())
    	goto ERR_W;

    /* パレット */
    TO_Write(t->rgb, 256*3);
    if (TO_Err())
    	goto ERR_W;

    /* ピクセル・データ */
    if (aspFlg) {
    	G_Load(t, TO_PutLineL2);
    } else {
    	G_Load(t, TO_PutLine);
    }

    /* コメント */
    if (t->comment && strlen(t->comment)) {
    	char *p;
    	for (p = t->comment, i = strlen(t->comment); i-- ; p++)
    	    TO_PutB(*p);
    	TO_PutB(0);
    	if (TO_Err())
    	    goto ERR_W;
    }

    fclose(TO_fp);
    return 0;

  ERR_W:
    fclose(TO_fp);
    return 3;
}



/*---------------------------------------------------------------------------*/
/*-----    メ　イ　ン　ル ー チ ン   ----------------------------------------*/
/*---------------------------------------------------------------------------*/


int cdecl main(int argc, char *argv[])
{
    enum {FMT_NON, FMT_DJP, FMT_PMT} fmt;
    static TOFMT1P t;
    int i, c, aflg;
    char *p, *srcname, *dstname;

    fmt = FMT_NON;
    aflg = 0;
    if (argc < 2)
    	G_Usage();

    /* オプションのチェック */
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p != '-')
    	    break;
    	++p;
    	c = *p++, c = toupper(c);
    	switch(c) {
    	case 'A':
    	    aflg = 1;
    	    if (*p == '-')
    	    	aflg = 0;
    	    break;
    	case '?':
    	case '\0':
    	    G_Usage();
    	default:
    	    printf("Incorrect command-line option. : %s\n", argv[i]), exit(1);
    	}
    }
    if (argc-i < 3) {
    	printf("Too few command-line arguments.\n");	exit(1);
    }
    srcname = argv[i+1];
    dstname = argv[i+2];

    /* 出力フォーマット指定のチェック */
    strupr(p);
    if	    (strcmp(p,"DJ505JC") == 0) fmt = FMT_DJP,	aflg = 1;
    else if (strcmp(p,"DJ505JM") == 0) fmt = FMT_DJP,	aflg = 1;
    else if (strcmp(p, "TO_DJP") == 0) fmt = FMT_DJP;
    else if (strcmp(p, "TO_PMT") == 0) fmt = FMT_PMT;
    else {
    	printf("Bad '<FORMAT>' name. : %s\n",p), exit(1);
    }

    /* 変換メッセージ */
    printf("%s : (%s) %s -> %s\n", argv[0], argv[i], argv[i+1], argv[i+2]);

    /* 画像を展開するための初期化 */
    memset(&t, 0, sizeof t);
    switch(G_Open(&t,srcname)) {
    case 0: break;
    case 1: printf("Not enough memory for input-buffer.\n");	return 1;
    case 2: printf("Unable to open file '%s'\n", srcname);  	return 1;
    case 3: printf("Incorrect header.\n");  	    	    	return 1;
    default:printf("Programer's error 'G_Open'\n"); 	    	return 1;
    }

    /* 展開＆出力 */
    switch (fmt) {
    case FMT_PMT: c = TO_Pmt(&t, dstname, aflg); break;
    case FMT_DJP: c = TO_Djp(&t, dstname, aflg); break;
    default:	  printf("Programer's error.\n"); return 1;
    }
    /* 終了 */
    G_Close(&t);
    switch (c) {
    case 1: printf("Not enough memory for output-buffer.\n");	return 1;
    case 2: printf("Cannot create file '%s'\n",dstname);    	return 1;
    case 3: printf("Cannot write output.\n");	    	    	return 1;
    }
    return 0;
}
