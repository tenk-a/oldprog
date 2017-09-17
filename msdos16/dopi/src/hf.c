#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include "dopi.h"
#include "fbport.h"

#ifdef HF
void Hf_Dsp(byte huge *gp, int xsize, int ysize, int sx, int sy)
    // gp:�s�N�Z���E�f�[�^�擪�A�h���X
    // xsize,ysize:�s�N�Z���E�f�[�^�̏c���T�C�Y
    // sx,sy: ��or 0 = �s�N�Z���E�f�[�^�E�o�b�t�@�ł̕\���J�n�ʒu
    //	      ��     = ��ʏ�ł̕\���J�n�ʒu
{
    int     xbgn,ybgn;	    //��ʏ�ł̕\���J�n�ʒu(����)
    int     xend,yend;	    //��ʏ�ł̕\���I���ʒu(�E��)
    int     x,y;

    //�͈͊O�Ȃ�A�J�n�ʒu��(0,0)
    if (sx < -639 || sx > xsize || sy < -399 || sy > ysize)
    	sx = sy = 0;
    xend = xsize - sx;
    if (xend > 640)
    	xend = 640;
    xbgn = 0;
    if (sx < 0)
    	xbgn = -sx, sx = 0;
    yend = ysize - sy;
    if (yend > 400)
    	yend = 400;
    ybgn = 0;
    if (sy < 0)
    	ybgn = -sy, sy = 0;

    gp = (byte huge *)(gp + (long)sy*(long)xsize + (long)sx);
    for (y = ybgn; y < yend; y++, gp += (long)xsize) {
    	byte far *p;

    	outport(0xd6,y);    	    	    	// HF��Y���W�ݒ�
    	for(p=(byte far *)gp, x = xbgn; x < xend; x++, p++) {
    	    int c;

    	    outport(0xd4,x);	    	    // HF��X���W�ݒ�
    	    //outport(0xd6,y);	    	    // HF��Y���W�ݒ�
    	    c = (*p) * 3;
    	    outp(0xd8+0*2, PiLd_pal256[c+0]);
    	    outp(0xd8+1*2, PiLd_pal256[c+1]);
    	    outp(0xd8+2*2, PiLd_pal256[c+2]);
    	    ////printf("(%d,%d) pxl=%x	ptr=%Fp\n", x,y,*p,p);
    	}
    	////printf("line %d. p=%Fp, gp=%Fp\n", y,p,gp);
    }
}

#if 1
int Hf_Init(void)
{
    int i;

    //if (gFBmode) {
    	i = FB_Init();
    	FB_Mode(BOTH);
    //} else {
    //	HFOn();     	    	    /* �n�C�p�[�t���[���n�m 	    */
    //	HFSoRe( 1 );	    	    /* �t���[���o�b�t�@�Ƃ��Ďg�p   */
    //	HFBnIo( 1 );	    	    /* �h�^�n�A�N�Z�X��v�� 	    */
    //	HFDisp( 2 );	    	    /* �X�[�p�[�C���|�[�Y�ɂ���     */
    //	HFRgIni();  	    	    /* ���W�X�^������������ 	    */
    //	HFRgSw( 1 );	    	    /* �g�p��������	    	    */
    //	HFAnDg( 1 );	    	    /* �\��������悤�ɗv�� 	    */
    //}
    return i;
}

void Hf_Term(void)
{
    //if (gFBmode) {
    	FB_Mode(PCONLY);
    	FB_Mode(FBOFF);
    //} else {
    //	HFOff();
    //}
}

void Hf_Cls(void)
{
    //if (gFBmode) {
    	FBClear();
    //} else {
    //	word x,y;
    //	//Hf_Init();
    //	for(y=0;y<640;y++) {
    //	    outport(0xd6,y);
    //	    for(x=0;x<640;x++) {
    //	    	outport(0xd4,x);
    //	    	outp(0xd8,0);
    //	    	outp(0xda,0);
    //	    	outp(0xdc,0);
    //	    }
    //	}
    //	//Hf_Term();
    //}
}

void Hf_DspOff(void)
{
    FB_Mode(PCONLY);
}

#else
void Hf_Init(void)
{
    HFOn(); 	    	    	/* �n�C�p�[�t���[���n�m     	*/
    HFSoRe( 1 );    	    	/* �t���[���o�b�t�@�Ƃ��Ďg�p	*/
    HFBnIo( 1 );    	    	/* �h�^�n�A�N�Z�X��v��     	*/
    HFDisp( 2 );    	    	/* �X�[�p�[�C���|�[�Y�ɂ��� 	*/
    HFRgIni();	    	    	/* ���W�X�^������������     	*/
    HFRgSw( 1 );    	    	/* �g�p��������   	    	*/
    HFAnDg( 1 );    	    	/* �\��������悤�ɗv��     	*/
}

void Hf_Term(void)
{
    HFOff();
}

void Hf_Cls(void)
{
    	word x,y;
    	//Hf_Init();
    	for(y=0;y<640;y++) {
    	    outport(0xd6,y);
    	    for(x=0;x<640;x++) {
    	    	outport(0xd4,x);
    	    	outp(0xd8,0);
    	    	outp(0xda,0);
    	    	outp(0xdc,0);
    	    }
    	}
    	//Hf_Term();
}

void Hf_DspOff(void)
{
    HFAnDg( 0 );
}

#endif


#endif	/* HF */
