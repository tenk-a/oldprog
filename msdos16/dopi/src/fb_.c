/*
    �r�e�^�g�e�̋��p�����[�`��
    �I���W�i����
    	Super-Frame and HyPER-Frame Common unit for Turbo Pascal 6.0
    	By Woody-Rinn 1991/02

�������֌W�Ɋւ���
�@�{�v���O�����̓t���[�\�t�g�ł��B���̐�����A�g�p����єz�z�ɑ΂��鐧���͈��
�݂��܂���B
�@�o����΁A�g�p�����ꍇ�ɂ́ufb.c�v�g�p�����Ə����Ē�����ƍK���ł����A������
���܂���B
�@�܂��A��҂͖{�v���O�������g�p�������Ƃɂ�邢���Ȃ���ɑ΂��Ă���؂̐ӔC
�͎��܂���B

������҂Ɋւ���
�@���̃v���O������W_Rinn�搶��Turbo-Pascal�ŏ����ꂽ"FB.PAS"�����ɁATEKITO����
���y�шڐA���s���܂����B
�@�����Ō����֌W�͈ȉ��̗l�ɂȂ�܂��B
    ������ҁF	W_Rinn
    ����ҁF	TEKITO
*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include "fbport.h"
#include "usertype.h"

int SF_Initdat[]={
    	0x6b00,0x5001,0x5b02,0x0e03,0x1a04,0x0205,
    	0x1906,0x1907,0x8008,0x0f09,0xff0c,0xff0d,
    	0x011b,0x8c1e,0x001f
};

int 	FB_Type=NoFrameBuf;

FB_Init()
{
    int     mode,l;

#if 0
    outportb(FBP_BnkReg0,0xc0);     /*	�Ƃ肠�����g�e�Ƃ��ď�����  */
#endif
    outportb(FBP_BnkReg1,0xc0);     /*	HF On�͂���Ă���   	    */
    mode = inport(FBP_cont0)|0x81;
    outport(FBP_cont0,mode);
    if (mode==inport(FBP_cont0))
    {
    	FB_Type = HyperFrame;	    /*	FB_Type���g�e�ɐݒ肷�� */
    	asm in	al,FBP_cont0
    	asm or	al,10000001b	    /*	I/O Access & Frame Buf	*/
    	asm out FBP_cont0,al
    	outport(FBP_Yreg,0x193);
    	for(l=0x80;l<=0xa1;l++) {
    	    outport(FBP_Xreg,l);
    	    outportb(FBP_GreReg,0);
    	}
    	mode = inport(FBP_cont0) | HF_RGBWR;
    	outport(FBP_cont0,mode);
    	outport(FBP_Yreg,0x191);
    	for(l=0x180;l<=0x188;l++) {
    	    outport(FBP_Xreg,l);
    	    outportb(FBP_RedReg,0);
    	}
    	mode = inport(FBP_cont0)&(~HF_RGBWR);
    	outport(FBP_cont0,mode);
    }
    else
    {
    	FB_Type = SuperFrame;	    /*	FB_Type���r�e��     	*/
    	for(l=0;l<15;l++)
    	{
    	    mode = SF_Initdat[l];
    	    outportb(FBP_cont0,mode&0xff);
    	    outportb(FBP_cont1,mode>>8);
    	}
    	outportb(FBP_ModReg,0);
    }
/*  �ŏI�I�Ƀt���[���o�b�t�@�����邩�H�@�̃`�F�b�N�����Ă���	*/
    outport(FBP_Xreg,0);
    outport(FBP_Yreg,0);    	/*  XY=0�Ƃ��Ă�����	*/
    l = inportb(FBP_RedReg);
    outportb(FBP_RedReg,0xaa);
    if (inportb(FBP_RedReg)!=0xaa)
    {
    	FB_Type=NoFrameBuf; 	/*  �t���[���o�b�t�@���Ȃ��I	*/
    	fprintf(stderr,"�t���[���o�b�t�@������܂���\n");
    	exit(1);
    }
    else
    {
    	outportb(FBP_RedReg,l);
    }
}

FB_Mode(uchar mode)
{
    if (FB_Type==SuperFrame)
    {
    	switch (mode)
    	{
    	case PCONLY:
    	    outport(FBP_ModReg,PCONLY);
    	    _AH=0x0c;	geninterrupt(0x18);
    	    break;
    	case FBONLY:
    	    outport(FBP_ModReg,FBONLY);
    	    _AH = 0x0d; geninterrupt(0x18); /* text off */
    	    _AH = 0x41; geninterrupt(0x18); /* graphics off */
    	    break;
    	case BOTH:
    	    outport(FBP_ModReg,BOTH);
    	    _AH = 0x0c; geninterrupt(0x18); /* text on	*/
    	    break;
    	default:
    	    return -1;	    /*	��̃��[�h���^����ꂽ	*/
    	}
    }
    else
    {
    	switch (mode)
    	{
    	case PCONLY:
    	    asm in  	al,FBP_cont0
    	    asm and 	al,11001111b
    	    asm out 	FBP_cont0,al	    /*	HFDisp(0);  	*/
    	    asm in  	al,FBP_contHF0
    	    asm or  	al,10000000b
    	    asm out 	FBP_contHF0,al	/*  HFAnDg(0);	    */
    	    break;
    	case FBONLY:
    	    asm in  	al,FBP_cont0
    	    asm and 	al,11001111b
    	    asm or  	al,00010000b
    	    asm out 	FBP_cont0,al	    /*	HFDisp(1)   	*/
    	    asm in  	al,FBP_contHF0
    	    asm and 	al,01111111b
    	    asm out 	FBP_contHF0,al	/*  HFAnDg(1);	    */
    	    break;
    	case BOTH:
    	    asm in  	al,FBP_cont0
    	    asm or  	al,00110000b
    	    asm out 	FBP_cont0,al	    /*	HFDisp(2)   	*/
    	    asm in  	al,FBP_contHF0
    	    asm and 	al,01111111b
    	    asm out 	FBP_contHF0,al	/*  HFAnDg(1);	    */
    	    break;
    	case FBOFF:
    	    asm xor 	al,al
    	    asm out 	FBP_BnkReg1,al	    /*	�����I�Ɏ~�߂�	*/
    	default:
    	    return  -1;
    	}
    }
    return  0;
}

void FBClear()
{
    int     mode;
    if (FB_Type==HyperFrame)
    {
    	mode = inport(FBP_cont0) | HF_RGBWR;	    /*	������������	*/
    	outport(FBP_cont0,mode);
    	for(_SI=0;_SI<400;_SI++)
    	{
    	    asm     mov ax,si
    	    asm     out FBP_Yreg,ax
    	    _CX=640;
    	    _DX=0;
    	xclrloop_hf:;
    	    {
    	    	asm 	mov ax,dx
    	    	asm 	out FBP_Xreg,ax
    	    	asm 	xor al,al
    	    	asm 	out FBP_RedReg,al
    	    	asm 	inc dx
    	    	asm 	loop	xclrloop_hf
    	    }
    	}
    	mode = inport(FBP_cont0)&(~HF_RGBWR);
    	outport(FBP_cont0,mode);
    }
    else
    {
    	asm xor     bx,bx
    	_SF_yloop:
    	    asm mov 	ax,bx
    	    asm out 	FBP_Yreg,ax
    	    asm mov 	cx,640/4
    	    asm xor 	dx,dx
    	    _SF_xloop:
    	    	asm mov     ax,dx
    	    	asm out     FBP_Xreg,ax
    	    	asm xor     ax,ax
    	    	asm out     FBP_RedReg,ax
    	    	asm add     dx,4    	/*  { post increment by 4 } */
    	    asm loop	_SF_xloop
    	    asm inc 	bx
    	    asm cmp 	bx,400
    	asm jne     _SF_yloop
    }
}

/*  �g�e�̃��W�X�^�`�Q�ɏ�������(�G���[�����͂��Ă��Ȃ�)    */

#if 0
void HFRegAWriteByte(int RegNum,int data)
{
    RegNum+=HF_REG_BASEX;
    outport(FBP_Xreg,RegNum);
    outport(FBP_Yreg,HF_REG_BASEY);
    outportb(FBP_GreReg,data);
}
#endif

void HFRegAWriteWord(int RegNum,int data)
{
    RegNum+=HF_REG_BASEX;
    outport(FBP_Xreg,RegNum++);
    outport(FBP_Yreg,HF_REG_BASEY);
    _AL = data&0xff;
    asm     out     FBP_GreReg,al   	/*  Write Low Byte  */
/**/
    outport(FBP_Xreg,RegNum++);
    _AL = data>>8;
    asm     out     FBP_GreReg,al   	/*  Write High byte */
}

#if 0
/*  SF_Scroll�FBy Woody Rinn ported by Tekito	*/
void SF_Scroll(int  pos)
{
    _BX = pos;
    asm     mov     dx,bx
    asm     inc     bx
/**/
    asm     mov     al,01h
    asm     out     FBP_ModReg,al
/**/
    asm     mov     al,01fh
    asm     out     FBP_cont0,al
/**/
_Ready:
    asm     in	    al,FBP_cont1
    asm     and     al,2
    asm     jz	    _Ready
/**/
    asm     mov     al,0Ch
    asm     out     FBP_cont0,al
    asm     mov     al,dh
    asm     not     al
    asm     out     FBP_cont1,al
/**/
    asm     mov     al,0Dh
    asm     out     FBP_cont0,al
    asm     mov     al,dl
    asm     not     al
    asm     out     FBP_cont1,al
/**/
    asm     mov     al,013h
    asm     out     FBP_cont0,al
    asm     mov     al,bh
    asm     not     al
    asm     out     FBP_cont1,al
/**/
    asm     mov     al,014h
    asm     out     FBP_cont0,al
    asm     mov     al,bl
    asm     not     al
    asm     out     FBP_cont1,al
}
#endif

FBScroll(int x,int y)
{
    if (FB_Type==HyperFrame)
    {
    	HFRegAWriteWord(HF_REG_SCROLLX,x);
    	HFRegAWriteWord(HF_REG_SCROLLY,y);
    }
    else
    {
    	/*  SF_Scroll�͂w�͓��삵�Ȃ��悤�Ȃ̂łw�͖������Ă��� */
    	y%=400;     /*	��������Ȃ��Ƌ���	*/
/*  	SF_Scroll(1968-(y/16)*80);  */
    }
}

/*
    ++ scrlbuf start ++
[6:3109] ��ӂ`framebuffer� 92-09-26 11:06 W_rinn #3 21.

>���`�`�`�`�N���`�`�`�r�e�̃X�N���[���̃e�X�g�̌��ʂ������Ă���`�`(;_;)

       ���܂�B����ł͂܂Ƃ��ɓ����Ȃ��B
       ���ʂ͂܂�݂�Ƃ�����������B

       �ꉞ�ȒP�ɐ�������ƁA�ǁ[���r�e�͂P�u���b�N8*16�s�N�Z���̒����`
       ���S����2048�����Ă��ꂪ80*25�Ƀ}�b�s���O����Ă���A�ƁB
       �ŁA��̃\�[�X��2048-80�����W�X�^�ɒ@�����ނ�80�h�b�g��ɏオ���
       �����ǁA���̎���ԉ��̃��C���́A���i�����Ȃ�48�u���b�N�����܂�
       �����āA48�L�����N�^�ڂ���0�u���b�N���n�܂�܂��B
       ����A���x�͂P���̃\�[�X�ɏ]���ă��W�X�^�ɓ����ƂP�L�����N�^
       �����ɃX�N���[������񂾂��ǁA���̎����͂P�u���b�N�i�܂�P�U�h�b�g�j
       �c�ɂ���ĉE�[����o�Ă��܂��B

       ���̐����łǁ[�����u�q�`�l�\���ɂȂ��Ă邩�����������邩�Ǝv���܂�
       �킩��Ȃ��H����Έꔭ�Ȃ񂾂��ǂȂ�:-)

       �ŁA���i�A�N�Z�X�o���Ȃ���ԉ��ɉB��Ă���48�h�b�g���ł����A�ǁ[���
       y=400�`407�Ax=0�`383 �łӂ[�ɓǂݏ����o����݂����ł��B

       �q�h�m�m

Read(3109/3114)>

[6:3110] ��ӂ`framebuffer� 92-09-26 11:12 W_rinn #3 14.

>   	�ŁA��̃\�[�X��2048-80�����W�X�^�ɒ@�����ނ�80�h�b�g��ɏオ���

       ���܂�B16�h�b�g�̊ԈႢ�B

       ���ƁA�ǁ[��

>   	 mov al,$13; out cont0,al
>   	 mov al,bh; not al; out cont1,al;
>   	 mov al,$14; out cont0,al
>   	 mov al,bl; not al; out cont1,al;

       ���̂S�s�͕K�v�Ȃ��݂����ł��B

       �܂��S���P�U���郌�W�X�^�̂قƂ�ǂ��䂾����Ȃ�:-)
*/
