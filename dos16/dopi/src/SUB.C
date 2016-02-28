/*
	DoPI sub
 */

#include <string.h>
#include <io.h>
#include <dos.h>
#include <errno.h>
#include "dopi.h"
#if 0
unsigned _dos_read(int hno, void far *buf, unsigned siz, unsigned *cnt)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.al = 0x00;
    reg.h.ah = 0x3f;
    reg.x.bx = hno;
    reg.x.cx = siz;
    reg.x.dx = PTR_OFF(buf);
    sreg.ds  = PTR_SEG(buf);
    intdosx(&reg, &reg, &sreg);

 #if defined(LSI_C)
    if (reg.x.flags & 1)
 #else
    if (reg.x.cflag)
 #endif
    {
        *cnt = 0;
        return (errno = reg.x.ax);
    } else {
        *cnt = reg.x.ax;
        return 0;
    }
}
#endif

#if 0
unsigned _dos_write(int hno, void far *buf, unsigned siz, unsigned *cnt)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.al = 0x00;
    reg.h.ah = 0x40;
    reg.x.bx = hno;
    reg.x.cx = siz;
    reg.x.dx = PTR_OFF(buf);
    sreg.ds  = PTR_SEG(buf);
    intdosx(&reg, &reg, &sreg);

 #if defined(LSI_C)
    if (reg.x.flags & 1)
 #else
    if (reg.x.cflag)
 #endif
    {
        *cnt = 0;
        return (errno = reg.x.ax);
    } else {
        *cnt = reg.x.ax;
        return 0;
    }
}
#endif
/*---------------------------------------------------------------------------*/
/* 常駐パレット関係のルーチンは, mag.exe(Sam氏作) のソースを参考にしました */
struct MCB_t {
    byte  flag;
    word owner;
    word blksiz;
    byte reserve[11];
};

struct MCB_t far *_dos_mcbfirst(void)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.ah = 0x52;
    intdosx(&reg,&reg,&sreg);
    return MAKE_FP(*(word far *)MAKE_FP(sreg.es, reg.x.bx - 2), 0);
}

_S int far_strcmp(unsigned char far *s1, unsigned char far *s2)
{
    while (*s1 == *s2)
        if (*s1 == '\0')
            return 0;
        else {
            ++s1;
            ++s2;
        }
    if (*s1 > *s2)
        return 1;
    else
        return -1;
}

struct RAMPAL far *
RPal_Search(void)
{
    struct MCB_t far *p;
    word seg;

    p = _dos_mcbfirst();
    seg = PTR_SEG(p);
    for (;;) {
        if (p->owner != NULL) {
            if (far_strcmp((byte far *)p + 0x10, "pal98 grb") == 0)
                return (struct RAMPAL far *)((byte far *)p + 0x10);
        }
        seg += p->blksiz + 1;
        p = MAKE_FP(seg,0);
        if (p->flag == 'Z')
            break;
    }
    return NULL;
}


/*--------------------------------------------------------------------------*/
#if 1
void RPal_ChgTone(int tone)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) == NULL)
       	return;
    p->tone = tone;
    for (i = 0; i < 16; ++i) {
    	int r,g,b;
    	r = p->grb[i][1] * tone / 100; r = (r > 15) ? 15 : r;
    	g = p->grb[i][0] * tone / 100; g = (g > 15) ? 15 : g;
    	b = p->grb[i][2] * tone / 100; b = (b > 15) ? 15 : b;
        setGRB(i,g,r,b);
    }
}
#endif

void RPal_Set(int tone, byte *rgb)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) != NULL) {
        p->tone = tone;
        for (i = 0; i < 16; ++i) {
            p->grb[i][0] = rgb[i*3 + 1];
            p->grb[i][1] = rgb[i*3 + 0];
            p->grb[i][2] = rgb[i*3 + 2];
        }
    }
    for (i = 0; i < 16; ++i) {
    	int r,g,b;
    	r = rgb[i*3 + 0] * tone / 100; r = (r > 15) ? 15 : r;
    	g = rgb[i*3 + 1] * tone / 100; g = (g > 15) ? 15 : g;
    	b = rgb[i*3 + 2] * tone / 100; b = (b > 15) ? 15 : b;
        setGRB(i,g,r,b);
    }
}

int RPal_Get(word *tone, byte *rgb)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) == NULL)
        return -1;
	if (tone)
    	*tone = p->tone;
    for (i = 0; i < 16; ++i) {
        rgb[i * 3 + 0] = p->grb[i][1];
        rgb[i * 3 + 1] = p->grb[i][0];
        rgb[i * 3 + 2] = p->grb[i][2];
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
#if 0
void DspText(int mode)
	/*
	 * テキスト画面 ON,OFF
	 */
{
	union REGS reg;

	reg.x.ax = (mode != 0) ? 0x0c00 : 0x0d00;
	int86(0x18, &reg, &reg);
}

void DspGrph(int mode)
	/*
	 * グラフィック画面 ON,OFF
	 */
{
	union REGS reg;

	reg.x.ax = (mode != 0) ? 0x4000 : 0x4100;
	int86(0x18, &reg, &reg);
}

void DspSwitch(int t,int g)
{
	DspText(t);
	DspGrph(g);
}
#endif

#if 1
void DspGPage(int n)
{
	union REGS inreg;
	union REGS outreg;

	inreg.x.ax = 0x4200;

	inreg.x.cx = (n == 0) ? 0xC000 : 0xD000;	/* Set 400line mode */
	int86(0x18, &inreg, &outreg);
}
#endif

#if 1
void ActGPage(int n)
{
	outp(0xa6,n);
}
#endif

/*---------------------------------------------------------------------------*/
void ChgExt(byte *onam, byte *inam, byte *ext)
{
	byte *p;

	p = strncpy(onam,inam,80);
	if (*p == '.')
		p++;
	if (*p == '.')
		p++;
	if ((p = strrchr(p,'.')) == NULL) {
		p = onam;
		while(*p)
			p++;
	}
	if (ext == NULL||*ext == '\0') {
		*p = '\0';
	} else {
		*p++ = '.';
		strcpy(p,ext);
	}
}
#if 0
_S byte *ChgExtTmp(byte *s, byte *p)
{
	_S byte buf[130];

	ChgExt(buf,s,p);
	return buf;
}
#endif

/*---------------------------------------------------------------------------*/
