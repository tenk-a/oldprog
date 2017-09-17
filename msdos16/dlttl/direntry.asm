DGROUP	group	_DATA,_BSS

DirEntry_TEXT	segment byte public 'CODE'
	assume	cs:DirEntry_TEXT
	assume	ds:DGROUP,es:NOTHING
DirEntry_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
_BSS	ends

_STACK	segment para stack 'STACK'
	db	512 dup(?)
_STACK	ends

_BSS	segment word public 'BSS'
	public	_DirEntrydta
_DirEntrydta	label	byte
	db	43 dup(?)
	public	_DirEntrykey
_DirEntrykey	label	word
	db	2 dup(?)
	public	_DirEntrydir
_DirEntrydir	label	byte
	db	200 dup(?)
_BSS	ends

_DATA	segment word public 'DATA'
L$1	label	byte
	db	1 dup(?)
_DATA	ends

DirEntry_TEXT	segment byte public 'CODE'
	public	_DirEntryGet
_DirEntryGet	label	far
	assume	ds:DGROUP,es:NOTHING
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
	mov	word ptr ss:[bp-4],bx
	mov	word ptr ss:[bp-2],es
	mov	si,DGROUP
	mov	ds,si
	mov	dx,offset DGROUP:_DirEntrydta
	mov	ah,26
	int	33
	mov	dx,word ptr ss:[bp+28]
	mov	cx,word ptr ss:[bp+26]
	mov	bx,word ptr ss:[bp+24]
	mov	ax,word ptr ss:[bp+22]
	or	bx,bx
	jne	L$3
	or	ax,ax
	jne	L$3
	jmp	L$2
L$3:
	or	dx,dx
	jne	L$4
	or	cx,cx
	jne	L$4
	cmp	byte ptr [L$1],0
	je	L$6
	jmp	L$5
L$6:
	mov	ax,1
	jmp	L$7
L$4:
	mov	byte ptr [L$1],0
	mov	si,cx
	and	si,15
	shr	cx,1
	shr	cx,1
	shr	cx,1
	shr	cx,1
	add	dx,cx
	mov	es,dx
	mov	bx,si
	mov	dx,offset DGROUP:_DirEntrydir
	mov	di,dx
L$8:
	mov	al,byte ptr es:[si]
	inc	si
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
	mov	byte ptr [di],al
	inc	di
	mov	al,byte ptr es:[si]
	inc	si
	or	al,al
	jne	L$14
	jmp	L$2
L$14:
	jmp	L$15
L$11:
	cmp	al,97
	jb	L$16
	cmp	al,122
	ja	L$16
	sub	al,32
	jmp	L$15
L$16:
	cmp	al,47
	jne	L$17
	mov	al,92
L$17:
	cmp	al,92
	je	L$19
	cmp	al,58
	jne	L$18
L$19:
	mov	byte ptr es:[si-1],al
	mov	dx,di
	inc	dx
	mov	bx,si
L$18:
L$15:
	mov	byte ptr [di],al
	inc	di
	or	al,al
	jne	L$8
	mov	di,dx
	mov	byte ptr [di],42
	inc	di
	mov	byte ptr [di],46
	inc	di
	mov	byte ptr [di],42
	inc	di
	mov	byte ptr [di],0
	inc	di
	mov	word ptr [_DirEntrykey],di
	mov	byte ptr ss:[bp-5],0
L$20:
	mov	al,byte ptr es:[bx]
	inc	bx
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
	mov	byte ptr [di],al
	inc	di
	mov	al,byte ptr es:[bx]
	inc	bx
	or	al,al
	jne	L$26
	jmp	L$2
L$26:
	jmp	L$27
L$23:
	cmp	al,97
	jb	L$28
	cmp	al,122
	ja	L$28
	sub	al,32
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
	mov	byte ptr [L$1],1
	jmp	L$27
L$29:
	cmp	al,46
	jne	L$31
	cmp	byte ptr es:[bx],0
	jne	L$31
	mov	byte ptr ss:[bp-5],1
L$31:
L$27:
	mov	byte ptr [di],al
	inc	di
	or	al,al
	jne	L$20
	mov	dx,offset DGROUP:_DirEntrydir
	mov	cx,word ptr ss:[bp+30]
	mov	ah,78
	int	33
	jnc	L$33
	jmp	L$32
L$33:
L$34:
	mov	si,offset DGROUP:_DirEntrydta+30
	xor	di,di
L$37:
	lodsb
	or	al,al
	jne	L$40
	mov	di,si
	dec	di
	mov	byte ptr [si],0
	cmp	byte ptr ss:[bp-5],0
	je	L$39
	mov	byte ptr [di],46
	jmp short	L$39
L$40:
	cmp	al,46
	jne	L$37
L$39:
	mov	cx,offset DGROUP:_DirEntrydta+30
	push	ds
	push	cx
	push	ds
	push	word ptr [_DirEntrykey]
	call far ptr	_WildCmp
	add	sp,8
	or	di,di
	je	L$41
	mov	byte ptr [di],0
L$41:
	or	ax,ax
	je	L$36
L$5:
	mov	dx,offset DGROUP:_DirEntrydir
	mov	cx,word ptr ss:[bp+30]
	mov	ah,79
	int	33
	jc	L$32
	jmp short	L$34
L$36:
	mov	bx,word ptr ss:[bp+24]
	mov	ax,word ptr ss:[bp+22]
	mov	di,ax
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
	add	bx,ax
	mov	es,bx
	and	di,15
	mov	si,offset DGROUP:_DirEntrydir
L$42:
	lodsb
	stosb
	or	al,al
	jne	L$42
	sub	di,4
	mov	si,offset DGROUP:_DirEntrydta+30
L$45:
	lodsb
	stosb
	or	al,al
	jne	L$45
	xor	ax,ax
	mov	byte ptr es:[di],al
	jmp short	L$7
L$2:
	mov	ax,(-1)
	jmp short	L$7
L$32:
	cmp	ax,18
	jne	L$48
	mov	ax,1
L$48:
L$7:
	mov	word ptr [bp+16],ax
	lds	dx,dword ptr ss:[bp-4]
	mov	ah,26
	int	33
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
	db	0CBh	;retf

DirEntry_TEXT	ends
extrn _WildCmp:far
	end
;int DirEntryGet(byte far *pRslt, byte far *pKey, word pFlg)
;//	ワイルド・カード機能付ファイル名のディレクトリ・エントリを得る
;//  ワイルド・カード指定で継続のファイル名を得るときは pKey = NULL
;//	で呼び出す
;//	arg pRslt	見つかったパス名を格納する場所 要128バイト以上
;//	arg pKey	ワイルドカード機能付パス名 128バイト以内
;//	arg pFlg    ディレクトリ・エントリ検索で対象とするファイル属性
;//					0x01(bit 0)	読出専用ファイル
;//					0x02(bit 1)	不可視属性
;//					0x04(bit 2)	システム・ファイル
;//					0x08(bit 3)	ボリュームラベル
;//					0x10(bit 4) ディレクトリ
;//					0x20(bit 5) 保存ビット
;//	out ax		0:エラー無  -1:設定誤り 1:これ以上ファイルがない
;//				2:ファイルが無い
