#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <errno.h>
#include <alloc.h>
#include <io.h>
#include "def.h"

#define PTR_OFF(p)	((unsigned short) (p))
#define PTR_SEG(p)	((unsigned short)((unsigned long)(void far*)(p) >> 16))


char *FIL_ChgExt(char filename[], char *ext)
{
	char *p;

	p = strrchr(filename, '/');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr(p, '\\');	/*�S�p̧�ٖ����肾�Ƃ�΂����A����������܂�*/
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr( p, '.');
	if (p == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	} else {
		strcpy(p+1, ext);
	}
	return filename;
}

char *FIL_AddExt(char filename[], char *ext)
{
	char *p;

	p = strrchr(filename, '/');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr(p, '\\');
	if ( p == NULL) {
		p = filename;
	}
	if ( strrchr( p, '.') == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	}
	return filename;
}

char far *FIL_BaseNameFar(char far *adr)
{
	char far *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\') {
			adr = p + 1;
		}
		if (iskanji((*(unsigned char far *)p)) && *(p+1) ) {
			p++;
		}
		p++;
	}
	return adr;
}

char *FIL_BaseName_N(char *adr)
{
	char *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\') {
			adr = p + 1;
		}
		if (iskanji((*(unsigned char *)p)) && *(p+1) ) {
			p++;
		}
		p++;
	}
	return adr;
}

/*---------------------------------------------------------------------------*/

char *FIL_GetLt(int sw)
	// ��s����. 1�s��256�o�C�g����.
	// �܂� sw=�n���h���ŏ�����. sw=-1�łP�s�ǂݍ���.
{
	static char buf[302],*b,*p;
	static int l;
	static int hdl = -1;

	l = 0;
	b = p;
	if (sw != -1) {
		hdl = sw;
		memset(buf,0,302);
		p = buf;
		b = NULL;
	} else if (p) {
		if (*p == 0) {
  L1:
			b = buf;
			memset(buf+l,0,302-l);
			if (_read(hdl,buf+l,300-l) == 0) {
				p = NULL;
				if (l == 0)
					b = NULL;
				goto RET;
			}
		}
		if ((p = strchr(b,'\r')) != NULL) {
			*p++ = '\0';
			if (*p == '\n')
				*p++ = '\0';
		} else if ((p = strchr(b,'\n')) != NULL) {
			*p++ = '\0';
			b = p;
		} else {
			l = strlen(b);
			memmove(buf,b,l);
			goto L1;
		}
	}
  RET:
	return b;
}



/*---------------------------------------------------------------------------*/

#if 0

void far *MEM_Allocz(long n)
{
	void far *p;
	if (n == 0)
		return NULL;
	p = farmalloc(n + 12);
	if (p) {
		p = (void far *)(((unsigned long)p + 0x10000L) & 0xFFFF0000L);
	}
	return p;
}

int MEM_Freez(void far *p)
{
	/*if ((p & 0xffff) == 0)*/
	if ((short)p == 0)
	{
		p = (void far *)(((unsigned long)p - 0x10000L) | 0x0004);
	}
	farfree(p);
	return 0;
}
#endif

#if 0
void far *MEM_Callocz(int n, int sz)
{
	return MEM_Allocz(n*sz);
}

unsigned long MEM_CoreLeftz(void)
{
	unsinged long l;
	l=farcoreleft();
	if (l > 4) {
		l -= 4;
	} else {
		l = 0;
	}
	return l;
}

#endif

#if 0

unsigned MEM_Allocp(unsigned n)
{
	if (n == 0)
		return n;
	n = (unsigned)((unsigned long)farmalloc(n*16L+12) >> 16);
	if (n)
		n++;
	return n;
}

int MEM_Freep(unsigned n)
{
	if (n)
		farfree((void far *)(((unsigned long)(n-1) << 16)|0x0004) );
	return 0;
}

unsigned MEM_CoreLeftp(void)
{
	unsigned n;
	n = (unsigned)(farcoreleft() >> 4);
	if (n)
		--n;
	return n;
}
#else

unsigned MEM_Allocp(unsigned n)
{
	unsigned sg;

	if (n == 0)
		return 0;
	if (allocmem(n,&sg) != -1)
		return 0;
	return sg;
}

#if 1
int MEM_Freep(unsigned n)
{
	if (n)
		freemem(n);
	return 0;
}
#endif

unsigned MEM_CoreLeftp(void)
{
	unsigned sg,n;
	n = allocmem(0xffff,&sg);
	if (n)
		--n;
	return n;
}

#endif

/*---------------------------------------------------------------------------*/

#if 0
void Text_Sw(int mode)
	/*
	 * �e�L�X�g��� ON,OFF
	 */
{
	union REGS reg;

	reg.x.ax = (mode != 0) ? 0x0c00 : 0x0d00;
	int86(0x18, &reg, &reg);
}
#endif

void Grph_Show(int mode)
	/*
	 * �O���t�B�b�N��� ON,OFF
	 */
{
	union REGS reg;

	reg.x.ax = (mode != 0) ? 0x4000 : 0x4100;
	int86(0x18, &reg, &reg);
}

void Grph_DspPage(int n)
{
	union REGS inreg;
	union REGS outreg;

	inreg.x.ax = 0x4200;

	inreg.x.cx = (n == 0) ? 0xC000 : 0xD000;	/* Set 400line mode */
	int86(0x18, &inreg, &outreg);
}

void Grph_ActPage(int n)
{
	//outp(0xa6,n);
	outportb(0xa6,n);
}

void Grph_Analog(void)
{
	//outp( 0x6a, 1); //analog mode
	outportb(0x6a,1);
}


/* FIL_FindFirst/next ------------------------------------------------------*/
unsigned FIL_FindFirst(char* pathName, unsigned attr, FIL_FIND_T *pfind)
	/*�t�@�C���������J�n	ret 0:ok  other:MSDOS error code
	  pathName ... �����p�X��
	  attr	   ... �����t�@�C���A�g���r���[�g
	  pfind    ... �����o�b�t�@
	*/
{
	union REGS reg;
	struct SREGS sreg;

	/* set DTA */
	reg.h.ah = 0x1a;
	reg.x.dx = PTR_OFF(pfind);
	sreg.ds  = PTR_SEG(pfind);
	intdosx(&reg, &reg, &sreg);
	/* findfirst */
	reg.h.ah = 0x4e;
	reg.x.cx = attr;
	reg.x.dx = PTR_OFF(pathName);
	sreg.ds  = PTR_SEG(pathName);
	intdosx(&reg, &reg, &sreg);
	if (pfind->name[0] == 0x05)
			pfind->name[0] = (char)(0xe5); /* DOS 3.1�ȑO�̃o�O�΍� */
 #if defined(LSI_C)
	return (reg.x.flags & 1) ? (errno = reg.x.ax) : 0;
 #else
	return (reg.x.cflag) ? (errno = reg.x.ax) : 0;
 #endif
}

unsigned FIL_FindNext(FIL_FIND_T *pfind)
	/*FIL_FindFirst() �ɑ����ăt�@�C���������s��(DOS #4f)
	  ret 0:ok	other:MSDOS error code
	  pfind    ... �����o�b�t�@
	*/
{
	union REGS reg;
	struct SREGS sreg;

	/* set DTA */
	reg.h.ah = 0x1a;
	reg.x.dx = PTR_OFF(pfind);
	sreg.ds  = PTR_SEG(pfind);
	intdosx(&reg, &reg, &sreg);
	/* findnext */
	reg.h.ah = 0x4f;
	intdos(&reg, &reg);
	if (pfind->name[0] == 0x05)
			pfind->name[0] = (char)0xe5; /* DOS 3.1�ȑO�̃o�O�΍� */
 #if defined(LSI_C)
	return (reg.x.flags & 1) ? (errno = reg.x.ax) : 0;
 #else
	return (reg.x.cflag) ? (errno = reg.x.ax) : 0;
 #endif
}
