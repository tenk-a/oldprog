//FG.CPP Fg subroutines
//Lin, Y.J 1993/2/9
#define _DOS
#include <afx.h> //Microsoft C/C++
#include <iostream.h>
#include <stdio.h>
#include <dos.h>
#include <malloc.h>
#include <memory.h>
#include "fgx.h"

TLfont::TLfont()
     {
     char __far *vector = (char __far *)_dos_getvect(0x66); // Get fg interrupt
     _FP_OFF(vector) = 0;
     if (_fmemcmp(vector+0x10,"TEgf",4))
        {
        valid = 0; // Not installed
        return;
        }
     _FP_OFF(FgSub) = *((short __far *)(vector+0x17));
     _FP_SEG(FgSub) = *((short __far *)(vector+0x19));
     short (__far *ff)() = FgSub; // Call by stack
     _asm mov ax,Fg_Reset
     (*ff)(); // Call by stack
     set_mode();    // Set default value
     set_mask();
     set_attr();
     set_wexp();
     set_hexp();
     set_net();
     set_color();
     set_degree();
     fgfont.ftp = &font;
     FgBuf = ((unsigned char __far *)_fcalloc(45,1024)+4);
     valid = (FgBuf)?1:0; // 0 when allocate fail
     }

short TLfont::get_font(char hb, char lb)
     {
     fgfont.hb = hb;
     if (lb)
        {
        fgfont.lb = lb;
        fgfont.flag = 0; //Chinese
        }
     else fgfont.flag = 1; //English
     return FgCall(Fg_FontGet,(char __far *)((FgFont __far *)&fgfont),(char __far *)FgBuf);
     }

short TLfont::FgCall(short para, char __far *par1, char __far *par2)
     {
     //Call FgSub
     unsigned short __ds, __si, __di, __ax;
     short (__far *ff)() = FgSub; // Call by stack
     _asm { // Save register and set value
          mov __ds,ds
          mov __si,si
          mov __di,di
          mov ax,para
          lds si,par1
          les di,par2
          }
     __ax = (*ff)();
     _asm {
          mov ds,__ds
          mov si,__si
          mov di,__di
          } //restore register
     return __ax;
     }
/*
//This is a test font program
#include <graph.h>
#include <conio.h>
#define XSize 640
#define YSize 480

void main()
     {
     TLfont fd;
     if (!fd.valid)
        { cout << "driver error or not install.\n"; return; }
     fd.set_cstyle(Ft_Kai);
     fd.set_estyle(Ft_F0);
     fd.set_size(XSize,YSize);
     fd.set_ystart();
     fd.set_yend();
     short v = fd.get_font(0xc4,0xa3);
     if (v == -1)
        { cout << "font not found.\n"; return; }
     if (!_setvideomode(_VRES16COLOR))
        { cout << "Graphics mode can't occur.\n"; return; }
     _setcolor(9);
     if (v & 4) //Line segment format
        {
        do {
           unsigned char __far *index = fd.get_buffer();
           for (short y = fd.get_ystart(); y <= fd.get_yend(); y++)
               {
               unsigned short count=(unsigned short) (*index);
               index++;
               for (unsigned short i = 0; i < count; i++)
                   {
                   _moveto(*((short __far *) index),y);
                   index+=2;
                   _lineto(*((short __far *) index),y);
                   index+=2;
                   }
               }
            if (fd.get_yend() >= YSize-1) break;
            fd.set_size(XSize,YSize);
            fd.set_ystart(fd.get_yend()+1);
            fd.set_yend();
            v = fd.get_font();
            } while (v !=-1);
        }
     else if (v & 8)
          { // Dynalab large font format
          do { // No test because hard to let FG use this format
             short __far *index=(short __far *)fd.get_buffer();
             unsigned short count = *((unsigned short __far *) index);
             index+=2;
             for (unsigned short i = 0; i < count; i++)
                 {
                 short y  = *index++;
                 _moveto(*index++,y);
                 _lineto(*index++,y);
                 }
             if (*((unsigned short __far *)(fd.get_buffer()+2)) ==0x8000) break;
             v = fd.get_next();
             } while (v != -1);
          }
     else { //bitmap
          short width = (fd.get_xsize()+7)/8;
          char *idata = (char *)calloc(1,4+width*4); // VGA 16 color compatibility
          *(short *)idata = fd.get_xsize();
          *(short *)(idata+2) = 1;
          for (short i = 0; i < YSize; i++)
              {
              _fmemcpy((char __far *)(idata+4),fd.get_buffer()+width*i,width);
              _fmemcpy((char __far *)(idata+4+width*3),fd.get_buffer()+width*i,width);
              _putimage(0,i,(char __huge *)idata,_GPSET);
              }
          free(idata);
          }
     _getch();
     _setvideomode(_DEFAULTMODE);
     }
*/
