#define _DOS
#include <afx.h>
#include <afxcoll.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "ntuttf.h"
#include "array.h"

extern "C"
     {
     void save_default(FILE *fo);
     void save_coor(FILE *fn,CObArray &ary);
     }

void main(short argc, char *argv[])
     {
     if (argc < 3) { cout << "Usage: TOGLYF splinefile glyfile"; return; }
     FILE *fi = fopen(argv[1],"rb");
     FILE *fo = fopen(argv[2],"wb");
     if (!fi || !fo) { cout << "File open error.\n"; return; }
     CObArray ary;
     for (;;)
         {
         coorarray *ct = new coorarray;
         ct->parent = _getw(fi);
         if (feof(fi)) break;
         short count = _getw(fi);
         for (short i = 0; i < count; i++)
             {
             short flag = _getw(fi);
             short x = _getw(fi);
             short y = (YSIZE - _getw(fi)) - DESCENT; //baseline
             ct->add(x,y,flag);
             }
         ary.Add(ct);
         }
     fclose(fi);
     if (!ary.GetSize()) { save_default(fo); return; }
     else {
          bigfirst head;
          head.addshort(ary.GetSize());
          head.addshort(0);
          head.addshort((unsigned short)(-1*DESCENT));
          head.addshort(XSIZE);
          head.addshort(YSIZE-DESCENT);
          for (short i = 0, noCnt = -1; i < ary.GetSize(); i++)
              {
              short j = ((coorarray *)ary[i])->getlen();
              if (j) noCnt += j;
              else noCnt++; //At least one point exists in each contour
              head.addshort(noCnt);
              }
          head.addshort(0);
          for (unsigned short j = 0; j < head.getlen(); j++) putc(head[j],fo);
          }
     save_coor(fo, ary);
     for (short i = 0; i < ary.GetSize(); i++)
         {
         short j = ((coorarray *)ary[i])->parent;
         putc(H(j),fo); putc(L(j),fo);
         }
     fclose(fo);
     }

void save_default(FILE *fo)
     {
     bigfirst data;
     data.addshort(1);
     data.addshort(0);
     data.addshort(-1*DESCENT);
     data.addshort(XSIZE);
     data.addshort(YSIZE-DESCENT);
     data.addshort(0);
     data.addshort(0);
     data.addbyte(0x37); //On curve, both x,y are positive short
     data.addbyte(1);
     data.addbyte(1);
     data.addshort(-1); //for parent add
     for (unsigned short i = 0; i < data.getlen(); i++) putc(data[i],fo);
     fclose(fo);
     }

void save_coor(FILE *fn,CObArray &ary)
     {
     coorarray ct;
     for (short i = 0; i < ary.GetSize(); i++)
         {
         coorarray *tmp = (coorarray *)ary[i];
         if (!tmp->getlen()) ct.add(1,500,500);  //Null contour
         else for (short j = 0; j < tmp->getlen(); j++)
              ct.add((*tmp)[j].x,(*tmp)[j].y,((*tmp)[j].flag == Scontrol)?0:1);
         }
     for (i = ct.getlen()-1; i >= 0; i--)
         {
         struct coor tmp = ct[i];
         if (i) { tmp.x -= ct[i-1].x; tmp.y -= ct[i-1].y;}
         if (tmp.x == 0) tmp.flag |= 0x10; //same
         else if (abs(tmp.x) < 256)
              {
              if (tmp.x > 0) tmp.flag |= 0x12; //positive short
              else tmp.flag |= 2; //negtive short
              }
         if (tmp.y == 0) tmp.flag |= 0x20; //same
         else if (abs(tmp.y) < 256)
              {
              if (tmp.y > 0) tmp.flag |= 0x24; //positive short
              else tmp.flag |= 4; //negtive short
              }
         ct.set(i,tmp);
         }
     for (i = 0; i < ct.getlen(); i++) putc(ct[i].flag,fn);
     for (i = 0; i < ct.getlen(); i++)  //X
         {
         if (!ct[i].x) continue;
         else if (abs(ct[i].x) < 256) putc(abs(ct[i].x),fn);
         else { putc(H(ct[i].x),fn); putc(L(ct[i].x),fn); }
         }
     for (i = 0; i < ct.getlen(); i++) //Y
         {
         if (!ct[i].y) continue;
         else if (abs(ct[i].y) < 256) putc(abs(ct[i].y),fn);
         else { putc(H(ct[i].y),fn); putc(L(ct[i].y),fn); }
         }
     }
