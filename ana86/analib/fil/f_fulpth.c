#include <string.h>
//#include <jctype.h>
#include "far.h"

  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )

unsigned char far * _saveregs
fil_fullpath(unsigned char far *src, unsigned char far *dst0)
{
	unsigned char far *dst;
	unsigned char buf[400];

	dst = dst0;
	{
		unsigned char drvno;

		if (src[0] && src[1] == ':') {
			drvno = *src;
			drvno = toupper(drvno) - 'A';
			src += 2;
		} else {
			drvno = fil_getdrive();
		}
		dst[0] = drvno + 'A';
		dst[1] = ':';
		dst += 2;
		if (*src == '\\' || *src == '/') {
			str_cpy (buf+2, src);
		} else {
			unsigned char far *p;
			fil_getcdir(drvno+1, buf);
			p = str_end(buf);
			if (*(p-1) != '\\' && *(p-1) != '/') {
				*p++ = '\\';
			}
			str_cpy(p, src);
		}
	}
	/*	ｾﾊﾟﾚｰﾀ'\\' '/' を'/'にし、".." "."で不要ﾃﾞｨﾚｸﾄﾘ名文字列を削除 */
	{	/* わざわざポインタ操作でなく配列操作なのは、far-pointer対策よっ！ */
		int s,d,dir;

		src = buf+2;
		dir = d = s = 0;
		while (src[s]) {
			if (src[s] == '\\' || src[s] == '/') {
				dir = d;
				dst[d] = '/';
				d++;
				s++;
			} else if (src[s] == '.') {
				if (src[s+1] == '.'
					&& (src[s+2]=='\\' || src[s+2]=='/' || src[s+2]=='\0')) {
					s += 2;
					d = dir;
					while (d && dst[--d] != '/')
						;
					dir = d;
					d++;
				} else if (src[s+1]=='\\'|| src[s+1]=='/'|| src[s+1]=='\0') {
					s++;
					d = dir;
					d++;
				} else {
					goto J1;
				}
			} else {
		  J1:
				while (src[s] != '\0' && src[s] != '\\' && src[s] != '/') {
					if (str_getflagj() && src[s+1] && iskanji(src[s])) {
						dst[d++] = src[s++];
					}
					dst[d++] = src[s++];
				}
			}
		}
		dst[d] = '\0';
	}
	/* '/' を '\\' に置き換え、英小文字を大文字化 */
	{
		int c;
		while ((c = *dst) != '\0') {
			if (c == '/') {
				*(dst++) = '\\';
			} else if (c >= 'a' && c <= 'z') {
				*(dst++) = c + 'A' - 'a';
			} else {
				if (str_getflagj() && iskanji(c) && dst[1]) {
					dst++;
				}
				dst++;
			}
		}
	}
	return 0;
}

#if 0	/*-------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

void Option(char *p)
{
	char *p0 = p;
	int c;

	p++;
	c = *p++;
	switch(c) {
	case '?':
	case '\0':
		puts("usage: fullpath file(s)\n");
		exit(1);
		break;
	default:
		printf("オプションがおかしい %s\n",p0);
		exit(1);
	}
}

int main (int argc, char *argv[])
{
	int i,c;
	char *p;
	char name[400];

	// オプションの処理
	for (c = i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			Option(p);
		} else {
			c = 0;
		}
	}
	if (c) {
		printf ("ファイル名が指定されてないよぉ...\n");
		exit(1);
	}

	//ファイル毎に処理
	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			continue;
		}
		file_fullpath(p, name);
		printf("%s\n",name);
	}
	return dst;
}
#endif
