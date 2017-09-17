	ifndef	??version
?debug	macro
	endm
$comm	macro	name,dist,size,count
	comm	dist name:BYTE:count*size
	endm
	else
$comm	macro	name,dist,size,count
	comm	dist name[size]:BYTE:count
	endm
	endif
	?debug	S "fb_.c"
	?debug	C E9A773971A0566625F2E63
	?debug	C E92008251715443A5C54435C494E434C5544455C737464696F2E68
	?debug	C E92008251716443A5C54435C494E434C5544455C7374646C69622E+
	?debug	C 68
	?debug	C E92008251713443A5C54435C494E434C5544455C646F732E68
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
	assume	cs:_TEXT
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
   ;		mode = inport(FBP_cont0)|0x81;
   ;	
	mov	ax,208
	push	ax
	call	near ptr _inport
	inc	sp
	inc	sp
	or	ax,129
	mov	word ptr [bp-2],ax
   ;	
   ;		outport(FBP_cont0,mode);
   ;	
	push	word ptr [bp-2]
	mov	ax,208
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;		if (mode==inport(FBP_cont0))
   ;	
	mov	ax,208
	push	ax
	call	near ptr _inport
	inc	sp
	inc	sp
	cmp	ax,word ptr [bp-2]
	je	@@0
	jmp	@1@362
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
	mov	ax,403
	push	ax
	mov	ax,214
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;			for(l=0x80;l<=0xa1;l++) {
   ;	
	mov	word ptr [bp-4],128
	jmp	short @1@218
@1@170:
   ;	
   ;				outport(FBP_Xreg,l);
   ;	
	push	word ptr [bp-4]
	mov	ax,212
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;				outportb(FBP_GreReg,0);
   ;	
	mov	dx,218
	mov	al,0
	out	dx,al
	inc	word ptr [bp-4]
@1@218:
	cmp	word ptr [bp-4],161
	jle	short @1@170
   ;	
   ;			}
   ;			mode = inport(FBP_cont0) | HF_RGBWR;
   ;	
	mov	ax,208
	push	ax
	call	near ptr _inport
	inc	sp
	inc	sp
	or	ax,4
	mov	word ptr [bp-2],ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	push	word ptr [bp-2]
	mov	ax,208
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;			outport(FBP_Yreg,0x191);
   ;	
	mov	ax,401
	push	ax
	mov	ax,214
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;			for(l=0x180;l<=0x188;l++) {
   ;	
	mov	word ptr [bp-4],384
	jmp	short @1@314
@1@266:
   ;	
   ;				outport(FBP_Xreg,l);
   ;	
	push	word ptr [bp-4]
	mov	ax,212
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;				outportb(FBP_RedReg,0);
   ;	
	mov	dx,216
	mov	al,0
	out	dx,al
	inc	word ptr [bp-4]
@1@314:
	cmp	word ptr [bp-4],392
	jle	short @1@266
   ;	
   ;			}
   ;			mode = inport(FBP_cont0)&(~HF_RGBWR);
   ;	
	mov	ax,208
	push	ax
	call	near ptr _inport
	inc	sp
	inc	sp
	and	ax,65531
	mov	word ptr [bp-2],ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	push	word ptr [bp-2]
	mov	ax,208
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;		}
   ;	
	jmp	short @1@482
@1@362:
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
	jmp	short @1@434
@1@386:
   ;	
   ;			{
   ;				mode = SF_Initdat[l];
   ;	
	mov	bx,word ptr [bp-4]
	shl	bx,1
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
	mov	cl,8
	sar	ax,cl
	mov	dx,210
	out	dx,al
	inc	word ptr [bp-4]
@1@434:
	cmp	word ptr [bp-4],15
	jl	short @1@386
   ;	
   ;			}
   ;			outportb(FBP_ModReg,0);
   ;	
	mov	dx,222
	mov	al,0
	out	dx,al
@1@482:
   ;	
   ;		}
   ;	/*	最終的にフレームバッファがあるか？　のチェックをしている	*/
   ;		outport(FBP_Xreg,0);
   ;	
	xor	ax,ax
	push	ax
	mov	ax,212
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;		outport(FBP_Yreg,0);		/*	XY=0としておいて	*/
   ;	
	xor	ax,ax
	push	ax
	mov	ax,214
	push	ax
	call	near ptr _outport
	add	sp,4
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
	mov	al,170
	out	dx,al
   ;	
   ;		if (inportb(FBP_RedReg)!=0xaa)
   ;	
	in	al,dx
	cmp	al,170
	je	short @1@530
   ;	
   ;		{
   ;			FB_Type=NoFrameBuf;		/*	フレームバッファがない！	*/
   ;	
	mov	word ptr DGROUP:_FB_Type,0
   ;	
   ;			fprintf(stderr,"フレームバッファがありません\n");
   ;	
	mov	ax,offset DGROUP:s@
	push	ax
	mov	ax,offset DGROUP:__streams+32
	push	ax
	call	near ptr _fprintf
	add	sp,4
   ;	
   ;			exit(1);
   ;	
	mov	ax,1
	push	ax
	call	near ptr _exit
	inc	sp
	inc	sp
   ;	
   ;		}
   ;	
	jmp	short @1@554
@1@530:
   ;	
   ;		else
   ;		{
   ;			outportb(FBP_RedReg,l);
   ;	
	mov	dx,216
	mov	al,byte ptr [bp-4]
	out	dx,al
@1@554:
   ;	
   ;		}
   ;	}
   ;	
	mov	sp,bp
	pop	bp
	ret	
_FB_Init	endp
   ;	
   ;	FB_Mode(uchar mode)
   ;	
	assume	cs:_TEXT
_FB_Mode	proc	near
	push	bp
	mov	bp,sp
   ;	
   ;	{
   ;		if (FB_Type==SuperFrame)
   ;	
	cmp	word ptr DGROUP:_FB_Type,1
	jne	short @2@266
   ;	
   ;		{
   ;			switch (mode)
   ;	
	mov	al,byte ptr [bp+4]
	mov	ah,0
	or	ax,ax
	je	short @2@170
	cmp	ax,1
	je	short @2@218
	cmp	ax,3
	je	short @2@194
	jmp	short @2@242
@2@170:
   ;	
   ;			{
   ;			case PCONLY:
   ;				outport(FBP_ModReg,PCONLY);
   ;	
	xor	ax,ax
	push	ax
	mov	ax,222
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;				_AH=0x0c;	geninterrupt(0x18);
   ;	
	mov	ah,12
	int	24
   ;	
   ;				break;
   ;	
	jmp	@2@938
@2@194:
   ;	
   ;			case FBONLY:
   ;				outport(FBP_ModReg,FBONLY);
   ;	
	mov	ax,3
	push	ax
	mov	ax,222
	push	ax
	call	near ptr _outport
	add	sp,4
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
	jmp	@2@938
@2@218:
   ;	
   ;			case BOTH:
   ;				outport(FBP_ModReg,BOTH);
   ;	
	mov	ax,1
	push	ax
	mov	ax,222
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;				_AH = 0x0c;	geninterrupt(0x18);	/* text on	*/
   ;	
	mov	ah,12
	int	24
   ;	
   ;				break;
   ;	
	jmp	@2@938
@2@242:
   ;	
   ;			default:
   ;				return -1;		/*	謎のモードが与えられた	*/
   ;	
	mov	ax,65535
	jmp	@2@962
@2@266:
   ;	
   ;			}
   ;		}
   ;		else
   ;		{
   ;			switch (mode)
   ;	
	mov	al,byte ptr [bp+4]
	mov	ah,0
	mov	bx,ax
	cmp	bx,4
	jbe	@@1
	jmp	@2@914
@@1:
	shl	bx,1
	jmp	word ptr cs:@2@C94[bx]
@2@338:
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
	jmp	short @2@938
@2@506:
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
	jmp	short @2@938
@2@698:
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
	jmp	short @2@938
@2@866:
   ;	
   ;			case FBOFF:
   ;				asm	xor		al,al
   ;	
		xor			al,al
   ;	
   ;				asm	out		FBP_BnkReg1,al		/*	強制的に止める	*/
   ;	
		out			0eeH,al		
@2@914:
   ;	
   ;			default:
   ;				return	-1;
   ;	
	mov	ax,65535
	jmp	short @2@962
@2@938:
   ;	
   ;			}
   ;		}
   ;		return	0;
   ;	
	xor	ax,ax
@2@962:
   ;	
   ;	}
   ;	
	pop	bp
	ret	
_FB_Mode	endp
@2@C94	label	word
	dw	@2@338
	dw	@2@698
	dw	@2@914
	dw	@2@506
	dw	@2@866
   ;	
   ;	void FBClear()
   ;	
	assume	cs:_TEXT
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
	jne	short @3@362
   ;	
   ;		{
   ;			mode = inport(FBP_cont0) | HF_RGBWR;		/*	同時書き込み	*/
   ;	
	mov	ax,208
	push	ax
	call	near ptr _inport
	inc	sp
	inc	sp
	or	ax,4
	mov	di,ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	push	di
	mov	ax,208
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;			for(_SI=0;_SI<400;_SI++)
   ;	
	xor	si,si
	jmp	short @3@314
@3@74:
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
@3@146:
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
	loop	short @3@146
	inc	si
@3@314:
	cmp	si,400
	jb	short @3@74
   ;	
   ;				}
   ;			}
   ;			mode = inport(FBP_cont0)&(~HF_RGBWR);
   ;	
	mov	ax,208
	push	ax
	call	near ptr _inport
	inc	sp
	inc	sp
	and	ax,65531
	mov	di,ax
   ;	
   ;			outport(FBP_cont0,mode);
   ;	
	push	di
	mov	ax,208
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;		}
   ;	
	jmp	short @3@698
@3@362:
   ;	
   ;		else
   ;		{
   ;			asm	xor 	bx,bx
   ;	
		xor	 	bx,bx
@3@386:
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
@3@482:
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
	loop	short @3@482
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
	jne	short @3@386
@3@698:
   ;	
   ;		}
   ;	}
   ;	
	pop	di
	pop	si
	pop	bp
	ret	
_FBClear	endp
   ;	
   ;	void HFRegAWriteWord(int RegNum,int data)
   ;	
	assume	cs:_TEXT
_HFRegAWriteWord	proc	near
	push	bp
	mov	bp,sp
	push	si
	mov	si,word ptr [bp+4]
   ;	
   ;	{
   ;		RegNum+=HF_REG_BASEX;
   ;	
	add	si,128
   ;	
   ;		outport(FBP_Xreg,RegNum++);
   ;	
	mov	ax,si
	inc	si
	push	ax
	mov	ax,212
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;		outport(FBP_Yreg,HF_REG_BASEY);
   ;	
	mov	ax,403
	push	ax
	mov	ax,214
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;		_AL = data&0xff;
   ;	
	mov	al,byte ptr [bp+6]
	and	al,255
   ;	
   ;		asm		out		FBP_GreReg,al		/*	Write Low Byte	*/
   ;	
			out			0daH,al		
   ;	
   ;	/**/
   ;		outport(FBP_Xreg,RegNum++);
   ;	
	mov	ax,si
	inc	si
	push	ax
	mov	ax,212
	push	ax
	call	near ptr _outport
	add	sp,4
   ;	
   ;		_AL = data>>8;
   ;	
	mov	ax,word ptr [bp+6]
	mov	cl,8
	sar	ax,cl
   ;	
   ;		asm		out		FBP_GreReg,al		/*	Write High byte	*/
   ;	
			out			0daH,al		
   ;	
   ;	}
   ;	
	pop	si
	pop	bp
	ret	
_HFRegAWriteWord	endp
   ;	
   ;	FBScroll(int x,int y)
   ;	
	assume	cs:_TEXT
_FBScroll	proc	near
	push	bp
	mov	bp,sp
	push	si
	mov	si,word ptr [bp+6]
   ;	
   ;	{
   ;		if (FB_Type==HyperFrame)
   ;	
	cmp	word ptr DGROUP:_FB_Type,2
	jne	short @5@74
   ;	
   ;		{
   ;			HFRegAWriteWord(HF_REG_SCROLLX,x);
   ;	
	push	word ptr [bp+4]
	mov	ax,4
	push	ax
	call	near ptr _HFRegAWriteWord
	add	sp,4
   ;	
   ;			HFRegAWriteWord(HF_REG_SCROLLY,y);
   ;	
	push	si
	mov	ax,6
	push	ax
	call	near ptr _HFRegAWriteWord
	add	sp,4
   ;	
   ;		}
   ;	
	jmp	short @5@98
@5@74:
   ;	
   ;		else
   ;		{
   ;			/*	SF_ScrollはＸは動作しないようなのでＸは無視している	*/
   ;			y%=400;		/*	これをやらないと恐い	*/
   ;	
	mov	bx,400
	mov	ax,si
	cwd	
	idiv	bx
	mov	si,dx
@5@98:
   ;	
   ;	/*		SF_Scroll(1968-(y/16)*80);	*/
   ;		}
   ;	}
   ;	
	pop	si
	pop	bp
	ret	
_FBScroll	endp
	?debug	C E9
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
	public	_SF_Initdat
	public	_FBScroll
	public	_FB_Type
	extrn	_inport:near
	public	_FB_Init
	public	_FB_Mode
	public	_FBClear
	extrn	_outport:near
	extrn	_fprintf:near
	public	_HFRegAWriteWord
	extrn	__streams:word
	extrn	_exit:near
	end
