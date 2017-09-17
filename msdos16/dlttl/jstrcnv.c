/*
  unsigned char *jstrcnv(js1,js2,opts)
	 unsigned char    *js1; �ϊ����ꂽ��������i�[����o�b�t�@�ւ̃|�C���^
	 unsigned char    *js2; ���̕�����
	 unsigned short   opts; �ϊ����@

    ������js2��opts�Ŏw�肳�ꂽ�ϊ����@�ŕϊ����A���ʂ�js1�Ɋi�[���܂��B
  �l�Ƃ���js1��Ԃ��܂��B�������A�G���[���������Ƃ���NULL��Ԃ��܂��B
    js1�͌��ʂ��i�[����̂ɏ\���ȃT�C�Y���K�v�ł��B
    �L���͔��p�A�S�p�ň�Έ�ɑΉ������ϊ����s���܂��B

    �S�p�𔼊p�ɂ���Ƃ��A�����E�����������͕������ĕϊ����܂��B
  ���Ƃ��� '�_' �Ȃ�' ��' �Ƃ����ӂ��ɂ��܂��B
    �܂����p�� ��S�p�ɕϊ�����Ƃ��A�����E�������t���̕����őS�p�ɑΉ�����
  ����������΁A���̑S�p�����ɕϊ����܂��B'��'�Ȃ�'�_'�Ƃ����ӂ��ɂȂ�܂��B

    ����(�J)�E������(�K)�E��������(�[)�E�Ȃ��_�i�E�j�́A�S�p�̂΂����A�J�^
  �J�i�̌��ɂ���΃J�^�J�i�Ƃ��āA�Ђ炪�Ȃ̌��ɂ���΂Ђ炪�ȂƂ��āA
  �����łȂ���΃J�i�L���Ƃ��Ĉ����܂��B�܂��A���p�Ȃ�΁A�ł̌��ɂ����
  �� �Ƃ��āA�łȂ���� �ŋL���Ƃ��Ĉ����܂��B
  ���Ƃ��ΑS�p�J�^�J�i�̔��p���̂Ƃ��A"�����[�E�E�H�[�Y"�Ȃ�"�����[�E�����"
�@�ɁA"�X�^�[�E�����[��" �Ȃ� "���������[��" �ɂȂ�܂��B

    �ϊ��ł��Ȃ��S�p�����i�悤�����2�޲Ėڂ��s���Ȃ��́j���������΂����A
  �����ɂ���Ď��̂悤�ȏ��������܂��B
  2�޲Ėڂ��A
    '\0'�̏ꍇ:
	   1byte�ڂ�'\0'�ɂ���B
    �\���s�\�ȕ����i0x01�`0x08,0x0a�`0x1f,0x7f,0xfd�`0xff�j�̏ꍇ:
	   ���̕s���ȕ���(2�޲�)���Ƃ΂��ď����𑱂��܂��B
	   ���A�l�Ƃ���NULL��Ԃ��܂��B
    �\���\�ȕ���(0x09,0x20�`0x3f)�̏ꍇ:
	   ����1�޲Ėڂ݂̂Ƃ΂��ď����𑱂��܂��B
�@�@�@�@���A�l�Ƃ���NULL��Ԃ��܂��B

    �ϊ����@�� JTOPUNS|JTODGT �̂悤��'|'�łȂ��Ďw�肵�Ă��������B

  opts �̒l
    JTOPUNS  (0x0001)  �S�p�L���𔼊p�ɕϊ����܂��i�󔒂��ϊ����܂��j
    JTODGT   (0x0002)  �S�p�����𔼊p�ɕϊ����܂�
    JTOALPH  (0x0004)  �S�p�A���t�@�x�b�g�𔼊p�ɂ��܂�
    JTOKPUN  (0x0008)  �S�p�J�i�L���𔼊p�ɕϊ����܂�
    JTOKATA  (0x0010)  �S�p�J�^�J�i�𔼊p�łɕϊ����܂�
    JTOHIRA  (0x0020)  �S�p�Ђ炪�Ȃ𔼊p�łɕϊ����܂�
    JSPC2SPC (0x0040)  �S�p�L���𔼊p�ɕϊ�����Ƃ��A�S�p�󔒂𔼊p�󔒂Q
				   �@�@�����ɕϊ����܂�
    YENOFF   (0x0080)  \ �ɑΉ�����S�p�� �� �łȂ� �_ �ɂ���

    TOJPUNS  (0x4100)  ���p�L����S�p�ɕϊ����܂�(�󔒂��ϊ����܂�)
    TOJPUN   (0x0100)  ���p�L����S�p�ɕϊ����܂�(�󔒂͕ϊ����܂���I)
    TOJDGT   (0x0200)  ���p������S�p�ɕϊ����܂�
    TOJALPH  (0x0400)  ���p��̧�ޯĂ�S�p�ɕϊ����܂�
    TOJKPUN  (0x0800)  ���p�ŋL����S�p�ɕϊ����܂�
    TOJKATA  (0x1000)  ���p�ł�S�p�J�^�J�i�ɕϊ����܂�
    TOJHIRA  (0x2000)  ���p�ł�S�p�Ђ炪�Ȃɕϊ����܂�
    TOJSPC   (0x4000)  ���p�󔒂�S�p�ɕϊ����܂�
    DAKUOFF  (0x8000)  ���p�ł̑S�p���ő����E�����������ւ̕ϊ���}�����A
				   �����E���������������P�����Ƃ��Ĉ����܂�

   * TOJKATA �� TOJHIRA �������Ɏw�肳�ꂽ�΂����� TOJKATA ���D�悳��܂��B

   * 1991/06/20 Writen by M.Kitamura
   * 1991/08/23 Debug:�S�p�����̔��p���̔���н�B('&&'��'&'�̑ł��ԈႢ��2��...)
   * 1992/10/25 Debug:�f(8166h)�Ɓe(8165h)�𔼊p�ɂ���Ƃ��t�ɕϊ����Ă���
                Debug:�b(8162h)�����p�ɂł��Ȃ�����.
                \ �ɑΉ�����S�p�� �� �� �_ �����w��ł���悤�ɂ���
 */

#include <stddef.h>
#include "jstr.h"
#include "tenkdefs.h"

#define  iskanji2(c)	((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)

/***/
byte *jstrcnv(jstr1,js2,opts)
	byte *jstr1;
	REGI byte *js2;
	word opts;
{
	static byte tojkigo[] = {
		0x49,0x68,0x94,0x90,0x93, /* �I�h������ */
		0x95,0x66,0x69,0x6a,0x96, /* ���f�i�j�� */
		0x7b,0x43,0x7c,0x44,0x5e, /* �{�C�\�D�^ */
		0x46,0x47,0x83,0x81,0x84, /* �F�G������ */
		0x48,0x97,                /* �H��       */
				  0x6d,0x8f,0x6e, /*     �m���n */
		0x4f,0x51,0x65,           /* �O�Q�e     */
					   0x6f,0x62, /*       �o�b */
		0x70,0x50,                /* �p�P       */
				  0x42,0x75,0x76, /*     �B�u�v */
		0x41,                     /* �A         */
			 0x45,                /*  �E        */
				  0x4A,0x4B,      /*    �J�K    */
						    0x5f  /*         �_ */
	};
	static byte jtokigo[] = {
			  ' ', '�', '�', ',', '.', '�', ':',
		 ';', '?', '!', '�', '�',0x00,0x00,0x00,
		 '^', '~', '_',0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00, '�',0x00,0x00, '/',
		0x00,0x00,0x00, '|',0x00,0x00,0x60,'\'',
		0x00,'\"', '(', ')',0x00,0x00, '[', ']',
		'{' ,'}' ,0x00,0x00,0x00,0x00, '�', '�',
		0x00,0x00,0x00,0x00, '+', '-',0x00,0x00,0x00,
		0x00, '=',0x00, '<', '>',0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0/*'\\'*/,
		 '$',0x00,0x00, '%', '#', '&', '*', '@'
	};
	static byte tojkana[] = {
		0x51,0x00,0x02,0x04,0x06,0x08,	/* � */
		0x42,0x44,0x46,0x22,0x00,		/* � */
		0x01,0x03,0x85,0x07,0x09,		/* � */
		0x8a,0x8c,0x8e,0x90,0x92,		/* � */
		0x94,0x96,0x98,0x9a,0x9c,		/* � */
		0x9e,0xA0,0xA3,0xA5,0xA7,		/* � */
		0x29,0x2a,0x2b,0x2c,0x2d,		/* � */
		0xAe,0xB1,0xB4,0xB7,0xBa,		/* � */
		0x3d,0x3e,0x3f,0x40,0x41,		/* � */
		0x43,0x45,0x47,					/* � */
		0x48,0x49,0x4a,0x4b,0x4c,		/* � */
		0x4e,0x52,0x00 					/* � */
	};
	static byte jtokana[] = {
		0x01,0x0b,0x02,0x0c,0x03,0x0d,0x04,0x0e,0x05,0x0f,		/* �� */
		0x10,0x90,0x11,0x91,0x12,0x92,0x13,0x93,0x14,0x94,		/* �� */
		0x15,0x95,0x16,0x96,0x17,0x97,0x18,0x98,0x19,0x99,		/* �� */
		0x1a,0x9a,0x1b,0x9b,0x09,0x1c,0x9c,0x1d,0x9d,0x1e,0x9e,	/* �� */
		0x1f,0x20,0x21,0x22,0x23,								/* �� */
		0x24,0xA4,0x64,0x25,0xA5,0x65,0x26,0xA6,0x66,			/* �� */
		0x27,0xA7,0x67,0x28,0xA8,0x68,
		0x29,0x2a,0x2b,0x2c,0x2d,								/* �� */
		0x06,0x2e,0x07,0x2f,0x08,0x30,							/* �� */
		0x31,0x32,0x33,0x34,0x35,								/* �� */
		0xff,0x36,0xff,0xff,0x00,0x37,0x8d
	};
	byte f;
	byte err_f;
	byte *js1;
	byte d;
	byte c;
	word jc;

	err_f = f = 0;
	js1 = jstr1;
	while ( (jc = *(js2++)) != '\0') {
		if (jc <= 0x80) {   			/* ���p�����̂Ƃ� */
			f = 0;
			if (jc <= 0x1f) {   		/* ���۰٥���� */
				;
			} else if (jc == 0x20) { 	/* ��߰� */
				if (opts & TOJSPC)
					jc = 0x8140;
			} else if (jc <= 0x2f) { 	/* �L�� */
				c = '!';
				goto KIGO;
			} else if (jc <= '9') {  	/* ���� */
				if (opts & TOJDGT)
					jc += 0x824f - '0';
			} else if (jc <= '@') {  	/* �L�� */
				c  = ':' - 15;
				goto KIGO;
			} else if (jc <= 'Z') {  	/* ��̧�ޯĂ̑啶�� */
				if (opts & TOJALPH)
					jc += 0x825f - '@';
			} else if (jc <= 0x60) { 	/* �L�� */
				c = '[' - 22;
				if (jc == '\\' && (opts & YENOFF))
					c = '\\' - 39;
				goto KIGO;
			} else if (jc <= 'z') {  	/* ��̧�ޯĂ̏����� */
				if (opts & TOJALPH)
					jc += 0x8280 - 0x60;
			} else if (jc <= 0x7e) { 	/* �L�� */
				c = '{' - 28;
KIGO:
				if (opts & TOJPUN)
					jc = 0x8100 + tojkigo[jc - c];
			}
		} else {
			if (jc <= 0x9f) {   		/* ���JIS1�޲Ė� */
				goto SJIS;
			} else if (jc == 0xa0) {
				;
			} else if (jc <= 0xa4) { 	/* ���p�����L�� */
				if (opts & TOJKPUN)
					jc = 0x8100 + tojkigo[jc - 0xA1/*'�'*/ + 32];
			} else if (jc == 0xa5 || jc == 0xb0) {/* ���p�Ȃ��Ă�||(�) */
				if (    (f == 1 && (opts & (TOJKATA|TOJHIRA)))
					|| (f != 1 && (opts & TOJKPUN))    	   )
				{
					jc = (jc == 0xa5) ? 0x8145 : 0x815b;
				}
			} else if (jc <= 0xdd) { 	/* ���p�� */
				f = 1;
				if (opts & (TOJKATA|TOJHIRA)) {
					c = tojkana[jc - 0xA6/*'�'*/];
					d = c & 0x7f;
					if (*js2 == 0xDE/*'�'*/ && (c & 0x80)) {
						++js2;
						if (jc == 0xB3/*'�'*/ && (opts & TOJKATA)
							&& ((opts & DAKUOFF) == 0)   ) {
							jc = 0x8394;
							goto KJ1;
						} else
							++d;
					} else if (*js2 == 0xDF/*'�'*/ 
						&& (jc >= 0xCA/*'�'*/ && jc <= 0xCE/*'�'*/)
						&& ((opts & DAKUOFF) == 0)   ) {
						++js2;
						d += 2;
					}
					if (opts & TOJKATA) {
						if (d > 0x3e)
							++d;
						jc = d + 0x8340;
					} else {
						jc = d + 0x829f;
					}
				}
			} else if (jc <= 0xdf) { 	/* ���p�����E������ */
				if ((f==1 && (opts&(TOJKATA|TOJHIRA))) || (opts&TOJKPUN) )
					jc = 0x8100 + tojkigo[jc - 0xDE/*'�'*/ + 37];
			} else if (jc <= 0xfc) { 	/* ���JIS */
SJIS:
				if ((c = *(js2++)) == '\0')			/* 2�޲Ėڂ�'\0' */
					break;/* exit while */
				else if (iskanji2(c) == 0) {		/* 2�޲Ėڂ��s���̂Ƃ�*/
					f = 2;
					err_f = 1;
					if (c < 0x08)
						continue;

					else if (c == 0x09)
						goto ERR1;
					else if (c <= 0x1f)
						continue;
					else if (c <= 0x3f) {
ERR1:
						--js2;
						continue;
					} else if (c == 0x7f || c >= 0xfd)
						continue;
				}
				if (jc == 0x81) {   	/* �S�p�L�� */
					if (c == 0x40 && (opts & JSPC2SPC)) {
						f = 2;
						*(js1++) = ' ';
						jc = ' ';
					} else if(c==0x5b||c==0x4a||c==0x4b||c==0x45) {
						/*		�[		�J			�K		�E   */
						if ( (f == 3 && (opts & JTOHIRA))
							||(f == 4 && (opts & JTOKATA))
							||(f != 3 && f != 4 && (opts & JTOKPUN)) )
						{
							goto JKIGO;
						} else {
							goto JCC;
						}
					} else if(c==0x41||c==0x42||c==0x75||c==0x76) {
						/*		�A		�B			�u		�v	*/
						if (opts & JTOKPUN)
							goto JKIGO;
						else
							goto JCC;
					} else if ( (c <= 0x97 && (opts & JTOPUNS)) ) {
						f = 2;
						if (opts & YENOFF) {
							if (c == 0x5f) { /*�_*/
								jc = '\\';
								goto KJ1;
							}
						} else {
							if (c == 0x8f) { /*��*/
								jc = '\\';
								goto KJ1;
							}
						}
JKIGO:
						if ((d = jtokigo[c - 0x40]) == 0x00)
							goto JCC;
						jc = d;
					} else
						goto JCC;
				} else if (jc == 0x82) {
					if (c >= 0x4f && c <= 0x58) { 	 /* �S�p���� */
						f = 2;
						if (opts & JTODGT)
							jc = c - 0x4f + '0';
						else
							goto JCC;
					} else if (c >= 0x60 && c <= 0x79) {/* �S�p��̧�ޯ� */
						f = 2;
						if (opts & JTOALPH)
							jc = c - 0x60 + 'A';
						else
							goto JCC;
					} else if (c >= 0x81 && c <= 0x9a) {/* �S�p��̧�ޯ� */
						f = 2;
						if (opts & JTOALPH)
							jc = c - 0x81 + 'a';
						else
							goto JCC;
					} else {
						f = 3;
						if (c >= 0x9f && c <= 0xf1) { /* �Ђ炪�� */
							if (opts & JTOHIRA) {
								d = c - 0x9f;
								goto JKANA;
							} else
								goto JCC;
						} else
							goto JCC;
					}
				} else if (jc == 0x83) {
					f = 4;
					if (c >= 0x40 && c <= 0x94) {/*����*/
						if (opts & JTOKATA) {
							d = c - 0x40;
							if (d > 0x3e)
								--d;
JKANA:
							d = jtokana[d];
							if (d == 0xFF)
								goto JCC;
							jc = (d & 0x3f) + 0xA6/*'�'*/;
							if (d & 0x80) {
								*(js1++) = (byte)jc;
								jc = 0xDE/*'�'*/;
							} else if (d & 0x40) {
								*(js1++) = (byte)jc;
								jc = 0xDF/*'�'*/;
							}
						} else
							goto JCC;
					} else
						goto JCC;
				} else {
					f = 2;
JCC: 				/* �ϊ����Ȃ��Ƃ� */
					*js1++ = (byte)jc;
					jc = c;
				}
			} /* else {
				;
			} */
		}
KJ1:
		if (jc > 0xff) {
			*(js1++) = (byte)(jc / 0x100);
			jc &= 0xff;
		}
		*(js1++) = (byte)jc;
	} /* end of while */

	*js1 = '\0';
	if (err_f)
		return NULL;
	else
		return jstr1;
}


#if 0
/*
 *  ���������Ղ낮���
 */
#include <stdio.h>
#include <stdlib.h>
#include <jctype.h>
/**********************/
byte Ibuf[1024];
byte Obuf[4096];

int main(argc,argv)
	int argc;
	byte *argv[];
{
	int  f;
	word c,d;
	word o;

	o = 0x7f7f;
	d = 0;
	f = -1;
	if (--argc > 0) {
		c = (byte)(*argv[1]);
		if (c == '0')
			f = d = 0;
		else if (c == '1')
			f = d = 0xDE;
		else if (c == '2')
			f = d = 0xDF;
		else if (c == '3')
			f = 0x100;
		else
			f = -1;
	}
	if (-- argc > 0) {
		o = htoi(argv[2]);
		printf("%04x\n",o);
	}

	if (f < 0) {
		while(fgets(Ibuf,1000,stdin)) {
			jstrcnv(Obuf,Ibuf,0xffff);
			fputs(Obuf,stdout);
		}
	} else if (f < 0xff) {
		for (c = 0;c < 0x100;c ++) {
			if (isprkana(c)) {
				Ibuf[0] = c;
				Ibuf[1] = d;
				Ibuf[2] = '\0';
				jstrcnv(Obuf,Ibuf,o);
				printf("%02x:%s\t",c,Obuf);
			}
		}
		printf("\n");
	} else {
		d = 0;
		for (c = 0x0840;c < 0xfcfc;c ++) {
			if (jiszen(c)) {
				Ibuf[0] = c / 0x100;
				Ibuf[1] = c % 0x100;
				Ibuf[2] = '\0';
				jstrcnv(Obuf,Ibuf,o);
				printf("%04x:%s\t",c,Obuf);
				if (++d == 8) {
					d = 0;
					printf("\n");
				}
			}
		}
		printf("\n");
	}
	return 0;
}
#endif
