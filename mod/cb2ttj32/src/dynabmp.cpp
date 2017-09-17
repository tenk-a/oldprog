//DYNABMP type #font file
#define _DOS
#include <afx.h> //Microsoft C/C++ definition
#include <iostream.h>
//#include <iomanip.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include "ntuttf.h"

// This first part is dirrect code from DynaLab databook

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

DynaFont::DynaFont()
     {
     static const char search_pattern[16] =
            { 0xc4,0xf9,0xee,0xe1,0xc4,0xf2,0xe9,0xf6,
              0xe5,0xf2,0x2d,0xcc,0xe1,0xf3,0xe5,0xf2};
     char __far *base = (char __far *)0x1000000L;
     char __huge *ends = (char __huge *)search_pattern;
     short x;
     do {
        x = 0;
        if (!_fmemcmp(base,(const char __far *)search_pattern,16)) break;
        x = 1;
        _FP_SEG(base)++;
        } while ((char __huge *)base < ends);
     if (x == 1) // End of search
        { valid = 0; return; }
     _FP_SEG(DynaSub) = _FP_SEG(base) - 2; // Get CS
     _FP_OFF(DynaSub) = 0x30;
     reset(); // Reset system
     Buf = (short __far *)_fcalloc(6,4096);
     valid = (Buf)?1:0;
     }

short DynaFont::set_type(short chr, short ftype)
      {
      short paras[2];
      paras[0] = chr;
      paras[1] = ftype;
      return DynaCall(SetFONTtype,(short __far *)paras);
      }

short DynaFont::set_size(short chr, short x, short y)
      {
      if (y == -1) y = x;
      short paras[3];
      paras[0] = chr;
      paras[1] = y; // y first
      paras[2] = x;
      return DynaCall(SetFONTsize,(short __far *)paras);
      }

short DynaFont::get_bitmap(unsigned char hb, unsigned char lb,
      unsigned char __far *buf, short width)
     {
     short paras[7];
     if (lb)
        { // CFont
        paras[0] = 1;
        paras[1] = from_big5(hb,lb);
        }
     else {
          paras[0] = 0;
          paras[1] = (short)hb;
          }
     paras[2] = paras[3] = 0;
     paras[4] = _FP_OFF(buf);
     paras[5] = _FP_SEG(buf);
     paras[6] = width;
     return DynaCall(GetBITMAPatn,(short __far *)paras);
     }

short DynaFont::get_font(unsigned char hb, unsigned char lb)
     {
     short paras[6];
     if (lb)
        { // CFont
        paras[0] = 1;
        paras[1] = from_big5(hb,lb);
        }
     else {
          paras[0] = 0;
          paras[1] = (short)hb;
          }
     paras[2] = _FP_OFF(Buf);
     paras[3] = _FP_SEG(Buf);
     paras[4] = paras[5] = 0;
     short r = DynaCall(GetRaster,(short __far *)paras);
     if (r) return -1; // Dyna error
     line_count = paras[4];
     return paras[5];
     }

short DynaFont::get_next()
     {
     short paras[4];
     paras[0] = _FP_OFF(Buf);
     paras[1] = _FP_SEG(Buf);
     paras[2] = paras[3] = 0;
     short r = DynaCall(GetNEXTraster,(short __far *)paras);
     if (r) return -1; // Dyna error
     line_count = paras[2];
     return paras[3];
     }

short DynaFont::query_fontset(short chr, short __far *buf)
     {
     short __far *tmp = (short __far *)&chr;
     return DynaCall(QueryFontSet,tmp,buf);
     }

short DynaFont::DynaCall(short para)
     {
     short (__far *ff)() = DynaSub;
     _asm mov ax,para
     return (*ff)();
     }

short DynaFont::DynaCall(short para, short __far *ppr)
     {
     unsigned short __ax, __ds, __si;
     short (__far *ff)() = DynaSub;
     _asm { // Save register
          mov __ds,ds
          mov __si,si
          mov ax,para
          lds si,ppr
          }
     __ax = (*ff)();
     _asm { //Restore register
          mov ds,__ds
          mov si,__si
          }
     return __ax;
     }

short DynaFont::DynaCall(short para, short __far *ppr, short __far *ppr2)
     {
     unsigned short __ax, __ds, __si, __di;
     short (__far *ff)() = DynaSub;
     _asm { // Save register
          mov __ds,ds
          mov __si,si
          mov __di,di
          mov ax,para
          lds si,ppr
          les di,ppr2
          }
     __ax = (*ff)();
     _asm { //Restore register
          mov ds,__ds
          mov si,__si
          mov di,__di
          }
     return __ax;
     }

short DynaFont::from_big5(unsigned char hb, unsigned char lb)
     {
     if ((hb >= 0xa1) && (hb <= 0xfe))
        {
        if ((lb >= 0xa1) && (lb <= 0xfe))
           return (hb - 0xa1)*94+(lb-0xa1);
        else if ((lb >= 0x21) && (lb <= 0x7e))
             return (hb - 0xa1)*94+(lb-0x21)+8836;
        }
     else if ((hb >= 0x81) && (hb <= 0xa0))
          {
          if ((lb >= 0xa1) && (lb <= 0xfe))
             return (hb - 0x81)*94+(lb-0xa1)+17672;
          else if ((lb >= 0x21) && (lb <= 0x7e))
               return (hb - 0x81)*94+(lb-0x21)+20680;
          }
     return 0;
     }

void main(short argc, char *argv[])
     {
     if (argc < 4) { cout << "Usage: DYNABMP type font_no filename\n"; return; }
     short ftype = atoi(argv[1]);
     unsigned short f_no = debig5(atoi(argv[2]));
     FILE *fn = fopen(argv[3],"wb");
     if (!fn) { cout << "File " << argv[3] << " open error.\n"; return; }
     fwrite("DYNA",1,4,fn);
     DynaFont fd; //Header
     if (!fd.valid) { cout << "driver error or not install.\n"; return; }
     fd.set_type(1,ftype);
     fd.set_size(1,XSIZE,YSIZE);
     short v = fd.get_font(L(f_no),H(f_no));
     if (v == -1) {cout << "Font can't get.\n"; return;}
     do {
        short __far *index = fd.get_buffer();
        for (short i = 0; i < fd.line_count; i++)
            { _putw(*index++,fn); _putw(*index++,fn); _putw(*index++,fn); }
        if (v == 0x8000) break;
        v = fd.get_next();
        } while (v != -1);
     fclose(fn);
     }
