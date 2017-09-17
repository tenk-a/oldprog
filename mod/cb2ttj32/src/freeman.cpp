#define _DOS
#include <afx.h>
#include <afxcoll.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <conio.h>
#include "ntuttf.h"
#include "array.h"

#define Width (long)(XSIZE/8)

class bitmap
     {
public:
     bitmap(char *filename, const char *_hf);
     ~bitmap() { _hfree(data); }
     unsigned char test(short x, short y)
          { return (data[(long)y*Width+x/8] & mask[x&7])?1:0; }
     void set(short x, short y) { data[(long)y*Width+x/8] |= mask[x&7]; }
     void reset(short x, short y) { data[(long)y*Width+x/8] &= ~mask[x&7]; }
     void hline(short x1, short y, short x2);
     void readline(short y, unsigned char *buf);
     short valid; /* 3:bitmap 1:empty 0:error */
private:
     void read_dyna(FILE *fn);
     void read_etfg(FILE *fn);
     static unsigned char mask[8];
     unsigned char __huge *data;
     short D_flag;
     };

unsigned char bitmap::mask[8] = { 0x80,0x40,0x20,0x10,8,4,2,1 };

bitmap::bitmap(char *filename, const char *_hf)
     {
     D_flag = strchr(_hf,'D')?1:0;
     data = (unsigned char __huge *)_halloc(YSIZE,Width);
     valid = data?1:0;
     if (valid)
        {
        for (short i = 0; i < YSIZE; i++)
            _fmemset((unsigned char __far *)(data+(long)i*Width),0,Width);
        FILE *fn = fopen(filename,"rb");
        if (!fn) return;
        char tag[5];
        fread(tag,1,4,fn); tag[4] = 0;
        if (!strcmp(tag,"DYNA")) read_dyna(fn);
        else if (!strcmp(tag,"ETFG")) read_etfg(fn);
        else { fclose(fn); return; }
        valid |= 2;
        fclose(fn);
        }
     }

void bitmap::hline(short x1, short y, short x2)
     {
     if ((y < 0) ||(y >= YSIZE)) return;
     if (x1 < 0) x1 = 0;
     if (x2 >= XSIZE) x2 = XSIZE-1;
     for (; (x1 & 7) && (x1 <= x2); x1++) set(x1,y);
     for (; (x1 <= x2) && (x1+7 <= x2); x1 += 8) data[(long)y*Width+x1/8] = 0xff;
     for (; x1 <= x2; x1++) set(x1,y);
     }

void bitmap::readline(short y, unsigned char *buf) // buf[XSIZE]
     {
     register unsigned short m = 0x8080;
     unsigned char __far *d = data + (long)y*Width;
     for (short i = 0; i < XSIZE; i++)
         {
         *buf++ = (d[i/8]&m)?1:0;
         m = _rotr(m,1);
         }
     }

void bitmap::read_dyna(FILE *fn)
     {
     for (;;)
         {
         short y = _getw(fn);
         if (feof(fn)) break;
         short x1 = _getw(fn);
         short x2 = _getw(fn);
         hline(x1,y,x2);
         }
     }

void bitmap::read_etfg(FILE *fn)
     {
     for (;;)
         {
         short y = _getw(fn);
         if (feof(fn)) break;
         short count = _getw(fn);
         for (short i = 0; i < count; i++)
             {
             short x1 = _getw(fn);
             short x2 = _getw(fn);
	     if (D_flag)
		for (short j = 0; j < 10; j++) hline(x1-4,y-4+j,x2+5);
             else hline(x1,y,x2);
             }
         }
     }

class contour: public array
     {
public:
     contour() : array() {enm = x = y = 0; parent = o_idx = r_idx = -1; }
     contour(short _x, short _y, short _idx, short prt): array()
         { x = _x; y = _y; o_idx = _idx; parent = prt; enm = 0; r_idx = -1;}
void dump() { cout << enm << " " << parent << " " << x << " " << y << " " << o_idx << " " << r_idx << " " << getlen() << "\n";}
     short enm, parent, x, y, o_idx, r_idx;
     };

CString heu_flag("B");

extern "C"
     {
     void gethead(bitmap &bp, CObArray &ary);
     void getfree(bitmap &bp, CObArray &ary);
     void scan_special(contour &ct);
     void savefile(CObArray &ary, FILE *fn);
     }

void main(short argc, char *argv[])
     {
     if (argc < 3)
        { cout << "Usage FREEMAN infile outfile [heurestic_flag]\n"; return; }
     if (argc == 4) heu_flag = argv[3];
     bitmap bp(argv[1],heu_flag);
     if (bp.valid != 3)
        { cout << "Memory over or " << argv[1] << " open error.\n"; return; }
     FILE *fo = fopen(argv[2],"wb");
     if (!fo) { cout << "File " << argv[2] << " open error\n"; return; }
     CObArray hCT;
     gethead(bp, hCT);
     getfree(bp, hCT);
     savefile(hCT, fo);
     fclose(fo);
     }

void gethead(bitmap &bp, CObArray &ary)
     {
     short *oldlist = new short[ XSIZE];
     short *newlist = new short[ XSIZE];
     unsigned char *oldrun = new unsigned char[ XSIZE];
     unsigned char *newrun = new unsigned char[ XSIZE];
     unsigned char curr = bp.test(0,0)?0:1; //not
     short nowvalue = 0, c = 0;
     bp.readline(0,oldrun);
     for (short i = 0; i < XSIZE; i++) //First line
         if (curr != oldrun[i])
            {
            oldlist[i] = c = nowvalue++;
            ary.Add(new contour(i,0,c,i?oldlist[i-1]:-1));
            curr = oldrun[i];
            }
         else oldlist[i] = c;

     for (short j = 1; j < YSIZE; j++)
         {
         bp.readline(j,newrun);
         curr = newrun[0]?0:1; //not
         c = oldlist[0];
         for (i = 0; i < XSIZE; newlist[i++] = c) if (curr != newrun[i])
             {
             curr = newrun[i];
             c = -1; //Initial reset
             for (short k = i; (k < XSIZE) && (curr == newrun[k]) ;k++);
             //[i,k-1] is compare region if curr is false (white)
             short _i = i;
             if (curr) { if (_i) _i--; if (k < XSIZE) k++; }
             //[i-1,k] is compare region if curr is true (black)
             for (short m = _i; m < k; m++) if (curr == oldrun[m])
                 {  // Conjection area
                 if (c == -1) c = oldlist[m];
                 else if (oldlist[m] > c) // Merge 2 same segments
                      {
                      ((contour *)ary[oldlist[m]])->r_idx = c;
                      for (short x = oldlist[m], n = m; n < XSIZE; n++)
                          if (oldlist[n] == x) oldlist[n] = c;
                      }
                 else if (oldlist[m] < c) // Replace current
                      {
                      ((contour *)ary[c])->r_idx = oldlist[m];
                      for (short x = 0; x < i; x++)
                          if (newlist[x] == c) newlist[x] = oldlist[m];
                      c = oldlist[m];
                      }
                 }
             if (c == -1) //No conjection area
                {
                newlist[i] = c = nowvalue++;
                ary.Add(new contour(i,j,c,oldlist[i]));
                } //o_idx (c) is same as array order
             }
         short *tmplist = oldlist; oldlist = newlist; newlist = tmplist;
         unsigned char *tmprun = oldrun; oldrun = newrun; newrun = tmprun;
         }
     delete oldlist;
     delete newlist;
     delete oldrun;
     delete newrun;
     for (i = j = 0; i < ary.GetSize(); i++) //Enumeration
         if (((contour *)ary[i])->r_idx == -1) ((contour *)ary[i])->enm = j++;
     for (i = 0; i < ary.GetSize(); i++)
         {
         j = ((contour *)ary[i])->parent;
         if (j < 0) continue;
         while (((contour *)ary[j])->r_idx >= 0) j = ((contour *)ary[j])->r_idx;
         ((contour *)ary[i])->parent = ((contour *)ary[j])->enm;
         }
     for (i = 0; i < ary.GetSize();)
         {
         if (((contour *)ary[i])->r_idx >= 0) { delete ary[i]; ary.RemoveAt(i);}
         else i++;
         }
//for (i = 0; i < ary.GetSize(); i++, cout.flush()) ((contour *)ary[i])->dump();
     }

short modify[8][2] = { 0,-1, -1,-1, -1,0, -1,1, 0,1, 1,1, 1,0, 1,-1 };
//modify[dir][x for 0|y for 1]
short nextdir[8] = {5,6,7,0,1,2,3,4};

void getfree(bitmap &bp, CObArray &ary)
     {
     for (short _i = 0; _i < ary.GetSize(); _i++)
         {
         contour *tmp = (contour *)ary[_i];
         unsigned char curr = bp.test(tmp->x,tmp->y);
         if (curr || (tmp->x && tmp->y))
            {
            short x = tmp->x, y = tmp->y, dir = 0;
            do {
               for (short i = 0; i < 8; i++)
                   {
                   short _x = x + modify[(dir+i)&7][0];
                   short _y = y + modify[(dir+i)&7][1];
                   if ((_x >= 0) && (_x < XSIZE) && (_y >= 0) && (_y < YSIZE))
                      if (curr  == bp.test(_x,_y))
                         {
                         if (!curr && ((dir+i) & 1))
                            {
                            short __x = x + modify[(dir+i+1)&7][0];
                            short __y = y + modify[(dir+i+1)&7][1];
                            if (bp.test(__x,__y)) continue;
                            }  // for tips adj.
                         tmp->addbyte((unsigned char)((dir+i)&7));
                         x = _x; y = _y;
                         dir = nextdir[(dir+i)&7];
                         break;
                         }
                   }
               if (!curr && ((x <= 0) || (x >= XSIZE-1) ||
                  (y <= 0) || (y >= YSIZE-1))) { tmp->r_idx = 1; break; }
               //Terminate when 0 contour touch edge, r_idx must -1 when correct
               if (i == 8) {tmp->r_idx = 1; break; }//Terminate, most noise or error
               } while ((x != tmp->x) || (y != tmp->y));
            if (tmp->getlen() < MinFree) tmp->r_idx = 1; //No data
            else if (tmp->r_idx == -1)  scan_special(*tmp);
            }
         else tmp->r_idx = 1; // Useless contour
         }
     for (short j = _i = 0; _i < ary.GetSize(); _i++)
         if (((contour *)ary[_i])->r_idx == -1) ((contour *)ary[_i])->enm = j++;
     for (_i = 0; _i < ary.GetSize(); _i++)
         {
         if (((contour *)ary[_i])->r_idx == -1)
            {
            j = ((contour *)ary[_i])->parent;
            short k = -1;
            while ((j >= 0) && (j < ary.GetSize()))
                  {
                  if (((contour *)ary[j])->r_idx == -1)
                     { k = ((contour *)ary[j])->enm; break;}
                  if (j == ((contour *)ary[j])->parent) break;
                  j = ((contour *)ary[j])->parent;
                  }
            ((contour *)ary[_i])->parent = k;
            }
         }
     for (_i = 0; _i < ary.GetSize();)
         {
         if (((contour *)ary[_i])->r_idx >= 0) { delete ary[_i]; ary.RemoveAt(_i);}
         else _i++;
         }
//for (short i = 0; i < ary.GetSize(); i++, cout.flush()) ((contour *)ary[i])->dump();
     }

void savefile(CObArray &ary, FILE *fn)
     {
     for (short i = 0; i < ary.GetSize(); i++)
         {
         contour *tmp  = (contour *)ary[i];
         _putw(tmp->parent,fn);
         _putw(tmp->x,fn);
         _putw(tmp->y,fn);
         _putw(tmp->getlen(),fn);
         for (unsigned short j = 0; j < tmp->getlen(); j++) putc((*tmp)[j],fn);
         }
     }

void scan_special(contour &ct)
     {
     for (unsigned short i = 0; i < ct.getlen(); )
         {
         if (!(ct[i]&1)) //odd
            {
            for (unsigned short j = 1; (i+j < ct.getlen()) && (ct[i+j] == ct[i]);
                j++); //j of same odd
            if ((i+j <= ct.getlen()) && (ct[i+j] == (unsigned char)((ct[i]-1)&7)))
               {
               if (strchr(heu_flag,'A'))
                  { //Only process n,..,n,n-1,n-2,...,n-2
                  if ((i+j+1 <= ct.getlen()) && (ct[i+j+1] == (unsigned char)((ct[i]-2)&7)))
                     {
                     for (unsigned k = 1; (i+j+k+1 < ct.getlen()) &&
                         (ct[i+j+k+1] == ct[i+j+1]); k++); //k of n-2
                     if ((j > VecUnit) && (k > VecUnit))
                        {
                        ct.set(i+j,ct[i]); ct.insert(i+j+1,ct[i+j+1]);
                        i += j+1; continue;
                        }
                     }
                  }
               else if (strchr(heu_flag,'B'))
                    { //Only process n,...,n,n-1,n-2/n-3
                    if ((i+j+1 <= ct.getlen()) && (ct[i+j+1] == (unsigned char)((ct[i]-2)&7)))
                       if (j > VecUnit) { ct.set(i+j,ct[i]); ct.insert(i+j+1,ct[i+j+1]); }
                       else { // Process n,n-1,n-2,..,n-2
                            for (unsigned k = 1; (i+j+k+1 < ct.getlen()) &&
                                (ct[i+j+k+1] == ct[i+j+1]); k++); //k of n-2
                            if (k > VecUnit)
                               {
                               ct.set(i+j,ct[i]); ct.insert(i+j+1,ct[i+j+1]);
                               i += j+1; continue;
                               }
                            }
                    else if ((i+j+1 <= ct.getlen()) && (j > VecUnit) &&
                         (ct[i+j+1] == (unsigned char)((ct[i]-3)&7)))
                         {
                         ct.set(i+j,ct[i]); ct.insert(i+j+1,ct[i]); ct.insert(i+j+2,ct[i+j+2]);
                         i += j+1; continue;
                         }
                    }
               }
            i += j; continue;
            }
         else if (strchr(heu_flag,'B')) //Even, process n, n-2, n-3,...,n-3
              {
              if ((i+2 < ct.getlen()) && (ct[i+1] == (unsigned char)((ct[i]-2)&7)) &&
                 (ct[i+2] == (unsigned char)((ct[i]-3)&7)))
                 {
                 for (unsigned short j = 1; (i+j+2 < ct.getlen()) && (ct[i+j+2] == ct[i+2]); j++); //j of same n-3
                 if (j > VecUnit)
                    {
                    ct.set(i+1,ct[i]); ct.insert(i+2,ct[i+2]); ct.insert(i+2,ct[i+2]);
                    i += 2; continue;
                    }
                 }
              }
         i++;
         }
     }
