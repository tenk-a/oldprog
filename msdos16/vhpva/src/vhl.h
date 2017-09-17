#ifndef _TENKAFUN_H_
#include "tenkafun.h"
#endif

#ifndef EXT
#define EXTERN extern
#else
#define EXTERN
#endif

#define EX_IN_BUF   /* 大きな入力ﾊﾞｯﾌｧを確保 */

#ifdef PC88VA
  #include "va.h"
  #define setGRB(n,g,r,b) Va_PalGRB((n),(g),(r),(b))
  #define puts(n)   	  Va_puts(n)
  void vaInit(void);
  void vaEnd(void);
    #ifdef KAIHATU
      EXTERN int vaVramModeFlg;
    #endif
#else /* 98 */
  #define setGRB(n,g,r,b) do{\
    	      outp(0xa8, (n));\
    	      outp(0xaa, (g));\
    	      outp(0xac, (r));\
    	      outp(0xae, (b));\
    	  }while(0)
  #define VRAM_B 0xa8000000L	/* Vram Blue  Base Pointer */
  #define VRAM_R 0xb0000000L	/* Vram Red   Base Pointer */
  #define VRAM_G 0xb8000000L	/* Vram Green Base Pointer */
  #define VRAM_I 0xe0000000L	/* Vram I     Base Pointer */
#endif

void dspSwitch(int t, int g);
void GLinit(int h, int c, int toon, byte *rgb);

/* RGBI データ */
EXTERN byte Patb1, Patr1, Patg1, Pati1;
EXTERN byte Patb2, Patr2, Patg2, Pati2;
EXTERN byte Patb,  Patr,  Patg,  Pati;
EXTERN byte Rpatb, Rpatr, Rpatg, Rpati;
/* 最後に使ったパターンの2色を退避(pass2) */
EXTERN int  Ucol1, Ucol2;
/* 8色 or 16色 */
EXTERN int  Color16;
/* ペイント */
EXTERN int  Offy;
/* ビットマップテーブル */
EXTERN int  Ly;
#ifdef TST8BIT	/* tiny modelでのｺﾝﾊﾟｲﾙを試してみるとき */
  #ifdef EX_IN_BUF
  #undef EX_IN_BUF
  #endif
  /* Vbufは実際には下2bitsしか使われていないので、それをつめてVbufのサイズを
    小さく(1/4)する。ただしVbufに対するｱｸｾｽが余計に時間がかかることになる。*/
  EXTERN byte Vbuf[LMAXY][LMAXX/4];
  int  getVb(int x,int y);
#else
  EXTERN byte Vbuf[LMAXY][LMAXX];   	    	/* 4*2 ビットマップバッファ */
  #define getVb(x,y)	(Vbuf[y][x])
#endif

void get4bitpat(int x, int y, int w, byte *pb, byte *pr, byte *pg, byte *pi);
void convpat(void);
void putpat2(int x,int y);
void vhload(byte *load_name,word toon,byte rgb_flg);
void error(int);
void putPalFile(int sw_r,byte *grb,byte *fname);
void pass22(int x, int y, int w);

word getdata(void);
word getbits(int n);
