;	/*
;		Pi 展開
;			writen by M.Kitamura(Tenk*)
;			1993-1995
;	 */
;	module PI
;		c_name "PI","_",1
;		import Std
;		import Dos
;		//import Pri
;	
;	//@define DEBUG() 1
;	
;	///////////////////////////////////////////////////////////////////////////////
;	*const COMMENT_SIZE = 2048+512-2
	.186
DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
;	*const READBUFFER_SIZE = 1024*36
;	*const PIXBUF_SIZE = 1024*24
;	*const PIXLINSIZ = 4096/*1280*2+2*/ /*PIXBUF_SIZE / 4 */
;	
;	@define PIT()	PI.T[ds:0]
;	
;	*struct T
;		//dummy:b(16)		// セグメント境界調整用
;		/*-------------*/
;		pln:w				// 色のビット数(4 or 8)
;		xsize:w				// 横幅(ドット)
;		ysize:w				// 縦幅(ドット)
;		xstart:w			// 始点x
;		ystart:w			// 始点y
;		aspect1:w			// アスペクト比x
;		aspect2:w			// アスペクト比y
;		overlayColor:w		// 透明色
;		color:w				// パレットの使用個数
;		palBit:w			// パレットのビット数
;		exSize:w			// pi拡張領域サイズ
;		dfltPalFlg:w		// piデフォルトパレットフラグ
;		loopMode:w			// piループモード(pimkの実験/隠しTAG)
;		filHdl:w			// ファイル・ハンドル
;		saverName:b(4+2)	// 機種名
;		artist:b(18+2)		// 作者名
;		/*-------------*/
;		putLin:d			// 出力手続きのアドレス
;		putLinDt:d			// 出力手続きへわたすデータ
;		pixStart:w			// ピクセルバッファ中の解凍開始位置
;		pixEnd:w			//                         終了位置
;		pixHisSrc:w			// バッファ更新でヒストリバッファへのコピー元
;		pixHisSiz:w			// ヒストリ・サイズ
;		ymax:w				// 展開する最大行数
;		/*-------------*/
;		readPtr:w			// 読み込みバッファ中のポインタ
;		ofs:w				// 比較点への相対位置
;		ofs1:w
;		ofs2:w
;		ofs3:w
;		//ofs4:w
;		readColor:w			// 色データ展開ルーチンへのポインタ
;		pixBlkCnt:w			// 展開バッファ関係
;		pixBlkLin:w
;		pixEndLin:w
;		/*-------------*/
;		dummy2:b( 256 /* -16*/ - (14*2+6+20) - (2*4+5*2) - (9*2) )
;		/*-------------*/
;		palette:b(256*3)
;		fileName:b(254+2)
;		comment:b(COMMENT_SIZE+2)
;		/*--------------------------------------------------------*/
;		colorTable:b(16*16)
;		/*------------------------- ここまでで 4Kバイト ---*/
;		pixBuf:b(PIXBUF_SIZE)
;		readBuffer:b(READBUFFER_SIZE)
;	endstruct
;	
;	*const RDBUF_END = %PI.T.readBuffer + READBUFFER_SIZE
;	
;	
;	///////////////////////////////////////////////////////////////////////////////
;	cvar sav_sp:w
_BSS	ends

_TEXT	segment byte public 'CODE'
_PI_sav_sp	label	word
	db	2 dup(?)
;	
;	proc Exit(ax)
;		far
;		// Pi_Load,Pi_Open 終了
;		// dx.ax 復帰コード
;		//	0:正常
;		//	1:読み込みエラー
;		//	2:データを読み込みすぎた
;		//	3:横幅が 2以下または 大きすぎる
;		//	4:PRGERR:256色画の読み込みなのに色表バッファが確保されてない
;	begin
;		intr.off
_PI_Exit	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	cli
;		sp = sav_sp
	mov	sp,word ptr cs:[_PI_sav_sp]
;		intr.on
	sti
;	
;		dx = 0
	xor	dx,dx
;	
;		pop bp
	pop	bp
;		pop es
	pop	es
;		pop ds
	pop	ds
;		pop di
	pop	di
;		pop si
	pop	si
;		pop bx
	pop	bx
;		pop cx
	pop	cx
;		return
	db	0CBh	;retf
;	endproc

;	
;	proc ReadBuf(si)
;	//	ファイルよりデータを読み込む
;	//	in	ds
;	//	out si
;	begin
;		save ax,bx,cx,dx
;	
;		si = %PI.T.readBuffer
_PI_ReadBuf	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	cx
	push	dx
	push	bx
	mov	si,07000h
;		PIT.readPtr = si
	mov	word ptr ds:[048h],si
;		Dos.Read PIT.filHdl, si, PI.READBUFFER_SIZE
	mov	cx,09000h
	mov	dx,si
	mov	bx,word ptr ds:[01Ah]
	mov	ah,03Fh
	int	021h
;		if (cf)
	jnc	L$1
;			ax = 1
	mov	ax,01h
;			jmp Exit
	jmp far ptr	_PI_Exit
;		fi
L$1:
;		if (ax == 0)
	or	ax,ax
	jne	L$2
;			//Pri.Fmt "読み込みでエラーが発生したよお(T^T)\n"
;			//Dos.Exit 1
;			ax = 2
	mov	ax,02h
;			jmp Exit
	jmp far ptr	_PI_Exit
;		fi
L$2:
;		return
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc ReadB(si)
;	begin
;		if (si == PI.RDBUF_END-1)
_PI_ReadB	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	cmp	si,0FFFFh
	jne	L$3
;			ReadBuf si
	call	_PI_ReadBuf
;		fi
L$3:
;		rep.load al,si
	lodsb
;		return
	ret
;	endproc

;	
;	proc ReadW_sub(si)
;		// out si,ax
;	begin
;		if (si == PI.RDBUF_END-2)
_PI_ReadW_sub	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	cmp	si,0FFFEh
	jne	L$4
;			ax = w[si]
	mov	ax,word ptr [si]
;			ah <=> al
	xchg	ah,al
;			ReadBuf si
	call	_PI_ReadBuf
;		else
	jmp	L$5
L$4:
;			ah = b[si]
	mov	ah,byte ptr [si]
;			ReadBuf si
	call	_PI_ReadBuf
;			al = b[si]
	mov	al,byte ptr [si]
;			++si
	inc	si
;		fi
L$5:
;		return
	ret
;	endproc

;	
;	proc ReadW(si)
;	begin
;		if (si < PI.RDBUF_END-2);
_PI_ReadW	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	cmp	si,0FFFEh
	jae	L$6
;			rep.load.w ax,si;
	lodsw
;		else;
	jmp	L$7
L$6:
;			ReadW_sub si;
	call	_PI_ReadW_sub
;		fi;
L$7:
;		ah<=>al
	xchg	ah,al
;		return
	ret
;	endproc

;	
;	///////////////////////////////////////////////////////////////////////////////
;	cvar userLinFlg:w = 0	//コメントの１行目をユーザ名としてあつかうかのフラグ
_PI_userLinFlg	label	word
	db	2 dup(0)
;	cvar exAreaFlg:w = 0
_PI_exAreaFlg	label	word
	db	2 dup(0)
;	cvar apicgFlg:w = 1
_PI_apicgFlg	label	word
	dw	01h
;	
;	cvar c256bufSeg:w = 0
_PI_c256bufSeg	label	word
	db	2 dup(0)
;	
;	*proc SetExAreaFlg (flg:w)
;		cdecl
;		//far
;	enter
;		save ax
;		exAreaFlg = ax = flg
	public	_PI_SetExAreaFlg
_PI_SetExAreaFlg	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	bp
	mov	bp,sp
	mov	ax,word ptr [bp+6]
	mov	word ptr cs:[_PI_exAreaFlg],ax
;		return
	db	0C9h	;leave
	pop	ax
	ret
;	endproc

;	
;	*proc SetUserLineFlg (flg:w)
;		cdecl
;		//far
;	enter
;		save ax
;		userLinFlg = ax = flg
	public	_PI_SetUserLineFlg
_PI_SetUserLineFlg	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	bp
	mov	bp,sp
	mov	ax,word ptr [bp+6]
	mov	word ptr cs:[_PI_userLinFlg],ax
;		return
	db	0C9h	;leave
	pop	ax
	ret
;	endproc

;	
;	*proc SetApicgFlg(flg:w)
;		cdecl
;		//far
;	enter
;		save ax
;		apicgFlg = ax = flg
	public	_PI_SetApicgFlg
_PI_SetApicgFlg	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	bp
	mov	bp,sp
	mov	ax,word ptr [bp+6]
	mov	word ptr cs:[_PI_apicgFlg],ax
;		return
	db	0C9h	;leave
	pop	ax
	ret
;	endproc

;	
;	@if 0
;	*proc SetC256buf(bufSeg:w)
;		cdecl
;		far
;	 /* 256色展開時に必要な64Kﾊﾞｲﾄ色表のﾒﾓﾘを指定.
;		ﾒﾓﾘは事前に確保すること. PI_Load を実行する前に設定すること.
;		ret ax:error no
;	  */
;	enter
;		save ax
;		c256bufSeg = ax = bufSeg
;		return
;	endproc
;	@endif
;	
;	proc StrCpy(es.di,si)
;	//	arg es.di	コピー先
;	//	arg si		コピー元
;	begin
;		save ax,cx,si,di,es
;	
;		push es,di
_PI_StrCpy	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	cx
	push	si
	push	di
	push	es
	push	di
	push	es
;		es = cx = ds
	mov	cx,ds
	mov	es,cx
;		di = si
	mov	di,si
;		cx = 0xffff
	mov	cx,0FFFFh
;		al = 0
	xor	al,al
;		repn.scan di,al,cx
	repne	scasb
;		++cx
	inc	cx
;		com cx
	not	cx
;		++cx
	inc	cx
;		pop es,di
	pop	es
	pop	di
;	
;		rep.cpy   di,si,cx
	rep	movsb
;		//--di
;		return
	pop	es
	pop	di
	pop	si
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc GetUserName()
;	//	in	ds,es
;	enter
;		save pusha
;		cvar user:b() = "User:"
L$8	label	byte
	db	85,115,101,114,58
	db	1 dup(0)
;	
;		di = %PI.T.comment
_PI_GetUserName	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	db	60h	;pusha
	push	bp
	mov	bp,sp
	mov	di,0500h
;		go.w (b[di] == '\0') RET
	cmp	byte ptr [di],00h
	jne	L$10
	jmp	L$9
L$10:
;		//go (userLinFlg) J1
;		push ds
	push	ds
;			ds = cs
	push	cs
	pop	ds
;			rep.cmp di,&user,5
	mov	cx,05h
	mov	si,offset _TEXT:L$8
	rep	cmpsb
;		|pop ds
	pop	ds
;		if (==)
	jne	L$11
;	  J1:
L$12:
;			si = di
	mov	si,di
;			di = %PI.T.artist
	mov	di,022h
;			cx = 0
	xor	cx,cx
;			loop
L$13:
;				rep.load al,si
	lodsb
;				exit(al == '\0'|| al == 0x1a|| al == '\r'|| al == '\n')
	or	al,al
	je	L$15
	cmp	al,01Ah
	je	L$15
	cmp	al,0Dh
	je	L$15
	cmp	al,0Ah
	je	L$15
;				next (cx >= 31)
	cmp	cx,01Fh
	jae	L$14
;				rep.set  di,al
	stosb
;				++cx
	inc	cx
;			endloop
L$14:
	jmp short	L$13
L$15:
;			if (al == '\r' && b[si] == '\n')
	cmp	al,0Dh
	jne	L$16
	cmp	byte ptr [si],0Ah
	jne	L$16
;				++si
	inc	si
;			fi
L$16:
;			if (al == '\0')
	or	al,al
	jne	L$17
;				--si
	dec	si
;			fi
L$17:
;			rep.set di,'\0'
	xor	al,al
	stosb
;			di = %PI.T.comment
	mov	di,0500h
;			
;			StrCpy es.di, si
	call	_PI_StrCpy
;		fi
L$11:
;	 RET:
L$9:
;		return
	db	0C9h	;leave
	db	61h	;popa
	ret
;	endproc

;	
;	proc GetExData(si,cx)
;		// in ds,es
;		// break ax,bx,cx,dx,di
;	begin
;		loop.w (cx >= 5)
_PI_GetExData	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	jmp	L$19
L$18:
;			--cx
	dec	cx
;			ReadB si
	call	_PI_ReadB
;			if (al == 0x01) /* 始点 */
	cmp	al,01h
	jne	L$21
;				ReadW si;	PIT.xstart = ax
	call	_PI_ReadW
	mov	word ptr ds:[06h],ax
;				ReadW si;	PIT.ystart = ax
	call	_PI_ReadW
	mov	word ptr ds:[08h],ax
;				cx -= 4
	sub	cx,04h
;			elsif (al == 0x02)	/* 透明色 */
	jmp	L$22
L$21:
	cmp	al,02h
	jne	L$23
;				ReadW si
	call	_PI_ReadW
;				ReadW si
	call	_PI_ReadW
;				++ax
	inc	ax
;				PIT.overlayColor = ax
	mov	word ptr ds:[0Eh],ax
;				cx -= 4
	sub	cx,04h
;			elsif (al == 0x03)	/* パレットの有効ビット数 */
	jmp	L$22
L$23:
	cmp	al,03h
	jne	L$24
;				ReadW si
	call	_PI_ReadW
;				ReadW si
	call	_PI_ReadW
;				PIT.palBit = ax
	mov	word ptr ds:[012h],ax
;				cx -= 4
	sub	cx,04h
;			elsif (al == 0x04)	/* パレットの使用個数 */
	jmp	L$22
L$24:
	cmp	al,04h
	jne	L$25
;				ReadW si
	call	_PI_ReadW
;				ReadW si
	call	_PI_ReadW
;				PIT.color = ax
	mov	word ptr ds:[010h],ax
;				cx -= 4
	sub	cx,04h
;			elsif (al <= 0x1f)
	jmp	L$22
L$25:
	cmp	al,01Fh
	ja	L$26
;				ReadW si
	call	_PI_ReadW
;				ReadW si
	call	_PI_ReadW
;				cx -= 4
	sub	cx,04h
;			elsif (al == 'A' && apicgFlg)	//APICG(X68K)でセーブされた PI対策
	jmp	L$22
L$26:
	cmp	al,041h
	jne	L$27
	cmp	word ptr cs:[_PI_apicgFlg],00h
	je	L$27
;				di = %PI.T.artist
	mov	di,022h
;				dx = 0
	xor	dx,dx
;				loop
L$28:
;					ReadB si
	call	_PI_ReadB
;					--cx
	dec	cx
;					exit (al == 0)
	or	al,al
	je	L$30
;					rep.set di,al
	stosb
;					++dx
	inc	dx
;				endloop (dx<18 && cx)
	cmp	dx,012h
	jae	L$31
	or	cx,cx
	jne	L$28
L$31:
L$30:
;				rep.set di,0
	xor	al,al
	stosb
;			elsif (Std.IsAlpha(al))
	jmp	L$22
L$27:
	cmp	al,041h
	jb	L$34
	cmp	al,05Ah
	jbe	L$33
L$34:
	cmp	al,061h
	jb	L$32
	cmp	al,07Ah
	ja	L$32
L$33:
;				exit (cx < 5)
	cmp	cx,05h
	jb	L$20
;				bh = al
	mov	bh,al
;				ReadB si	//TAG 2
	call	_PI_ReadB
;				bl = al
	mov	bl,al
;				ReadW si	//TAG 3,4
	call	_PI_ReadW
;				dx = ax
	mov	dx,ax
;				ReadW si	//ax=ｻｲｽﾞ
	call	_PI_ReadW
;				cx -= 5
	sub	cx,05h
;				exit (cx == 0 || ax > cx)
	or	cx,cx
	je	L$20
	cmp	ax,cx
	ja	L$20
;				cx -= ax
	sub	cx,ax
;				if (ax == 1 && bx == 'l'*0x100+'o' && dx == 'o'*0x100+'p')
	cmp	ax,01h
	jne	L$35
	cmp	bx,06C6Fh
	jne	L$35
	cmp	dx,06F70h
	jne	L$35
;					//ループ画像
;					dx.ax = d[ds:%PI.T.saverName]
	mov	ax,word ptr ds:[01Ch]
	mov	dx,word ptr ds:[01Eh]
;					if (ax == 'M' + 'G'*0x100 && dx == ' '+' '*0x100)
	cmp	ax,0474Dh
	jne	L$36
	cmp	dx,02020h
	jne	L$36
;						ReadB si
	call	_PI_ReadB
;						PIT.loopMode = ax
	mov	word ptr ds:[018h],ax
;					fi
L$36:
;				else
	jmp	L$37
L$35:
;					if (ax)
	or	ax,ax
	je	L$38
;						dx = ax
	mov	dx,ax
;						loop
L$39:
;							ReadB si
	call	_PI_ReadB
;						endloop (--dx)
	dec	dx
	jne	L$39
;					fi
L$38:
;				fi
L$37:
;			else
	jmp	L$22
L$32:
;				exit
	jmp short	L$20
;			fi
L$22:
;		endloop
L$19:
	cmp	cx,05h
	jb	L$42
	jmp	L$18
L$42:
L$20:
;		return
	ret
;	endproc

;	
;	proc GetPal(al,si,di)
;	begin
;		save ax,bx,cx,dx
;		cvar dfltPalRGB:b(16*3) = data	// 省略時の16色パレット
L$43	label	byte
;			b	   0,	0,	 0
	db	00h
	db	00h
	db	00h
;			b	   0,	0,0x70
	db	00h
	db	00h
	db	070h
;			b	0x70,	0,	 0
	db	070h
	db	00h
	db	00h
;			b	0x70,	0,0x70
	db	070h
	db	00h
	db	070h
;			b	   0,0x70,	 0
	db	00h
	db	070h
	db	00h
;			b	   0,0x70,0x70
	db	00h
	db	070h
	db	070h
;			b	0x70,0x70,	 0
	db	070h
	db	070h
	db	00h
;			b	0x70,0x70,0x70
	db	070h
	db	070h
	db	070h
;			b	   0,	0,	 0
	db	00h
	db	00h
	db	00h
;			b	   0,	0,0xf0
	db	00h
	db	00h
	db	0F0h
;			b	0xf0,	0,	 0
	db	0F0h
	db	00h
	db	00h
;			b	0xf0,	0,0xf0
	db	0F0h
	db	00h
	db	0F0h
;			b	   0,0xf0,	 0
	db	00h
	db	0F0h
	db	00h
;			b	   0,0xf0,0xf0
	db	00h
	db	0F0h
	db	0F0h
;			b	0xf0,0xf0,	 0
	db	0F0h
	db	0F0h
	db	00h
;			b	0xf0,0xf0,0xf0
	db	0F0h
	db	0F0h
	db	0F0h
;		enddata
;	
;		di = %PI.T.palette
_PI_GetPal	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	cx
	push	dx
	push	bx
	mov	di,0100h
;		if (PIT.pln != 8)
	cmp	word ptr ds:[00h],08h
	je	L$44
;			if (al)
	or	al,al
	je	L$45
;				push ds,si
	push	si
	push	ds
;					ds = cs
	push	cs
	pop	ds
;					si = &dfltPalRGB
	mov	si,offset _TEXT:L$43
;					rep.cpy.w di,si,48/2
	mov	cx,018h
	rep	movsw
;				pop  ds,si
	pop	ds
	pop	si
;			else
	jmp	L$46
L$45:
;				ah = 16*3
	mov	ah,030h
;				loop
L$47:
;					ReadB si
	call	_PI_ReadB
;					rep.set di,al
	stosb
;				endloop (--ah)
	dec	ah
	jne	L$47
;			fi
L$46:
;			push ds
	push	ds
;				ds = cs
	push	cs
	pop	ds
;				rep.set.w di,0,(256*3-16*3)/2
	mov	cx,0168h
	xor	ax,ax
	rep	stosw
;			pop  ds
	pop	ds
;		else
	jmp	L$50
L$44:
;			if (al)
	or	al,al
	je	L$51
;				push si
	push	si
;				dh = 0
	xor	dh,dh
;				loop	//G
L$52:
;					dl = 0
	xor	dl,dl
;					loop	//R
L$55:
;						ah = 0
	xor	ah,ah
;						loop	//B
L$58:
;							al = dl; if (al); al |= 0x1f; fi	//R
	mov	al,dl
	or	al,al
	je	L$61
	or	al,01Fh
L$61:
;							rep.set di,al
	stosb
;							al = dh; if (al); al |= 0x1f; fi	//G
	mov	al,dh
	or	al,al
	je	L$62
	or	al,01Fh
L$62:
;							rep.set di,al
	stosb
;							al = ah; if (al); al |= 0x3f; fi	//B
	mov	al,ah
	or	al,al
	je	L$63
	or	al,03Fh
L$63:
;							rep.set di,al
	stosb
;							ah += 64
	add	ah,040h
;						endloop (ah)
	or	ah,ah
	jne	L$58
;						dl += 32
	add	dl,020h
;					endloop (dl)
	or	dl,dl
	jne	L$55
;					dh += 32
	add	dh,020h
;				endloop(dh)
	or	dh,dh
	jne	L$52
;				pop si
	pop	si
;			else
	jmp	L$64
L$51:
;				cx = 256
	mov	cx,0100h
;				loop
L$65:
;					ReadB si
	call	_PI_ReadB
;					rep.set di,al
	stosb
;					ReadB si
	call	_PI_ReadB
;					rep.set di,al
	stosb
;					ReadB si
	call	_PI_ReadB
;					rep.set di,al
	stosb
;				endloop (--cx)
	loop	L$65
;			fi
L$64:
;		fi
L$50:
;		return
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	*proc Open(piPtr:d, fileName:d);cdecl ;far
;		//fileNameをopen して、ヘッダを読み込む. piPtr で使用するメモリを指定.
;		//ﾒﾓﾘは呼び出し側で事前に確保しておくこと.
;		//必要ﾒﾓﾘは64Kﾊﾞｲﾄ.
;		//256色画のﾛｰﾄﾞではさらに64Kﾊﾞｲﾄ確保してPI_SetC256bufで設定する必要がある
;		//ret dx.ax:構造体へのポインタ(ｵﾌｾｯﾄ0) ｾｸﾞﾒﾝﾄが0ならｴﾗｰ(ｵﾌｾｯﾄ値がｴﾗｰNo)
;	enter
;		save bx,cx,si,di,es,ds
;	
;		// 設定
;		rep.inc
	public	_PI_Open
_PI_Open	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	push	bp
	mov	bp,sp
	cld
;		sav_sp = sp
	mov	word ptr cs:[_PI_sav_sp],sp
;		
;		bx.ax = piPtr
	mov	ax,word ptr [bp+18]
	mov	bx,word ptr [bp+20]
;		piPtr = 0
	mov	word ptr [bp+18],00h
	mov	word ptr [bp+20],00h
;		Std.FpToHp bx,ax,cx
	mov	cx,ax
	shr	cx,04h
	add	bx,cx
	and	ax,0Fh
;		go.w (bx == 0 && ax == 0) RTS
	or	bx,bx
	jne	L$69
	or	ax,ax
	jne	L$69
	jmp	L$68
L$69:
;		if (ax)
	or	ax,ax
	je	L$70
;			++bx
	inc	bx
;		fi
L$70:
;		ax = 0
	xor	ax,ax
;		piPtr = bx.ax
	mov	word ptr [bp+18],ax
	mov	word ptr [bp+20],bx
;		es = bx
	mov	es,bx
;		//ds = bx
;	
;		rep.set.w 0, 0x0000, %PI.T.dummy2
	mov	cx,05Ah
	xor	ax,ax
	xor	di,di
	rep	stosw
;	
;		// ファイル名コピー
;		ds.si = fileName
	lds	si,dword ptr [bp+22]
;		di = %PI.T.fileName
	mov	di,0400h
;		StrCpy es.di, si
	call	_PI_StrCpy
;	//Pri.Fmt "%Fs:%Fs\n", fileName, ww(es,%PI.T.fileName)
;		//ファイル・オープン
;		ds = bx
	mov	ds,bx
;		Dos.Open %PI.T.fileName, 0x00
	xor	al,al
	mov	dx,0400h
	mov	ah,03Dh
	int	021h
;		go.w (cf) ERR
	jnc	L$72
	jmp	L$71
L$72:
;		PIT.filHdl = ax
	mov	word ptr ds:[01Ah],ax
;	
;		//バッファ読み込み
;		ReadBuf si
	call	_PI_ReadBuf
;	
;		// 'Pi'チェック
;		ReadW si
	call	_PI_ReadW
;	 //Pri.Fmt "AX=%x\n", ax
;		go.w (ax != 'P'*0x100 + 'i') HDR_ERR
	cmp	ax,05069h
	je	L$74
	jmp	L$73
L$74:
;	
;		// コメント取得
;		cx = PI.COMMENT_SIZE
	mov	cx,09FEh
;		di = %PI.T.comment
	mov	di,0500h
;		loop
L$75:
;			ReadB si
	call	_PI_ReadB
;			exit (al == 0x1a)
	cmp	al,01Ah
	je	L$77
;			next (cx == 0)
	jcxz	L$76
;			next (al == '\0')
	or	al,al
	je	L$76
;			rep.set di,al
	stosb
;			--cx
	dec	cx
;		endloop
L$76:
	jmp short	L$75
L$77:
;		//if (cx == 0)
;		//	Pri.Fmt "コメントが長すぎたので途中で切りました\N"
;		//fi
;		b[di] = 0
	mov	byte ptr [di],00h
;		//Pri.Fmt "comment:%Fs\N", ww(ds, %PI.T.comment)
;	
;		//コメントがあるとき、作者名があるかどうか調べる
;		GetUserName
	call	_PI_GetUserName
;	
;		// ダミーコメントをスキップ
;		loop
L$78:
;			ReadB si
	call	_PI_ReadB
;		endloop (al)
	or	al,al
	jne	L$78
;	
;		//---- 各種パラメータ取得 ----
;		// パレット・モード
;		ReadB si
	call	_PI_ReadB
;		if ((al &= 0x80) != 0)
	and	al,080h
	je	L$81
;			PIT.dfltPalFlg.l = al
	mov	byte ptr ds:[016h],al
;		fi
L$81:
;	
;		//アスペクト比を拾得
;		ah = 0
	xor	ah,ah
;		ReadB si;  PIT.aspect2 = ax
	call	_PI_ReadB
	mov	word ptr ds:[0Ch],ax
;		ReadB si;  PIT.aspect1 = ax
	call	_PI_ReadB
	mov	word ptr ds:[0Ah],ax
;		if (PIT.aspect2 == 0 || PIT.aspect1 == 0)
	cmp	word ptr ds:[0Ch],00h
	je	L$83
	cmp	word ptr ds:[0Ah],00h
	jne	L$82
L$83:
;			PIT.aspect1 = 1
	mov	word ptr ds:[0Ah],01h
;			PIT.aspect2 = 1
	mov	word ptr ds:[0Ch],01h
;		fi
L$82:
;	
;		//色数を取得
;		ReadB si;  PIT.pln = ax
	call	_PI_ReadB
	mov	word ptr ds:[00h],ax
;		PIT.color = 16
	mov	word ptr ds:[010h],010h
;		if (al == 8)
	cmp	al,08h
	jne	L$84
;			PIT.color = 256
	mov	word ptr ds:[010h],0100h
;		else
	jmp	L$85
L$84:
;			go.w (al != 4) VAL_ERR
	cmp	al,04h
	je	L$87
	jmp	L$86
L$87:
;		fi
L$85:
;	
;		// セーバ機種名を取得
;		ReadB si;  PIT.saverName(0) = al
	call	_PI_ReadB
	mov	byte ptr ds:[01Ch],al
;		ReadB si;  PIT.saverName(1) = al
	call	_PI_ReadB
	mov	byte ptr ds:[01Dh],al
;		ReadB si;  PIT.saverName(2) = al
	call	_PI_ReadB
	mov	byte ptr ds:[01Eh],al
;		ReadB si;  PIT.saverName(3) = al
	call	_PI_ReadB
	mov	byte ptr ds:[01Fh],al
;		PIT.saverName(4) = 0
	mov	byte ptr ds:[020h],00h
;	
;		// 機種依存データの処理
;		ReadW si
	call	_PI_ReadW
;		cx = ax
	mov	cx,ax
;		PIT.exSize = ax
	mov	word ptr ds:[014h],ax
;		PIT.xstart = 0
	mov	word ptr ds:[06h],00h
;		PIT.ystart = 0
	mov	word ptr ds:[08h],00h
;		if.w (exAreaFlg==0)//拡張領域のデータを取得
	cmp	word ptr cs:[_PI_exAreaFlg],00h
	je	L$89
	jmp	L$88
L$89:
;			GetExData si,cx
	call	_PI_GetExData
;		fi
L$88:
;		if (cx) // 残りの機種依存データを読み飛ばす
	jcxz	L$90
;			loop
L$91:
;				ReadB si
	call	_PI_ReadB
;			endloop (--cx)
	loop	L$91
;		fi
L$90:
;	
;		// 画像サイズ(X*Y)を拾得
;		ReadW si;	PIT.xsize = ax; go.w (ax == 0) VAL_ERR
	call	_PI_ReadW
	mov	word ptr ds:[02h],ax
	or	ax,ax
	jne	L$94
	jmp	L$86
L$94:
;		ReadW si;	PIT.ysize = ax; go (ax == 0) VAL_ERR
	call	_PI_ReadW
	mov	word ptr ds:[04h],ax
	or	ax,ax
	je	L$86
;	
;		// パレットを拾得
;		GetPal PIT.dfltPalFlg.l, si, di
	mov	al,byte ptr ds:[016h]
	call	_PI_GetPal
;	
;		//終了
;		PIT.readPtr = si	//read pointer を保存
	mov	word ptr ds:[048h],si
;	
;		// エラー無し
;	 //Pri.Fmt "comment:%Fs\N", ww(ds, %PI.T.comment)
;		dx.ax = piPtr
	mov	ax,word ptr [bp+18]
	mov	dx,word ptr [bp+20]
;	RTS:
L$68:
;		return
	db	0C9h	;leave
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	cx
	db	0CBh	;retf
;	
;	ERR:		//ファイル・オープンできなかった
L$71:
;		dx.ax = 1
	mov	ax,01h
	xor	dx,dx
;		go RTS
	jmp short	L$68
;	
;	HDR_ERR:	//Pri.Fmt "Piのヘッダでない\N"
L$73:
;		dx.ax = 2
	mov	ax,02h
	xor	dx,dx
;		go RTS
	jmp short	L$68
;	
;	VAL_ERR:	//Pri.Fmt "ヘッダに矛盾がある\N"
L$86:
;		dx.ax = 3
	mov	ax,03h
	xor	dx,dx
;		go RTS
	jmp short	L$68
;	endproc

;	
;	*proc Close(piPtr:d)
;		cdecl
;		far
;	 /* PI ﾌｧｲﾙのclose
;		piPtr の解放はユーザが行なうこと
;		ret ax:error no
;	  */
;	enter
;		save bx,ax,dx,ds
;		rep.inc
	public	_PI_Close
_PI_Close	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	dx
	push	bx
	push	ds
	push	bp
	mov	bp,sp
	cld
;		bx.ax = piPtr
	mov	ax,word ptr [bp+14]
	mov	bx,word ptr [bp+16]
;		piPtr = 0
	mov	word ptr [bp+14],00h
	mov	word ptr [bp+16],00h
;		Std.FpToHp bx,ax,dx
	mov	dx,ax
	shr	dx,04h
	add	bx,dx
	and	ax,0Fh
;		go.w (bx == 0 && ax == 0) RTS
	or	bx,bx
	jne	L$96
	or	ax,ax
	jne	L$96
	jmp	L$95
L$96:
;		if (ax)
	or	ax,ax
	je	L$97
;			++bx
	inc	bx
;		fi
L$97:
;		ds = bx
	mov	ds,bx
;		bx = PIT.filHdl
	mov	bx,word ptr ds:[01Ah]
;		Dos.Close bx
	mov	ah,03Eh
	int	021h
;	  RTS:
L$95:
;		return
	db	0C9h	;leave
	pop	ds
	pop	bx
	pop	dx
	pop	ax
	db	0CBh	;retf
;	endproc

;	
;	///////////////////////////////////////////////////////////////////////////////
;	
;	proc RdBit_ReadBuf(bx)
;		// out si,dx
;	begin
;		save si
;		ReadBuf bx
_PI_RdBit_ReadBuf	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	si
	mov	si,bx
	call	_PI_ReadBuf
;		bx = si
	mov	bx,si
;		return
	pop	si
	ret
;	endproc

;	
;	
;	@define RdBit(NO)				\
;		go (--dl == 0) NO@@_RDB;	\
;	NO@@_RDBRET:;					\
;		|dh <<= 1;
;	
;	@define RDB(NO) 	NO@@_RDB:;						\
;		dl = 8; 										\
;		dh = b[bx]; 									\
;		++bx;											\
;		@if (PI.RDBUF_END&0xFFFF);						\
;			go(bx != (PI.RDBUF_END&0xFFFF)) NO@@_RDBRET;\
;		@else;											\
;			go(!=) NO@@_RDBRET; 						\
;		@fi;											\
;		call RdBit_ReadBuf; 							\
;		go NO@@_RDBRET;
;	
;	//-----------------------------------------------------------------------------
;	
;	*proc InitColTbl16()
;	//	色表(16色*16通り) の初期化
;	//	in	rep.inc,ds,es
;	begin
;		save pusha
;	
;		ax = 0
	public	_PI_InitColTbl16
_PI_InitColTbl16	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	db	60h	;pusha
	xor	ax,ax
;		di = %PI.T.colorTable
	mov	di,0F00h
;		bx = 16
	mov	bx,010h
;		loop
L$98:
;			cx = 16
	mov	cx,010h
;			loop
L$101:
;				al &= 0x0f
	and	al,0Fh
;				rep.set di,al
	stosb
;				--al
	dec	al
;			endloop (--cx)
	loop	L$101
;			++al
	inc	al
;		endloop (--bx)
	dec	bx
	jne	L$98
;		return
	db	61h	;popa
	ret
;	endproc

;	
;	
;	@if defined(DEBUG)
;	proc Deb_Cn(cx)
;	begin
;		Pri.Fmt "Cn %d\N",cx
;		return
;	endproc
;	@fi
;	
;	proc ReadColor16(al)
;	//	色データを１つ読み込む
;	//	arg al = 1ﾋﾟｸｾﾙ(２ﾄﾞｯﾄ)
;	//	in	ds,es,bx,dx
;	//	out ax	,bx,dx
;	//	break cx,si
;	begin
;		bp = di
_PI_ReadColor16	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	bp,di
;	
;		ah = 0
	xor	ah,ah
;		al <<= 4
	shl	al,04h
;		si = ax
	mov	si,ax
;		si += %PI.T.colorTable
	add	si,0F00h
;	
;		RdBit(C1)
	dec	dl
	je	L$104
L$105:
	shl	dh,01h
;		go (cf) CP0or1
	jc	L$106
;		cx = 1
	mov	cx,01h
;		RdBit(C2)
	dec	dl
	je	L$107
L$108:
	shl	dh,01h
;		if (cf)
	jnc	L$109
;			RdBit(C3)
	dec	dl
	je	L$110
L$111:
	shl	dh,01h
;			if (cf)
	jnc	L$112
;				RdBit(C4);rcl cx
	dec	dl
	je	L$113
L$114:
	shl	dh,01h
	rcl	cx,01h
;			fi
L$112:
;			RdBit(C5);rcl cx
	dec	dl
	je	L$115
L$116:
	shl	dh,01h
	rcl	cx,01h
;		fi
L$109:
;		RdBit(C6);rcl cx
	dec	dl
	je	L$117
L$118:
	shl	dh,01h
	rcl	cx,01h
;		@if defined(DEBUG);Deb_Cn cx;@fi
;		si += cx
	add	si,cx
;		di = si
	mov	di,si
;		--si
	dec	si
;		al = b[di]
	mov	al,byte ptr [di]
;		rep.dec
	std
;		rep.cpy di,si,cx
	rep	movsb
;		rep.set di,al
	stosb
;		rep.inc
	cld
;		go NEXTCOLOR
	jmp short	L$119
;		RDB(C1)
L$104:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$105
	call	_PI_RdBit_ReadBuf
	jmp short	L$105
;		RDB(C2)
L$107:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$108
	call	_PI_RdBit_ReadBuf
	jmp short	L$108
;		RDB(C3)
L$110:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$111
	call	_PI_RdBit_ReadBuf
	jmp short	L$111
;		RDB(C4)
L$113:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$114
	call	_PI_RdBit_ReadBuf
	jmp short	L$114
;		RDB(C5)
L$115:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$116
	call	_PI_RdBit_ReadBuf
	jmp short	L$116
;	  CP0or1:
L$106:
;		RdBit(C7)	//in bx,dx	out cf
	dec	dl
	je	L$120
L$121:
	shl	dh,01h
;		if (cf)
	jnc	L$122
;			rep.load.w ax,si
	lodsw
;			@if defined(DEBUG);Pri.Fmt "Cn 1\N";@fi
;			ah <=> al
	xchg	ah,al
;			w[si-2] = ax
	mov	word ptr [si-02h],ax
;			go NEXTCOLOR
	jmp short	L$119
;			RDB(C6)
L$117:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$118
	call	_PI_RdBit_ReadBuf
	jmp short	L$118
;			RDB(C7)
L$120:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$121
	call	_PI_RdBit_ReadBuf
	jmp short	L$121
;		fi
L$122:
;		@if defined(DEBUG);Pri.Fmt "Cn 0\N";@fi
;		rep.load al,si
	lodsb
;		//go NEXTCOLOR
;	
;	NEXTCOLOR:
L$119:
;		ah = 0
	xor	ah,ah
;		si = ax
	mov	si,ax
;		si <<= 4
	shl	si,04h
;		si += %PI.T.colorTable
	add	si,0F00h
;	
;		RdBit(D1)
	dec	dl
	je	L$123
L$124:
	shl	dh,01h
;		go (cf) D0or1
	jc	L$125
;		ah = al
	mov	ah,al
;		cx = 1
	mov	cx,01h
;		RdBit(D2)
	dec	dl
	je	L$126
L$127:
	shl	dh,01h
;		if (cf)
	jnc	L$128
;			RdBit(D3)
	dec	dl
	je	L$129
L$130:
	shl	dh,01h
;			if (cf)
	jnc	L$131
;				RdBit(D4);rcl cx
	dec	dl
	je	L$132
L$133:
	shl	dh,01h
	rcl	cx,01h
;			fi
L$131:
;			RdBit(D5);rcl cx
	dec	dl
	je	L$134
L$135:
	shl	dh,01h
	rcl	cx,01h
;		fi
L$128:
;		RdBit(D6);rcl cx
	dec	dl
	je	L$136
L$137:
	shl	dh,01h
	rcl	cx,01h
;		@if defined(DEBUG);Deb_Cn cx;@fi
;		si += cx
	add	si,cx
;		di = si
	mov	di,si
;		--si
	dec	si
;		al = b[di]
	mov	al,byte ptr [di]
;		rep.dec
	std
;		rep.cpy di,si,cx
	rep	movsb
;		rep.set di,al
	stosb
;		rep.inc
	cld
;		al <=> ah
	xchg	al,ah
;		di = bp
	mov	di,bp
;		return
	ret
;	
;		RDB(D1)
L$123:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$124
	call	_PI_RdBit_ReadBuf
	jmp short	L$124
;		RDB(D2)
L$126:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$127
	call	_PI_RdBit_ReadBuf
	jmp short	L$127
;		RDB(D3)
L$129:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$130
	call	_PI_RdBit_ReadBuf
	jmp short	L$130
;		RDB(D4)
L$132:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$133
	call	_PI_RdBit_ReadBuf
	jmp short	L$133
;	  D0or1:
L$125:
;		RdBit(D7)	//in bx,dx	out cf
	dec	dl
	je	L$138
L$139:
	shl	dh,01h
;		if (cf)
	jnc	L$140
;			cl = al
	mov	cl,al
;			rep.load.w ax,si
	lodsw
;			ah <=> al
	xchg	ah,al
;			w[si-2] = ax
	mov	word ptr [si-02h],ax
;			ah = al
	mov	ah,al
;			al = cl
	mov	al,cl
;			di = bp
	mov	di,bp
;			@if defined(DEBUG);Pri.Fmt "Cn 1\N";@fi
;			return
	ret
;			RDB(D5)
L$134:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$135
	call	_PI_RdBit_ReadBuf
	jmp short	L$135
;			RDB(D6)
L$136:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$137
	call	_PI_RdBit_ReadBuf
	jmp short	L$137
;			RDB(D7)
L$138:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$139
	call	_PI_RdBit_ReadBuf
	jmp short	L$139
;		fi
L$140:
;		ah = al
	mov	ah,al
;		rep.load al,si
	lodsb
;		al <=> ah
	xchg	al,ah
;		di = bp
	mov	di,bp
;		@if defined(DEBUG);Pri.Fmt "Cn 0\N";@fi
;		return
	ret
;	endproc

;	
;	
;	*proc InitColTbl256()
;	//	色表(256色*256通り) の初期化
;	//	in	rep.inc
;	begin
;		save pusha,es
;	
;		es = c256bufSeg
	public	_PI_InitColTbl256
_PI_InitColTbl256	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	db	60h	;pusha
	push	es
	mov	es,word ptr cs:[_PI_c256bufSeg]
;		di = ax = 0
	xor	ax,ax
	mov	di,ax
;		bx = 256
	mov	bx,0100h
;		loop
L$141:
;			cx = 256
	mov	cx,0100h
;			loop
L$144:
;				rep.set di,al
	stosb
;				--al
	dec	al
;			endloop (--cx)
	loop	L$144
;			++al
	inc	al
;		endloop (--bx)
	dec	bx
	jne	L$141
;		return
	pop	es
	db	61h	;popa
	ret
;	endproc

;	
;	
;	proc ReadColor256(al)
;	//	色データを１つ読み込む
;	//	arg al = 1ﾋﾟｸｾﾙ(２ﾄﾞｯﾄ)
;	//	in	ds,es,bx,dx
;	//	out ax	,bx,dx
;	//	break cx,si,bp
;	begin
;		save es,di
;		es = bp = c256bufSeg
_PI_ReadColor256	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	di
	push	es
	mov	bp,word ptr cs:[_PI_c256bufSeg]
	mov	es,bp
;		push ds
	push	ds
;	
;		ah = al
	mov	ah,al
;		al = 0
	xor	al,al
;		si = ax
	mov	si,ax
;	
;		RdBit(C0)
	dec	dl
	je	L$147
L$148:
	shl	dh,01h
;		if (cf)
	jnc	L$149
;			RdBit(Ce)	//in bx,dx	out cf
	dec	dl
	je	L$150
L$151:
	shl	dh,01h
;			ds = bp
	mov	ds,bp
;			if (cf)
	jnc	L$152
;				rep.load.w ax,si
	lodsw
;				ah <=> al
	xchg	ah,al
;				w[si-2] = ax
	mov	word ptr [si-02h],ax
;				go.w NEXTCOLOR
	jmp	L$153
;				RDB(Ce)
L$150:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$151
	call	_PI_RdBit_ReadBuf
	jmp short	L$151
;			fi
L$152:
;			rep.load al,si
	lodsb
;			go.w NEXTCOLOR
	jmp	L$153
;		////
;			RDB(C0)
L$147:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$148
	call	_PI_RdBit_ReadBuf
	jmp short	L$148
;			RDB(C1)
L$154:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$155
	call	_PI_RdBit_ReadBuf
	jmp short	L$155
;			RDB(C2)
L$156:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$157
	call	_PI_RdBit_ReadBuf
	jmp short	L$157
;			RDB(C3)
L$158:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$159
	call	_PI_RdBit_ReadBuf
	jmp short	L$159
;			RDB(C4)
L$160:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$161
	call	_PI_RdBit_ReadBuf
	jmp short	L$161
;			RDB(C5)
L$162:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$163
	call	_PI_RdBit_ReadBuf
	jmp short	L$163
;		fi
L$149:
;		cx = 1
	mov	cx,01h
;		RdBit(C1)
	dec	dl
	je	L$154
L$155:
	shl	dh,01h
;		if (cf)
	jnc	L$164
;			RdBit(C2)
	dec	dl
	je	L$156
L$157:
	shl	dh,01h
;			if (cf)
	jnc	L$165
;				RdBit(C3)
	dec	dl
	je	L$158
L$159:
	shl	dh,01h
;				if (cf)
	jnc	L$166
;					RdBit(C4)
	dec	dl
	je	L$160
L$161:
	shl	dh,01h
;					if (cf)
	jnc	L$167
;						RdBit(C5)
	dec	dl
	je	L$162
L$163:
	shl	dh,01h
;						if (cf)
	jnc	L$168
;							RdBit(C6)
	dec	dl
	je	L$169
L$170:
	shl	dh,01h
;							if (cf)
	jnc	L$171
;								RdBit(C7);rcl cx
	dec	dl
	je	L$172
L$173:
	shl	dh,01h
	rcl	cx,01h
;								//go J1
;							fi
L$171:
;						  //J1:
;							RdBit(C8);rcl cx
	dec	dl
	je	L$174
L$175:
	shl	dh,01h
	rcl	cx,01h
;						fi
L$168:
;						RdBit(C9);rcl cx
	dec	dl
	je	L$176
L$177:
	shl	dh,01h
	rcl	cx,01h
;					fi
L$167:
;					RdBit(Ca);rcl cx
	dec	dl
	je	L$178
L$179:
	shl	dh,01h
	rcl	cx,01h
;				fi
L$166:
;				RdBit(Cb);rcl cx
	dec	dl
	je	L$180
L$181:
	shl	dh,01h
	rcl	cx,01h
;			fi
L$165:
;			RdBit(Cc);rcl cx
	dec	dl
	je	L$182
L$183:
	shl	dh,01h
	rcl	cx,01h
;		fi
L$164:
;		RdBit(Cd);rcl cx
	dec	dl
	je	L$184
L$185:
	shl	dh,01h
	rcl	cx,01h
;		si += cx
	add	si,cx
;		di = si
	mov	di,si
;		--si
	dec	si
;		ds = bp
	mov	ds,bp
;		al = b[di]
	mov	al,byte ptr [di]
;		rep.dec
	std
;		rep.cpy di,si,cx
	rep	movsb
;		rep.set di,al
	stosb
;		rep.inc
	cld
;		go NEXTCOLOR
	jmp short	L$153
;		RDB(C6)
L$169:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$170
	call	_PI_RdBit_ReadBuf
	jmp short	L$170
;		RDB(C7)
L$172:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$173
	call	_PI_RdBit_ReadBuf
	jmp short	L$173
;		RDB(C8)
L$174:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$175
	call	_PI_RdBit_ReadBuf
	jmp short	L$175
;		RDB(C9)
L$176:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$177
	call	_PI_RdBit_ReadBuf
	jmp short	L$177
;		RDB(Ca)
L$178:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$179
	call	_PI_RdBit_ReadBuf
	jmp short	L$179
;		RDB(Cb)
L$180:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$181
	call	_PI_RdBit_ReadBuf
	jmp short	L$181
;		RDB(Cc)
L$182:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$183
	call	_PI_RdBit_ReadBuf
	jmp short	L$183
;		RDB(Cd)
L$184:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$185
	call	_PI_RdBit_ReadBuf
	jmp short	L$185
;	
;	NEXTCOLOR:
L$153:
;		pop  ds
	pop	ds
;		push ds
	push	ds
;	
;		ah = al
	mov	ah,al
;		al = 0
	xor	al,al
;		si = ax
	mov	si,ax
;	
;		RdBit(D0)
	dec	dl
	je	L$186
L$187:
	shl	dh,01h
;		if (cf)
	jnc	L$188
;			RdBit(De)	//in bx,dx	out cf
	dec	dl
	je	L$189
L$190:
	shl	dh,01h
;			|ds = bp
	mov	ds,bp
;			if (cf)
	jnc	L$191
;				cl = ah
	mov	cl,ah
;				rep.load.w ax,si
	lodsw
;				ah <=> al
	xchg	ah,al
;				w[si-2] = ax
	mov	word ptr [si-02h],ax
;				ah = al
	mov	ah,al
;				al = cl
	mov	al,cl
;				pop  ds
	pop	ds
;				return
	pop	es
	pop	di
	ret
;				RDB(De)
L$189:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$190
	call	_PI_RdBit_ReadBuf
	jmp short	L$190
;			fi
L$191:
;			rep.load al,si
	lodsb
;			al <=> ah
	xchg	al,ah
;			pop  ds
	pop	ds
;			return
	pop	es
	pop	di
	ret
;		////
;			RDB(D0)
L$186:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$187
	call	_PI_RdBit_ReadBuf
	jmp short	L$187
;			RDB(D1)
L$192:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$193
	call	_PI_RdBit_ReadBuf
	jmp short	L$193
;			RDB(D2)
L$194:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$195
	call	_PI_RdBit_ReadBuf
	jmp short	L$195
;			RDB(D3)
L$196:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$197
	call	_PI_RdBit_ReadBuf
	jmp short	L$197
;			RDB(D4)
L$198:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$199
	call	_PI_RdBit_ReadBuf
	jmp short	L$199
;			RDB(D5)
L$200:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$201
	call	_PI_RdBit_ReadBuf
	jmp short	L$201
;		fi
L$188:
;		cx = 1
	mov	cx,01h
;		RdBit(D1)
	dec	dl
	je	L$192
L$193:
	shl	dh,01h
;		if (cf)
	jnc	L$202
;			RdBit(D2)
	dec	dl
	je	L$194
L$195:
	shl	dh,01h
;			if (cf)
	jnc	L$203
;				RdBit(D3)
	dec	dl
	je	L$196
L$197:
	shl	dh,01h
;				if (cf)
	jnc	L$204
;					RdBit(D4)
	dec	dl
	je	L$198
L$199:
	shl	dh,01h
;					if (cf)
	jnc	L$205
;						RdBit(D5)
	dec	dl
	je	L$200
L$201:
	shl	dh,01h
;						if (cf)
	jnc	L$206
;							RdBit(D6)
	dec	dl
	je	L$207
L$208:
	shl	dh,01h
;							if (cf)
	jnc	L$209
;								RdBit(D7);rcl cx
	dec	dl
	je	L$210
L$211:
	shl	dh,01h
	rcl	cx,01h
;							fi
L$209:
;							RdBit(D8);rcl cx
	dec	dl
	je	L$212
L$213:
	shl	dh,01h
	rcl	cx,01h
;						fi
L$206:
;						RdBit(D9);rcl cx
	dec	dl
	je	L$214
L$215:
	shl	dh,01h
	rcl	cx,01h
;					fi
L$205:
;					RdBit(Da);rcl cx
	dec	dl
	je	L$216
L$217:
	shl	dh,01h
	rcl	cx,01h
;				fi
L$204:
;				RdBit(Db);rcl cx
	dec	dl
	je	L$218
L$219:
	shl	dh,01h
	rcl	cx,01h
;			fi
L$203:
;			RdBit(Dc);rcl cx
	dec	dl
	je	L$220
L$221:
	shl	dh,01h
	rcl	cx,01h
;		fi
L$202:
;		RdBit(Dd);rcl cx
	dec	dl
	je	L$222
L$223:
	shl	dh,01h
	rcl	cx,01h
;		si += cx
	add	si,cx
;		di = si
	mov	di,si
;		--si
	dec	si
;		ds = bp
	mov	ds,bp
;		al = b[di]
	mov	al,byte ptr [di]
;		rep.dec
	std
;		rep.cpy di,si,cx
	rep	movsb
;		rep.set di,al
	stosb
;		rep.inc
	cld
;		al <=> ah
	xchg	al,ah
;		pop  ds
	pop	ds
;		return
	pop	es
	pop	di
	ret
;	
;		RDB(D6)
L$207:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$208
	call	_PI_RdBit_ReadBuf
	jmp short	L$208
;		RDB(D7)
L$210:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$211
	call	_PI_RdBit_ReadBuf
	jmp short	L$211
;		RDB(D8)
L$212:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$213
	call	_PI_RdBit_ReadBuf
	jmp short	L$213
;		RDB(D9)
L$214:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$215
	call	_PI_RdBit_ReadBuf
	jmp short	L$215
;		RDB(Da)
L$216:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$217
	call	_PI_RdBit_ReadBuf
	jmp short	L$217
;		RDB(Db)
L$218:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$219
	call	_PI_RdBit_ReadBuf
	jmp short	L$219
;		RDB(Dc)
L$220:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$221
	call	_PI_RdBit_ReadBuf
	jmp short	L$221
;		RDB(Dd)
L$222:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$223
	call	_PI_RdBit_ReadBuf
	jmp short	L$223
;	endproc

;	
;	//-----------------------------------------------------------------------------
;	
;	proc PiLoad0(ds,es)
;		endproc
;	
;	*proc Load(piPtr:d, putLinFunc:d, putLinDat:d, linCnt:w);cdecl;far
;		//Pi画像の展開.
;		//putLinFuncは16色/256色対応の１行出力ﾙｰﾁﾝのｱﾄﾞﾚｽ
;		//linCnt は展開する最大行数. 0なら ysizeとなる
;		//putLinDat は １行出力ﾙｰﾁﾝに渡される。そのﾙｰﾁﾝのﾃﾞｰﾀ領域へのポインタを想定
;		//ret ax:error no
;	enter
;		save bx,cx,si,di,es,ds
;	
;		// 設定
;		rep.inc
	public	_PI_Load
_PI_Load	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	push	bp
	mov	bp,sp
	cld
;		sav_sp = sp
	mov	word ptr cs:[_PI_sav_sp],sp
;		
;		dx.ax = piPtr
	mov	ax,word ptr [bp+18]
	mov	dx,word ptr [bp+20]
;		Std.FpToHp dx,ax,cx
	mov	cx,ax
	shr	cx,04h
	add	dx,cx
	and	ax,0Fh
;		if (dx == 0 && ax == 0)
	or	dx,dx
	jne	L$224
	or	ax,ax
	jne	L$224
;			ax = 4
	mov	ax,04h
;			jmp Exit
	jmp far ptr	_PI_Exit
;		fi
L$224:
;		if (ax)
	or	ax,ax
	je	L$225
;			++dx
	inc	dx
;		fi
L$225:
;		ds = dx
	mov	ds,dx
;		es = dx
	mov	es,dx
;		PIT.putLinDt = dx.ax = putLinDat
	mov	ax,word ptr [bp+26]
	mov	dx,word ptr [bp+28]
	mov	word ptr ds:[03Ah],ax
	mov	word ptr ds:[03Ch],dx
;	
;		ax = linCnt
	mov	ax,word ptr [bp+30]
;		if (ax == 0 || ax > PIT.ysize)
	or	ax,ax
	je	L$227
	cmp	ax,word ptr ds:[04h]
	jbe	L$226
L$227:
;			ax = PIT.ysize
	mov	ax,word ptr ds:[04h]
;		fi
L$226:
;		PIT.ymax = ax
	mov	word ptr ds:[046h],ax
;	
;		//出力関数の用意
;		if (PIT.pln == 4)
	cmp	word ptr ds:[00h],04h
	jne	L$228
;			PIT.readColor = ax = &ReadColor16
	mov	ax,offset _TEXT:_PI_ReadColor16
	mov	word ptr ds:[052h],ax
;			PIT.putLin = dx.ax = putLinFunc
	mov	ax,word ptr [bp+22]
	mov	dx,word ptr [bp+24]
	mov	word ptr ds:[036h],ax
	mov	word ptr ds:[038h],dx
;			InitColTbl16		//色表の初期化
	call	_PI_InitColTbl16
;		else	//256色
	jmp	L$229
L$228:
;			PIT.readColor = ax = &ReadColor256
	mov	ax,offset _TEXT:_PI_ReadColor256
	mov	word ptr ds:[052h],ax
;			PIT.putLin = dx.ax = putLinFunc
	mov	ax,word ptr [bp+22]
	mov	dx,word ptr [bp+24]
	mov	word ptr ds:[036h],ax
	mov	word ptr ds:[038h],dx
;		  @if 1
;			if (dx == 0)
	or	dx,dx
	jne	L$230
;				ax = 4
	mov	ax,04h
;				jmp Exit
	jmp far ptr	_PI_Exit
;			fi
L$230:
;			ax = ds
	mov	ax,ds
;			ax += 0x1000
	add	ax,01000h
;			c256bufSeg = ax
	mov	word ptr cs:[_PI_c256bufSeg],ax
;		  @else
;			if (c256bufSeg == 0 || (dx == 0 /*&& ax == 0*/))
;				ax = 4
;				jmp Exit
;			fi
;		  @fi
;			InitColTbl256		//色表の初期化
	call	_PI_InitColTbl256
;		fi
L$229:
;	
;		//初期設定
;		cx = ax = PIT.xsize
	mov	ax,word ptr ds:[02h]
	mov	cx,ax
;		if (cx > PI.PIXLINSIZ|| cx <= 2)
	cmp	cx,01000h
	ja	L$232
	cmp	cx,02h
	ja	L$231
L$232:
;			ax = 3
	mov	ax,03h
;			jmp Exit
	jmp far ptr	_PI_Exit
;		fi
L$231:
;	
;	
;		//pixBuf関係設定
;		cx = ax = PIT.xsize
	mov	ax,word ptr ds:[02h]
	mov	cx,ax
;		ax += ax
	add	ax,ax
;		bx = %PI.T.pixBuf
	mov	bx,01000h
;		ax += bx			// ax = xsize * 2 + %PI.T.pixBuf
	add	ax,bx
;		PIT.pixStart = ax
	mov	word ptr ds:[03Eh],ax
;		dx.ax = ww(0, PI.PIXBUF_SIZE)
	mov	ax,06000h
	xor	dx,dx
;		div dx.ax , cx
	div	cx
;		ax -= 2
	dec	ax
	dec	ax
;		ax &= 0xFFFE
	and	ax,0FFFEh
;		//if ((cx & 0x0001) && (ax & 0x0001))
;		//	ax -= 1
;		//fi
;		PIT.pixBlkLin = ax		//pixBlkLin = (PIXBUF_SIZE / xsize - 2)&0xfffe
	mov	word ptr ds:[056h],ax
;		dx.ax = cx * ax
	mul	cx
;		ax += bx
	add	ax,bx
;		PIT.pixHisSrc = ax		//pixHisSrc = %PI.T.pixBuf + xsize * pixBlkLin
	mov	word ptr ds:[042h],ax
;		ax += cx
	add	ax,cx
;		ax += cx
	add	ax,cx
;		PIT.pixEnd = ax		//pixEnd = %PI.T.pixBuf + xsize * (pixBlkLin+2)
	mov	word ptr ds:[040h],ax
;		//
;		ax = PIT.ymax
	mov	ax,word ptr ds:[046h]
;		dx = 0
	xor	dx,dx
;		div dx.ax, PIT.pixBlkLin
	div	word ptr ds:[056h]
;		PIT.pixEndLin = dx
	mov	word ptr ds:[058h],dx
;		if (dx == 0)
	or	dx,dx
	jne	L$233
;			PIT.pixEndLin = dx = PIT.pixBlkLin
	mov	dx,word ptr ds:[056h]
	mov	word ptr ds:[058h],dx
;			--ax
	dec	ax
;		fi
L$233:
;		PIT.pixBlkCnt = ax
	mov	word ptr ds:[054h],ax
;	
;		PiLoad0 ds,es
	call	_PI_PiLoad0
;		return
	db	0C9h	;leave
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	cx
	db	0CBh	;retf
;	endproc

;	
;	@if defined(DEBUG)
;	proc Deb_Col(ax)
;	begin
;		save ax
;		ah&=0x0f;al<<=4;al|=ah;ah=0;Pri.Fmt ".%02X\N",ax
;		return
;	endproc
;	@fi
;	
;	proc PiLoad0(ds,es)
;	begin
;	
;		//位置番号1～4 のオフセット値を設定
;		ax = PIT.xsize
_PI_PiLoad0	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	ax,word ptr ds:[02h]
;		neg ax
	neg	ax
;		cx = ax
	mov	cx,ax
;		PIT.ofs1 = ax	//( 0,-1)
	mov	word ptr ds:[04Ch],ax
;		ax += ax
	add	ax,ax
;		PIT.ofs2 = ax	//( 0,-2)
	mov	word ptr ds:[04Eh],ax
;		cx += 1
	inc	cx
;		PIT.ofs3 = cx	//( 1,-1)
	mov	word ptr ds:[050h],cx
;	
;		gosub JIKOKAKIKAE
	call	L$234
;	
;		// 読み込みスタート
;		bx = PIT.readPtr
	mov	bx,word ptr ds:[048h]
;		dl = 1
	mov	dl,01h
;	
;		// １番左上のピクセルを取り出して、仮想上の前２ラインを塗りつぶす
;		ax = 0
	xor	ax,ax
;		call PIT.readColor
	call	word ptr ds:[052h]
;		rep.set.w %PI.T.pixBuf, ax, PIT.xsize
	mov	cx,word ptr ds:[02h]
	mov	di,01000h
	rep	stosw
;		@if defined(DEBUG);al <<= 4; al |= ah; ah = 0;Pri.Fmt "Lc%02X\N",ax;@fi
;	
;		//メイン・ループ
;		bp = 0			//bp 前回のオフセット値(位置情報)
	xor	bp,bp
;		go MAINLOOP
	jmp short	L$235
;	
;			RDB(P0)
L$236:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$237
	call	_PI_RdBit_ReadBuf
	jmp short	L$237
;			RDB(P1)
L$238:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$239
	call	_PI_RdBit_ReadBuf
	jmp short	L$239
;	
;		  POS01:
L$240:
;				RdBit(P1)	//in bx,dx	out cf
	dec	dl
	je	L$238
L$239:
	shl	dh,01h
;				if (cf == 0)
	jc	L$241
;					si = -4
	mov	si,(-4)
;					ax = w[di-2]
	mov	ax,word ptr [di-02h]
;					|al -= ah
	sub	al,ah
;					@if defined(DEBUG);|ax = 0;@fi
;					if (==)
	jne	L$242
;						si += 2
	inc	si
	inc	si
;						go (bp != -4) J1
	cmp	bp,(-4)
	jne	L$243
;						go.w LOOP_COL
	jmp	L$244
;					fi
L$242:
;					go (bp != -2) J1
	cmp	bp,(-2)
	jne	L$243
;					go.w LOOP_COL
	jmp	L$244
;				else
L$241:
;					//si = PIT.ofs1
;					data;b 0xbe;enddata; ofs1:	;data;w 0;enddata
	db	0BEh
L$246:
	dw	00h
;					@if defined(DEBUG);ax = 1;@fi
;					go J1
	jmp short	L$243
;				fi
L$245:
;	
;		MAINLOOP:
L$235:
;			//位置情報(0-4)を得、それに対応するオフセット値を得る
;			RdBit(P0)
	dec	dl
	je	L$236
L$237:
	shl	dh,01h
;			go (cf == 0) POS01
	jnc	L$240
;			RdBit(P2)	//in bx,dx	out cf
	dec	dl
	je	L$247
L$248:
	shl	dh,01h
;			if (cf == 1)
	jnc	L$249
;				//si = PIT.ofs3
;				data;b 0xbe;enddata; ofs3:;data;w 0;enddata
	db	0BEh
L$250:
	dw	00h
;				@if defined(DEBUG);ax = 3;@fi
;				RdBit(P3)	//in bx,dx	out cf
	dec	dl
	je	L$251
L$252:
	shl	dh,01h
;				if (cf)
	jnc	L$253
;					si -= 2
	dec	si
	dec	si
;					@if defined(DEBUG);ax = 4;@fi
;				fi
L$253:
;				go J1
	jmp short	L$243
;				RDB (P2)
L$247:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$248
	call	_PI_RdBit_ReadBuf
	jmp short	L$248
;				RDB (P3)
L$251:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$252
	call	_PI_RdBit_ReadBuf
	jmp short	L$252
;			else
	jmp	L$254
L$249:
;				@if defined(DEBUG);ax = 2;@fi
;				//si = PIT.ofs2
;				data;b 0xbe;enddata; ofs2:;data;w 0;enddata
	db	0BEh
L$255:
	dw	00h
;			fi
L$254:
;		  J1:
L$243:
;	
;			//前回の位置と同じなら色展開のループへ移行
;			if (si == bp)
	cmp	si,bp
	jne	L$256
;			  LOOP_COL:
L$244:
;				loop
L$257:
;					al = b[di - 1]
	mov	al,byte ptr [di-01h]
;					@if defined(DEBUG);ah=0;Pri.Fmt"Lc%02X\N",ax;@fi
;					call PIT.readColor
	call	word ptr ds:[052h]
;					@if defined(DEBUG);Deb_Col ax;@fi
;					rep.set.w di,ax
	stosw
;					//al = ah
;					go (di >= PIT.pixEnd) COL_PUTLIN
	cmp	di,word ptr ds:[040h]
	jae	L$260
;				  COL_PUTLIN_RET:
L$261:
;					RdBit(CO)	//in bx,dx	out cf //result cf
	dec	dl
	je	L$262
L$263:
	shl	dh,01h
;				endloop (cf)
	jc	L$257
;				bp = 0
	xor	bp,bp
;				go MAINLOOP
	jmp short	L$235
;			///////////////////////////////
;				RDB(CO)
L$262:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$263
	call	_PI_RdBit_ReadBuf
	jmp short	L$263
;			CPY1:
L$264:
;				rep.cpy.w di,si
	movsw
;			  @if defined(DEBUG)
;				Pri.Fmt "1\N"
;				go.w (di </*!=*/ PIT.pixEnd) MAINLOOP
;			  @else
;				go (di </*!=*/ PIT.pixEnd) MAINLOOP
	cmp	di,word ptr ds:[040h]
	jb	L$235
;			  @fi
;				gosub PutPixBuf
	call	L$265
;			  @if defined(DEBUG)
;				go.w MAINLOOP
;			  @else
;				go MAINLOOP
	jmp short	L$235
;			  @fi
;			COL_PUTLIN:
L$260:
;				gosub PutPixBuf
	call	L$265
;				go COL_PUTLIN_RET
	jmp short	L$261
;			/////////////////
;			fi
L$256:
;		  J2:
L$266:
;			@if defined(DEBUG); Pri.Fmt "@%d ",ax; @fi
;			// 今回の位置番号を設定
;			bp = si
	mov	bp,si
;			// コピー元アドレスの設定
;			si += di
	add	si,di
;	
;			//長さを読み込む
;			RdBit(L1)	//in bx,dx	out cf
	dec	dl
	je	L$267
L$268:
	shl	dh,01h
;			go (cf == 0) CPY1	//１ピクセル(2ﾄﾞｯﾄ)・コピー
	jnc	L$264
;			cx = 0
	xor	cx,cx
;			loop
L$269:
;				++cx
	inc	cx
;				RdBit(L2)	//in bx,dx	out cf
	dec	dl
	je	L$272
L$273:
	shl	dh,01h
;			endloop(cf)
	jc	L$269
;			go (cx > 15) LPCPY	//長さが 2ﾊﾞｲﾄ正数におさまらないとき LCPYへ
	cmp	cx,0Fh
	ja	L$274
;			ax = 1
	mov	ax,01h
;			loop
L$275:
;				RdBit(L3)	//in bx,dx	out cf
	dec	dl
	je	L$278
L$279:
	shl	dh,01h
;				rcl ax
	rcl	ax,01h
;			endloop (--cx)
	loop	L$275
;	
;			@if defined(DEBUG); Pri.Fmt "%d\N",ax; @fi
;			// 長さ分、相対位置よりコピー
;		  PCPY:
L$280:
;			loop
L$281:
;				cx = ax
	mov	cx,ax
;				ax = PIT.pixEnd
	mov	ax,word ptr ds:[040h]
;				ax -= di
	sub	ax,di
;				ax >>= 1
	shr	ax,01h
;				if (cx < ax)
	cmp	cx,ax
	jae	L$284
;					rep.cpy.w di,si,cx
	rep	movsw
;					go.w MAINLOOP
	jmp	L$235
;				fi
L$284:
;				cx -= ax
	sub	cx,ax
;				cx <=> ax
	xchg	cx,ax
;				rep.cpy.w di,si,cx
	rep	movsw
;				si -= di
	sub	si,di
;				gosub PutPixBuf
	call	L$265
;				si += di
	add	si,di
;			endloop
	jmp short	L$281
;		///////////////////////////
;			RDB(L1)
L$267:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$268
	call	_PI_RdBit_ReadBuf
	jmp short	L$268
;			RDB(L2)
L$272:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$273
	call	_PI_RdBit_ReadBuf
	jmp short	L$273
;			RDB(L3)
L$278:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$279
	call	_PI_RdBit_ReadBuf
	jmp short	L$279
;			RDB(L4)
L$285:
	mov	dl,08h
	mov	dh,byte ptr [bx]
	inc	bx
	jne	L$286
	call	_PI_RdBit_ReadBuf
	jmp short	L$286
;		LPCPY:	// 長さ分、相対位置よりコピー
L$274:
;			push bp
	push	bp
;				bp.ax = 1
	mov	ax,01h
	xor	bp,bp
;				loop
L$287:
;					RdBit(L4)	//in bx,dx	out cf
	dec	dl
	je	L$285
L$286:
	shl	dh,01h
;					rcl bp.ax
	rcl	ax,01h
	rcl	bp,01h
;				endloop (--cx)
	loop	L$287
;				@if defined(DEBUG); Pri.Fmt "%ld\N",bp.cx; @fi
;	
;				loop
L$290:
;					cx = 0
	xor	cx,cx
;					loop
L$293:
;						rep.cpy.w di,si
	movsw
;						if (di >= PIT.pixEnd)
	cmp	di,word ptr ds:[040h]
	jb	L$296
;							si -= di
	sub	si,di
;							gosub PutPixBuf
	call	L$265
;							si += di
	add	si,di
;						fi
L$296:
;					endloop (--cx)
	loop	L$293
;				endloop (--bp)
	dec	bp
	jne	L$290
;			pop bp
	pop	bp
;		  @if defined(DEBUG)
;			go.w PCPY
;		  @else
;			go PCPY
	jmp short	L$280
;		  @fi
;		return
	ret
;	
;	PutPixBuf:
L$265:
;		pusha
	db	60h	;pusha
;		push es
	push	es
;		rep.cpy.w %PI.T.pixBuf, PIT.pixHisSrc, PIT.xsize
	mov	cx,word ptr ds:[02h]
	mov	si,word ptr ds:[042h]
	mov	di,01000h
	rep	movsw
;		cx = PIT.pixBlkLin
	mov	cx,word ptr ds:[056h]
;		if (PIT.pixBlkCnt == 0)
	cmp	word ptr ds:[054h],00h
	jne	L$297
;			cx = PIT.pixEndLin
	mov	cx,word ptr ds:[058h]
;		fi
L$297:
;		loop
L$298:
;			push di,cx
	push	cx
	push	di
;			push ds
	push	ds
;			push PIT.putLinDt, ds.di
	push	ds
	push	di
	push	word ptr ds:[03Ch]
	push	word ptr ds:[03Ah]
;				call PIT.putLin
	call	dword ptr ds:[036h]
;			sp += 8
	add	sp,08h
;			pop  ds
	pop	ds
;			pop  di,cx
	pop	di
	pop	cx
;			di += PIT.xsize
	add	di,word ptr ds:[02h]
;		endloop (--cx)
	loop	L$298
;		if (PIT.pixBlkCnt == 0)
	cmp	word ptr ds:[054h],00h
	jne	L$301
;			ax = 0
	xor	ax,ax
;			jmp Exit
	jmp far ptr	_PI_Exit
;		fi
L$301:
;		--PIT.pixBlkCnt
	dec	word ptr ds:[054h]
;		if (PIT.pixBlkCnt == 0)
	cmp	word ptr ds:[054h],00h
	jne	L$302
;			cx = PIT.xsize
	mov	cx,word ptr ds:[02h]
;			dx.ax = PIT.pixEndLin * cx
	mov	ax,cx
	mul	word ptr ds:[058h]
;			ax += cx
	add	ax,cx
;			ax += cx
	add	ax,cx
;			ax += %PI.T.pixBuf
	add	ax,01000h
;		  @if 1
;			++ax
	inc	ax
;			ax &= ~1
	and	ax,(-2)
;		  @fi
;			PIT.pixEnd = ax
	mov	word ptr ds:[040h],ax
;		fi
L$302:
;		pop es
	pop	es
;		popa
	db	61h	;popa
;		di = PIT.pixStart
	mov	di,word ptr ds:[03Eh]
;		ret
	ret
;	
;	JIKOKAKIKAE:
L$234:
;		w[cs:%ofs1] = ax = PIT.ofs1
	mov	ax,word ptr ds:[04Ch]
	mov	word ptr cs:[L$246],ax
;		w[cs:%ofs2] = ax = PIT.ofs2
	mov	ax,word ptr ds:[04Eh]
	mov	word ptr cs:[L$255],ax
;		w[cs:%ofs3] = ax = PIT.ofs3
	mov	ax,word ptr ds:[050h]
	mov	word ptr cs:[L$250],ax
;		ret
	ret
;	
;	endproc

;	
;	endmodule
_TEXT	ends
	end
