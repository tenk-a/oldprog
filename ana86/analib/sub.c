#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <errno.h>
#include "def.h"

#define PTR_OFF(p)	((unsigned short) (p))
#define PTR_SEG(p)	((unsigned short)((unsigned long)(void far*)(p) >> 16))


BYTE *ChgExt(BYTE filename[], BYTE *ext)
{
	BYTE *p;

	p = strrchr(filename, '/');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr(p, '\\');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr( p, '.');
	if (p == NULL) {
		strcat(filename,".");
		strcat( filename, ext);
	} else {
		strcpy(p+1, ext);
	}
	return filename;
}

BYTE *AddExt(BYTE filename[], BYTE *ext)
{
	BYTE *p;

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

unsigned char far *file_BaseNameFar(unsigned char far *adr)
{
	unsigned char far *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\') {
			adr = p + 1;
		}
		if (iskanji(*p) && *(p+1) ) {
			p++;
		}
		p++;
	}
	return adr;
}

#if 0
void Text_Sw(int mode)
	/*
	 * テキスト画面 ON,OFF
	 */
{
	union REGS reg;

	reg.x.ax = (mode != 0) ? 0x0c00 : 0x0d00;
	int86(0x18, &reg, &reg);
}
#endif

void Grph_Show(int mode)
	/*
	 * グラフィック画面 ON,OFF
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
	outp(0xa6,n);
}

void Grph_Analog(void)
{
	outp( 0x6a, 1); //analog mode
}


/* _dos_findfirst/next ------------------------------------------------------*/
unsigned _dos_findfirst(char* pathName, unsigned attr, struct find_t *pfind)
	/*ファイル検索を開始	ret 0:ok  other:MSDOS error code
	  pathName ... 検索パス名
	  attr	   ... 検索ファイルアトリビュート
	  pfind    ... 検索バッファ
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
			pfind->name[0] = 0xe5; /* DOS 3.1以前のバグ対策 */
 #if defined(LSI_C)
	return (reg.x.flags & 1) ? (errno = reg.x.ax) : 0;
 #else
	return (reg.x.cflag) ? (errno = reg.x.ax) : 0;
 #endif
}

unsigned _dos_findnext(struct find_t *pfind)
	/*_dos_findfirst() に続いてファイル検索を行う(DOS #4f)
	  ret 0:ok	other:MSDOS error code
	  pfind    ... 検索バッファ
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
			pfind->name[0] = 0xe5; /* DOS 3.1以前のバグ対策 */
 #if defined(LSI_C)
	return (reg.x.flags & 1) ? (errno = reg.x.ax) : 0;
 #else
	return (reg.x.cflag) ? (errno = reg.x.ax) : 0;
 #endif
}
