DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
_BSS	ends

_TEXT	segment byte public 'CODE'
_ax_get	label	near
	assume	ds:DGROUP,es:NOTHING
	xor	ah,ah
	mov	al,byte ptr [si]
	inc	si
	cmp	al,129
	jb	L$3
	cmp	al,159
	jbe	L$2
L$3:
	cmp	al,224
	jb	L$1
	cmp	al,252
	ja	L$1
L$2:
	mov	ah,byte ptr [si]
	inc	si
	or	ah,ah
	je	L$4
	xchg	ah,al
L$1:
	ret
L$4:
	xor	al,al
	ret

_cx_get	label	near
	assume	ds:DGROUP,es:NOTHING
	xor	ch,ch
	mov	cl,byte ptr es:[di]
	inc	di
	cmp	cl,129
	jb	L$7
	cmp	cl,159
	jbe	L$6
L$7:
	cmp	cl,224
	jb	L$5
	cmp	cl,252
	ja	L$5
L$6:
	mov	ch,byte ptr es:[di]
	inc	di
	or	ch,ch
	je	L$8
	xchg	ch,cl
L$5:
	ret
L$8:
	xor	cl,cl
	ret

	public	_WildCmp
_WildCmp	label	far
	assume	ds:DGROUP,es:NOTHING
	push	cx
	push	bx
	push	si
	push	di
	push	ds
	push	es
	pushf
	push	bp
	mov	bp,sp
	dec	sp
	dec	sp
	mov	ax,word ptr ss:[bp+20]
	mov	bx,word ptr ss:[bp+22]
	mov	cx,word ptr ss:[bp+24]
	mov	dx,word ptr ss:[bp+26]
	mov	si,ax
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
	add	bx,ax
	mov	ds,bx
	mov	di,cx
	shr	cx,1
	shr	cx,1
	shr	cx,1
	shr	cx,1
	add	dx,cx
	mov	es,dx
	assume	ds:NOTHING,es:NOTHING
	and	si,15
	and	di,15
	mov	word ptr ss:[bp-2],di
L$9:
	mov	al,byte ptr [si]
	inc	si
	cmp	al,129
	jb	L$14
	cmp	al,159
	jbe	L$13
L$14:
	cmp	al,224
	jb	L$12
	cmp	al,252
	ja	L$12
L$13:
	mov	ah,byte ptr [si]
	or	ah,ah
	jne	L$16
	jmp	L$15
L$16:
	xchg	ah,al
	inc	si
	call	_cx_get
	cmp	ax,cx
	jne	L$17
	jmp	L$10
L$12:
	xor	ah,ah
	cmp	al,42
	jne	L$18
L$19:
	push	es
	push	di
	push	ds
	push	si
	call far ptr	_WildCmp
	add	sp,8
	or	ax,ax
	jne	L$23
	jmp	L$22
L$23:
	cmp	ax,(-1)
	jne	L$24
	jmp	L$15
L$24:
	call	_cx_get
	or	cx,cx
	jne	L$19
	jmp short	L$17
L$18:
	cmp	al,63
	jne	L$25
	call	_cx_get
	jcxz	L$17
	jmp	L$26
L$25:
	cmp	al,94
	jne	L$27
	call	_ax_get
	or	ax,ax
	jne	L$28
	jmp	L$15
L$28:
	call	_cx_get
	cmp	ax,cx
	je	L$17
	jmp	L$26
L$27:
	cmp	al,91
	jne	L$29
	call	_cx_get
	jcxz	L$17
	call	_ax_get
	xor	dl,dl
	cmp	ax,94
	jne	L$30
	mov	dl,1
	call	_ax_get
L$30:
	mov	bx,65535
	jmp short	L$31
L$17:
	jmp short	L$32
L$33:
	cmp	ax,45
	jne	L$36
	call	_ax_get
	or	ax,ax
	je	L$15
	cmp	ax,93
	jne	L$37
	cmp	bx,cx
	je	L$38
	jmp short	L$39
L$37:
	cmp	cx,bx
	jb	L$40
	cmp	cx,ax
	jbe	L$38
L$40:
	jmp	L$41
L$36:
L$31:
	cmp	ax,cx
	jne	L$42
L$38:
	or	dl,dl
	jne	L$32
L$43:
	call	_ax_get
	or	ax,ax
	je	L$15
	cmp	ax,93
	jne	L$43
	jmp short	L$35
L$42:
L$41:
	mov	bx,ax
	call	_ax_get
	or	ax,ax
	je	L$15
	cmp	ax,93
	jne	L$46
L$39:
	or	dl,dl
	jne	L$35
	jmp short	L$32
L$46:
	jmp short	L$33
L$35:
	jmp	L$26
L$29:
	call	_cx_get
	cmp	ax,cx
	je	L$47
	or	ax,ax
	jne	L$32
	sub	di,word ptr ss:[bp-2]
	je	L$32
	mov	ax,di
	mov	dx,ax
	jmp short	L$48
L$47:
	jcxz	L$22
L$26:
L$10:
	jmp	L$9
L$22:
	xor	ax,ax
	sub	di,word ptr ss:[bp-2]
	mov	dx,di
	jmp short	L$48
L$15:
	mov	ax,(-1)
	jmp short	L$49
L$32:
	mov	ax,(-2)
L$49:
	xor	dx,dx
L$48:
	mov	sp,bp
	pop	bp
	popf
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	cx
	db	0CBh	;retf

_TEXT	ends
	end

;/*
;  WildCmp  -- シフトJIS 対応ワイルドカード機能付文字列比較
;	proc WildCmp(key:d, str:d)
;	入力 TC|not TC
;		key|bx.ax	ワイルドカード文字を含む文字列
;		str|dx.cx	文字列
;	出力
;		ax |ax		結果
;		dx |bx		マッチしたときの文字列の長さ
;
;	文字列 key と str を比較し、マッチすれば０をしなければ０以外を返す。
;	返される値の意味は、
;		0	  key と str はマッチした
;		-1	  key の指定がおかしい（ワイルドカード文字の指定ミスなど;)
;		-2	  str は key よりも文字列長が短い
;		正数n str は 先頭 n-1 バイトで key とマッチした（str は key より;長い）
;
;	str は極端に長すぎてはいけない。一応、63Kバイトくらいに対応しているはず。
;	（正確には64K - 17バイトくらいですが）
;
;	文字列の終端は'\0'. 文字は全角(2ﾊﾞｲﾄ)文字も１文字として扱われる.
;
;	ワイルド・カード文字の意味（c は 半角か全角文字の 1文字のこと）
;		*	  0文字以上の任意の文字列にマッチ.
;		?	  任意の1文字にマッチ.
;		^c	  c 以外の任意の１文字にマッチ.
;		[..]  [  ] でくくられた文字列（１文字以上）中のどれか１文字にマッチ.
;		[^..] [^ ] でくくられた文字列（１文字以上）中にない１文字にマッチ.
;
;　　括弧（[..],[^..]）のなかではワイルドカード文字の機能は抑制される。
;　　括弧のなかに']' を含めたい場合は、[]abc] や [^]ed] のように、'[' や '[^'
;	の直後に']'を置く。
;	括弧内では、A-Z のように,'-'を用いて文字コードで A から Z の範囲を指定で
;	きる。ただし、'-' の両側に文字がないといけない。[-abc] や [abc-] のよう
;	に,'[' や '[^' の直後や ']' の直前に '-' があるばあい,'-'は 1文字として
;	扱われる。[0-9-A] というようなのは不定扱い（実際にはこの版では[0-99-A]と
;	指定されたばあいと同じ動作をして、エラーにしていない）
;	あと'-' の右側の文字は左側の文字よりも大きいものを指定しなければならない。
;	Z-A ではマッチしない。
;*/

