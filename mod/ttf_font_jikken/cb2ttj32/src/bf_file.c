#include <stdio.h>
#include "ntuttf.h"

unsigned char get8(FILE	*fi)
     {
     return (unsigned char)((unsigned short)getc(fi) & 0xff);
     }

unsigned short get16(FILE *fi)
     {
     unsigned short i =	((unsigned short)getc(fi)) << 8;
     return ((((unsigned short)getc(fi)) & 0xff) | i);
     }

unsigned long get32(FILE *fi)
     {
     unsigned long i = 0L;
     short j;
     for (j = 0; j < 4;	j++)
    	 i = (i	<< 8) |	(((unsigned short)getc(fi)) & 0xff);
     return i;
     }

void put16(unsigned short s, FILE *fn)
     { putc(H(s),fn); putc(L(s),fn); }

void put32(unsigned long s, FILE *fn)
     { put16(HX(s),fn);	put16(LX(s),fn); }

