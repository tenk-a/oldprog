#include <string.h>
#include <dos.h>
#include <stddef.h>
#include "tenkafun.h"
#include "sub.h"
#ifndef LSI_C
  #include "tc_sub.h"
#endif

/*-----------------------------------------------------------------------------
  jstrrplchr  - �S�p�Ή���1�����u���֐�
    byte *jstrrplchr(byte *s,word c1, word c2)
    	byte *s; ������̐擪
    	word c1; �ϊ�����镶��
    	word c2; �ϊ���̕���
    ������ s ���� ����c1 �� c2 �ɕϊ����A�l�Ƃ��ĕ�����s �ւ��߲����Ԃ��܂��B
    c1 , c2 ���Ƃ��ɔ��p���A�܂��́A�Ƃ��ɑS�p�̂΂����̂ݕϊ��ł��܂��B
    �S�p�Ɣ��p�̑g�����ʹװ��NULL ���Ԃ���܂�.

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
    return s;/* �����p������� jstprplchr() */
}
#endif


/*---------------------------------------------------------------------------
  jwildcmp  -- ���JIS�Ή�ܲ��޶��ދ@�\�t���������r�֐�
    int jwildcmp( byte *key, byte *str);
    	byte *key;  ܲ��޶��ދ@�\�̕�����
    	byte *str;

    ������key��str���r���Aϯ������0���A���Ȃ����0�ȊO(-2,-1,����n)��Ԃ�.
    	0     key �� str ��ϯ�����
    	-1    key �̎w�肪���������iܲ��޶��ނ̎w��н�Ȃ�)
    	-2    str �� key ���������񒷂��Z��
    	����n str �� �擪 n-1�޲� �� key ��ϯ�����. (str �� key ��蒷��)

    ܲ��ޥ���ޕ��� (c �� ���p���S�p������ 1�����̂���)
�@�@�@�@c     ����c ��ϯ�.
    	*     0�����ȏ�̔C�ӂ̕������ϯ�.
    	?     �C�ӂ�1������ϯ�.
    	^c    c �ȊO�̔C�ӂ�1������ϯ�.

    	[..]  [ ] �ł�����ꂽ�����񒆂̂ǂꂩ1������ϯ�.
    	[^..] [^ ] �ł�����ꂽ�����񒆂ɂȂ�1������ϯ�.
    	      ����̒��ł�ܲ��޶��ޕ����̋@�\�͗}������܂�.
    	      ����̒���']'���܂߂����΂����� []abc] �� [^]] �̂悤��,'['��'[^'
    	      �̒����']'��u���Ă�������. �܂趯����͕K�� 1���͐ݒ肳���
    	      �K�v������܂�.
    	      ����̒��ł�, A-Z �̂悤��,'-'��p���ĕ������ނ� A ���� Z �͈̔�
    	      �ɂ���ǂꂩ 1������ϯ�����邱�Ƃ��ł��܂�.
    	      '-'�̗����ɕ������Ȃ��Ƃ��A�@�\���܂���.
    	      �܂�A[-abc] �� [abc-] �̂悤��,'['��'[^'�̒����']'�̒��O��
    	      '-'������΂�����, [0-9-A] �Ƃ����悤�Ȏw���, '9' �̒��� ('A'��
�@�@�@�@�@�@�@���O)��'-'�́A1�����Ƃ��Ĉ����܂�.
    	      ����'-'�̉E���̕����� �����̕��������傫�����̂��w�肵�Ă�����
    	      ��. Z-A �ł�ϯ����܂���.

    	\c    c �� *,?,^,\,[ �ł���΂���ܲ��ޥ���ދ@�\��}����1���Ƃ��Ĉ���
    	\xx   xx ��16�i��2���܂łȂ炻�̒l�̕����P���Ƃ��Ĉ���.
    	      [ ],[^ ] ���ł͖���.
    	      _ESC_ ��ݒ肵�ĺ��߲ق��Ȃ��Ƃ����܂���.

    	�����͑S�p(2�޲�)�������P�����Ƃ��Ĉ���ꂷ.
    	'\0'�͂P�����ł͂���܂���B

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
    byte*  src; /* ̧�ٖ��ւ��߲�� */
    byte** dir; /* �ިڸ�ؖ��ւ��߲���ϐ� �ւ��߲���ϐ� */
    byte** nam; /* ̧�ٖ��ւ��߲���ϐ�	  �ւ��߲���ϐ� */
    byte** ext; /* �g���q���ւ��߲���ϐ�  �ւ��߲���ϐ� */
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
    str���̔��p��������啶���ɂ���
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
    byte *f_nam; �݂�����������������߂��ޯ̧.
    	    	 �\���Ȼ���(100-200�޲Ă��炢?)���m�ۂ��邱��
    byte *key;	 1���:ܲ��޶��މ�̧�ٖ�.�ިڸ�ؖ���	2��ڈȍ~:NULL

    ���������Ƃ��A0, ������Ȃ������Ƃ� 0�ȊO��Ԃ�.
*/
    #define FLG  _A_NORMAL | _A_RDONLY
int
getWildFname(byte *f_nam, byte *key)
{
    static struct find_t ft;  /* _dos_findfirst/next ���g���\���� */
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
    /* �g���q��ext0 �Ɠ����Ȃ炻�̂܂܁A�g���q���Ȃ����ext0�ɁA����ȊO��
     * �G���[�ɂ���
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
    /* �g���q��ύX���� */
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
