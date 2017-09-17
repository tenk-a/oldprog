#define _DOS
#include <afx.h>
#include <afxcoll.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include "ntuttf.h"
#include "array.h"

#define RefPt 0x80
#define CORNER 0x40

extern "C"
     {
     void get_vector(FILE *fn, coorarray &ct);
     void set_refpt(unsigned short count, array &tmp);
     void curve_detect(array &ct);
     void adjust_corner(array &ct);
     void write_file(FILE *fn,CObArray &ary);
     short testline(short x, short y, coorarray &lt);
     short level(CObArray &ary, short idx);
     }

short modify[8][2] = { 0,-1, -1,-1, -1,0, -1,1, 0,1, 1,1, 1,0, 1,-1 };
float LineThsd = 1.001;

void main(short argc, char *argv[])
     {
     if (argc < 3) { cout << "Usage: VECTOR freemanfile vectorfile [LineThreashold]\n"; return; }
     FILE *fi = fopen(argv[1],"rb");
     FILE *fo = fopen(argv[2],"wb");
     if (!fi || !fo) { cout << "File open error\n"; return; }
     if (argc >= 4) LineThsd = (float)atof(argv[3]);
     CObArray ary;
     for (;;)
         {
         short prt = _getw(fi);
         if (feof(fi)) break;
         coorarray *tmp = new coorarray;
         tmp->parent = prt;
         get_vector(fi,*tmp);
         ary.Add(tmp); //Maybe empty when error
         }
     fclose(fi);
     write_file(fo,ary);
     fclose(fo);
     }

void write_file(FILE *fn,CObArray &ary)
     {
//cout << "Write file\n"; cout.flush();
     for (short i = 0; i < ary.GetSize(); i++)
         {
         coorarray *tmp = (coorarray *)ary[i];
         _putw(tmp->parent,fn);
         _putw(tmp->getlen(),fn);
         if (level(ary,i) & 1) for (short j = 0; j < tmp->getlen(); j++)
            {
            _putw((*tmp)[j].flag,fn);
            _putw((*tmp)[j].x,fn);
            _putw((*tmp)[j].y,fn);
            }
         else {
              _putw((*tmp)[0].flag,fn);
              _putw((*tmp)[0].x,fn);
              _putw((*tmp)[0].y,fn);
              for (short j = tmp->getlen()-1; j > 0; j--)
                  {
                  _putw((*tmp)[j].flag,fn);
                  _putw((*tmp)[j].x,fn);
                  _putw((*tmp)[j].y,fn);
                  }
              }
         }
     }

void get_vector(FILE *fn, coorarray &ct)
     {
//cout << "Get vector 1\n"; cout.flush();
     short x = _getw(fn);
     short y = _getw(fn);
     unsigned short count = _getw(fn), j;
     if (!count) return;
     array tmp;
     for (unsigned short i = 0; i < count; i++) tmp.addbyte(getc(fn));
     tmp.set(0,tmp[0] | RefPt);  //Head is Ref Point
     set_refpt(count, tmp);
     curve_detect(tmp);
     adjust_corner(tmp); //At least need 2 corners, first must corner
//cout << "Get vector 2\n"; cout.flush();
     ct.add(x,y,Scorner);
     coorarray lt; //Use for line test
     short nx = 0, ny = 0;
     for (i = 0; i < count; i = j)
         {
         short dx = modify[tmp[i]&7][0], dy = modify[tmp[i]&7][1];
         for (j = i+1; (j < count) && !(tmp[j] & 0xf8); j++)
             { dx += modify[tmp[j]&7][0]; dy += modify[tmp[j]&7][1]; }
         nx += dx; ny += dy;
         if ((j >= count) || (tmp[j] & CORNER))
            {
            if (!testline(nx,ny,lt))
               { //Curve
               for (short k = 0; k < lt.getlen(); k++)
                   ct.add(x+lt[k].x,y+lt[k].y,Scurve);
               }
            x += nx; y += ny; if (j < count) ct.add(x,y,Scorner);
            nx = ny = 0; lt.clear();
            }
         else lt.add(nx,ny,Scurve);
         }
     }

short testline(short x, short y, coorarray &lt)
     {              //All relative coor
     if (!lt.getlen()) return 1; //Line
     if ((x == 0) && (y == 0)) return 0; //Curve
     double _x = x, _y = y;
     float l = (float)sqrt(_x*_x+_y*_y);
     _x = lt[0].x; _y = lt[0].y;
     float la = (float)sqrt(_x*_x+_y*_y);
     for (short i = 1; i < lt.getlen(); i++)
         {
         _x = lt[i].x - lt[i-1].x;
         _y = lt[i].y - lt[i-1].y;
         la += (float)sqrt(_x*_x+_y*_y);
         }
     _x = x - lt[i-1].x;
     _y = y - lt[i-1].y;
     la += (float)sqrt(_x*_x+_y*_y);
     if (la/l < LineThsd) return 1; //line
     else return 0;
     }

void adjust_corner(array &ct)
     {
//cout << "Adjust corner\n"; cout.flush();
     for (unsigned short fc = 1; (fc < ct.getlen()*3/4) && !(ct[fc] &CORNER); fc++);
     if (fc == ct.getlen()*3/4) //Too long curve ratio
        ct.set(ct.getlen()/2,ct[ct.getlen()/2] | CORNER);
     }   //Only one corner

void set_refpt(unsigned short count, array &tmp)
     {
//cout << "Set refpt\n"; cout.flush();
     for (unsigned short i = 0, j; i < count; i = j) //j will set in inner loop
         {  // Deter Ref Point
         unsigned char first = tmp[i]&7, second = 8, last = first;
         unsigned short fc = 1, sc = 0, k = 1; // at most one of fc and sc can greater then 1
         short no_sw = 0, last_sw = i;
         for (j = i+1; (j < count) && (j-i <= VecUnit); j++)
             {
             if (tmp[j] == last)
                {
                k++;
                if (last == first) { if (k > fc) fc++; }
                else if (k > sc) sc++;
                if ((fc > 1) && (sc > 1)) break;
                }
             else if (last == first)
                  { //switch to second
                  if (second == 8) //Initial
                     {
                     last = second = tmp[j];
                     sc = 1;
                     if ((last != (unsigned char)((first+1)&7))
                        && (last !=(unsigned char)((first-1)&7))) break;
                     } // 1st and 2nd only can differ by 1
                  else if (tmp[j] != second) break; // Only 2 type
                  else last = second;
                  k = 1; last_sw = j; no_sw++;
                  }
             else { //Switch to first
                  if (tmp[j] != first) break;
                  last = first; k = 1; last_sw = j; no_sw++;
                  }
             } //[i,j) is posibile line region
         if (no_sw == 1) //ZigZac line
            {
            j = last_sw;
            tmp.set(j,tmp[j] | RefPt);
            continue;
            }
         if (sc && (j-i > VecUnit)) //break by bound
            { //sc is 0 means straight line
            unsigned char *mbase = new unsigned char[(j-i)*2];
            unsigned char *cbase = new unsigned char [j-i];
            for (k = i; k < j; k++) cbase[k-i] = tmp[k] & 1;
            while (j-i > 5) // Always line when less then 6
                  { // Local x,y
                  short x = j-i, y = 0, cy = 0; // Compute distance
                  for (short k = 0; k < x; k++) if (cbase[k]) y++;
                  for (k = 0; k < 2*x; k++)
                      { // Build compare base
                      cy += y;
                      if (cy > (x/2)) {mbase[k] = 1; cy-=x;} //Ideal line
                      else mbase[k] = 0; //mbase only contain 0 1
                      }
                  for (k = 0; (k < x) && memcmp(mbase+k,cbase,x); k++);
                  if (k < x) break;
                  j--; //shrink posibile line range
                  }
            delete mbase;
            delete cbase;
            }
         if (j < count) tmp.set(j,tmp[j] | RefPt);
         }
     }

void curve_detect(array &ct)
     {
//cout << "Curve detect\n"; cout.flush();
     short lastdx = 0, lastdy = 0;
     for (unsigned short i = 0, j = 0; i < ct.getlen(); i = j)
         {
         for (short nextdx = 0, nextdy = 0; j < ct.getlen(); j++)
             {
             if (ct[j] & RefPt)
                {        //Don't allow two continue RefPt
                if (j-i > NOISE) break;
                else if (j > i) ct.set(j,ct[j]&7);
                }
             nextdx += modify[ct[j]&7][0];
             nextdy += modify[ct[j]&7][1];
             }
         if (i && (nextdx || nextdy) && (lastdx || lastdy))
            {
            float psita = (float)atan2((double)lastdy,(double)lastdx);
            float nsita = (float)atan2((double)nextdy,(double)nextdx);
            nsita = (float)fabs(nsita-psita);
            if ((nsita >= CurveDeg) && (nsita <= NegCurveDeg))
               ct.set(i, ct[i] | CORNER);
            }
         else ct.set(0, ct[0] | CORNER); //forcing first and error are corners
         lastdx = nextdx; lastdy = nextdy;
         }
     }
#define MAX_LEVEL 10

short level(CObArray &ary, short idx)
      {
      short l = 0;
      while (((coorarray *)ary[idx])->parent >= 0)
            {
            l++; if (l > MAX_LEVEL) break;
            idx = ((coorarray *)ary[idx])->parent;
            }
      return l;
      }
