#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include "dopi.h"

/*---------------------------------------------------------------------------*/
#if 0
/* GDCｽｸﾛｰﾙの方法は、Magd v1.12(Tak氏作)のソースを参考にしました(無断利用^^;)*/
#define GdcWait() while(inp(0xa0)&2)

int Gdc_ofsX,Gdc_ofsY;
static byte Gdc_sclDat;

void Gdc_SclInit(void)
{
    Gdc_ofsX = Gdc_ofsY = 0;
    if (inp(0x31)&0x80)
    	Gdc_sclDat = 0x00;// + (Gv_btmLine == 200) ? 0x80 : 0;
    else
    	Gdc_sclDat = 0x40;// + (Gv_btmLine == 200) ? 0x80 : 0;
    GdcWait(); outp(0xa2,0x70);
    GdcWait(); outp(0xa0,0x00);
    GdcWait(); outp(0xa0,0x00);
    GdcWait(); outp(0xa0,(400 & 0x0f) << 4);
    GdcWait(); outp(0xa0,(400 >> 4) + Gdc_sclDat);
    GdcWait(); outp(0xa0,0x00);
    GdcWait(); outp(0xa0,0x00);
    GdcWait(); outp(0xa0,0x00);
    GdcWait(); outp(0xa0,0x00 + Gdc_sclDat);
}

#if 0
void Gdc_SclY(int dd)
{
/*  dd	: +n roll up
    	  -n roll down
*/
    word sad1,sl1,sad2,sl2;

 //cprintf("s:%d+dd:%d=%d  ",Gdc_ofsY,dd,Gdc_ofsY+dd);
    Gdc_ofsY += dd;
    if (Gdc_ofsY < 0)
    	Gdc_ofsY += 400;
    else if (Gdc_ofsY >= 400)
    	Gdc_ofsY -= 400;
 //cprintf("  %d",Gdc_ofsY);
    if (Gdc_ofsY) {
    	sad1 = Gdc_ofsY*40;
    	sad2 = 0;
    	sl2 = Gdc_ofsY;
    	sl1 = 400-sl2;
    }else {
    	sad1 = 0;
    	sl1 = 400;
    	sad2 = 0;
    	sl2 = 0;
    }
    GdcWait(); outp(0xa2,0x70);
    GdcWait(); outp(0xa0,sad1&0xff);
    GdcWait(); outp(0xa0,sad1>>8);
    GdcWait(); outp(0xa0,(sl1&15)<<4);
    GdcWait(); outp(0xa0,(sl1>>4)+Gdc_sclDat);
    GdcWait(); outp(0xa0,sad2&0xff);
    GdcWait(); outp(0xa0,sad2>>8);
    GdcWait(); outp(0xa0,(sl2&15)<<4);
    GdcWait(); outp(0xa0,(sl2>>4)+Gdc_sclDat);
}
#endif

void Gdc_SclSub(void)
{
    byte far *bp,far *rp,far *gp,far *ip;
    int  d,i;

    bp = (byte far *)0xA8000000L;
    rp = (byte far *)0xB0000000L;
    gp = (byte far *)0xB8000000L;
    ip = (byte far *)0xE0000000L;
    d = 80 * Gv_btmLine;
    i = 80;
    while (--i >= 0) {
    	*(bp+d)= *bp;
    	*(rp+d)= *rp;
    	*(gp+d)= *gp;
    	*(ip+d)= *ip;
    	bp++;rp++;gp++;ip++;
    }
}

void Gdc_SclSub2(void)
{
    byte far *bp,far *rp,far *gp,far *ip;
    int  d,i;

    bp = (byte far *)0xA8000000L;
    rp = (byte far *)0xB0000000L;
    gp = (byte far *)0xB8000000L;
    ip = (byte far *)0xE0000000L;
    d = 80*Gv_btmLine;
    i = 80;
    while (--i >= 0) {
    	*bp = *(bp+d);
    	*rp = *(rp+d);
    	*gp = *(gp+d);
    	*ip = *(ip+d);
    	bp++;rp++;gp++;ip++;
    }
}

void Gdc_Scl(int ddx, int ddy)
{
    word sad1,sl1,sad2,sl2;
    int x,btmLine;

    btmLine = 400;//Gv_btmLine;
 //cprintf("s:%d+ddy:%d=%d  ",Gdc_ofsY,ddy,Gdc_ofsY+ddy);
    Gdc_ofsX += ddx;
    if (Gdc_ofsX < 0) {
    	Gdc_SclSub();
    	Gdc_ofsX += 80;
    	Gdc_ofsY--;
    } else if (Gdc_ofsX >= 80) {
    	Gdc_SclSub2();
    	Gdc_ofsX -= 80;
    	Gdc_ofsY++;
    }
    x = Gdc_ofsX>>1;
    Gdc_ofsY += ddy;
    if (Gdc_ofsY < 0)
    	Gdc_ofsY += btmLine;
    else if (Gdc_ofsY >= btmLine)
    	Gdc_ofsY -= btmLine;
 //cprintf("  %d",Gdc_ofsY);
    if (Gdc_ofsY) {
    	sad1 = Gdc_ofsY*40 + x;
    	sad2 = x;
    	sl2 = Gdc_ofsY;
    	sl1 = btmLine-sl2;
    } else {
    	sad1 = x;
    	sl1 = btmLine;
    	sad2 = 0;
    	sl2 = 0;
    }
    if (gDebSclFlg) {
    	gotoxy(1,24);
    	cprintf("(%03d,%03d) sad1:%04x:%-3d sad2:%04x:%-3d   ",
    	    Gdc_ofsX,Gdc_ofsY,sad1,sl1,sad2,sl2);
    }
    GdcWait(); outp(0xa2,0x70);
    GdcWait(); outp(0xa0,sad1&0xff);
    GdcWait(); outp(0xa0,sad1>>8);
    GdcWait(); outp(0xa0,(sl1&15)<<4);
    GdcWait(); outp(0xa0,(sl1>>4)+Gdc_sclDat);
    GdcWait(); outp(0xa0,sad2&0xff);
    GdcWait(); outp(0xa0,sad2>>8);
    GdcWait(); outp(0xa0,(sl2&15)<<4);
    GdcWait(); outp(0xa0,(sl2>>4)+Gdc_sclDat);
}
#endif
/*---------------------------------------------------------------------------*/

_S void GetVRamQ2(byte huge *bufp,int linsiz0, int szy0,
    int sx,int sy,int xsz,int ysz)
{
    static byte far *vp[]={
    	(byte far *)0xA8000000L,
    	(byte far *)0xB0000000L,
    	(byte far *)0xB8000000L,
    	(byte far *)0xE0000000L
    };
    int x,y;
    static byte a[8];
    int linsiz;

    linsiz = linsiz0*4;
    bufp++;bufp--;
  //printf ("Gv_GetVRamQ2(%lx,%d,%d;%d,%d;%d,%d)\n",
  //	    bufp,linsiz0,szy0,sx,sy,xsz,ysz);
    for (y = 0; y < ysz; y++) {
    	for (x = 0; x < xsz; x++) {
    	    int tx,ty;
    	    int bnk,d;
    	    byte huge *p;

    	    tx = szy0 - sy - y - 1;
    	    ty = sx + x;
    	    p = (byte huge *)((dword)bufp + (dword)(LtoFP(
    	    	ty * linsiz * 8L + tx*4L ) ) );
    	    p++;p--;
    	    d = (y * 8) * 80 + x;
    	    for (bnk = 0; bnk < 4;bnk++) {
    	    	int i;
    	    	for (i = 0;i < 8;i++)
    	    	    a[i] = vp[bnk][d + i * 80];
    	    	ror8x8(a);
    	    	for (i = 0;i < 8;i++) {
    	    	    p[i * linsiz + bnk] = a[i];
    	    	}
    	    }
    	}
    }
}

int Gv_GetVRamQ(byte far *bufp,int ysize,int xsize,int f)
{
    int xsz,ysz,xsz2,ysz2;

  //printf ("Gv_GetVRamQ(%lx,%d,%d,%d)\n",bufp,xsize,ysize,f);
    xsz = xsize;
    ysz = ysize;
    xsz2 = 0;
    ysz2 = 0;
    if (xsz > 80) {
    	xsz2 = xsize - 80;
    	if (xsz2 > 80)
    	    xsz2 = 80;
    	xsz = 80;
    } else if (ysz > 50) {
    	ysz2 = ysize - 50;
    	if (ysz2 > 50)
    	    ysz2 = 50;
    	ysz = 50;
    }
    ActGPage(0);
    GetVRamQ2((byte huge *)bufp,ysize, ysz+ysz2,0,0,xsz,ysz);
    ActGPage(1);
    if (f == 1) {
    	GetVRamQ2(bufp,ysize, ysz+ysz2,0,50,xsz,ysz2);
    } else if (f == 2) {
    	GetVRamQ2(bufp,ysize, ysz+ysz2,80,0,xsz2,ysz);
    }
    ActGPage(0);
    return f;
}

/*---------------------------------------------------------------------------*/
