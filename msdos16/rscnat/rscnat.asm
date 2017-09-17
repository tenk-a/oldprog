;----------------------------------------------------------------------------
; PC�݊��@�p RSCNT���ǂ�	RSCNAT v0.50
;----------------------------------------------------------------------------

	if 0
RCSPORT 	= 03FCh		; COM1
	else
RCSPORT 	= 02FCh		; COM2
	endif

BIOSnoDISK	= 013h
BIOSnoEMS	= 067h


_TEXT		segment WORD PUBLIC 'CODE'
		assume	cs:_TEXT
		assume	ds:NOTHING,es:NOTHING

;----------------------------------------------------------------------------
	even
StayBegin:	; .EXE�Ȃ̂ŁA���̃A�h���X��0000h�ɂȂ�̂ŁA���̂��Ƃ�
		; ���p���Ĉȉ��A�y�����Ă���^^;


STAYid	db	'RSCNAT'	;0-5

sav_r1	db	0		;6
sav_r2	db	0		;7

	dw	0		;8-9
EMM_ID:				;10
	db	'EMMXXXX0'

	even
;-----------;
DISKintr_new:
;-----------;
	pushf

	push	ax
	push	dx
		mov	dx,RCSPORT
		in	al,dx
		mov	cs:sav_r1,al
		and	al,11111101b
		out	dx,al
	pop	dx
	pop	ax

	;call	far ptr DISKbiosADDR
	db	9ah
  DISKbiosADDR:
	dw	0000h,0000h

	push	bp
	push	ax
	push	dx
		mov	bp,sp
		mov	dx,RCSPORT
		mov	al,cs:sav_r1
;		or	al,00000010b
		out	dx,al
	pop	dx
	pop	ax
	pop	bp
	iret

	even
;----------;
EMSintr_new:
;----------;
	pushf

	push	ax
	push	dx
		mov	dx,RCSPORT
		in	al,dx
		mov	cs:sav_r2,al
		and	al,11111101b
		out	dx,al
	pop	dx
	pop	ax

	;call	far ptr EMSbiosADDR
	db	9ah
  EMSbiosADDR:
	dw	0000h,0000h

	push	bp
	push	ax
	push	dx
		mov	bp,sp
		mov	dx,RCSPORT
		mov	al,cs:sav_r2
;		or	al,00000010b
		out	dx,al
	pop	dx
	pop	ax
	pop	bp
	iret

StayEnd:	; �풓���̏I���BStayEnd�̱��ڽ���풓���ނƂȂ�.


;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

	even
;-----;
PutStr:		; cs:dx ��asciiz�������W���o��
;-----;
	push	ax
	push	cx
	push	dx
	push	bx
	push	si
	push	ds

	push	cs
	pop	ds
	mov	si,dx
	mov	cx,-1
	LOOP_ME:
		inc	cx
		lodsb
		or	al,al
	jne	LOOP_ME
	mov	bx,1
	mov	ah,040h		; WRITE
	int	021h

	pop	ds
	pop	si
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	ret

	even
;------;
ChkStay:
;------;
	; out  ax:1=�풓��(YES)  0=�풓���Ă��Ȃ�(NO)  (zf:0=YES 1=NO)
	;      es|dx:�풓�����̃Z�O�����g
	
	; �܂� DISK-BIOS���荞�݂ւ̃t�b�N�����邩�`�F�b�N
	mov	ax,03500h+BIOSnoDISK
	int	021h
	cmp	bx,offset _TEXT:DISKintr_new
	jne	ChkStay_NON

	; EMS-BIOS�ւ̃t�b�N�����邩�`�F�b�N
	push	es
		mov	ax,03500h+BIOSnoEMS
		int	021h
	pop	dx
	cmp	bx,offset _TEXT:EMSintr_new
	jne	ChkStay_NON

	; ��̓�̊��荞�݂�����Z�O�����g�ɂ��邩�`�F�b�N
	mov	ax,es
	cmp	ax,dx
	jne	ChkStay_NON
	
	; �풓�� ID �̃`�F�b�N
	cmp	WORD ptr ES:[STAYid+0],'R'+'S'*256
	jne	ChkStay_NON
	cmp	WORD ptr ES:[STAYid+2],'C'+'N'*256
	jne	ChkStay_NON
	cmp	WORD ptr ES:[STAYid+4],'A'+'T'*256
	jne	ChkStay_NON

	xor	ax,ax
	inc	ax	;zf = 0
	ret

  ChkStay_NON:
	xor	ax,ax	;zf = 1
	ret



	even
;----;
Start:	; ��풓��...�v���O�����͂�������J�n���邼
;----;
	cld

	; ���ϐ��̈�̉��
	push	es
	pop	ds
	mov	es,word ptr es:[02Ch]	; PSP:0x2C  ���ϐ��̃Z�O�����g���i�[
	mov	ah,049h			; MemFree
	int	021h
	
	; �^�C�g���\��
	mov	dx,offset _TEXT:M_TITL
	call	PutStr

	; �����ײݵ�߼�݂̉��(�蔲��^^;)
	mov	si,129
	xor	cx,cx
	mov	cl,ds:[128]
	jcxz	ENDIF_start
		LOOP_start:
			lodsb
			cmp	al,'R'
			je	REMOVE
			cmp	al,'r'
			je	REMOVE
		loop LOOP_start
	ENDIF_start:

	jmp	STAY


;-----;
REMOVE:
;-----;
	; �풓�̗L�����`�F�b�N
	call	ChkStay
	je	ERR_RM

	;���łɏ풓���Ă�������Aremove����

	; DISK-BIOS�̕��A
	lds	dx,DWORD ptr es:[DISKbiosADDR]
	mov	ax,02500h+BIOSnoDISK
	int	021h

	; EMS-BIOS�̕��A
	lds	dx,DWORD ptr es:[EMSbiosADDR]
	mov	ax,02500h+BIOSnoEMS
	int	021h

	; �풓���������J��
	mov	ax,es
	mov	es,ax
	mov	ah,049h		; MemFree
	int	021h

	; ���b�Z�[�W�\��&�I��
	mov	dx,offset _TEXT:M_REMV
	jmp	MSG_EXIT

ERR_RM:	; �풓���ĂȂ�����
	mov	dx,offset _TEXT:M_ER_R
	jmp	MSG_EXIT


;---;
STAY:	; �풓����ꍇ
;---;
	; �풓�̗L�����`�F�b�N
	call	ChkStay
	je ENDIF_stay
		; ���łɏ풓���Ă���
		mov	dx,offset _TEXT:M_ER_S
		jmp	MSG_EXIT
	ENDIF_stay:

	; �풓���̂��߂̃��������m�ۂ���
	mov	bx,offset _TEXT:StayEnd+15
	shr	bx,1
	shr	bx,1
	shr	bx,1
	shr	bx,1
	mov	cx,bx
	shl	cx,1
	shl	cx,1
	shl	cx,1
	;shl	cx,1		; �풓���ւ̺�߰�̶���
	mov	ah,048h		; MemAlloc
	int	021h
	jc	ERR_MM

	; �풓ٰ�݂��߰
	mov	es,ax		; es = �m�ۂ������ڽ(SEG)
	push	cs
	pop	ds
	xor	si,si
	mov	di,si
	rep	movsw

	; DISK-BIOS�̐ݒ�
	mov	ds,ax		; ds = �풓����
	mov	ax,03500h+BIOSnoDISK
	int	021h
	mov	WORD ptr ds:[DISKbiosADDR+0],bx
	mov	WORD ptr ds:[DISKbiosADDR+2],es
	mov	dx,offset _TEXT:DISKintr_new
	mov	ax,02500h+BIOSnoDISK
	int	021h

	; EMS-BIOS�̐ݒ�
	mov	ax,03500h+BIOSnoEMS
	int	021h
	mov	WORD ptr ds:[EMSbiosADDR+0],bx
	mov	WORD ptr ds:[EMSbiosADDR+2],es
	mov	dx,offset _TEXT:EMSintr_new
	mov	ax,02500h+BIOSnoEMS
	int	021h

	; EMM-ID ���R�s�[
	mov	dx,ds
	mov	ax,es
	mov	ds,ax
	mov	es,dx
	mov	si,offset _TEXT:EMM_ID
	mov	di,si
	mov	cx,4
	cld
	rep	movsw

	; MCB�̒���
	mov	ax,es
	dec	ax
	mov	ds,ax
	inc	ax
	mov	WORD ptr ds:[1],ax

	; �ꉞ���O��MCB�ɐݒ�
	mov	WORD ptr ds:[ 8],'R'+'S'*256
	mov	WORD ptr ds:[10],'C'+'N'*256
	mov	WORD ptr ds:[12],'A'+'T'*256
	mov	WORD ptr ds:[14],0


	; ���b�Z�[�W�\�����I��
	mov	dx,offset _TEXT:M_STAY
	jmp	MSG_EXIT


ERR_MM:
	mov	dx,offset _TEXT:M_ER_M
	jmp	MSG_EXIT


;-----------------------------------------------
;-------;
MSG_EXIT:	; ���b�Z�[�W��\�����ďI������
;-------;
	; ���b�Z�[�W�\��
	call	PutStr

	; �I��
	mov	ax,04c00h
	int	021h

;----------------------------------------------------------
M_TITL: db	'RSCNAT ver0.50   by TENK*',0Dh,0Ah,0
M_REMV: db	'Removed. ',0Dh,0Ah,0
M_STAY: db	'Resident.',0Dh,0Ah,0
M_ER_R: db	'Not found.',0Dh,0Ah,0
M_ER_S: db	'Already resided.',0Dh,0Ah,0
M_ER_M: db	'Not enough memory.',0Dh,0Ah,0


_TEXT	ends


;----------------------------------------------------------------------------
_STACK	segment STACK 'STACK'
	dw	64
	ends

;----------------------------------------------------------------------------
	end	Start
