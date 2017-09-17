/*--------------------------------------------------------------------------*/
/* BMP 画像データ出力														*/
/*  ※  入出力で色数や画像サイズの違う場合の圧縮時に malloc&freeを使用		*/
/* by tenk																	*/
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"


#if 0	// 別にヘッダを使う場合
#include "def.h"
#else	// このファイルのみで閉じる場合
typedef	unsigned char  Uint8;
typedef	unsigned short Uint16;
typedef	unsigned       Uint32;
#define	inline			__inline
#define CPU_X86			// リトルエンディアンかつ多バイト整数のアドレスのアライメントを無視できるときに定義

#define	BB(a,b)			((((Uint8)(a))<<8)+(Uint8)(b))
#define	WW(a,b)			((((Uint16)(a))<<16)+(Uint16)(b))
#define	BBBB(a,b,c,d)	((((Uint8)(a))<<24)+(((Uint8)(b))<<16)+(((Uint8)(c))<<8)+((Uint8)(d)))

#define	GLB(a)			((unsigned char)(a))
#define	GHB(a)			GLB(((unsigned short)(a))>>8)
#define	GLW(a)			((unsigned short)(a))
#define	GHW(a)			GLW(((unsigned long)(a))>>16)

/* バイト単位のアドレスから値を取り出すメモリアクセス用マクロ */
#define	PEEKB(a)		(*(const unsigned char  *)(a))
#define	POKEB(a,b)		(*(unsigned char  *)(a) = (b))

#ifdef CPU_X86
#define	PEEKiW(a)		(*(const unsigned short *)(a))
#define	PEEKiD(a)		(*(const unsigned long  *)(a))
#define	POKEiW(a,b)		(*(unsigned short *)(a) = (b))
#define	POKEiD(a,b)		(*(unsigned       *)(a) = (b))
#else
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#endif

#define	PEEKmW(a)		( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )
#define	PEEKmD(a)		( (PEEKmW(a)<<16) | PEEKmW((const char *)(a)+2) )
#define	POKEmW(a,b)		(POKEB((a),GHB(b)), POKEB((char *)(a)+1,GLB(b)))
#define	POKEmD(a,b)		(POKEmW((a),GHW(b)), POKEmW((char *)(a)+2,GLW(b)))

#ifdef BIG_ENDIAN
#define	PEEKW(a)		PEEKmW(a)
#define	PEEKD(a)		PEEKmD(a)
#define	POKEW(a,b)		POKEmW(a,b)
#define	POKED(a,b)		POKEmD(a,b)
#else /* LITTLE_ENDIAN */
#define	PEEKW(a)		PEEKiW(a)
#define	PEEKD(a)		PEEKiD(a)
#define	POKEW(a,b)		POKEiW(a,b)
#define	POKED(a,b)		POKEiD(a,b)
#endif

#define	BPP2BYT(bpp)	(((bpp) > 24) ? 4 : ((bpp) > 16) ? 3 : ((bpp) > 8) ? 2 : 1)
#define	WID2BYT(w,bpp)	(((bpp) > 24) ? (w)<<2 : ((bpp) > 16) ? (w)*3 : ((bpp) > 8) ? (w)<<1 : ((bpp) > 4) ? (w) : ((bpp) > 1) ? ((w+1)>>1) : ((w+7)>>3))
#define WID2BYT4(w,bpp)	((WID2BYT(w,bpp) + 3) & ~3)
#define	BYT2WID(w,bpp)	(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 1) ? ((w)<<1) : ((w)<<3))

#endif




/*---------------------------------------------------------------------------*/
/* デバッグ・ログ・ルーチン */

#if	1	//def NDEBUG
#define	DBG_F(n)
#else
#include <stdarg.h>
#define DBG_F(n)		(dbgf n)

static void dbgf(char *fmt, ...)
{
	FILE *fp;
	va_list app;

	fp = stdout;//	fp = fopen("dbg.txt", "at");
	if (fp) {
		va_start(app, fmt);
		vfprintf(fp, fmt, app);
		va_end(app);
		//fclose(fp);
	}
}
#endif



/*--------------------------------------------------------------------------*/
/* 出力のための書きこみ処理(エンディアン対策)								*/


#ifdef CPU_X86	// リトルエンディアンかつ、Uint16,Uint32のアライメントを無視できるとき
// 汎用ではなく、このファイルのみで有効なマクロ。
// 引数 a は必ずUint8*型変数へのポインタであること!
// mput_b3()の c は副作用のない値であること！

#define mput_b1(a, c)		(*((*a)++) = (c))
#define mput_b2(a, c)		(*(Uint16*)(*a) = (c),  (*a) += 2)
#define mput_b3(a, c)		(*(Uint16*)(*a) = (c),  (*a)[2] = (c)>>16, (*a) += 3)
#define mput_b4(a, c)		(*(Uint32*)(*a) = (c),  (*a) += 4)
#define mput_cpy(d,s,l,n)	memcpy((d),(s),(l)*(n))

//#define mput_cpy1(d,s,l)	memcpy(d, s, (l))
//#define mput_cpy2(d,s,l)	MEMCPY2(d, s, (l)*2)
//#define mput_cpy3(d,s,l)	memcpy(d, s, (l)*3)
//#define mput_cpy4(d,s,l)	MEMCPY4(d, s, (l)*4)

#else

static inline void mput_b1(void **a, int c)
{
	// 1バイトメモリ書きこみ
	Uint8 *d = (Uint8*)*a;
	//DBG_F(("b1:%x  %02x\n", d, c));
	*d++ = (Uint8)c;
	*a   = (void*)d;
}

static inline void mput_b2(void **a, int c)
{
	// 2バイトメモリ書きこみ
	Uint8 *d = (Uint8*)*a;
	//DBG_F(("b2:%x  %04x\n", d, c));
	*d++ = (Uint8)c;
	*d++ = (Uint8)(c>>8);
	*a   = (void*)d;
}

static inline void mput_b3(void **a, int c)
{
	// 3バイトメモリ書きこみ
	Uint8 *d = (Uint8*)*a;
	//DBG_F(("b3:%x  %06x\n", d, c));
	*d++ = (Uint8)c;
	*d++ = (Uint8)(c>>8);
	*d++ = (Uint8)(c>>16);
	*a   = (void*)d;
}

static inline void mput_b4(void **a, int c)
{
	// 4バイトメモリ書きこみ
	Uint8 *d = (Uint8*)*a;
	//DBG_F(("b4:%x  %08x\n", d, c));
	*d++ = (Uint8)c;
	*d++ = (Uint8)(c>>8);
	*d++ = (Uint8)(c>>16);
	*d++ = (Uint8)(c>>24);
	*a   = (void*)d;
}

static inline void mput_cpy(void *dst, void *src, int len, int n)
{
	Uint8 *d = (Uint8*)dst;
	if (n == 4) {
		Uint32 *s = (Uint32*)src;
		do {
			mput_b4(&d, *s++);
		} while (--len);
	} else if (n == 2) {
		Uint16 *s = (Uint16*)src;
		do {
			mput_b2(&d, *s++);
		} while (--len);
	//} else if (n == 3) {
	//	Uint8 *s = (Uint8*)src;
	//	do {
	//		int c = BBBB(0, s[2], s[1], s[0]);
	//		mput_b3(&d, c);
	//		s += 3;
	//	} while (--len);
	} else {
		memcpy(d, src, len * n);
	}
}

#endif



/*--------------------------------------------------------------------------*/
static int bmp_putPixs(Uint8 *dst, int dst_w, int h, int dstBpp, Uint8 *src, int src_wb, int srcBpp, Uint32 *clut, int dir);
static void bmp_getDfltClut(int bpp, Uint32 *clut);


int  bmp_write(void *bmp_data, int w, int h, int bpp, void *src, int src_wb, int srcBpp, void *clut0, int dir)
{
	return bmp_writeEx(bmp_data, w, h, bpp, src, src_wb, srcBpp, clut0, 1<<srcBpp, dir);
}


int  bmp_writeEx(void *bmp_data, int w, int h, int dstBpp, void *src, int src_wb, int srcBpp, void *clut0, int clutNum, int dir)
{
	enum {BMP_TOP_HDR_SZ = 2+12};
	Uint32 		 clut1[256];
	Uint8 *d     = bmp_data;
	Uint32 *clut = clut0;
	//int   cm  = (dir & 0x80) ? 8 : 0;	// 圧縮モードは未対応
	//int   dn  = BPP2BYT(dstBpp);
	int   sn  = BPP2BYT(srcBpp);
	int   c, i;

	dir = (dir & 1) ;
	src_wb = (src_wb) ? src_wb : (w * sn + 3) & ~3;

	//DBG_F(("dstBpp = %d dir=%x dr=%x\n",dstBpp,dir,dr));

	/* ヘッダ作成 */
	mput_b1(&d, 'B');
	mput_b1(&d, 'M');
	c = BMP_TOP_HDR_SZ + 40;
	mput_b4(&d, c);							/* ファイルサイズ */
	mput_b4(&d, 0);							/* 予約 */
	if (dstBpp <= 8)
		c = BMP_TOP_HDR_SZ + 40 + (1<<dstBpp)*4;
	else
		c = BMP_TOP_HDR_SZ + 40;
	mput_b4(&d, c);							/* ピクセル・データへのオフセット */
	mput_b4(&d, 40);						/* ヘッダサイズ */
	mput_b4(&d, w);
	mput_b4(&d, h);
	mput_b2(&d, 1);							/* プレーン数. must be 1 */
	mput_b2(&d, dstBpp);						/* bits per pixel */
  #if 1
	mput_b4(&d, 0);							/* 圧縮モード. 無圧縮なら0 */
	mput_b4(&d, 0);							/* 圧縮データサイズ. 無圧縮なら0 */
  #endif
	mput_b4(&d, 0);							/* x resolution */
	mput_b4(&d, 0);							/* y resolution */
	mput_b4(&d, 0);							/* clutの数. 2,4,8ビット色ではこの値に関係なく1<<n色ある. 普通 0でよい */
	mput_b4(&d, 0);							/* 重要な色の数……普通無視するデータ. 普通 0でよい. */

	/* パレット生成 --------------- */
	if (dstBpp <= 8) {
		if (srcBpp > 8 || clut == NULL) {
			clut = clut1;
			bmp_getDfltClut(dstBpp, clut);
		}
		for (i = 0; i < (1<<dstBpp); i++) {
			c = (i < clutNum) ? clut[i] : 0;
			mput_b4(&d, c);
		}
	}

	/*if (cm == 0)*/ {	//無圧縮
		bmp_putPixs(d, w, h, dstBpp, src, src_wb, srcBpp, clut, dir);
		d += WID2BYT4(w, dstBpp) * h;
	}
	return (int)d - (int)bmp_data;
}


static void bmp_getDfltClut(int bpp, Uint32 *clut)
{
	int i,c;

	if (clut == NULL)
		return;
	if (bpp == 1) {			// 黒白
		clut[0] = 0;
		clut[1] = 0xFFFFFF;
	} else if (bpp == 4) {	// 単色濃淡
		for (i = 0; i < 16; i++) {
			c = (i << 4) | i;
			clut[i] = BBBB(0, c,c,c);
		}
	} else {				// TOWNSや88VA系の G3R3B2
		for (i = 0; i < 256; i++)
			clut[i] = BBBB(0, (i&0x1C)<<3, (i&0xE0), (i&3)<<6);
	}
}



static inline int	 GetPx1(Uint8 **sp, int srcBpp, Uint32 *clut)
{
	Uint8 *s = *sp;
	int c,r,g,b,a;

	if (srcBpp <= 8) {
		c = clut[*s++];
	} else if (srcBpp <= 15) {
		c  = PEEKW(s);
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		c = BBBB(0, r, g, b);
		s += 2;
	} else if (srcBpp <= 16) {
		c  = PEEKW(s);
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		a  = (c & 0x8000) ? 0x80 : 0;
		c = BBBB(a, r, g, b);
		s += 2;
	} else if (srcBpp <= 24) {
		c = BBBB(0, s[2], s[1], s[0]);
		s += 3;
	} else {
		c = PEEKD(s);
		s += 4;
	}
	*sp = s;
	return c;
}


static inline void	 PutPx1(Uint8 **dp, int c, int dstBpp)
{
	Uint8 *d = *dp;
	int   r, g, b, a;

	if (dstBpp == 8) {
		r = (Uint8)(c >> 16);
		g = (Uint8)(c >>  8);
		b = (Uint8)(c >>  0);
		mput_b1(&d, ((g >> 5)<<5) | ((r >> 5)<<2) | (b >> 6));
	} else if (dstBpp == 15) {
		r = (Uint8)(c >> 16);
		g = (Uint8)(c >>  8);
		b = (Uint8)(c >>  0);
		c = ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		mput_b2(&d, c);
	} else if (dstBpp == 16) {
		a = (c >> 24) ? 0x8000 : 0;
		r = (Uint8)(c >> 16);
		g = (Uint8)(c >>  8);
		b = (Uint8)(c >>  0);
		c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		mput_b2(&d, c);
	} else if (dstBpp == 24) {
		mput_b3(&d, c);
	} else {
		mput_b4(&d, c);
	}
	*dp = d;
}




static inline Uint8 *bmp_putPix32(Uint8 *d, int c, int bpp)
{
	int r,g,b,a;

	if (bpp <= 15) {
		r = (Uint8)(c >> 16);
		g = (Uint8)(c >>  8);
		b = (Uint8)(c >>  0);
		c = ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		*(Uint16*)d = c;
		return d + 2;
	} else if (bpp <= 16) {
		a = (c >> 24) ? 0x8000 : 0;
		r = (Uint8)(c >> 16);
		g = (Uint8)(c >>  8);
		b = (Uint8)(c >>  0);
		c = a | ((r >> 3)<<10) | ((g >> 3)<<5) | (b >> 3);
		*(Uint16*)d = c;
		return d + 2;
	} else if (bpp <= 24) {
		d[0] = (Uint8)c;
		d[1] = (Uint8)(c>>8);
		d[2] = (Uint8)(c>>16);
		return d + 3;
	} else {
		*(Uint32*)d = c;
		return d + 4;
	}
}



static inline int bmp_getPix32(const Uint8 *s, int x, int bpp, Uint32 *clut, int ofs)
{
	int c,r,g,b,a;

	if (bpp <= 1) {
		int n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);		//c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
		return clut[c + ofs];
	} else if (bpp <= 4) {
		c = (s[x>>1] >> ((((x&1)^1)<<2))) & 0x0f;	//c = (x & 1) ? s[x>>1] & 0x0f : (Uint8)s[x>>1] >> 4;
		return clut[c + ofs];
	} else if (bpp <= 8) {
		return clut[s[x] + ofs];
	} else if (bpp <= 16) {
		s += x * 2;
		c  = (s[1]<<8) | s[0];
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		a  = (c & 0x8000) ? 0x80 : 0;
		return  BBBB(a, r,g,b);
	} else if (bpp <= 24) {
		s += x * 3;
		return BBBB(0, s[2], s[1], s[0]);
	} else {
		s += x * 4;
		return BBBB(s[3], s[2], s[1], s[0]);
	}
}



static inline int bmp_getPix8(const Uint8 *s, int x, int bpp, Uint32 *clut, int ofs)
{
	// 多色の 256色化は G3R3B2 形式への簡易変換.
	int c,r,g,b;

	if (bpp <= 1) {
		int n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);		//c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
		return c + ofs;
	} else if (bpp <= 4) {
		c = (s[x>>1] >> ((((x&1)^1)<<2))) & 0x0f;	//c = (x & 1) ? s[x>>1] & 0x0f : (Uint8)s[x>>1] >> 4;
		return c + ofs;
	} else if (bpp <= 8) {
		return s[x] + ofs;
	} else if (bpp <= 16) {
		s += x * 2;
		c  = s[1]<<8 | s[0];
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		return  (((r>>5)&7)<<2) |(((g>>5)&7)<<5)| ((b>>6)&3);
	} else if (bpp <= 24) {
		s += x * 3;
		return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);
	} else {
		s += x * 4;
		return (((s[2]>>5)&7)<<2) | (((s[1]>>5)&7)<<5) | ((s[0]>>6)&3);
	}
}


static int bmp_getPix4(const Uint8 *s, int x, int bpp, Uint32 *dmy_clut, int ofs)
{
	// 多色の 256色化は G3R3B2 形式への簡易変換.
	int c,r,g,b;

	if (bpp <= 1) {
		//c = (s[x>>3] & (0x80 >> (x&7))) ? 1 : 0;
		int n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);
		return c + ofs;
	} else if (bpp <= 4) {
		c = (s[x>>1] >> ((((x&1)^1)<<2)));
		return (c + ofs) & 0x0f;
	} else if (bpp <= 8) {
		return (s[x] + ofs) & 0x0f;
	} else if (bpp <= 16) {
		s += x * 2;
		c  = PEEKiW(s);
		b  = ((c      ) & 0x1f) << 3;
		g  = ((c >>  5) & 0x1f) << 3;
		r  = ((c >> 10) & 0x1f) << 3;
		return (g * 9 + r * 5 + b * 2) >> 8;
	} else if (bpp <= 24) {
		s += x * 3;
		return (s[1] * 9 + s[2] * 5 + s[0] * 2) >> 8;
	} else {
		s += x * 4;
		return (s[1] * 9 + s[2] * 5 + s[0] * 2) >> 8;
	}
}



static int bmp_getPix1(const Uint8 *s, int x, int bpp, Uint32 *dmy_clut, int ofs)
{
	// 多色の 256色化は G3R3B2 形式への簡易変換.
	int c, n;

	if (bpp <= 1) {
		n = x & 7;
		c = (s[x>>3] & (0x80 >> n)) >> (7-n);
		//c = (x & 1) ? s[x>>1] & 0x0f : (Uint8)s[x>>1] >> 4;
		return (c+ofs)&1;
	} else if (bpp <= 4) {
		c = s[x>>1] >> ((((x&1)^1)<<2));
		return (c + ofs)&1;
	} else if (bpp <= 8) {
		return (s[x] + ofs) & 1;
	} else if (bpp <= 16) {
		s += x * 2;
		return (PEEKiW(s) > 0);
	} else if (bpp <= 24) {
		s += x * 3;
		return BBBB(0,s[2],s[1],s[0]) > 0;
	} else {
		s += x * 4;
		return (PEEKiD(s) > 0);
	}
}



static int bmp_putPixs(Uint8 *dst, int dst_w, int h, int dstBpp, Uint8 *src, int src_wb, int srcBpp, Uint32 *clut, int dir)
{
	// srcBpp色 w*hドットのsrc 画像を、dstBpp色 w*h の dst画像に変換する.
	// dir : bit0:ピクセル順は0=上から 1=下から   bit1:0=左から 1=右から
	enum {ofs=0};
	const Uint8 *s;
	Uint8 *d;
	int  c, dpat, spat, dst_wb;
	int  x,y, y0, y1, yd, w;

DBG_F(("%x,%d,%d,%d,%x,%d,%d,%x,%d\n",dst,dst_w,h,dstBpp,src,src_wb,srcBpp,clut,dir));
	if (dst == NULL || src == NULL || dst_w == 0 || h == 0 || src_wb == 0) {
		return 0;
	}
	spat = src_wb;
	w = BYT2WID(src_wb, srcBpp);
	dst_wb = WID2BYT4(dst_w, dstBpp);
	dpat = dst_wb - WID2BYT(w, dstBpp);
//	if (dpat < 0) {
//		w    = BYT2WID(dst_wb, dstBpp);
//		dpat = dst_wb - WID2BYT(w, dstBpp);
//	}
DBG_F(("spat=%d, dpat=%d, w=%d, dst_wb=%d, srcBpp=%d, dstBpp=%d\n", spat, dpat, w, dst_wb, srcBpp,dstBpp));

	// 画像の向きの調整
	d   = (Uint8 *)dst;
	s   = (const Uint8 *)src;
	y0  = 0, y1 = h, yd = +1;
	if (dir & 1) {
		y0  = h-1, y1 = -1, yd = -1;
		d   = d + y0 * dst_wb;
		dpat = dpat - dst_wb * 2;
	}
	//DBG_F(("[%d, %d, %d, %d]\n", y0,y1,yd,dpat));

  #if 0 //ndef BIG_ENDIAN
	// 入出力が同じbppで変換等なく横幅4バイト単位であれば、素早くコピー
	if (dstBpp == srcBpp && ofs == 0 && (spat&3) == 0 && (dpat&3) == 0) {
		for (y = y0; y != y1; y += yd) {
			MEMCPY4(d, s, spat);
			d += spat + dpat;
			s += spat;
		}
		return 1;
	}
  #endif

	// 画像コピー
	if (dstBpp > 8) {	// 出力が多色のとき
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = bmp_getPix32(s, x, srcBpp, clut,ofs);
				d = bmp_putPix32(d, c, dstBpp);
			}
			d += dpat;
			s += spat;
		}
	} else if (dstBpp > 4) {	// 出力が256色のとき
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = bmp_getPix8(s, x, srcBpp, clut,ofs);
				*d++ = c;
			}
			d += dpat;
			s += spat;
		}
	} else if (dstBpp > 1) {	// 出力が 16色のとき
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = (Uint8)(bmp_getPix4(s, x, srcBpp, clut,ofs) << 4);
				if (++x < w)
					c |= bmp_getPix4(s, x, srcBpp, clut,ofs) & 0x0F;
				*d++ = c;
			}
			d += dpat;
			s += spat;
		}
	} else {	// 出力が 2色のとき
		for (y = y0; y != y1; y += yd) {
			for (x = 0; x < w; x ++) {
				c = bmp_getPix1(s, x, srcBpp, clut,ofs) << 7;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 6;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 5;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 4;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 3;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 2;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs) << 1;
				if (++x < w) c |= bmp_getPix1(s, x, srcBpp, clut,ofs);
				*d++ = c;
			}
			d += dpat;
			s += spat;
		}
	}
	return 1;
}

