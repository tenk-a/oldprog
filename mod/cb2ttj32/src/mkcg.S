! mkcg.S
! This program makes CG image files.
!  ank.cg:	0 - $ff
!  kanji.cg:	$2120 - $7c7f (except $xx00-$xx1f,$xx80-$xxff)
!  quarter.cg:	0 - $ff
! To compile:	
!	as86 -0 -o mkcg.o mkcg.S
!	ld86 -0 -s -o mkcg.com.tmp mkcg.o
!	dd if=mkcg.com.tmp of=mkcg.com bs=1 skip=288
!	rm mkcg.com.tmp mkcg.o


BUFFER_BYTES		= 32 + 2
DOS_INT			= 0x21

CR			= 0x0d
LF			= 0x0a
END_CHAR		= '$'

ANY			= 0


use16

.text		
.bss
.data

.align	0

.globl	_main

	.org	0x100
_main:
	mov	bx, #cg_ank
	call	one_cg
	mov	bx, #cg_kanji
	call	one_cg
	mov	bx, #cg_quarter
	call	one_cg
	mov	ax, #0x4c00
	int	#DOS_INT	; exit

	
! already must set cg_???? to bx
one_cg:
	mov	bx_orig, bx
	
	mov	ah, #0x3c
	lea	dx, [bx+6]
	mov	cx, #0
	int	#DOS_INT	; create handle
	jc	create_error_exit
	mov	handle, ax

	mov	dx, [bx+0]	; start code
	mov	si, [bx+2]	; end code
	mov	ax, [bx+4]
	mov	font_bytes, ax

read_font:
	mov	cx, font_bytes
	mov	di, #buffer+2
	mov	al, 0
	rep
	stosb			; clear buffer
	
	mov	ah, #0x14
	mov	bx, ds
	mov	cx, #buffer
	int	#0x18		; read font
	
	push	dx
	mov	bx, handle
	mov	cx, font_bytes	; len
	mov	dx, #buffer+2	; offset
	mov	ah, #0x40
	int	#DOS_INT	; write handle
	pop	dx
	jc	write_error_exit
		
	inc	dx
	cmp	dx, si
	ja	close_handle
	cmp	dh, #0x80	; ank?
	je	read_font
	cmp	dh, #0x00	; quarter?
	je	read_font
	cmp	dl, #0x80	; except $xx00-$xx1f and $xx80-$xxff
	jne	read_font
	inc	dh
	mov	dl, #0x20
	jmp	read_font

close_handle:
	mov	ah, #0x3e
	mov	bx, handle
	int	#DOS_INT	; close handle
	
	mov	bx, bx_orig
	lea	dx, [bx+6]
	call	print
	mov	dx, #mk_cg_msg
	call	print
	ret
	
print:	
	mov	ah, #0x09
	int	#DOS_INT	; print
	ret
create_error_exit:
	mov	dx, #create_error_msg
	jmp	error_exit
write_error_exit:
	mov	dx, #write_error_msg
	jmp	error_exit
error_exit:
	call	print
	mov	ax, #0x4c01
	int	#DOS_INT	; exit
		
handle:
		.word	ANY
bx_orig:
		.word	ANY
font_bytes:
		.word	ANY

mk_cg_msg:
		.ascii	" made."
		.byte	CR, LF, END_CHAR	
create_error_msg:	
		.ascii	"create handle error"
		.byte	CR, LF, END_CHAR
write_error_msg:	
		.ascii	"write handle error"
		.byte	CR, LF, END_CHAR
	
cg_ank:
	.word	0x8000		; start code
	.word	0x80ff		; end code
	.word	16		; font bytes
	.ascii	"ank.cg"	; filename
	.byte	0
	.byte	END_CHAR	; for print
cg_kanji:
	.word	0x2120		; start code
	.word	0x7c7f		; end code
	.word	32		; font bytes
	.ascii	"kanji.cg"	; filename
	.byte	0
	.byte	END_CHAR	; for print
cg_quarter:
	.word	0x0000		; start code
	.word	0x00ff		; end code
	.word	8		; font bytes
	.ascii	"quarter.cg"	; filename
	.byte	0
	.byte	END_CHAR	; for print

buffer:
	.space	BUFFER_BYTES

end
