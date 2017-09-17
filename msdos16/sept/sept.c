/*
 * sept
 * ver 1.00  91/04/30    writen by M.Kitamura
 * ver 1.01  91/06/09    -s�݂̂̂Ƃ��̓����ύX
 *           94/04/11    ���J����ɂ������Ė��O��sep����sept�ɕύX
 * ver 1.02  00/05/29    os9�ňȊO�ł̃o�b�t�@�T�C�Y�𑝂₷.
 *                       -s�̋�؂蕶���̎w���MS�S�p��Ή��ɂ��A
 *                       -z��MS�S�p�Ή��Ƃ���
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef OS9
  #include <jstring.h>

  typedef   char byte;
  #define ep(s)     fputs(s,stderr)
  #define F_MAX     128
  #define SBUFSIZ   1024
  #define KETA      100
#else
#ifdef DOS16
  #include <jstring.h>
  typedef   unsigned char byte;
  #define ep(s)     fputs(s,stdout)
  #define F_MAX     (2*1024)
  #define SBUFSIZ   (32*1024-15)
  #define KETA      512
#else
  #include <mbstring.h>
  typedef   unsigned char byte;
  #define ep(s)     fputs(s,stdout)
  #define F_MAX     (16*1024)
  #define SBUFSIZ   (64*1024-15)
  #define KETA      512
  #define jstrtok   _mbstok
#endif
#endif

byte Buf[SBUFSIZ+6];


byte *Sep = " \t\n";
int  Cnt = 0;
int  P_flg = 0;
int  Pra = 1;
int  Fi = 0;
char Sjis_flg = 0;

void
usage(n)
     int  n;
{
     ep("usage:sept [-opts] [files]...\n");
     ep("���͂��ꂽ÷�Ă�' ','\\t','\\n'�ŒP��ɕ�����. ��߼�݂ōēx�Ȃ��ĕ\���ł���\n");
     ep(" -?       ����\n");
     ep(" -p       �P������s�����󔒂ŋ�؂��ĕ\��\n");
     ep(" -pN      1�s�ɂ��Ȃ炸N��\�� (N�ꖈ�ɉ��s����)\n");
     ep(" -s       �P��ɋ�؂�Ȃ�(���͂���1�s��P�ꈵ��)\n");
     ep(" -s=<Str> ���ڰ������𕶎���ł������� (strtok�̑�����)\n");
     ep(" -z=<Str> ���ڰ������𕶎���ł������� (jstrtok�̑�����)\n");
     ep(" -wN      -fL�w�莞,1�s�͈̔͂�N��(N*L�޲�)�Ƃ���,�͈͓��ŕ\��\n");
   #ifdef OS9
     ep(" -fL      �P��̻��ނ�L(0-80)�޲ĂƂ��ĕ\�������낦��\n");
   #else
     ep(" -fL      �P��̻��ނ�L(0-512)�޲ĂƂ��ĕ\�������낦��\n");
   #endif
     exit(n);
}

void
opts_err()
{
     ep("��߼�݂̎w�肪��������\n");
     usage(1);
}

void
open_err(p)
     byte *p;
{
     ep("\n");
     ep(p);
     ep(":̧�ق�����݂ł��܂���\n");
     exit(errno);
}

void
pri(s)
     byte *s;
{
     int  n;

     n = strlen(s);
     if (n == 0)
          return;
     if (Pra != 0 && Fi != 0 && P_flg != 0 && (Cnt += n/Fi) >= Pra) {
          fputs("\n",stdout);
          Cnt = n / Fi;
     }
     fputs(s,stdout);
     Cnt++;
     if (Pra != 0 && (Cnt %= Pra) == 0) {
          fputs("\n",stdout);
     } else {
          if (Fi > 0 ) {
               n = Fi - n % Fi;
          } else
               n = 1;
          while (--n >= 0)
               fputs(" ",stdout);
     }
}

int
sept(fp,join_flg)
     FILE *fp;
     int  join_flg;
{
     byte *s;

     for (; ;) {
          s = fgets(Buf,BUFSIZ,fp);
          if (s == NULL)
               break;
          if (join_flg == 0) {
               if (Sjis_flg) {
                    while( (s = jstrtok(s,Sep)) != NULL && *s != '\0' ){
                         pri(s);
                         s = NULL;
                    }
               } else {
                    while( (s = strtok(s,Sep)) != NULL && *s != '\0' ){
                         pri(s);
                         s = NULL;
                    }
               }
          } else {
               if ((s = strchr(Buf,'\n')) != NULL)
                    *s = '\0';
               pri(Buf);
          }
          if (ferror(fp))
               exit(errno);
     }
     return 0;
}

int
main(argc,argv)
     int  argc;
     byte *argv[];
{
     FILE *fp;
     int  join_flg;
     int  f_cnt;
     int  i;
     static byte *f_name[F_MAX+1];

     join_flg = f_cnt = 0;
     while (--argc > 0) {
          ++argv;
          if (**argv == '-') {
               switch( toupper(*(++*argv)) ) {
               case 'W':
                    P_flg = 1;
                    goto j1;
               case 'P':
                    P_flg = 0;
j1:
                    if (*++(*argv) == '=')
                         ++*argv;
                    Pra = atoi(*argv);
                    if (Pra < 0 || Pra > KETA)
                         opts_err();
                    break;
               case 'F':
                    if (*++(*argv) == '=')
                         ++*argv;
                    Fi = atoi(*argv);
                    if (Fi < 0 || Fi > KETA)
                         opts_err();
                    break;
               case 'Z':
                    Sjis_flg = 1;
                    goto SKIP_J1;
               case 'S':
                    Sjis_flg = 0;
            SKIP_J1:
                    if (*++(*argv) == '=')
                        ++*argv;
                    join_flg = 0;
                    if (**argv != '\0')
                        Sep = *argv;
                    else
                        join_flg = 1;
                    break;
               case '\0':
               case '?':
                    usage(0);
               default:
                    opts_err();
               }
          } else {
               if (f_cnt < F_MAX)
                    f_name[f_cnt++] = *argv;
          }
     }
     if (f_cnt) {
          for ( i = 0;i < f_cnt; i++ )  {
               fp = fopen(f_name[i],"r");
               if (fp == NULL)
                    open_err(f_name[i]);
               sept(fp,join_flg);
          }
     } else {
          sept(stdin,join_flg);
     }
     if (Pra != 0 && (Cnt % Pra) != 0)
          fputs("\n",stdout);
     return 0;
}
