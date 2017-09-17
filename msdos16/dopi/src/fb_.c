/*
    ＳＦ／ＨＦの共用化ルーチン
    オリジナルは
    	Super-Frame and HyPER-Frame Common unit for Turbo Pascal 6.0
    	By Woody-Rinn 1991/02

●権利関係に関して
　本プログラムはフリーソフトです。その性質上、使用および配布に対する制限は一切
設けません。
　出来れば、使用した場合には「fb.c」使用したと書いて頂けると幸いですが、強制は
しません。
　また、作者は本プログラムを使用したことによるいかなる問題に対しても一切の責任
は取りません。

●著作者に関して
　このプログラムはW_Rinn先生がTurbo-Pascalで書かれた"FB.PAS"を元に、TEKITOが改
訂及び移植を行いました。
　そこで権利関係は以下の様になります。
    原著作者：	W_Rinn
    著作者：	TEKITO
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
    outportb(FBP_BnkReg0,0xc0);     /*	とりあえずＨＦとして初期化  */
#endif
    outportb(FBP_BnkReg1,0xc0);     /*	HF Onはされている   	    */
    mode = inport(FBP_cont0)|0x81;
    outport(FBP_cont0,mode);
    if (mode==inport(FBP_cont0))
    {
    	FB_Type = HyperFrame;	    /*	FB_TypeをＨＦに設定する */
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
    	FB_Type = SuperFrame;	    /*	FB_TypeをＳＦに     	*/
    	for(l=0;l<15;l++)
    	{
    	    mode = SF_Initdat[l];
    	    outportb(FBP_cont0,mode&0xff);
    	    outportb(FBP_cont1,mode>>8);
    	}
    	outportb(FBP_ModReg,0);
    }
/*  最終的にフレームバッファがあるか？　のチェックをしている	*/
    outport(FBP_Xreg,0);
    outport(FBP_Yreg,0);    	/*  XY=0としておいて	*/
    l = inportb(FBP_RedReg);
    outportb(FBP_RedReg,0xaa);
    if (inportb(FBP_RedReg)!=0xaa)
    {
    	FB_Type=NoFrameBuf; 	/*  フレームバッファがない！	*/
    	fprintf(stderr,"フレームバッファがありません\n");
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
    	    return -1;	    /*	謎のモードが与えられた	*/
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
    	    asm out 	FBP_BnkReg1,al	    /*	強制的に止める	*/
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
    	mode = inport(FBP_cont0) | HF_RGBWR;	    /*	同時書き込み	*/
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

/*  ＨＦのレジスタＡ群に書き込む(エラー処理はしていない)    */

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
/*  SF_Scroll：By Woody Rinn ported by Tekito	*/
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
    	/*  SF_ScrollはＸは動作しないようなのでＸは無視している */
    	y%=400;     /*	これをやらないと恐い	*/
/*  	SF_Scroll(1968-(y/16)*80);  */
    }
}

/*
    ++ scrlbuf start ++
[6:3109] ｢ふつ～framebuffer｣ 92-09-26 11:06 W_rinn #3 21.

>う～～～～誰か～～～ＳＦのスクロールのテストの結果を教えてくれ～～(;_;)

       すまん。あれではまともに動かない。
       結果はまるみんといっしょっす。

       一応簡単に説明すると、どーもＳＦは１ブロック8*16ピクセルの長方形
       が全部で2048個あってこれが80*25にマッピングされている、と。
       で、例のソースで2048-80をレジスタに叩き込むと80ドット上に上がるん
       だけど、この時一番下のラインは、普段見えない48ブロック分がまず
       見えて、48キャラクタ目から0ブロックが始まります。
       も一つ、今度は１を例のソースに従ってレジスタに入れると１キャラクタ
       分左にスクロールするんだけど、この時横は１ブロック（つまり１６ドット）
       縦にずれて右端から出てきます。

       この説明でどーいうＶＲＡＭ構造になってるかだいたい判るかと思います
       わからない？見れば一発なんだけどなぁ:-)

       で、普段アクセス出来ない一番下に隠れている48ドット分ですが、どーやら
       y=400～407、x=0～383 でふつーに読み書き出来るみたいです。

       ＲＩＮＮ

Read(3109/3114)>

[6:3110] ｢ふつ～framebuffer｣ 92-09-26 11:12 W_rinn #3 14.

>   	で、例のソースで2048-80をレジスタに叩き込むと80ドット上に上がるん

       すまん。16ドットの間違い。

       あと、どーも

>   	 mov al,$13; out cont0,al
>   	 mov al,bh; not al; out cont1,al;
>   	 mov al,$14; out cont0,al
>   	 mov al,bl; not al; out cont1,al;

       この４行は必要ないみたいです。

       まだ全く１６個あるレジスタのほとんどが謎だからなぁ:-)
*/
