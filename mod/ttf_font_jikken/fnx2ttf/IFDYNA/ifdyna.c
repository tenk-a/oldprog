/* Susie用 DYNA(2値 1024x1024) プラグ */

#define CPU_X86

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>


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


/* SUSIE プラグイン関係 -----------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)
/*typedef*/ struct PictureInfo {	/* Susie プラグイン用構造体 */
	long left,top;				/* 画像を展開する位置   */
	long width;					/* 画像の幅(pixel)      */
	long height;				/* 画像の高さ(pixel)    */
	WORD x_density;				/* 画素の水平方向密度   */
	WORD y_density;				/* 画素の垂直方向密度   */
	short colorDepth;			/* １画素当たりのbit数  */
	HLOCAL hInfo;				/* 画像内のテキスト情報 */
} /*PictureInfo*/;
#pragma pack(pop)

#define ER_OK             0	 /* 正常終了               */
#define ER_NO_FUNCTION   -1  /* その機能はインプリメントされていない */
#define ER_ABORT          1  /* コールバック関数が非0を返したので展開を中止した */
#define ER_NOT_SUPPROT    2  /* 未知のフォーマット     */
#define ER_OUT_OF_ORDER   3  /* データが壊れている     */
#define ER_NOT_ENOUGH_MEM 4  /* メモリーが確保出来ない */
#define ER_MEMORY         5  /* メモリーエラー（Lock出来ない、等）*/
#define ER_FILE_READ      6  /* ファイルリードエラー   */
/*#define ER_RESERVE      7*//* (予約)                 */
#define ER_ETC            8  /* 内部エラー             */


EXTRN_C __declspec(dllexport) int PASCAL GetPluginInfo(int infono, LPSTR buf, int buflen);
EXTRN_C __declspec(dllexport) int PASCAL GetPictureInfo(LPSTR buf, long len, unsigned flag, struct PictureInfo *lpInfo);
EXTRN_C __declspec(dllexport) int PASCAL IsSupported(LPSTR filename, DWORD dw);
EXTRN_C __declspec(dllexport) int PASCAL GetPicture(
		LPSTR buf, long len, unsigned flag, HLOCAL *pHBInfo, HLOCAL *pHBm,
		int (CALLBACK *lpPrgressCallback)(int,int,long),long lData
		);
EXTRN_C __declspec(dllexport) int PASCAL GetPreview(
		LPSTR buf, long len, unsigned flag, HANDLE *pHBInfo, HANDLE *pHBm,
		FARPROC lpPrgressCallback, long lData
		);


#define	WID2BYT(w,bpp)	(((bpp) > 24) ? ((w)<<2) : ((bpp) > 16) ? ((w)*3) : ((bpp) > 8) ? ((w)<<1) : ((bpp) > 4) ? (w) : ((bpp) > 1) ? (((w)+1)>>1) : (((w)+7)>>3))
#define WID2BYT4(w,bpp)	((WID2BYT(w,bpp) + 3) & ~3)

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

	fp = fopen("dbg.txt", "at");
	if (fp) {
		va_start(app, fmt);
		vfprintf(fp, fmt, app);
		va_end(app);
		fclose(fp);
	}
}
#endif
#define DBG_M()		DBG_F(("%s %d\n",__FILE__, __LINE__))


/*---------------------------------------------------------------------------*/
static void dyna_read(BYTE *src, BYTE *dst, int size);

#if 1
BOOL WINAPI   DllEntryPoint(HINSTANCE dummy_hInst, DWORD dummy_flag, LPVOID dummy_rsv)
#else
BOOL APIENTRY DllMain(HANDLE hInstance,  DWORD flag, LPVOID rsv)
#endif
{
  #if 0
    switch (flag) {
    case DLL_PROCESS_ATTACH: break;
    case DLL_THREAD_ATTACH:  break;
    case DLL_THREAD_DETACH:  break;
    case DLL_PROCESS_DETACH: break;
    }
  #endif
	return TRUE;
}

EXTRN_C __declspec(dllexport) int PASCAL GetPluginInfo(int infono, LPSTR buf, int buflen)
{
	static char *infoMsg[] = {
		"00IN",									/* 0   : Plug-in API ver. */
		"DYN to DIB ver 0.10 writen by tenk*",	/* 1   : About.. */
		"*.DYN", "DYNA",						/* 2,3 : 拡張子 & 形式名 */
		"",
  	};
	int l = 0;

	if ((unsigned)infono < 4 && buf) {
		l = strlen(infoMsg[infono]);
		if (l >= buflen)
			l = buflen - 1;
		buf[0] = 0;
		if (l > 0) {
			strncpy(buf, infoMsg[infono], l);
			buf[l] = 0;
		}
		DBG_F(("GetPluginInfo(%d) = %s, len=%d/%d\n", infono, buf, l,buflen));
	} else {
		DBG_F(("GetPluginInfo end : %d, %x, %d\n", infono, buf, buflen));
	}
	return l;
}


EXTRN_C __declspec(dllexport) int PASCAL IsSupported(LPSTR dummy_fname, DWORD dw)
{
	char buf[16];
	UCHAR *p;
	int  c,w,h,bpp;

	/* ファイルかメモリか */
	if ((dw & 0xFFFF0000) == 0) {	/* ファイルハンドル */
		ReadFile((HANDLE)dw, buf, 0x12, 0, NULL);
		p = (UCHAR*)buf;
	} else {
		p = (UCHAR*)dw;
	}
	/* DYNA の ID チェック */
	if (memcmp(p, "DYNA", 4) != 0) {
		DBG_F(("ヘッダチェックでエラー\n"));
		return 0;
	}
	DBG_F(("isS ok\n"));
	return 1;
}


EXTRN_C __declspec(dllexport) int PASCAL GetPictureInfo(LPSTR buf, long ofs, unsigned flag, struct PictureInfo *info)
{
	char tmp[32];
	int  w,h,c,bpp;

	flag &= 7;
	if (flag == 0) {	/* ファイルよりIDを入力 */
		FILE *fp;
		fp = fopen(buf, "rb");
		if (fp == NULL) {
			return ER_FILE_READ;
		}
		if (ofs)
			fseek(fp, ofs, SEEK_SET);
		fread(tmp, 1, 0x12, fp);
		fclose(fp);
		buf = (LPSTR)tmp;
	/*} else if (flag == 1) {*/
	/*} else {*/
	/*	return ER_ETC;*/
	}

	if (memcmp(buf, "DYNA", 4) != 0) {
		DBG_F(("ヘッダチェックでエラー.\n"));
		return ER_NOT_SUPPROT;
	}
	info->left      = 0;
	info->top       = 0;
	info->width     = 1024;
	info->height    = 1024;
	info->x_density = 0;
	info->y_density = 0;
	info->hInfo     = NULL;
	info->colorDepth = 1;
	return ER_OK;
}


EXTRN_C __declspec(dllexport) int PASCAL GetPreview(
		LPSTR	 dummy_buf,
		long	 dummy_len,
		unsigned dummy_flag,
		HLOCAL*	 dummy_pHBInfo,
		HLOCAL*	 dummy_pHBm,
		FARPROC  dummy_lpPrgressCallback,
		long 	 dummy_lData
){
	return ER_NO_FUNCTION;
}


static int  FileLoad_size;

static int  FileLoad(char *name, long ofs, BYTE **bufp)
{
	FILE *fp;
	long l;
	BYTE *buf;

	fp = fopen(name, "rb");
	if (fp == NULL) {
		return ER_FILE_READ;
	}
	l = filelength(fileno(fp));
  #if 10	/* MACバイナリ等対策... */
	if (ofs) {
		fseek(fp, ofs, SEEK_SET);
		l -= ofs;
	}
  #endif
	FileLoad_size = l;
	buf = (BYTE *)malloc(l + 32);
	if (buf == NULL) {
		return ER_NOT_ENOUGH_MEM;
	}
	fread(buf, 1, l, fp);
	if (ferror(fp)) {
		return ER_FILE_READ;
	}
	fclose(fp);
	*bufp = buf;
	return ER_OK;
}



EXTRN_C __declspec(dllexport) int PASCAL GetPicture(
		LPSTR    nameOrData,
		long     ofs,
		unsigned flag,
		HLOCAL  *pHBInfo,
		HLOCAL  *pHBm,
		int (CALLBACK * dummy_lpPrgressCallback)(int,int,long),
		long    dummy_lData
){
	BITMAPINFO *bm;
	int  n, ww,w=1024, h=1024, bpp=1, flen;
	BYTE *d, *gdat;

	DBG_F(("GetPicture %x, %x, %d, %x,%x\n", nameOrData, ofs, flag, pHBInfo, pHBm /*, lpPrgressCallback, lData*/));

	flag &= 7;
	if (flag == 0) { /* ファイル読込 */
		n = FileLoad(nameOrData, ofs, &gdat);
		if (n) {
			DBG_F(("GetPicture file read error(%x,%d)\n", gdat, n));
			return n;
		}
		flen = FileLoad_size;
	} else {	// EOF でデータ終了を判定するので、メモリ上のデータならば、失敗にして対処
		DBG_M();
		return ER_NO_FUNCTION;
		//gdat = (BYTE*)nameOrData;
	}

	if (memcmp(gdat, "DYNA", 4) != 0) {
		DBG_F(("ヘッダチェックでエラー..\n"));
		return ER_NOT_SUPPROT;
	}

	*pHBInfo = LocalAlloc(LMEM_FIXED, sizeof(BITMAPINFO)+sizeof(RGBQUAD)*(1<<bpp));
	bm       = (BITMAPINFO*)*pHBInfo;
	if (bm == NULL)
		return ER_NOT_ENOUGH_MEM;
	bm->bmiHeader.biBitCount = bpp;
	bm->bmiHeader.biClrUsed  = 1<<bpp;
	ww = WID2BYT4(w,bpp);

	bm->bmiHeader.biSize	      = sizeof(BITMAPINFOHEADER);
	bm->bmiHeader.biWidth	      = w;
	bm->bmiHeader.biHeight	      = h;
	bm->bmiHeader.biPlanes	      = 1;
	bm->bmiHeader.biCompression   = BI_RGB;
	bm->bmiHeader.biSizeImage	  = ww*h;
	bm->bmiHeader.biXPelsPerMeter = 0;
	bm->bmiHeader.biYPelsPerMeter = 0;
	bm->bmiHeader.biClrImportant  = 0;
	if (pHBm) {
		/* ピクセルデータの取得 */
		*pHBm    = LocalAlloc(LMEM_FIXED, ww * h);
		if (*pHBm == NULL) {
			DBG_F(("@\n"));
			return ER_NOT_ENOUGH_MEM;
		}
		*(int*)&bm->bmiColors[0] = 0;
		*(int*)&bm->bmiColors[1] = 0xFFFFFF;
		dyna_read(gdat, (BYTE*)*pHBm, flen);
	}
	/* ファイル読み込みだったらば読み込みバッファ開放 */
	if (flag == 0) {
		free(gdat);
	}
	return ER_OK;
}



static void dyna_read(BYTE *src, BYTE *dst, int size)
{
	BYTE *s, *e, *d;
	int x,y,x1,x2,ry;

	memset(dst, 0,1024*1024/8);
	s = src + 4;
	e = s + size - 4;
	while (s+6 < e) {
		y  = PEEKiW(s);
		if (y >= 1023) {
			DBG_M();
			y = 1023;
		}
		ry = 1023 - y;		// (普通の)BMPへの変換なので上下さかさまにする
		x1 = PEEKiW(s+2);
		if (x1 >= 1023) {
			DBG_M();
			x1 = 1023;
		}
		x2 = PEEKiW(s+4);
		if (x2 >= 1023) {
			DBG_M();
			x2 = 1023;
		}
//DBG_F(("%d,%d,%d\n", y,x1,x2));
		d  = &dst[ry*1024/8];
		for (x = x1; x <= x2; x++) {
			d[x>>3] |= 1<<(7-(x&7));
		}
		s += 6;
	}
}

