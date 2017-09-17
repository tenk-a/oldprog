#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <stddef.h>
#include "tenkafun.h"
#include "vhp.h"
#include "vhs.h"

#define vof(x,y) (LMAXX*(y)+(x))

/* VRAMアクセス */
static BYTE_FP vbp = (BYTE_FP) VRAM_B, /* Vram Blue  Base Pointer */
    	       vrp = (BYTE_FP) VRAM_R, /* Vram Red   Base Pointer */
    	       vgp = (BYTE_FP) VRAM_G, /* Vram Green Base Pointer */
    	       vip = (BYTE_FP) VRAM_I; /* Vram I     Base Pointer */


void
swapvram1(void)
    /*
     * VRAM DATAを並べ換える
     */
{
    word vbase[4];
    byte sbuf[LMAXX], dbuf[LMAXX];
    word i, j, k;
    struct SREGS segregs;

    vbase[0] = FP_SEG(vbp);
    vbase[1] = FP_SEG(vrp);
    vbase[2] = FP_SEG(vgp);
    vbase[3] = FP_SEG(vip);
    segread(&segregs);
    for (i = 0; i < 4; ++i) {
    for (j = 0; j < LMAXX * Ly; j += LMAXX) {
    	movedata(vbase[i], j, segregs.ds, (int) sbuf, LMAXX);
    	for (k = 0; k < LMAXX / 2; ++k) {
    	dbuf[k * 2] = (sbuf[k] & 0xf0) | (sbuf[k + LMAXX / 2] >> 4);
    	dbuf[k * 2 + 1] = (sbuf[k] << 4) | (sbuf[k + LMAXX/2] & 0x0f);
    	}
    	movedata(segregs.ds, (int) dbuf, vbase[i], j, LMAXX);
    }
    }
}

void
swapvram2(void)
    /*
     * VRAM DATAを並べ換える
     */
{
    word vbase[4];
    byte sbuf[LMAXX], dbuf[LMAXX];
    struct SREGS segregs;
    int  i, j, k;

    vbase[0] = FP_SEG(vbp);
    vbase[1] = FP_SEG(vrp);
    vbase[2] = FP_SEG(vgp);
    vbase[3] = FP_SEG(vip);
    segread(&segregs);
    for (i = 0; i < 4; ++i) {
    for (j = 0; j < LMAXX * Ly; j += LMAXX) {
    	movedata(vbase[i], j, segregs.ds, (int) sbuf, LMAXX);
    	for (k = 0; k < LMAXX; k += 2) {
    	dbuf[k / 2] = (sbuf[k] & 0xf0) | (sbuf[k + 1] >> 4);
    	dbuf[k / 2 + LMAXX / 2] = (sbuf[k] << 4) | (sbuf[k+1] & 0x0f);
    	}
    	movedata(segregs.ds, (int) dbuf, vbase[i], j, LMAXX);
    }
    }
}


void
getpat(int x, int y)
{
    Patb1 = *(vbp + vof(x, y));
    Patr1 = *(vrp + vof(x, y));
    Patg1 = *(vgp + vof(x, y));
    Patb2 = *(vbp + vof(x, y + 1));
    Patr2 = *(vrp + vof(x, y + 1));
    Patg2 = *(vgp + vof(x, y + 1));

    if (Color16) {
    Pati1 = *(vip + vof(x, y));
    Pati2 = *(vip + vof(x, y + 1));
    }
}


void
get4bitpat(int x, int y, int w, byte *pb, byte *pr, byte *pg, byte *pi)
{
    word v;

    v = vof(x, y);
#if 1
    if (w == 0) {
    	*pb = (byte) (*(vbp + v) & 0xf);
    	*pr = (byte) (*(vrp + v) & 0xf);
    	*pg = (byte) (*(vgp + v) & 0xf);
    	*pi = (byte) (*(vip + v) & 0xf);
    } else {
    	*pb = (byte) (*(vbp + v) >> 4);
    	*pr = (byte) (*(vrp + v) >> 4);
    	*pg = (byte) (*(vgp + v) >> 4);
    	*pi = (byte) (*(vip + v) >> 4);
    }
#else
    *pb = (byte) ((*(vbp + v) >> (w * 4)) & 0xf);
    *pr = (byte) ((*(vrp + v) >> (w * 4)) & 0xf);
    *pg = (byte) ((*(vgp + v) >> (w * 4)) & 0xf);
    *pi = (byte) ((*(vip + v) >> (w * 4)) & 0xf);
#endif
}


int
_cmppat1(int x, int y)
{
    word v;

    v = vof(x, y);
    if (   (Patb1 == *(vbp + v))
    && (Patr1 == *(vrp + v))
    && (Patg1 == *(vgp + v))
    && (!Color16 || Pati1 == *(vip + v)) )
    	return 1;
    return 0;
}

int
_cmppat2(int x, int y)
{
    word v;

    v = vof(x, y);
    if (   (Patb2 == *(vbp + v))
    && (Patr2 == *(vrp + v))
    && (Patg2 == *(vgp + v))
    && (!Color16 || Pati2 == *(vip + v)) )
    	return 1;
    return 0;
}


#ifdef PC88VA
void
va_init(void)
{
    int mode;

    mode = Va_GetMseMode();
    switch(mode) {
    case _VA_NOMSE:
    	Va_puts("MSEが組み込まれていません");
    	exit(1);
    case _VA_STD:
    	break;
    case _VA_98MONO:
    	Va_puts("注意! 98ﾓﾉｸﾛ･ﾓｰﾄﾞです");
    	goto J1;
    case _VA_98GRPH:
    	Va_puts("注意! 98ｸﾞﾗﾌｨｯｸ･ﾓｰﾄﾞです");
J1:
    	vbp = (BYTE_FP)VRAM98_B;
    	vrp = (BYTE_FP)VRAM98_R;
    	vgp = (BYTE_FP)VRAM98_G;
    	vip = (BYTE_FP)VRAM98_I;
    	break;
    case _VA_98TEXT:
    case _VA_NATIVE:
    default:
    	Va_SetMseMode(_VA_STD);
    }
}
#endif
