#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "tenkafun.h"
#include "rpal.h"

static void
extcat(byte name[], byte *ext, byte p[])
{
    byte ch;
    int  i, fnp;

    fnp = 0;
    for (i = 0; name[i] != '\0'; ++i) {
    	ch = name[i];
    	if (ch == '/' || ch == '\\') {
    	    fnp = i;
    	    ch = '\\';
    	}
    	p[i] = ch;
    }
    p[i] = '\0';
    for (i = fnp; p[i] != '\0' && p[i] != '.'; ++i)
    	;
    p[i] = '\0';
    strcat(p, ext);
}

static void
usage(void)
{
    printf("use:rpalt [-opts] [<̧��1>[.RGB]]\n");
    printf(
    	"<̧��1>[.RGB]���w�肳��Ă���΂������گĂɐݒ肵,\n"
    	"�풓��گĂ�����΂���ɑΉ����܂�.\n"
    	"<̧��1>[.RGB]���߼�݂��w�肳��Ă��Ȃ��Ƃ���-l���w�肳�ꂽ���ƂɂȂ�܂�\n"
    	"-?        ����\n"
    	"-l        ���݂̏풓��گĂ̒l��\�����܂�\n"
    	"-v[N]     ��گĂ̖��x(toon)��N�ɂ���. N = 0�`100[%] �ȗ�=50\n"
    	"-r<̧��2> ���݂̏풓��گĂ̒l��<̧��2>.RGB�ɏo��(toon����)\n"
    	"-t<̧��2> ���݂̏풓��گĂ̒l��toon���v�Z����<̧��2>.RGB�ɏo��\n\n"
    	"l,v,r,t��߼�݂�̧��1[.RGB]���w�肳��Ă����,����ɂ���Đݒ肳�ꂽ\n"
    	"��گĂɑ΂��Ď��s���܂�\n"
    	"l,r�͏풓��گĂ��Ȃ��ƴװ�ł�."
    	"v,t��<̧��1>[.RGB]������Ώ풓��گĂ�\n"
    	"�Ȃ��Ă����삵�܂���, �����Ƃ��Ȃ��Ƃ��ʹװ�ł�\n"

    );
    exit(0);
}


int
main(int argc, byte *argv[])
{
    static byte grb[48];
    static int toon,dmy;
    FILE *fp;
    byte *p;
    int  i;
    byte l_flg,r_flg,t_flg,v_flg;
    static byte filNam1[256];
    byte *filNam2;

    l_flg = r_flg = t_flg = v_flg = 0;
    filNam2 = NULL;
    filNam1[0] = '\0';
    toon = 100;
    if (argc == 1)
    	usage();
    for (i = 1;i < argc; ++i) {
    	p = argv[i];
    	if ((*p) != '-')
    	    continue;
    	while (*(++p) != '\0') {
    	    switch(*p) {
    	    case '?':
    	    	usage();
    	    case 'l':
    	    	l_flg = 1;
    	    	break;
    	    case 'v':
    	    	v_flg = 1;
    	    	++p;
    	    	if (*p == '=')
    	    	    ++p;
    	    	if (*p >= '0' && *p <= '9')
    	    	    toon = (int)strtol(p,&p,10);
    	    	else
    	    	    toon = 50;
    	    	--p;
    	    	break;
    	    case 'r':
    	    	r_flg = 1;
    	    	goto J1;
    	    case 't':
    	    	t_flg = 1;
J1:
    	    	if (filNam2 == NULL) {
    	    	     ++p;
    	    	     if (*p == '=')
    	    	    	 ++p;
    	    	     filNam2 = p;
    	    	     goto LOOP_OUT;
    	    	}
    	    	printf("r,t��߼�݂͓����w����d�w��ł��܂���\n");
    	    	exit(1);
    	    default:
    	    	printf("�w�肳�ꂽ��߼�݂͂�������\n");
    	    	exit(1);
    	    }
    	}
LOOP_OUT:;
    }
    for (i = 1; i < argc; ++i) {
    	if (*(argv[i]) == '-')
    	    continue;
    	if (filNam1[0]) {
    	    printf("̧�ٖ����������܂�");
    	    exit(1);
    	}
    	extcat(argv[i],".RGB",filNam1);
    }

    if (RPal_Get(&dmy, grb)) {
    	printf("�풓��گĂ�����܂���\n");
    	if ((v_flg == 0) && (l_flg == 0))
    	    exit(1);
    }
    if (filNam1[0]) {
    	fp = fopen(filNam1,"rb");
    	if (fp == NULL) {
    	    printf("%s ������܂���",filNam1);
    	    exit(1);
    	}
    	for (i = 0; i < 16; ++i) {
    	    grb[i*3+1] = fgetc(fp);
    	    grb[i*3+0] = fgetc(fp);
    	    grb[i*3+2] = fgetc(fp);
    	}
    	fclose(fp);
    	RPal_Set(toon,grb);
    }else if (v_flg) {
    	RPal_Set(toon,grb);
    }
    if (r_flg||t_flg) {
    	if (t_flg) {
    	    for (i = 0; i < 48; ++i) {
    	    	 grb[i] = (grb[i] * toon) / 100;
    	    }
    	}
    	fp = fopen(filNam2,"wb");
    	if (fp == NULL) {
    	    printf("%s ������܂���",filNam1);
    	    exit(1);
    	}
    	for (i = 0;i < 16;i++) {
    	    fputc(grb[i*3+1],fp);
    	    fputc(grb[i*3+0],fp);
    	    fputc(grb[i*3+2],fp);
    	}
    	fclose(fp);
    }

    if (l_flg) {
    	RPal_Get(&toon, grb);
    	printf("toon = %d\n",toon);
    	for(i = 0;i < 16; ++i) {
    	    printf("%2d G:%02x  R:%02x  B:%02x\n",
    	    	i,grb[i*3],grb[i*3+1],grb[i*3+2]);
    	}
    }
    return 0;
}
