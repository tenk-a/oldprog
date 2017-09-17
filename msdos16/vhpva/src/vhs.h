#ifndef _TENKAFUN_H_
#include "tenkafun.h"
#endif

#ifndef EXT
#define EXTERN extern
#else
#define EXTERN
#endif

#ifdef PC88VA
  #include "va.h"
  #define puts(n)  Va_puts(n)
#else
  #define VRAM_B (0xa8000000L)
  #define VRAM_R (0xb0000000L)
  #define VRAM_G (0xb8000000L)
  #define VRAM_I (0xe0000000L)
#endif

EXTERN byte Patb1, Patr1, Patg1, Pati1;
EXTERN byte Patb2, Patr2, Patg2, Pati2;
EXTERN int Color16;
EXTERN int Ly;

void swapvram1(void);
void swapvram2(void);
void getpat(int x, int y);
void get4bitpat(int x, int y, int w, byte *pb, byte *pr, byte *pg, byte *pi);
int  _cmppat1(int x, int y);
int  _cmppat2(int x, int y);
void va_init(void);
