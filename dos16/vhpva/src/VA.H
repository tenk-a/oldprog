/* VA : MSE ÇÃ ”∞ƒﬁ */
#define _VA_NOMSE   -1
#define _VA_NATIVE  0
#define _VA_STD     1
#define _VA_98TEXT  2
#define _VA_98MONO  3
#define _VA_98GRPH  4

/* text-vram,grph-vram ±∏æΩ”∞ƒﬁêÿÇËä∑Ç¶ */
#define acs_vram()  outp(0x153,(inp(0x153) & 0xf0) | 4);\
				 /* outp(0x510, 0) *//* ...? */
#define acs_text()  outp(0x153,(inp(0x153) & 0xf0) | 1)
/* outp(0x124,0) ... ? */

#define VRAM_B   (0xa0000000L)
#define VRAM_R   (0xb0000000L)
#define VRAM_G   (0xc0000000L)
#define VRAM_I   (0xd0000000L)

#define VRAM98_B (0xa8000000L)
#define VRAM98_R (0xb0000000L)
#define VRAM98_G (0xb8000000L)
#define VRAM98_I (0xe0000000L)

int  Va_GetMseMode(void);
int  Va_SetMseMode(int mode);

void Va_puts(void far*);
void Va_Puts(void far*);

int  Va_ScreenWidth(int mode,int w0,int c0,int w1,int c1);
int  Va_Compose(int p1, int p2, int p3, int p4,int col);
int  Va_Palette(int n,unsigned data);
int  Va_PalGRB(int n,int g,int r,int b);
int  Va_PaletteMode(int mode,int page,int bm,int bd);
int  Va_PaletteInit(void);
int  Va_Screen(int f,int act,int dsp);
