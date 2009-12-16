; m.ANA    1 :	module Hello
; m.ANA    2 :		stack 0x1000
; m.ANA    3 :		start main
; m.ANA    4 :	
; m.ANA    5 :	@define EXIT(er)	ax = 0x4c00+er;intr 0x21
DGROUP	group	_DATA,_BSS,_STACK

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
_BSS	ends

_STACK	segment para stack 'STACK'
	db	4096 dup(?)
; m.ANA    6 :	
; m.ANA    7 :	proc allocmem(bx)
; m.ANA    8 :	macro
; m.ANA    9 :		ah = 0x48
; m.ANA   10 :		intr 0x21
; m.ANA   11 :	endproc
; m.ANA   12 :	
; m.ANA   13 :	proc main()
; m.ANA   14 :	begin
; m.ANA   15 :		ds = ax = %var
_STACK	ends

_TEXT	segment byte public 'CODE'
Hello@main	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	ax,DGROUP
	mov	ds,ax
; m.ANA   16 :		bx = 0x400
	mov	bx,0400h
; m.ANA   17 :		allocmem bx
	mov	ah,048h
	int	021h
; m.ANA   18 :		intr 0xf0
	int	0F0h
; m.ANA   19 :		EXIT (0)
	mov	ax,04C00h
	int	021h
; m.ANA   20 :	endproc

; m.ANA   21 :	
; m.ANA   22 :	endmodule
_TEXT	ends
	end	Hello@main
