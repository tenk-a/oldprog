;	// ana86 L1 v0.15用ソース
;	module CkGS
;		model TINY
;		stack 0x400
;		start Main
;	
;		import Dos
;		import Param
;		import RPal
;	
;	@define TITLE "CkGS : COPY-Key Graphic Saver v0.81  by Tenk*\N"
DGROUP	group	_DATA,_BSS,_TEXT

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
bsstop	label	byte
_BSS	ends

_STACK	segment para stack 'STACK'
	db	1024 dup(?)
;	
;	//////////////////////////////////////////////////////////////////////////////
;	const PORT_CRTV = 0x64, PORT_BEL = 0x37, PORT_IMR = 0x02, PORT_GC = 0x7C
;	const _TV_ON = 0x0c, _TV_OFF = 0x0d
;	const _GV_ON = 0x40, _GV_OFF = 0x41
;	const _GET_KEY = 0x00
;	const CR =	0x0d, LF = 0x0a, BS  = 0x08, SPC = 0x20, BEL = 0x07
;	
;	const TV_SEG = 0xa000
;	const TV_CON_ADR = 0x0000, TV_ATR_ADR = 0x2000
;	const TV_CON2_ADR = 0x1000, TV_ATR2_ADR = 0x3000
;	const DFLT_VECT = 0x05
;	///////////////////////////// 常駐部 /////////////////////////////////////////
;	cvar TsrBegin:b()	// 常駐部 begin
_STACK	ends

_TEXT	segment byte public 'CODE'
CkGS@TsrBegin	label	byte
;	
;	const BUFSZ = 68
;	cvar inputBuf:b(BUFSZ+2)
CkGS@inputBuf	label	byte
	db	70 dup(?)
;	cvar origMsDosVect:d = 0
CkGS@origMsDosVect	label	dword
	db	4 dup(0)
;	cvar origCrtBiosVect:d = 0
CkGS@origCrtBiosVect	label	dword
	db	4 dup(0)
;	cvar origHardErrVect:d = 0
CkGS@origHardErrVect	label	dword
	db	4 dup(0)
;	cvar origStopKeyVect:d = 0
CkGS@origStopKeyVect	label	dword
	db	4 dup(0)
;	cvar rpalSeg:w = 0
CkGS@rpalSeg	label	word
	db	2 dup(0)
;	cvar saveMode:w = 0
CkGS@saveMode	label	word
	db	2 dup(0)
;	cvar textSwOff:b = 0, grphSwOn:b = 0, gcSw:b = 0
CkGS@textSwOff	label	byte
	db	1 dup(0)
CkGS@grphSwOn	label	byte
	db	1 dup(0)
CkGS@gcSw	label	byte
	db	1 dup(0)
;	cvar savePage:b = 0
CkGS@savePage	label	byte
	db	1 dup(0)
;	cvar maskSw:b = 0
CkGS@maskSw	label	byte
	db	1 dup(0)
;	cvar msdosSw:b = 0
CkGS@msdosSw	label	byte
	db	1 dup(0)
;	cvar msdosFlg:b = 0
CkGS@msdosFlg	label	byte
	db	1 dup(0)
;	cvar rtsPage:b = 0
CkGS@rtsPage	label	byte
	db	1 dup(0)
;	cvar textMode:b = 0
CkGS@textMode	label	byte
	db	1 dup(0)
;	cvar rpalSw:b = 0
CkGS@rpalSw	label	byte
	db	1 dup(0)
;	cvar crtBiosSw:b = 1
CkGS@crtBiosSw	label	byte
	db	1
;	cvar vectNo:b = DFLT_VECT
CkGS@vectNo	label	byte
	db	5
;	cvar crtSw:b = 1
CkGS@crtSw	label	byte
	db	1
;	
;	//---------------------------------------------------------------------------//
;	proc HardErrVect();far
;	begin
;		ax = 0
CkGS@HardErrVect	label	far
	xor	ax,ax
;		iret
	iret
;	endproc

;	
;	proc VMsDos()
;	//	引数は不定
;	begin
;		push fx
CkGS@VMsDos	label	near
	pushf
;		call origMsDosVect
	call	dword ptr cs:[CkGS@origMsDosVect]
;		ret
	ret
;	endproc

;	
;	proc CrtBios(ah)
;	macro
;		intr 0x18
;	endproc
;	
;	@if 0
;	proc PutC(al)
;	macro
;		intr 0x29
;	endproc
;	@else
;	proc PutC(al)
;	begin
;		save dx,ax
;		if (al != 0xff)
CkGS@PutC	label	near
	push	ax
	push	dx
	cmp	al,255
	je	L$1
;			ah = 0x06
	mov	ah,6
;			dl = al
	mov	dl,al
;			VMsDos
	call	CkGS@VMsDos
;		fi
L$1:
;		return
	pop	dx
	pop	ax
	ret
;	endproc

;	@fi
;	
;	@if 1
;	proc GetC_Wait()
;	macro
;		ah = _GET_KEY
;		intr 0x18
;	endproc
;	@else
;	proc GetC_Wait()
;	begin
;		save dx
;		loop
;			ah = 0x06
;			dl = 0xff
;			VMsDos
;		endloop (al == 0)
;		return
;	endproc
;	@fi
;	
;	proc Print(ax)
;	//  arg ax : 文字列へのポインタ
;	//  out ax : 出力した文字数(byte)
;	begin
;		save bx,si
;		si = ax
CkGS@Print	label	near
	push	bx
	push	si
	mov	si,ax
;		bx = 0
	xor	bx,bx
;		loop
L$2:
;			al = b[si+bx]
	mov	al,byte ptr [si+bx]
;			exit (al == '\0')
	or	al,al
	je	L$4
;			++bx
	inc	bx
;			PutC al
	call	CkGS@PutC
;		endloop
	jmp short	L$2
L$4:
;		ax = bx
	mov	ax,bx
;		return
	pop	si
	pop	bx
	ret
;	endproc

;	
;	proc Input(ax,bx)
;	//	arg ax : 入力用バッファへのポインタ
;	//  arg bx : 入力する最大文字数(byte)
;	begin
;		save ax,bx,cx,si
;		si = ax
CkGS@Input	label	near
	push	ax
	push	cx
	push	bx
	push	si
	mov	si,ax
;		cx = bx
	mov	cx,bx
;		bx = 0
	xor	bx,bx
;		if (b[si])
	cmp	byte ptr [si],0
	je	L$5
;			Print ax	//result ax
	call	CkGS@Print
;			bx = ax
	mov	bx,ax
;		fi
L$5:
;		loop
L$6:
;			GetC_Wait 	//result al
	xor	ah,ah
	int	24
;			exit (al == CR)
	cmp	al,13
	je	L$8
;			if (al == BS)
	cmp	al,8
	jne	L$9
;				if (bx)
	or	bx,bx
	je	L$10
;					--bx
	dec	bx
;					PutC al
	call	CkGS@PutC
;					al = SPC
	mov	al,32
;					PutC al
	call	CkGS@PutC
;					al = BS
	mov	al,8
;					PutC al
	call	CkGS@PutC
;					b[si+bx] = '\0'
	mov	byte ptr [si+bx],0
;				fi
L$10:
;			elsif (al >= ' ')
	jmp	L$11
L$9:
	cmp	al,32
	jb	L$12
;			    if (bx < cx)
	cmp	bx,cx
	jae	L$13
;					b[si+bx] = al
	mov	byte ptr [si+bx],al
;					++bx
	inc	bx
;					PutC al
	call	CkGS@PutC
;				fi
L$13:
;			fi
L$12:
L$11:
;		endloop
	jmp short	L$6
L$8:
;		b[si+bx] = '\0'
	mov	byte ptr [si+bx],0
;		return
	pop	si
	pop	bx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc ChgExt(ax,bx)
;	//	axで示されるファイル名の拡張子をbxで示されるものに変更
;	begin
;		save di,si,es,ax,bx
;		si = ax
CkGS@ChgExt	label	near
	push	ax
	push	bx
	push	si
	push	di
	push	es
	mov	si,ax
;		es = ax = ds
	mov	ax,ds
	mov	es,ax
;		if (b[si] == '.')
	cmp	byte ptr [si],46
	jne	L$14
;			++si
	inc	si
;		fi
L$14:
;		if (b[si] == '.')
	cmp	byte ptr [si],46
	jne	L$15
;			++si
	inc	si
;		fi
L$15:
;		loop
L$16:
;			rep.load al,si
	lodsb
;		endloop (al != '.' && al != 0)
	cmp	al,46
	je	L$19
	or	al,al
	jne	L$16
L$19:
;		--si
	dec	si
;		b[si] = 0
	mov	byte ptr [si],0
;		di = si
	mov	di,si
;		si = bx
	mov	si,bx
;		if (si)
	or	si,si
	je	L$20
;			al = '.'
	mov	al,46
;			rep.set di,al
	stosb
;			b[di] = 0
	mov	byte ptr [di],0
;			if (b[si])
	cmp	byte ptr [si],0
	je	L$21
;				loop
L$22:
;					rep.load al,si
	lodsb
;					rep.set  di,al
	stosb
;				endloop (al != 0)
	or	al,al
	jne	L$22
;			fi
L$21:
;		fi
L$20:
;		return
	pop	es
	pop	di
	pop	si
	pop	bx
	pop	ax
	ret
;	endproc

;	
;	@if 0
;	proc Bell(ah)
;	begin
;		save ax,cx
;		loop
;			port(PORT_BEL) = al = 6
;			cx = 15000;	loop; endloop (--cx)
;			port(PORT_BEL) = al = 7
;			cx = 15000;	loop; endloop (--cx)
;		endloop(--ah)
;		return
;	endproc
;	@fi
;	
;	@if 0
;	proc ChkD()
;	begin
;		save ax
;		cvar nnn:b='0'
;		al = nnn
;		PutC al
;		++al
;		if (al > '9')
;			al = '0'
;		fi
;		nnn = al
;		return
;	endproc
;	@fi
;	
;	proc ActGvPage(al)
;	macro
;		port(0xA6) = al
;	endproc
;	
;	//---------------------------------------------------------------------------//
;	proc Txt2Bak()
;	//  表テキストRAM を裏テキストRAMに転送して、表テキストをクリア
;	//	break ax,cx,dx,si,di
;	begin
;		save ds,es
;		// テキスト画面を裏テキストRAMに退避
CkGS@Txt2Bak	label	near
	push	ds
	push	es
;		ds = ax = TV_SEG
	mov	ax,40960
	mov	ds,ax
;		es = ax
	mov	es,ax
;		dx = 80*25
	mov	dx,2000
;		cx = dx
	mov	cx,dx
;		si = TV_CON_ADR
	xor	si,si
;		di = TV_CON2_ADR
	mov	di,4096
;		rep.cpy.w di,si,cx
	rep	movsw
;		cx = dx
	mov	cx,dx
;		si = TV_ATR_ADR
	mov	si,8192
;		di = TV_ATR2_ADR
	mov	di,12288
;		rep.cpy.w di,si,cx
	rep	movsw
;		//テキスト画面消去
;		ax = 0x0020
	mov	ax,32
;		cx = dx
	mov	cx,dx
;		di = TV_CON_ADR
	xor	di,di
;		rep.set.w di,ax,cx
	rep	stosw
;		ax = 0x00E1
	mov	ax,225
;		cx = dx
	mov	cx,dx
;		di = TV_ATR_ADR
	mov	di,8192
;		rep.set.w di,ax,cx
	rep	stosw
;		return
	pop	es
	pop	ds
	ret
;	endproc

;	
;	proc Bak2Txt()
;	//  裏テキストRAM を表テキストRAMに転送
;	//	break ax,cx,dx,si,di
;	begin
;		save ds,es
;	
CkGS@Bak2Txt	label	near
	push	ds
	push	es
;		ds = ax = TV_SEG
	mov	ax,40960
	mov	ds,ax
;		es = ax
	mov	es,ax
;		dx = 80*25
	mov	dx,2000
;		cx = dx
	mov	cx,dx
;		si = TV_CON2_ADR
	mov	si,4096
;		di = TV_CON_ADR
	xor	di,di
;		rep.cpy.w di,si,cx
	rep	movsw
;		cx = dx
	mov	cx,dx
;		si = TV_ATR2_ADR
	mov	si,12288
;		di = TV_ATR_ADR
	mov	di,8192
;		rep.cpy.w di,si,cx
	rep	movsw
;		return
	pop	es
	pop	ds
	ret
;	endproc

;	
;	proc WriteBuf(ax.dx,cx,bx)
;	//	arg ax.dx : バッファ
;	//  arg cx    : サイズ(byte)
;	//  arg bx    : dos のファイル・ハンドル
;	begin
;		save ax,bx,cx,dx,ds
;		ds = ax
CkGS@WriteBuf	label	near
	push	ax
	push	cx
	push	dx
	push	bx
	push	ds
	mov	ds,ax
;		ax = Dos._Write * 0x100 + 0
	mov	ax,16384
;		VMsDos
	call	CkGS@VMsDos
;		go (cf) ERR
	jc	L$25
;		go (ax != cx) ERR
	cmp	ax,cx
	jne	L$25
;		cf = 0
	clc
;	 RTS:
L$26:
;		return
	pop	ds
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	 ERR:
L$25:
;	 	cf = 1
	stc
;	 	go RTS
	jmp short	L$26
;	endproc

;	
;	proc SwapRG(ax)
;	begin
;		save ax,cx,si,ds,fx
;		ds = ax
CkGS@SwapRG	label	near
	push	ax
	push	cx
	push	si
	push	ds
	pushf
	mov	ds,ax
;		si = 0
	xor	si,si
;		cx = 16
	mov	cx,16
;		loop
L$27:
;			ax = w[si]
	mov	ax,word ptr [si]
;			al <=> ah
	xchg	al,ah
;			w[si] = ax
	mov	word ptr [si],ax
;			si += 3
	add	si,3
;		endloop (--cx)
	loop	L$27
;		return
	popf
	pop	ds
	pop	si
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc SaveGVRam(ax)
;	begin
;		save ax,bx,cx,dx
;	
CkGS@SaveGVRam	label	near
	push	ax
	push	cx
	push	dx
	push	bx
;		//push ds
;		dx = ax
	mov	dx,ax
;		ah = Dos._Create0
	mov	ah,60
;		cx = 0
	xor	cx,cx
;		VMsDos
	call	CkGS@VMsDos
;		//|pop  ds
;		go (cf) ERR
	jc	L$30
;	
;		dx = 0
	xor	dx,dx
;		cx = 80*400
	mov	cx,32000
;		bx = ax
	mov	bx,ax
;		ax = 0xA800
	mov	ax,43008
;		WriteBuf ax.dx,cx,bx
	call	CkGS@WriteBuf
;		go (cf) ERR
	jc	L$30
;		ax = 0xB000
	mov	ax,45056
;		WriteBuf ax.dx,cx,bx
	call	CkGS@WriteBuf
;		go (cf) ERR
	jc	L$30
;		ax = 0xB800
	mov	ax,47104
;		WriteBuf ax.dx,cx,bx
	call	CkGS@WriteBuf
;		go (cf) ERR
	jc	L$30
;		ax = 0xE000
	mov	ax,57344
;		WriteBuf ax.dx,cx,bx
	call	CkGS@WriteBuf
;		go (cf) ERR
	jc	L$30
;		
;		if (rpalSw && rpalSeg)
	cmp	byte ptr cs:[CkGS@rpalSw],0
	je	L$31
	cmp	word ptr cs:[CkGS@rpalSeg],0
	je	L$31
;			ax.dx = ww(rpalSeg,0x0000)
	xor	dx,dx
	mov	ax,word ptr cs:[CkGS@rpalSeg]
;			cx = 48
	mov	cx,48
;			SwapRG ax
	call	CkGS@SwapRG
;			WriteBuf ax.dx,cx,bx
	call	CkGS@WriteBuf
;			SwapRG ax
	call	CkGS@SwapRG
;		fi
L$31:
;	ERR:
L$30:
;		ah = Dos._Close
	mov	ah,62
;		VMsDos
	call	CkGS@VMsDos
;		return
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc Ckgs()
;	begin
;		save pusha,es,ds,fx
;		cvar sav_vramFlg:b, sav_crtFlg:b
L$32	label	byte
	db	1 dup(?)
L$33	label	byte
	db	1 dup(?)
;		cvar saveCurPos:b() ={"\[[s$"}		//画面保存データ
L$34	label	byte
	db	27,91,115,36
;		cvar loadCurPos:b() ={"\[[u$"}		//画面戻しデータ
L$35	label	byte
	db	27,91,117,36
;		cvar inputMsg:b() = "\[[24;1H Save Name ? "
L$36	label	byte
	db	27,91,50,52,59,49,72,32,83,97,118,101,32,78,97,109
	db	101,32,63,32
	db	1 dup(0)
;		cvar ext:b() = "frm"
L$37	label	byte
	db	102,114,109
	db	1 dup(0)
;		cvar ext2:b() = "frn"
L$38	label	byte
	db	102,114,110
	db	1 dup(0)
;	
CkGS@Ckgs	label	near
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	pushf
;		rep.inc
	cld
;		ds = ax = cs
	mov	ax,cs
	mov	ds,ax
;		es = ax = 0
	xor	ax,ax
	mov	es,ax
;		al = b[es:0x54c]	//98MSDOSの VRAM の表示状態を持つワークエリア
	mov	al,byte ptr es:[1356]
;		al &= 0x80
	and	al,128
;		sav_vramFlg = al
	mov	byte ptr cs:[L$32],al
;		sav_crtFlg = al = crtSw
	mov	al,byte ptr cs:[CkGS@crtSw]
	mov	byte ptr cs:[L$33],al
;		
;		CrtBios _GV_OFF
	mov	ah,65
	int	24
;		CrtBios _TV_ON
	mov	ah,12
	int	24
;		intr.off
	cli
;	
;		// カーソル位置を保存
;	  @if 1
;		ax = Dos._DispStr * 0x100
	mov	ax,2304
;		dx = &saveCurPos
	mov	dx,offset DGROUP:L$34
;		VMsDos
	call	CkGS@VMsDos
;	  @else
;	  	ax = &saveCurPos
;	  	Print ax
;	  @fi
;	 
;		Txt2Bak			//テキスト画面の退避
	call	CkGS@Txt2Bak
;	
;		//致命的エラーベクタ書き換え
;		ax = Dos._GetVct * 0x100 + 0x24
	mov	ax,13604
;		VMsDos
	call	CkGS@VMsDos
;		origHardErrVect = es.bx
	mov	word ptr cs:[CkGS@origHardErrVect],bx
	mov	word ptr cs:[CkGS@origHardErrVect+2],es
;		ax = Dos._SetVct * 0x100 + 0x24
	mov	ax,9508
;		dx = &HardErrVect
	mov	dx,offset DGROUP:CkGS@HardErrVect
;		VMsDos
	call	CkGS@VMsDos
;	
;		if (gcSw)
	cmp	byte ptr cs:[CkGS@gcSw],0
	je	L$39
;			port(PORT_GC) = al = 0
	xor	al,al
	out	124,al
;		fi
L$39:
;		//ファイル名入力
;		ax = &inputMsg
	mov	ax,offset DGROUP:L$36
;		Print ax
	call	CkGS@Print
;		ax = &inputBuf
	mov	ax,offset DGROUP:CkGS@inputBuf
;		bx = BUFSZ-4
	mov	bx,64
;		Input ax,bx
	call	CkGS@Input
;		bx = &ext
	mov	bx,offset DGROUP:L$37
;		ChgExt ax,bx
	call	CkGS@ChgExt
;	
;		cx = saveMode
	mov	cx,word ptr cs:[CkGS@saveMode]
;		if (cx == 0)
	or	cx,cx
	jne	L$40
;			SaveGVRam ax	//セーブ
	call	CkGS@SaveGVRam
;		elsif (--cx == 0)
	jmp	L$41
L$40:
	dec	cx
	jne	L$42
;			bx = ax
	mov	bx,ax
;			al = savePage
	mov	al,byte ptr cs:[CkGS@savePage]
;			ActGvPage al
	out	166,al
;			ax = bx
	mov	ax,bx
;			SaveGVRam ax	//セーブ
	call	CkGS@SaveGVRam
;			al = rtsPage
	mov	al,byte ptr cs:[CkGS@rtsPage]
;			ActGvPage al
	out	166,al
;		else
	jmp	L$41
L$42:
;			cx = ax
	mov	cx,ax
;			ax = 0
	xor	ax,ax
;			ActGvPage al
	out	166,al
;			ax = cx
	mov	ax,cx
;			SaveGVRam ax	//セーブ
	call	CkGS@SaveGVRam
;			al = 1
	mov	al,1
;			ActGvPage al
	out	166,al
;			ax = cx
	mov	ax,cx
;			bx = &ext2
	mov	bx,offset DGROUP:L$38
;			ChgExt ax,bx
	call	CkGS@ChgExt
;			SaveGVRam ax	//セーブ
	call	CkGS@SaveGVRam
;			al = rtsPage
	mov	al,byte ptr cs:[CkGS@rtsPage]
;			ActGvPage al
	out	166,al
;		fi
L$41:
;	
;		//致命的エラーベクタを戻す
;		push ds
	push	ds
;		ax = Dos._SetVct * 0x100 + 0x24
	mov	ax,9508
;		ds.dx = origHardErrVect
	lds	dx,dword ptr cs:[CkGS@origHardErrVect]
;		VMsDos
	call	CkGS@VMsDos
;		pop ds
	pop	ds
;	
;		Bak2Txt			//テキスト画面の復活
	call	CkGS@Bak2Txt
;	
;		// カーソル位置の復帰
;	  @if 1
;		ax = Dos._DispStr * 0x100
	mov	ax,2304
;		dx = &loadCurPos
	mov	dx,offset DGROUP:L$35
;		VMsDos
	call	CkGS@VMsDos
;	  @else
;	  	ax = &saveCurPos
;	  	Print ax
;	  @fi
;		//テキスト、グラフィック画面の状態を戻す
;		if (sav_crtFlg == 0 || textSwOff != 0)
	cmp	byte ptr cs:[L$33],0
	je	L$44
	cmp	byte ptr cs:[CkGS@textSwOff],0
	je	L$43
L$44:
;			CrtBios _TV_OFF
	mov	ah,13
	int	24
;		fi
L$43:
;		if (sav_vramFlg || grphSwOn != 0)
	cmp	byte ptr cs:[L$32],0
	jne	L$46
	cmp	byte ptr cs:[CkGS@grphSwOn],0
	je	L$45
L$46:
;			CrtBios _GV_ON
	mov	ah,64
	int	24
;		fi
L$45:
;	    inputBuf(0) = 0
	mov	byte ptr cs:[CkGS@inputBuf],0
;		return
	popf
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	
;	cvar inputFlg:b = 0
CkGS@inputFlg	label	byte
	db	1 dup(0)
;	cvar sav_ss:w,sav_sp:w
CkGS@sav_ss	label	word
	db	2 dup(?)
CkGS@sav_sp	label	word
	db	2 dup(?)
;	
;	proc CopyKeyVect();far
;	//	Copyキー・ベクタ
;	begin
;		intr.off
CkGS@CopyKeyVect	label	far
	cli
;		if (msdosSw == 2 && (inputFlg == 0 || inputFlg == 3))
	cmp	byte ptr cs:[CkGS@msdosSw],2
	jne	L$47
	cmp	byte ptr cs:[CkGS@inputFlg],0
	je	L$48
	cmp	byte ptr cs:[CkGS@inputFlg],3
	jne	L$47
L$48:
;			if (msdosFlg)
	cmp	byte ptr cs:[CkGS@msdosFlg],0
	je	L$49
;				inputFlg = 3
	mov	byte ptr cs:[CkGS@inputFlg],3
;				iret
	iret
;			fi
L$49:
;		elsif (inputFlg)	//CopyKey割り込みがネストするようなら iret
	jmp	L$50
L$47:
	cmp	byte ptr cs:[CkGS@inputFlg],0
	je	L$51
;			iret
	iret
;		elsif (msdosSw == 1)
	jmp	L$50
L$51:
	cmp	byte ptr cs:[CkGS@msdosSw],1
	jne	L$52
;			inputFlg = 2
	mov	byte ptr cs:[CkGS@inputFlg],2
;			iret
	iret
;		fi
L$52:
L$50:
;		inputFlg = 1
	mov	byte ptr cs:[CkGS@inputFlg],1
;		sav_ss = ss
	mov	word ptr cs:[CkGS@sav_ss],ss
;		sav_sp = sp
	mov	word ptr cs:[CkGS@sav_sp],sp
;		push ax
	push	ax
;		ax = cs
	mov	ax,cs
;		ax -= 0x10
	sub	ax,16
;		ss = ax
	mov	ss,ax
;		pop  ax
	pop	ax
;		sp = 0x100
	mov	sp,256
;		//
;		Ckgs
	call	CkGS@Ckgs
;		//
;		ss = sav_ss
	mov	ss,word ptr cs:[CkGS@sav_ss]
;		sp = sav_sp
	mov	sp,word ptr cs:[CkGS@sav_sp]
;		inputFlg = 0
	mov	byte ptr cs:[CkGS@inputFlg],0
;		iret
	iret
;	endproc

;	
;	proc CrtBiosVect();far
;	begin
;		if (ah == _TV_ON)
CkGS@CrtBiosVect	label	far
	cmp	ah,12
	jne	L$53
;			crtSw = 1
	mov	byte ptr cs:[CkGS@crtSw],1
;		elsif (ah == _TV_OFF)
	jmp	L$54
L$53:
	cmp	ah,13
	jne	L$55
;			crtSw = 0
	mov	byte ptr cs:[CkGS@crtSw],0
;		fi
L$55:
L$54:
;		//port(PORT_CRTV) = al
;		jmp origCrtBiosVect
	jmp	dword ptr cs:[CkGS@origCrtBiosVect]
;	endproc

;	
;	proc MsDosVect();far
;	begin
;		cvar sav_ax:w
L$56	label	word
	db	2 dup(?)
;	
CkGS@MsDosVect	label	far
;		if (maskSw && ah == 0x25 && al == vectNo)
	cmp	byte ptr cs:[CkGS@maskSw],0
	je	L$57
	cmp	ah,37
	jne	L$57
	cmp	al,byte ptr cs:[CkGS@vectNo]
	jne	L$57
;			iret
	iret
;		fi
L$57:
;		msdosFlg = 1
	mov	byte ptr cs:[CkGS@msdosFlg],1
;		if (inputFlg >= 2 && msdosSw > 0)
	cmp	byte ptr cs:[CkGS@inputFlg],2
	jb	L$58
	cmp	byte ptr cs:[CkGS@msdosSw],0
	jbe	L$58
;			intr.off
	cli
;			inputFlg = 1
	mov	byte ptr cs:[CkGS@inputFlg],1
;			sav_ss = ss
	mov	word ptr cs:[CkGS@sav_ss],ss
;			sav_sp = sp
	mov	word ptr cs:[CkGS@sav_sp],sp
;			sav_ax = ax
	mov	word ptr cs:[L$56],ax
;			ax = cs
	mov	ax,cs
;			ax -= 0x10
	sub	ax,16
;			ss = ax
	mov	ss,ax
;			ax = sav_ax
	mov	ax,word ptr cs:[L$56]
;			sp = 0x100
	mov	sp,256
;			//
;			Ckgs
	call	CkGS@Ckgs
;			//
;			ss = sav_ss
	mov	ss,word ptr cs:[CkGS@sav_ss]
;			sp = sav_sp
	mov	sp,word ptr cs:[CkGS@sav_sp]
;			inputFlg = 0
	mov	byte ptr cs:[CkGS@inputFlg],0
;		fi
L$58:
;	    if (msdosSw != 2)
	cmp	byte ptr cs:[CkGS@msdosSw],2
	je	L$59
;			msdosFlg = 0
	mov	byte ptr cs:[CkGS@msdosFlg],0
;			jmp  origMsDosVect
	jmp	dword ptr cs:[CkGS@origMsDosVect]
;	  	else
L$59:
;			//sav_ss = ss
;			//sav_sp = sp
;			//sav_ax = ax
;			//ax = cs
;			//ax -= 0x10
;			//ss = ax
;			//ax = sav_ax
;			//sp = 0x100
;	  		intr.off
	cli
;	  		push fx
	pushf
;	  		call origMsDosVect
	call	dword ptr cs:[CkGS@origMsDosVect]
;			msdosFlg = 0
	mov	byte ptr cs:[CkGS@msdosFlg],0
;			//ss = sav_ss
;			//sp = sav_sp
;			iret
	iret
;	 	fi
L$60:
;	endproc

;	
;	cvar TsrEnd:b()		// 常駐部 end
CkGS@TsrEnd	label	byte
;	
;	//////////////////////////  非常駐部 /////////////////////////////////////////
;	var  optStay:b = 0
_TEXT	ends

_DATA	segment word public 'DATA'
CkGS@optStay	label	byte
	db	1 dup(?)
;	var  debugFlg:b  = 0
CkGS@debugFlg	label	byte
	db	1 dup(?)
;	
;	proc StrNCpy(ax,bx,cx)
;	begin
;		save ax,bx,cx,si,di,es
;		di = ax
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@StrNCpy	label	near
	push	ax
	push	cx
	push	bx
	push	si
	push	di
	push	es
	mov	di,ax
;		si = bx
	mov	si,bx
;		es = ax = ds
	mov	ax,ds
	mov	es,ax
;		if (cx)
	jcxz	L$61
;			loop
L$62:
;				rep.load al,si
	lodsb
;				rep.set di,al
	stosb
;				exit(al == 0)
	or	al,al
	je	L$64
;			endloop (--cx)
	loop	L$62
L$64:
;			--di
	dec	di
;			b[di] = 0
	mov	byte ptr [di],0
;		fi
L$61:
;		return
	pop	es
	pop	di
	pop	si
	pop	bx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc HtoUI(si)
;	begin
;		save bx
;		rep.inc
CkGS@HtoUI	label	near
	push	bx
	cld
;		//loop
;		//	rep.load al,si
;		//	go (al == 0) RTS
;		//endloop (al <= ' ')
;		//--si
;		bx = 0
	xor	bx,bx
;		al = bl
	mov	al,bl
;		loop
L$65:
;			rep.load al,si
	lodsb
;			exit ((al -= '0') < 0)
	sub	al,48
	jb	L$67
;			if (al > 9)
	cmp	al,9
	jbe	L$68
;				exit ((al -= 'A' - '0') < 0)
	sub	al,17
	jb	L$67
;				if (al > 5)
	cmp	al,5
	jbe	L$69
;					al -= 'a' - 'A'
	sub	al,32
;					exit (al < 0 || al > 5)
	or	al,al
	jb	L$67
	cmp	al,5
	ja	L$67
;				fi
L$69:
;				al += 10
	add	al,10
;			fi
L$68:
;			bx <<= 4
	shl	bx,1
	shl	bx,1
	shl	bx,1
	shl	bx,1
;			bx += ax
	add	bx,ax
;		endloop
	jmp short	L$65
L$67:
;		ax = bx
	mov	ax,bx
;		return
	pop	bx
	ret
;	endproc

;	
;	proc MemEqu(es.di, ds.si, cx)
;	begin
;		loop
CkGS@MemEqu	label	near
L$70:
;			rep.cmp.w di,si
	cmpsw
;			exit (!=)
	jne	L$72
;		endloop (--cx)
	loop	L$70
L$72:
;		return
	ret
;	endproc

;	
;	proc TsrPrgErr()
;	begin
;		save bx,dx,cx,ax
;		var msg:b() = {"常駐に矛盾がある\N"}
_TEXT	ends

_DATA	segment word public 'DATA'
L$73	label	byte
	db	143,237,146,147,130,201,150,181,143,130,130,170,130,160,130,233
	db	13,10
;		Dos.Write Dos.STDOUT, &msg, sizeof(msg)
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@TsrPrgErr	label	near
	push	ax
	push	cx
	push	dx
	push	bx
	mov	cx,18
	mov	dx,offset DGROUP:L$73
	mov	bx,1
	mov	ah,64
	int	33
;		return
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc ChkStay()
;	//  out zf  常駐しているかどうか。常駐していれば、zf=1, es = そのセグメント
;	//	out es
;	begin
;		save ax,bx,cx,si,di
;		Dos.GetVct 0x18
CkGS@ChkStay	label	near
	push	ax
	push	cx
	push	bx
	push	si
	push	di
	mov	al,24
	mov	ah,53
	int	33
;		if (bx == &CrtBiosVect)
	cmp	bx,offset DGROUP:CkGS@CrtBiosVect
	jne	L$74
;			ah = b[es:%crtBiosSw]
	mov	ah,byte ptr es:[CkGS@crtBiosSw]
;			if (ah == 0)
	or	ah,ah
	jne	L$75
;				TsrPrgErr
	call	CkGS@TsrPrgErr
;			fi
L$75:
;			si = di = bx
	mov	di,bx
	mov	si,di
;			cx = 10
	mov	cx,10
;			MemEqu es.di,ds.si,cx
	call	CkGS@MemEqu
;			go (==) NOS
	je	L$76
;		fi
L$74:
;		al = vectNo
	mov	al,byte ptr cs:[CkGS@vectNo]
;		Dos.GetVct al
	mov	ah,53
	int	33
;		if (bx == &CopyKeyVect)
	cmp	bx,offset DGROUP:CkGS@CopyKeyVect
	jne	L$77
;			al = vectNo
	mov	al,byte ptr cs:[CkGS@vectNo]
;			ah = b[es:%vectNo]
	mov	ah,byte ptr es:[CkGS@vectNo]
;			cl = b[es:%crtBiosSw]
	mov	cl,byte ptr es:[CkGS@crtBiosSw]
;			if (al != ah || cl)
	cmp	al,ah
	jne	L$79
	or	cl,cl
	je	L$78
L$79:
;				TsrPrgErr
	call	CkGS@TsrPrgErr
;			fi
L$78:
;			si = di = bx
	mov	di,bx
	mov	si,di
;			cx = 10
	mov	cx,10
;			MemEqu es.di,ds.si,cx
	call	CkGS@MemEqu
;		fi
L$77:
;	  NOS:
L$76:
;		return
	pop	di
	pop	si
	pop	bx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	var msg_stay:b()	= {"常駐しています\N"}
_TEXT	ends

_DATA	segment word public 'DATA'
CkGS@msg_stay	label	byte
	db	143,237,146,147,130,181,130,196,130,162,130,220,130,183,13,10
;	var msg_notstay:b() = {"常駐していません\N"}
CkGS@msg_notstay	label	byte
	db	143,237,146,147,130,181,130,196,130,162,130,220,130,185,130,241
	db	13,10
;	
;	proc StayCkgs()
;	//  in  ds,cs
;	begin
;		save pusha,es,ds
;		var msg2:b() = {"常駐します\n"}
L$80	label	byte
	db	143,237,146,147,130,181,130,220,130,183,10
;		ChkStay
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@StayCkgs	label	near
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	call	CkGS@ChkStay
;		if (==)
	jne	L$81
;			Dos.Write Dos.STDOUT, &msg_stay, sizeof(msg_stay)
	mov	cx,16
	mov	dx,offset DGROUP:CkGS@msg_stay
	mov	bx,1
	mov	ah,64
	int	33
;			return
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;		fi
L$81:
;		// CRT Bios のベクタ変更
;		al = 0x18
	mov	al,24
;		Dos.GetVct al
	mov	ah,53
	int	33
;		origCrtBiosVect = es.bx
	mov	word ptr cs:[CkGS@origCrtBiosVect],bx
	mov	word ptr cs:[CkGS@origCrtBiosVect+2],es
;		if (crtBiosSw && debugFlg == 0)
	cmp	byte ptr cs:[CkGS@crtBiosSw],0
	je	L$82
	cmp	byte ptr [CkGS@debugFlg],0
	jne	L$82
;			dx = &CrtBiosVect
	mov	dx,offset DGROUP:CkGS@CrtBiosVect
;			Dos.SetVct al,dx
	mov	ah,37
	int	33
;		fi
L$82:
;		// vectNo のベクタ変更
;		al = vectNo
	mov	al,byte ptr cs:[CkGS@vectNo]
;		Dos.GetVct al
	mov	ah,53
	int	33
;		origStopKeyVect = es.bx
	mov	word ptr cs:[CkGS@origStopKeyVect],bx
	mov	word ptr cs:[CkGS@origStopKeyVect+2],es
;		if (debugFlg == 0)
	cmp	byte ptr [CkGS@debugFlg],0
	jne	L$83
;			dx = &CopyKeyVect
	mov	dx,offset DGROUP:CkGS@CopyKeyVect
;			Dos.SetVct al,dx
	mov	ah,37
	int	33
;		fi
L$83:
;		// dos callのベクタ変更
;		al = 0x21
	mov	al,33
;		Dos.GetVct al
	mov	ah,53
	int	33
;		origMsDosVect = es.bx
	mov	word ptr cs:[CkGS@origMsDosVect],bx
	mov	word ptr cs:[CkGS@origMsDosVect+2],es
;		if ((maskSw || msdosSw) && debugFlg == 0)
	cmp	byte ptr cs:[CkGS@maskSw],0
	jne	L$85
	cmp	byte ptr cs:[CkGS@msdosSw],0
	je	L$84
L$85:
	cmp	byte ptr [CkGS@debugFlg],0
	jne	L$84
;			dx = &MsDosVect
	mov	dx,offset DGROUP:CkGS@MsDosVect
;			Dos.SetVct al,dx
	mov	ah,37
	int	33
;		fi
L$84:
;	
;		//al = port(PORT_IMR)
;		//al &= 0xFB
;		//port(PORT_IMR) = al
;		//intr.on
;		//port(PORT_CRTV) = al
;	
;		CrtBios _TV_ON
	mov	ah,12
	int	24
;		CrtBios _GV_OFF
	mov	ah,65
	int	24
;	
;		if (debugFlg)
	cmp	byte ptr [CkGS@debugFlg],0
	je	L$86
;			return
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;		fi
L$86:
;		Dos.Write Dos.STDOUT, &msg2, sizeof(msg2)
	mov	cx,11
	mov	dx,offset DGROUP:L$80
	mov	bx,1
	mov	ah,64
	int	33
;	
;		dx = &TsrEnd
	mov	dx,offset DGROUP:CkGS@TsrEnd
;		//dx -= &TsrBegin
;		dx += 15+0x100
	add	dx,271
;		dx >>= 4
	shr	dx,1
	shr	dx,1
	shr	dx,1
	shr	dx,1
;		Dos.StayExit 0,dx
	xor	al,al
	mov	ah,49
	int	33
;	endproc

;	
;	proc RemoveCkgs()
;	begin
;		var msg2:b() = {"解放に失敗しました\n"}
_TEXT	ends

_DATA	segment word public 'DATA'
L$87	label	byte
	db	137,240,149,250,130,201,142,184,148,115,130,181,130,220,130,181
	db	130,189,10
;		var msg3:b() = {"解放しました\n"}
L$88	label	byte
	db	137,240,149,250,130,181,130,220,130,181,130,189,10
;		
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@RemoveCkgs	label	near
;		ChkStay
	call	CkGS@ChkStay
;		if (!=)
	je	L$89
;			Dos.Write Dos.STDOUT,&msg_notstay,sizeof(msg_notstay)
	mov	cx,18
	mov	dx,offset DGROUP:CkGS@msg_notstay
	mov	bx,1
	mov	ah,64
	int	33
;			return
	ret
;		fi
L$89:
;	
;		//al = port(PORT_IMR)
;		//al |= 0x04
;		//port(PORT_IMR) = al
;		//intr.on
;	
;		if (b[es:%msdosSw] || b[es:%maskSw])
	cmp	byte ptr es:[CkGS@msdosSw],0
	jne	L$91
	cmp	byte ptr es:[CkGS@maskSw],0
	je	L$90
L$91:
;			ds.dx = d[es:%origMsDosVect]
	lds	dx,dword ptr es:[CkGS@origMsDosVect]
;			Dos.SetVct 0x21,dx
	mov	al,33
	mov	ah,37
	int	33
;		fi
L$90:
;		if (b[es:%crtBiosSw])
	cmp	byte ptr es:[CkGS@crtBiosSw],0
	je	L$92
;			ds.dx = d[es:%origCrtBiosVect]
	lds	dx,dword ptr es:[CkGS@origCrtBiosVect]
;			Dos.SetVct 0x18,dx
	mov	al,24
	mov	ah,37
	int	33
;		fi
L$92:
;		ds.dx = d[es:%origStopKeyVect]
	lds	dx,dword ptr es:[CkGS@origStopKeyVect]
;		Dos.SetVct b[es:%vectNo],dx
	mov	al,byte ptr es:[CkGS@vectNo]
	mov	ah,37
	int	33
;	
;		ds = ax = cs
	mov	ax,cs
	mov	ds,ax
;		ax = es
	mov	ax,es
;		ax -= 0x10
	sub	ax,16
;		es = ax
	mov	es,ax
;		Dos.MemFree es
	mov	ah,73
	int	33
;		if (cf)
	jnc	L$93
;			dx = &msg2
	mov	dx,offset DGROUP:L$87
;			cx = sizeof (msg2)
	mov	cx,19
;		else
	jmp	L$94
L$93:
;			dx = &msg3
	mov	dx,offset DGROUP:L$88
;			cx = sizeof (msg3)
	mov	cx,13
;		fi
L$94:
;	
;		Dos.Write Dos.STDOUT,dx,cx
	mov	bx,1
	mov	ah,64
	int	33
;		return
	ret
;	endproc

;	
;	proc DispCkgs()
;	begin
;		save pusha,es,ds
;		cvar msg:b() = {"\N-? でヘルプを表示します\N"}
L$95	label	byte
	db	13,10,45,63,32,130,197,131,119,131,139,131,118,130,240,149
	db	92,142,166,130,181,130,220,130,183,13,10
;	
CkGS@DispCkgs	label	near
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	ds
	push	es
;		ChkStay
	call	CkGS@ChkStay
;		if (!=)
	je	L$96
;			Dos.Write Dos.STDOUT, &msg_notstay, sizeof(msg_notstay)
	mov	cx,18
	mov	dx,offset DGROUP:CkGS@msg_notstay
	mov	bx,1
	mov	ah,64
	int	33
;		else
	jmp	L$97
L$96:
;			Dos.Write Dos.STDOUT, &msg_stay, sizeof(msg_stay)
	mov	cx,16
	mov	dx,offset DGROUP:CkGS@msg_stay
	mov	bx,1
	mov	ah,64
	int	33
;		fi
L$97:
;		Dos.Write Dos.STDOUT, &msg, sizeof(msg)
	mov	cx,27
	mov	dx,offset DGROUP:L$95
	mov	bx,1
	mov	ah,64
	int	33
;		return
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc Usage()
;	begin
;		var msg:b() = {
_TEXT	ends

_DATA	segment word public 'DATA'
L$98	label	byte
;			"usage:CkGS [-opts] file\N"\
;			"  -v[N] int05hをフックして常駐. 05h以外にするにはN(0～FF)を指定\N"\
;			"  -r[N] 解放. -l- 指定時は常駐時の N を指定する必要がある\N"\
;			"  -aN   画面を強制的にページ N(0or1)にしてセーブ\N"\
;			"  -bN   セーブ後の画面を強制的にページ N(0or1)にする\N"\
;			"  -d    両画面をセーブ(0:frm 1:frn). セーブ後はヘージ 0\N"\
;			"  -g[-] セーブ後強制的に graphic on にする/ -c- off\N"\
;			"  -t[-] セーブ後強制的に text on にする/ -t- off\N"\
;			"  -c    セーブ前に GC をオフ\N"\
;			"  -m    Set Intrrupt Vectorで-vNのベクタを書換えられないようint21hを乗っ取る\N"\
;			"  -e    MSDOSと排他的に処理を行なう\N"\
;			"  -l-   テキストon/offの監視のための int18hの乗っ取りをやめる\N"\
;			"  -p    常駐パレットの内容も一緒にセーブ\N"\
;			"  file  セーブ名入力時の初期文字列\N"\
;		    "COPY-Key (int 05h)でセーブするファイル名を入力する\N"\
;		    "もちろん -v で割り込みのベクタを変更するとCOPY-Key以外になる\N"\
;			"常駐判定をint18hのアドレスで行なっている. が、-l-を指定すると無理になるので\N"\
;			"int05hのアドレスを用いるが05h変更のときは, 解除は-rで Nを指定する必要がある.\N"\
;			"セーブ後不都合が生じることがおおいので、セーブ後はリセットしたほうがよい.\N"\
;			"割り込みのタイミングによっては致命的な破壊も有り得るので覚悟のこと.\N"\
;			"-e を指定すると少し安全になるが、セーブできないばあいが多くなる.\N"\
;			""
;		}
	db	117,115,97,103,101,58,67,107,71,83,32,91,45,111,112,116
	db	115,93,32,102,105,108,101,13,10,32,32,45,118,91,78,93
	db	32,105,110,116,48,53,104,130,240,131,116,131,98,131,78,130
	db	181,130,196,143,237,146,147,46,32,48,53,104,136,200,138,79
	db	130,201,130,183,130,233,130,201,130,205,78,40,48,129,96,70
	db	70,41,130,240,142,119,146,232,13,10,32,32,45,114,91,78
	db	93,32,137,240,149,250,46,32,45,108,45,32,142,119,146,232
	db	142,158,130,205,143,237,146,147,142,158,130,204,32,78,32,130
	db	240,142,119,146,232,130,183,130,233,149,75,151,118,130,170,130
	db	160,130,233,13,10,32,32,45,97,78,32,32,32,137,230,150
	db	202,130,240,139,173,144,167,147,73,130,201,131,121,129,91,131
	db	87,32,78,40,48,111,114,49,41,130,201,130,181,130,196,131
	db	90,129,91,131,117,13,10,32,32,45,98,78,32,32,32,131
	db	90,129,91,131,117,140,227,130,204,137,230,150,202,130,240,139
	db	173,144,167,147,73,130,201,131,121,129,91,131,87,32,78,40
	db	48,111,114,49,41,130,201,130,183,130,233,13,10,32,32,45
	db	100,32,32,32,32,151,188,137,230,150,202,130,240,131,90,129
	db	91,131,117,40,48,58,102,114,109,32,49,58,102,114,110,41
	db	46,32,131,90,129,91,131,117,140,227,130,205,131,119,129,91
	db	131,87,32,48,13,10,32,32,45,103,91,45,93,32,131,90
	db	129,91,131,117,140,227,139,173,144,167,147,73,130,201,32,103
	db	114,97,112,104,105,99,32,111,110,32,130,201,130,183,130,233
	db	47,32,45,99,45,32,111,102,102,13,10,32,32,45,116,91
	db	45,93,32,131,90,129,91,131,117,140,227,139,173,144,167,147
	db	73,130,201,32,116,101,120,116,32,111,110,32,130,201,130,183
	db	130,233,47,32,45,116,45,32,111,102,102,13,10,32,32,45
	db	99,32,32,32,32,131,90,129,91,131,117,145,79,130,201,32
	db	71,67,32,130,240,131,73,131,116,13,10,32,32,45,109,32
	db	32,32,32,83,101,116,32,73,110,116,114,114,117,112,116,32
	db	86,101,99,116,111,114,130,197,45,118,78,130,204,131,120,131
	db	78,131,94,130,240,143,145,138,183,130,166,130,231,130,234,130
	db	200,130,162,130,230,130,164,105,110,116,50,49,104,130,240,143
	db	230,130,193,142,230,130,233,13,10,32,32,45,101,32,32,32
	db	32,77,83,68,79,83,130,198,148,114,145,188,147,73,130,201
	db	143,136,151,157,130,240,141,115,130,200,130,164,13,10,32,32
	db	45,108,45,32,32,32,131,101,131,76,131,88,131,103,111,110
	db	47,111,102,102,130,204,138,196,142,139,130,204,130,189,130,223
	db	130,204,32,105,110,116,49,56,104,130,204,143,230,130,193,142
	db	230,130,232,130,240,130,226,130,223,130,233,13,10,32,32,45
	db	112,32,32,32,32,143,237,146,147,131,112,131,140,131,98,131
	db	103,130,204,147,224,151,101,130,224,136,234,143,143,130,201,131
	db	90,129,91,131,117,13,10,32,32,102,105,108,101,32,32,131
	db	90,129,91,131,117,150,188,147,252,151,205,142,158,130,204,143
	db	137,138,250,149,182,142,154,151,241,13,10,67,79,80,89,45
	db	75,101,121,32,40,105,110,116,32,48,53,104,41,130,197,131
	db	90,129,91,131,117,130,183,130,233,131,116,131,64,131,67,131
	db	139,150,188,130,240,147,252,151,205,130,183,130,233,13,10,130
	db	224,130,191,130,235,130,241,32,45,118,32,130,197,138,132,130
	db	232,141,158,130,221,130,204,131,120,131,78,131,94,130,240,149
	db	207,141,88,130,183,130,233,130,198,67,79,80,89,45,75,101
	db	121,136,200,138,79,130,201,130,200,130,233,13,10,143,237,146
	db	147,148,187,146,232,130,240,105,110,116,49,56,104,130,204,131
	db	65,131,104,131,140,131,88,130,197,141,115,130,200,130,193,130
	db	196,130,162,130,233,46,32,130,170,129,65,45,108,45,130,240
	db	142,119,146,232,130,183,130,233,130,198,150,179,151,157,130,201
	db	130,200,130,233,130,204,130,197,13,10,105,110,116,48,53,104
	db	130,204,131,65,131,104,131,140,131,88,130,240,151,112,130,162
	db	130,233,130,170,48,53,104,149,207,141,88,130,204,130,198,130
	db	171,130,205,44,32,137,240,143,156,130,205,45,114,130,197,32
	db	78,130,240,142,119,146,232,130,183,130,233,149,75,151,118,130
	db	170,130,160,130,233,46,13,10,131,90,129,91,131,117,140,227
	db	149,115,147,115,141,135,130,170,144,182,130,182,130,233,130,177
	db	130,198,130,170,130,168,130,168,130,162,130,204,130,197,129,65
	db	131,90,129,91,131,117,140,227,130,205,131,138,131,90,131,98
	db	131,103,130,181,130,189,130,217,130,164,130,170,130,230,130,162
	db	46,13,10,138,132,130,232,141,158,130,221,130,204,131,94,131
	db	67,131,126,131,147,131,79,130,201,130,230,130,193,130,196,130
	db	205,146,118,150,189,147,73,130,200,148,106,137,243,130,224,151
	db	76,130,232,147,190,130,233,130,204,130,197,138,111,140,229,130
	db	204,130,177,130,198,46,13,10,45,101,32,130,240,142,119,146
	db	232,130,183,130,233,130,198,143,173,130,181,136,192,145,83,130
	db	201,130,200,130,233,130,170,129,65,131,90,129,91,131,117,130
	db	197,130,171,130,200,130,162,130,206,130,160,130,162,130,170,145
	db	189,130,173,130,200,130,233,46,13,10
;		Dos.Write Dos.STDOUT, &msg, sizeof(msg)
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@Usage	label	near
	mov	cx,1178
	mov	dx,offset DGROUP:L$98
	mov	bx,1
	mov	ah,64
	int	33
;		Dos.EXIT(0)
	mov	ax,19456
	int	33
;	endproc

;	
;	proc OptsErr(ax)
;	begin
;		var msg:b() = {"知らないオプションだよ\N"}
_TEXT	ends

_DATA	segment word public 'DATA'
L$99	label	byte
	db	146,109,130,231,130,200,130,162,131,73,131,118,131,86,131,135
	db	131,147,130,190,130,230,13,10
;		Dos.Write Dos.STDOUT, &msg, sizeof(msg)
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@OptsErr	label	near
	mov	cx,24
	mov	dx,offset DGROUP:L$99
	mov	bx,1
	mov	ah,64
	int	33
;		Dos.EXIT(1)
	mov	ax,19457
	int	33
;	endproc

;	
;	proc NotStayRPalErr()
;	begin
;		var msg:b() = {"常駐パレットが見つかりません\N"}
_TEXT	ends

_DATA	segment word public 'DATA'
L$100	label	byte
	db	143,237,146,147,131,112,131,140,131,98,131,103,130,170,140,169
	db	130,194,130,169,130,232,130,220,130,185,130,241,13,10
;		Dos.Write Dos.STDOUT, &msg, sizeof(msg)
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@NotStayRPalErr	label	near
	mov	cx,30
	mov	dx,offset DGROUP:L$100
	mov	bx,1
	mov	ah,64
	int	33
;		Dos.EXIT(1)
	mov	ax,19457
	int	33
;	endproc

;	
;	proc OptsRngErr(ax)
;	begin
;		var msg:b() = {"オプションの指定がおかしい\N"}
_TEXT	ends

_DATA	segment word public 'DATA'
L$101	label	byte
	db	131,73,131,118,131,86,131,135,131,147,130,204,142,119,146,232
	db	130,170,130,168,130,169,130,181,130,162,13,10
;	
_DATA	ends

_TEXT	segment byte public 'CODE'
CkGS@OptsRngErr	label	near
;		Dos.Write Dos.STDOUT, &msg, sizeof(msg)
	mov	cx,28
	mov	dx,offset DGROUP:L$101
	mov	bx,1
	mov	ah,64
	int	33
;		Dos.EXIT(1)
	mov	ax,19457
	int	33
;	endproc

;	
;	proc Option(di)
;	begin
;		save ax,si
;	
CkGS@Option	label	near
	push	ax
	push	si
;		si = di
	mov	si,di
;		ah = 0
	xor	ah,ah
;		al = b[si];++si
	mov	al,byte ptr [si]
	inc	si
;		if (al >= 'a' && al <= 'z')
	cmp	al,97
	jb	L$102
	cmp	al,122
	ja	L$102
;			al -= 0x20
	sub	al,32
;		fi
L$102:
;		if (al == 'R')
	cmp	al,82
	jne	L$103
;			optStay = -1
	mov	byte ptr [CkGS@optStay],(-1)
;			go JJJ
	jmp short	L$104
;		elsif (al == 'V')
L$103:
	cmp	al,86
	jne	L$106
;			optStay = 1
	mov	byte ptr [CkGS@optStay],1
;		  JJJ:
L$104:
;			ax = DFLT_VECT
	mov	ax,5
;			if (b[si])
	cmp	byte ptr [si],0
	je	L$107
;				HtoUI	si		//result ax
	call	CkGS@HtoUI
;				if (ah)
	or	ah,ah
	je	L$108
;					ax = 'v';	OptsRngErr ax
	mov	ax,118
	call	CkGS@OptsRngErr
;				fi
L$108:
;			fi
L$107:
;			vectNo = al
	mov	byte ptr cs:[CkGS@vectNo],al
;		elsif (al == 'M')
	jmp	L$105
L$106:
	cmp	al,77
	jne	L$109
;			maskSw = 1
	mov	byte ptr cs:[CkGS@maskSw],1
;		elsif (al == 'E')
	jmp	L$105
L$109:
	cmp	al,69
	jne	L$110
;			ah = b[si]
	mov	ah,byte ptr [si]
;			if (ah == 0)
	or	ah,ah
	jne	L$111
;				ah = '1'
	mov	ah,49
;			elsif (ah == '-')
	jmp	L$112
L$111:
	cmp	ah,45
	jne	L$113
;				ah = '0'
	mov	ah,48
;			fi
L$113:
L$112:
;			ah -= '0'
	sub	ah,48
;			if (ah >= 0 && ah <= 2)
	or	ah,ah
	jb	L$114
	cmp	ah,2
	ja	L$114
;				msdosSw = ah;
	mov	byte ptr cs:[CkGS@msdosSw],ah
;			else
	jmp	L$115
L$114:
;				OptsRngErr ax
	call	CkGS@OptsRngErr
;			fi
L$115:
;		elsif (al == 'D')
	jmp	L$105
L$110:
	cmp	al,68
	jne	L$116
;			saveMode = 2
	mov	word ptr cs:[CkGS@saveMode],2
;			rtsPage = 0
	mov	byte ptr cs:[CkGS@rtsPage],0
;		elsif (al == 'A')
	jmp	L$105
L$116:
	cmp	al,65
	jne	L$117
;			saveMode = 1
	mov	word ptr cs:[CkGS@saveMode],1
;			HtoUI si
	call	CkGS@HtoUI
;			if (ax > 1)
	cmp	ax,1
	jbe	L$118
;				ax = 'a';	OptsRngErr ax
	mov	ax,97
	call	CkGS@OptsRngErr
;			fi
L$118:
;			savePage = al
	mov	byte ptr cs:[CkGS@savePage],al
;		elsif (al == 'B')
	jmp	L$105
L$117:
	cmp	al,66
	jne	L$119
;			HtoUI si
	call	CkGS@HtoUI
;			if (ax > 1)
	cmp	ax,1
	jbe	L$120
;				ax = 'b';	OptsRngErr ax
	mov	ax,98
	call	CkGS@OptsRngErr
;			fi
L$120:
;			rtsPage = al
	mov	byte ptr cs:[CkGS@rtsPage],al
;		elsif (al == 'T')
	jmp	L$105
L$119:
	cmp	al,84
	jne	L$121
;			al = 0
	xor	al,al
;			if (b[si] == '-')
	cmp	byte ptr [si],45
	jne	L$122
;				al = 1
	mov	al,1
;			fi
L$122:
;			textSwOff = al
	mov	byte ptr cs:[CkGS@textSwOff],al
;		elsif (al == 'G')
	jmp	L$105
L$121:
	cmp	al,71
	jne	L$123
;			al = 1
	mov	al,1
;			if (b[si] == '-')
	cmp	byte ptr [si],45
	jne	L$124
;				al = 0
	xor	al,al
;			fi
L$124:
;			grphSwOn = al
	mov	byte ptr cs:[CkGS@grphSwOn],al
;		elsif (al == 'L')
	jmp	L$105
L$123:
	cmp	al,76
	jne	L$125
;			al = 1
	mov	al,1
;			if (b[si] == '-')
	cmp	byte ptr [si],45
	jne	L$126
;				al = 0
	xor	al,al
;			fi
L$126:
;			crtBiosSw = al
	mov	byte ptr cs:[CkGS@crtBiosSw],al
;		elsif (al == 'P')
	jmp	L$105
L$125:
	cmp	al,80
	jne	L$127
;			rpalSw = 1
	mov	byte ptr cs:[CkGS@rpalSw],1
;		elsif (al == 'Z')
	jmp	L$105
L$127:
	cmp	al,90
	jne	L$128
;			debugFlg = 1
	mov	byte ptr [CkGS@debugFlg],1
;		elsif (al == 'C')
	jmp	L$105
L$128:
	cmp	al,67
	jne	L$129
;			gcSw = 1
	mov	byte ptr cs:[CkGS@gcSw],1
;		elsif (al == '\0' || al == '?')
	jmp	L$105
L$129:
	or	al,al
	je	L$131
	cmp	al,63
	jne	L$130
L$131:
;			Usage
	call	CkGS@Usage
;		else
	jmp	L$105
L$130:
;			OptsErr ax
	call	CkGS@OptsErr
;		fi
L$105:
;		return
	pop	si
	pop	ax
	ret
;	endproc

;	
;	*proc Main()
;	//	起動時に呼ばれる一番目の手続き
;	enter
;		var title:b() = {TITLE}
_TEXT	ends

_DATA	segment word public 'DATA'
L$132	label	byte
	db	67,107,71,83,32,58,32,67,79,80,89,45,75,101,121,32
	db	71,114,97,112,104,105,99,32,83,97,118,101,114,32,118,48
	db	46,56,49,32,32,98,121,32,84,101,110,107,42,13,10
;	
_DATA	ends

_TEXT	segment byte public 'CODE'
	public	CkGS@Main
CkGS@Main	label	near
	push	bp
	mov	bp,sp
;		rep.inc
	cld
;		ds = ax = cs
	mov	ax,cs
	mov	ds,ax
;	
;		CrtBios _TV_ON
	mov	ah,12
	int	24
;		Dos.Write Dos.STDOUT, &title, sizeof(title)
	mov	cx,47
	mov	dx,offset DGROUP:L$132
	mov	bx,1
	mov	ah,64
	int	33
;	
;		Param.Init es
	call	Param@Init
;		bx = 0
	xor	bx,bx
;		loop
L$133:
;			Param.Get
	call	Param@Get
;			exit(di == 0)
	or	di,di
	je	L$135
;			if (b[di] == '-')
	cmp	byte ptr [di],45
	jne	L$136
;				++di
	inc	di
;				Option di
	call	CkGS@Option
;			else
	jmp	L$137
L$136:
;				bx = di
	mov	bx,di
;			fi
L$137:
;		endloop
	jmp short	L$133
L$135:
;		inputBuf(0) = '\0'
	mov	byte ptr cs:[CkGS@inputBuf],0
;		if (bx)
	or	bx,bx
	je	L$138
;			ax = &inputBuf
	mov	ax,offset DGROUP:CkGS@inputBuf
;			cx = BUFSZ
	mov	cx,68
;			StrNCpy ax,bx,cx
	call	CkGS@StrNCpy
;		fi
L$138:
;		// 環境変数領域の解放
;		es = w[es:0x2c]			//PSP:0x2C  環境変数のセグメントを格納
	mov	es,word ptr es:[44]
;		Dos.MemFree es
	mov	ah,73
	int	33
;	
;		if (debugFlg)
	cmp	byte ptr [CkGS@debugFlg],0
	je	L$139
;			StayCkgs
	call	CkGS@StayCkgs
;			CopyKeyVect
	call far ptr	CkGS@CopyKeyVect
;		elsif (optStay .>. 0)
	jmp	L$140
L$139:
	cmp	byte ptr [CkGS@optStay],0
	jle	L$141
;			if (rpalSw)
	cmp	byte ptr cs:[CkGS@rpalSw],0
	je	L$142
;				RPal.Search
	call	_RPalSearch
;				if (dx)
	or	dx,dx
	je	L$143
;					++dx
	inc	dx
;					rpalSeg = dx
	mov	word ptr cs:[CkGS@rpalSeg],dx
;				else
	jmp	L$144
L$143:
;					NotStayRPalErr
	call	CkGS@NotStayRPalErr
;				fi
L$144:
;			fi
L$142:
;			StayCkgs
	call	CkGS@StayCkgs
;		elsif (optStay .<. 0)
	jmp	L$140
L$141:
	cmp	byte ptr [CkGS@optStay],0
	jge	L$145
;			RemoveCkgs
	call	CkGS@RemoveCkgs
;		else
	jmp	L$140
L$145:
;			DispCkgs
	call	CkGS@DispCkgs
;		fi
L$140:
;		Dos.EXIT(0)
	mov	ax,19456
	int	33
;	endproc

;	
;	endmodule
_TEXT	ends
extrn _RPalSearch:near
extrn Param@Get:near
extrn Param@Init:near
	end	CkGS@Main
