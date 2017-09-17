/*
	cb2tt で用いられる DYNA 画像の読みこみを
	行います。

	int=32bitコンパイラを前提.
 */

#include "dp_add.h"			/* dpixed-add-inの定義 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>


/* ----------------------------------------------------------------------- */
#define CPU_X86

#ifdef __cplusplus
#define EXTRN_C		extern "C"
#else
#define EXTRN_C
#endif

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

#define	GR_ARGB(a,r,g,b)			((((unsigned char)(a))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define GR_SET_ARGB(q, a,r,g,b) 	(*((int*)(q)) = GR_ARGB(a,r,g,b))		// リトルエンディアン依存
#define	GR_WID2BYT(w,bpp)			(((bpp) > 24) ? ((w)<<2) : ((bpp) > 16) ? ((w)*3) : ((bpp) > 8) ? ((w)<<1) : ((bpp) > 4) ? (w) : ((bpp) > 1) ? (((w)+1)>>1) : (((w)+7)>>3))
#define GR_WID2BYT4(w,bpp)			((GR_WID2BYT(w,bpp) + 3) & ~3)


/* ----------------------------------------------------------------------- */

static void dyna_fnt_read(BYTE *src, BYTE *dst, int size);
static void *file_load(char *name, void *buf, int bufsz, int *rdszp);


EXTERN_C __declspec(dllexport) BOOL DPGetInfo(DP_ADDININFO *pInfo)
{
	//この Add-In に関する情報を D-Pixed へ返す
	pInfo->dwType 			= DPADDIN_DECODER;				// プラグインタイプ
	pInfo->name   			= "DYNA Loader";				// ツール名
	pInfo->copyright		= "2000(C) tenk*";				// 著作権表示
	pInfo->addinDescription = "DYNA 2値1024x1024画 読込";	// 説明
	pInfo->version 			= "0.01";						// バージョン
	pInfo->description 		= "DYNA File(*.dyn)";			// ツール説明
	pInfo->defExt			= "dyn";						// 拡張子指定
	return TRUE;
}


EXTERN_C __declspec(dllexport) BOOL DPDecode(LPCSTR fname, DP_ENCDECINFO *pi)
{
	// d-pixed 用画像ファイルローダ
	LPBITMAPINFOHEADER bm;					// BMP/DIB header
	RGBQUAD *clut;
	BYTE 	*p, *src;
	int  	w, h, sz;

	// データファイルの読み込み
	src = (BYTE*)file_load((char*)fname, NULL, 0, &sz);
	if (src == NULL) {
		MessageBox(NULL, "メモリが足りません！", "DYNA Load", MB_OK);
		return FALSE;
	}
	w  = 1024;
	h  = 1024;

	// アドインの準備
	pi->dwFlag   = DPDECENC_KEYCOLOR|DPDECENC_BACKGROUNDCOLOR;	// 透明色,背景色が有効
	pi->keyColor = 0;											// 透明色番号
	pi->bgColor  = 0;											// 背景色番号
	pi->nPicture = 1;											// 絵の枚数

	pi->pDibBuf = (LPBITMAPINFOHEADER*)calloc(1, 1 * sizeof(LPBITMAPINFOHEADER));
	pi->pBitBuf = (LPBYTE*)calloc(1, 1 * sizeof(LPBYTE));
	p = (BYTE*)calloc(1, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) + 1024*1024);
	if (p == NULL || pi->pBitBuf == NULL || pi->pDibBuf == NULL) {
		MessageBox(NULL, "メモリが足りません", "DYNA Load", MB_OK);
		return FALSE;
	}
	bm				= (LPBITMAPINFOHEADER)p;
	pi->pDibBuf[0]	= bm;
	clut			= (RGBQUAD*)(p + sizeof(BITMAPINFOHEADER));
	p				= p + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
	pi->pBitBuf[0]	= p;

	// DIB ヘッダの設定
	bm->biBitCount 		= 8;
	bm->biClrUsed  		= 256;

	bm->biSize	 		= sizeof(BITMAPINFOHEADER);
	bm->biWidth	 		= w;
	bm->biHeight	 	= h;
	bm->biPlanes	 	= 1;
	bm->biCompression	= BI_RGB;
	bm->biSizeImage		= GR_WID2BYT4(w,8) * h;
	bm->biXPelsPerMeter	= 0;
	bm->biYPelsPerMeter	= 0;
	bm->biClrImportant	= 0;

	// とりあえずのパレット設定。DYNA自体にはパレットなんてないので
	GR_SET_ARGB(&clut[0], 0,0x00,0x00,0x00);
	GR_SET_ARGB(&clut[1], 0,0xFF,0xFF,0xFF);

	// DYNA データの展開
	dyna_fnt_read(src, p, sz);
	return TRUE;
}


static void dyna_fnt_read(BYTE *src, BYTE *dst, int size)
{
	BYTE *s, *e, *d;
	int x,y,x1,x2,ry;

	memset(dst, 0,1024*1024/8);
	s = src + 4;
	e = s + size - 4;
	while (s+6 < e) {
		y  = PEEKiW(s);
		if (y >= 1023) {
			y = 1023;
		}
		ry = 1023 - y;		// (普通の)BMPへの変換なので上下さかさまにする
		x1 = PEEKiW(s+2);
		if (x1 >= 1023) {
			x1 = 1023;
		}
		x2 = PEEKiW(s+4);
		if (x2 >= 1023) {
			x2 = 1023;
		}
		d  = &dst[ry*1024];
		for (x = x1; x <= x2; x++) {
			d[x] = 1;
		}
		s += 6;
	}
}


static void *file_load(char *name, void *buf, int bufsz, int *rdszp)
{
	/* name : 読みこむファイル */
	/* buf  : 読みこむメモリ。NULLが指定されれば mallocし、16バイト余分に確保する */
	/* bufsz: bufのサイズ。0が指定されれば ファイルサイズとなる */
	/* rdszp: NULLでなければ、読みこんだファイルサイズを入れて返す */
	/* 戻り値: bufのアドレスかmallocされたアドレス. エラー時はNULLを返す */
	FILE *fp;
	int  l;

	fp = fopen(name, "rb");
	if (fp == NULL)
		return NULL;
	l = filelength(fileno(fp));
	if (rdszp)
		*rdszp = l;
	if (bufsz == 0)
		bufsz = l;
	if (l > bufsz)
		l = bufsz;
	if (buf == NULL) {
		bufsz = (bufsz + 15 + 16) & ~15;	//16バイト保険的に余分に確保。
		buf = calloc(1, bufsz);
		if (buf == NULL)
			return NULL;
	} 
	fread(buf, 1, l, fp);
	if (ferror(fp)) {
		fclose(fp);
		buf = NULL;
	}
	fclose(fp);
	return buf;
}


