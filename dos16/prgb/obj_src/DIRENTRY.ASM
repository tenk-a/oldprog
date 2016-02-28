;	module DIRENTEY
;		c_name "DirEntry","", defined(TURBO_C) + defined(LSI_C)*2
;		//model SMALL
;		stack 0x200
;	
;		import WD:WILDCMP
;		import Is:IS
;		//import Ps:PUTSTR
;	
;	*struct DTA_T
DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
_BSS	ends

_STACK	segment para stack 'STACK'
	db	512 dup(?)
;		attr_in:byte
;		drv:byte
;		name:byte(8)
;		ext:byte(3)
;		rsv:byte(8)
;		attr:byte
;		time:word
;		date:word
;		size:dword
;		pname:byte(13)
;	endstruct
;	
;	*var dta:DTA_T
_STACK	ends

_BSS	segment word public 'BSS'
	public	_DirEntrydta
_DirEntrydta	label	byte
	db	43 dup(?)
;	*var key:word
	public	_DirEntrykey
_DirEntrykey	label	word
	db	2 dup(?)
;	*var dir:byte(200)
	public	_DirEntrydir
_DirEntrydir	label	byte
	db	200 dup(?)
;	
;	proc DtaGet()
;	//	DTA のアドレスを得る
;	//	out   es.bx
;	//	break ah
;	macro
;		ah = 0x2f
;		intr 0x21
;	endproc
;	
;	proc DtaSet(dx)
;	//	DTA のアドレスを変更する
;	//	in  ds
;	//	break ah
;	macro
;		ah = 0x1a
;		intr 0x21
;	endproc
;	
;	proc DirEntryFirst(dx,cx)
;	//	in  ds
;	//	out dx,cx,ax,cf
;	macro
;		ah = 0x4e
;		intr 0x21
;	endproc
;	
;	proc DirEntryNext(dx,cx)
;	//	in  ds
;	//	out dx,cx,ax,cf
;	macro
;		ah = 0x4f
;		intr 0x21
;	endproc
;	
;	*proc Get
;	  @if defined(TURBO_C)
;		//(pRslt:d,pKey:d,pFlg:w)
;		(pRslt_l:w,pRslt_h:w,pKey_l:w,pKey_h:w,pFlg:w)
;	  @else
;		(bx.ax,dx.cx,pFlg:w)
;	  @fi
;		c_decl
;	//	ワイルド・カード機能付ファイル名のディレクトリ・エントリを得る
;	//  ワイルド・カード指定で継続のファイル名を得るときは dx.cx（pKey）= NULL
;	//	で呼び出す
;	//	arg pRslt:bx.ax	見つかったパス名を格納する場所 要128バイト以上
;	//	arg pKey:dx.cx	ワイルドカード機能付パス名 128バイト以内
;	//	arg pFlg	    ディレクトリ・エントリ検索で対象とするファイル属性
;	//						0x01(bit 0)	読出専用ファイル
;	//						0x02(bit 1)	不可視属性
;	//						0x04(bit 2)	システム・ファイル
;	//						0x08(bit 3)	ボリュームラベル
;	//						0x10(bit 4) ディレクトリ
;	//						0x20(bit 5) 保存ビット
;	//	out ax			0:エラー無  -1:設定誤り 1:これ以上ファイルがない
;	//					2:ファイルが無い
;	//	out dx.cx		そのまま
;	//	break bx
;	enter
;		local dtaSave:d, aPeriFlg:b
;		var	  aWdCdFlg:b = 0
_BSS	ends

_DATA	segment word public 'DATA'
L$1	label	byte
	db	1 dup(?)
;	  @if !defined(TURBO_C)
;		save pusha,pRslt_h:bx,pRslt_l:ax, pKey_h:dx, pKey_l:cx, si,di,es,ds
;	  @else
;		save pusha,pRet:ax,bx,cx,dx,si,di,es,ds
;	  @fi
;	
;		// DTA を設定
;		DtaGet
_DATA	ends

_TEXT	segment byte public 'CODE'
	public	_DirEntryGet
_DirEntryGet	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	push	bp
	mov	bp,sp
	sub	sp,6
	mov	ah,47
	int	33
;		dtaSave = es.bx
	mov	word ptr [bp-4],bx
	mov	word ptr [bp-2],es
;		ds = si = %var
	mov	si,DGROUP
	mov	ds,si
;		dx = &dta
	mov	dx,offset DGROUP:_DirEntrydta
;		DtaSet dx
	mov	ah,26
	int	33
;	
;	    // 引数チェック＆変数初期化
;		dx = pKey_h; cx = pKey_l
	mov	dx,word ptr [bp+26]
	mov	cx,word ptr [bp+24]
;		bx = pRslt_h;ax = pRslt_l
	mov	bx,word ptr [bp+22]
	mov	ax,word ptr [bp+20]
;		go.w (bx == 0 && ax == 0) ERR		// 結果を入れる pRslt が NULL とき
	or	bx,bx
	jne	L$3
	or	ax,ax
	jne	L$3
	jmp	L$2
L$3:
;		if (dx == 0 && cx == 0)				// pKey が NULL のとき
	or	dx,dx
	jne	L$4
	or	cx,cx
	jne	L$4
;			go.w (aWdCdFlg) SRCH_NEXT
	cmp	byte ptr [L$1],0
	je	L$6
	jmp	L$5
L$6:
;			ax = 1
	mov	ax,1
;			go.w RTS
	jmp	L$7
;		fi
L$4:
;		aWdCdFlg = 0
	mov	byte ptr [L$1],0
;		// es.si = dx.cx ポインタの正規化
;		si = cx
	mov	si,cx
;		si &= 0x0f
	and	si,15
;		cx >>= 4
	shr	cx,1
	shr	cx,1
	shr	cx,1
	shr	cx,1
;		dx += cx
	add	dx,cx
;		es = dx
	mov	es,dx
;		bx = si
	mov	bx,si
;		// pKey の ディレクトリ名を dir にコピー
;		di = dx = &dir
	mov	dx,offset DGROUP:_DirEntrydir
	mov	di,dx
;		loop
L$8:
;			al = b[es:si]
	mov	al,byte ptr es:[si]
;			++si
	inc	si
;			if (Is.Kanji(al))
	cmp	al,129
	jb	L$13
	cmp	al,159
	jbe	L$12
L$13:
	cmp	al,224
	jb	L$11
	cmp	al,252
	ja	L$11
L$12:
;				b[di] = al
	mov	byte ptr [di],al
;				++di
	inc	di
;				al = b[es:si]
	mov	al,byte ptr es:[si]
;				++si
	inc	si
;				go.w (al == 0) ERR
	or	al,al
	jne	L$14
	jmp	L$2
L$14:
;			elsif (al >= 'a' && al <= 'z')	//  半角小文字の大文字化
	jmp	L$15
L$11:
	cmp	al,97
	jb	L$16
	cmp	al,122
	ja	L$16
;				al -= 0x20
	sub	al,32
;			else
	jmp	L$15
L$16:
;				if (al == '/')	//  '/'を'\'に変える
	cmp	al,47
	jne	L$17
;					al = '\\'
	mov	al,92
;				fi
L$17:
;				if (al == '\\' || al == ':')
	cmp	al,92
	je	L$19
	cmp	al,58
	jne	L$18
L$19:
;					b[es:si-1] = al
	mov	byte ptr es:[si-1],al
;					dx = di
	mov	dx,di
;					++dx
	inc	dx
;					bx = si
	mov	bx,si
;				fi
L$18:
;			fi
L$15:
;			b[di] = al
	mov	byte ptr [di],al
;			++di
	inc	di
;		endloop (al)
	or	al,al
	jne	L$8
;		// 取り出したﾃﾞｨﾚｸﾄﾘ名 dir に *.* をつける。
;		di = dx
	mov	di,dx
;		b[di] = '*'
	mov	byte ptr [di],42
;		++di
	inc	di
;		b[di] = '.'
	mov	byte ptr [di],46
;		++di
	inc	di
;		b[di] = '*'
	mov	byte ptr [di],42
;		++di
	inc	di
;		b[di] = '\0'
	mov	byte ptr [di],0
;		// pKeyより検索するファイル名を key にコピー
;		++di
	inc	di
;		key = di
	mov	word ptr [_DirEntrykey],di
;		aPeriFlg = 0
	mov	byte ptr [bp-5],0
;		loop
L$20:
;			al = b[es:bx]
	mov	al,byte ptr es:[bx]
;			++bx
	inc	bx
;			if (Is.Kanji(al))
	cmp	al,129
	jb	L$25
	cmp	al,159
	jbe	L$24
L$25:
	cmp	al,224
	jb	L$23
	cmp	al,252
	ja	L$23
L$24:
;				b[di] = al
	mov	byte ptr [di],al
;				++di
	inc	di
;				al = b[es:bx]
	mov	al,byte ptr es:[bx]
;				++bx
	inc	bx
;				go.w (al == 0) ERR
	or	al,al
	jne	L$26
	jmp	L$2
L$26:
;			elsif (al >= 'a' && al <= 'z')
	jmp	L$27
L$23:
	cmp	al,97
	jb	L$28
	cmp	al,122
	ja	L$28
;				al -= 0x20
	sub	al,32
;			elsif (al == '*' || al == '?' || al == '[' || al == '^')
	jmp	L$27
L$28:
	cmp	al,42
	je	L$30
	cmp	al,63
	je	L$30
	cmp	al,91
	je	L$30
	cmp	al,94
	jne	L$29
L$30:
;				aWdCdFlg = 1
	mov	byte ptr [L$1],1
;			elsif (al == '.' && b[es:bx] == '\0')
	jmp	L$27
L$29:
	cmp	al,46
	jne	L$31
	cmp	byte ptr es:[bx],0
	jne	L$31
;				aPeriFlg = 1
	mov	byte ptr [bp-5],1
;			fi
L$31:
L$27:
;			b[di] = al
	mov	byte ptr [di],al
;			++di
	inc	di
;		endloop (al)
	or	al,al
	jne	L$20
;	
;	  //pusha;Ps.PutStrCr 1,ww(ds,&dir);popa
;	  //pusha;Ps.PutStrCr 1,ww(ds,key);popa
;		dx = &dir
	mov	dx,offset DGROUP:_DirEntrydir
;		cx = pFlg
	mov	cx,word ptr [bp+28]
;		DirEntryFirst dx,cx
	mov	ah,78
	int	33
;		go.w (cf) FIN
	jnc	L$33
	jmp	L$32
L$33:
;	
;		loop
L$34:
;		  //push bx,dx.ax;Ps.PutStrCr 1,ww(ds,&dta.pname);pop bx,dx.ax
;			si = &dta.pname
	mov	si,offset DGROUP:_DirEntrydta+30
;			di = 0
	xor	di,di
;			loop
L$37:
;				rep.load  al, si
	lodsb
;				if (al == '\0')
	or	al,al
	jne	L$40
;					di = si
	mov	di,si
;					--di
	dec	di
;					b[si] = '\0'
	mov	byte ptr [si],0
;					exit (aPeriFlg == 0)
	cmp	byte ptr [bp-5],0
	je	L$39
;					b[di] = '.'
	mov	byte ptr [di],46
;					exit
	jmp short	L$39
;				fi
L$40:
;			endloop (al != '.')
	cmp	al,46
	jne	L$37
L$39:
;		  //push bx,dx.ax;Ps.PutStrCr 1,ww(ds,&dta.pname);pop bx,dx.ax
;		  @if defined(TURBO_C)
;		  	cx = &dta.pname
	mov	cx,offset DGROUP:_DirEntrydta+30
;		    WD.WildCmp ww(ds,key),ds.cx
	push	ds
	push	cx
	push	ds
	push	word ptr [_DirEntrykey]
	call	_WildCmp
	add	sp,8
;		  @else
;			bx.ax = ww(ds,key)
;			dx.cx = ww(ds,&dta.pname)
;			WD.WildCmp bx.ax, dx.cx
;		  @fi
;			if (di)
	or	di,di
	je	L$41
;				b[di] = '\0'
	mov	byte ptr [di],0
;			fi
L$41:
;			exit (ax == 0)
	or	ax,ax
	je	L$36
;	  SRCH_NEXT:
L$5:
;			dx = &dir
	mov	dx,offset DGROUP:_DirEntrydir
;			cx = pFlg
	mov	cx,word ptr [bp+28]
;			DirEntryNext dx,cx
	mov	ah,79
	int	33
;			go (cf) FIN
	jc	L$32
;		endloop
	jmp short	L$34
L$36:
;		// pRslt に dir をコピー
;		bx = pRslt_h
	mov	bx,word ptr [bp+22]
;		ax = pRslt_l
	mov	ax,word ptr [bp+20]
;		di = ax
	mov	di,ax
;		ax >>= 4
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
;		bx += ax
	add	bx,ax
;		es = bx
	mov	es,bx
;		di &= 0x0f
	and	di,15
;		si = &dir
	mov	si,offset DGROUP:_DirEntrydir
;		loop
L$42:
;			rep.load al,si
	lodsb
;			rep.set  di,al
	stosb
;		endloop (al)
	or	al,al
	jne	L$42
;		// pRslt の後に見つかったファイル名をコピー
;		di -= 4
	sub	di,4
;		si = &dta.pname
	mov	si,offset DGROUP:_DirEntrydta+30
;		loop
L$45:
;			rep.load al,si
	lodsb
;			rep.set  di,al
	stosb
;		endloop (al)
	or	al,al
	jne	L$45
;		ax = 0
	xor	ax,ax
;		b[es:di] = al
	mov	byte ptr es:[di],al
;		go  RTS
	jmp short	L$7
;	  ERR:
L$2:
;	  	ax = -1
	mov	ax,(-1)
;	  	go  RTS
	jmp short	L$7
;	  FIN:
L$32:
;	  	if (ax == 0x12)
	cmp	ax,18
	jne	L$48
;	  		ax = 1
	mov	ax,1
;	  	fi
L$48:
;	  RTS:
L$7:
;	  @if defined(TURBO_C)
;	    pRet = ax
	mov	word ptr [bp+16],ax
;	  @else
;	    dx.ax .=. ax
;	    pRslt_l = ax
;	    pRslt_h = dx
;	  @fi
;		ds.dx = dtaSave
	lds	dx,dword ptr [bp-4]
;		DtaSet dx
	mov	ah,26
	int	33
;		return
	mov	sp,bp
	pop	bp
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
;	endmodule
_TEXT	ends
extrn _WildCmp:near
	end
