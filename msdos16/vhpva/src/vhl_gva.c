#include <stdlib.h>
#include <dos.h>
#ifndef PC88VA
#define PC88VA
#endif
#include "tenkafun.h"
#include "vhp.h"
#include "vhl.h"


/* VRAMアクセス */
static BYTE_FP	vbp = (BYTE_FP) VRAM_B,    /* Vram Blue  Base Pointer */
    	    	vrp = (BYTE_FP) VRAM_R,    /* Vram Red	 Base Pointer */
    	    	vgp = (BYTE_FP) VRAM_G,    /* Vram Green Base Pointer */
    	    	vip = (BYTE_FP) VRAM_I;    /* Vram I	 Base Pointer */


/*---------------------------------------------------------------------------*/
static int MseMode;

void
vaInit(void)
{
    MseMode = Va_GetMseMode();
    if (MseMode == _VA_NOMSE) {
    	Va_puts("MSEが組み込まれていません");
    	exit(1);
    }
    Va_SetMseMode(_VA_STD);
}

#if 0
void
vaEnd(void)
{
    Va_SetMseMode(MseMode);
}
#endif

/*---------------------------------------------------------------------------*/
void
setPal(int t,byte *grb)
{
    int  i;

    for (i = 0; i < 16; ++i)
    	setGRB(i,grb[i*3] * t /100, grb[i*3+1] * t /100, grb[i*3+2] * t /100);
}


void
dspSwitch(int t,int g)
{
    if (t == 0)
    	Va_Compose(((g == 0) ? 0 : 3),0,0,0,0);
    else
    	Va_Compose(2,1,((g == 0) ? 0 : 3),0,0);
}


void
GLinit(int h, int color16, int toon, byte *grb)
    /* h:0,1  color16:0,1   */
{
    int i;

 #ifdef KAIHATU
    Va_ScreenWidth(vaVramModeFlg + h,640,4,0,0);
 #else
    Va_ScreenWidth(h,640,4,0,0);
 #endif
    Va_PaletteMode(2,0,0,0);

    setPal(toon,grb);
    if (Color16 == 0) /* 8色ならiﾌﾟﾚｰﾝを消す */
    	for (i = 0; i < 80*400; ++i)
    	    *(vip + i) = 0x00;

    Va_Compose(3,0,0,0,0);
}


/*---------------------------------------------------------------------------*/

#define vof(x,y) (LMAXX*(y)+(x))
#define VAX 320
#define vox(x,y)  ((y) * 320 * 2 + (x)*2)


/* RGBI データ */
static byte Patb1u, Patr1u, Patg1u, Pati1u;
static byte Patb1l, Patr1l, Patg1l, Pati1l;
static byte Patb2u, Patr2u, Patg2u, Pati2u;
static byte Patb2l, Patr2l, Patg2l, Pati2l;
static byte Patb3u, Patr3u, Patg3u, Pati3u;
static byte Patb3l, Patr3l, Patg3l, Pati3l;
static byte Patb4u, Patr4u, Patg4u, Pati4u;
static byte Patb4l, Patr4l, Patg4l, Pati4l;

#ifdef KAIHATU
static void
_get4bitpat(int x, int y, int w, char *pb, char *pr, char *pg, char *pi)
{
}
#endif

void
get4bitpat(int x, int y, int w, char *pb, char *pr, char *pg, char *pi)
{
    word v;

#ifdef KAIHATU
    if (vaVramModeFlg) {
    	_get4bitpat(x,y,w,pb,pr,pg,pi);
    	return;
    }
#endif

    v = vof( ((x >> 1) + ((w == 1) ? 0 : 80)) , y);
    if (x & 1) {
    	*pb = (char) (*(vbp + v) & 0xf);
    	*pr = (char) (*(vrp + v) & 0xf);
    	*pg = (char) (*(vgp + v) & 0xf);
    	*pi = (char) (*(vip + v) & 0xf);
    } else {
    	*pb = (char) ((*(vbp + v) >> 4) & 0xf);
    	*pr = (char) ((*(vrp + v) >> 4) & 0xf);
    	*pg = (char) ((*(vgp + v) >> 4) & 0xf);
    	*pi = (char) ((*(vip + v) >> 4) & 0xf);
    }
}

#ifdef KAIHATU
static void
_convpat(void)
{
}
#endif

void
convpat(void)
{
#ifdef KAIHATU
    if (vaVramModeFlg) {
    	_convpat();
    	return;
    }
#endif

    Patb1u = Patb1;
    Patr1u = Patr1;
    Patg1u = Patg1;

    Patb1l = Patb1u >> 4;
    Patr1l = Patr1u >> 4;
    Patg1l = Patg1u >> 4;

    Patb2u = Patb1u << 4;
    Patr2u = Patr1u << 4;
    Patg2u = Patg1u << 4;

    Patb2l = Patb1u & 0x0f;
    Patr2l = Patr1u & 0x0f;
    Patg2l = Patg1u & 0x0f;

    Patb1u &= 0xf0;
    Patr1u &= 0xf0;
    Patg1u &= 0xf0;

    Patb3u = Patb2;
    Patr3u = Patr2;
    Patg3u = Patg2;

    Patb3l = Patb3u >> 4;
    Patr3l = Patr3u >> 4;
    Patg3l = Patg3u >> 4;

    Patb4u = Patb3u << 4;
    Patr4u = Patr3u << 4;
    Patg4u = Patg3u << 4;

    Patb4l = Patb3u & 0x0f;
    Patr4l = Patr3u & 0x0f;
    Patg4l = Patg3u & 0x0f;

    Patb3u &= 0xf0;
    Patr3u &= 0xf0;
    Patg3u &= 0xf0;

    if (!Color16)
    	return;

    Pati1u = Pati1;
    Pati1l = Pati1u >> 4;

    Pati2u = Pati1u << 4;
    Pati2l = Pati1u & 0x0f;

    Pati1u &= 0xf0;

    Pati3u = Pati2;
    Pati3l = Pati3u >> 4;

    Pati4u = Pati3u << 4;
    Pati4l = Pati3u & 0x0f;

    Pati3u &= 0xf0;
}


#ifdef KAIHATU
static void
_putpat2(int x,int y)
{
}
#endif

void
putpat2(int x, int y)
{
    word v,w;

#ifdef KAIHATU
    if (vaVramModeFlg) {
    	_putpat2(x,y);
    	return;
    }
#endif

    if (x & 1) {
    	x >>= 1;
    	v = vof(x,y);
    	w = v + 80;/* = vof(x+80,y); */
    	*(vbp + v) = *(vbp + v) & 0xf0 | Patb1l;
    	*(vrp + v) = *(vrp + v) & 0xf0 | Patr1l;
    	*(vgp + v) = *(vgp + v) & 0xf0 | Patg1l;
    	*(vbp + w) = *(vbp + w) & 0xf0 | Patb2l;
    	*(vrp + w) = *(vrp + w) & 0xf0 | Patr2l;
    	*(vgp + w) = *(vgp + w) & 0xf0 | Patg2l;
    	if (Color16) {
    	    *(vip + v) = *(vip + v) & 0xf0 | Pati1l;
    	    *(vip + w) = *(vip + w) & 0xf0 | Pati2l;
    	}

    	if (Offy == 1)
    	    return;

    	v += LMAXX;/* v = vof(x,y+1);	 */
    	w += LMAXX;/* w = vof(x+80,y+1); */
    	*(vbp + v) = *(vbp + v) & 0xf0 | Patb3l;
    	*(vrp + v) = *(vrp + v) & 0xf0 | Patr3l;
    	*(vgp + v) = *(vgp + v) & 0xf0 | Patg3l;
    	*(vbp + w) = *(vbp + w) & 0xf0 | Patb4l;
    	*(vrp + w) = *(vrp + w) & 0xf0 | Patr4l;
    	*(vgp + w) = *(vgp + w) & 0xf0 | Patg4l;
    	if (Color16) {
    	    *(vip + v) = *(vip + v) & 0xf0 | Pati3l;
    	    *(vip + w) = *(vip + w) & 0xf0 | Pati4l;
    	}

    } else {
    	x >>= 1;
    	v = vof(x,y);
    	w = v + 80;/* = vof(x+80,y); */
    	*(vbp + v) = *(vbp + v) & 0x0f | Patb1u;
    	*(vrp + v) = *(vrp + v) & 0x0f | Patr1u;
    	*(vgp + v) = *(vgp + v) & 0x0f | Patg1u;
    	*(vbp + w) = *(vbp + w) & 0x0f | Patb2u;
    	*(vrp + w) = *(vrp + w) & 0x0f | Patr2u;
    	*(vgp + w) = *(vgp + w) & 0x0f | Patg2u;
    	if (Color16) {
    	    *(vip + v) = *(vip + v) & 0x0f | Pati1u;
    	    *(vip + w) = *(vip + w) & 0x0f | Pati2u;
    	}

    	if (Offy == 1)
    	    return;

    	v += LMAXX;/*:=  v = vof(x,y+1);    */
    	w += LMAXX;/*:=  w = vof(x+80,y+1); */
    	*(vbp + v) = *(vbp + v) & 0x0f | Patb3u;
    	*(vrp + v) = *(vrp + v) & 0x0f | Patr3u;
    	*(vgp + v) = *(vgp + v) & 0x0f | Patg3u;
    	*(vbp + w) = *(vbp + w) & 0x0f | Patb4u;
    	*(vrp + w) = *(vrp + w) & 0x0f | Patr4u;
    	*(vgp + w) = *(vgp + w) & 0x0f | Patg4u;
    	if (Color16) {
    	    *(vip + v) = *(vip + v) & 0x0f | Pati3u;
    	    *(vip + w) = *(vip + w) & 0x0f | Pati4u;
    	}
    }
}
