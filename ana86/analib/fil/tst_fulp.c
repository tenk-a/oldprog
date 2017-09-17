#include <string.h>
//#include <jctype.h>
#include "\a\myl\far.h"

  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )

#if 1	/*-------------------------------------------------------------------*/
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
		fil_fullpath(p, name);
		printf("%s\n",name);
	}
	return 0;
}
#endif
