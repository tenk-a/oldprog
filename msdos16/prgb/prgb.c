/*
     ＰＲＧＢ	 ファイルよりＲＧＢパレットを取り出す
    v1.00  92/10/20  writen by てんか☆
    v1.01  92/11/03  隠しオプション -wt -> -wf に変更
    v1.02  93/05/16  rgb出力で-vを指定しなくてもtoneの影響を受けていたのを修正
    	    	     -8ｵﾌﾟｼｮﾝ追加.
    v1.03  93/06/02  -m4,-m5ｵﾌﾟｼｮﾝ追加.
 */
#define  PRGB	"PRGB v1.03"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>
#include <signal.h>

#define PC98  /* この PC98 を定義しないばあい、DOS汎用のコンバート・ツール
    	    	 になるはずです */

/*--------------------------------------------------------------------------*/
#define _S  static
typedef unsigned char	byte;
typedef unsigned    	word;
typedef unsigned long	dword;

#define MAKE_FP(s,o)  ((void far *)(((dword)(s)<<16) | ((o)&0xffff)))
#define PTR_OFF(p)  ((unsigned short) (p))
#define PTR_SEG(p)  ((unsigned short)((unsigned long)(void far*)(p) >> 16))

#ifndef toascii /* ctype.h がincludeされていないとき */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif
#define pr(s)	printf(s)
extern int DirEntryGet(byte far *,byte far *,word);
/*--------------------------------------------------------------------------*/
byte *gDfltExt = "RGB";
byte gRgbOfs[6][3] = {{0,1,2},{2,0,1},{1,2,0},{1,0,2},{2,1,0},{0,2,1}};
word gTone = 100;
int  gPalSiz = 48;
int  gRgbMode,gMtlFlg,gH4bFlg,gDspFlg,gInitFlg,gToneFlg;
int  gPSftB,gPSftR,gPSftG;
int  gPSftB2,gPSftR2,gPSftG2;
int  gRgbOfsNo;
long gSeekPos;
byte *gRgbName;
int  gChgPalCnt;
int  gChgPalNo[16];
int  gChgPal[16][3];
byte gPal[48];
byte gPal32[32];
#ifdef PC98
int  gPc98Flg = 1;
#endif

/*-------------------------------------------------------------------------*/

#ifdef PC98
#define AnalogPal() outp(0x6a,0x01)
#define WaitVsync() {while((inp(0x60)&0x20));	while(!(inp(0x60)&0x20));}

#define SetGRB(n,g,r,b) do{\
    	      outp(0xa8, (n));\
    	      outp(0xaa, (g));\
    	      outp(0xac, (r));\
    	      outp(0xae, (b));\
    	  }while(0)

struct RAMPAL {
    byte id[10];
    byte tone;
    byte rsv[5];
    byte grb[16][3];
};
struct RAMPAL far *RPal_Search(void);
int RPal_Get(word *tone, byte *rgb);
void RPal_Set(int tone, byte *rgb);
void RPal_ChgTone(int tone);

struct mcb_t {
    byte  flag;
    word owner;
    word blksiz;
    byte reserve[11];
};

struct mcb_t far *_dos_mcbfirst(void)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.ah = 0x52;
    intdosx(&reg,&reg,&sreg);
    return MAKE_FP(*(word far *)MAKE_FP(sreg.es, reg.x.bx - 2), 0);
}

_S int far_strcmp(unsigned char far *s1, unsigned char far *s2)
{
    while (*s1 == *s2)
    	if (*s1 == '\0')
    	    return 0;
    	else {
    	    ++s1;
    	    ++s2;
    	}
    if (*s1 > *s2)
    	return 1;
    else
    	return -1;
}

struct RAMPAL far *
RPal_Search(void)
{
    struct mcb_t far *p;
    word seg;

    p = _dos_mcbfirst();
    seg = PTR_SEG(p);
    for (;;) {
    	if (p->owner != NULL) {
    	    if (far_strcmp((byte far *)p + 0x10, "pal98 grb") == 0)
    	    	return (struct RAMPAL far *)((byte far *)p + 0x10);
    	}
    	seg += p->blksiz + 1;
    	p = MAKE_FP(seg,0);
    	if (p->flag == 'Z')
    	    break;
    }
    return NULL;
}


void RPal_Set(int tone, byte *rgb)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) != NULL) {
    	p->tone = tone;
    	for (i = 0; i < 16; ++i) {
    	    p->grb[i][0] = rgb[i*3 + 1];
    	    p->grb[i][1] = rgb[i*3 + 0];
    	    p->grb[i][2] = rgb[i*3 + 2];
    	}
    }
    for (i = 0; i < 16; ++i) {
    	int r,g,b;
    	r = rgb[i*3 + 0] * tone / 100; r = (r > 15) ? 15 : r;
    	g = rgb[i*3 + 1] * tone / 100; g = (g > 15) ? 15 : g;
    	b = rgb[i*3 + 2] * tone / 100; b = (b > 15) ? 15 : b;
    	SetGRB(i,g,r,b);
    }
}

int RPal_Get(word *tone, byte *rgb)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) == NULL)
    	return -1;
    if (tone)
    	*tone = p->tone;
    for (i = 0; i < 16; ++i) {
    	rgb[i * 3 + 0] = p->grb[i][1];
    	rgb[i * 3 + 1] = p->grb[i][0];
    	rgb[i * 3 + 2] = p->grb[i][2];
    }
    return 0;
}

#endif	/* PC98 */

/*--------------------------------------------------------------------------*/

#if 0
void TCls(void)
{
    printf ("\x1b[2J\x1b[>5h\x1b[>1h");/*ﾃｷｽﾄ画面ｸﾘｱ;ｶｰｿﾙoff;PF表示off*/
}
#endif

void ChgExt(byte *onam, byte *inam, byte *ext)
{
    byte *p;

    p = onam;
    if (p != inam)
    	strncpy(p,inam,80);
    if (*p == '.')
    	p++;
    if (*p == '.')
    	p++;
    if ((p = strrchr(p,'.')) == NULL) {
    	p = onam;
    	while(*p)
    	    p++;
    }
    if (ext == NULL||*ext == '\0') {
    	*p = '\0';
    } else {
    	*p++ = '.';
    	strcpy(p,ext);
    }
}

void AddExt(byte *onam,byte *inam,byte *ext)
{
    byte *s;
    s = inam;
    if (*s == '.')
    	s++;
    if (*s == '.')
    	s++;
    if (strrchr(s,'.') == NULL) {
    	ChgExt(onam,inam,ext);
    } else {
    	strcpy(onam,inam);
    }
}

int EquExt(byte *s,byte *e)
{
    if (*s == '.')
    	s++;
    if (*s == '.')
    	s++;
    if ((s = strrchr(s,'.')) == NULL) {
    	if (e == NULL||*e == '\0')
    	    return 1;
    } else {
    	if (stricmp(s+1,e) == 0)
    	    return 1;
    }
    return 0;
}

byte *SkipDir(byte *adr)
{
    byte *p;

    p = adr;
    if (*p == '.')
    	++p;
    if (*p == '.')
    	++p;
    while (*p != '\0') {
    	if (*p == ':' || *p == '/' || *p == '\\')
    	    adr = p + 1;
    	p++;
    }
    return adr;
}


/*--------------------------------------------------------------------------*/
void Exit(int n)
{
  #if 0
    if (gPc98Flg)
    	printf("\x1b[>5l\x1b[>1l");/*ｶｰｿﾙon;PF表示on*/
  #endif
    exit(n);
}

void PrExit(byte *p)
{
    printf("%s\n",p);
    Exit(1);
}

FILE *fopen_e(byte *fnam,byte *mode)
{
    FILE *fp;

    fp = fopen(fnam,mode);
    if (fp == NULL) {
    	printf("%s をオープンできません\n",fnam);
    	exit(1);
    }
    return fp;
}

int StrToInt(byte **p)
{
    byte *s,*ss;
    int  t,xf,sf;

    s = *p;
    t = xf = 0;
    sf = 1;
    if (*s == '-') {
    	sf = -1;
    	s++;
    }
    if (*s == '$') {
    	xf = 1;
    	s++;
    } else if (*s == '0' && (*(s+1) == 'x' || *(s+1) == 'X')) {
    	xf = 1;
    	s += 2;
    }
    ss = s;
    if (isxdigit(*s) == 0)
    	return 0x8000;
    while (isxdigit(*s)) {
    	if ((*s >= 'A' && *s <= 'F')||(*s >= 'a' && *s <= 'f'))
    	    xf = 1;
    	s++;
    }
    if (s != *p && (*s == 'H' || *s == 'h')) {
    	xf = 1;
    	s++;
    }
    *p = s;
    if (xf) {
    	t = (int)strtol(ss,NULL,16);
    } else {
    	t = (int)strtol(ss,NULL,10);
    }
    return sf * t;
}

byte *FGetTok(byte **wk, FILE *fp)
{
    static byte buf[300+40];
    byte *p,*s;

    if (*wk == NULL) {
  JJJ:
    	*wk = p = fgets(buf,300,fp);
    	if (p == NULL)
    	    return NULL;
    } else
    	p = *wk;
    for (;;) {
    	if (*p == '\0' || *p == '#') {
    	    goto JJJ;
    	} else if (*p > ' '/* && *p != ','*/) {
    	    break;
    	}
    	++p;
    }
    s = p;
    for (;;) {
    	if (*p == '\0') {
    	    *wk = NULL;
    	    return s;
    	} else if (*p <= ' '/*|| *p == ','*/) {
    	    *p = '\0';
    	    *wk = p+1;
    	    return s;
    	}
    	++p;
    }
}
/*--------------------------------------------------------------------------*/
/*
   .alg ファイルの読み込み＆作成ルーチンは、マシーンＭ氏作のVHPローダ＆
   セーバのＣのソースより拝借しました^^;
 */

_S void RgbToHsv(int r, int g, int b, int hsv[])
    /*
       RGB -> hsv
       in : r  - R (0-15)
    	    g  - G (0-15)
    	    b  - B (0-15)

       out: hsv[0]  - 色相     (0-360)
    	    hsv[1]  - 色彩     (0-100)
    	    hsv[2]  - 最大明度 (0-100)
     */
{
    int  h, s, v, x, r1, g1, b1;

    r = r * 100 / 15;
    g = g * 100 / 15;
    b = b * 100 / 15;

    h = 0;
    v = (r > g) ? r : g;
    v = (v > b) ? v : b;

    x = (r < g) ? r : g;
    x = (x < b) ? x : b;

    s = (v == 0) ? 0 : (v - x) * 100 / v;

    if (s != 0) {
    	r1 = (v - r) * 60 / (v - x);
    	g1 = (v - g) * 60 / (v - x);
    	b1 = (v - b) * 60 / (v - x);

    	if (r == v) {
    	    if (g == x)
    	    	h = 300 + b1;
    	    else
    	    	h = 60 - g1;
    	} else if (g == v) {
    	    if (b == x)
    	    	h = 60 + r1;
    	    else
    	    	h = 180 - b1;
    	} else {
    	    if (r == x)
    	    	h = 180 + g1;
    	    else
    	    	h = 300 - r1;
    	}
    	if (h < 0)
    	    h += 360;
    	h %= 360;
    }
    hsv[0] = h;
    hsv[1] = s;
    hsv[2] = v;
}


_S void PutAlgFile(FILE *fp, byte *rgb)
{
    static int cv[] = {0, 9, 10, 11, 12, 13, 14, 15, 8, 1, 2, 3, 4, 5, 6, 7};
    static int hsv[3];
    static short int hsvbuf[48];
    int  i, a, b;

    for (i = 0; i < 16; ++i) {
    	RgbToHsv(rgb[i * 3 + 0], rgb[i * 3 + 1], rgb[i * 3 + 2], hsv);
    	a = cv[i] / 8;
    	b = cv[i] % 8;
    	hsvbuf[a * 24 + b] = hsv[0];
    	hsvbuf[a * 24 + b + 8] = hsv[1];
    	hsvbuf[a * 24 + b + 16] = hsv[2];
    }
  #if 0
    for (i = 0; i < 48; ++i) {
    	a = hsvbuf[i] & 0xff;
    	b = hsvbuf[i] >> 8;
    	*((byte*)hsvbuf + i*2 +0) = a;
    	*((byte*)hsvbuf + i*2 +1) = b;
    }
  #endif
    fwrite(hsvbuf,2,48,fp);
}

_S void HsvToRgb(int h, int s, int v, byte *rgb)
    /*--------------
     * hsv -> RGB
     *--------------
     * in : h  - 色相	  (0-360)
     *	    s  - 色彩	  (0-100)
     *	    v  - 最大明度 (0-100)
     *
     * out: rgb[0]  - R (0-15)
     *	    rgb[1]  - G (0-15)
     *	    rgb[2]  - B (0-15)
     */
{
    int  a[7];
    int  i, f, r, g, b;

    i = h / 60;
    f = (h % 60) * 100 / 60;

    a[1] = a[2] = v;
    a[3] = v * (100 - s * f / 100) / 100;
    a[4] = a[5] = v * (100 - s) / 100;
    a[6] = v * (100 - (s * (100 - f)) / 100) / 100;

    r = a[i = (i > 4) ? i - 4 : i + 2] * 16 / 100;
    b = a[i = (i > 4) ? i - 4 : i + 2] * 16 / 100;
    g = a[i = (i > 4) ? i - 4 : i + 2] * 16 / 100;

    rgb[0] = (r >= 16) ? 15 : r;
    rgb[1] = (g >= 16) ? 15 : g;
    rgb[2] = (b >= 16) ? 15 : b;
}


_S void GetAlgFile(byte *fnam)
{
    FILE *fp;
    static int cv[] = {0, 9, 10, 11, 12, 13, 14, 15, 8, 1, 2, 3, 4, 5, 6, 7};
    int  a, b, i, h, s, v;
    static short int hsv[48];

    fp = fopen_e(fnam,"rb");
    for (i = 0; i < 48; ++i) {
    	hsv[i] = getc(fp);
    	hsv[i] += getc(fp) * 256;
    }
    for (i = 0; i < 16; ++i) {
    	a = cv[i] / 8;
    	b = cv[i] % 8;
    	h = hsv[a * 24 + b];
    	s = hsv[a * 24 + b + 8];
    	v = hsv[a * 24 + b + 16];
    	HsvToRgb(h, s, v, gPal + i*3);
    }
    fclose(fp);
}
/*--------------------------------------------------------------------------*/

void PalInit(int flg)
{
    static byte pal[] = {
    	0x0,0x0,0x0,
    	0x0,0x0,0x7,
    	0x7,0x0,0x0,
    	0x7,0x0,0x7,
    	0x0,0x7,0x0,
    	0x0,0x7,0x7,
    	0x7,0x7,0x0,
    	0x7,0x7,0x7,
    	0x0,0x0,0x0,
    	0x0,0x0,0xF,
    	0xF,0x0,0x0,
    	0xF,0x0,0xF,
    	0x0,0xF,0x0,
    	0x0,0xF,0xF,
    	0xF,0xF,0x0,
    	0xF,0xF,0xF,
    };
    static byte digpal[] = {
    	0x0,0x0,0x0,
    	0x0,0x0,0xF,
    	0xF,0x0,0x0,
    	0xF,0x0,0xF,
    	0x0,0xF,0x0,
    	0x0,0xF,0xF,
    	0xF,0xF,0x0,
    	0xF,0xF,0xF,
    	0x0,0x0,0x0,
    	0x0,0x0,0xF,
    	0xF,0x0,0x0,
    	0xF,0x0,0xF,
    	0x0,0xF,0x0,
    	0x0,0xF,0xF,
    	0xF,0xF,0x0,
    	0xF,0xF,0xF,
    };
    memcpy(gPal,(flg == 0) ? pal : digpal, 48);
  #ifdef PC98
    if (gPc98Flg)
    	RPal_Set(gTone,gPal);
  #endif
}

_S void DspPal(void)
{
    static byte pal[48];
    static word tone;
    int i;

    tone = gTone;
  #ifdef PC98
    if (gPc98Flg == 0 || RPal_Get(&tone,pal))
  #endif
    {
    	memcpy(pal,gPal,48);
    }
    printf("tone=%d%%\n",tone);
    printf("n   R G B\n");
    for (i = 0; i < 16; i++) {
    	printf("%x   %x %x %x\n",i,pal[i*3+0],pal[i*3+1],pal[i*3+2]);
    }
}

_S void PutRgbFile(int palmode, int mflg, byte *name)
{
    byte rnam[130];
    static byte pal[48];
    FILE *fp;
    int  i;

    if (gToneFlg) {
    	for (i = 0; i < 48; i++) {
    	    pal[i] = gPal[i] * gTone / 100;
    	    if (pal[i] > 15)
    	    	pal[i] = 15;
    	}
    } else {
    	for (i = 0; i < 48; i++) {
    	    pal[i] = gPal[i];
    	}
    }
    switch (palmode) {
    case 0:
    	ChgExt(rnam,name,"RGB");
    	name = (mflg) ? SkipDir(rnam) : rnam;
    	fp = fopen_e(name,"wb");
    	fwrite(pal,1,48,fp);
    	fclose(fp);
    	break;
    case 1:
    	ChgExt(rnam,name,"FRM");
    	name = (mflg) ? SkipDir(rnam) : rnam;
    	fp = fopen_e(name,"r+b");
    	fseek(fp,128000L,SEEK_SET);
    	fwrite(pal,1,48,fp);
    	fclose(fp);
    	break;
    case 2:
    	ChgExt(rnam,name,"PAL");
    	name = (mflg) ? SkipDir(rnam) : rnam;
    	fp = fopen_e(name,"w");
    	for (i = 0;i < 16;i++)
    	    fprintf(fp,"%d\t%d\t%d\t%d\n",i,pal[i*3],pal[i*3+1],pal[i*3+2]);
    	fclose(fp);
    	break;
    case 3:
    	ChgExt(rnam,name,"ALG");
    	name = (mflg) ? SkipDir(rnam) : rnam;
    	fp = fopen_e(name,"wb");
    	PutAlgFile(fp,pal);
    	fclose(fp);
    	break;
    }
}

_S void ChgRgb(byte *fnam)
{
    int i;
    for (i = 0; i < 16;i++) {
    	int r,g,b;
    	r = gPal[i*3+gRgbOfs[gRgbOfsNo][0]];
    	g = gPal[i*3+gRgbOfs[gRgbOfsNo][1]];
    	b = gPal[i*3+gRgbOfs[gRgbOfsNo][2]];
    	if (gH4bFlg) {
    	    r >>= 4;
    	    g >>= 4;
    	    b >>= 4;
    	}
    	gPal[i*3+0] = r;
    	gPal[i*3+1] = g;
    	gPal[i*3+2] = b;
    }
    if (gChgPalCnt) {
    	for (i = 0; i < gChgPalCnt; i++) {
    	    int t;
    	    t = gChgPalNo[i];
    	    gPal[t*3+0] = gChgPal[i][0];
    	    gPal[t*3+1] = gChgPal[i][1];
    	    gPal[t*3+2] = gChgPal[i][2];
    	}
    }
    if (gRgbMode) {
    	if (gRgbName == NULL && fnam == NULL) {
    	    PrExit("-wでファイル名が指定されていない\n");
    	}
    	if (gRgbName == NULL)
    	    PutRgbFile(gRgbMode - 1, 1, fnam);
    	else
    	    PutRgbFile(gRgbMode - 1, 0, gRgbName);
    }
  #ifdef PC98
    if (gPc98Flg)
    	RPal_Set(gTone,gPal);
  #endif
    if (gDspFlg)
    	DspPal();
}

_S void GetRgbFile(byte *fnam)
{
    FILE *fp;
    int  i;
    word t;

    fp = fopen_e(fnam,"rb");
    if (gSeekPos >= 0)
    	t = fseek(fp,gSeekPos,SEEK_SET);
    else
    	t = fseek(fp,gSeekPos,SEEK_END);
    if (t)
    	PrExit("シーク・エラー\n");
    if (gPalSiz == 32) {
    	fread(gPal32,2,16,fp);
    	for (i = 0; i < 16; i++) {
    	    if (gMtlFlg)
    	    	t = gPal32[i*2] * 0x100 + gPal32[i*2+1];
    	    else
    	    	t = gPal32[i*2+1] * 0x100 + gPal32[i*2];
    	    gPal[i*3+0] = (t >> gPSftB) & 0x0f;
    	    gPal[i*3+1] = (t >> gPSftR) & 0x0f;
    	    gPal[i*3+2] = (t >> gPSftG) & 0x0f;
    	}
    } else if (gPalSiz == 24) {
    	fread(gPal32,3,8,fp);
      #if 0
    	for (i = 0; i < 8; i++) {
    	    int t2;
    	    if (gMtlFlg) {
    	    	t = gPal32[i*3	] * 0x10 + gPal32[i*3+1]/0x10;
    	    	t2= (gPal32[i*3+1]&0x0f) * 0x100 + gPal32[i*3+2];
    	    } else {
    	    	t = gPal32[i*3	] + (gPal32[i*3+1]&0x0f)*0x100;
    	    	t2= (gPal32[i*3+1]/0x10) * 0x100 + gPal32[i*3+2];
    	    }
    	    gPal[i*2*3+0] = (t >> gPSftB) & 0x0f;
    	    gPal[i*2*3+1] = (t >> gPSftR) & 0x0f;
    	    gPal[i*2*3+2] = (t >> gPSftG) & 0x0f;
    	    gPal[(i*2+1)*3+0] = (t2 >> gPSftB) & 0x0f;
    	    gPal[(i*2+1)*3+1] = (t2 >> gPSftR) & 0x0f;
    	    gPal[(i*2+1)*3+2] = (t2 >> gPSftG) & 0x0f;
    	}
      #else
    	for (i = 0; i < 8; i++) {
    	    unsigned long tt;
    	    if (gMtlFlg) {
    	    	tt = gPal32[i*3]*0x10000L +gPal32[i*3+1]*0x100 +gPal32[i*3+2];
    	    } else {
    	    	tt = gPal32[i*3+2]*0x10000L +gPal32[i*3+1]*0x100 +gPal32[i*3];
    	    }
    	    gPal[i*2*3+0] = (tt >> gPSftB) & 0x0f;
    	    gPal[i*2*3+1] = (tt >> gPSftR) & 0x0f;
    	    gPal[i*2*3+2] = (tt >> gPSftG) & 0x0f;
    	    gPal[(i*2+1)*3+0] = (tt >> gPSftB2) & 0x0f;
    	    gPal[(i*2+1)*3+1] = (tt >> gPSftR2) & 0x0f;
    	    gPal[(i*2+1)*3+2] = (tt >> gPSftG2) & 0x0f;
    	}
      #endif
    } else {
    	fread (gPal,1,48,fp);
    }
    fclose(fp);
}

/*--------------------------------------------------------------------------*/
int StrToInt_0_15(byte **p)
{
    int t;
    t = StrToInt(p);
    if (t < 0 || t > 15)
    	PrExit("値が 0から15 の範囲外\n");
    return t;
}

_S int	ChkP(byte *s)
{
    if (stricmp(s,"R") == 0)
    	return 0;
    if (stricmp(s,"G") == 0)
    	return 1;
    if (stricmp(s,"B") == 0)
    	return 2;
    return -1;
}

_S int ChkPPP(byte **aFGetTokWk, FILE *fp, byte *p,int *o0,int *o1,int *o2)
{
    if (p == NULL)
    	return -1;
    *o0 = ChkP(p);
    if (*o0 < 0)
    	return -1;
    p = FGetTok(aFGetTokWk,fp);
    if (p == NULL)
    	return -1;
    *o1 = ChkP(p);
    if (*o1 < 0 || *o1 == *o0)
    	return -1;
    p = FGetTok(aFGetTokWk,fp);
    if (p == NULL)
    	return -1;
    *o2 = ChkP(p);
    if (*o2 < 0 || *o2 == *o0 || *o2 == *o1)
    	return -1;
    return 0;
}

_S void PalErr(void)
{
    PrExit("PALファイルがおかしい\n");
}

_S void GetPalFile(byte *fnam)
{
    FILE *fp;
    static byte *aFGetTokWk = NULL;
    int  n,mode;
    static byte *p;
    static int o0,o1,o2;

    mode = 0;
    n = 0;
    o0 = 0; o1 = 1; o2 = 2;
    fp = fopen_e(fnam,"r");
    while ((p = FGetTok(&aFGetTokWk,fp)) != NULL) {
    	if (stricmp(p,"N") == 0) {
    	    mode = 0;
    	    p = FGetTok(&aFGetTokWk,fp);
    	    if (ChkPPP(&aFGetTokWk,fp,p,&o0,&o1,&o2)) {
    	    	PalErr();
    	    }
    	} else if (ChkP(p) >= 0) {
    	    mode = 1;
    	    if (ChkPPP(&aFGetTokWk,fp,p,&o0,&o1,&o2)) {
    	    	PalErr();
    	    }
    	} else {
    	    if (mode == 0) {
    	    	n = StrToInt_0_15(&p);
    	    	p = FGetTok(&aFGetTokWk,fp);
    	    	gPal[n*3+o0] = StrToInt_0_15(&p);
    	    	p = FGetTok(&aFGetTokWk,fp);
    	    	gPal[n*3+o1] = StrToInt_0_15(&p);
    	    	p = FGetTok(&aFGetTokWk,fp);
    	    	gPal[n*3+o2] = StrToInt_0_15(&p);
    	/*printf("%d %d %d %d\n",n,gPal[n*3+o0],gPal[n*3+o1],gPal[n*3+o2]);*/
    	    } else if (n <= 15) {
    	    	gPal[n*3+o0] = StrToInt_0_15(&p);
    	    	p = FGetTok(&aFGetTokWk,fp);
    	    	gPal[n*3+o1] = StrToInt_0_15(&p);
    	    	p = FGetTok(&aFGetTokWk,fp);
    	    	gPal[n*3+o2] = StrToInt_0_15(&p);
    	    	n++;
    	    } else {
    	    	PalErr();
    	    }
    	}
    }
    fclose(fp);
}


/*--------------------------------------------------------------------------*/
_S void Dos_KbdIn(void)
{
    union REGS regs;

    regs.h.ah = 0x08;
    intdos(&regs,&regs);
}

_S void More(void)
{
    pr("[more]");
    Dos_KbdIn();
    pr("\b\b\b\b\b\b      \b\b\b\b\b\b");
}

_S void Usage(void)
{
    pr(PRGB " by てんか☆\n");
    pr("   ･ファイルの指定した位置よりパレット・データ(32or48ﾊﾞｲﾄ)を抜き出し、\n");
    pr("    (常駐)パレットを変更したり、RGB ファイルに出力したりするプログラム.\n");
    pr("   ･入力ファイルとして.PAL, .ALG ファイルにも対応.\n");
    pr("   ･ファイル名の拡張子が省略されると .RGB が指定されたことになる.\n");
    pr("\n");
    pr("usage: prgb [-opts] [file(s)]...\n");
    pr("\n");
    pr("@OPTFILE  ファイル OPTFILE よりオプションを入力\n");
    pr("-d        パレットの表示\n");
    pr("-i        パレットの初期化\n");
    pr("-8        パレットをデジタル８色に変更\n");
  #ifdef PC98
    pr("-q-       パレットの設定を行わない（９８依存部を利用しない）\n");
  #endif
    pr("-v[N]     パレットのトーンを N % にする. 省略 N=50\n");
    pr("-l[N]     N が正または 0ならパレットの値をファイルの先頭 Nバイト目より収得\n");
    pr("          N が負ならばファイルの最後から Nバイト目より収得. N省略は 0\n");
    pr("-x[N]     RGB の並びを変更. N= 0:RGB 1:BRG 2:GBR 3:GRB 4:BGR 5:RBG\n");
    /*pr("  	    0:RGB,012 1:BRG,201 2:GBR,120 3:GRB,102 4:BGR,210 5:RBG,021\n");*/
    pr("-r[FILE]  パレットを RGB ファイル FILE.rgb に出力. -vN 指定時はトーンを計算\n");
    pr("          FILE省略時は、ファイルの拡張子を.RGBにしたものに出力. -wrでもよい\n");
    pr("-wp,-wa   RGBでなく PAL,ALG ファイルに出力する以外は -r(-wr)と同様.\n");
  #if 0
    pr("-wf[FILE] -r と同様だが、RGBファイルでなくFRMファイルのRGBパレット部を変更\n");
  #endif
    pr("-eEXT     省略時の拡張子を EXT にする(EXT は3文字までの文字列)\n");
    pr("-cN,R,G,B パレット N を変更  N,R,G,B:0-15,A-F\n");
    More();
    pr("-m[M][,Nr,Ng,Nb] ファイルからのパレット・データの収得の仕方\n");
    pr("          M=0:48バイト RGBデータ. 各バイトの下位４ビットを利用\n");
    pr("            1:48バイト RGBデータ. 各バイトの上位４ビットを利用\n");
    pr("            2:32バイト. 1ﾜｰﾄﾞ(0000GGGGRRRRBBBB)*16. ﾜｰﾄﾞはｲﾝﾃﾙ形式\n");
    pr("            3:32バイト. 1ﾜｰﾄﾞ(0000GGGGRRRRBBBB)*16. ﾜｰﾄﾞはﾓﾄﾛｰﾗ形式\n");
    pr("            4:24バイト. 3byteｾｲｽｳ*8. G1R1B1G0R0B0. ｲﾝﾃﾙ形式\n");
    pr("            5:24バイト. 3byteｾｲｽｳ*8. G1R1B1G0R0B0. ﾓﾄﾛｰﾗ形式\n");
    pr("          M=2,3では [Nr,Ng,Nb] が指定できる\n");
    pr("          １ワード中の指定された位置の４ビットを R,G,B の値にする\n");
    pr("          Nr,Ng,Nbはワード下位からの各R,G,B データの最下位ビットの位置...\n");
    pr("          別のいいかたをすれば, Nビット右シフトしてから下４ビットとりだす\n");
    pr("          Nr,Ng,Nb 省略時は 4,8,0 が指定されたことになる\n");
    pr("          M=4,5では、M=2,3と同様に６個指定する.\n");
    pr("		  -m[M][,Nr0,Ng0,Nb0,Nr1,Ng1,Nb1] で省略は -m[M],4,8,0,16,20,12\n");
    Exit(0);
}

_S void OptsErr(byte *s)
{
    printf("%s の指定がおかしい\n",s);
    Exit(1);
}

_S void Option(byte *p)
{
    int c,t;
    static byte *pp;

    c = *p++;
    c = toupper(c);
    switch (c) {
    case '?':
    case '\0':
    	Usage();
    case 'I':
    	gInitFlg = 1;
    	break;
    case '8':
    	gInitFlg = 2;
    	break;
    case 'V':
    	gToneFlg = 1;
    	gTone = 50;
    	if (*p == '=')
    	    p++;
    	if (*p)
    	    gTone = (word)strtol(p,NULL,10);
    	if (gTone > 200)
    	    OptsErr("-v");
    	break;
    case 'D':
    	gDspFlg = 1;
    	break;
  #ifdef PC98
    case 'Q':
    	gPc98Flg = 1;
    	if (*p == '-' || *p == '0')
    	    gPc98Flg = 0;
    	break;
  #endif
    case 'R':
    	gRgbMode = 1;
    	if (*p == '=')
    	    p++;
    	if (*p)
    	    gRgbName = p;
    	break;
    case 'W':
    	c = *p++;
    	if (*p == '=')
    	    p++;
    	if (*p)
    	    gRgbName = p;
    	c = toupper(c);
    	switch(c) {
    	case 'R':
    	    gRgbMode = 1;
    	    break;
    	case 'F':
    	    gRgbMode = 2;
    	    break;
    	case 'P':
    	    gRgbMode = 3;
    	    break;
    	case 'A':
    	    gRgbMode = 4;
    	    break;
    	default:
    	    OptsErr("-w");
    	}
    	break;
    case 'L':
    	if (*p == '=')
    	    p++;
    	gSeekPos = strtol(p,NULL,0);
    	break;
    case 'X':
    	if (*p == '=')
    	    p++;
    	gRgbOfsNo = (int)strtol(p,NULL,0);
    	if (gRgbOfsNo < 0 || gRgbOfsNo > 5)
    	    OptsErr("-x");
    	break;
    case 'C':
    	if (*p == '=')
    	    p++;
    	if (gChgPalCnt >= 16)
    	    PrExit("-cオプションの数が多すぎる\n");
    	pp = p;
    	gChgPalNo[gChgPalCnt] = StrToInt_0_15(&pp);
    	if (*pp++ != ',')
    	    OptsErr("-t");
    	gChgPal[gChgPalCnt][0] = StrToInt_0_15(&pp);
    	if (*pp++ != ',')
    	    OptsErr("-t");
    	gChgPal[gChgPalCnt][1] = StrToInt_0_15(&pp);
    	if (*pp++ != ',')
    	    OptsErr("-t");
    	gChgPal[gChgPalCnt][2] = StrToInt_0_15(&pp);
    	gChgPalCnt++;
    	break;
    case 'E':
    	if (*p == '=')
    	    p++;
    	gDfltExt = p;
    	if (strlen(p) > 3)
    	    OptsErr("-e");
    	break;
    case 'M':
    	if (*p == '=')
    	    p++;
    	t = (int)strtoul(p,&(char *)pp,10);
    	switch (t) {
    	case 0:
    	    gPalSiz = 48;
    	    gH4bFlg = 0;
    	    gMtlFlg = 0;
    	    break;
    	case 1:
    	    gPalSiz = 48;
    	    gH4bFlg = 1;
    	    gMtlFlg = 0;
    	    break;
    	case 2:
    	    gPalSiz = 32;
    	    gH4bFlg = 0;
    	    gMtlFlg = 0;
    	    break;
    	case 3:
    	    gPalSiz = 32;
    	    gH4bFlg = 0;
    	    gMtlFlg = 1;
    	    break;
    	case 4:
    	    gPalSiz = 24;
    	    gH4bFlg = 0;
    	    gMtlFlg = 0;
    	    break;
    	case 5:
    	    gPalSiz = 24;
    	    gH4bFlg = 0;
    	    gMtlFlg = 1;
    	    break;
    	default:
    	    OptsErr("-m");
    	}
    	gPSftB = 0;
    	gPSftR = 4;
    	gPSftG = 8;
    	gPSftB2 = 12;
    	gPSftR2 = 16;
    	gPSftG2 = 20;
    	if ((t >= 2) && *p++ == ',') {
    	    pp = p;
    	    gPSftR = StrToInt_0_15(&pp);
    	    if (*pp++ != ',')
    	    	OptsErr("-m");
    	    gPSftG = StrToInt_0_15(&pp);
    	    if (*pp++ != ',')
    	    	OptsErr("-m");
    	    gPSftB = StrToInt_0_15(&pp);
    	    if ((t >= 4) && *pp++ == ',') {
    	    	gPSftR2 = StrToInt(&pp);
    	    	if (*pp++ != ',')
    	    	    OptsErr("-m");
    	    	gPSftG2 = StrToInt(&pp);
    	    	if (*pp++ != ',')
    	    	    OptsErr("-m");
    	    	gPSftB2 = StrToInt(&pp);
    	    }
    	}
    	break;
    default:
    	PrExit("指定されたオプションがおかしいよん");
    }
}

void FilOpt(byte *name)
{
    FILE *fp;
    byte *p;
    static byte *aFGetTokWk = NULL;

    fp = fopen_e(name,"r");
    while ((p = FGetTok(&aFGetTokWk,fp)) != NULL) {
    	if (*p == '-')
    	    Option(p+1);
    	else {
    	    PrExit("オプションファイル中にはファイルは指定できません\n");
    	}
    }
    fclose(fp);
}

_S void KeyBrk(int sig)
{
    sig = 1;
    Exit(sig);
}

void main(int argc,byte *argv[])
{
    byte *p;
    int  i,c;
    static word tone;
    static byte nambuf[130],name[130];

    signal(SIGINT,KeyBrk);
    if (argc < 2)
    	Usage();
    for (c = i = 1;i < argc;i++) {
    	p = argv[i];
    	if (*p == '-')
    	    Option(p+1);
    	else if (*p == '@')
    	    FilOpt(p+1);
    	else
    	    c = 0;
    }
  #ifdef PC98
    if (gPc98Flg) {
    	AnalogPal();
    	if (gInitFlg)
    	    PalInit(gInitFlg - 1);
    	RPal_Get(&tone,gPal);
    }
    if (c) {
    	if (gToneFlg == 0)
    	    gTone = tone;
    	if (gPc98Flg == 0)
    	    PrExit("-q- のときはファイルを指定してね\n");
    	ChgRgb(NULL);
    	Exit(0);
    }
  #else
    if (gInitFlg)
    	PalInit(gInitFlg - 1);
    if (c)
    	PrExit("ファイルを指定してください\n");
  #endif
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-' || *p == '@')
    	    continue;
    	AddExt(nambuf,p,gDfltExt);
    	if (DirEntryGet(name,nambuf,0) == 0) {
    	    do {
    	    	if (EquExt(name,"PAL"))
    	    	    GetPalFile(name);
    	    	else if (EquExt(name,"ALG"))
    	    	    GetAlgFile(name);
    	    	else
    	    	    GetRgbFile(name);
    	    	ChgRgb(name);
    	    } while (DirEntryGet(name,NULL,0) == 0);
    	} else {
    	    PrExit("指定されたファイルがみつからないよぉ\n");
    	}
    }
    Exit(0);
}
