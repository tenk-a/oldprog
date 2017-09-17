/*===========================================================================*/
/*  MKI_PLUG	    	    	    	    	    	    	    	     */
/*===========================================================================*/

#include "tofmt4p.c"


/*---------------------------------------------------------------------------*/
/*-----    ヘ  ル  プ	 ----------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void Usage(void)
{
    puts(
    	"mki_plug ver1.00                    by Tenka* 1995\n"
    	"usage: mki_plug <COMMAND> <inputfile> <outputfile>\n"
    	" <COMMAND> ... DJ505JC, TO_DJP or TO_BMP.\n"
    	);
    exit(1);
}



/*---------------------------------------------------------------------------*/
/*-----    入 力 ル ー チ ン   ----------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*static char  MKI_comment[34];*/   	/* セーバ・メッセージ	    	     */
static UCHAR FAR *MKI_g[4]; 	    	/* VRAM 4ﾌﾟﾚｰﾝへのﾎﾟｲﾝﾀ     	     */
static FILE  *MKI_fp;	    	    	/* 入力のFILE Pointer	    	     */
static int   MKI_mode = 0;  	    	/* 0=typeA  1=typeB 	    	     */
static UINT  MKI_flagBsz;
static UINT  MKI_pdatAsz;
static UINT  MKI_pdatBsz;
static UINT  MKI_pdatSz;
static UINT  MKI_pdatp;
static UINT  MKI_flagBp;
static UCHAR FAR *MKI_pdat;
static UCHAR FAR *MKI_flagBbuf;



static void MKI_PutLine(UCHAR *p, int y)
    /* 4ﾌﾟﾚｰﾝ640x400画面の y行目に buf を変換転送 */
    /* ... 出力側で逆変換やってること思うと、遅くなるしすごく無駄だが、*/
    /*	   専用の出力側を作るのがめんどくさいので^^;	    	       */
    /*	   それと, 一つで 64Kを越えるバッファを扱いたくなかったので... */
{
    UCHAR x;
    UCHAR a,b,c,d;
    UCHAR FAR *gp, FAR *rp, FAR *bp, FAR *ep;

    bp = MKI_g[0] + y*80;
    rp = MKI_g[1] + y*80;
    gp = MKI_g[2] + y*80;
    ep = MKI_g[3] + y*80;

    for (x = 0; x < 80; x++) {
    	a = *p++; b = *p++; c = *p++; d = *p++;
    	*ep++ =   ((a&0x80)    /*>>7<<7*/) | ((a&0x08)<<3 /*>>3<<6*/)
    	    	| ((b&0x80)>>2 /*>>7<<5*/) | ((b&0x08)<<1 /*>>3<<4*/)
    	    	| ((c&0x80)>>4 /*>>7<<3*/) | ((c&0x08)>>1 /*>>3<<2*/)
    	    	| ((d&0x80)>>6 /*>>7<<1*/) | ((d&0x08)>>3 /*>>3<<0*/);
    	*gp++ =   ((a&0x40)<<1 /*>>6<<7*/) | ((a&0x04)<<4 /*>>2<<6*/)
    	    	| ((b&0x40)>>1 /*>>6<<5*/) | ((b&0x04)<<2 /*>>2<<4*/)
    	    	| ((c&0x40)>>3 /*>>6<<3*/) | ((c&0x04)	  /*>>2<<2*/)
    	    	| ((d&0x40)>>5 /*>>6<<1*/) | ((d&0x04)>>2 /*>>2<<0*/);
    	*rp++ =   ((a&0x20)<<2 /*>>5<<7*/) | ((a&0x02)<<5 /*>>1<<6*/)
    	    	| ((b&0x20)    /*>>5<<5*/) | ((b&0x02)<<3 /*>>1<<4*/)
    	    	| ((c&0x20)>>2 /*>>5<<3*/) | ((c&0x02)<<1 /*>>1<<2*/)
    	    	| ((d&0x20)>>4 /*>>5<<1*/) | ((d&0x02)>>1 /*>>1<<0*/);
    	*bp++ =   ((a&0x10)<<3 /*>>4<<7*/) | ((a&0x01)<<6 /*>>0<<6*/)
    	    	| ((b&0x10)<<1 /*>>4<<5*/) | ((b&0x01)<<4 /*>>0<<4*/)
    	    	| ((c&0x10)>>1 /*>>4<<3*/) | ((c&0x01)<<2 /*>>0<<2*/)
    	    	| ((d&0x10)>>3 /*>>4<<1*/) | ((d&0x01)	  /*>>0<<0*/);
    }
}



static void MKI_Read(void FAR *buf, UINT size)
{
  #ifdef TCSML
    UINT n;
    if (_dos_read(fileno(MKI_fp), buf, size, &n) != 0) {
    	printf("[MKI-DECODER] read error\n"); exit(1);
    }
  #else
    fread(buf, 1, size, MKI_fp);
    if (ferror(MKI_fp)) {
    	printf("[MKI-DECODER] read error\n"); exit(1);
    }
  #endif
}



static void MKI_GetHeader(UCHAR *rgb)
{
    struct MKI_H {
    	char  comment[32];
    	UCHAR fBsz[2];
    	UCHAR pAsz[2];
    	UCHAR pBsz[2];
    	USHORT tilCnt;
    	USHORT x0;
    	USHORT y0;
    	UCHAR  xsz[2];
    	UCHAR  ysz[2];
    	UCHAR  grb[48];
    } m;

    MKI_Read(&m, sizeof m);
    /* セーバ・コメント取得 */
  #if 0
    memcpy(MKI_comment, m.comment, 32);
    MKI_comment[31] = '\0';
  #endif
    /* ID チェック */
    if (strncmp(m.comment, "MAKI01A",7) == 0) {
    	MKI_mode = 2;
    } else if (strncmp(m.comment, "MAKI01B",7) == 0) {
    	MKI_mode = 4;
    } else {
    	printf("[MKI-DECODER] Bad ID\n"); exit(1);
    }

    /* 圧縮データ関係 */
    MKI_flagBsz = (m.fBsz[0]<<8) + m.fBsz[1];
    MKI_pdatAsz = (m.pAsz[0]<<8) + m.pAsz[1];
    MKI_pdatBsz = (m.pBsz[0]<<8) + m.pBsz[1];
    if (m.tilCnt != 0) {
    	printf("[MKI-DECODER] Unknown header. (Must be 'tile_size = 0')\n");
    	exit(1);
    }
    /* 始点.  (0,0)でなければならない. */
    if (m.x0 != 0 || m.y0 != 0) {
    	printf("[MKI-DECODER] Unknown header. (Bad 'start_position')\n");
    	exit(1);
    }
    /* 画像サイズ 640x400 でなければならない */
    if (((m.xsz[0]<<8)|m.xsz[1]) != 640 || ((m.ysz[0]<<8)|m.ysz[1]) != 400) {
    	printf("[MKI-DECODER] Unknown header.(Bad 'picture_size')\n");
    	exit(1);
    }
    /* パレット */
    {int i;
    	for (i = 0; i < 16; i++) {
    	    rgb[i*3+1] = m.grb[i*3+0];	/* G */
    	    rgb[i*3+0] = m.grb[i*3+1];	/* R */
    	    rgb[i*3+2] = m.grb[i*3+2];	/* B */
    	}
    }
}



static void MKI_AllocMem(void)
{
    UINT pdataSiz;
    MKI_flagBbuf = MALLOC(MKI_flagBsz);
    pdataSiz = (MKI_pdatAsz > MKI_pdatBsz) ? MKI_pdatAsz : MKI_pdatBsz;
    MKI_pdat = MALLOC(pdataSiz);
    if (MKI_flagBbuf == NULL || MKI_pdat == NULL) {
    	printf("[MKI-DECODER] Out of memory.\n"); exit(1);
    }
}

static void MKI_FreeMem(void)
{
    FREE(MKI_flagBbuf);
    FREE(MKI_pdat);
}

#define MKI_ReadFlagB()     \
    	(MKI_Read(MKI_flagBbuf, MKI_flagBsz), MKI_flagBp = 0)

static UINT MKI_GetFlagBB(void)
{
    UINT c;
    c = MKI_flagBbuf[MKI_flagBp++];
    c = (c << 8) | MKI_flagBbuf[MKI_flagBp++];
  #if 1
    if (MKI_flagBp > MKI_flagBsz) {
    	printf("[MKI-DECODER] Abnormal data.(Bad 'flag-B')\n"); exit(1);
    }
  #endif
    return c;
}

#define MKI_ReadPdataA()    \
    	(MKI_Read(MKI_pdat,MKI_pdatAsz), MKI_pdatSz = MKI_pdatAsz, MKI_pdatp=0)

#define MKI_ReadPdataB()    \
    	(MKI_Read(MKI_pdat,MKI_pdatBsz), MKI_pdatSz = MKI_pdatBsz, MKI_pdatp=0)

#if 0
#define     MKI_GetPdat()   	(MKI_pdat[MKI_pdatp++])
#else
static UINT MKI_GetPdat(void)
{
    UINT c;
    c = MKI_pdat[MKI_pdatp++];
    if (MKI_pdatp > MKI_pdatSz) {
    	printf("[MKI-DECODER] Abnormal data.(Bad 'Pixel-DATA')\n"); exit(1);
    }
    return c;
}
#endif



static void MKI_DecoSubE(UCHAR *vv, UINT dat)
{
    *vv = ((dat>> 8) & 0xF0) | (*vv & 0x0F);	vv += 40;
    *vv = ((dat>> 4) & 0xF0) | (*vv & 0x0F);	vv += 40;
    *vv = ((dat    ) & 0xF0) | (*vv & 0x0F);	vv += 40;
    *vv = ((dat<< 4) & 0xF0) | (*vv & 0x0F);
}

static void MKI_DecoSubO(UCHAR *vv, UINT dat)
{
    *vv = ((dat>>12) & 0x0F) | (*vv & 0xF0);	vv += 40;
    *vv = ((dat>> 8) & 0x0F) | (*vv & 0xF0);	vv += 40;
    *vv = ((dat>> 4) & 0x0F) | (*vv & 0xF0);	vv += 40;
    *vv = ((dat    ) & 0x0F) | (*vv & 0xF0);
}

static void MKI_Decode(void)
{
    UCHAR flagA[100][(80/8)];	/* 80=320/4, 100=400/4 */
    UCHAR vv[4][40];
    UCHAR pp[4][320];
    UCHAR bb[320];
    UCHAR c, *p, *v, *b;
    int   i,x,y,ya;

    MKI_Read(flagA, 1000);
    MKI_ReadFlagB();
    MKI_ReadPdataA();
    memset(pp, 0x00, sizeof pp);
    y = 0;
    for (ya = 0; ya < 100; ya++) {
    	if (ya == 50) {
    	    MKI_ReadPdataB();
    	}
    	memset(vv, 0, sizeof vv);
    	p = flagA[ya];
    	for (p = flagA[ya], v = vv[0], x = 10; --x >= 0; v += 4) {
    	    c = *p++;
    	    if (c & 0x80) MKI_DecoSubE(v+0,MKI_GetFlagBB());
    	    if (c & 0x40) MKI_DecoSubO(v+0,MKI_GetFlagBB());
    	    if (c & 0x20) MKI_DecoSubE(v+1,MKI_GetFlagBB());
    	    if (c & 0x10) MKI_DecoSubO(v+1,MKI_GetFlagBB());
    	    if (c & 0x08) MKI_DecoSubE(v+2,MKI_GetFlagBB());
    	    if (c & 0x04) MKI_DecoSubO(v+2,MKI_GetFlagBB());
    	    if (c & 0x02) MKI_DecoSubE(v+3,MKI_GetFlagBB());
    	    if (c & 0x01) MKI_DecoSubO(v+3,MKI_GetFlagBB());
    	}
    	for (i = 0; i < 4; i++) {
    	    memset(bb, 0, sizeof bb);
    	    for (v = vv[i], b = bb, x = 40; --x >= 0; b += 8) {
    	    	c = *v++;
    	    	if (c & 0x80)	b[0] = MKI_GetPdat();
    	    	if (c & 0x40)	b[1] = MKI_GetPdat();
    	    	if (c & 0x20)	b[2] = MKI_GetPdat();
    	    	if (c & 0x10)	b[3] = MKI_GetPdat();
    	    	if (c & 0x08)	b[4] = MKI_GetPdat();
    	    	if (c & 0x04)	b[5] = MKI_GetPdat();
    	    	if (c & 0x02)	b[6] = MKI_GetPdat();
    	    	if (c & 0x01)	b[7] = MKI_GetPdat();
    	    }
    	    x = 320, p = pp[i], b = bb, v = pp[(i-MKI_mode)&3];
    	    do {
    	    	*p++ = *v++ ^ *b++;
    	    } while (--x);
    	    MKI_PutLine(pp[i],y++);
    	}
    }
}



int  GLoad(TOFMT4P *t, char *name)
{
    MKI_g[0] = t->v[0];
    MKI_g[1] = t->v[1];
    MKI_g[2] = t->v[2];
    MKI_g[3] = t->v[3];
    MKI_fp = fopen(name,"rb");
    if (MKI_fp == NULL) {
    	printf("[MKI-DECODER] Unable to open file %s\n",name); return -1;
    }

    MKI_GetHeader(t->rgb);
    MKI_AllocMem();
    MKI_Decode();
    MKI_FreeMem();

    fclose(MKI_fp);

    t->xsz = 640;
    t->ysz = 400;
  #if 0
    if (strlen(MKI_comment))
    	t->comment = MKI_comment;
  #endif
    return 0;
}
