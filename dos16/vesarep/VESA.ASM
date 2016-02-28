;	module VESA
;		c_name "VESA","_",1
;	
;	
;	struct INFO
	.186
DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
;		id:d			//00	'VESA'
;		version:w		//04
;		oemNames:d		//06
;		flags:b(4)		//0A
;		videoModeTbl:d	//0E
;		totalMem:w		//12
;		//rsv:b(242)	//14
;	endstruct
;	
;	struct MODEINFO
;		mode:w			//00
;		winAatr:b		//02
;		winBatr:b		//03
;		winPagSiz:w		//04	N Kbytes
;		winSiz:w		//06	N Kbytes
;		winAseg:w		//08
;		winBseg:w		//0A
;		winAdr:d		//0C
;		scanlineBytes:w	//10
;		gxsz:w			//12
;		gysz:w			//14
;		chrXsz:b		//16
;		chrYsz:b		//17
;		plnCnt:b		//18
;		pbits:b			//19
;		bnkCnt:b		//1A
;		type:b			//1B
;		bnkSz:b			//1C
;		imgPagCnt:b		//1D
;		rsv1:b			//1E = 1
;		mskBitsR:b		//1F
;		mskBitPosR:b	//20
;		mskBitsG:b		//21
;		mskBitPosG:b	//22
;		mskBitsB:b		//23
;		mskBitPosB:b	//24
;		mskBitsRSV:b	//25
;		mskBitPosRSV:b	//26
;		drctColAtr:b	//27
;		rsv:b(216)		//28
;	endstruct
;	
;	
;	*proc GetInfo(infp:d);far;cdecl
;	enter
;		save cx,si,di,es,ds
;		local buf:b(256)
;		
;		ax = 0x4F00
_BSS	ends

_TEXT	segment byte public 'CODE'
	public	_VESA_GetInfo
_VESA_GetInfo	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	si
	push	di
	push	ds
	push	es
	enter	0100h,00h
	mov	ax,04F00h
;		es.di = ww(ss, &buf)
	lea	di,[bp-256]
	push	ss
	pop	es
;		intr 0x10
	int	010h
;		go (ax != 0x004f) ERR
	cmp	ax,04Fh
	jne	L$1
;	
;		es.di = infp
	les	di,dword ptr [bp+16]
;		ds.si = ww(ss,&buf)
	lea	si,[bp-256]
	push	ss
	pop	ds
;		rep.cpy.w di, si, sizeof(INFO)/2
	mov	cx,0Ah
	rep	movsw
;	
;		ax = 0
	xor	ax,ax
;	 RET:
L$2:
;		return
	db	0C9h	;leave
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$1:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$2
;	endproc

;	
;	
;	*proc GetModeInfo(mode:w,minfp:d);cdecl;far
;	enter
;		save cx,di,es
;		
;		cx = mode
	public	_VESA_GetModeInfo
_VESA_GetModeInfo	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	di
	push	es
	push	bp
	mov	bp,sp
	mov	cx,word ptr [bp+12]
;		ax = 0x4F01
	mov	ax,04F01h
;		es.di = minfp
	les	di,dword ptr [bp+14]
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$3
;		ax = 0
	xor	ax,ax
;	 RET:
L$4:
;		return
	db	0C9h	;leave
	pop	es
	pop	di
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$3:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$4
;	endproc

;	
;	
;	*proc SetMode(mode:w);cdecl;far
;	enter
;		save bx
;		
;		bx = mode
	public	_VESA_SetMode
_VESA_SetMode	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	bx
	push	bp
	mov	bp,sp
	mov	bx,word ptr [bp+8]
;		ax = 0x4F02
	mov	ax,04F02h
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$5
;		ax = 0
	xor	ax,ax
;	 RET:
L$6:
;		return
	db	0C9h	;leave
	pop	bx
	db	0CBh	;retf
;	 ERR:
L$5:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$6
;	endproc

;	
;	
;	*proc GetMode();cdecl;far
;	begin
;		save bx
;		
;		ax = 0x4F03
	public	_VESA_GetMode
_VESA_GetMode	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	bx
	mov	ax,04F03h
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$7
;		ax = bx
	mov	ax,bx
;	 RET:
L$8:
;		return
	pop	bx
	db	0CBh	;retf
;	 ERR:
L$7:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$8
;	endproc

;	
;	
;	
;	*proc GetVideoStatSize(stt:w);cdecl;far
;	enter
;		save bx,cx,dx
;		
;		cx = stt
	public	_VESA_GetVideoStatSize
_VESA_GetVideoStatSize	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	dx
	push	bx
	push	bp
	mov	bp,sp
	mov	cx,word ptr [bp+12]
;		ax = 0x4F04
	mov	ax,04F04h
;		dx = 0
	xor	dx,dx
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$9
;		ax = bx
	mov	ax,bx
;	 RET:
L$10:
;		return
	db	0C9h	;leave
	pop	bx
	pop	dx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$9:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$10
;	endproc

;	
;	
;	*proc GetVideoStat(stt:w,sttBuf:d);cdecl;far
;	enter
;		save bx,cx,dx
;		
;		cx = stt
	public	_VESA_GetVideoStat
_VESA_GetVideoStat	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	dx
	push	bx
	push	bp
	mov	bp,sp
	mov	cx,word ptr [bp+12]
;		ax = 0x4F04
	mov	ax,04F04h
;		dx = 1
	mov	dx,01h
;		es.bx = sttBuf
	les	bx,dword ptr [bp+14]
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$11
;		ax = 0
	xor	ax,ax
;	 RET:
L$12:
;		return
	db	0C9h	;leave
	pop	bx
	pop	dx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$11:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$12
;	endproc

;	
;	*proc SetVideoStat(stt:w,sttBuf:d);cdecl;far
;	enter
;		save cx,dx
;		
;		cx = stt
	public	_VESA_SetVideoStat
_VESA_SetVideoStat	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	dx
	push	bp
	mov	bp,sp
	mov	cx,word ptr [bp+10]
;		ax = 0x4F04
	mov	ax,04F04h
;		dx = 2
	mov	dx,02h
;		es.bx = sttBuf
	les	bx,dword ptr [bp+12]
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$13
;		ax = 0
	xor	ax,ax
;	 RET:
L$14:
;		return
	db	0C9h	;leave
	pop	dx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$13:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$14
;	endproc

;	
;	*proc SetWinPos(num:w, pos:w);cdecl;far
;	enter
;		save bx,dx
;		
;		ax = 0x4F05
	public	_VESA_SetWinPos
_VESA_SetWinPos	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	dx
	push	bx
	push	bp
	mov	bp,sp
	mov	ax,04F05h
;		bh = 0
	xor	bh,bh
;		bl = num.l
	mov	bl,byte ptr [bp+10]
;		dx =  pos
	mov	dx,word ptr [bp+12]
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$15
;		ax = 0
	xor	ax,ax
;	 RET:
L$16:
;		return
	db	0C9h	;leave
	pop	bx
	pop	dx
	db	0CBh	;retf
;	 ERR:
L$15:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$16
;	endproc

;	
;	*proc GetWinPos(num:w);cdecl;far
;	enter
;		save bx,dx
;		
;		ax = 0x4F05
	public	_VESA_GetWinPos
_VESA_GetWinPos	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	dx
	push	bx
	push	bp
	mov	bp,sp
	mov	ax,04F05h
;		bh = 0
	xor	bh,bh
;		bl = num.l
	mov	bl,byte ptr [bp+10]
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$17
;		ax = dx
	mov	ax,dx
;	 RET:
L$18:
;		return
	db	0C9h	;leave
	pop	bx
	pop	dx
	db	0CBh	;retf
;	 ERR:
L$17:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$18
;	endproc

;	
;	*proc SetLogicalXsize(xsz:w);cdecl;far
;		// ret ax=表示xsz dx=論理xsz
;	enter
;		save bx,cx
;		
;		cx = xsz
	public	_VESA_SetLogicalXsize
_VESA_SetLogicalXsize	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	push	bp
	mov	bp,sp
	mov	cx,word ptr [bp+10]
;		bx = 0
	xor	bx,bx
;		ax = 0x4F06
	mov	ax,04F06h
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$19
;		ax = cx
	mov	ax,cx
;	 RET:
L$20:
;		return
	db	0C9h	;leave
	pop	bx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$19:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$20
;	endproc

;	
;	
;	*proc GetLogicalXsize();cdecl;far
;		// ret ax=表示xsz dx=論理xsz
;	begin
;		save bx,cx
;		
;		bx = 1
	public	_VESA_GetLogicalXsize
_VESA_GetLogicalXsize	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	mov	bx,01h
;		ax = 0x4F06
	mov	ax,04F06h
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$21
;		ax = cx
	mov	ax,cx
;	 RET:
L$22:
;		return
	pop	bx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$21:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$22
;	endproc

;	
;	
;	*proc SetOrgPos(x0:w,y0:w);cdecl;far
;	enter
;		save bx,cx
;		
;		bx = 0
	public	_VESA_SetOrgPos
_VESA_SetOrgPos	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	push	bp
	mov	bp,sp
	xor	bx,bx
;		ax = 0x4F07
	mov	ax,04F07h
;		cx = x0
	mov	cx,word ptr [bp+10]
;		dx = y0
	mov	dx,word ptr [bp+12]
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$23
;		ax = 0
	xor	ax,ax
;	 RET:
L$24:
;		return
	db	0C9h	;leave
	pop	bx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$23:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$24
;	endproc

;	
;	
;	*proc GetOrgPos();cdecl;far
;		// ret ax=x0 dx=y0
;	begin
;		save bx,cx
;		
;		bx = 1
	public	_VESA_GetOrgPos
_VESA_GetOrgPos	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	mov	bx,01h
;		ax = 0x4F07
	mov	ax,04F07h
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$25
;		ax = cx
	mov	ax,cx
;	 RET:
L$26:
;		return
	pop	bx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$25:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$26
;	endproc

;	
;	
;	*proc SetPalMode(palbit:w);cdecl;far
;		// ret ax=表示色のビット数
;		// ret ax=実際の表示色のビット数
;	enter
;		save bx,cx
;		
;		bx = 0
	public	_VESA_SetPalMode
_VESA_SetPalMode	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	push	bp
	mov	bp,sp
	xor	bx,bx
;		bh = palbit.l
	mov	bh,byte ptr [bp+10]
;		ax = 0x4F08
	mov	ax,04F08h
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$27
;		ax = 0
	xor	ax,ax
;		al = bh
	mov	al,bh
;	 RET:
L$28:
;		return
	db	0C9h	;leave
	pop	bx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$27:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$28
;	endproc

;	
;	
;	*proc GetPalMode();cdecl;far
;		// ret ax=表示色のビット数
;	begin
;		save bx,cx
;		bx = 1
	public	_VESA_GetPalMode
_VESA_GetPalMode	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	mov	bx,01h
;		ax = 0x4F08
	mov	ax,04F08h
;		intr 0x10
	int	010h
;		go (al != 0x4f) ERR
	cmp	al,04Fh
	jne	L$29
;		ax = 0
	xor	ax,ax
;		al = bh
	mov	al,bh
;	 RET:
L$30:
;		return
	pop	bx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$29:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$30
;	endproc

;	
;	
;	
;	
;	
;	
;	//---------------------------------------------------------------------------
;	
;	@if 0
;	*const S320x200=0,S640x480,S800x600,S1024x768,S1280x1024
;	strcut SMOD
;		/*txt:w*/
;		/*p4:w*/
;		p8:w
;		p15:w
;		p16:w
;		p24:w
;		/*dmy:d*/
;	endstruct
;	
;	*cvar  smd:SMOD[6] = data
;		w(4)	/* 0x00, 0x0d,*/  0x13,0x10D,0x10E,0x10F	//320x200
;		w(4)	/*0x108, 0x12,*/ 0x101,0x110,0x111,0x112	//640x480
;		w(4)	/* 0x03,0x102,*/ 0x103,0x113,0x114,0x115	//800x600
;		w(4)	/* 0x03,0x104,*/ 0x105,0x116,0x117,0x118	//1024x768
;		w(4)	/* 0x03,0x106,*/ 0x107,0x119,0x11A,0x11B	//1280x1024
;	enddata
;	@endif
;	
;	*const M320x200=0x01,M640x480=0x02,M800x600=0x04,M1024x768=0x08,M1280x1024=0x10
;	*cvar  m8s:b,m15s:b,m16s:b,m24s:b
	public	_VESA_m8s
_VESA_m8s	label	byte
	db	1 dup(?)
	public	_VESA_m15s
_VESA_m15s	label	byte
	db	1 dup(?)
	public	_VESA_m16s
_VESA_m16s	label	byte
	db	1 dup(?)
	public	_VESA_m24s
_VESA_m24s	label	byte
	db	1 dup(?)
;	
;	
;	*cvar  info:INFO,modeInfo:MODEINFO
	public	_VESA_info
_VESA_info	label	byte
	db	20 dup(?)
	public	_VESA_modeInfo
_VESA_modeInfo	label	byte
	db	256 dup(?)
;	
;	*proc Init();far
;	begin
;		save bx,cx,dx,si,di,es,ds
;	
;		m8s = al = 0
	public	_VESA_Init
_VESA_Init	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	xor	al,al
	mov	byte ptr cs:[_VESA_m8s],al
;		m15s = al
	mov	byte ptr cs:[_VESA_m15s],al
;		m16s = al
	mov	byte ptr cs:[_VESA_m16s],al
;		m24s = al
	mov	byte ptr cs:[_VESA_m24s],al
;	
;		// VESA-BIOSが存在するかどうか
;		GetInfo ww(cs,%info)
	push	cs
	push	offset _TEXT:_VESA_info
	call far ptr	_VESA_GetInfo
	add	sp,04h
;		go.w (ax) ERR
	or	ax,ax
	je	L$32
	jmp	L$31
L$32:
;	
;		// 利用できる画面モードを確認する
;		m8s |= M320x200
	or	byte ptr cs:[_VESA_m8s],01h
;		ds.si = info.videoModeTbl
	lds	si,dword ptr cs:[_VESA_info+14]
;		loop.w
L$33:
;			rep.load.w ax,si
	lodsw
;			if.w (ax == 0x101);		m8s  |= M640x480;
	cmp	ax,0101h
	je	L$37
	jmp	L$36
L$37:
	or	byte ptr cs:[_VESA_m8s],02h
;			elsif (ax == 0x103);	m8s  |= M800x600;
	jmp	L$38
L$36:
	cmp	ax,0103h
	jne	L$39
	or	byte ptr cs:[_VESA_m8s],04h
;			elsif (ax == 0x105);	m8s  |= M1024x768;
	jmp	L$38
L$39:
	cmp	ax,0105h
	jne	L$40
	or	byte ptr cs:[_VESA_m8s],08h
;			elsif (ax == 0x107);	m8s  |= M1280x1024;
	jmp	L$38
L$40:
	cmp	ax,0107h
	jne	L$41
	or	byte ptr cs:[_VESA_m8s],010h
;			elsif (ax == 0x10D);	m15s |= M320x200;
	jmp	L$38
L$41:
	cmp	ax,010Dh
	jne	L$42
	or	byte ptr cs:[_VESA_m15s],01h
;			elsif (ax == 0x110);	m15s |= M640x480;
	jmp	L$38
L$42:
	cmp	ax,0110h
	jne	L$43
	or	byte ptr cs:[_VESA_m15s],02h
;			elsif (ax == 0x113);	m15s |= M800x600;
	jmp	L$38
L$43:
	cmp	ax,0113h
	jne	L$44
	or	byte ptr cs:[_VESA_m15s],04h
;			elsif (ax == 0x116);	m15s |= M1024x768;
	jmp	L$38
L$44:
	cmp	ax,0116h
	jne	L$45
	or	byte ptr cs:[_VESA_m15s],08h
;			elsif (ax == 0x119);	m15s |= M1280x1024;
	jmp	L$38
L$45:
	cmp	ax,0119h
	jne	L$46
	or	byte ptr cs:[_VESA_m15s],010h
;			elsif (ax == 0x10E);	m16s |= M320x200;
	jmp	L$38
L$46:
	cmp	ax,010Eh
	jne	L$47
	or	byte ptr cs:[_VESA_m16s],01h
;			elsif (ax == 0x111);	m16s |= M640x480;
	jmp	L$38
L$47:
	cmp	ax,0111h
	jne	L$48
	or	byte ptr cs:[_VESA_m16s],02h
;			elsif (ax == 0x114);	m16s |= M800x600;
	jmp	L$38
L$48:
	cmp	ax,0114h
	jne	L$49
	or	byte ptr cs:[_VESA_m16s],04h
;			elsif (ax == 0x117);	m16s |= M1024x768;
	jmp	L$38
L$49:
	cmp	ax,0117h
	jne	L$50
	or	byte ptr cs:[_VESA_m16s],08h
;			elsif (ax == 0x11A);	m16s |= M1280x1024;
	jmp	L$38
L$50:
	cmp	ax,011Ah
	jne	L$51
	or	byte ptr cs:[_VESA_m16s],010h
;			elsif (ax == 0x10F);	m24s |= M320x200;
	jmp	L$38
L$51:
	cmp	ax,010Fh
	jne	L$52
	or	byte ptr cs:[_VESA_m24s],01h
;			elsif (ax == 0x112);	m24s |= M640x480;
	jmp	L$38
L$52:
	cmp	ax,0112h
	jne	L$53
	or	byte ptr cs:[_VESA_m24s],02h
;			elsif (ax == 0x115);	m24s |= M800x600;
	jmp	L$38
L$53:
	cmp	ax,0115h
	jne	L$54
	or	byte ptr cs:[_VESA_m24s],04h
;			elsif (ax == 0x118);	m24s |= M1024x768;
	jmp	L$38
L$54:
	cmp	ax,0118h
	jne	L$55
	or	byte ptr cs:[_VESA_m24s],08h
;			elsif (ax == 0x11B);	m24s |= M1280x1024;
	jmp	L$38
L$55:
	cmp	ax,011Bh
	jne	L$56
	or	byte ptr cs:[_VESA_m24s],010h
;			endif
L$56:
L$38:
;		endloop (ax != 0xffff)
	cmp	ax,0FFFFh
	je	L$57
	jmp	L$33
L$57:
;	
;		ax = 0
	xor	ax,ax
;	 RET:
L$58:
;		return
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	db	0CBh	;retf
;	 ERR:
L$31:
;		ax = -1
	mov	ax,(-1)
;		goto RET
	jmp short	L$58
;	endproc

;	
;	
;	endmodule
_TEXT	ends
	end
