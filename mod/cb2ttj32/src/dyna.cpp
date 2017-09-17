//Dynafont definition
//Lin,Y.J. 1993/2/10
#define _DOS
#include <afx.h> // Microsoft C/C++ specification
#include <iostream.h>
#include <iomanip.h>
#include <stdio.h>
#include <memory.h>
#include <dos.h>
#include <malloc.h>
#include <graph.h>
#include "dyna.h"

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

//This part is special to bitmap operator
//Lin, Y.J. 1993/2/19
unsigned char bitmap::mask[8] = { 0x80,0x40,0x20,0x10,8,4,2,1 };

bitmap::bitmap(short x, short y)
     {
     xs = x; ys = y; width = (x+7)/8;
     data = (unsigned char __far *)_fcalloc(width,y);
     valid = (data)?1:0;
     }

void bitmap::show_vga16(short x, short y, short color)
     {
     char *idata = (char *)calloc(1,4+width*4); // VGA 16 color compatibility
     *(short *)idata = xs;
     *(short *)(idata+2) = 1;
     for (short i = 0; i < ys; i++)
         {
         for (short j = 0; j < 4; j++)
             if (color & mask[7-j])
                _fmemcpy((char __far *)(idata+4+width*j),data+width*i,width);
         _putimage(x,i+y,(char __huge *)idata,_GPSET);
         }
     free(idata);
     }

void bitmap::hline(short x1, short y, short x2)
     {
     for (; (x1 & 7) && (x1 <= x2); x1++) set(x1,y);
     for (; (x1 <= x2) && (x1+7 <= x2); x1 += 8) data[y*width+x1/8] = 0xff;
     for (; x1 <= x2; x1++) set(x1,y);
     }
/*
//This part is a example to test dyna driver
//Lin,Y.J. 1993/2/11
#include <conio.h>
#define XSize 512
#define YSize 512
short ftype[20];

void main()
     {
     DynaFont fd;
     if (!fd.valid)
        { cout << "driver error or not install.\n"; return; }
     fd.query_fontset(1,(short __far *)ftype);
     cout << "Total " << *ftype  << " fonts:\n";
     for (short i = 1; i <= ftype[0]; i++)
         cout << ftype[i] << "\t";
     cout << "\n"; cout.flush();
     fd.set_type(1,Df_LMing);
     //fd.set_type(0,0);
     fd.set_size(1,XSize,YSize);
     short v = fd.get_font(0xc4,0xa3); // Yaw
     //short v = fd.get_font('A');
     if (v == -1)
        {cout << "Font can't get.\n"; return;}
     if (!_setvideomode(_SRES16COLOR))
        { cout << "Graphics mode can't occur.\n"; return; }
     bitmap bp(XSize, YSize);
     if (!bp.valid)
        { cout << "memory not enough.\n"; return; }
     _setcolor(9);
     do {
        short __far *index = fd.get_buffer();
        for (short i = 0; i < fd.line_count; i++)
            bp.hline(index[i*3+1],index[i*3],index[i*3+2]);
        if (v == 0x8000) break;
        v = fd.get_next();
        } while (v != -1);
     bp.show_vga16(0,0,9);
     _getch();
     _setvideomode(_DEFAULTMODE);
     }
*/
/*
//This is a example to test dyna bitmap
#include <conio.h>
#define XSize 512
#define YSize 512

void main()
     {
     DynaFont fd;
     if (!fd.valid)
        { cout << "driver error or not install.\n"; return; }
     bitmap bp(XSize,YSize);
     if (!bp.valid)
        { cout << "memory not enough.\n"; return; }
     fd.set_type(1,Df_LMing);
     //fd.set_type(0,0);
     fd.set_size(1,XSize,YSize);
     if (fd.get_bitmap(0xc4,0xa3,bp.getbuf(),(XSize+7)/8))
        { cout << "Font error.\n"; return; }
     if (!_setvideomode(_SRES16COLOR))
        { cout << "Graphics mode can't occur.\n"; return; }
     bp.show_vga16(0,0,10);
     _getch();
     _setvideomode(_DEFAULTMODE);
     }
*/
