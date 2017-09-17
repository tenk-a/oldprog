;	module WILDCMP
;	// WildCmp
;	// コマンドラインで -DTURBO_C=1 を指定すればTURBO-C用に, -DLSI_C=2を指定
;	// すればLSI-C用になると思います（ためしてない）
;	// 指定がなければアセンブラのみでしょう
;	
;		//model SMALL
;		//stack 1024
;	  @if defined(TURBO_C) || defined(LSI_C)
;	    c_name "","",defined(TURBO_C)+defined(LSI_C)*2
;	  @else
;	    c_name "Str"
;		import s:STR
;	  @fi
;	
;		import Is:IS
;	
;	proc ax_get(si)
DGROUP	group	_DATA,_BSS

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

_BSS	segment word public 'BSS'
;	//	in    ds
;	//	out   ax,si,cf
;	//	break fx
;	begin
;		ax = b[si]
_BSS	ends

_TEXT	segment byte public 'CODE'
_ax_get	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	al,byte ptr [si]
	xor	ah,ah
;		++si
	inc	si
;		if (Is.Kanji(al))
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
;			ah = b[si]
	mov	ah,byte ptr [si]
;			++si
	inc	si
;			go (ah == 0) J1
	or	ah,ah
	je	L$4
;			ah <=> al
	xchg	ah,al
;		fi
L$1:
;		ret
	ret
;	  J1:
L$4:
;		al = 0
	xor	al,al
;		ret
	ret
;	endproc

;	
;	proc cx_get(di)
;	//	in    es
;	//	out   cx,di
;	//	break fx
;	begin
;		cx = b[es:di]
_cx_get	label	near
	assume	ds:DGROUP,es:NOTHING,ss:DGROUP
	mov	cl,byte ptr es:[di]
	xor	ch,ch
;		++di
	inc	di
;		if (Is.Kanji(cl))
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
;			ch = b[es:di]
	mov	ch,byte ptr es:[di]
;			++di
	inc	di
;			go (ch == 0) J1
	or	ch,ch
	je	L$8
;			ch <=> cl
	xchg	ch,cl
;		fi
L$5:
;		ret
	ret
;	  J1:
L$8:
;		cl = 0
	xor	cl,cl
;		ret
	ret
;	endproc

;	
;	*proc WildCmp
;	  @if defined(TURBO_C)
;		(key:dword, fstr:dword)
;		//out dx.ax
;	  @else
;		(bx.ax, dx.cx)
;		//out  bx.ax,cx,dx
;	  @fi
;		c_decl
;	enter
;	  @if defined(TURBO_C)
;		save fx,es,ds,di,si,cx,bx
;	  @else
;		save fx,es,ds,di,si,dx,cx
;	  @fi
;		const _N_ = -2, _E_ = -1, _Y_ = 0
;		local  str:word
;	
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
;	  @if defined(TURBO_C)
;		bx.ax = key
	mov	ax,word ptr [bp+18]
	mov	bx,word ptr [bp+20]
;		dx.cx = fstr
	mov	cx,word ptr [bp+22]
	mov	dx,word ptr [bp+24]
;	  @fi
;		//ds.si = bx.ax と es.di = dx.cx 。ポインタの正規化を行う
;		si = ax
	mov	si,ax
;		ax >>= 4
	shr	ax,1
	shr	ax,1
	shr	ax,1
	shr	ax,1
;		bx += ax
	add	bx,ax
;		ds = bx
	mov	ds,bx
;		di = cx
	mov	di,cx
;		cx >>= 4
	shr	cx,1
	shr	cx,1
	shr	cx,1
	shr	cx,1
;		dx += cx
	add	dx,cx
;		es = dx
	mov	es,dx
;		assume ds:nothing, es:nothing
	assume	ds:NOTHING,es:NOTHING
;		si &= 0x0F
	and	si,15
;		di &= 0x0F
	and	di,15
;		str = di
	mov	word ptr [bp-2],di
;	
;		loop.w
L$9:
;			al = b[si]
	mov	al,byte ptr [si]
;			++si
	inc	si
;			if (Is.Kanji(al))
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
;				ah = b[si]
	mov	ah,byte ptr [si]
;				go.w (ah == '\0') RTS_ERR
	or	ah,ah
	jne	L$16
	jmp	L$15
L$16:
;				ah <=> al
	xchg	ah,al
;				++si
	inc	si
;				cx_get(di)
	call	_cx_get
;				go (ax != cx) RTS_N
	cmp	ax,cx
	jne	L$17
;				next.w
	jmp	L$10
;	        fi
L$12:
;	        ah = 0
	xor	ah,ah
;	        if (al == '*')
	cmp	al,42
	jne	L$18
;	        	loop
L$19:
;				  @if defined(TURBO_C)
;					WildCmp ds.si, es.di
	push	es
	push	di
	push	ds
	push	si
	call	_WildCmp
	add	sp,8
;	        	  @else
;					bx.ax = ds.si
;					dx.cx = es.di
;					WildCmp bx.ax, dx.cx
;	        	  @fi
;	        		go.w (ax == _Y_) RTS_YES
	or	ax,ax
	jne	L$23
	jmp	L$22
L$23:
;	        		go.w (ax == _E_) RTS_ERR
	cmp	ax,(-1)
	jne	L$24
	jmp	L$15
L$24:
;	        		cx_get(di)
	call	_cx_get
;	        	endloop (cx)
	or	cx,cx
	jne	L$19
;				go RTS_N
	jmp short	L$17
;			fi
L$18:
;	        if (al == '?')
	cmp	al,63
	jne	L$25
;	        	cx_get(di)
	call	_cx_get
;	        	go (cx == '\0') RTS_N
	jcxz	L$17
;	  @if 0  // 使用頻度が低そうなのと、ファイル名に ^ を使うことがあるらしいので
;	        elsif (al == '^')
;	        	ax_get(si)
;	        	go.w (ax == 0) RTS_ERR
;	            cx_get(di)
;	            go (ax == cx) RTS_N
;	  @fi
;	        elsif (al == '[')
	jmp	L$26
L$25:
	cmp	al,91
	jne	L$27
;				cx_get(di)
	call	_cx_get
;				go (cx == 0) RTS_N
	jcxz	L$17
;	            ax_get(si)
	call	_ax_get
;	            dl = 0
	xor	dl,dl
;	            if (ax == '^')
	cmp	ax,94
	jne	L$28
;	            	dl = 1
	mov	dl,1
;	            	ax_get(si)
	call	_ax_get
;	            fi
L$28:
;	            bx = 0xffff
	mov	bx,65535
;	            go J0
	jmp short	L$29
;	  RTS_N:
L$17:
;	  			go RTS_NO
	jmp short	L$30
;	 
;	            loop
L$31:
;					if (ax == '-')
	cmp	ax,45
	jne	L$34
;						ax_get(si)
	call	_ax_get
;						go (ax == '\0') RTS_ERR
	or	ax,ax
	je	L$15
;						if (ax == ']')
	cmp	ax,93
	jne	L$35
;							go (bx == cx) J1
	cmp	bx,cx
	je	L$36
;							go J2
	jmp short	L$37
;						fi
L$35:
;						go (cx >= bx && cx <= ax) J1
	cmp	cx,bx
	jb	L$38
	cmp	cx,ax
	jbe	L$36
L$38:
;					else
	jmp	L$39
L$34:
;	 J0:
L$29:
;	                    if (ax == cx)
	cmp	ax,cx
	jne	L$40
;	 J1:
L$36:
;	                        go (dl) RTS_NO
	or	dl,dl
	jne	L$30
;	                        loop
L$41:
;	                        	ax_get(si)
	call	_ax_get
;	                        	go (ax == 0) RTS_ERR
	or	ax,ax
	je	L$15
;	                        endloop (ax != ']')
	cmp	ax,93
	jne	L$41
;	                    	exit
	jmp short	L$33
;	                    fi
L$40:
;	                fi
L$39:
;	                bx = ax
	mov	bx,ax
;	                ax_get(si)
	call	_ax_get
;	                go (ax == '\0') RTS_ERR
	or	ax,ax
	je	L$15
;	                if (ax == ']')
	cmp	ax,93
	jne	L$44
;	 J2:
L$37:
;	                    exit (dl)
	or	dl,dl
	jne	L$33
;	                    go RTS_NO
	jmp short	L$30
;	                fi
L$44:
;	            endloop
	jmp short	L$31
L$33:
;	
;	        else
	jmp	L$26
L$27:
;	        	cx_get(di)
	call	_cx_get
;	        	if (ax != cx)
	cmp	ax,cx
	je	L$45
;	        		go (ax) RTS_NO
	or	ax,ax
	jne	L$30
;	        		go ((di -= str) == 0) RTS_NO
	sub	di,word ptr [bp-2]
	je	L$30
;	        	  @if defined(TURBO_C)
;	        		dx = ax = di
	mov	ax,di
	mov	dx,ax
;	        	  @else
;	        		bx = ax = di
;	        	  @fi
;	        		go RTS
	jmp short	L$46
;	        	fi
L$45:
;				go (cx == 0) RTS_YES
	jcxz	L$22
;	        fi
L$26:
;	    endloop// (ax)
L$10:
	jmp	L$9
;	 RTS_YES:
L$22:
;	 	ax = _Y_
	xor	ax,ax
;	 	di -= str
	sub	di,word ptr [bp-2]
;	  @if defined(TURBO_C)
;	  	dx = di
	mov	dx,di
;	  @else
;	 	bx = di
;	  @fi
;	 	go RTS
	jmp short	L$46
;	 
;	 RTS_ERR:
L$15:
;	 	ax = _E_
	mov	ax,(-1)
;	 	go RTS_NO2
	jmp short	L$47
;	
;	 RTS_NO:
L$30:
;	 	ax = _N_
	mov	ax,(-2)
;	 RTS_NO2:
L$47:
;	  @if defined(TURBO_C)
;	    dx = 0
	xor	dx,dx
;	  @else
;	 	bx = 0
;	  @fi
;	 RTS:
L$46:
;	 	return
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
;	endproc

;	
;	endmodule
_TEXT	ends
	end
