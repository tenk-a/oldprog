#include <string.h>
#include <dos.h>
#include <stddef.h>
#include "tenkafun.h"
#include "sub.h"
#ifndef LSI_C
  #include "tc_sub.h"
#endif

/*-----------------------------------------------------------------------------
  jstrrplchr  - 全角対応の1文字置換関数
    byte *jstrrplchr(byte *s,word c1, word c2)
    	byte *s; 文字列の先頭
    	word c1; 変換される文字
    	word c2; 変換後の文字
    文字列 s 中の 文字c1 を c2 に変換し、値として文字列s へのﾎﾟｲﾝﾀを返します。
    c1 , c2 がともに半角か、または、ともに全角のばあいのみ変換できます。
    全角と半角の組合せはｴﾗｰでNULL が返されます.

  1991/09/23 by M.kitamura
*/

#if 1
byte *
jstrrplchr(s,c1,c2)
    byte *s;
    word c1;
    word c2;
{
    byte *p;

    if (s == NULL)
    	return NULL;
    p = s;
    if (c1 <= 0xff && c2 <= 0xff)
    	for (p = s; *p; ++p ) {
    	    if (iskanji(*p)) {
    	    	if (*(p+1))
    	    	    ++p;
    	    } else if (*p == (byte)c1) {
    	    	*p = (byte)c2;
    	    }
    	}
    else if (c1 > 0xff && c2 >0xff) {
    	byte a1,a2,b1,b2;

    	a1 = c1 / 0x100;
    	a2 = c1 % 0x100;
    	b1 = c2 / 0x100;
    	b2 = c2 % 0x100;
    	for (p = s; *p; ++p ) {
    	    if (*p == a1 && *(p+1) == a2) {
    	    	*(p++) = b1;
    	    	*(p++) = b2;
    	    }
    	}
    } else
    	return NULL;
    return s;/* これをpかえれば jstprplchr() */
}
#endif


/*---------------------------------------------------------------------------
  jwildcmp  -- ｼﾌﾄJIS対応ﾜｲﾙﾄﾞｶｰﾄﾞ機能付き文字列比較関数
    int jwildcmp( byte *key, byte *str);
    	byte *key;  ﾜｲﾙﾄﾞｶｰﾄﾞ機能可の文字列
    	byte *str;

    文字列keyとstrを比較し、ﾏｯﾁすれば0を、しなければ0以外(-2,-1,正数n)を返す.
    	0     key と str はﾏｯﾁした
    	-1    key の指定がおかしい（ﾜｲﾙﾄﾞｶｰﾄﾞの指定ﾐｽなど)
    	-2    str は key よりも文字列長が短い
    	正数n str は 先頭 n-1ﾊﾞｲﾄ で key とﾏｯﾁした. (str は key より長い)

    ﾜｲﾙﾄﾞ･ｶｰﾄﾞ文字 (c は 半角か全角文字の 1文字のこと)
　　　　c     文字c にﾏｯﾁ.
    	*     0文字以上の任意の文字列にﾏｯﾁ.
    	?     任意の1文字にﾏｯﾁ.
    	^c    c 以外の任意の1文字にﾏｯﾁ.

    	[..]  [ ] でくくられた文字列中のどれか1文字にﾏｯﾁ.
    	[^..] [^ ] でくくられた文字列中にない1文字にﾏｯﾁ.
    	      ｶｯｺの中ではﾜｲﾙﾄﾞｶｰﾄﾞ文字の機能は抑制されます.
    	      ｶｯｺの中に']'を含めたいばあいは []abc] や [^]] のように,'['や'[^'
    	      の直後に']'を置いてください. つまりｶｯｺ内は必ず 1字は設定される
    	      必要があります.
    	      ｶｯｺの中では, A-Z のように,'-'を用いて文字ｺｰﾄﾞで A から Z の範囲
    	      にあるどれか 1文字にﾏｯﾁされることができます.
    	      '-'の両側に文字がないとき、機能しません.
    	      つまり、[-abc] や [abc-] のように,'['や'[^'の直後や']'の直前に
    	      '-'があるばあいや, [0-9-A] というような指定で, '9' の直後 ('A'の
　　　　　　　直前)の'-'は、1文字として扱われます.
    	      あと'-'の右側の文字は 左側の文字よりも大きいものを指定してくださ
    	      い. Z-A ではﾏｯﾁしません.

    	\c    c が *,?,^,\,[ であればそのﾜｲﾙﾄﾞ･ｶｰﾄﾞ機能を抑制し1字として扱う
    	\xx   xx が16進数2桁までならその値の文字１字として扱う.
    	      [ ],[^ ] 内では無効.
    	      _ESC_ を設定してｺﾝﾊﾟｲﾙしないとつかえません.

    	文字は全角(2ﾊﾞｲﾄ)文字も１文字として扱われす.
    	'\0'は１文字ではありません。

*/

/*
#define _ESC_ '\\'
*/

#ifdef _ESC_
static word
sget2b(byte **p)
{
    word d;
    word d2;

    d = *((*p)++);
    if (isdigit(d))
    	d -= '0';
    if (d >= 'A' && d <= 'F')
    	d -= 'A'-10;
    if (d >= 'a' && d <= 'f')
    	d -= 'a'-10;
    else
    	goto J1;
    d2 = **p;
    if (isdigit(d))
    	d2 -= '0';
    if (d >= 'A' && d <= 'F')
    	d2 -= 'A'-10;
    if (d >= 'a' && d <= 'f')
    	d2 -= 'a'-10;
    else
    	goto J1;
    d = d * 0x10 + d2;
    (*p)++;
 J1:
    return d;
}

static word
sgetc_esc(byte **p)
{
    word d;
    word d2;

    d = sget2b(p);
    if (iskanji(**p)) {
    	d2 = *((*p)++);
    	if (d2 == _ESC_);
    	    d2 = sget2b(p);
    	if (d2 != '\0')
    	    d = d * 0x10 + d2;
    	else
    	    d = '\0';
    return d;
}
#endif


static word
sgetc(byte **p)
{
    word d;

    d = *((*p)++);
    if (iskanji(**p)) {
    	if (**p != '\0')
    	    d = d * 0x100 + *((*p)++);
    	else
    	    d = '\0';
    }
    return d;
}


int
jwildcmp(key,str)
    byte *key;
    byte *str;
{
    #define _Y_ 0
    #define _N_ -2
    #define _E_ -1
    word c,d,c1,c2;
    byte *sp;
    byte f;

    sp = str;
 #ifdef DEBUG
    printf(" wild: '%s' '%s'\n",key,sp);
 #endif
    for (;;) {
    	c = *(key++);
    	if (iskanji(c)) {
    	    if (*key == '\0')
    	    	return _E_;
    	    else {
    	    	c = c*0x100 + *(key++);
    	    	if (c == sgetc(&sp))
    	    	    continue;
    	    	else
    	    	    return _N_;
    	    }
    	}
    	switch(c) {
    	case '*':
    	    do {
    	    	if (jwildcmp(key,sp) == _Y_)
    	    	    return _Y_;
    	    } while (sgetc(&sp));
    	    return _N_;

    	case '?':
    	    if (sgetc(&sp) == '\0')
    	    	return _N_;
    	    break;

    	case '[':
    	    d = sgetc(&sp);
    	    if (d == '\0')
    	    	return _E_;
    	    c = sgetc(&key);
    	    f = (c == '^');
    	    if (f)
    	    	c = sgetc(&key);
    	    c1 = '\0';
    	    for (;;) {
    	    	if (c == '-' && c1 != '\0') {
    	    	    c = sgetc(&key);
    	    	    if (c == ']') {
    	    	    	if (d == '-')
    	    	    	    goto J1;
    	    	    	else
    	    	    	    goto J2;
    	    	    } else if (c == '\0') {
    	    	    	return _E_;
    	    	    } else {
    	    	    	c2 = c;
    	    	    }
    	    	    c = 0x00;
    	    	    if (c1 <= d && d <= c2)
    	    	    	goto J1;
    	    	} else {
    	    	    if (c == d) {
 J1:
    	    	    	if (f)
    	    	    	    return _N_;
    	    	    	else {
    	    	    	    for (;;) {
    	    	    	    	c = sgetc(&key);
    	    	    	    	if (c == ']')
    	    	    	    	    break;
    	    	    	    	else if (c == '\0')
    	    	    	    	    return _E_;
    	    	    	    }
    	    	    	    break;
    	    	    	}
    	    	    }
    	    	}
    	    	c1 = c;
    	    	c = sgetc(&key);
    	    	if (c == '\0')
    	    	    return _E_;
    	    	if (c == ']') {
 J2:
    	    	    if (f)
    	    	    	break;
    	    	    else
    	    	    	return _N_;
    	    	}
    	    } /* end for (;;) */
    	    break;

    	case '^':
    	    c = sgetc(&key);
 #ifdef _ESC_
    	    if (c == _ESC_)
    	    	c = sgetc_esc(&key);
 #endif
    	    if (c == '\0')
    	    	return _E_;
    	    if (c == sgetc(&sp))
    	    	return _N_;
    	    break;

 #ifdef _ESC_
    	case _ESC_:
    	    c = sgetc_esc(&key);
 #endif

    	default:
    	    if (c != sgetc(&sp)) {
    	    	if (c == '\0')
    	    	    return sp - str;
    	    	else
    	    	    return _N_;
    	    }
    	}
    	if (c == '\0')
    	    return _Y_;
    }/* end for(;;) */
}

/* getFnamPtr ---------------------------------------------------------------*/
byte *
getFnamPtr(src,dir,nam,ext)
    byte*  src; /* ﾌｧｲﾙ名へのﾎﾟｲﾝﾀ */
    byte** dir; /* ﾃﾞｨﾚｸﾄﾘ名へのﾎﾟｲﾝﾀ変数 へのﾎﾟｲﾝﾀ変数 */
    byte** nam; /* ﾌｧｲﾙ名へのﾎﾟｲﾝﾀ変数	  へのﾎﾟｲﾝﾀ変数 */
    byte** ext; /* 拡張子名へのﾎﾟｲﾝﾀ変数  へのﾎﾟｲﾝﾀ変数 */
{
    byte *ep;
    byte *np;
    byte *s;

    for (ep = NULL, np = s = src; *s != '\0'; ++s) {
    	if (iskanji(*s)) {
    	    if (*(++s) == '\0')
    	    	break;
    	} else if (*s == ':' || *s == '/' || *s == '\\') {
    	    np = s + 1;
    	} else if (*s == '.') {
    	    ep = s + 1;
    	}
    }
    if (dir != NULL) {
    	if (np != src)
    	    *dir = src;
    	else
    	    *dir = NULL;
    }
    if (nam != NULL)
    	*nam = np;
    if (ext != NULL)
    	*ext = ep;
    return s;
}

/* struprj -------------------------------------------------------------------
byte *struprj(byte str[]);
    str中の半角小文字を大文字にする
*/
byte *
struprj(byte s[])
{
    byte c;

    while ((c = *s) != '\0') {
    	if (iskanji(c)) {
    	    if (*++s == '\0')
    	    	break;
    	} else if (c >= 'a' && c <= 'z')
    	    *s -= 0x20;
    	++s;
    }
    return s;
}

/* getWildFname --------------------------------------------------------------
int getWildFname(f_nam,key)
    byte *f_nam; みつかった文字列をおさめるﾊﾞｯﾌｧ.
    	    	 十分なｻｲｽﾞ(100-200ﾊﾞｲﾄくらい?)を確保すること
    byte *key;	 1回目:ﾜｲﾙﾄﾞｶｰﾄﾞ可のﾌｧｲﾙ名.ﾃﾞｨﾚｸﾄﾘ名可	2回目以降:NULL

    見つかったとき、0, 見つからなかったとき 0以外を返す.
*/
    #define FLG  _A_NORMAL | _A_RDONLY
int
getWildFname(byte *f_nam, byte *key)
{
    static struct find_t ft;  /* _dos_findfirst/next が使う構造体 */
    static byte key_nam[102];
    static byte dir[102];
    static int	dirlen;
    byte *p,*q,*s;
    int  e;

    if (key != NULL) {
    	dirlen = 0;
    	s = getFnamPtr(key,&p,&q,NULL);
    	if (p != NULL) {
    	    dirlen = q - p;
    	    if (dirlen > 100)
    	    	return -1;
    	    memcpy(dir,p,dirlen);
    	    *(dir+dirlen) = '\0';
    	    jstrrplchr(dir,'/','\\');
    	}
    	if (s - q > 100)
    	    return -1;
    	strcpy(dir + dirlen,"*.*");
    	strcpy(key_nam,q);
    	e = _dos_findfirst(dir,FLG,&ft);
    	*(dir+dirlen) = '\0';
    } else {
    	e = _dos_findnext(&ft);
    }
 #ifdef DEBUG
    if (dir)
    	printf("_dos_wildfind:dir == %s\n",dir);
    else
    	printf("_dos_wildfind:dir == NULL\n");
 #endif
    if (e != 0)
    	return e;
    do {
    	if (jwildcmp(key_nam,ft.name) == 0) {
    	    if (dirlen != 0)
    	    	memcpy(f_nam,dir,dirlen);
    	    strcpy(f_nam + dirlen,ft.name);
    	    struprj(f_nam);
 #ifdef DEBUG
    	    printf("  f_nam   = %s\n",f_nam);
 #endif
    	    return 0;
    	}
 #ifdef DEBUG
    	    printf("  ft.name == %s\n",ft.name);
 #endif
    } while ((e = _dos_findnext(&ft)) == 0);
    return e;
}

/*------------------------------------------------------------------------*/
int
set_ext(byte name[], byte *ext0)
    /* 拡張子がext0 と同じならそのまま、拡張子がなければext0に、それ以外は
     * エラーにする
     */
{
    byte *ext;
    byte *s;

    s = getFnamPtr(name,NULL,NULL,&ext);
    if (ext == NULL) {
    	*s++ = '.';
    	strcpy(s,ext0);
    } else if (stricmp(ext0,ext))
    	return 1;
    struprj(name);
    return 0;
}

/*------------------------------------------------------------------------*/
int
reset_ext(byte name[], byte *ext0)
    /* 拡張子を変更する */
{
    byte *ext;
    byte *s;

    s = getFnamPtr(name,NULL,NULL,&ext);
    if (ext == NULL) {
    	*s++ = '.';
    	strcpy(s,ext0);
    } else {
    	strcpy(ext,ext0);
    }
    struprj(name);
    return 0;
}
