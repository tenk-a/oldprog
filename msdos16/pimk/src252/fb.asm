	.286p
	ifndef	??version
?debug	macro
	endm
publicdll macro	name
	public	name
	endm
$comm	macro	name,dist,size,count
	comm	dist name:BYTE:count*size
	endm
	else
$comm	macro	name,dist,size,count
	comm	dist name[size]:BYTE:count
	endm
	endif
	?debug	V 301h
	?debug	S "fb.c"
	?debug	C E90D89F71C0466622E63
	?debug	C E940264D2017563A5C424334355C494E434C5544455C737464696F+
	?debug	C 2E68
	?debug	C E940264D2017563A5C424334355C494E434C5544455C5F64656673+
	?debug	C 2E68
	?debug	C E940264D2018563A5C424334355C494E434C5544455C5F6E66696C+
	?debug	C 652E68
	?debug	C E940264D2017563A5C424334355C494E434C5544455C5F6E756C6C+
	?debug	C 2E68
	?debug	C E940264D2018563A5C424334355C494E434C5544455C7374646C69+
	?debug	C 622E68
	?debug	C E940264D2015563A5C424334355C494E434C5544455C646F732E68
	?debug	C E95296261A086662706F72742E68
	?debug	C E9D073971A0A75736572747970652E68
_TEXT	segment byte public 'CODE'
_TEXT	ends
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
_BSS	ends
_DATA	segment word public 'DATA'
_SF_Initdat	label	word
	db	0
	db	107
	db	1
	db	80
	db	2
	db	91
	db	3
	db	14
	db	4
	db	26
	db	5
	db	2
	db	6
	db	25
	db	7
	db	25
	db	8
	db	128
	db	9
	db	15
	db	12
	db	255
	db	13
	db	255
	db	27
	db	1
	db	30
	db	140
	db	31
	db	0
_FB_Type	label	word
	db	0
	db	0
_DATA	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	FB_Init()
   ;	
	assume	cs:_TEXT,ds:DGROUP
_FB_Init	proc	near
	push	bp
	mov	bp,sp
	sub	sp,4
   ;	
   ;	{
   ;		int		mode,l;
   ;	
   ;	#if 0
   ;		outportb(FBP_BnkReg0,0xc0);		/*	とりあえずＨＦとして初期化	*/
   ;	#endif
   ;		outportb(FBP_BnkReg1,0xc0);		/*	HF Onはされている			*/
   ;	
	mov	dx,238
	mov	al,192
	out	dx,al
   ;	
   ;		outportb(FBP_BnkReg1,0xc0);		/*	HF Onはされている			*/
   ;	
	mov	dx,238
	mov	al,192
	out	dx,al
   ;	
   ;		mode = inport(FBP_cont0)|0x81;
   ;	
	mov	dx,208
	in	ax,dx
	or	ax,129
	mov	word ptr [bp-2],ax
   ;	
   ;		outport(FBP_cont0,mode);
   ;	
	mov	dx,208
	mov	ax,word ptr [bp-2]
	out	dx,ax
   ;	
   ;		if (mode==inport(FBP_cont0))
   ;	
	mov	dx,208
	in	ax,dx
	cmp	ax,word ptr [bp-2]
	je	@@0
	jmp	@1@422
@@0:
   ;	
   ;		{
   ;			FB_Type = HyperFrame;		/*	FB_TypeをＨＦに設定する	*/
   ;	
	mov	word ptr DGROUP:_FB_Type,2
   ;	
   ;			asm	in	al,FBP_cont0
   ;	
		in		al,0d0H
   ;	
   ;			asm	or	al,10000001b		/*	I/O Access & Frame Buf	*/
   ;	
		or		al,10000001b		
   ;	
   ;			asm	out	FBP_cont0,al
   ;	
		out		0d0H,al
   ;	
   ;			outport(FBP_Yreg,0x193);
   ;	
	mov	dx,214
	mov	ax,403
	out	dx,ax
   ;	
   ;			for(l=0x80;l<=0xa1;l++) {
   ;	
	mov	word ptr [bp-4],128
	jmp	short @1@254
@1@198:
   ;	
   ;				outport(FBP_Xreg,l);
   ;	
	mov	dx,212
	mov	ax,word ptr [bp-4]
	out	dx,ax
   ;	
   ;				outportb(FBP_GreReg,0);
   ;	
	mov	dx,218
	mov	al,0
	out	dx,al
	inc	word ptr [bp-4]
@1@254:
	cmp	word ptr [bp-4],161
	jle	short @1@198
   ;	
   ;			}
   ;			mode = inport(FBP_cont0) | HF_RGBWR | HF_EXTON;
   ;	
	mov	dx,208
	in	ax,dx
	or	ax,68
	mov	word ptr [bp-2],ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	mov	dx,208
	mov	ax,word ptr [bp-2]
	out	dx,ax
   ;	
   ;			outport(FBP_Yreg,0x191);
   ;	
	mov	dx,214
	mov	ax,401
	out	dx,ax
   ;	
   ;			for(l=0x180;l<=0x188;l++) {
   ;	
	mov	word ptr [bp-4],384
	jmp	short @1@366
@1@310:
   ;	
   ;				outport(FBP_Xreg,l);
   ;	
	mov	dx,212
	mov	ax,word ptr [bp-4]
	out	dx,ax
   ;	
   ;				outportb(FBP_RedReg,0);
   ;	
	mov	dx,216
	mov	al,0
	out	dx,al
	inc	word ptr [bp-4]
@1@366:
	cmp	word ptr [bp-4],392
	jle	short @1@310
   ;	
   ;			}
   ;			mode = inport(FBP_cont0)&(~HF_RGBWR);
   ;	
	mov	dx,208
	in	ax,dx
	and	ax,-5
	mov	word ptr [bp-2],ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	mov	dx,208
	mov	ax,word ptr [bp-2]
	out	dx,ax
   ;	
   ;		}
   ;	
	jmp	short @1@562
@1@422:
   ;	
   ;		else
   ;		{
   ;			FB_Type = SuperFrame;		/*	FB_TypeをＳＦに			*/
   ;	
	mov	word ptr DGROUP:_FB_Type,1
   ;	
   ;			for(l=0;l<15;l++)
   ;	
	mov	word ptr [bp-4],0
	jmp	short @1@506
@1@450:
   ;	
   ;			{
   ;				mode = SF_Initdat[l];
   ;	
	mov	bx,word ptr [bp-4]
	add	bx,bx
	mov	ax,word ptr DGROUP:_SF_Initdat[bx]
	mov	word ptr [bp-2],ax
   ;	
   ;				outportb(FBP_cont0,mode&0xff);
   ;	
	mov	al,byte ptr [bp-2]
	and	al,255
	mov	dx,208
	out	dx,al
   ;	
   ;				outportb(FBP_cont1,mode>>8);
   ;	
	mov	ax,word ptr [bp-2]
	sar	ax,8
	mov	dx,210
	out	dx,al
	inc	word ptr [bp-4]
@1@506:
	cmp	word ptr [bp-4],15
	jl	short @1@450
   ;	
   ;			}
   ;			outportb(FBP_ModReg,0);
   ;	
	mov	dx,222
	mov	al,0
	out	dx,al
@1@562:
   ;	
   ;		}
   ;	/*	最終的にフレームバッファがあるか？　のチェックをしている	*/
   ;		outport(FBP_Xreg,0);
   ;	
	mov	dx,212
	xor	ax,ax
	out	dx,ax
   ;	
   ;		outport(FBP_Yreg,0);		/*	XY=0としておいて	*/
   ;	
	mov	dx,214
	xor	ax,ax
	out	dx,ax
   ;	
   ;		l = inportb(FBP_RedReg);
   ;	
	mov	dx,216
	in	al,dx
	mov	ah,0
	mov	word ptr [bp-4],ax
   ;	
   ;		outportb(FBP_RedReg,0xaa);
   ;	
	mov	dx,216
	mov	al,170
	out	dx,al
   ;	
   ;		if (inportb(FBP_RedReg)!=0xaa)
   ;	
	mov	dx,216
	in	al,dx
	cmp	al,170
	je	short @1@618
   ;	
   ;		{
   ;			FB_Type=NoFrameBuf;		/*	フレームバッファがない！	*/
   ;	
	mov	word ptr DGROUP:_FB_Type,0
   ;	
   ;			fprintf(stderr,"フレームバッファがありません\n");
   ;	
	push	offset DGROUP:s@
	push	offset DGROUP:__streams+32
	call	near ptr _fprintf
	add	sp,4
   ;	
   ;			Exit(1);
   ;	
	push	1
	call	near ptr _Exit
	add	sp,2
   ;	
   ;		}
   ;	
	leave	
	ret	
@1@618:
   ;	
   ;		else
   ;		{
   ;			outportb(FBP_RedReg,l);
   ;	
	mov	dx,216
	mov	al,byte ptr [bp-4]
	out	dx,al
   ;	
   ;		}
   ;	}
   ;	
	leave	
	ret	
_FB_Init	endp
   ;	
   ;	FB_Mode(uchar mode)
   ;	
	assume	cs:_TEXT,ds:DGROUP
_FB_Mode	proc	near
	push	bp
	mov	bp,sp
	mov	cl,byte ptr [bp+4]
   ;	
   ;	{
   ;		if (FB_Type==SuperFrame)
   ;	
	cmp	word ptr DGROUP:_FB_Type,1
	jne	short @2@282
   ;	
   ;		{
   ;			switch (mode)
   ;	
	mov	al,cl
	mov	ah,0
	or	ax,ax
	je	short @2@198
	cmp	ax,1
	je	short @2@254
	cmp	ax,3
	je	short @2@226
	jmp	@2@1038
@2@198:
   ;	
   ;			{
   ;			case PCONLY:
   ;				outport(FBP_ModReg,PCONLY);
   ;	
	mov	dx,222
	xor	ax,ax
	out	dx,ax
   ;	
   ;				_AH=0x0c;	geninterrupt(0x18);
   ;	
	mov	ah,12
	int	24
   ;	
   ;				break;
   ;	
	jmp	@2@1066
@2@226:
   ;	
   ;			case FBONLY:
   ;				outport(FBP_ModReg,FBONLY);
   ;	
	mov	dx,222
	mov	ax,3
	out	dx,ax
   ;	
   ;				_AH = 0x0d;	geninterrupt(0x18);	/* text off	*/
   ;	
	mov	ah,13
	int	24
   ;	
   ;				_AH = 0x41;	geninterrupt(0x18);	/* graphics off	*/
   ;	
	mov	ah,65
	int	24
   ;	
   ;				break;
   ;	
	jmp	@2@1066
@2@254:
   ;	
   ;			case BOTH:
   ;				outport(FBP_ModReg,BOTH);
   ;	
	mov	dx,222
	mov	ax,1
	out	dx,ax
   ;	
   ;				_AH = 0x0c;	geninterrupt(0x18);	/* text on	*/
   ;	
	mov	ah,12
	int	24
   ;	
   ;				break;
   ;	
	jmp	@2@1066
@2@282:
   ;	
   ;			default:
   ;				return -1;		/*	謎のモードが与えられた	*/
   ;			}
   ;		}
   ;		else
   ;		{
   ;			switch (mode)
   ;	
	mov	al,cl
	mov	ah,0
	mov	bx,ax
	cmp	bx,4
	jbe	@@1
	jmp	@2@1038
@@1:
	add	bx,bx
	jmp	word ptr cs:@2@C434[bx]
@2@366:
   ;	
   ;			{
   ;			case PCONLY:
   ;				asm	in		al,FBP_cont0
   ;	
		in			al,0d0H
   ;	
   ;				asm	and		al,11001111b
   ;	
		and			al,11001111b
   ;	
   ;				asm	out		FBP_cont0,al		/*	HFDisp(0);		*/
   ;	
		out			0d0H,al		
   ;	
   ;				asm	in		al,FBP_contHF0
   ;	
		in			al,0d3H
   ;	
   ;				asm	or		al,10000000b
   ;	
		or			al,10000000b
   ;	
   ;				asm	out		FBP_contHF0,al	/*	HFAnDg(0);		*/
   ;	
		out			0d3H,al	
   ;	
   ;				break;
   ;	
	jmp	short @2@1066
@2@562:
   ;	
   ;			case FBONLY:
   ;				asm	in		al,FBP_cont0
   ;	
		in			al,0d0H
   ;	
   ;				asm	and		al,11001111b
   ;	
		and			al,11001111b
   ;	
   ;				asm	or		al,00010000b
   ;	
		or			al,00010000b
   ;	
   ;				asm	out		FBP_cont0,al		/*	HFDisp(1)		*/
   ;	
		out			0d0H,al		
   ;	
   ;				asm	in		al,FBP_contHF0
   ;	
		in			al,0d3H
   ;	
   ;				asm	and		al,01111111b
   ;	
		and			al,01111111b
   ;	
   ;				asm	out		FBP_contHF0,al	/*	HFAnDg(1);		*/
   ;	
		out			0d3H,al	
   ;	
   ;				break;
   ;	
	jmp	short @2@1066
@2@786:
   ;	
   ;			case BOTH:
   ;				asm	in		al,FBP_cont0
   ;	
		in			al,0d0H
   ;	
   ;				asm	or		al,00110000b
   ;	
		or			al,00110000b
   ;	
   ;				asm	out		FBP_cont0,al		/*	HFDisp(2)		*/
   ;	
		out			0d0H,al		
   ;	
   ;				asm	in		al,FBP_contHF0
   ;	
		in			al,0d3H
   ;	
   ;				asm	and		al,01111111b
   ;	
		and			al,01111111b
   ;	
   ;				asm	out		FBP_contHF0,al	/*	HFAnDg(1);		*/
   ;	
		out			0d3H,al	
   ;	
   ;				break;
   ;	
	jmp	short @2@1066
@2@982:
   ;	
   ;			case FBOFF:
   ;				asm	xor		al,al
   ;	
		xor			al,al
   ;	
   ;				asm	out		FBP_BnkReg1,al		/*	強制的に止める	*/
   ;	
		out			0eeH,al		
@2@1038:
   ;	
   ;				/*break;*/
   ;			default:
   ;				return	-1;
   ;	
	mov	ax,-1
	pop	bp
	ret	
@2@1066:
   ;	
   ;			}
   ;		}
   ;		return	0;
   ;	
	xor	ax,ax
   ;	
   ;	}
   ;	
	pop	bp
	ret	
_FB_Mode	endp
@2@C434	label	word
	dw	@2@366
	dw	@2@786
	dw	@2@1038
	dw	@2@562
	dw	@2@982
   ;	
   ;	void FBClear()
   ;	
	assume	cs:_TEXT,ds:DGROUP
_FBClear	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
   ;	
   ;	{
   ;		int		mode;
   ;		if (FB_Type==HyperFrame)
   ;	
	cmp	word ptr DGROUP:_FB_Type,2
	jne	short @3@422
   ;	
   ;		{
   ;			mode = inport(FBP_cont0) | HF_RGBWR;		/*	同時書き込み	*/
   ;	
	mov	dx,208
	in	ax,dx
	or	ax,4
	mov	di,ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	mov	dx,208
	mov	ax,di
	out	dx,ax
   ;	
   ;			for(_SI=0;_SI<400;_SI++)
   ;	
	xor	si,si
	jmp	short @3@366
@3@86:
   ;	
   ;			{
   ;				asm		mov	ax,si
   ;	
			mov		ax,si
   ;	
   ;				asm		out	FBP_Yreg,ax
   ;	
			out		0d6H,ax
   ;	
   ;				_CX=640;
   ;	
	mov	cx,640
   ;	
   ;				_DX=0;
   ;	
	xor	dx,dx
@3@170:
   ;	
   ;			xclrloop_hf:;
   ;				{
   ;					asm		mov	ax,dx
   ;	
			mov		ax,dx
   ;	
   ;					asm		out	FBP_Xreg,ax
   ;	
			out		0d4H,ax
   ;	
   ;					asm		xor	al,al
   ;	
			xor		al,al
   ;	
   ;					asm		out	FBP_RedReg,al
   ;	
			out		0d8H,al
   ;	
   ;					asm		inc	dx
   ;	
			inc		dx
   ;	
   ;					asm		loop	xclrloop_hf
   ;	
	loop	short @3@170
	inc	si
@3@366:
	cmp	si,400
	jb	short @3@86
   ;	
   ;				}
   ;			}
   ;			mode = inport(FBP_cont0)&(~HF_RGBWR);
   ;	
	mov	dx,208
	in	ax,dx
	and	ax,-5
	mov	di,ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	mov	dx,208
	mov	ax,di
	out	dx,ax
   ;	
   ;		}
   ;	
	jmp	short @3@814
@3@422:
   ;	
   ;		else
   ;		{
   ;		  #if 1
   ;			asm	xor 	bx,bx
   ;	
		xor	 	bx,bx
@3@450:
   ;	
   ;			_SF_yloop:
   ;				asm	mov 	ax,bx
   ;	
		mov	 	ax,bx
   ;	
   ;				asm	out 	FBP_Yreg,ax
   ;	
		out	 	0d6H,ax
   ;	
   ;				asm	mov 	cx,640/4
   ;	
		mov	 	cx,640/4
   ;	
   ;				asm	xor 	dx,dx
   ;	
		xor	 	dx,dx
@3@562:
   ;	
   ;				_SF_xloop:
   ;					asm	mov 	ax,dx
   ;	
		mov	 	ax,dx
   ;	
   ;					asm	out 	FBP_Xreg,ax
   ;	
		out	 	0d4H,ax
   ;	
   ;					asm	xor 	ax,ax
   ;	
		xor	 	ax,ax
   ;	
   ;					asm	out		FBP_RedReg,ax
   ;	
		out			0d8H,ax
   ;	
   ;					asm	add 	dx,4		/*	{ post increment by 4 }	*/
   ;	
		add	 	dx,4		
   ;	
   ;				asm	loop	_SF_xloop
   ;	
	loop	short @3@562
   ;	
   ;				asm	inc 	bx
   ;	
		inc	 	bx
   ;	
   ;				asm	cmp 	bx,400
   ;	
		cmp	 	bx,400
   ;	
   ;			asm	jne 	_SF_yloop
   ;	
	jne	short @3@450
@3@814:
   ;	
   ;		  #else
   ;			outportb(FBP_ModReg, inportb(FBP_ModReg)|SF_AUTOINC);
   ;			asm	xor 	bx,bx
   ;			_SF_yloop:
   ;				asm	mov 	ax,bx
   ;				asm	out 	FBP_Yreg,ax
   ;				asm	mov 	cx,640/4
   ;				asm xor 	ax,ax	//
   ;				_SF_xloop:
   ;					asm	out		FBP_RedReg,al
   ;				asm	loop	_SF_xloop
   ;				asm	inc 	bx
   ;				asm	cmp 	bx,400
   ;			asm	jne 	_SF_yloop
   ;			outportb(FBP_ModReg, inportb(FBP_ModReg)&(~SF_AUTOINC));
   ;		  #endif
   ;		}
   ;	}
   ;	
	pop	di
	pop	si
	pop	bp
	ret	
_FBClear	endp
	?debug	C E9
	?debug	C FA00000000
_TEXT	ends
_DATA	segment word public 'DATA'
s@	label	byte
	db	-125
	db	't'
	db	-125
	db	-116
	db	-127
	db	'['
	db	-125
	db	-128
	db	-125
	db	'o'
	db	-125
	db	'b'
	db	-125
	db	't'
	db	-125
	db	'@'
	db	-126
	db	-86
	db	-126
	db	-96
	db	-126
	db	-24
	db	-126
	db	-36
	db	-126
	db	-71
	db	-126
	db	-15
	db	10
	db	0
_DATA	ends
_TEXT	segment byte public 'CODE'
_TEXT	ends
_s@	equ	s@
	extrn	__streams:word
	extrn	_fprintf:near
	public	_SF_Initdat
	public	_FB_Type
	public	_FB_Init
	extrn	_Exit:near
	public	_FB_Mode
	public	_FBClear
	end
