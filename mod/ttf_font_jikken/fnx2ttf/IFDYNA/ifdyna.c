/* Susie�p DYNA(2�l 1024x1024) �v���O */

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


/* SUSIE �v���O�C���֌W -----------------------------------------------------*/
#pragma pack(push)
#pragma pack(1)
/*typedef*/ struct PictureInfo {	/* Susie �v���O�C���p�\���� */
	long left,top;				/* �摜��W�J����ʒu   */
	long width;					/* �摜�̕�(pixel)      */
	long height;				/* �摜�̍���(pixel)    */
	WORD x_density;				/* ��f�̐����������x   */
	WORD y_density;				/* ��f�̐����������x   */
	short colorDepth;			/* �P��f�������bit��  */
	HLOCAL hInfo;				/* �摜���̃e�L�X�g��� */
} /*PictureInfo*/;
#pragma pack(pop)

#define ER_OK             0	 /* ����I��               */
#define ER_NO_FUNCTION   -1  /* ���̋@�\�̓C���v�������g����Ă��Ȃ� */
#define ER_ABORT          1  /* �R�[���o�b�N�֐�����0��Ԃ����̂œW�J�𒆎~���� */
#define ER_NOT_SUPPROT    2  /* ���m�̃t�H�[�}�b�g     */
#define ER_OUT_OF_ORDER   3  /* �f�[�^�����Ă���     */
#define ER_NOT_ENOUGH_MEM 4  /* �������[���m�ۏo���Ȃ� */
#define ER_MEMORY         5  /* �������[�G���[�iLock�o���Ȃ��A���j*/
#define ER_FILE_READ      6  /* �t�@�C�����[�h�G���[   */
/*#define ER_RESERVE      7*//* (�\��)                 */
#define ER_ETC            8  /* �����G���[             */


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
/* �f�o�b�O�E���O�E���[�`�� */

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
		"*.DYN", "DYNA",						/* 2,3 : �g���q & �`���� */
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

	/* �t�@�C������������ */
	if ((dw & 0xFFFF0000) == 0) {	/* �t�@�C���n���h�� */
		ReadFile((HANDLE)dw, buf, 0x12, 0, NULL);
		p = (UCHAR*)buf;
	} else {
		p = (UCHAR*)dw;
	}
	/* DYNA �� ID �`�F�b�N */
	if (memcmp(p, "DYNA", 4) != 0) {
		DBG_F(("�w�b�_�`�F�b�N�ŃG���[\n"));
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
	if (flag == 0) {	/* �t�@�C�����ID����� */
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
		DBG_F(("�w�b�_�`�F�b�N�ŃG���[.\n"));
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
  #if 10	/* MAC�o�C�i�����΍�... */
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
	if (flag == 0) { /* �t�@�C���Ǎ� */
		n = FileLoad(nameOrData, ofs, &gdat);
		if (n) {
			DBG_F(("GetPicture file read error(%x,%d)\n", gdat, n));
			return n;
		}
		flen = FileLoad_size;
	} else {	// EOF �Ńf�[�^�I���𔻒肷��̂ŁA��������̃f�[�^�Ȃ�΁A���s�ɂ��đΏ�
		DBG_M();
		return ER_NO_FUNCTION;
		//gdat = (BYTE*)nameOrData;
	}

	if (memcmp(gdat, "DYNA", 4) != 0) {
		DBG_F(("�w�b�_�`�F�b�N�ŃG���[..\n"));
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
		/* �s�N�Z���f�[�^�̎擾 */
		*pHBm    = LocalAlloc(LMEM_FIXED, ww * h);
		if (*pHBm == NULL) {
			DBG_F(("@\n"));
			return ER_NOT_ENOUGH_MEM;
		}
		*(int*)&bm->bmiColors[0] = 0;
		*(int*)&bm->bmiColors[1] = 0xFFFFFF;
		dyna_read(gdat, (BYTE*)*pHBm, flen);
	}
	/* �t�@�C���ǂݍ��݂�������Γǂݍ��݃o�b�t�@�J�� */
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
		ry = 1023 - y;		// (���ʂ�)BMP�ւ̕ϊ��Ȃ̂ŏ㉺�������܂ɂ���
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

