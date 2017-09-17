/*
	cb2tt で用いられる DYNA 画像をセーブします。

 */

#include "dp_add.h"			/* dpixed-add-inの定義 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>


/* ----------------------------------------------------------------------- */
#ifdef __cplusplus
#define EXTRN_C		extern "C"
#else
#define EXTRN_C
#endif


/* ----------------------------------------------------------------------- */
static int dyna_fnt_write(char *name, BYTE *buf);



EXTERN_C __declspec(dllexport) BOOL DPGetInfo(DP_ADDININFO *pInfo)
{
	//この Add-In に関する情報を D-Pixed へ返す
	pInfo->dwType 			= DPADDIN_ENCODER;				// プラグインタイプ
	pInfo->name   			= "DYNA Saver";					// ツール名
	pInfo->copyright		= "2000(C) tenk*";				// 著作権表示
	pInfo->addinDescription = "DYNA 2値1024x1024画 書込";	// 説明
	pInfo->version 			= "0.01";						// バージョン
	pInfo->description 		= "DYNA File(*.dyn)";			// ツール説明
	pInfo->defExt			= "dyn";						// 拡張子指定
	return TRUE;
}


EXTERN_C __declspec(dllexport) BOOL DPEncode(LPCSTR fname, DP_ENCDECINFO *pi)
{
	// d-pixed 用画像ファイルローダ
	LPBITMAPINFOHEADER bm;					// BMP/DIB header
	BYTE 	*p;

	// 画像の取得
	bm = pi->pDibBuf[0];
	p  = pi->pBitBuf[0];

	if (bm->biWidth != 1024 || bm->biHeight != 1024) {
		MessageBox(NULL, "1024x1024画像でない！", "DYNA Save", MB_OK);
		return FALSE;
	}

	if (dyna_fnt_write((char*)fname, p) == 0) {
		MessageBox(NULL, "セーブに失敗しました", "DYNA Save", MB_OK);
	}
	return TRUE;
}


static void fputc2i(int c, FILE *fp)
{
	/* fpに ﾘﾄﾙｴﾝﾃﾞｨｱﾝで 2バイト書き込む. エラーがあれば即終了 */
	BYTE buf[4];

	buf[0] = (BYTE)(c);
	buf[1] = (BYTE)(c>> 8);
	fwrite(buf, 1, 2, fp);
}


static int dyna_fnt_write(char *name, BYTE *buf)
{
	FILE *fp;
	int w = 1024, h = 1024;
	int x,y,l,s,ry;

	fp = fopen(name, "wb");
	if (fp == NULL)
		return 0;
	fwrite("DYNA", 1, 4, fp);
	for (y = h; --y >= 0;) {
		s = l = 0;
		for (x = 0; x < w; x++) {
			if (buf[y*w + x]) {
				l++;
			} else {
				if (l > 0) {
					ry = h - 1 - y;			/* 普通のBMPは上下逆なため */
					fputc2i(ry, fp);
					fputc2i(s, fp);
					fputc2i(s + l, fp);
					l = 0;
				}
				s = x;
			}
		}
		if (ferror(fp)) {
			fclose(fp);
			return 0;
		}
	}
	fclose(fp);
	return 1;
}


