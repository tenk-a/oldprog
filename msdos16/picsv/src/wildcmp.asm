DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
_BSS	ends

_TEXT	segment byte public 'CODE'
_ax_get	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	xor	ah,ah
	mov	al,byte ptr [si]
	inc	si
	cmp	al,081h
	jb	L$3
	cmp	al,09Fh
	jbe	L$2
L$3:
	cmp	al,0E0h
	jb	L$1
	cmp	al,0FCh
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
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	xor	ch,ch
	mov	cl,byte ptr es:[di]
	inc	di
	cmp	cl,081h
	jb	L$7
	cmp	cl,09Fh
	jbe	L$6
L$7:
	cmp	cl,0E0h
	jb	L$5
	cmp	cl,0FCh
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
_WildCmp	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
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
	mov	ax,word ptr [bp+18]
	mov	bx,word ptr [bp+20]
	mov	cx,word ptr [bp+22]
	mov	dx,word ptr [bp+24]
	mov	si,ax
	shr	ax,01h
	shr	ax,01h
	shr	ax,01h
	shr	ax,01h
	add	bx,ax
	mov	ds,bx
	mov	di,cx
	shr	cx,01h
	shr	cx,01h
	shr	cx,01h
	shr	cx,01h
	add	dx,cx
	mov	es,dx
	assume	ds:NOTHING,es:NOTHING
	and	si,0Fh
	and	di,0Fh
	mov	word ptr [bp-2],di
L$9:
	mov	al,byte ptr [si]
	inc	si
	cmp	al,081h
	jb	L$14
	cmp	al,09Fh
	jbe	L$13
L$14:
	cmp	al,0E0h
	jb	L$12
	cmp	al,0FCh
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
	cmp	al,02Ah
	jne	L$18
L$19:
	push	es
	push	di
	push	ds
	push	si
	call	_WildCmp
	add	sp,08h
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
	cmp	al,03Fh
	jne	L$25
	call	_cx_get
	jcxz	L$17
	jmp	L$26
L$25:
	cmp	al,05Bh
	jne	L$27
	call	_cx_get
	jcxz	L$17
	call	_ax_get
	xor	dl,dl
	cmp	ax,05Eh
	jne	L$28
	mov	dl,01h
	call	_ax_get
L$28:
	mov	bx,0FFFFh
	jmp short	L$29
L$17:
	jmp short	L$30
L$31:
	cmp	ax,02Dh
	jne	L$34
	call	_ax_get
	or	ax,ax
	je	L$15
	cmp	ax,05Dh
	jne	L$35
	cmp	bx,cx
	je	L$36
	jmp short	L$37
L$35:
	cmp	cx,bx
	jb	L$38
	cmp	cx,ax
	jbe	L$36
L$38:
	jmp	L$39
L$34:
L$29:
	cmp	ax,cx
	jne	L$40
L$36:
	or	dl,dl
	jne	L$30
L$41:
	call	_ax_get
	or	ax,ax
	je	L$15
	cmp	ax,05Dh
	jne	L$41
	jmp short	L$33
L$40:
L$39:
	mov	bx,ax
	call	_ax_get
	or	ax,ax
	je	L$15
	cmp	ax,05Dh
	jne	L$44
L$37:
	or	dl,dl
	jne	L$33
	jmp short	L$30
L$44:
	jmp short	L$31
L$33:
	jmp	L$26
L$27:
	call	_cx_get
	cmp	ax,cx
	je	L$45
	or	ax,ax
	jne	L$30
	sub	di,word ptr [bp-2]
	je	L$30
	mov	ax,di
	mov	dx,ax
	jmp short	L$46
L$45:
	jcxz	L$22
L$26:
L$10:
	jmp	L$9
L$22:
	xor	ax,ax
	sub	di,word ptr [bp-2]
	mov	dx,di
	jmp short	L$46
L$15:
	mov	ax,(-1)
	jmp short	L$47
L$30:
	mov	ax,(-2)
L$47:
	xor	dx,dx
L$46:
	mov	sp,bp
	pop	bp
	popf
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bx
	pop	cx
	ret

_TEXT	ends
	end
