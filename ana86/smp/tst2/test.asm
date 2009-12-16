;   1 : 
;   2 : extern var testAs:w
	.186
DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
;   3 : extern proc Xy();endproc
;   4 : 
;   5 : proc test(ax,t1:w,t2:w,t3:w)
;   6 : enter
;   7 : 	testAs <<= 1
_BSS	ends

_TEXT	segment byte public 'CODE'
test	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	push	bp
	mov	bp,sp
	shl	word ptr [testAs],01h
;   8 : 	ax >>= 1
	shr	ax,01h
;   9 : 	ax .>>= 1
	sar	ax,01h
;  10 : 	ax <<=. 1
	rcl	ax,01h
;  11 : 	ax >>=. 1
	rcr	ax,01h
;  12 : 	ax <<<= 1
	rol	ax,01h
;  13 : 	ax >>>= 1
	ror	ax,01h
;  14 : 	ax +=. 1
	adc	ax,01h
;  15 : 	ax -=. 4
	sbb	ax,04h
;  16 : 	if ((cx >>= 1) _o_ 0)
	shr	cx,01h
	jo	L$1
;  17 : 		dx.ax = cx * 40
	mov	ax,028h
	mul	cx
;  18 : 	fi
L$1:
;  19 : 	go (_c_) L1
	jc	L$2
;  20 : 	go (_s_) L1
	js	L$2
;  21 : 	go (_z_) L1
	jz	L$2
;  22 : 	go (_p_) L1
	jp	L$2
;  23 : 	go (_o_) L1
	jo	L$2
;  24 : 	go (cf) L1
	jc	L$2
;  25 : 	go (sf) L1
	js	L$2
;  26 : 	go (zf) L1
	jz	L$2
;  27 : 	go (pf) L1
	jp	L$2
;  28 : 	go (ovf) L1
	jo	L$2
;  29 : 	clc
	clc
;  30 : 	stc
	stc
;  31 : 	cmc
	cmc
;  32 : 	Xy
	call	Xy
;  33 :  L1:
L$2:
;  34 : 	return
	db	0C9h	;leave
	ret	06h
;  35 : endproc

_TEXT	ends
extrn Xy:near
extrn testAs:word
	end
