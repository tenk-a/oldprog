;	// ana86 L1 v0.15用ソース
;	module RPal
;		c_name "RPal","",1
;	
;	*struct rpal_t
DGROUP	group	_DATA,_BSS,_TEXT

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
;		id:b(10)
;		tone:b
;		rsv:b(5)
;		grb:b(48)
;	endstruct
;	
;	struct mcb_t
;	    flg:b
;	    owner:w
;	    blksiz:w
;	    reserve:b(11)
;	endstruct
;	
;	*proc Search()
;	//	out dx.ax : pointer to rpal_t
;	begin
;		save bx,cx,si,di,es
;		var rpalID:b() = "pal98 grb"
_BSS	ends

_DATA	segment word public 'DATA'
L$1	label	byte
	db	112,97,108,57,56,32,103,114,98
	db	1 dup(0)
;		
_DATA	ends

_TEXT	segment byte public 'CODE'
	public	_RPalSearch
_RPalSearch	label	near
	push	cx
	push	bx
	push	si
	push	di
	push	es
;		ah = 0x52
	mov	ah,82
;		intr 0x21
	int	33
;		es = dx = w[es:bx-2]
	mov	dx,word ptr es:[bx-2]
	mov	es,dx
;		rep.inc
	cld
;		loop (dx && mcb_t[es:0].flg != 'Z')
	jmp short	L$3
L$2:
;			dx = es
	mov	dx,es
;			++dx
	inc	dx
;			if (mcb_t[es:0].owner != 0)
	cmp	word ptr es:[1],0
	je	L$5
;				di = 0x10
	mov	di,16
;				cx = 10
	mov	cx,10
;				si = &rpalID
	mov	si,offset DGROUP:L$1
;				rep.cmp di,si,cx
	rep	cmpsb
;				go (==) FIND
	je	L$6
;			fi
L$5:
;			dx += mcb_t[es:0].blksiz
	add	dx,word ptr es:[3]
;			es = dx
	mov	es,dx
;		endloop
L$3:
	or	dx,dx
	je	L$7
	cmp	byte ptr es:[0],90
	jne	L$2
L$7:
;		|dx = 0
	mov	dx,0
;	FIND:
L$6:
;		ax = 0
	xor	ax,ax
;		return
	pop	es
	pop	di
	pop	si
	pop	bx
	pop	cx
	ret
;	endproc

;	
;	endmodule
_TEXT	ends
	end
