/*
	acpy
	1992/06/10	v1.00 writen by M.Kitamura
	1994/04/10	v1.01 fwrite�ŏ������݃G���[�`�F�b�N������悤�ɂ���.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char	byte;
typedef unsigned		word;
typedef unsigned long	dword;

#ifndef toascii /* ctype.h ��include����Ă��Ȃ��Ƃ� */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif
#if 0
#ifndef CT_KJ1	/* jctype.h ��include����Ă��Ȃ��Ƃ� */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif
#endif

extern int DirEntryGet(byte far *,byte far *,word);

#define pr(s) fputs(s,stdout)

static void Usage(void)
{
	pr(
		"usage: acpy [-opts] file...\n"
		"-s<dir>   ���͂���t�@�C���̂���f�B���N�g�����w��\n"
		"-d<dir>   �w�肳�ꂽ�t�@�C�����f�B���N�g��<dir>�ɃR�s�[\n"
		"-m<mfile> �w�肳�ꂽ�t�@�C�������Ƀt�@�C��<mfile>�ɏo��\n"
		"-a        �o�͐�ɓ���file�����݂���΁A�ǉ�����(��̫��)\n"
		"-o        �o�͐�ɓ���file�����݂���΁A�㏑��\n"
		"-t        �e�L�X�g�E���[�h�ŃI�[�v�� (��̫��)\n"
		"-b        �o�C�i���E���[�h�ŃI�[�v��\n"
		"\n"
		"-d,-m �̂����ꂩ�̎w�肪�K�v\n"
		"-s,-i��<dir>�͏ȗ��ł��Ȃ����A-m��<mfile>�͏ȗ������ƕW���o��\n"
		"-a,-o�ȗ����� -a ���w�肳�ꂽ���ƂɂȂ�\n"
		"-b,-t�ȗ����� -t ���w�肳�ꂽ���ƂɂȂ�\n"
		);
	exit(1);
}

static void cpy(FILE *ifp, FILE *ofp)
{
	#define BUFSIZE 30*1024U
	static byte buf[BUFSIZE+2];
	int siz;

	for(;;) {
		siz = (int)fread(buf,1,BUFSIZE,ifp);
		if (siz <= 0)
			break;
		if (fwrite(buf,1,siz,ofp) != siz) {
			pr("�����ŃG���[?\n");
			exit(1);
		}
	}
}

static byte *FName(byte *p)
{
	byte *s;
	byte c;

	s = p;
	while(*s) {
		c = *s++;
		if (c == '/' || c == '\\' || c == ':') {
			p = s;
		}
	}
	return p;
}

static FILE *fopen_e(byte *name, byte *mode)
{
	FILE *fp;

	if (name == NULL)
		return stdin;
	fp = fopen(name,mode);
	if (fp == NULL) {
		/* fprintf(stderr,"\n %s ����݂ł��܂���\n",name); */
		pr("\n");
		pr(name);
		pr(" ���I�[�v���ł��܂���\n");
		exit(errno);
	}
	return fp;
}

static byte *oOutDir, *oInDir, *oOutName;
static byte oApOvFlg, oBinTxt;

static void OptErr(void)
{
	pr("�I�v�V��������������\n");
	exit(1);
}

static void SamOptErr(void)
{
	pr("����̃I�v�V�����������w�肳�ĂĂ���\n");
	exit(1);
}

static void Option(byte *p)
{
	word c;

	if (*p == '\0')
		Usage();
	while (*p) {
		c = *p++;
		switch(toupper(c)) {
		case 'A':
			if (oApOvFlg)
				SamOptErr();
			oApOvFlg = 'a';
			break;
		case 'O':
			if (oApOvFlg)
				SamOptErr();
			oApOvFlg = 'w';
			break;
		case 'S':
			if (oInDir)
				SamOptErr();
			if (*p == '\0')
				OptErr();
			oInDir = p;
			goto EXIT_WHILE;
		case 'D':
			if (oOutDir)
				SamOptErr();
			if (*p == '\0')
				OptErr();
			oOutDir = p;
			goto EXIT_WHILE;
		case 'M':
			if (oOutName)
				SamOptErr();
			oOutName = p;
			goto EXIT_WHILE;
		case 'T':
			if (oBinTxt)
				SamOptErr();
			oBinTxt = 't';
			break;
		case 'B':
			if (oBinTxt)
				SamOptErr();
			oBinTxt = 'b';
			break;
		case '?':
			Usage();
		default:
			OptErr();
		}
	}
  EXIT_WHILE:;
}

int main(int argc, byte *argv[])
{
	byte *p,*s,*onp;
	int i;
	byte fmode[4];
	FILE *ofp,*ifp;
	static byte inambuf[130],onambuf[130],buf[130];

	if (argc < 2) {
		Usage();
	}
	for(i = 1; i < argc; i++) {
		p = argv[i];
		if (*p++ == '-')
			Option(p);
	}
	if (oOutName == NULL && oOutDir == NULL) {
	  #if 0
		Usage();
	  #else
		pr("use: acpy [-opts] file...\n");
		pr("-d,-m �̂����ꂩ�̎w�肪�K�v('-'��'-?'�Ő����\��)\n");
	  #endif
		exit(1);
	}
	if (oApOvFlg == 0) {
		oApOvFlg = 'a';
	}
	if (oBinTxt == 0) {
		oBinTxt = 't';
	}
	fmode[0] = oApOvFlg;
	fmode[1] = oBinTxt;
	fmode[2] = '\0';
	onp = onambuf;
	if (oOutDir) {
		onp = stpcpy(onambuf,oOutDir);
		if (*(onp-1) != '\\' && *(onp-1) != '/' && *(onp-1) != ':')
			*onp++ = '\\';
	} else if (oOutName) {
		if (*oOutName == '\0') {
			if (oBinTxt == 'b') {
				fsetbin(stdout);
			} else {
				fsettext(stdout);
			}
			ofp = stdout;
		} else {
			ofp = fopen_e(oOutName,fmode);
		}
	}
	for(i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-')
			continue;
		s = inambuf;
		if (oInDir) {
			if (!(*p == '.' || *(p+1) == ':' || *p == '\\' || *p == '/')) {
				s = stpcpy (inambuf,oInDir);
				if (*(s-1) != '\\' && *(s-1) != '/' && *(s-1) != ':')
					*s++ = '\\';
			}
		}
		stpcpy(s,p);
		if (DirEntryGet(buf,inambuf,0) == 0) {
			do {
				ifp = fopen_e(buf,(oBinTxt == 'b') ? "rb" : "r");
				if (oOutDir) {
					stpcpy(onp,FName(buf));
					ofp = fopen_e(onambuf,fmode);
					cpy(ifp,ofp);
					fclose(ofp);
				} else {
					cpy(ifp,ofp);
				}
				fclose(ifp);
			} while (DirEntryGet(buf,NULL,0) == 0);
		}
	}
	if (oOutName)
		fclose(ofp);
	return 0;
}
