#define _DOS
#include <afx.h> //Microsoft C/C++ definition
#include <iostream.h>
#include <stdio.h>
#include <malloc.h>
#include <conio.h>
#include <memory.h>
#include <stdlib.h>
#include "fgx.h"
#include "ntuttf.h"

void main(short argc, char *argv[])
     {
     if (argc <= 3)
        { cout << "Usage: FGBMP ftype font_no filename\n"; return;}
     short ftype = atoi(argv[1]);
     unsigned short f_no = debig5(atoi(argv[2]));
     FILE *fn = fopen(argv[3],"wb");
     if (!fn) { cout << "File " << argv[3] << " open error.\n"; return; }
     TLfont fd;
     if (!fd.valid) { cout << "driver error or not install.\n"; return; }
     fd.set_cstyle((char)ftype);
     fd.set_estyle((char)ftype);
     fd.set_size(XSIZE,YSIZE);
     fd.set_ystart();
     fd.set_yend();
     short v = fd.get_font(L(f_no),H(f_no));
     if (v == -1) { cout << "Font error.\n"; return; }
     if (v & 4) //Line segment format
        {
        fwrite("ETFG",1,4,fn);
        do {
           unsigned char __far *index = fd.get_buffer();
           for (short y = fd.get_ystart(); y <= fd.get_yend(); y++)
               {
               unsigned short count=(unsigned short) (*index);
               if (count)
                  { _putw(y,fn); _putw(count,fn);}
               index++;
               for (unsigned short i = 0; i < count; i++)
                   {
                   _putw(*(short __far *)index,fn);
                   _putw(__min(*(short __far *)(index+2),XSIZE-1),fn);
                   index+=4;
                   }
               }
            if (fd.get_yend() >= YSIZE-1) break;
            fd.set_size(XSIZE,YSIZE);
            fd.set_ystart(fd.get_yend()-10/*+1*/);
            fd.set_yend();            //Bugs of large font
            v = fd.get_font();
            } while (v !=-1);
        }
     else if (v & 8)
          { // Dynalab large font format
          fwrite("DYNA",1,4,fn);
          do {
             short __far *index=(short __far *)fd.get_buffer();
             unsigned short count = *((unsigned short __far *) index);
             index+=2; //Skip count and flag
             for (unsigned short i = 0; i < count; i++)
                { _putw(*index++,fn); _putw(*index++,fn); _putw(*index++,fn); }
             if (*((unsigned short __far *)(fd.get_buffer()+2)) ==0x8000) break;
             v = fd.get_next();
             } while (v != -1);
          }
     else {cout << "Unknown flag\n";}
     fclose(fn);
     }
