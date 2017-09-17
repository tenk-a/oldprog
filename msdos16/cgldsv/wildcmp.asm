;   1 : module WILDCMP
;   2 : // WildCmp
;   3 : // コマンドラインで -DTURBO_C=1 を指定すればTURBO-C用に, -DLSI_C=2を指定
;   4 : // すればLSI-C用になると思います（ためしてない）
;   5 : // 指定がなければアセンブラのみでしょう
;   6 : 
;   7 : 	model SMALL
;   8 : 	//stack 1024
;   9 :   @if defined(TURBO_C) || defined(LSI_C)
;  10 :     c_name "","",defined(TURBO_C)+defined(LSI_C)*2
;  11 :   @else
;  12 :     c_name "Str"
;  13 : 	import s:STR
;  14 :   @fi
;  15 : 
;  16 : 	import Is:IS
;  17 : 
;  18 : proc ax_get(si)
DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
;  19 : //	in    ds
;  20 : //	out   ax,si,cf
;  21 : //	break fx
;  22 : begin
;  23 : 	ax = b[si]
_BSS	ends

_TEXT	segment byte public 'CODE'
_ax_get	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	al,byte ptr [si]
	xor	ah,ah
;  24 : 	++si
	inc	si
;  25 : 	if (Is.Kanji(al))
	cmp	al,129
	jb	L$3
	cmp	al,159
	jbe	L$2
L$3:
	cmp	al,224
	jb	L$1
	cmp	al,252
	ja	L$1
L$2:
;  26 : 		ah = b[si]
	mov	ah,byte ptr [si]
;  27 : 		++si
	inc	si
;  28 : 		go (ah == 0) J1
	or	ah,ah
	je	L$4
;  29 : 		ah <=> al
	xchg	ah,al
;  30 : 	fi
L$1:
;  31 : 	ret
	ret
;  32 :   J1:
L$4:
;  33 : 	al = 0
	xor	al,al
;  34 : 	ret
	ret
;  35 : endproc

;  36 : 
;  37 : proc cx_get(di)
;  38 : //	in    es
;  39 : //	out   cx,di
;  40 : //	break fx
;  41 : begin
;  42 : 	cx = b[es:di]
_cx_get	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	cl,byte ptr es:[di]
	xor	ch,ch
;  43 : 	++di
	inc	di
;  44 : 	if (Is.Kanji(cl))
	cmp	cl,129
	jb	L$7
	cmp	cl,159
	jbe	L$6
L$7:
	cmp	cl,224
	jb	L$5
	cmp	cl,252
	ja	L$5
L$6:
;  45 : 		ch = b[es:di]
	mov	ch,byte ptr es:[di]
;  46 : 		++di
	inc	di
;  47 : 		go (ch == 0) J1
	or	ch,ch
	je	L$8
;  48 : 		ch <=> cl
	xchg	ch,cl
;  49 : 	fi
L$5:
;  50 : 	ret
	ret
;  51 :   J1:
L$8:
;  52 : 	cl = 0
	xor	cl,cl
;  53 : 	ret
	ret
;  54 : endproc

;  55 : 
;  56 : *proc WildCmp
;  57 :   @if defined(TURBO_C)
;  58 : 	(key:dword, fstr:dword)
;  59 : 	//out dx.ax
;  60 :   @else
;  61 : 	(bx.ax, dx.cx)
;  62 : 	//out  bx.ax,cx,dx
;  63 :   @fi
;  64 : 	c_decl
;  65 : enter
;  66 :   @if defined(TURBO_C)
;  67 : 	save fx,es,ds,di,si,cx,bx
;  68 :   @else
;  69 : 	save fx,es,ds,di,si,dx,cx
;  70 :   @fi
;  71 : 	const _N_ = -2, _E_ = -1, _Y_ = 0
;  72 : 	local  str:word
;  73 : 
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
;  74 :   @if defined(TURBO_C)
;  75 : 	bx.ax = key
	mov	ax,word ptr [bp+18]
	mov	bx,word ptr [bp+20]
;  76 : 	dx.cx = fstr
	mov	cx,word ptr [bp+22]
	mov	dx,word ptr [bp+24]
;  77 :   @fi
;  78 : 	//ds.si = bx.ax と es.di = dx.cx 。ポインタの正規化を行う
;  79 : 	si = ax
	mov	si,ax
;  80 : 	ax >>= 4
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
;  81 : 	bx += ax
	add	bx,ax
;  82 : 	ds = bx
	mov	ds,bx
;  83 : 	di = cx
	mov	di,cx
;  84 : 	cx >>= 4
	shr	cx,1
	shr	cx,1
	shr	cx,1
	shr	cx,1
;  85 : 	dx += cx
	add	dx,cx
;  86 : 	es = dx
	mov	es,dx
;  87 : 	assume ds:nothing, es:nothing
	assume	ds:NOTHING,es:NOTHING
;  88 : 	si &= 0x0F
	and	si,15
;  89 : 	di &= 0x0F
	and	di,15
;  90 : 	str = di
	mov	word ptr [bp-2],di
;  91 : 
;  92 : 	loop.w
L$9:
;  93 : 		al = b[si]
	mov	al,byte ptr [si]
;  94 : 		++si
	inc	si
;  95 : 		if (Is.Kanji(al))
	cmp	al,129
	jb	L$14
	cmp	al,159
	jbe	L$13
L$14:
	cmp	al,224
	jb	L$12
	cmp	al,252
	ja	L$12
L$13:
;  96 : 			ah = b[si]
	mov	ah,byte ptr [si]
;  97 : 			go.w (ah == '\0') RTS_ERR
	or	ah,ah
	jne	L$16
	jmp	L$15
L$16:
;  98 : 			ah <=> al
	xchg	ah,al
;  99 : 			++si
	inc	si
; 100 : 			cx_get(di)
	call	_cx_get
; 101 : 			go (ax != cx) RTS_N
	cmp	ax,cx
	jne	L$17
; 102 : 			next.w
	jmp	L$10
; 103 :         fi
L$12:
; 104 :         ah = 0
	xor	ah,ah
; 105 :         if (al == '*')
	cmp	al,42
	jne	L$18
; 106 :         	loop
L$19:
; 107 :         		WildCmp(ds.si,es.di)
	push	es
	push	di
	push	ds
	push	si
	call	_WildCmp
	add	sp,8
; 108 :         		go.w (ax == _Y_) RTS_YES
	or	ax,ax
	jne	L$23
	jmp	L$22
L$23:
; 109 :         		go.w (ax == _E_) RTS_ERR
	cmp	ax,(-1)
	jne	L$24
	jmp	L$15
L$24:
; 110 :         		cx_get(di)
	call	_cx_get
; 111 :         	endloop (cx)
	or	cx,cx
	jne	L$19
; 112 : 			go RTS_N
	jmp short	L$17
; 113 : 		fi
L$18:
; 114 :         if (al == '?')
	cmp	al,63
	jne	L$25
; 115 :         	cx_get(di)
	call	_cx_get
; 116 :         	go (cx == '\0') RTS_N
	jcxz	L$17
; 117 :   @if 0  // 使用頻度が低そうなのと、ファイル名に ^ を使うことがあるらしいので
; 118 :         elsif (al == '^')
; 119 :         	ax_get(si)
; 120 :         	go.w (ax == 0) RTS_ERR
; 121 :             cx_get(di)
; 122 :             go (ax == cx) RTS_N
; 123 :   @fi
; 124 :         elsif (al == '[')
	jmp	L$26
L$25:
	cmp	al,91
	jne	L$27
; 125 : 			cx_get(di)
	call	_cx_get
; 126 : 			go (cx == 0) RTS_N
	jcxz	L$17
; 127 :             ax_get(si)
	call	_ax_get
; 128 :             dl = 0
	xor	dl,dl
; 129 :             if (ax == '^')
	cmp	ax,94
	jne	L$28
; 130 :             	dl = 1
	mov	dl,1
; 131 :             	ax_get(si)
	call	_ax_get
; 132 :             fi
L$28:
; 133 :             bx = 0xffff
	mov	bx,65535
; 134 :             go J0
	jmp short	L$29
; 135 :   RTS_N:
L$17:
; 136 :   			go RTS_NO
	jmp short	L$30
; 137 :  
; 138 :             loop
L$31:
; 139 : 				if (ax == '-')
	cmp	ax,45
	jne	L$34
; 140 : 					ax_get(si)
	call	_ax_get
; 141 : 					go (ax == '\0') RTS_ERR
	or	ax,ax
	je	L$15
; 142 : 					if (ax == ']')
	cmp	ax,93
	jne	L$35
; 143 : 						go (bx == cx) J1
	cmp	bx,cx
	je	L$36
; 144 : 						go J2
	jmp short	L$37
; 145 : 					fi
L$35:
; 146 : 					go (cx >= bx && cx <= ax) J1
	cmp	cx,bx
	jb	L$38
	cmp	cx,ax
	jbe	L$36
L$38:
; 147 : 				else
	jmp	L$39
L$34:
; 148 :  J0:
L$29:
; 149 :                     if (ax == cx)
	cmp	ax,cx
	jne	L$40
; 150 :  J1:
L$36:
; 151 :                         go (dl) RTS_NO
	or	dl,dl
	jne	L$30
; 152 :                         loop
L$41:
; 153 :                         	ax_get(si)
	call	_ax_get
; 154 :                         	go (ax == 0) RTS_ERR
	or	ax,ax
	je	L$15
; 155 :                         endloop (ax != ']')
	cmp	ax,93
	jne	L$41
; 156 :                     	exit
	jmp short	L$33
; 157 :                     fi
L$40:
; 158 :                 fi
L$39:
; 159 :                 bx = ax
	mov	bx,ax
; 160 :                 ax_get(si)
	call	_ax_get
; 161 :                 go (ax == '\0') RTS_ERR
	or	ax,ax
	je	L$15
; 162 :                 if (ax == ']')
	cmp	ax,93
	jne	L$44
; 163 :  J2:
L$37:
; 164 :                     exit (dl)
	or	dl,dl
	jne	L$33
; 165 :                     go RTS_NO
	jmp short	L$30
; 166 :                 fi
L$44:
; 167 :             endloop
	jmp short	L$31
L$33:
; 168 : 
; 169 :         else
	jmp	L$26
L$27:
; 170 :         	cx_get(di)
	call	_cx_get
; 171 :         	if (ax != cx)
	cmp	ax,cx
	je	L$45
; 172 :         		go (ax) RTS_NO
	or	ax,ax
	jne	L$30
; 173 :         		go ((di -= str) == 0) RTS_NO
	sub	di,word ptr [bp-2]
	je	L$30
; 174 :         	  @if defined(TURBO_C)
; 175 :         		dx = ax = di
	mov	ax,di
	mov	dx,ax
; 176 :         	  @else
; 177 :         		bx = ax = di
; 178 :         	  @fi
; 179 :         		go RTS
	jmp short	L$46
; 180 :         	fi
L$45:
; 181 : 			go (cx == 0) RTS_YES
	jcxz	L$22
; 182 :         fi
L$26:
; 183 :     endloop// (ax)
L$10:
	jmp	L$9
; 184 :  RTS_YES:
L$22:
; 185 :  	ax = _Y_
	xor	ax,ax
; 186 :  	di -= str
	sub	di,word ptr [bp-2]
; 187 :   @if defined(TURBO_C)
; 188 :   	dx = di
	mov	dx,di
; 189 :   @else
; 190 :  	bx = di
; 191 :   @fi
; 192 :  	go RTS
	jmp short	L$46
; 193 :  
; 194 :  RTS_ERR:
L$15:
; 195 :  	ax = _E_
	mov	ax,(-1)
; 196 :  	go RTS_NO2
	jmp short	L$47
; 197 : 
; 198 :  RTS_NO:
L$30:
; 199 :  	ax = _N_
	mov	ax,(-2)
; 200 :  RTS_NO2:
L$47:
; 201 :   @if defined(TURBO_C)
; 202 :     dx = 0
	xor	dx,dx
; 203 :   @else
; 204 :  	bx = 0
; 205 :   @fi
; 206 :  RTS:
L$46:
; 207 :  	return
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
; 208 : endproc

; 209 : 
; 210 : endmodule
_TEXT	ends
	end
