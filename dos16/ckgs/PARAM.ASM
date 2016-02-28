;	// ana86 L1 v0.15用ソース
;	module Param
;	
;		//import PUTSTR
;		//type Ps:PUTSTR
;	//var em:b() = "error!"
;	
;	/////////////////  
;	*var pos:w
DGROUP	group	_DATA,_BSS,_TEXT

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
	public	Param@pos
Param@pos	label	word
	db	2 dup(?)
;	var pos2:w
Param@pos2	label	word
	db	2 dup(?)
;	var cmdlin:w
Param@cmdlin	label	word
	db	2 dup(?)
;	var cmdend:w
Param@cmdend	label	word
	db	2 dup(?)
;	var buf:b(130)
Param@buf	label	byte
	db	130 dup(?)
;	
;	*proc Init(es)
;	//	コマンドライン解析の準備 （プログラムの最初に呼ぶ）
;	//	PSPにあるコマンドライン・パラメータを、空白(0x20以下)を'\0'に置き換
;	//	ながら自前のバッファにコピーする。
;	//	ただし、" か ' で囲まれた文字列中の空白は置き換えないが、" や ' を
;	//	取り除かない。
;	//	arg es		PSP のセグメント
;	//	in  ds		ds = %var (DGROUP セグメント)
;	//	in  df		df = 0 (順方向サーチ)
;	//	out pos		自前のバッファへのポインタ
;	//	out es
;	enter
;		save ax,cx,si,di,fx //,ds
;	
_BSS	ends

_TEXT	segment byte public 'CODE'
	public	Param@Init
Param@Init	label	near
	push	ax
	push	cx
	push	si
	push	di
	pushf
	push	bp
	mov	bp,sp
;		//rep.inc
;		//ds = ax = %var
;		si = &buf
	mov	si,offset DGROUP:Param@buf
;		cmdlin = si
	mov	word ptr [Param@cmdlin],si
;		Param.pos = si
	mov	word ptr [Param@pos],si
;		Param.pos2 = si
	mov	word ptr [Param@pos2],si
;		di = 0x80
	mov	di,128
;		loop
L$1:
;			++di
	inc	di
;			al = b[es:di]
	mov	al,byte ptr es:[di]
;			go (al == 0x0d) ERR
	cmp	al,13
	je	L$4
;		endloop (al <= ' ')
	cmp	al,32
	jbe	L$1
;		--di
	dec	di
;		--si
	dec	si
;		ah = 0
	xor	ah,ah
;		loop
L$5:
;			++di
	inc	di
;			++si
	inc	si
;			b[si] = al = b[es:di]
	mov	al,byte ptr es:[di]
	mov	byte ptr [si],al
;			if (al == '"'||al == '\'')
	cmp	al,34
	je	L$9
	cmp	al,39
	jne	L$8
L$9:
;				if (ah == 0)
	or	ah,ah
	jne	L$10
;					ah = al
	mov	ah,al
;				elsif (ah == al)
	jmp	L$11
L$10:
	cmp	ah,al
	jne	L$12
;					ah = 0
	xor	ah,ah
;				fi
L$12:
L$11:
;			fi
L$8:
;			if (al <= ' ' && ah == 0)
	cmp	al,32
	ja	L$13
	or	ah,ah
	jne	L$13
;				b[si] = '\0'
	mov	byte ptr [si],0
;			fi
L$13:
;			exit (al == 0x0d)
	cmp	al,13
	je	L$7
;		endloop (di < 0x100)
	cmp	di,256
	jb	L$5
L$7:
;		b[si] = '\0'
	mov	byte ptr [si],0
;		cmdend = si
	mov	word ptr [Param@cmdend],si
;		go RTS
	jmp short	L$14
;	 ERR:
L$4:
;	 	ax = 0
	xor	ax,ax
;	 	cmdlin = ax
	mov	word ptr [Param@cmdlin],ax
;	 	pos    = ax
	mov	word ptr [Param@pos],ax
;	 	pos2   = ax
	mov	word ptr [Param@pos2],ax
;	 	cmdend = ax
	mov	word ptr [Param@cmdend],ax
;		//pusha;Ps.PutStrCr 1,ww(ds,&em);popa
;	 RTS:
L$14:
;		return
	mov	sp,bp
	pop	bp
	popf
	pop	di
	pop	si
	pop	cx
	pop	ax
	ret
;	endproc

;	
;	@if 0
;	*proc Top()
;	//	Param.Getでとりだす順番を一番目に移動
;	//	in  ds// = %var
;	begin
;		save ax
;		Param.pos2 = ax = cmdlin
;		Param.pos = ax
;		return
;	endproc
;	@fi
;	
;	*proc Get()
;	//  (Init でコピーされた)コマンドライン・パラメータよりトークンを一つ
;	//	取り出し、内部ポインタを次のトークンへ進める
;	//	取り出すべきトークンのないときは NULL を返す
;	//	in    ds	= %var (DGROUP) に設定されていること
;	//	in    df	= 0
;	//	out   di	di(pos) 取り出したトークン  無し時:0
;	//	out   cf   	取り出すものがないとき cf = 1
;	//	break fx
;	begin
;		save ax,es
;	
	public	Param@Get
Param@Get	label	near
	push	ax
	push	es
;		es = ds
	push	ds
	pop	es
;		di = pos2
	mov	di,word ptr [Param@pos2]
;		pos = di
	mov	word ptr [Param@pos],di
;		go (di == 0) FIN
	or	di,di
	je	L$15
;		ax = 0
	xor	ax,ax
;		loop
L$16:
;			go (di > cmdend) FIN
	cmp	di,word ptr [Param@cmdend]
	ja	L$15
;			|rep.scan di,al
	scasb
;		endloop (!=)
	jne	L$16
;		--di
	dec	di
;		loop
L$19:
;			go (di > cmdend) FIN
	cmp	di,word ptr [Param@cmdend]
	ja	L$15
;			|rep.scan di,al
	scasb
;		endloop (==)
	je	L$19
;		--di
	dec	di
;	  RTS:
L$22:
;		Param.pos2 = di
	mov	word ptr [Param@pos2],di
;		di = pos
	mov	di,word ptr [Param@pos]
;		if (di == 0)
	or	di,di
	jne	L$23
;			cf = 1
	stc
;		else
	jmp	L$24
L$23:
;			cf = 0
	clc
;		fi
L$24:
;		return
	pop	es
	pop	ax
	ret
;	  FIN:
L$15:
;	  	di = 0
	xor	di,di
;		go RTS
	jmp short	L$22
;	endproc

;	
;	endmodule
_TEXT	ends
	end
