;----------------------------------------------------------------------------
; PC互換機用 RSCNTもどき	RSCNAT v0.50
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
StayBegin:	; .EXEなので、このアドレスは0000hになるので、このことを
		; 利用して以下、楽をしている^^;


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

StayEnd:	; 常駐部の終わり。StayEndのｱﾄﾞﾚｽが常駐ｻｲｽﾞとなる.


;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

	even
;-----;
PutStr:		; cs:dx のasciiz文字列を標準出力
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
	; out  ax:1=常駐済(YES)  0=常駐していない(NO)  (zf:0=YES 1=NO)
	;      es|dx:常駐部分のセグメント
	
	; まず DISK-BIOS割り込みへのフックがあるかチェック
	mov	ax,03500h+BIOSnoDISK
	int	021h
	cmp	bx,offset _TEXT:DISKintr_new
	jne	ChkStay_NON

	; EMS-BIOSへのフックがあるかチェック
	push	es
		mov	ax,03500h+BIOSnoEMS
		int	021h
	pop	dx
	cmp	bx,offset _TEXT:EMSintr_new
	jne	ChkStay_NON

	; 先の二つの割り込みが同一セグメントにあるかチェック
	mov	ax,es
	cmp	ax,dx
	jne	ChkStay_NON
	
	; 常駐部 ID のチェック
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
Start:	; 非常駐部...プログラムはここから開始するぞ
;----;
	cld

	; 環境変数領域の解放
	push	es
	pop	ds
	mov	es,word ptr es:[02Ch]	; PSP:0x2C  環境変数のセグメントを格納
	mov	ah,049h			; MemFree
	int	021h
	
	; タイトル表示
	mov	dx,offset _TEXT:M_TITL
	call	PutStr

	; ｺﾏﾝﾄﾞﾗｲﾝｵﾌﾟｼｮﾝの解析(手抜き^^;)
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
	; 常駐の有無をチェック
	call	ChkStay
	je	ERR_RM

	;すでに常駐していたから、removeする

	; DISK-BIOSの復帰
	lds	dx,DWORD ptr es:[DISKbiosADDR]
	mov	ax,02500h+BIOSnoDISK
	int	021h

	; EMS-BIOSの復帰
	lds	dx,DWORD ptr es:[EMSbiosADDR]
	mov	ax,02500h+BIOSnoEMS
	int	021h

	; 常駐部メモリ開放
	mov	ax,es
	mov	es,ax
	mov	ah,049h		; MemFree
	int	021h

	; メッセージ表示&終了
	mov	dx,offset _TEXT:M_REMV
	jmp	MSG_EXIT

ERR_RM:	; 常駐してなかった
	mov	dx,offset _TEXT:M_ER_R
	jmp	MSG_EXIT


;---;
STAY:	; 常駐する場合
;---;
	; 常駐の有無をチェック
	call	ChkStay
	je ENDIF_stay
		; すでに常駐していた
		mov	dx,offset _TEXT:M_ER_S
		jmp	MSG_EXIT
	ENDIF_stay:

	; 常駐部のためのメモリを確保する
	mov	bx,offset _TEXT:StayEnd+15
	shr	bx,1
	shr	bx,1
	shr	bx,1
	shr	bx,1
	mov	cx,bx
	shl	cx,1
	shl	cx,1
	shl	cx,1
	;shl	cx,1		; 常駐部へのｺﾋﾟｰのｶｳﾝﾀ
	mov	ah,048h		; MemAlloc
	int	021h
	jc	ERR_MM

	; 常駐ﾙｰﾁﾝをｺﾋﾟｰ
	mov	es,ax		; es = 確保したｱﾄﾞﾚｽ(SEG)
	push	cs
	pop	ds
	xor	si,si
	mov	di,si
	rep	movsw

	; DISK-BIOSの設定
	mov	ds,ax		; ds = 常駐部分
	mov	ax,03500h+BIOSnoDISK
	int	021h
	mov	WORD ptr ds:[DISKbiosADDR+0],bx
	mov	WORD ptr ds:[DISKbiosADDR+2],es
	mov	dx,offset _TEXT:DISKintr_new
	mov	ax,02500h+BIOSnoDISK
	int	021h

	; EMS-BIOSの設定
	mov	ax,03500h+BIOSnoEMS
	int	021h
	mov	WORD ptr ds:[EMSbiosADDR+0],bx
	mov	WORD ptr ds:[EMSbiosADDR+2],es
	mov	dx,offset _TEXT:EMSintr_new
	mov	ax,02500h+BIOSnoEMS
	int	021h

	; EMM-ID をコピー
	mov	dx,ds
	mov	ax,es
	mov	ds,ax
	mov	es,dx
	mov	si,offset _TEXT:EMM_ID
	mov	di,si
	mov	cx,4
	cld
	rep	movsw

	; MCBの調整
	mov	ax,es
	dec	ax
	mov	ds,ax
	inc	ax
	mov	WORD ptr ds:[1],ax

	; 一応名前をMCBに設定
	mov	WORD ptr ds:[ 8],'R'+'S'*256
	mov	WORD ptr ds:[10],'C'+'N'*256
	mov	WORD ptr ds:[12],'A'+'T'*256
	mov	WORD ptr ds:[14],0


	; メッセージ表示＆終了
	mov	dx,offset _TEXT:M_STAY
	jmp	MSG_EXIT


ERR_MM:
	mov	dx,offset _TEXT:M_ER_M
	jmp	MSG_EXIT


;-----------------------------------------------
;-------;
MSG_EXIT:	; メッセージを表示して終了する
;-------;
	; メッセージ表示
	call	PutStr

	; 終了
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
