;	/*
;		Mag 展開
;			writen by M.Kitamura(Tenk*)
;			1993-1995
;	 */
;	module MAG
;		c_name "MAG","_",1
;		import Std
;		import Dos
;	//	import Pri
;	//	import Key
;	//@define DEBUG() 1
;	
;	///////////////////////////////////////////////////////////////////////////////
;	//@define M64	1
;	@if defined(M64)	//T=64Kバイト版
;	const PIXBUF_SIZE  = 2048*17	// 34Kバイト^^;
;	const PIXLINSIZ    = 2048
;	const FLAGLINESIZE = 2048/8	 						//(PIXLINSIZ/8)
;	const COMMENT_SIZE = 4096-256-768-256-(2048/8)-2	//(2048/8)=(FLAGLINESIZE)
;	const FLG_A_SIZE   = 4*1024
;	const FLG_B_SIZE   = (8)*1024
;	const DAT_P_SIZE   = (14)*1024
;	@else	//T=128Kバイト版
;	const PIXBUF_SIZE  = 64*1024
	.186
DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
;	const PIXLINSIZ    = 4096
;	const FLAGLINESIZE = 4096/8 						//(PIXLINSIZ/8)
;	const COMMENT_SIZE = 4096-256-768-256-(4096/8)-2	//(4096/8)=(FLAGLINESIZE)
;	const FLG_A_SIZE   = 8*1024
;	const FLG_B_SIZE   = 20*1024
;	const DAT_P_SIZE   = 32*1024
;	@fi
;	
;	*const MF_200L = 0x01,MF_8C = 0x02, MF_DIG = 0x04, MF_256C = 0x80
;	
;	@define MT()	MAG.T[ds:0]
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
;		xofs8:w				// 始点ｘオフセット
;		xsize8:w			// 8ドット間隔でのｘサイズ
;		xsizeO:w			// 展開での横カウンタ
;		ycnt:w				// 展開での縦カウンタ
;		rdAcnt:b,rdAdat:b	// フラグA 読み込み関係
;		ptrA:w
;		ptrB:w				// フラグB 読み込み関係
;		ptrP:w				// ピクセルデータ読み込み関係
;		pos:w(16)			// 比較点へのオフセット
;		/*-------------*/
;		warning:b			//
;		macNo:b				// 機種番号
;		macFlg:b			// 機種依存データ
;		scrnMode:b			// スクリーン・モード
;		flagAofs:d			// フラグＡへのオフセット
;		flagAsiz:d			// フラグＡのサイズ
;		flagBofs:d			// フラグＢへのオフセット
;		flagBsiz:d			// フラグＢのサイズ
;		dataPofs:d			// ピクセル・データへのオフセット
;		dataPsiz:d			// ピクセル・データのサイズ
;		hdrOfs:d			// ヘッダへのオフセット
;		/*-------------*/
;		dummy2:b( 256 /* -16*/ -(14*2+6+20) -(2*4+5*2) -(8*2+16*2) -(4+7*4))
;		/*-------------*/
;		palette:b(256*3)
;		fileName:b(254+2)
;		comment:b(COMMENT_SIZE + 2)
;		/*--------------------------------------------------------*/
;		flagLine:b(FLAGLINESIZE)
;		/*------------------------- ここまでで 4Kバイト ---*/
;	//  @if defined(M64)	//ANA のバグで、構造体中で @if が使えない(T T)
;	//	pixBuf:b(PIXBUF_SIZE)
;	//  @fi
;		flagAbuf:b(FLG_A_SIZE)
;		flagBbuf:b(FLG_B_SIZE)
;		dataPbuf:b(DAT_P_SIZE)
;	endstruct
;	
;	
;	struct HDR		// MAGヘッダ
;		nil:b		// 0
;		macNo:b		// 機種番号
;		macFlg:b	// 機種依存データ
;		scrnMode:b	// スクリーン・モード
;		xstart:w	// 始点ｘ
;		ystart:w	// 始点ｙ
;		xend:w		// 終点ｘ
;		yend:w		// 終点ｙ
;		flagAofs:d	// フラグＡへのオフセット
;		flagBofs:d	// フラグＢへのオフセット
;		flagBsiz:d	// フラグＢのサイズ
;		dataPofs:d	// ピクセル・データへのオフセット
;		dataPsiz:d	// ピクセル・データのサイズ
;		grbPal:b()	// GRBパレット・データ
;	endstruct
;	
;	
;	///////////////////////////////////////////////////////////////////////////////
;	
;	cvar hdrHdl:w
_BSS	ends

_TEXT	segment byte public 'CODE'
_MAG_hdrHdl	label	word
	db	2 dup(?)
;	
;	proc InitGetC(ax)
;	begin
;		hdrHdl = ax
_MAG_InitGetC	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	word ptr cs:[_MAG_hdrHdl],ax
;		return
	ret
;	endproc

;	
;	proc GetC()
;		// ax : 読み込んだ値
;		// cf : Read Error
;	begin
;		save bx,cx,dx,ds,es
;		var gcbuf:w = 0
_TEXT	ends

_DATA	segment word public 'DATA'
L$1	label	word
	db	2 dup(0)
;	
;		ds = ax = %var
_DATA	ends

_TEXT	segment byte public 'CODE'
_MAG_GetC	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	dx
	push	bx
	push	ds
	push	es
	mov	ax,DGROUP
	mov	ds,ax
;		Dos.Read hdrHdl,&gcbuf,1
	mov	cx,01h
	mov	dx,offset DGROUP:L$1
	mov	bx,word ptr cs:[_MAG_hdrHdl]
	mov	ah,03Fh
	int	021h
;		|ax = gcbuf
	mov	ax,word ptr [L$1]
;		return
	pop	es
	pop	ds
	pop	bx
	pop	dx
	pop	cx
	ret
;	endproc

;	
;	
;	*proc Open(magPtr:d, fileName:d); cdecl; far
;	//	MAGファイルをオープンしてヘッダを読み込む
;	enter
;		save bx,cx,si,di,es,ds
;		local idbuf:b(10)
;		local hdr:MAG.HDR
;	
;		// 設定
;		rep.inc
	public	_MAG_Open
_MAG_Open	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	enter	02Ah,00h
	cld
;	
;		bx.ax = magPtr
	mov	ax,word ptr [bp+18]
	mov	bx,word ptr [bp+20]
;		magPtr = 0
	mov	word ptr [bp+18],00h
	mov	word ptr [bp+20],00h
;		Std.FpToHp bx,ax,cx
	mov	cx,ax
	shr	cx,04h
	add	bx,cx
	and	ax,0Fh
;		go.w (bx == 0 && ax == 0) RTS
	or	bx,bx
	jne	L$3
	or	ax,ax
	jne	L$3
	jmp	L$2
L$3:
;		if (ax)
	or	ax,ax
	je	L$4
;			++bx
	inc	bx
;		fi
L$4:
;		ax = 0
	xor	ax,ax
;		magPtr = bx.ax
	mov	word ptr [bp+18],ax
	mov	word ptr [bp+20],bx
;		es = bx
	mov	es,bx
;		rep.set.w 0, ax, %MAG.T.dummy2
	mov	cx,098h
	xor	di,di
	rep	stosw
;	
;		// ファイル名コピー
;		ds.si = fileName
	lds	si,dword ptr [bp+22]
;		di = %MAG.T.fileName
	mov	di,0400h
;		loop
L$5:
;			rep.load al,si
	lodsb
;			rep.set di,al
	stosb
;		endloop (al)
	or	al,al
	jne	L$5
;	
;		//ファイル・オープン
;		ds = bx	//es
	mov	ds,bx
;		//Pri.Fmt "%Fs:%Fs\n", fileName, ww(ds,%MAG.T.fileName)
;		Dos.Open %MAG.T.fileName, 0x00
	xor	al,al
	mov	dx,0400h
	mov	ah,03Dh
	int	021h
;		go.w (cf) OPEN_ERR
	jnc	L$9
	jmp	L$8
L$9:
;		MT.filHdl = ax
	mov	word ptr ds:[01Ah],ax
;		bx = ax
	mov	bx,ax
;	
;		//IDチェック
;		push ds,es
	push	es
	push	ds
;			ax.dx = ww(ss, &idbuf)
	lea	dx,BYTE ptr [bp-10]
	mov	ax,ss
;			ds = ax
	mov	ds,ax
;			es = ax
	mov	es,ax
;			Dos.Read bx, dx, 8
	mov	cx,08h
	mov	ah,03Fh
	int	021h
;			go.w (cf) FIL_ERR
	jnc	L$11
	jmp	L$10
L$11:
;			ds = ax = %var
	mov	ax,DGROUP
	mov	ds,ax
;			repe.cmp.w &idbuf, "MAKI02  ", 4
	mov	cx,04h
	mov	si,offset DGROUP:L$12
	lea	di,BYTE ptr [bp-10]
	repe	cmpsw
;			go.w (!=) HDR_ERR
	je	L$14
	jmp	L$13
L$14:
;		pop  ds,es
	pop	ds
	pop	es
;	
;		//機種名取得
;		Dos.Read bx, %MAG.T.saverName, 4
	mov	cx,04h
	mov	dx,01Ch
	mov	ah,03Fh
	int	021h
;		go.w (cf) FIL_ERR
	jnc	L$15
	jmp	L$10
L$15:
;		MT.saverName(4) = 0
	mov	byte ptr ds:[020h],00h
;	
;		//バイト読み込み初期化
;		InitGetC MT.filHdl
	mov	ax,word ptr ds:[01Ah]
	call	_MAG_InitGetC
;	
;		// ユーザ名取得
;		di = %MAG.T.artist
	mov	di,022h
;		cx = 18
	mov	cx,012h
;		GetC	//result al
	call	_MAG_GetC
;		go.w (cf) FIL_ERR
	jnc	L$16
	jmp	L$10
L$16:
;		go (al != ' ' && al != '\t') J1
	cmp	al,020h
	je	L$18
	cmp	al,09h
	jne	L$17
L$18:
;		loop
L$19:
;			GetC
	call	_MAG_GetC
;			go.w (cf) FIL_ERR
	jnc	L$22
	jmp	L$10
L$22:
;	  J1:
L$17:
;			exit (al == 0x1a)
	cmp	al,01Ah
	je	L$21
;			if (al)
	or	al,al
	je	L$23
;				rep.set di,al
	stosb
;			fi
L$23:
;		endloop (--cx)
	loop	L$19
L$21:
;		dh = al
	mov	dh,al
;		rep.set di,'\0'
	xor	al,al
	stosb
;		//Pri.Fmt "%Fs\n",ww(ds,%MAG.T.artist)
;	
;		//コメント収得
;		cx = COMMENT_SIZE
	mov	cx,08FEh
;		di = %MAG.T.comment
	mov	di,0500h
;		b[di] = 0
	mov	byte ptr [di],00h
;		go.w (dh == 0x1A) J3	//
	cmp	dh,01Ah
	jne	L$25
	jmp	L$24
L$25:
;		GetC
	call	_MAG_GetC
;		go.w (cf) FIL_ERR
	jnc	L$26
	jmp	L$10
L$26:
;		go.w (al == 0x1A) J3	//
	cmp	al,01Ah
	jne	L$27
	jmp	L$24
L$27:
;		go (al != ' ' && al != '\t') J2
	cmp	al,020h
	je	L$29
	cmp	al,09h
	jne	L$28
L$29:
;		loop
L$30:
;			GetC
	call	_MAG_GetC
;			go.w (cf) FIL_ERR
	jnc	L$33
	jmp	L$10
L$33:
;		  J2:
L$28:
;			exit (al == 0x1a)
	cmp	al,01Ah
	je	L$32
;			next (cx == 0)
	jcxz	L$31
;			next (al == '\0')
	or	al,al
	je	L$31
;			rep.set di,al
	stosb
;			--cx
	dec	cx
;		endloop
L$31:
	jmp short	L$30
L$32:
;		b[di] = 0
	mov	byte ptr [di],00h
;	  J3:
L$24:
;		//Pri.Fmt "[cmt]%Fs\n",ww(ds,%MAG.T.comment)
;	
;		//-- ヘッダ位置を求めておく
;		cx.dx = 0
	xor	dx,dx
	mov	cx,dx
;		al = 1
	mov	al,01h
;		Dos.Lseek bx, cx.dx, al
	mov	ah,042h
	int	021h
;		go.w (cf) FIL_ERR
	jnc	L$34
	jmp	L$10
L$34:
;		MT.hdrOfs = dx.ax
	mov	word ptr ds:[094h],ax
	mov	word ptr ds:[096h],dx
;		//Pri.Fmt "ヘッダ先頭:%lx\N", dx.ax
;	
;		//-- ヘッダ読み込み--
;		push ds
	push	ds
;			ds.dx = ww(ss,&hdr)
	lea	dx,BYTE ptr [bp-42]
	push	ss
	pop	ds
;			Dos.Read bx, dx, sizeof(MAG.HDR)
	mov	cx,020h
	mov	ah,03Fh
	int	021h
;		|pop ds
	pop	ds
;		go.w (cf) FIL_ERR
	jnc	L$35
	jmp	L$10
L$35:
;	
;		// ヘッダ情報判別
;		MT.macNo = ah = hdr.macNo
	mov	ah,byte ptr [bp-41]
	mov	byte ptr ds:[079h],ah
;		MT.macFlg = al = hdr.macFlg
	mov	al,byte ptr [bp-40]
	mov	byte ptr ds:[07Ah],al
;		if (ah == 98 && al & 0x10)	//透明色があるばあい
	cmp	ah,062h
	jne	L$36
	test	al,010h
	je	L$36
;			al &= 0x0f
	and	al,0Fh
;			al += 1
	inc	al
;			ah = 0
	xor	ah,ah
;			MT.overlayColor = ax
	mov	word ptr ds:[0Eh],ax
;		fi
L$36:
;		MT.scrnMode = al = hdr.scrnMode
	mov	al,byte ptr [bp-39]
	mov	byte ptr ds:[07Bh],al
;	
;		// ドット比
;		MT.aspect1 = ax = 0
	xor	ax,ax
	mov	word ptr ds:[0Ah],ax
;		MT.aspect2 = ax = 0
	xor	ax,ax
	mov	word ptr ds:[0Ch],ax
;		if (MT.scrnMode & MAG.MF_200L)
	test	byte ptr ds:[07Bh],01h
	je	L$37
;			MT.aspect1 = 1
	mov	word ptr ds:[0Ah],01h
;			MT.aspect2 = 2
	mov	word ptr ds:[0Ch],02h
;		fi
L$37:
;	
;		// 色数
;		MT.pln = 4
	mov	word ptr ds:[00h],04h
;		MT.color = 16
	mov	word ptr ds:[010h],010h
;		if (MT.scrnMode & MAG.MF_256C)
	test	byte ptr ds:[07Bh],080h
	je	L$38
;			MT.pln = 8
	mov	word ptr ds:[00h],08h
;			MT.color = 256
	mov	word ptr ds:[010h],0100h
;		fi
L$38:
;	
;		//座標を設定
;		MT.ystart = ax = hdr.ystart
	mov	ax,word ptr [bp-36]
	mov	word ptr ds:[08h],ax
;		dx = hdr.yend
	mov	dx,word ptr [bp-32]
;		++dx
	inc	dx
;		dx -= ax
	sub	dx,ax
;		MT.ysize = dx
	mov	word ptr ds:[04h],dx
;		go.w (dx == 0) VAL_ERR
	or	dx,dx
	jne	L$40
	jmp	L$39
L$40:
;	
;		MT.xstart = ax = hdr.xstart
	mov	ax,word ptr [bp-38]
	mov	word ptr ds:[06h],ax
;		dx = hdr.xend
	mov	dx,word ptr [bp-34]
;		++dx
	inc	dx
;		cx = dx
	mov	cx,dx
;		dx -= ax
	sub	dx,ax
;		MT.xsize = dx
	mov	word ptr ds:[02h],dx
;		go.w (dx == 0) VAL_ERR
	or	dx,dx
	jne	L$41
	jmp	L$39
L$41:
;	
;		//８ドット間隔でのｘサイズを求める
;		dx = ax	//MT.xstart
	mov	dx,ax
;		if (MT.pln == 4)
	cmp	word ptr ds:[00h],04h
	jne	L$42
;			ax &= 0x7
	and	ax,07h
;			MT.xofs8 = ax
	mov	word ptr ds:[048h],ax
;			if (cx & 0x07)
	test	cx,07h
	je	L$43
;				cx += 8
	add	cx,08h
;			fi
L$43:
;			cx &= ~0x07
	and	cx,(-8)
;			dx &= ~0x07
	and	dx,(-8)
;			cx -= dx
	sub	cx,dx
;			MT.xsize8 = cx
	mov	word ptr ds:[04Ah],cx
;		else
	jmp	L$44
L$42:
;			ax &= 0x3
	and	ax,03h
;			MT.xofs8 = ax
	mov	word ptr ds:[048h],ax
;			if (cx & 0x03)
	test	cx,03h
	je	L$45
;				cx += 4
	add	cx,04h
;			fi
L$45:
;			cx &= ~0x03
	and	cx,(-4)
;			dx &= ~0x03
	and	dx,(-4)
;			cx -= dx
	sub	cx,dx
;			MT.xsize8 = cx
	mov	word ptr ds:[04Ah],cx
;		fi
L$44:
;		go.w (cx == 0) VAL_ERR
	jcxz	L$46
	jmp short	L$47
L$46:
	jmp	L$39
L$47:
;	
;		// フラグAのオフセット
;		dx.ax = hdr.flagAofs
	mov	ax,word ptr [bp-30]
	mov	dx,word ptr [bp-28]
;		if (ax & 0x01)
	test	ax,01h
	je	L$48
;			MT.warning = 1
	mov	byte ptr ds:[078h],01h
;			//Pri.Fmt "MAGヘッダでフラグＡのオフセット値が不正かも"
;		fi
L$48:
;		dx.ax += MT.hdrOfs
	add	ax,word ptr ds:[094h]
	adc	dx,word ptr ds:[096h]
;		MT.flagAofs = dx.ax
	mov	word ptr ds:[07Ch],ax
	mov	word ptr ds:[07Eh],dx
;	
;		// フラグAのサイズを求める
;		dx.ax = MT.xsize8 * MT.ysize
	mov	ax,word ptr ds:[04h]
	mul	word ptr ds:[04Ah]
;		if (MT.pln == 8)
	cmp	word ptr ds:[00h],08h
	jne	L$49
;			if (ax & 0x1f)
	test	ax,01Fh
	je	L$50
;				dx.ax += 4*8
	add	ax,020h
	adc	dx,00h
;			fi
L$50:
;			dx.ax >>= 5		//dx.ax /= 4*8
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
;		else
	jmp	L$51
L$49:
;			if (ax & 0x3f)
	test	ax,03Fh
	je	L$52
;				dx.ax += 4*2*8
	add	ax,040h
	adc	dx,00h
;			fi
L$52:
;			dx.ax >>= 6		//dx.ax /= 4*2*8
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
	shr	dx,01h
	rcr	ax,01h
;		fi
L$51:
;		MT.flagAsiz = dx.ax
	mov	word ptr ds:[080h],ax
	mov	word ptr ds:[082h],dx
;	
;		// フラグBのサイズ&オフセット
;		MT.flagBsiz = dx.ax = hdr.flagBsiz
	mov	ax,word ptr [bp-22]
	mov	dx,word ptr [bp-20]
	mov	word ptr ds:[088h],ax
	mov	word ptr ds:[08Ah],dx
;		dx.ax = hdr.flagBofs
	mov	ax,word ptr [bp-26]
	mov	dx,word ptr [bp-24]
;		if (ax & 0x01)
	test	ax,01h
	je	L$53
;			MT.warning = 2
	mov	byte ptr ds:[078h],02h
;			//Pri.Fmt "MAGヘッダでフラグＢのオフセット値が不正かも"
;		fi
L$53:
;		dx.ax += MT.hdrOfs
	add	ax,word ptr ds:[094h]
	adc	dx,word ptr ds:[096h]
;		MT.flagBofs = dx.ax
	mov	word ptr ds:[084h],ax
	mov	word ptr ds:[086h],dx
;	
;		// ピクセルデータへのサイズ＆オフセット
;		MT.dataPsiz = dx.ax = hdr.dataPsiz
	mov	ax,word ptr [bp-14]
	mov	dx,word ptr [bp-12]
	mov	word ptr ds:[090h],ax
	mov	word ptr ds:[092h],dx
;		dx.ax = hdr.dataPofs
	mov	ax,word ptr [bp-18]
	mov	dx,word ptr [bp-16]
;		if (ax & 0x01)
	test	ax,01h
	je	L$54
;			MT.warning = 3
	mov	byte ptr ds:[078h],03h
;			//Pri.Fmt "MAGヘッダでピクセル・データのオフセット値が不正かも"
;		fi
L$54:
;		dx.ax += MT.hdrOfs
	add	ax,word ptr ds:[094h]
	adc	dx,word ptr ds:[096h]
;		MT.dataPofs = dx.ax
	mov	word ptr ds:[08Ch],ax
	mov	word ptr ds:[08Eh],dx
;	
;		//パレット読み込み
;		di = %MAG.T.palette
	mov	di,0100h
;		rep.set.w di,0,256*3/2
	mov	cx,0180h
	xor	ax,ax
	rep	stosw
;		dx.ax = 48
	mov	ax,030h
	xor	dx,dx
;		if (MT.pln == 8)
	cmp	word ptr ds:[00h],08h
	jne	L$55
;			ax = 256 * 3
	mov	ax,0300h
;		fi
L$55:
;		cx = ax
	mov	cx,ax
;		di = dx = %MAG.T.palette
	mov	dx,0100h
	mov	di,dx
;		Dos.Read bx, dx, cx
	mov	ah,03Fh
	int	021h
;		go.w (cf) FIL_ERR
	jnc	L$56
	jmp	L$10
L$56:
;		loop
L$57:
;			ax = w[di]
	mov	ax,word ptr [di]
;			ah <=> al
	xchg	ah,al
;			w[di] = ax
	mov	word ptr [di],ax
;			di += 3
	add	di,03h
;		endloop ((cx -= 3) > 0)
	sub	cx,03h
	ja	L$57
;	
;	  @if 0
;		Pri.Fmt "%04x : %04x\n", %MAG.T.palette, %MAG.T.fileName
;		Pri.Fmt "%Fs : %Fs\n", ww(ds,%MAG.T.fileName), ww(ds,%MAG.T.artist)
;		Pri.Fmt "[%d] %3d*%-3d  (%3d,%3d)  %d:%d\n",
;			MT.pln,MT.xsize,MT.ysize,MT.xstart,MT.ystart,MT.aspect1,MT.aspect2
;		Pri.Fmt "     xsize8=%d xofs8=%d\n",MT.xsize8,MT.xofs8
;		Pri.Fmt "flagA:%08lx sz%8lx(%ld)\n",MT.flagAofs, MT.flagAsiz, MT.flagAsiz
;		Pri.Fmt "flagB:%08lx sz%8lx(%ld)\n",MT.flagBofs, MT.flagBsiz, MT.flagBsiz
;		Pri.Fmt "dataP:%08lx sz%8lx(%ld)\n",MT.dataPofs, MT.dataPsiz, MT.dataPsiz
;		Pri.Fmt "[CMT]%Fs\n",ww(ds,%MAG.T.comment);
;	  @fi
;		dx.ax = magPtr
	mov	ax,word ptr [bp+18]
	mov	dx,word ptr [bp+20]
;	  RTS:
L$2:
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
;	FIL_ERR:
L$10:
;	OPEN_ERR:	//ファイル・オープンできなかった
L$8:
;		dx.ax = 1
	mov	ax,01h
	xor	dx,dx
;		go RTS
	jmp short	L$2
;	
;	HDR_ERR:	//Pri.Fmt "magのヘッダでない\N"
L$13:
;		dx.ax = 2
	mov	ax,02h
	xor	dx,dx
;		go RTS
	jmp short	L$2
;	
;	VAL_ERR:	//Pri.Fmt "ヘッダに矛盾がある\N"
L$39:
;		dx.ax = 3
	mov	ax,03h
	xor	dx,dx
;		go RTS
	jmp short	L$2
;	
;	endproc

_TEXT	ends

_DATA	segment word public 'DATA'
L$12 label byte
	db	77,65,75,73,48,50,32,32,0
;	
;	
;	*proc Close(magPtr:d);cdecl;far
;	enter
;		save ax,bx,dx,ds
;	
;		bx.ax = magPtr
_DATA	ends

_TEXT	segment byte public 'CODE'
	public	_MAG_Close
_MAG_Close	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	dx
	push	bx
	push	ds
	push	bp
	mov	bp,sp
	mov	ax,word ptr [bp+14]
	mov	bx,word ptr [bp+16]
;		Std.FpToHp bx,ax,dx
	mov	dx,ax
	shr	dx,04h
	add	bx,dx
	and	ax,0Fh
;		go.w (bx == 0 && ax == 0) RTS
	or	bx,bx
	jne	L$61
	or	ax,ax
	jne	L$61
	jmp	L$60
L$61:
;		if (ax)
	or	ax,ax
	je	L$62
;			++bx
	inc	bx
;		fi
L$62:
;		ds = bx
	mov	ds,bx
;		Dos.Close MT.filHdl
	mov	bx,word ptr ds:[01Ah]
	mov	ah,03Eh
	int	021h
;	  RTS:
L$60:
;		return
	db	0C9h	;leave
	pop	ds
	pop	bx
	pop	dx
	pop	ax
	db	0CBh	;retf
;	endproc

;	
;	
;	proc ReadBufA()
;	begin
;		save ax,bx,cx,dx
;		bx = MT.filHdl
_MAG_ReadBufA	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	cx
	push	dx
	push	bx
	mov	bx,word ptr ds:[01Ah]
;		cx.dx = MT.flagAofs
	mov	dx,word ptr ds:[07Ch]
	mov	cx,word ptr ds:[07Eh]
;		Dos.Lseek bx, cx.dx, 0
	xor	al,al
	mov	ah,042h
	int	021h
;		dx.ax = MT.flagAsiz
	mov	ax,word ptr ds:[080h]
	mov	dx,word ptr ds:[082h]
;		if (dx || ax > FLG_A_SIZE)
	or	dx,dx
	jne	L$64
	cmp	ax,02000h
	jbe	L$63
L$64:
;			dx.ax -= FLG_A_SIZE
	sub	ax,02000h
	sbb	dx,00h
;			MT.flagAsiz = dx.ax
	mov	word ptr ds:[080h],ax
	mov	word ptr ds:[082h],dx
;			Dos.Read bx, %MAG.T.flagAbuf, FLG_A_SIZE
	mov	cx,02000h
	mov	dx,01000h
	mov	ah,03Fh
	int	021h
;			dx.ax = MT.flagAofs
	mov	ax,word ptr ds:[07Ch]
	mov	dx,word ptr ds:[07Eh]
;			dx.ax += FLG_A_SIZE
	add	ax,02000h
	adc	dx,00h
;			MT.flagAofs = dx.ax
	mov	word ptr ds:[07Ch],ax
	mov	word ptr ds:[07Eh],dx
;		elsif (ax)
	jmp	L$65
L$63:
	or	ax,ax
	je	L$66
;			cx = ax
	mov	cx,ax
;			MT.flagAsiz = 0
	mov	word ptr ds:[080h],00h
	mov	word ptr ds:[082h],00h
;			dx.ax = MT.flagAofs
	mov	ax,word ptr ds:[07Ch]
	mov	dx,word ptr ds:[07Eh]
;			dx.ax += ww(0,cx)
	add	ax,cx
	adc	dx,00h
;			MT.flagAofs = dx.ax
	mov	word ptr ds:[07Ch],ax
	mov	word ptr ds:[07Eh],dx
;			Dos.Read bx, %MAG.T.flagAbuf, cx
	mov	dx,01000h
	mov	ah,03Fh
	int	021h
;		fi
L$66:
L$65:
;		MT.ptrA = %MAG.T.flagAbuf
	mov	word ptr ds:[052h],01000h
;		MT.rdAcnt = 1
	mov	byte ptr ds:[050h],01h
;		//MT.rdAdat = 0
;		return
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc ReadBufB()
;	begin
;		save ax,bx,cx,dx,es
;		es = ds
_MAG_ReadBufB	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	cx
	push	dx
	push	bx
	push	es
	push	ds
	pop	es
;		bx = MT.filHdl
	mov	bx,word ptr ds:[01Ah]
;		cx.dx = MT.flagBofs
	mov	dx,word ptr ds:[084h]
	mov	cx,word ptr ds:[086h]
;		Dos.Lseek bx, cx.dx, 0
	xor	al,al
	mov	ah,042h
	int	021h
;		dx.ax = MT.flagBsiz
	mov	ax,word ptr ds:[088h]
	mov	dx,word ptr ds:[08Ah]
;		if (dx || ax > FLG_B_SIZE)
	or	dx,dx
	jne	L$68
	cmp	ax,05000h
	jbe	L$67
L$68:
;			dx.ax -= FLG_B_SIZE
	sub	ax,05000h
	sbb	dx,00h
;			MT.flagBsiz = dx.ax
	mov	word ptr ds:[088h],ax
	mov	word ptr ds:[08Ah],dx
;			Dos.Read bx, %MAG.T.flagBbuf, FLG_B_SIZE
	mov	cx,05000h
	mov	dx,03000h
	mov	ah,03Fh
	int	021h
;			dx.ax = MT.flagBofs
	mov	ax,word ptr ds:[084h]
	mov	dx,word ptr ds:[086h]
;			dx.ax += FLG_B_SIZE
	add	ax,05000h
	adc	dx,00h
;			MT.flagBofs = dx.ax
	mov	word ptr ds:[084h],ax
	mov	word ptr ds:[086h],dx
;		elsif (ax)
	jmp	L$69
L$67:
	or	ax,ax
	je	L$70
;			cx = ax
	mov	cx,ax
;			MT.flagBsiz = 0
	mov	word ptr ds:[088h],00h
	mov	word ptr ds:[08Ah],00h
;			dx.ax = MT.flagBofs
	mov	ax,word ptr ds:[084h]
	mov	dx,word ptr ds:[086h]
;			dx.ax += ww(0,cx)
	add	ax,cx
	adc	dx,00h
;			MT.flagBofs = dx.ax
	mov	word ptr ds:[084h],ax
	mov	word ptr ds:[086h],dx
;			Dos.Read bx, %MAG.T.flagBbuf, cx
	mov	dx,03000h
	mov	ah,03Fh
	int	021h
;		fi
L$70:
L$69:
;		MT.ptrB = %MAG.T.flagBbuf
	mov	word ptr ds:[054h],03000h
;		return
	pop	es
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	proc ReadBufP()
;		// ret bx = &MAG.T.dataPbuf
;		// break ax
;	begin
;		save cx,dx,es
;		es = ds
_MAG_ReadBufP	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	dx
	push	es
	push	ds
	pop	es
;		bx = MT.filHdl
	mov	bx,word ptr ds:[01Ah]
;		cx.dx = MT.dataPofs
	mov	dx,word ptr ds:[08Ch]
	mov	cx,word ptr ds:[08Eh]
;		Dos.Lseek bx, cx.dx, 0
	xor	al,al
	mov	ah,042h
	int	021h
;		dx.ax = MT.dataPsiz
	mov	ax,word ptr ds:[090h]
	mov	dx,word ptr ds:[092h]
;		if (dx || ax > DAT_P_SIZE)
	or	dx,dx
	jne	L$72
	cmp	ax,08000h
	jbe	L$71
L$72:
;			dx.ax -= DAT_P_SIZE
	sub	ax,08000h
	sbb	dx,00h
;			MT.dataPsiz = dx.ax
	mov	word ptr ds:[090h],ax
	mov	word ptr ds:[092h],dx
;			Dos.Read bx, %MAG.T.dataPbuf, DAT_P_SIZE
	mov	cx,08000h
	mov	dx,(-32768)
	mov	ah,03Fh
	int	021h
;			dx.ax = MT.dataPofs
	mov	ax,word ptr ds:[08Ch]
	mov	dx,word ptr ds:[08Eh]
;			dx.ax += DAT_P_SIZE
	add	ax,08000h
	adc	dx,00h
;			MT.dataPofs = dx.ax
	mov	word ptr ds:[08Ch],ax
	mov	word ptr ds:[08Eh],dx
;		elsif (ax)
	jmp	L$73
L$71:
	or	ax,ax
	je	L$74
;			cx = ax
	mov	cx,ax
;			MT.dataPsiz = 0
	mov	word ptr ds:[090h],00h
	mov	word ptr ds:[092h],00h
;			dx.ax = MT.dataPofs
	mov	ax,word ptr ds:[08Ch]
	mov	dx,word ptr ds:[08Eh]
;			dx.ax += ww(0,cx)
	add	ax,cx
	adc	dx,00h
;			MT.dataPofs = dx.ax
	mov	word ptr ds:[08Ch],ax
	mov	word ptr ds:[08Eh],dx
;			Dos.Read bx, %MAG.T.dataPbuf, cx
	mov	dx,(-32768)
	mov	ah,03Fh
	int	021h
;		fi
L$74:
L$73:
;		MT.ptrP = bx = %MAG.T.dataPbuf
	mov	bx,(-32768)
	mov	word ptr ds:[056h],bx
;		return
	pop	es
	pop	dx
	pop	cx
	ret
;	endproc

;	
;	@if 0
;	//	proc ReadA()
;	//	フラグA のバッファより１ビット読み出す
;	//	out cf
;	//	break si
;	@define ReadA()									\
;		if (--MT.rdAcnt == 0);						\
;			push ax;								\
;			si = MT.ptrA;							\
;			if (si >= %MAG.T.flagAbuf+FLG_A_SIZE);	\
;				ReadBufA;							\
;				si = MT.ptrA;						\
;			fi;										\
;			rep.load al,si;							\
;			MT.rdAdat = al;							\
;			MT.ptrA = si;							\
;			MT.rdAcnt = 8;							\
;			pop  ax;								\
;		fi;											\
;		|MT.rdAdat <<= 1;							\
;	
;	//	proc ReadB()
;	//	フラグB のバッファより１バイト読み出す
;	//	out al
;	//	break	si
;	@define ReadB()									\
;		si = MT.ptrB;								\
;		if (si >= %MAG.T.flagBbuf+FLG_B_SIZE);		\
;			ReadBufB;								\
;			si = MT.ptrB;							\
;		fi;											\
;		rep.load al,si;								\
;		MT.ptrB = si;								\
;	
;	
;	//	proc ReadP()
;	//	ピクセル・データのバッファより 1バイト読み出す
;	//	out ax
;	//	break si
;	@define ReadP()									\
;		si = MT.ptrP;								\
;	  @if 0;										\
;		if (si >= %MAG.T.dataPbuf+DAT_P_SIZE);		\
;	  @else;										\
;		if (si == 0);								\
;	  @endif;										\
;			ReadBufP;								\
;			si = MT.ptrP;							\
;		fi;											\
;		rep.load.w ax, si;							\
;		MT.ptrP = si;								\
;	@fi
;	
;	
;	*proc Load(magPtr:d, putLinFunc:d, putLinDat:d, linCnt:w);cdecl;far
;		//画像の展開.
;		//putLinFuncは16色/256色対応の１行出力ﾙｰﾁﾝのｱﾄﾞﾚｽ
;		//linCnt は展開する最大行数. 0なら ysizeとなる
;		//putLinDat は １行出力ﾙｰﾁﾝに渡される。そのﾙｰﾁﾝのﾃﾞｰﾀ領域へのポインタを想定
;		//ret dx.ax:構造体へのポインタ(ｵﾌｾｯﾄ0) ｾｸﾞﾒﾝﾄが0ならｴﾗｰ(ｵﾌｾｯﾄ値がｴﾗｰNo)
;	enter
;		save  bx,cx,dx,si,di,es,ds,fx
;	
;		// 設定
;		rep.inc
	public	_MAG_Load
_MAG_Load	label	far
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	pushf
	push	bp
	mov	bp,sp
	cld
;		dx.ax = magPtr
	mov	ax,word ptr [bp+22]
	mov	dx,word ptr [bp+24]
;		Std.FpToHp dx,ax,cx
	mov	cx,ax
	shr	cx,04h
	add	dx,cx
	and	ax,0Fh
;		if (dx == 0 && ax == 0)
	or	dx,dx
	jne	L$75
	or	ax,ax
	jne	L$75
;			ax = 4
	mov	ax,04h
;			go.w RET
	jmp	L$76
;		fi
L$75:
;		if (ax)
	or	ax,ax
	je	L$77
;			++dx
	inc	dx
;		fi
L$77:
;		ds = dx
	mov	ds,dx
;		es = dx
	mov	es,dx
;	
;		// 出力手続き用意
;		MT.putLin   = dx.ax = putLinFunc
	mov	ax,word ptr [bp+26]
	mov	dx,word ptr [bp+28]
	mov	word ptr ds:[036h],ax
	mov	word ptr ds:[038h],dx
;		MT.putLinDt = dx.ax = putLinDat
	mov	ax,word ptr [bp+30]
	mov	dx,word ptr [bp+32]
	mov	word ptr ds:[03Ah],ax
	mov	word ptr ds:[03Ch],dx
;	
;		// 展開できる行数をセット
;		ax = linCnt
	mov	ax,word ptr [bp+34]
;		if (ax == 0 || ax > MT.ysize)
	or	ax,ax
	je	L$79
	cmp	ax,word ptr ds:[04h]
	jbe	L$78
L$79:
;			ax = MT.ysize
	mov	ax,word ptr ds:[04h]
;		fi
L$78:
;		MT.ycnt = ax	//展開する行数
	mov	word ptr ds:[04Eh],ax
;	
;		// 横幅チェック
;		ax = MT.xsize8
	mov	ax,word ptr ds:[04Ah]
;		if (ax > MAG.PIXLINSIZ)
	cmp	ax,01000h
	jbe	L$80
;			ax = 3
	mov	ax,03h
;			go.w RET
	jmp	L$76
;		fi
L$80:
;	
;		// フラグ展開用のライン・バッファの初期化
;		rep.set.w %MAG.T.flagLine, 0, (PIXLINSIZ / 8)/2
	mov	cx,0100h
	xor	ax,ax
	mov	di,0E00h
	rep	stosw
;	
;		// 読み込みバッファの準備
;		ReadBufA
	call	_MAG_ReadBufA
;		ReadBufB
	call	_MAG_ReadBufB
;		ReadBufP
	call	_MAG_ReadBufP
;	
;		//展開で用いる相対位置のオフセット値を求める.
;		//１６色時、１dot４ビット。作業1ピクセル(2byte)は4dot単位
;		//256 色時、１dot８ビット。作業1ピクセル(2byte)は2dot単位
;		cx = MT.xsize8
	mov	cx,word ptr ds:[04Ah]
;		dx = 4
	mov	dx,04h
;		if (MT.pln == 8)
	cmp	word ptr ds:[00h],08h
	jne	L$81
;			dx = 2
	mov	dx,02h
;		fi
L$81:
;		ax = 0
	xor	ax,ax
;		MT.pos(0) = ax		//( 0, 0)
	mov	word ptr ds:[058h],ax
;		ax += dx
	add	ax,dx
;		MT.pos(1) = ax		//(-1, 0)
	mov	word ptr ds:[05Ah],ax
;		ax += dx
	add	ax,dx
;		MT.pos(2) = ax		//(-2, 0)
	mov	word ptr ds:[05Ch],ax
;		ax += dx
	add	ax,dx
;		ax += dx
	add	ax,dx
;		MT.pos(3) = ax		//(-4, 0)
	mov	word ptr ds:[05Eh],ax
;		ax = cx
	mov	ax,cx
;		MT.pos(4) = ax		//( 0,-1)
	mov	word ptr ds:[060h],ax
;		ax += dx
	add	ax,dx
;		MT.pos(5) = ax		//(-1,-1)
	mov	word ptr ds:[062h],ax
;		ax = cx
	mov	ax,cx
;		ax <<= 1
	shl	ax,01h
;		MT.pos(6) = ax		//( 0,-2)
	mov	word ptr ds:[064h],ax
;		ax += dx
	add	ax,dx
;		MT.pos(7) = ax		//(-1,-2)
	mov	word ptr ds:[066h],ax
;		ax += dx
	add	ax,dx
;		MT.pos(8) = ax		//(-2,-2)
	mov	word ptr ds:[068h],ax
;		ax = cx
	mov	ax,cx
;		ax <<= 2
	shl	ax,02h
;		MT.pos(9) = ax		//( 0,-4)
	mov	word ptr ds:[06Ah],ax
;		ax += dx
	add	ax,dx
;		MT.pos(10) = ax 	//(-1,-4)
	mov	word ptr ds:[06Ch],ax
;		ax += dx
	add	ax,dx
;		MT.pos(11) = ax 	//(-2,-4)
	mov	word ptr ds:[06Eh],ax
;		ax = cx
	mov	ax,cx
;		ax <<= 3
	shl	ax,03h
;		MT.pos(12) = ax 	//( 0,-8)
	mov	word ptr ds:[070h],ax
;		ax += dx
	add	ax,dx
;		MT.pos(13) = ax 	//(-1,-8)
	mov	word ptr ds:[072h],ax
;		ax += dx
	add	ax,dx
;		MT.pos(14) = ax 	//(-2,-8)
	mov	word ptr ds:[074h],ax
;		ax = cx
	mov	ax,cx
;		ax <<= 4
	shl	ax,04h
;		MT.pos(15) = ax 	//(0,-16)
	mov	word ptr ds:[076h],ax
;	
;		// ピクセルバッファ初期化
;		//rep.set.b %MAG.T.pixBuf, 0, PIXBUF_SIZE
;	
;		// 展開したピクセルデータを収める開始アドレスと終了アドレスを求める
;		cx    = MT.xsize8
	mov	cx,word ptr ds:[04Ah]
;		//// Pri.Fmt "x8=%d  ",cx
;		dx.ax = cx * 16		// 先頭 16 行分はヒストリバッファ
	mov	ax,010h
	mul	cx
;		MT.pixHisSiz = ax	// ヒストリ・サイズ
	mov	word ptr ds:[044h],ax
;		//// Pri.Fmt "pixHisSiz=%x  ",ax
;		bx = ax
	mov	bx,ax
;	  @if defined(M64)
;		ax += %MAG.T.pixBuf
;	  @fi
;		MT.pixStart = ax	// ピクセルバッファの展開開始位置
	mov	word ptr ds:[03Eh],ax
;		//// Pri.Fmt "pixStart=%x  ",ax
;		dx.ax = PIXBUF_SIZE
	xor	ax,ax
	mov	dx,01h
;		div dx.ax, cx
	div	cx
;		//// Pri.Fmt "pix行数=%d  ",ax
;		dx.ax = cx * ax
	mul	cx
;	  @if defined(M64)
;		ax += %MAG.T.pixBuf
;	  @fi
;		MT.pixEnd   = ax	// ピクセルバッファの終り
	mov	word ptr ds:[040h],ax
;		//// Pri.Fmt "pixEnd=%x  ",ax
;		ax -= bx
	sub	ax,bx
;		MT.pixHisSrc = ax		// バッファ更新でヒストリバッファへのコピー元
	mov	word ptr ds:[042h],ax
;		//// Pri.Fmt "pixHisSrc=%x\n",ax
;	
;		// 展開開始
;	  @if defined(M64)==0
;		ax = ds
	mov	ax,ds
;		ax += 0x1000
	add	ax,01000h
;		es = ax
	mov	es,ax
;	  @fi
;		push bp
	push	bp
;		di = MT.pixStart		//ピクセルを収めるバッファへのポインタ
	mov	di,word ptr ds:[03Eh]
;	
;		ax = MT.xsize8			//展開する横幅
	mov	ax,word ptr ds:[04Ah]
;		if.w (MT.pln == 4)
	cmp	word ptr ds:[00h],04h
	je	L$83
	jmp	L$82
L$83:
;			ax >>= 3
	shr	ax,03h
;			MT.xsizeO = ax		// 8ドット単位の横幅
	mov	word ptr ds:[04Ch],ax
;		else
	jmp	L$84
L$82:
;			ax >>= 2
	shr	ax,02h
;			MT.xsizeO = ax		// 4ドット単位の横幅
	mov	word ptr ds:[04Ch],ax
;		fi
L$84:
;	
;		go LOOPM	//	サブルーチンがあるためスキップ
	jmp short	L$85
;			GET_FLG_A_SUB1:	//フラグ A より 1バイト取ってくる
L$86:
;				go (di >= %MAG.T.flagAbuf+FLG_A_SIZE) GET_FLG_A_SUB1_J
	cmp	di,03000h
	jae	L$87
;				dl = b[di]
	mov	dl,byte ptr [di]
;				++di
	inc	di
;				dh = 8
	mov	dh,08h
;				go RET_GET_FLG_A_SUB1
	jmp short	L$88
;			GET_FLG_A_SUB1_J:
L$87:
;				ReadBufA
	call	_MAG_ReadBufA
;				di = MT.ptrA
	mov	di,word ptr ds:[052h]
;				dl = b[di]
	mov	dl,byte ptr [di]
;				++di
	inc	di
;				dh = 8
	mov	dh,08h
;				go RET_GET_FLG_A_SUB1
	jmp short	L$88
;					
;			GET_FLG_B_SUB1:	//フラグ B をファイルより読み込む
L$89:
;				ReadBufB
	call	_MAG_ReadBufB
;				si = MT.ptrB
	mov	si,word ptr ds:[054h]
;				go RET_GET_FLG_B_SUB1
	jmp short	L$90
;		LOOPM:
L$85:
;	
;		loop.w	// 指定行数分、ループ
L$91:
;			//// Pri.Fmt "ycnt=%d  @di=%04x  ",MT.ycnt, di
;			// フラグA フラグB より１行分のフラグを展開
;			cx = MT.xsizeO
	mov	cx,word ptr ds:[04Ch]
;			bx = %MAG.T.flagLine
	mov	bx,0E00h
;			//// Pri.Fmt "@%d@flag=%04x-",cx,bx
;			push di
	push	di
;			di = MT.ptrA		//フラグ A へのポインタ
	mov	di,word ptr ds:[052h]
;			dh = MT.rdAcnt	//			何ビット残っているか
	mov	dh,byte ptr ds:[050h]
;			dl = MT.rdAdat	//			データ
	mov	dl,byte ptr ds:[051h]
;			si = MT.ptrB		//フラグ b へのポインタ
	mov	si,word ptr ds:[054h]
;	
;			loop
L$94:
;				//フラグ A より 1ビット得る
;				go (--dh == 0) GET_FLG_A_SUB1; RET_GET_FLG_A_SUB1:
	dec	dh
	je	L$86
L$88:
;				|dl <<= 1
	shl	dl,01h
;				if (cf)
	jnc	L$97
;					//フラグ B より 1バイト得る
;					go (si >= %MAG.T.flagBbuf+FLG_B_SIZE) GET_FLG_B_SUB1;
	cmp	si,08000h
	jae	L$89
;					RET_GET_FLG_B_SUB1:
L$90:
;					rep.load al,si
	lodsb
;					b[bx] ^= al
	xor	byte ptr [bx],al
;				fi
L$97:
;				++bx
	inc	bx
;			endloop (--cx)
	loop	L$94
;			MT.ptrA = di
	mov	word ptr ds:[052h],di
;			MT.rdAcnt = dh
	mov	byte ptr ds:[050h],dh
;			MT.rdAdat = dl
	mov	byte ptr ds:[051h],dl
;			MT.ptrB = si
	mov	word ptr ds:[054h],si
;			pop  di
	pop	di
;			//// Pri.Fmt "%04x  ",bx
;	
;			if.w (MT.pln == 4)	//--- 16色のとき ---------
	cmp	word ptr ds:[00h],04h
	je	L$99
	jmp	L$98
L$99:
;				// フラグ・データより画像１行を展開
;				cx = MT.xsizeO
	mov	cx,word ptr ds:[04Ch]
;				bp = %MAG.T.flagLine
	mov	bp,0E00h
;				bx = MT.ptrP
	mov	bx,word ptr ds:[056h]
;				//// Pri.Fmt "@flag=%04x-",bx
;				loop					//１回のループで８ドット(バイト)展開する
L$100:
;					al = b[ds:bp]
	mov	al,byte ptr ds:[bp]
;					++bp
	inc	bp
;					push ax
	push	ax
;					if ((al>>=4) == 0)	//	相対位置番号が０のとき、
	shr	al,04h
	jne	L$103
;						// ファイルより２バイト入力
;						//go(bx >= %MAG.T.dataPbuf+DAT_P_SIZE)
;						go (bx == 0) GET_DAT_P_SUB1; RET_GET_DAT_P_SUB1:
	or	bx,bx
	je	L$104
L$105:
;						ah = al = b[bx]; ++bx
	mov	al,byte ptr [bx]
	mov	ah,al
	inc	bx
;						ah &= 0x0f
	and	ah,0Fh
;						al >>= 4
	shr	al,04h
;						rep.set.w di,ax
	stosw
;						ah = al = b[bx]; ++bx
	mov	al,byte ptr [bx]
	mov	ah,al
	inc	bx
;						ah &= 0x0f
	and	ah,0Fh
;						al >>= 4
	shr	al,04h
;						rep.set.w di,ax
	stosw
;					else				//相対値番号１～１５のとき
	jmp	L$106
L$103:
;						ah = 0			//指定位置より１ピクセル取得
	xor	ah,ah
;						ax += ax
	add	ax,ax
;						si = ax
	mov	si,ax
;						ax = w[%MAG.T.pos+si]
	mov	ax,word ptr [si+058h]
;						si = di
	mov	si,di
;						si -= ax
	sub	si,ax
;					  @if defined(M64)==0;seg_es;@fi
	db	26h	;seg es
;						rep.cpy.w di,si
	movsw
;					  @if defined(M64)==0;seg_es;@fi
	db	26h	;seg es
;						rep.cpy.w di,si
	movsw
;					fi
L$106:
;					pop ax
	pop	ax
;					if ((al&=0x0f) == 0)	//	相対位置番号が０のとき、
	and	al,0Fh
	jne	L$107
;						// ファイルより２バイト入力
;						//if(bx >= %MAG.T.dataPbuf+DAT_P_SIZE)
;						go (bx == 0) GET_DAT_P_SUB2; RET_GET_DAT_P_SUB2:
	or	bx,bx
	je	L$108
L$109:
;						ah = al = b[bx]; ++bx
	mov	al,byte ptr [bx]
	mov	ah,al
	inc	bx
;						ah &= 0x0f
	and	ah,0Fh
;						al >>= 4
	shr	al,04h
;						rep.set.w di,ax
	stosw
;						ah = al = b[bx]; ++bx
	mov	al,byte ptr [bx]
	mov	ah,al
	inc	bx
;						ah &= 0x0f
	and	ah,0Fh
;						al >>= 4
	shr	al,04h
;						rep.set.w di,ax
	stosw
;					else				//相対値番号１～１５のとき
	jmp	L$110
L$107:
;						ah = 0			//指定位置より１ピクセル取得
	xor	ah,ah
;						ax += ax
	add	ax,ax
;						si = ax
	mov	si,ax
;						ax = w[%MAG.T.pos+si]
	mov	ax,word ptr [si+058h]
;						si = di
	mov	si,di
;						si -= ax
	sub	si,ax
;					  @if defined(M64)==0;seg_es;@fi
	db	26h	;seg es
;						rep.cpy.w di,si
	movsw
;					  @if defined(M64)==0;seg_es;@fi
	db	26h	;seg es
;						rep.cpy.w di,si
	movsw
;					fi
L$110:
;				endloop (--cx)
	loop	L$100
;				MT.ptrP = bx
	mov	word ptr ds:[056h],bx
;				//// Pri.Fmt "%04x  ",bx
;				go.w J_FLG4	//サブルーチンがあるためスキップ
	jmp	L$111
;					GET_DAT_P_SUB1:
L$104:
;						ReadBufP
	call	_MAG_ReadBufP
;						go RET_GET_DAT_P_SUB1
	jmp short	L$105
;					GET_DAT_P_SUB2:
L$108:
;						ReadBufP
	call	_MAG_ReadBufP
;						go RET_GET_DAT_P_SUB2
	jmp short	L$109
;	
;			else.w	//------- 256色のとき ---------------
	jmp	L$112
L$98:
;	
;				// フラグ・データより画像１行を展開
;				cx = MT.xsizeO
	mov	cx,word ptr ds:[04Ch]
;				bp = %MAG.T.flagLine
	mov	bp,0E00h
;				bx = MT.ptrP
	mov	bx,word ptr ds:[056h]
;				//// Pri.Fmt "@flag=%04x-",bx
;				loop					//１回のループで８ドット(バイト)展開する
L$113:
;					al = b[ds:bp]
	mov	al,byte ptr ds:[bp]
;					++bp
	inc	bp
;					push ax
	push	ax
;					if ((al>>=4) == 0)	//	相対位置番号が０のとき、
	shr	al,04h
	jne	L$116
;						// ファイルより２バイト入力
;						//go(bx >= %MAG.T.dataPbuf+DAT_P_SIZE)
;						go (bx == 0) GET_DAT_P_2SUB1; RET_GET_DAT_P_2SUB1:
	or	bx,bx
	je	L$117
L$118:
;						ax = w[bx]; bx += 2
	mov	ax,word ptr [bx]
	inc	bx
	inc	bx
;						rep.set.w di,ax
	stosw
;					else				//相対値番号１～１５のとき
	jmp	L$119
L$116:
;						ah = 0			//指定位置より１ピクセル取得
	xor	ah,ah
;						ax += ax
	add	ax,ax
;						si = ax
	mov	si,ax
;						ax = w[%MAG.T.pos+si]
	mov	ax,word ptr [si+058h]
;						si = di
	mov	si,di
;						si -= ax
	sub	si,ax
;					  @if defined(M64)==0;seg_es;@fi
	db	26h	;seg es
;						rep.cpy.w di,si
	movsw
;					fi
L$119:
;					pop ax
	pop	ax
;					if ((al&=0x0f) == 0)	//	相対位置番号が０のとき、
	and	al,0Fh
	jne	L$120
;						// ファイルより２バイト入力
;						//if(bx >= %MAG.T.dataPbuf+DAT_P_SIZE)
;						go (bx == 0) GET_DAT_P_2SUB2; RET_GET_DAT_P_2SUB2:
	or	bx,bx
	je	L$121
L$122:
;						ax = w[bx]; bx += 2
	mov	ax,word ptr [bx]
	inc	bx
	inc	bx
;						rep.set.w di,ax
	stosw
;					else				//相対値番号１～１５のとき
	jmp	L$123
L$120:
;						ah = 0			//指定位置より１ピクセル取得
	xor	ah,ah
;						ax += ax
	add	ax,ax
;						si = ax
	mov	si,ax
;						ax = w[%MAG.T.pos+si]
	mov	ax,word ptr [si+058h]
;						si = di
	mov	si,di
;						si -= ax
	sub	si,ax
;					  @if defined(M64)==0;seg_es;@fi
	db	26h	;seg es
;						rep.cpy.w di,si
	movsw
;					fi
L$123:
;				endloop (--cx)
	loop	L$113
;				MT.ptrP = bx
	mov	word ptr ds:[056h],bx
;				//// Pri.Fmt "%04x  ",bx
;				go FLG8	//サブルーチンがあるためスキップ
	jmp short	L$124
;					GET_DAT_P_2SUB1:
L$117:
;						ReadBufP
	call	_MAG_ReadBufP
;						go RET_GET_DAT_P_2SUB1
	jmp short	L$118
;					GET_DAT_P_2SUB2:
L$121:
;						ReadBufP
	call	_MAG_ReadBufP
;						go RET_GET_DAT_P_2SUB2
	jmp short	L$122
;				FLG8:
L$124:
;			fi
L$112:
;			J_FLG4:
L$111:
;	
;			// 展開した一行を出力
;			block
L$125:
;				ax = di
	mov	ax,di
;				ax -= MT.xsize8
	sub	ax,word ptr ds:[04Ah]
;				ax += MT.xofs8
	add	ax,word ptr ds:[048h]
;				//// Pri.Fmt "@px%08x ",es.ax
;				pusha
	db	60h	;pusha
;				push	ds
	push	ds
;				push	es
	push	es
;				push	MT.putLinDt, es.ax
	push	es
	push	ax
	push	word ptr ds:[03Ch]
	push	word ptr ds:[03Ah]
;				call	MT.putLin
	call	dword ptr ds:[036h]
;				sp += 8
	add	sp,08h
;				pop		es
	pop	es
;				pop		ds
	pop	ds
;				popa
	db	61h	;popa
;			endblock
;	
;			//// Pri.Fmt "?%04x:%04x ",di,MT.pixEnd
;			// ピクセルバッファいっぱいまで展開したなら、バッファ更新
;			if (di == MT.pixEnd)
	cmp	di,word ptr ds:[040h]
	jne	L$128
;	  		  @if defined(M64)
;				cx = MT.pixHisSiz
;				cx >>= 1
;				rep.cpy.w %MAG.T.pixBuf, MT.pixHisSrc, cx
;			  @else
;				push ds
	push	ds
;					cx = MT.pixHisSiz
	mov	cx,word ptr ds:[044h]
;					si = MT.pixHisSrc
	mov	si,word ptr ds:[042h]
;					ds = ax = es
	mov	ax,es
	mov	ds,ax
;					cx >>= 1
	shr	cx,01h
;					rep.cpy.w 0, si, cx
	xor	di,di
	rep	movsw
;				pop  ds
	pop	ds
;			  @fi
;			fi
L$128:
;			//// Pri.Fmt "\n"
;	//Key.Wait
;		endloop (--MT.ycnt)
	dec	word ptr ds:[04Eh]
	je	L$129
	jmp	L$91
L$129:
;		pop	bp
	pop	bp
;		ax = 0
	xor	ax,ax
;	  RET:
L$76:
;		return
	db	0C9h	;leave
	popf
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	dx
	pop	cx
	db	0CBh	;retf
;	endproc

;	
;	endmodule
_TEXT	ends
	end
