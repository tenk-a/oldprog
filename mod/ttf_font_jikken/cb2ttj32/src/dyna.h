// This first part is dirrect code from DynaLab databook
//Lin,Y.J. 1993/2/10

#define SetFONTtype 0x0002
#define SetFONTsize 0x0003
#define ClrATTR 0x0010
#define GetBITMAPatn 0x0082
#define ResetSYS 0x8004
#define GetRaster 0x8092
#define GetNEXTraster 0x8093
#define QueryFontSet 0x0202

#define Df_LMing 0
#define Df_Black 1
#define Df_FSung 2
#define Df_MKai  3
#define Df_MMing 4
#define Df_BMing 5
#define Df_BBlk  6
#define Df_Lie   7
#define Df_BRond 8

// This second part is define driver class
#ifndef _INC_MALLOC
#include <malloc.h>
#endif

class DynaFont
     {
public:
     DynaFont();
     ~DynaFont() {_ffree(Buf);}
     short set_type(short chr, short ftype);
     short set_size(short chr, short x, short y = -1);
     short clr_attr() { return DynaCall(ClrATTR); }
     short get_bitmap(unsigned char hb, unsigned char lb,
           unsigned char __far *buf, short width); // lb == 0 means english
     short reset() { return DynaCall(ResetSYS); }
     short __far *get_buffer() { return Buf; }
     short get_font(unsigned char hb, unsigned char lb = 0);
     short get_next();
     short query_fontset(short chr, short __far *buf);
     short valid;
     short line_count;
private:
     short DynaCall(short para);
     short DynaCall(short para, short __far *ppr);
     short DynaCall(short para, short __far *ppr, short __far *ppr2);
     short from_big5(unsigned char hb, unsigned char lb);
     short (__far *DynaSub)();
     short __far *Buf;
     };

class bitmap
     {
public:
     bitmap(short x, short y);
     ~bitmap() { _ffree(data); }
     unsigned char __far *getbuf() { return data; }
     unsigned char test(short x, short y)
          { return (data[y*width+x/8] & mask[x&7]); }
     void set(short x, short y) { data[y*width+x/8] |= mask[x&7]; }
     void reset(short x, short y) { data[y*width+x/8] &= ~mask[x&7]; }
     void show_vga16(short x, short y, short color = 15);
     void hline(short x1, short y, short x2);
     short valid;
private:
     static unsigned char mask[8];
     short xs, ys, width;
     unsigned char __far *data;
     };
