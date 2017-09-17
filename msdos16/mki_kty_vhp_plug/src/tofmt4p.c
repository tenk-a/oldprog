/*===========================================================================*/
/*  	DJPなプラグイン仕様の　?->DJP,BMP,PMT コンバータ 共通ルーチン	     */
/*  	　98等の640*400(200) 4プレーン(16色) 画像ファイルを対象とする	     */
/*===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

typedef unsigned char	UCHAR;
typedef unsigned int	UINT;
typedef unsigned short	USHORT;
typedef unsigned long	ULONG;

#ifndef toascii /* ctype.h がincludeされていないとき */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif

#ifdef DEBUG
#define D(a)	a
#else
#define D(a)	{}
#endif

#ifdef TCSML	/* TC,BC,LSI-C ｽﾓｰﾙ･ﾐﾃﾞｨｱﾑ･ﾓﾃﾞﾙな場合	 */
    #include <dos.h>
    #include <malloc.h>
    #define MALLOC(n)	    farmalloc(n)
    #define CALLOC(c,n)     farcalloc(c,n)
    #define FREE(p) 	    farfree(p)
    #define FAR     	    far
    #define CDECL   	    cdecl
    #define VBUFSIZE	    0x2000
#else	    	/* 32ビット(または far data)なコンパイラな場合 */
    #define MALLOC(n)	    malloc(n)
    #define CALLOC(c,n)     calloc(c,n)
    #define FREE(p) 	    free(p)
    #define FAR
    #define CDECL
    #define VBUFSIZE	    0x7000
#endif


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

typedef struct TOFMT4P {
    int     xsz, ysz;	    	/* 画像サイズ(横幅は8で割り切れること)	*/
    int     xasp, yasp;     	/* アスペクト比     	    	    	*/
    int     x0, y0; 	    	/* 始点 (x0は8で割り切れること)     	*/
    int     bcol;   	    	/* 透明色 0:無し 1～:あり(bcol-1)   	*/
    UCHAR   rgb[16*3];	    	/* パレット (r,g,b)*256 r,g,bは各8ﾋﾞｯﾄ	*/
    UCHAR FAR *v[4];	    	/* VRAMﾌﾟﾚｰﾝへのポインタ(B,R,G,E)   	*/
    char    *comment;	    	/* コメントへのポインタ     	    	*/
} TOFMT4P;



static int TO_InitG(TOFMT4P *t)
{
    #define PSIZE   (0x8000U)
    memset(t, 0, sizeof(TOFMT4P));
    t->v[0] = CALLOC(1,PSIZE);
    t->v[1] = CALLOC(1,PSIZE);
    t->v[2] = CALLOC(1,PSIZE);
    t->v[3] = CALLOC(1,PSIZE);
    	/*16ﾋﾞｯﾄｺﾝﾊﾟｲﾗでは callocの返す値は、hugeポインタを満たしていること.*/
    if (t->v[0]==NULL || t->v[1]==NULL || t->v[2]==NULL || t->v[3]==NULL)
    	return -1;
    return 0;
}

static int TO_TermG(TOFMT4P *t)
{
    FREE(t->v[0]);
    FREE(t->v[1]);
    FREE(t->v[2]);
    FREE(t->v[3]);
    memset(t, 0, sizeof(TOFMT4P));
    return 0;
}


static void TO_GetLine(TOFMT4P *t, int x, int y, int w, UCHAR FAR *p)
    /* (x,y)の横 len ドットを 1ドット1バイトで バッファ p に納める */
    /* x,w は８ドット間隔の値であるこ */
{
    UCHAR FAR *b, FAR *r, FAR *g, FAR *e;

    x = (x>>3) + y*80;
    b = t->v[0]+x, r = t->v[1]+x, g = t->v[2]+x, e = t->v[3]+x;
    w >>= 3;
    do {
    	UCHAR ee,gg,rr,bb;
    	bb = *b++, rr = *r++, gg = *g++, ee = *e++;
    	*p++ = ((ee&0x80)>>4)+((gg&0x80)>>5)+((rr&0x80)>>6)+((bb&0x80)>>7);
    	*p++ = ((ee&0x40)>>3)+((gg&0x40)>>4)+((rr&0x40)>>5)+((bb&0x40)>>6);
    	*p++ = ((ee&0x20)>>2)+((gg&0x20)>>3)+((rr&0x20)>>4)+((bb&0x20)>>5);
    	*p++ = ((ee&0x10)>>1)+((gg&0x10)>>2)+((rr&0x10)>>3)+((bb&0x10)>>4);
    	*p++ = ((ee&0x08)   )+((gg&0x08)>>1)+((rr&0x08)>>2)+((bb&0x08)>>3);
    	*p++ = ((ee&0x04)<<1)+((gg&0x04)   )+((rr&0x04)>>1)+((bb&0x04)>>2);
    	*p++ = ((ee&0x02)<<2)+((gg&0x02)<<1)+((rr&0x02)   )+((bb&0x02)>>1);
    	*p++ = ((ee&0x01)<<3)+((gg&0x01)<<2)+((rr&0x01)<<1)+((bb&0x01)	 );
    } while (--w);
}



/*---------------------------------------------------------------------------*/
/*-----    出 力　ル ー チ ン	---------------------------------------------*/
/*---------------------------------------------------------------------------*/
static FILE *TO_fp;
static int  TO_err = 0;

static void TO_Create(char *name)
{
    TO_err = 0;
    TO_fp = fopen(name, "wb");
    TO_err = ferror(TO_fp);
    if (TO_err)
    	return;
    if( setvbuf(TO_fp, NULL, _IOFBF, VBUFSIZE) )
    	fclose(TO_fp);
    TO_err = ferror(TO_fp);
}

static void TO_Close(void)
{
    fclose(TO_fp);
}

static void TO_Write(void *buf, size_t bufsz)
{
    fwrite((buf), 1, (bufsz), TO_fp);
    TO_err = ferror(TO_fp);
}

static void TO_PutB(int b)
{
    fputc((UCHAR)(b),TO_fp);
    TO_err = ferror(TO_fp);
}

static void TO_PutW(UINT w)
{
    fputc((UCHAR)(w),TO_fp);
    fputc((UCHAR)((w)>>8),TO_fp);
    TO_err = ferror(TO_fp);
}

static void TO_PutD(ULONG d)
{
    fputc((UCHAR)(d),TO_fp);
    fputc((UCHAR)((d)>>8),TO_fp);
    fputc((UCHAR)((ULONG)(d)>>16),TO_fp);
    fputc((UCHAR)((ULONG)(d)>>24),TO_fp);
    TO_err = ferror(TO_fp);
}



/*---------------------------------------------------------------------------*/

static int TO_Bmp(TOFMT4P *t, char *name, int aspFlg)
    /* 入力側(t) の横幅が 8ﾄﾞｯﾄ単位であることを利用して手をぬいています！ */
{
    UCHAR buf[640];
    int   i, col, a;

    col = 16;	/* 16 色画像 */
    /* いわゆる 200ライン画像かどうか */
    a = (t->xasp && t->yasp && t->yasp / t->xasp >= 2);
    /* ドット比調整するかどうか */
    aspFlg = (aspFlg && a);

    TO_Create(name);
    if (TO_err)
    	return 2;
    TO_Write("BM",2);	    	    	    	    	    /* ID   	    */
    if (TO_err)
    	goto ERR_W;
    i = (col == 16) ? (t->xsz>>1) : t->xsz;
    TO_PutD(14L+40L+col*4L+(long)i*(long)t->ysz);   	    /* file size    */
    TO_PutD(0L);    	    	    	    	    	    /* rsv  	    */
    TO_PutD(14L+40L+col*4L);	    	    	    	    /* pdata ofs    */
    TO_PutD(40L);   	    	    	    	    	    /* hdr size     */
    TO_PutD(t->xsz);	    	    	    	    	    /* x size	    */
    TO_PutD((aspFlg)?(t->ysz*2):t->ysz);    	    	    /* y size	    */
    TO_PutW(1);     	    	    	    	    	    /* plane	    */
    TO_PutW(col==16?4:8);   	    	    	    	    /* colorBits    */
    TO_PutD(0L);    	    	    	    	    	    /* comp Mode    */
    TO_PutD(0L);    	    	    	    	    	    /* compPdataSiz */
    TO_PutD((aspFlg) ? 0 : (a?100*40:0) );  	    	    /* x resolu.    */
    TO_PutD((aspFlg) ? 0 : (a? 50*40:0) );  	    	    /* y resolu.    */
    TO_PutD(col);   	    	    	    	    	    /* palette Cnt. */
    TO_PutD(col);   	    	    	    	    	    /* col. impo.   */
    for (i = 0; i < col; i++) {     	    	    	    /* palette	    */
    	TO_PutB(t->rgb[i*3+2]);     	    	    	    /*	    b	    */
    	TO_PutB(t->rgb[i*3+1]);     	    	    	    /*	    g	    */
    	TO_PutB(t->rgb[i*3+0]);     	    	    	    /*	    r	    */
    	TO_PutB(0x00	     );     	    	    	    /*	    a	    */
    }
    if (TO_err)
    	goto ERR_W;

    /* ピクセル・データ */
    /*	 BMPの横幅は4バイト単位でないといけないが、入力側が必ず８ドット単位 */
    /*	 なので横幅調整の必要無し */
    a = t->xsz >> 1;
    for (i = t->y0+t->ysz; --i >= t->y0;) {
    	UCHAR *p, *q;
    	int  j;
    	TO_GetLine(t, t->x0, i, t->xsz, buf);
    	p = q = buf;
    	j = a;
    	do {
    	    *q++ = (p[0] << 4) | (p[1] & 0x0f);
    	    p+=2;
    	} while(--j);
    	TO_Write(buf, a);
    	if (aspFlg)
    	    TO_Write(buf, a);
    	if (TO_err)
    	    goto ERR_W;
    }
    TO_Close();
    return 0;

  ERR_W:
    TO_Close();
    return 3;
}



static int TO_Djp(TOFMT4P *t, char *name, int aspFlg)
{
    UCHAR buf[640];
    int  i;

    aspFlg = (aspFlg && t->xasp && t->yasp && t->yasp / t->xasp >= 2);

    TO_Create(name);
    if (TO_err)
    	return 2;

    TO_Write("DJ505J",6);   	    	    	    	    /* ID   	     */
    if (TO_err)
    	goto ERR_W;
    TO_PutW(t->xsz);	    	    	    	    	    /* x size	     */
    TO_PutW((aspFlg) ? (t->ysz+t->ysz) : t->ysz);   	    /* y size	     */
    TO_PutW(0);     	    	    	    	    	    /* 8ビット色画像 */
    if (TO_err)
    	goto ERR_W;
    TO_Write(t->rgb, 16*3); 	    	    	    	    /* パレット      */
    for (i = 16*3; i < 256*3; i++)
    	TO_PutB(0);
    if (TO_err)
    	goto ERR_W;
    for (i = t->y0; i < t->y0+t->ysz; i++) {	    	    /* ピクセルDATA  */
    	TO_GetLine(t, t->x0, i, t->xsz, buf);
    	TO_Write(buf, t->xsz);
    	if (aspFlg)
    	    TO_Write(buf, t->xsz);
    	if (TO_err)
    	    goto ERR_W;
    }
    TO_Close();
    return 0;

  ERR_W:
    TO_Close();
    return 3;
}


static int TO_Pmt(TOFMT4P *t, char *name, int aspFlg)
    /* 拙作 mg.exe の作業ﾌｧｲﾙﾌｫｰﾏｯﾄ PMT 対応... ほとんどの人に無関係^^; */
{
    UCHAR buf[640];
    int  i;

    aspFlg = (aspFlg && t->xasp && t->yasp && t->yasp / t->xasp >= 2);

    TO_Create(name);
    if (TO_err)
    	return 2;

    TO_Write("Pm",2);	    	    	    	    	    	/* ID	    */
    if (TO_err)
    	goto ERR_W;
    TO_PutB(0x04);  	    	    	    	    	    	/* 16色画像 */
    TO_PutB(0x00);  	    	    	    	    	    	/* flags    */
    TO_PutW(t->xsz);	    	    	    	    	    	/* x size   */
    TO_PutW((aspFlg)?(t->ysz*2):t->ysz);    	    	    	/* y size   */
    TO_PutW(t->x0); 	    	    	    	    	    	/* x start  */
    TO_PutW((aspFlg)?(t->y0*2):t->y0);	    	    	    	/* y start  */
    TO_PutW(t->bcol);	    	    	    	    	    	/* 透明色   */
    TO_PutW(0x00);  	    	    	    	    	    	/* rsv.     */
    TO_PutW((aspFlg) ? 1 : (t->xasp?t->xasp:1));    	    	/* x aspect.*/
    TO_PutW((aspFlg) ? 1 : (t->yasp?t->yasp:1));    	    	/* y aspect.*/
  #if 0
    if (t->comment && strlen(t->comment))   	    	    	/* comment  */
    	TO_PutD(64 + 256*3 + (ULONG)t->xsz*(ULONG)t->ysz);
    else
  #endif
    	TO_PutD(0L);
    /* 作者名、日付、等... めんどくさいので省略 ^^; */
    for(i = 0; i < 18+1+1+2+2+16; i++)
    	TO_PutB(0);
    if (TO_err)
    	goto ERR_W;
    /* パレット */
    TO_Write(t->rgb, 16*3);
    for (i = 16*3; i < 256*3; i++)
    	TO_PutB(0);
    if (TO_err)
    	goto ERR_W;
    /* ピクセル・データ */
    for (i = t->y0; i < t->y0+t->ysz; i++) {
    	TO_GetLine(t, t->x0, i, t->xsz, buf);
    	TO_Write(buf, t->xsz);
    	if (aspFlg)
    	    TO_Write(buf, t->xsz);
    	if (TO_err)
    	    goto ERR_W;
    }
  #if 0
    /* コメント */
    if (t->comment && strlen(t->comment)) {
    	char *p;
    	for (p = t->comment, i = strlen(t->comment); i-- ; p++)
    	    TO_PutB(*p);
    	TO_PutB(0);
    	if (TO_err)
    	    goto ERR_W;
    }
  #endif
    TO_Close();
    return 0;

  ERR_W:
    TO_Close();
    return 3;
}



/*---------------------------------------------------------------------------*/
/*-----    メ　イ　ン　ル ー チ ン   ----------------------------------------*/
/*---------------------------------------------------------------------------*/

void Usage(void);
int  GLoad(TOFMT4P *t, char *name);


int CDECL main(int argc, char *argv[])
{
    static TOFMT4P t;
    enum {FMT_NON, FMT_DJP, FMT_BMP, FMT_PMT} fmt;
    int i, c, aflg;
    char *p;

    fmt = FMT_NON;
    aflg = 0;
    if (argc < 2)
    	Usage();

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
    	    Usage();
    	default:
    	    printf("Incorrect command-line option. : %s\n", argv[i]), exit(1);
    	}
    }
    if (argc-i < 3) {
    	printf("Too few command-line arguments.\n");	exit(1);
    }

    /* 出力フォーマット指定のチェック */
    strupr(p);
    if	    (strcmp(p,"DJ505JC") == 0) fmt = FMT_DJP,	aflg = 1;
    else if (strcmp(p,"DJ505JM") == 0) fmt = FMT_DJP,	aflg = 1;
    else if (strcmp(p, "TO_DJP") == 0) fmt = FMT_DJP;
    else if (strcmp(p, "TO_BMP") == 0) fmt = FMT_BMP;
    else if (strcmp(p, "TO_PMT") == 0) fmt = FMT_PMT;
    else {
    	printf("Bad '<FORMAT>' name. : %s\n",p), exit(1);
    }

    /* 変換メッセージ */
    printf("%s : (%s) %s -> %s\n", argv[0], argv[i], argv[i+1], argv[i+2]);

    /* 画像を展開するための初期化 */
    if (TO_InitG(&t)) {
    	printf("Not enough memory for input-buffer.\n"), exit(1);
    }

    /* 画像を展開する */
    p = argv[++i];
    if (GLoad(&t, p) < 0) {
    	printf("Cannot decode file %s.\n", p), exit(1);
    }

    /* 展開した画像を出力 */
    p = argv[++i];
    switch (fmt) {
    case FMT_PMT: c = TO_Pmt(&t, p, aflg); break;
    case FMT_DJP: c = TO_Djp(&t, p, aflg); break;
    case FMT_BMP: c = TO_Bmp(&t, p, aflg); break;
    default:	  printf("Programer's error: @FMT^^;\n"); return 1;
    }
    TO_TermG(&t);
    switch (c) {
    case 1: printf("Not enough memory for output-buffer.\n");	return 1;
    case 2: printf("Cannot create file %s\n",p);    	    	return 1;
    case 3: printf("Cannot write output.\n");	    	    	return 1;
    }
    return 0;
}
