#define _DOS
//#include <afx.h>
#include <iostream.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "ntuttf.h"

struct ttfTB
     {
     unsigned long chk, off, len, seat;
     } glyf, loca;

extern "C"
     {
     void build_ref(FILE *fn);
     void get_table(struct ttfTB *tbl, short i, FILE *fn);
     void modify(short eng,FILE *fn);
     void getXrange(short num,FILE *fn);
     }

void main(short argc, char *argv[])
     {
     if (argc < 2) { cout << "Usage: ENGPATCH ttfile\n"; return; }
     FILE *fn = fopen(argv[1],"rb+");
     if (!fn) { cout << "File open error.\n"; return; }
     build_ref(fn);
#ifdef HANKANA
     for (short i = 2; i < 96+64; i++) modify(i,fn);
#else
     for (short i = 2; i < 96; i++) modify(i,fn);
#endif
     }

void build_ref(FILE *fn)
     {
     fseek(fn,4L,SEEK_SET);
     short numTbl = get16(fn);
     fseek(fn,12L,SEEK_SET);
     for (short i = 0; i < numTbl; i++)
         {
         char tmp[5];
         fread(tmp,1,4,fn);
         tmp[4] = 0;
         if (!strcmp(tmp,"glyf"))      get_table(&glyf,i,fn);
         else if (!strcmp(tmp,"loca")) get_table(&loca,i,fn);
         else { get32(fn); get32(fn); get32(fn); } //skip
         }
     }

void get_table(struct ttfTB *tbl, short i, FILE *fn)
     {
     tbl->chk = get32(fn);
     tbl->off = get32(fn);
     tbl->len = get32(fn);
     tbl->seat = 12L+(long)i*16L;
     }

short xmin, xmax;

void modify(short eng,FILE *fn)
     {
     float scale = 1;
     fseek(fn,loca.off+(unsigned long)eng*4L,SEEK_SET);
     unsigned long fs = glyf.off + get32(fn);
     fseek(fn,fs+12L,SEEK_SET);
     short gidx = get16(fn); //Full font index
     getXrange(gidx,fn);  //File seek will moved
//cout << eng << " " << xmin << " " << xmax << "\n"; cout.flush();
     short shift = xmin * -1;
     if (xmax - xmin > (XSIZE/2)) scale = (float)sqrt((double)(XSIZE/2)/(double)(xmax-xmin)); //bug of cwin
     else shift = (XSIZE/4)-(xmin+xmax)/2; //Fit grid center
     fseek(fn,fs+14L,SEEK_SET);
     put16((unsigned short)((float)shift*scale),fn);
     put16(0,fn);
     put16((unsigned short)(scale*16384.0),fn);
     put16(0x4000,fn);
     }

void getXrange(short num,FILE *fn)
     {
     fseek(fn,loca.off+(unsigned long)num*4L,SEEK_SET);
     fseek(fn,glyf.off+get32(fn),SEEK_SET);
     short noCnt = get16(fn);
     get16(fn); get16(fn); get16(fn); get16(fn); //XYmin,max
     for (short i = 0, lastCnt = 0; i < noCnt; i++) lastCnt = get16(fn);
     get16(fn); //#inst
     unsigned char *flag = new unsigned char[lastCnt+1]; //No compress flag
     fread(flag,1,lastCnt+1,fn);
     xmin = XSIZE; xmax = 0;
     short x = 0;
     for (i = 0; i <= lastCnt; i++)
         {
         unsigned char c;
         switch (flag[i] & 0x12)
                {
                case 0: x += get16(fn); //long delta
                        break;
                case 2: c = getc(fn); //Negtive
                        x -= (short)c; break;
                case 0x12: c = getc(fn); //positive
                        x += (short)c; break;
                }
         if (x < xmin) xmin = x;
         if (x > xmax) xmax = x;
         }
     delete flag;
//if ((xmin < 0) || (xmax > XSIZE)) {cout << "Error!\n"; cout.flush();}
     }
