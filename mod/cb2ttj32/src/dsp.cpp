#define _DOS
#include <afx.h>
#include <iostream.h>
#include <stdio.h>
#include <graph.h>
#include <string.h>
#include <conio.h>
#include <math.h>
#include <stdlib.h>
#include "ntuttf.h"

extern "C"
     {
     void curve(short x0,short y0,short x1,short y1,short x2,short y2);
     }

void main(short argc, char *argv[])
     {
     if (argc < 3) { cout << "Usage:DSP bitmapfile splinefile\n"; return; }
     FILE *fn = fopen(argv[1],"rb");
     FILE *fx = fopen(argv[2],"rb");
     if (!fn || !fx) { cout << "File open error\n"; return; }
     if (!_setvideomode(_SRES16COLOR))
        { cout << "VESA compatible driver is not installed.\n"; return; }
     char tag[5];
     fread(tag,1,4,fn); tag[4] = 0;
     _setcolor(7);
     if (!strcmp(tag,"DYNA")) for (;;)
        {
        short y = _getw(fn)/2;
        if (feof(fn)) break;
        _moveto(_getw(fn)/2,y); _lineto(_getw(fn)/2,y);
        }
     else if (!strcmp(tag,"ETFG")) for (;;)
        {
        short y = _getw(fn)/2;
        if (feof(fn)) break;
        short count = _getw(fn);
        for (short i = 0; i < count; i++)
            { _moveto(_getw(fn)/2,y); _lineto(_getw(fn)/2,y); }
        }
     fclose(fn);
     short lx, ly, x1, y1, x0, y0, flag = 0;
     _setcolor(12);
     for (;;)
         {
         _getw(fx); //skip parent
         if (feof(fx)) break;
         short count = _getw(fx);
         for (short i = 0; i < count; i++)
             {
             short f = _getw(fx);
             short x = _getw(fx)/2;
             short y = _getw(fx)/2;
             if (!i) { _moveto(x,y); lx = x0 = x; ly = y0 = y; flag = 0;}
             else {
                  if ((f != Scontrol) && !flag) { _lineto(x,y); lx = x; ly = y;}
                  else if (f != Scontrol)
                       {
                       curve(lx,ly,x1,y1,x,y);
                       lx = x; ly = y; flag = 0;
                       }
                  else if (flag == 0) { x1 = x; y1 = y; flag = 1; }
                  }
             }
         if (flag) curve(lx,ly,x1,y1,x0,y0);
         else _lineto(x0,y0);
         }
     fclose(fx);
     _getch();
     _setvideomode(_DEFAULTMODE);
     }

void curve(short x0,short y0,short x1,short y1,short x2,short y2)
     {
     _lineto((x0+x1)/2,(y0+y1)/2);
     for (float t = 0, dt = 1/(float)__max(abs(x2-x0),abs(y2-y0));
         (dt > 0) && (t < 1); t+=dt)
         _lineto((short)((x1+x2)*t*t/2+2*t*(1-t)*x1+(x0+x1)*(t-1)*(t-1)/2),
                 (short)((y1+y2)*t*t/2+2*t*(1-t)*y1+(y0+y1)*(t-1)*(t-1)/2) );
     _lineto(x2,y2);
     }
