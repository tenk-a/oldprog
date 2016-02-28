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
_STACK	ends

_BSS	segment word public 'BSS'
	public	_dirEntrydta
_dirEntrydta	label	byte
	db	43 dup(?)
	public	_dirEntrykey
_dirEntrykey	label	word
	db	2 dup(?)
	public	_dirEntrydir
_dirEntrydir	label	byte
	db	200 dup(?)
_BSS	ends

_DATA	segment word public 'DATA'
	public	_dirEntrywcFlg
_dirEntrywcFlg	label	byte
	db	1 dup(0)
L$1	label	byte
	db	1 dup(0)
_DATA	ends

_TEXT	segment byte public 'CODE'
	public	_dirEntryGet
_dirEntryGet	label	near
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
	sub	sp,06h
	mov	ah,02Fh
	int	021h
	mov	word ptr [bp-4],bx
	mov	word ptr [bp-2],es
	mov	si,DGROUP
	mov	ds,si
	mov	dx,offset DGROUP:_dirEntrydta
	mov	ah,01Ah
	int	021h
	mov	dx,word ptr [bp+26]
	mov	cx,word ptr [bp+24]
	mov	bx,word ptr [bp+22]
	mov	ax,word ptr [bp+20]
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
	cmp	byte ptr [_dirEntrywcFlg],00h
	je	L$6
	jmp	L$5
L$6:
	mov	ax,01h
	jmp	L$7
L$4:
	mov	byte ptr [L$1],00h
	mov	byte ptr [_dirEntrywcFlg],00h
	mov	si,cx
	and	si,0Fh
	shr	cx,01h
	shr	cx,01h
	shr	cx,01h
	shr	cx,01h
	add	dx,cx
	mov	es,dx
	mov	bx,si
	mov	dx,offset DGROUP:_dirEntrydir
	mov	di,dx
L$8:
	mov	al,byte ptr es:[si]
	inc	si
	cmp	al,081h
	jb	L$13
	cmp	al,09Fh
	jbe	L$12
L$13:
	cmp	al,0E0h
	jb	L$11
	cmp	al,0FCh
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
	cmp	al,061h
	jb	L$16
	cmp	al,07Ah
	ja	L$16
	sub	al,020h
	jmp	L$15
L$16:
	cmp	al,02Fh
	jne	L$17
	mov	al,05Ch
L$17:
	cmp	al,05Ch
	je	L$19
	cmp	al,03Ah
	jne	L$18
L$19:
	mov	byte ptr es:[si-01h],al
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
	mov	byte ptr [di],02Ah
	inc	di
	mov	byte ptr [di],02Eh
	inc	di
	mov	byte ptr [di],02Ah
	inc	di
	mov	byte ptr [di],00h
	inc	di
	mov	word ptr [_dirEntrykey],di
	mov	byte ptr [bp-5],00h
L$20:
	mov	al,byte ptr es:[bx]
	inc	bx
	cmp	al,081h
	jb	L$25
	cmp	al,09Fh
	jbe	L$24
L$25:
	cmp	al,0E0h
	jb	L$23
	cmp	al,0FCh
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
	cmp	al,061h
	jb	L$28
	cmp	al,07Ah
	ja	L$28
	sub	al,020h
	jmp	L$27
L$28:
	cmp	al,02Ah
	je	L$30
	cmp	al,03Fh
	je	L$30
	cmp	al,05Bh
	jne	L$29
L$30:
	mov	byte ptr [_dirEntrywcFlg],01h
	jmp	L$27
L$29:
	cmp	al,02Eh
	jne	L$31
	cmp	byte ptr es:[bx],00h
	jne	L$31
	mov	byte ptr [bp-5],01h
L$31:
L$27:
	mov	byte ptr [di],al
	inc	di
	or	al,al
	jne	L$20
	mov	dx,offset DGROUP:_dirEntrydir
	mov	cx,word ptr [bp+28]
	mov	ah,04Eh
	int	021h
	jnc	L$32
	jmp	L$33
L$32:
L$34:
	mov	si,offset DGROUP:_dirEntrydta+30
	xor	di,di
L$37:
	lodsb
	or	al,al
	jne	L$40
	mov	di,si
	dec	di
	mov	byte ptr [si],00h
	cmp	byte ptr [bp-5],00h
	je	L$39
	mov	byte ptr [di],02Eh
	jmp short	L$39
L$40:
	cmp	al,02Eh
	jne	L$37
L$39:
	mov	cx,offset DGROUP:_dirEntrydta+30
	push	ds
	push	cx
	push	ds
	push	word ptr [_dirEntrykey]
	call	_wildCmp
	add	sp,08h
	or	di,di
	je	L$41
	mov	byte ptr [di],00h
L$41:
	or	ax,ax
	je	L$36
L$5:
	mov	dx,offset DGROUP:_dirEntrydir
	mov	cx,word ptr [bp+28]
	mov	ah,04Fh
	int	021h
	jc	L$33
	jmp short	L$34
L$36:
	mov	bx,word ptr [bp+22]
	mov	ax,word ptr [bp+20]
	mov	di,ax
	shr	ax,01h
	shr	ax,01h
	shr	ax,01h
	shr	ax,01h
	add	bx,ax
	mov	es,bx
	and	di,0Fh
	mov	si,offset DGROUP:_dirEntrydir
L$42:
	lodsb
	stosb
	or	al,al
	jne	L$42
	sub	di,04h
	mov	si,offset DGROUP:_dirEntrydta+30
L$45:
	lodsb
	stosb
	or	al,al
	jne	L$45
	xor	ax,ax
	mov	byte ptr es:[di],al
	mov	byte ptr [L$1],01h
	jmp short	L$7
L$2:
	mov	ax,(-1)
	jmp short	L$7
L$33:
	cmp	ax,012h
	jne	L$48
	mov	ax,02h
	cmp	byte ptr [L$1],00h
	je	L$49
	dec	ax
L$49:
L$48:
L$7:
	mov	word ptr [bp+16],ax
	lds	dx,dword ptr [bp-4]
	mov	ah,01Ah
	int	021h
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

_TEXT	ends
extrn _wildCmp:near
extrn _wildCmp:near
	end
