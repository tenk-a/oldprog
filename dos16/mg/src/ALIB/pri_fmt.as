;このルーチンは NIFTY FLABO にテディー松本氏がアップされていた
;tprintf.asm です。
;	002/103   NAF00230  テディー松本      printfコンパチ関数(ISH 129L)
;	(11)   90/12/10 01:23
;ただし、呼び出す名前を _Pri_Fmt に変名しています。
;かってに使ってよかったのかわかりませんが、まあ、フリーウェアなら大丈夫だろう
;とアマい考えで利用させていただきました。
;************************************
;  Library module : _Pri_Fmt,(_p)
;   (tiny printf function for C)
;  %[-][+][0][n][l/F][c/s/d/u/x/X]
;  ------ by Teddy Matsumoto ------
;************************************
	PUBLIC	_Pri_Fmt
	PUBLIC	_Pri_FmtDS
	PUBLIC	_Pri_FmtS
	PUBLIC	_Pri_FmtHdl
;---- macros ----
PUSHM	MACRO	REGS
  IRP REG,<REGS>
	PUSH	REG
  ENDM
	ENDM
POPM	MACRO	REGS
  IRP REG,<REGS>
	POP	REG
  ENDM
	ENDM
JMPS	MACRO	LBL
	JMP	SHORT LBL
	ENDM
;---- segment definition ----
_TEXT	SEGMENT	BYTE PUBLIC 'CODE'
	assume	cs:_TEXT
_TEXT	ENDS
_DATA	SEGMENT	WORD PUBLIC 'DATA'
_DATA	ENDS

_TEXT	SEGMENT	BYTE PUBLIC 'CODE'
	ASSUME	CS:_TEXT,DS:_TEXT

_Pri_FmtHdl:
	PUSH	BP
	MOV	BP,SP
	PUSH	AX
	MOV	AX, WORD PTR [BP+2+2]
	MOV	CS:HDL,AX
	POP	AX
	POP	BP
	RET

HDL	DW	0

PUTCH:
	CMP	CS:HDL,0
	JNE	PUTCH_HDL
	;MOV	DL,AL
	;MOV	AH,6
	INT	21H
	RET
PUTCH_HDL:
	PUSH	AX
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH    DS
	MOV	BX,CS
	MOV	DS,BX
	MOV	BX,HDL
	MOV     _TEXT:PUTCH_BUF1, AL
	MOV	AH,40H
	MOV	DX,OFFSET _TEXT:PUTCH_BUF1
	MOV	CX,1
	INT	21H
	POP	DS
	POP	DX
	POP	CX
	POP	BX
	POP	AX
	RET
PUTCH_BUF1 DB  0


;**** tprintf ****
_Pri_Fmt:
_Pri_FmtDS:
	PUSH	BP
	MOV	BP,SP
	PUSHM	<ES,DS,DI,SI,BX,DX,CX,AX>
	MOV	AX,_DATA
	MOV	DS,AX
	JMP	SHORT JJJ1
;
_Pri_FmtS:
	PUSH	BP
	MOV	BP,SP
	PUSHM	<ES,DS,DI,SI,BX,DX,CX,AX>
JJJ1:
	  MOV	SI,[BP+4]
	if 0
	  OR	SI,SI		;if fmtstr=0 disable print
	  JZ	SETFLAG
	  CMP	SI,1		;if fmtstr=1 enable print (default)
	  JZ	SETFLAG
	  CMP	BYTE PTR CS:PRNFLAG,0
	  JZ	EXIT
	endif
	ADD	BP,+6		;for get args (1st arg address)
	MOV	DH,1		;DH = % sequence flag (1:enable)
	CALL	PRNSTR
EXIT:	POPM	<AX,CX,DX,BX,SI,DI,DS,ES>
	POP	BP
	RET
SETFLAG:MOV	CS:PRNFLAG,SI
	JMPS	EXIT
PRNFLAG	DW	1		;default: enable print
;---- print AL CX times ----
PRNCX:	MOV	DL,AL
	MOV	AH,6
PRNCX1:	CALL	PUTCH	;INT	21H
	LOOP	PRNCX1
	RET
;---- main loop ----
CRLF:	MOV	DL,0Dh		;print CRLF (AH must be 2)
	CALL	PUTCH	;INT	21h
	MOV	AL,0Ah
MLP:	MOV	DL,AL		;print AL
	CALL	PUTCH	;INT	21h
PRNSTR:	MOV	AH,6		;function call 6: not do TAB,CR
	LODSB			;get ch from fmtstr
	CMP	AL,'%'
	JZ	PCENT
PSTR1:	CMP	AL,' '		;if AL>=' ' print AL
	JAE	MLP
	;MOV	AH,2		;function call 2: do TAB,CR
	;CMP	AL,0Ah		;C compiler converts \n to 0Ah (no 0Dh)
	;JZ	CRLF
	OR	AL,AL		;check end of string
	JNZ	MLP
	RET
;---- '%' sequence ----
PCENT:	OR	DH,DH		;if DH!=0,ignore % sequence
	JZ	MLP
	LODSB
	CMP	AL,'%'		;%% is % own
	JBE	PSTR1
;---- parameter process ----
	MOV	BX,2000h	;BH=flag(b8:long,b5:zero,b3:left,b2:sign)
	JMPS	PARAM		;BL=length 
;---- check '-','+' ----
PMIPL:	AND	AL,0110b	;bit(3,2):'-'10,'+'01
	OR	BH,AL
	LODSB
PARAM:	CMP	AL,'-'
	JZ	PMIPL
	CMP	AL,'+'
	JZ	PMIPL
;---- check '0' ----
PZERO:	CMP	AL,'0'
	JNZ	PNUM
	OR	BH,10h		;set 0 flag
PNUM0:	LODSB
;---- check n ----
PNUM:	CMP	AL,'0'
	JB	PALPHA
	CMP	AL,'9'
	JA	PALPHA
	SUB	AL,'0'
	MOV	DL,AL
	MOV	AL,10
	MUL	BL
	ADD	AL,DL
	MOV	BL,AL
	JMPS	PNUM0
;---- get parameter value ----
PALPHA:	XOR	DX,DX
	MOV	DI,[BP]		;DI = arg low value
	INC	BP
	INC	BP
;---- check 'l','F' ----
	CMP	AL,'l'
	JZ	LONG
	CMP	AL,'F'
	JNZ	PCHAR
LONG:	MOV	DX,[BP]		;DX = arg high value
	INC	BP
	INC	BP
	OR	BH,80h		;set long flag
	LODSB
;---- case 'c' ----
PCHAR:	CMP	AL,'c'
	JNZ	PVAL
	MOV	AX,DI
	MOV	AH,2
	MOV	DH,1		;enable % sequence
	JMPS	MLP
;---- case 'X','x','d','u','s' ----
PVAL:	PUSHM	<DS,SI>
	CALL	VALSTR
;---- print converted string ----
PRNVAL:	XOR	DH,DH		;disable % sequence
	MOV	AL,BH
	XOR	BH,BH		;BX = formatted space
	SUB	BX,CX
	JBE	PVAL_G
	MOV	CX,BX		;CX = over space
	TEST	AL,100b		;test left flag
	JZ	PVAL_R
;---- case print left side ----
	CALL	PRNSTR
	MOV	AL,' '
	CALL	PRNCX
	JMPS	PVAL_E
;---- case print right side ----
PVAL_R:	AND	AL,30h		;' ' or '0'
	CALL	PRNCX
PVAL_G:	CALL	PRNSTR
;---- end of print val ----
PVAL_E:	POP	SI
BADFMT:	POP	DS
	MOV	DH,1		;enable % sequence
	JMP	PRNSTR

;*******************************************
;  get value string (return DS:SI,CX=size)
;*******************************************
VALSTR:	CMP	AL,'s'
	JZ	VAL_S
	PUSH	CS		;DS=CS
	POP	DS
	MOV	SI,OFFSET BUF+12
	CMP	AL,'X'
	JZ	VAL_X
	CMP	AL,'x'
	JZ	VAL_X
	CMP	AL,'d'
	JZ	VAL_D
	CMP	AL,'u'
	JZ	VAL_U
	POPM	<DI,SI>		;stack adjust
	DEC	SI		;for print bad ch
	JMPS	BADFMT
;****** case 's' ******
;---- get DS:SI ----
VAL_S:	OR	BH,BH
	JNS	VAL_S1
	MOV	DS,DX
VAL_S1:	MOV	SI,DI
;---- get strlen ----
	XOR	CX,CX
VAL_S2:	LODSB
	OR	AL,AL
	LOOPNZ	VAL_S2
	NOT	CX
	MOV	SI,DI
	JMPS	VAL_D3
;****** case 'd' ******
VAL_D:	MOV	AX,DI
;---- if short to signed long ----
	OR	BH,BH
	JS	VAL_D0
	CWD
;---- if minus to abs value ----
VAL_D0:	OR	DH,DH
	PUSHF
	JNS	VAL_D1
	NEG	AX
	ADC	DX,+0
	NEG	DX
;---- get string ----
VAL_D1:	CALL	VAL_U0
	POPF
;---- add '-','+' if necessary ---
	MOV	AL,'-'
	JS	VAL_D2
	TEST	BH,10B		;test sign flag
	JZ	VAL_D4
	MOV	AL,'+'
VAL_D2:	DEC	SI
	MOV	[SI],AL
	INC	CX
VAL_D3:	AND	BH,0EFH		;clear zero flag
VAL_D4:	RET
;****** case 'X','x' ******
VAL_X:	SUB	AL,'X'-7
	MOV	[SI-12],AL	;set to upper/lower
	MOV	AX,DI
;---- to HEX ----
HEXSTR:	PUSH	DX
	MOV	DI,10H
	MOV	CX,4
	XOR	DX,DX
	CALL	STDIGIT
	POP	AX
	CALL	STDIGIT
	JMPS	VAL_E
;****** case 'u' ******
VAL_U:	MOV	AX,DI


;---- DX.AX div 100000 ----
VAL_U0:	MOV	DI,50000
	SHR	DX,1		;not to overflow
	RCR	AX,1
	PUSHF
	DIV	DI
	XCHG	AX,DX
	POPF
	PUSH	DX		;keep DX.AX/100000
	MOV	DX,0
	RCL	AX,1
	RCL	DX,1
;---- get decimal value ----
	MOV	DI,10
	MOV	CX,5
	CALL	STDIGIT
	POP	AX
	CALL	STDIGIT
;---- get strlen ----
VAL_E:	ADD	CX,CX
VAL_E1:	CMP	BYTE PTR [SI],'0'
	JNZ	VAL_E2
	INC	SI
	DEC	CX
	JNZ	VAL_E1
	DEC	SI
	INC	CX
VAL_E2:	RET

;****** store digits ******
STDIGIT:PUSH	CX
STDIG1:	DIV	DI
	ADD	DL,'0'
	CMP	DL,'9'
	JBE	STDIG2
	ADD	DL,CS:BUF
STDIG2:	DEC	SI
	MOV	[SI],DL
	XOR	DX,DX
	LOOP	STDIG1
	POP	CX
	RET
;****** string buffer ******
BUF	DB	13 DUP(0)

_TEXT	ENDS
	END
