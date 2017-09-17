/*
	cb2tt �ŗp������ DYNA �摜�̓ǂ݂��݂�
	�s���܂��B

	int=32bit�R���p�C����O��.
 */

#include "dp_add.h"			/* dpixed-add-in�̒�` */
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
#define GR_SET_ARGB(q, a,r,g,b) 	(*((int*)(q)) = GR_ARGB(a,r,g,b))		// ���g���G���f�B�A���ˑ�
#define	GR_WID2BYT(w,bpp)			(((bpp) > 24) ? ((w)<<2) : ((bpp) > 16) ? ((w)*3) : ((bpp) > 8) ? ((w)<<1) : ((bpp) > 4) ? (w) : ((bpp) > 1) ? (((w)+1)>>1) : (((w)+7)>>3))
#define GR_WID2BYT4(w,bpp)			((GR_WID2BYT(w,bpp) + 3) & ~3)


/* ----------------------------------------------------------------------- */

static void dyna_fnt_read(BYTE *src, BYTE *dst, int size);
static void *file_load(char *name, void *buf, int bufsz, int *rdszp);


EXTERN_C __declspec(dllexport) BOOL DPGetInfo(DP_ADDININFO *pInfo)
{
	//���� Add-In �Ɋւ������ D-Pixed �֕Ԃ�
	pInfo->dwType 			= DPADDIN_DECODER;				// �v���O�C���^�C�v
	pInfo->name   			= "DYNA Loader";				// �c�[����
	pInfo->copyright		= "2000(C) tenk*";				// ���쌠�\��
	pInfo->addinDescription = "DYNA 2�l1024x1024�� �Ǎ�";	// ����
	pInfo->version 			= "0.01";						// �o�[�W����
	pInfo->description 		= "DYNA File(*.dyn)";			// �c�[������
	pInfo->defExt			= "dyn";						// �g���q�w��
	return TRUE;
}


EXTERN_C __declspec(dllexport) BOOL DPDecode(LPCSTR fname, DP_ENCDECINFO *pi)
{
	// d-pixed �p�摜�t�@�C�����[�_
	LPBITMAPINFOHEADER bm;					// BMP/DIB header
	RGBQUAD *clut;
	BYTE 	*p, *src;
	int  	w, h, sz;

	// �f�[�^�t�@�C���̓ǂݍ���
	src = (BYTE*)file_load((char*)fname, NULL, 0, &sz);
	if (src == NULL) {
		MessageBox(NULL, "������������܂���I", "DYNA Load", MB_OK);
		return FALSE;
	}
	w  = 1024;
	h  = 1024;

	// �A�h�C���̏���
	pi->dwFlag   = DPDECENC_KEYCOLOR|DPDECENC_BACKGROUNDCOLOR;	// �����F,�w�i�F���L��
	pi->keyColor = 0;											// �����F�ԍ�
	pi->bgColor  = 0;											// �w�i�F�ԍ�
	pi->nPicture = 1;											// �G�̖���

	pi->pDibBuf = (LPBITMAPINFOHEADER*)calloc(1, 1 * sizeof(LPBITMAPINFOHEADER));
	pi->pBitBuf = (LPBYTE*)calloc(1, 1 * sizeof(LPBYTE));
	p = (BYTE*)calloc(1, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) + 1024*1024);
	if (p == NULL || pi->pBitBuf == NULL || pi->pDibBuf == NULL) {
		MessageBox(NULL, "������������܂���", "DYNA Load", MB_OK);
		return FALSE;
	}
	bm				= (LPBITMAPINFOHEADER)p;
	pi->pDibBuf[0]	= bm;
	clut			= (RGBQUAD*)(p + sizeof(BITMAPINFOHEADER));
	p				= p + sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD);
	pi->pBitBuf[0]	= p;

	// DIB �w�b�_�̐ݒ�
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

	// �Ƃ肠�����̃p���b�g�ݒ�BDYNA���̂ɂ̓p���b�g�Ȃ�ĂȂ��̂�
	GR_SET_ARGB(&clut[0], 0,0x00,0x00,0x00);
	GR_SET_ARGB(&clut[1], 0,0xFF,0xFF,0xFF);

	// DYNA �f�[�^�̓W�J
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
		ry = 1023 - y;		// (���ʂ�)BMP�ւ̕ϊ��Ȃ̂ŏ㉺�������܂ɂ���
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
	/* name : �ǂ݂��ރt�@�C�� */
	/* buf  : �ǂ݂��ރ������BNULL���w�肳���� malloc���A16�o�C�g�]���Ɋm�ۂ��� */
	/* bufsz: buf�̃T�C�Y�B0���w�肳���� �t�@�C���T�C�Y�ƂȂ� */
	/* rdszp: NULL�łȂ���΁A�ǂ݂��񂾃t�@�C���T�C�Y�����ĕԂ� */
	/* �߂�l: buf�̃A�h���X��malloc���ꂽ�A�h���X. �G���[����NULL��Ԃ� */
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
		bufsz = (bufsz + 15 + 16) & ~15;	//16�o�C�g�ی��I�ɗ]���Ɋm�ہB
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


