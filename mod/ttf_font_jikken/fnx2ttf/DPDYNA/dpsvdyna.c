/*
	cb2tt �ŗp������ DYNA �摜���Z�[�u���܂��B

 */

#include "dp_add.h"			/* dpixed-add-in�̒�` */
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
	//���� Add-In �Ɋւ������ D-Pixed �֕Ԃ�
	pInfo->dwType 			= DPADDIN_ENCODER;				// �v���O�C���^�C�v
	pInfo->name   			= "DYNA Saver";					// �c�[����
	pInfo->copyright		= "2000(C) tenk*";				// ���쌠�\��
	pInfo->addinDescription = "DYNA 2�l1024x1024�� ����";	// ����
	pInfo->version 			= "0.01";						// �o�[�W����
	pInfo->description 		= "DYNA File(*.dyn)";			// �c�[������
	pInfo->defExt			= "dyn";						// �g���q�w��
	return TRUE;
}


EXTERN_C __declspec(dllexport) BOOL DPEncode(LPCSTR fname, DP_ENCDECINFO *pi)
{
	// d-pixed �p�摜�t�@�C�����[�_
	LPBITMAPINFOHEADER bm;					// BMP/DIB header
	BYTE 	*p;

	// �摜�̎擾
	bm = pi->pDibBuf[0];
	p  = pi->pBitBuf[0];

	if (bm->biWidth != 1024 || bm->biHeight != 1024) {
		MessageBox(NULL, "1024x1024�摜�łȂ��I", "DYNA Save", MB_OK);
		return FALSE;
	}

	if (dyna_fnt_write((char*)fname, p) == 0) {
		MessageBox(NULL, "�Z�[�u�Ɏ��s���܂���", "DYNA Save", MB_OK);
	}
	return TRUE;
}


static void fputc2i(int c, FILE *fp)
{
	/* fp�� ��ٴ��ި�݂� 2�o�C�g��������. �G���[������Α��I�� */
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
					ry = h - 1 - y;			/* ���ʂ�BMP�͏㉺�t�Ȃ��� */
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


