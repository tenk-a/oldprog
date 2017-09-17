;   1 : module DIRENTEY
;   2 : 	c_name "DirEntry","", defined(TURBO_C) + defined(LSI_C)*2
;   3 : 	model SMALL
;   4 : 	stack 0x200
;   5 : 
;   6 : 	import WD:WILDCMP
;   7 : 	import Is:IS
;   8 : 	//import Ps:PUTSTR
;   9 : 
;  10 : *struct DTA_T
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
;  11 : 	attr_in:byte
;  12 : 	drv:byte
;  13 : 	name:byte(8)
;  14 : 	ext:byte(3)
;  15 : 	rsv:byte(8)
;  16 : 	attr:byte
;  17 : 	time:word
;  18 : 	date:word
;  19 : 	size:dword
;  20 : 	pname:byte(13)
;  21 : endstruct
;  22 : 
;  23 : *var dta:DTA_T
_STACK	ends

_BSS	segment word public 'BSS'
	public	_DirEntrydta
_DirEntrydta	label	byte
	db	43 dup(?)
;  24 : *var key:word
	public	_DirEntrykey
_DirEntrykey	label	word
	db	2 dup(?)
;  25 : *var dir:byte(200)
	public	_DirEntrydir
_DirEntrydir	label	byte
	db	200 dup(?)
;  26 : 
;  27 : proc DtaGet()
;  28 : //	DTA �̃A�h���X�𓾂�
;  29 : //	out   es.bx
;  30 : //	break ah
;  31 : macro
;  32 : 	ah = 0x2f
;  33 : 	intr 0x21
;  34 : endproc
;  35 : 
;  36 : proc DtaSet(dx)
;  37 : //	DTA �̃A�h���X��ύX����
;  38 : //	in  ds
;  39 : //	break ah
;  40 : macro
;  41 : 	ah = 0x1a
;  42 : 	intr 0x21
;  43 : endproc
;  44 : 
;  45 : proc DirEntryFirst(dx,cx)
;  46 : //	in  ds
;  47 : //	out dx,cx,ax,cf
;  48 : macro
;  49 : 	ah = 0x4e
;  50 : 	intr 0x21
;  51 : endproc
;  52 : 
;  53 : proc DirEntryNext(dx,cx)
;  54 : //	in  ds
;  55 : //	out dx,cx,ax,cf
;  56 : macro
;  57 : 	ah = 0x4f
;  58 : 	intr 0x21
;  59 : endproc
;  60 : 
;  61 : *proc Get
;  62 :   @if defined(TURBO_C)
;  63 : 	//(pRslt:d,pKey:d,pFlg:w)
;  64 : 	(pRslt_l:w,pRslt_h:w,pKey_l:w,pKey_h:w,pFlg:w)
;  65 :   @else
;  66 : 	(bx.ax,dx.cx,pFlg:w)
;  67 :   @fi
;  68 : 	c_decl
;  69 : //	���C���h�E�J�[�h�@�\�t�t�@�C�����̃f�B���N�g���E�G���g���𓾂�
;  70 : //  ���C���h�E�J�[�h�w��Ōp���̃t�@�C�����𓾂�Ƃ��� dx.cx�ipKey�j= NULL
;  71 : //	�ŌĂяo��
;  72 : //	arg pRslt:bx.ax	���������p�X�����i�[����ꏊ �v128�o�C�g�ȏ�
;  73 : //	arg pKey:dx.cx	���C���h�J�[�h�@�\�t�p�X�� 128�o�C�g�ȓ�
;  74 : //	arg pFlg	    �f�B���N�g���E�G���g�������őΏۂƂ���t�@�C������
;  75 : //						0x01(bit 0)	�Ǐo��p�t�@�C��
;  76 : //						0x02(bit 1)	�s������
;  77 : //						0x04(bit 2)	�V�X�e���E�t�@�C��
;  78 : //						0x08(bit 3)	�{�����[�����x��
;  79 : //						0x10(bit 4) �f�B���N�g��
;  80 : //						0x20(bit 5) �ۑ��r�b�g
;  81 : //	out ax			0:�G���[��  -1:�ݒ��� 1:����ȏ�t�@�C�����Ȃ�
;  82 : //					2:�t�@�C��������
;  83 : //	out dx.cx		���̂܂�
;  84 : //	break bx
;  85 : enter
;  86 : 	local dtaSave:d, aPirFlg:b
;  87 : 	var	  aWdCdFlg:b = 0
_BSS	ends

_DATA	segment word public 'DATA'
L$1	label	byte
	db	1 dup(?)
;  88 :   @if !defined(TURBO_C)
;  89 : 	save pusha,pRslt_h:bx,pRslt_l:ax, pKey_h:dx, pKey_l:cx, si,di,es,ds
;  90 :   @else
;  91 : 	save pusha,pRet:ax,bx,cx,dx,si,di,es,ds
;  92 :   @fi
;  93 : 
;  94 : 	// DTA ��ݒ�
;  95 : 	DtaGet
_DATA	ends

_TEXT	segment byte public 'CODE'
	public	_DirEntryGet
_DirEntryGet	label	near
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
	sub	sp,6
	mov	ah,47
	int	33
;  96 : 	dtaSave = es.bx
	mov	word ptr [bp-4],bx
	mov	word ptr [bp-2],es
;  97 : 	ds = si = %var
	mov	si,DGROUP
	mov	ds,si
;  98 : 	dx = &dta
	mov	dx,offset DGROUP:_DirEntrydta
;  99 : 	DtaSet dx
	mov	ah,26
	int	33
; 100 : 	dx = pKey_h; cx = pKey_l
	mov	dx,word ptr [bp+26]
	mov	cx,word ptr [bp+24]
; 101 : 	bx = pRslt_h;ax = pRslt_l
	mov	bx,word ptr [bp+22]
	mov	ax,word ptr [bp+20]
; 102 : 	go.w (bx == 0 && ax == 0) ERR		// ���ʂ����� pRslt �� NULL �Ƃ�
	or	bx,bx
	jne	L$3
	or	ax,ax
	jne	L$3
	jmp	L$2
L$3:
; 103 : 	if (dx == 0 && cx == 0)				// pKey �� NULL �̂Ƃ�
	or	dx,dx
	jne	L$4
	or	cx,cx
	jne	L$4
; 104 : 		go.w (aWdCdFlg) SRCH_NEXT
	cmp	byte ptr [L$1],0
	je	L$6
	jmp	L$5
L$6:
; 105 : 		ax = 1
	mov	ax,1
; 106 : 		go.w RTS
	jmp	L$7
; 107 : 	fi
L$4:
; 108 : 	aWdCdFlg = 0
	mov	byte ptr [L$1],0
; 109 : 	// es.si = dx.cx �|�C���^�̐��K��
; 110 : 	si = cx
	mov	si,cx
; 111 : 	si &= 0x0f
	and	si,15
; 112 : 	cx >>= 4
	shr	cx,1
	shr	cx,1
	shr	cx,1
	shr	cx,1
; 113 : 	dx += cx
	add	dx,cx
; 114 : 	es = dx
	mov	es,dx
; 115 : 	bx = si
	mov	bx,si
; 116 : 	// pKey �� �f�B���N�g������ dir �ɃR�s�[
; 117 : 	di = dx = &dir
	mov	dx,offset DGROUP:_DirEntrydir
	mov	di,dx
; 118 : 	loop
L$8:
; 119 : 		al = b[es:si]
	mov	al,byte ptr es:[si]
; 120 : 		++si
	inc	si
; 121 : 		if (Is.Kanji(al))
	cmp	al,129
	jb	L$13
	cmp	al,159
	jbe	L$12
L$13:
	cmp	al,224
	jb	L$11
	cmp	al,252
	ja	L$11
L$12:
; 122 : 			b[di] = al
	mov	byte ptr [di],al
; 123 : 			++di
	inc	di
; 124 : 			al = b[es:si]
	mov	al,byte ptr es:[si]
; 125 : 			++si
	inc	si
; 126 : 			go.w (al == 0) ERR
	or	al,al
	jne	L$14
	jmp	L$2
L$14:
; 127 : 		elsif (al >= 'a' && al <= 'z')	//  ���p�������̑啶����
	jmp	L$15
L$11:
	cmp	al,97
	jb	L$16
	cmp	al,122
	ja	L$16
; 128 : 			al -= 0x20
	sub	al,32
; 129 : 		else
	jmp	L$15
L$16:
; 130 : 			if (al == '/')	//  '/'��'\'�ɕς���
	cmp	al,47
	jne	L$17
; 131 : 				al = '\\'
	mov	al,92
; 132 : 			fi
L$17:
; 133 : 			if (al == '\\' || al == ':')
	cmp	al,92
	je	L$19
	cmp	al,58
	jne	L$18
L$19:
; 134 : 				b[es:si-1] = al
	mov	byte ptr es:[si-1],al
; 135 : 				dx = di
	mov	dx,di
; 136 : 				++dx
	inc	dx
; 137 : 				bx = si
	mov	bx,si
; 138 : 			fi
L$18:
; 139 : 		fi
L$15:
; 140 : 		b[di] = al
	mov	byte ptr [di],al
; 141 : 		++di
	inc	di
; 142 : 	endloop (al)
	or	al,al
	jne	L$8
; 143 : 	// ���o�����ިڸ�ؖ� dir �� *.* ������B
; 144 : 	di = dx
	mov	di,dx
; 145 : 	b[di] = '*'
	mov	byte ptr [di],42
; 146 : 	++di
	inc	di
; 147 : 	b[di] = '.'
	mov	byte ptr [di],46
; 148 : 	++di
	inc	di
; 149 : 	b[di] = '*'
	mov	byte ptr [di],42
; 150 : 	++di
	inc	di
; 151 : 	b[di] = '\0'
	mov	byte ptr [di],0
; 152 : 	// pKey��茟������t�@�C������ key �ɃR�s�[
; 153 : 	++di
	inc	di
; 154 : 	key = di
	mov	word ptr [_DirEntrykey],di
; 155 : 	aPirFlg = 0
	mov	byte ptr [bp-5],0
; 156 : 	loop
L$20:
; 157 : 		al = b[es:bx]
	mov	al,byte ptr es:[bx]
; 158 : 		++bx
	inc	bx
; 159 : 		if (Is.Kanji(al))
	cmp	al,129
	jb	L$25
	cmp	al,159
	jbe	L$24
L$25:
	cmp	al,224
	jb	L$23
	cmp	al,252
	ja	L$23
L$24:
; 160 : 			b[di] = al
	mov	byte ptr [di],al
; 161 : 			++di
	inc	di
; 162 : 			al = b[es:bx]
	mov	al,byte ptr es:[bx]
; 163 : 			++bx
	inc	bx
; 164 : 			go.w (al == 0) ERR
	or	al,al
	jne	L$26
	jmp	L$2
L$26:
; 165 : 		elsif (al >= 'a' && al <= 'z')
	jmp	L$27
L$23:
	cmp	al,97
	jb	L$28
	cmp	al,122
	ja	L$28
; 166 : 			al -= 0x20
	sub	al,32
; 167 : 		elsif (al == '*' || al == '?' || al == '[' || al == '^')
	jmp	L$27
L$28:
	cmp	al,42
	je	L$30
	cmp	al,63
	je	L$30
	cmp	al,91
	je	L$30
	cmp	al,94
	jne	L$29
L$30:
; 168 : 			aWdCdFlg = 1
	mov	byte ptr [L$1],1
; 169 : 		elsif (al == '.' && b[es:bx] == '\0')
	jmp	L$27
L$29:
	cmp	al,46
	jne	L$31
	cmp	byte ptr es:[bx],0
	jne	L$31
; 170 : 			aPirFlg = 1
	mov	byte ptr [bp-5],1
; 171 : 		fi
L$31:
L$27:
; 172 : 		b[di] = al
	mov	byte ptr [di],al
; 173 : 		++di
	inc	di
; 174 : 	endloop (al)
	or	al,al
	jne	L$20
; 175 : 
; 176 :   //pusha;Ps.PutStrCr 1,ww(ds,&dir);popa
; 177 :   //pusha;Ps.PutStrCr 1,ww(ds,key);popa
; 178 : 	dx = &dir
	mov	dx,offset DGROUP:_DirEntrydir
; 179 : 	cx = pFlg
	mov	cx,word ptr [bp+28]
; 180 : 	DirEntryFirst dx,cx
	mov	ah,78
	int	33
; 181 : 	go.w (cf) FIN
	jnc	L$33
	jmp	L$32
L$33:
; 182 : 
; 183 : 	loop
L$34:
; 184 : 	  //push bx,dx.ax;Ps.PutStrCr 1,ww(ds,&dta.pname);pop bx,dx.ax
; 185 : 		si = &dta.pname
	mov	si,offset DGROUP:_DirEntrydta+30
; 186 : 		di = 0
	xor	di,di
; 187 : 		loop
L$37:
; 188 : 			rep.load  al, si
	lodsb
; 189 : 			if (al == '\0')
	or	al,al
	jne	L$40
; 190 : 				di = si
	mov	di,si
; 191 : 				--di
	dec	di
; 192 : 				b[si] = '\0'
	mov	byte ptr [si],0
; 193 : 				exit (aPirFlg == 0)
	cmp	byte ptr [bp-5],0
	je	L$39
; 194 : 				b[di] = '.'
	mov	byte ptr [di],46
; 195 : 				exit
	jmp short	L$39
; 196 : 			fi
L$40:
; 197 : 		endloop (al != '.')
	cmp	al,46
	jne	L$37
L$39:
; 198 : 	  //push bx,dx.ax;Ps.PutStrCr 1,ww(ds,&dta.pname);pop bx,dx.ax
; 199 : 	  @if defined(TURBO_C)
; 200 : 	  	cx = &dta.pname
	mov	cx,offset DGROUP:_DirEntrydta+30
; 201 : 	    WD.WildCmp ww(ds,key),ds.cx
	push	ds
	push	cx
	push	ds
	push	word ptr [_DirEntrykey]
	call	_WildCmp
	add	sp,8
; 202 : 	  @else
; 203 : 		bx.ax = ww(ds,key)
; 204 : 		dx.cx = ww(ds,&dta.pname)
; 205 : 		WD.WildCmp bx.ax, dx.cx
; 206 : 	  @fi
; 207 : 		if (di)
	or	di,di
	je	L$41
; 208 : 			b[di] = '\0'
	mov	byte ptr [di],0
; 209 : 		fi
L$41:
; 210 : 		exit (ax == 0)
	or	ax,ax
	je	L$36
; 211 :   SRCH_NEXT:
L$5:
; 212 : 		dx = &dir
	mov	dx,offset DGROUP:_DirEntrydir
; 213 : 		cx = pFlg
	mov	cx,word ptr [bp+28]
; 214 : 		DirEntryNext dx,cx
	mov	ah,79
	int	33
; 215 : 		go (cf) FIN
	jc	L$32
; 216 : 	endloop
	jmp short	L$34
L$36:
; 217 : 	// pRslt �� dir ���R�s�[
; 218 : 	bx = pRslt_h
	mov	bx,word ptr [bp+22]
; 219 : 	ax = pRslt_l
	mov	ax,word ptr [bp+20]
; 220 : 	di = ax
	mov	di,ax
; 221 : 	ax >>= 4
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
; 222 : 	bx += ax
	add	bx,ax
; 223 : 	es = bx
	mov	es,bx
; 224 : 	di &= 0x0f
	and	di,15
; 225 : 	si = &dir
	mov	si,offset DGROUP:_DirEntrydir
; 226 : 	loop
L$42:
; 227 : 		rep.load al,si
	lodsb
; 228 : 		rep.set  di,al
	stosb
; 229 : 	endloop (al)
	or	al,al
	jne	L$42
; 230 : 	// pRslt �̌�Ɍ��������t�@�C�������R�s�[
; 231 : 	di -= 4
	sub	di,4
; 232 : 	si = &dta.pname
	mov	si,offset DGROUP:_DirEntrydta+30
; 233 : 	loop
L$45:
; 234 : 		rep.load al,si
	lodsb
; 235 : 		rep.set  di,al
	stosb
; 236 : 	endloop (al)
	or	al,al
	jne	L$45
; 237 : 	ax = 0
	xor	ax,ax
; 238 : 	b[es:di] = al
	mov	byte ptr es:[di],al
; 239 : 	go  RTS
	jmp short	L$7
; 240 :   ERR:
L$2:
; 241 :   	ax = -1
	mov	ax,(-1)
; 242 :   	go  RTS
	jmp short	L$7
; 243 :   FIN:
L$32:
; 244 :   	if (ax == 0x12)
	cmp	ax,18
	jne	L$48
; 245 :   		ax = 1
	mov	ax,1
; 246 :   	fi
L$48:
; 247 :   RTS:
L$7:
; 248 :   @if defined(TURBO_C)
; 249 :     pRet = ax
	mov	word ptr [bp+16],ax
; 250 :   @else
; 251 :     dx.ax .=. ax
; 252 :     pRslt_l = ax
; 253 :     pRslt_h = dx
; 254 :   @fi
; 255 : 	ds.dx = dtaSave
	lds	dx,dword ptr [bp-4]
; 256 : 	DtaSet dx
	mov	ah,26
	int	33
; 257 : 	return
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
; 258 : endproc

; 259 : 
; 260 : endmodule
_TEXT	ends
extrn _WildCmp:near
	end
