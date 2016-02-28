#include <string.h>
#include <ctype.h>
#include <jctype.h>
#include "fil.h"

#define SJIS

int FIL_FullPath(char *src, char dst[])	///
	///	src �Ŏ����ꂽ�p�X���A��΃p�X�ɕϊ����� dst �ɔ[�߂�.
	///	RET : 0=ok 1=error (���o�[�W�����ł͂����� 0:ok ��Ԃ�)
{
	char buf[400];

	{
		int drvno = 0;

		if (src[0] && src[1] == ':') {
			drvno = toupper(*src) - 'A';
			src += 2;
		} else {
			drvno = FIL_GetDrive();
		}
		dst[0] = (char)(drvno + 'A');
		dst[1] = ':';
		dst += 2;
		if (*src == '\\' || *src == '/') {
			strcpy (buf+2, src);
		} else {
			char *p;
			FIL_GetCurDir(drvno+1, buf);
			p = buf + strlen(buf); /*strend(buf);*/
			if (*(p-1) != '\\' && *(p-1) != '/') {
				*p++ = '\\';
			}
			strcpy(p, src);
		}
	}
	/*	���ڰ�'\\' '/' ��'/'�ɂ��A".." "."�ŕs�v�ިڸ�ؖ���������폜 */
	{	/* �킴�킴�|�C���^����łȂ��z�񑀍�Ȃ̂́Afar-pointer�΍�...? */
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
					if (src[s])
						s++;
					d = dir;
					d++;
				} else {
					goto J1;
				}
			} else {
		  J1:
				while (src[s] != '\0' && src[s] != '\\' && src[s] != '/') {
				  #ifdef SJIS
					if (src[s+1] && iskanji(src[s])) {
						dst[d++] = src[s++];
					}
				  #endif
					dst[d++] = src[s++];
				}
			}
		}
		dst[d] = '\0';
	}
	/* '/' �� '\\' �ɒu�������A�p��������啶���� */
	{
		int c;
		while ((c = *dst) != '\0') {
		  #if 1
			if (c == '/') {
				*(dst++) = '\\';
			} else 
		  #endif
			if (c >= 'a' && c <= 'z') {
				*(dst++) = (char)(c + 'A' - 'a');
			} else {
			  #ifdef SJIS
				if (iskanji(c) && dst[1]) {
					dst++;
				}
			  #endif
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
		printf("�I�v�V�������������� %s\n",p0);
		exit(1);
	}
}

int main (int argc, char *argv[])
{
	int i,c;
	char *p;
	char name[400];

	// �I�v�V�����̏���
	for (c = i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			Option(p);
		} else {
			c = 0;
		}
	}
	if (c) {
		printf ("�t�@�C�������w�肳��ĂȂ��悧...\n");
		exit(1);
	}

	//�t�@�C�����ɏ���
	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			continue;
		}
		FIL_FullPath(p, name);
		printf("%s\n",name);
	}
	return 0;
}
#endif
