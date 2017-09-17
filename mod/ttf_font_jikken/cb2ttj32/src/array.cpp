#define _DOS
#include <afx.h>
#include <malloc.h>
#include <stdio.h>
#include "ntuttf.h"
#include "array.h"

short array::addbyte(unsigned char b)
     {
     if (!(len & 63))
        {
        unsigned char __far *nd = (unsigned char __far *)_frealloc(data,len+64);
        if (!nd) return FAIL;
        else data = nd;
        }
     data[len++] = b;
     return SUCC;
     }

short array::insert(unsigned short s, unsigned char b)
     {
     if (s >= len)  return addbyte(b);
     if (!(len & 63))
        {
        unsigned char __far *nd = (unsigned char __far *)_frealloc(data,len+64);
        if (!nd) return FAIL;
        else data = nd;
        }
     for (unsigned short i = len; i > s; i--) data[i] = data[i-1];
     data[s] = b; len++;
     return SUCC;
     }

void array::remove(unsigned short s)
     {
     if (s >= len)  return;
     len--;
     for (unsigned short i = s; i < len; i++) data[i] = data[i+1];
     }

void array::remove(unsigned short start, unsigned short size)
     {
     if (start+size >= len) return;
     len -= size;
     for (unsigned short i = start; i < len; i++) data[i] = data[i+size];
     }

struct coor coorarray::nul_item = { 0,0,0 };

short coorarray::add(struct coor c)
     {
     if (!(len & 15)) if (!test_exp()) return FAIL;
     data[len++] = c;
     return SUCC;
     }

short coorarray::add(short x, short y, short f)
     {
     if (!(len & 15)) if (!test_exp()) return FAIL;
     data[len].x = x;
     data[len].y = y;
     data[len++].flag = f;
     return SUCC;
     }

short coorarray::insert(short s, struct coor c)
     {
     if ((s >= len) || (s < 0)) return add(c);
     if (!(len & 15)) if (!test_exp()) return FAIL;
     for (short i = len; i > s; i--) data[i] = data[i-1];
     data[s] = c; len++;
     return SUCC;
     }

void coorarray::remove(short s)
     {
     if ((s >= len) || (s < 0)) return; len--;
     for (short i = s; i < len; i++) data[i] = data[i+1];
     }

short coorarray::test_exp()
     {
     struct coor __far *nd =
            (struct coor __far *)_frealloc(data,(len+16)*sizeof(struct coor));
     if (!nd) return FAIL;
     else data = nd;
     return SUCC;
     }
