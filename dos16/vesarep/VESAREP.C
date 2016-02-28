#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vesa.h"


int main(void)
{
	int i,n;
	VESA_INFO     inf;
	VESA_MODEINFO mi;
	unsigned short huge *pp;

	char *typs[8] = {"TEXT","CGA","HCG","Plane","PackedPixel","NoneChain4","DirectColor","YVU"};

	//i = VESA_GetInfo(&inf); printf("\t%04x\n",i);
	if (VESA_GetInfo(&inf)) goto ERR;

	printf("%c%c%c%c ver%02x.%02x flag:%04X memsiz%6dKbytes  OEM:%Fs\n",
		inf.id[0],inf.id[1],inf.id[2],inf.id[3],
		inf.version>>8, inf.version&0xff, inf.flags, inf.totalMem*64,inf.oemNames);

	n = 0;
	i = 8;
	pp = (unsigned short huge *)&(inf.videoModeTbl[0]);
	pp ++; --pp;
	printf("\tVIDEO_MODE_TABLE=%8lx(%8lx)\n", inf.videoModeTbl, pp);
	while (pp[n] != 0xffff) {
		printf("\t%03x",pp[n]);
		if (--i == 0)
			printf("\n"),i = 8;
		n++;
	}
	if (i < 8)
		printf("\n");

	printf("\n");
	/* */
	for (n = 0; pp[n] != 0xffff; n++) {
		printf("  %03X",pp[n]);
		//i = VESA_GetModeInfo(pp[n],&mi); printf("\t%04x\n",i);
		if ((i = VESA_GetModeInfo(pp[n],&mi)) != 0) goto ERR;
		if (mi.type < 8) {
			printf ("   %-11s",typs[mi.type]);
		} else {
			printf ("   type:%02x  ",mi.type);
		}
		printf("  Pln=%d",mi.plnCnt);
		printf("  %2dbitsCol",mi.pbits);
		printf("  %5d*%-5d",mi.gxsz,mi.gysz);
		printf("  (chr:%d*%d)",mi.chrXsz,mi.chrYsz);
		printf("  mode=%04x", mi.mode);
		printf("\n");
		printf("\tWIN:pagSz=%dK,sz=%dK,adr=%08lx", mi.winPagSiz, mi.winSiz, mi.winAdr);
		printf(" A:seg=%04x,atr=%02x",mi.winAseg, mi.winAatr);
		printf(" B:seg=%04x,atr=%02x",mi.winBseg, mi.winBatr);
		printf("\n");
		printf("\tBnkCnt=%d",mi.bnkCnt);
		printf("  BnkSz=%dK",mi.bnkSz);
		printf("  scanL=%d",mi.scanlineBytes);
		printf("  ImgPagCnt=%d",mi.imgPagCnt);
		printf("  DrctColAtr=%02x",mi.drctColAtr);
		/*printf(" 1.1Flg=%d",mi.rsv1);*/
		printf("\n");
		/*if (mi.type == 6 || mi.type == 7)*/ {
			printf("\tR:msk=%02x pos=%d",mi.mskBitsR,mi.mskBitPosR);
			printf("\tG:msk=%02x pos=%d",mi.mskBitsG,mi.mskBitPosG);
			printf("\tB:msk=%02x pos=%d",mi.mskBitsB,mi.mskBitPosB);
			printf("\tA:msk=%02x pos=%d",mi.mskBitsA,mi.mskBitPosA);
			printf("\n");
		}
		printf("\n");
	}

	return 0;
 ERR:
	printf("\n%x\n",i);
	printf("ƒGƒ‰[”­¶\n");
	return 0;
}
