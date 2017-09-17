#include <stddef.h>
#include <dos.h>
#include <stdlib.h>
#include "tenkafun.h"
#include "va.h"

/*---------------------------------------------------------------------------*/

void
Va_Puts(void far *p)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.ah = 0x02;
    reg.x.dx = 0x8000;
    reg.x.si = PTR_OFF(p);
    sreg.ds  = PTR_SEG(p);
    int86x(0x83,&reg,&reg,&sreg);
}

void
Va_puts(void far *p)
{
    Va_Puts(p);
    Va_Puts("\r\n");
}

#if 0
void
va_reg(union REGS *reg)
{
    static char _Buf[200];
    sprintf(_Buf,"AX:%04x CX:%04x DX:%04x\r\n",reg->x.ax,reg->x.cx,reg->x.dx);
    Va__puts(_Buf);
}
#endif

/*------------------------------------------------------------------------*/
int
Va_GetMseMode(void)
{
    union REGS reg;

    reg.x.cx = -1;
    reg.x.ax = 0x44e5;
    reg.h.dl = 0xff;
    intdos(&reg,&reg);
    if ((int)(reg.x.cx) != -1) {
    	return ((reg.h.dh <  0x40) ? _VA_STD   :
    	       ((reg.h.dh <  0x80) ? _VA_98TEXT :
    	       ((reg.h.dh == 0x80) ? _VA_98MONO: _VA_98GRPH) ) );
    }
    reg.x.ax = 0x7f00;
    intdos(&reg,&reg);
    reg.x.cx = -1;
    reg.x.ax = 0x44e5;
    reg.h.dl = 0xff;
    intdos(&reg,&reg);
    if ((int)(reg.x.cx) == -1)
    	return _VA_NOMSE;
    reg.x.ax = 0x7f01;
    intdos(&reg,&reg);
    return _VA_NATIVE;
}


int
Va_SetMseMode(int mode)
{
    union REGS reg;

    if (mode == _VA_NATIVE) {
    	reg.x.ax = 0x7f01;
    	intdos(&reg,&reg);
    	return 0;
    }
    reg.x.ax = 0x7f00;
    intdos(&reg,&reg);
    reg.x.cx = 0;
    reg.x.ax = 0x44e5;
    reg.h.dl = 0x00;
    reg.h.dh =	((mode == _VA_STD)    ? 0x00 :
    	    	((mode == _VA_98TEXT) ? 0x45 :
    	    	((mode == _VA_98MONO) ? 0x80 :
    	    	((mode == _VA_98GRPH) ? 0x85 : 0xFF) ) ) );
    if (reg.h.dh == 0xFF)
    	return -1;
    intdos(&reg,&reg);
    return 0;
}


/*-------- Va_GetMseMode(),Va_SetMseMode() テスト・プログラム. -----------*/
#if 0
#include <stdio.h>

void
dispMode(int mode)
{
    char *p;

    switch(mode) {
    case _VA_NOMSE:
    	Va_Puts("Native mode.(MSEが常駐していません)\r\n");
    	break;
    case _VA_NATIVE:
    	Va_Puts("Native mode.\r\n");
    	break;
    case _VA_STD:
    	p = "Standard";
    	goto J1;
    case _VA_98TEXT:
    	p = "98 Text";
    	goto J1;
    case _VA_98MONO:
    	p = "98 Monochrome";
    	goto J1;
    case _VA_98GRPH:
    	p = "98 Graphics";
J1:
    	printf("%s mode\n",p);
    	break;
    default:
    	printf ("Programer's Error!\n");
    }
}



int
main(int argc,char *argv[])
{
    int  mode;
    int  i;

    mode = Va_GetMseMode();
    if (mode == _VA_NOMSE || argc <= 1) {
    	dispMode(mode);
    	return 0;
    }

    mode = 0x100;
    for (i = 1;i < argc;i++) {
    	if (*(argv[i]) == '-') {
    	    switch (*(argv[i] + 1)) {
    	    case 'n':case 'N': mode = _VA_NATIVE;break;
    	    case 'v':case 'V': mode = _VA_STD	;break;
    	    case 't':case 'T': mode = _VA_98TEXT;break;
    	    case 'm':case 'M': mode = _VA_98MONO;break;
    	    case 'g':case 'G': mode = _VA_98GRPH;break;
    	    }
    	}
    }
    Va_SetMseMode(mode);
    dispMode(Va_GetMseMode());
    return 0;
}
#endif
