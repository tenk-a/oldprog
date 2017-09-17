#define _DOS
#include <afx.h>
#include <iostream.h>
#include <stdio.h>
#include <graph.h>
#include <string.h>
#include <conio.h>

short modify[8][2] = { 0,-1, -1,-1, -1,0, -1,1, 0,1, 1,1, 1,0, 1,-1 };

void main(short argc, char *argv[])
     {
     if (argc < 3) { cout << "Usage:DFM bitmapfile freemanfile\n"; return; }
     FILE *fn = fopen(argv[1],"rb");
     FILE *fx = fopen(argv[2],"rb");
     if (!fn || !fx) { cout << "File open error\n"; return; }
     if (!_setvideomode(_SRES16COLOR))
        { cout << "VESA compatible driver is not installed.\n"; return; }
     char tag[5];
     fread(tag,1,4,fn); tag[4] = 0;
     _setcolor(2);
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
     _setcolor(9);
     for (;;)
         {
         _getw(fx); //skip parent
         if (feof(fx)) break;
         short x = _getw(fx);
         short y = _getw(fx);
         _moveto(x/2,y/2);
         short count = _getw(fx);
         for (short i = 0; i < count; i++)
             {
             unsigned char c = getc(fx);
             x += modify[c][0];
             y += modify[c][1];
             _lineto(x/2,y/2);
             }
         }
     fclose(fx);
     _getch();
     _setvideomode(_DEFAULTMODE);
     }
