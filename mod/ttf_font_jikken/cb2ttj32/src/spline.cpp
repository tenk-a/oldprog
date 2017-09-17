#define	_DOS
#include <afx.h>
#include <afxcoll.h>
#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include "ntuttf.h"
#include "array.h"

#define	Tolence	0.000001
#define	MoveLeft1 0x80
#define	MoveRite1 0x40
#define	MoveUp1	0x20
#define	MoveDown1 0x10
#define	MoveLeft2 8
#define	MoveRite2 4
#define	MoveUp2	2
#define	MoveDown2 1

class farray
     {
public:
     farray(short size,short x,	short y) //all relative	coor
    	 {
    	 xlist = new float[size]; ylist	= new float[size];
    	 double	sita = ((x != 0) || (y != 0))?atan2((double)y,(double)x):0;
    	 cosx =	(float)cos(sita); //^Avoid small circle
    	 sinx =	(float)sin(sita);
    	 lx = x*cosx + y*sinx; //Normalizer
    	 len = size;
    	 }
     ~farray() { delete	xlist; delete ylist; }
     void set(short i, short x,	short y)
    	  {
    	  xlist[i] = (float)x*cosx+(float)y*sinx;
    	  ylist[i] = (float)y*cosx-(float)x*sinx;
    	  }
     void find_opt();
     void get_para(short &x1, short &y1)
    	  {
    	  x1 = (short)(px1*cosx-py1*sinx+0.5);
    	  y1 = (short)(py1*cosx+px1*sinx+0.5);
    	  }
     short almost_line()
    	  { return (fabs(py1) <	NOISE/2)?1:0; }
     float sq_err, max_err, avg_err;
     short maxerr_ptr;
private:
     void init_para() {	px1 = xlist[len/2];   py1 = ylist[len/2]; }
     float compute_SE();
     void search_MSE(short step);
     void null_op();

     float lx, px1, py1, cosx, sinx;
     float *xlist, *ylist;
     short len;
     };

inline float farray::compute_SE()
     {
     float _err	= 0.0;
     float t = 0, dt = (float)3/(float)(len+1);
     float _a1 = px1/2,	_a2 = lx/2-px1,	_c2 = _a2*2;
     float _a3 = (lx-px1)/2, _a4 = py1/2;
     for (short	k = 0; k < len;	k++)
    	 {
    	 float x = xlist[k];
    	 float low = t,	sup = 3, r;
    	 float _b2 = _a1-x, _b3	= (lx+px1)/2-x;
    	 t += dt; //default
    	 if (t < 1) r =	t*_a1-x;
    	 else if (t < 2)
    	      {
    	      float _t = t - 1;
    	      //r = (px1+lx)*_t*_t/2+px1*_t*(1-_t)*2+px1*(_t-1)*(_t-1)/2 - x;
    	      r	= _t*(_a2*_t+px1)+_b2;
    	      }
    	 else //r = (px1+lx)/2+(lx-px1)*(t-2)/2	- x;
    	      r	= _a3*(t-2)+_b3;
    	 short try_no =	1;
    	 while (fabs(r)	> Tolence) //Newton methid
    	       {
    	       float rd;
    	       if (t < 1) rd = _a1;
    	       else if (t < 2)
    	    	    //{	float _t = t - 1; rd = (lx+px1)*_t + (2-4*_t)*px1 + px1*(_t-1);	}
    	    	    rd = _c2*(t-1)+px1;
    	       else rd = _a3;//(lx-px1)/2;
    	       if (fabs(rd) > Tolence)
    	    	  {
    	    	  null_op(); //Here is an optimize bug
    	    	  float	ot = t;
    	    	  t -= r/rd; //t must between [low,sup]
    	    	  if (t	<= low)	{t = low; break;} //Avoid cyclic
    	    	  else if (t >=	sup) {t	= sup; break;}
    	    	  if (fabs(ot-t) < Tolence/2) break; //too near
    	    	  if (ot < t) low = ot;	else sup = ot; //t != ot
    	    	  }
    	       else if (try_no < SolTry) t += dt*try_no++; //Try another thread	if f'(t) = 0
    	       else break;
    	       if (t < 1) r = t*_a1-x;
    	       else if (t < 2)
    	    	    {
    	    	    float _t = t - 1;
    	    	    r =	_a2*_t*_t+px1*_t+_b2;
    	    	    }
    	       else r =	_a3*(t-2)+_b3;
    	       }
    	 float ny = -1 * ylist[k];
    	 if (t < 1) ny += _a4*t;
    	 else if (t < 2) ny += py1*(t-1)*(2-t)+_a4;
    	 else ny += _a4*(3-t);
    	 _err += ny*ny;
    	 if (fabs(ny) >	max_err) { max_err = (float)fabs(ny); maxerr_ptr = k;}
    	 }
     return _err;
     }

void farray::null_op()
     {
     }

void farray::find_opt()
     {
     init_para();
     sq_err = compute_SE();
     search_MSE(9); search_MSE(3); search_MSE(2); search_MSE(1);
     max_err = 0; maxerr_ptr = 0;
     sq_err = compute_SE();
     avg_err = (float)sqrt((double)sq_err/len);
     }

void farray::search_MSE(short step)
     {
     unsigned char sbd = 0, obd	= 0;
     float _err	= sq_err;
     do	{
    	sbd = 0; //Init	score board
    	if (!(obd & MoveDown1))
    	   {
    	   px1 += step;	py1 += step;
    	   float sq_tmp=compute_SE();
    	   px1 -= step;	py1 -= step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd = MoveUp1;}
    	   }
    	if (!(obd & MoveUp1))
    	   {
    	   px1 -= step;	py1 -= step;
    	   float sq_tmp=compute_SE();
    	   px1 += step;	py1 += step;
    	   if (sq_tmp <	_err - Tolence)	{_err =sq_tmp; sbd = MoveDown1;}
    	   }
    	if (!(obd & MoveRite1))
    	   {
    	   px1 -= step;	py1 += step;
    	   float sq_tmp=compute_SE();
    	   px1 += step;	py1 -= step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd=MoveLeft1;}
    	   }
    	if (!(obd & MoveLeft1))
    	   {
    	   px1 += step;	py1 -= step;
    	   float sq_tmp=compute_SE();
    	   px1 -= step;	py1 += step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd=MoveRite1;}
    	   }
    	switch (sbd)
    	       {
    	       case MoveUp1:   px1 += step; py1	+= step; break;
    	       case MoveDown1: px1 -= step; py1	-= step; break;
    	       case MoveLeft1: px1 -= step; py1	+= step; break;
    	       case MoveRite1: px1 += step; py1	-= step; break;
    	       }
    	obd = sbd; //copy score	board to old board
    	} while	(sbd); //Break when min	found with step	2
     sq_err = _err;
     }

extern "C"
     {
     void get_spline(FILE *fn, coorarray &ct);
     void get_control(coorarray	&ct,short x,short y,short *xlist,short *ylist,short len);
     void write_file(FILE *fn, CObArray	&ary);
     short testline(short x,short y,short *xlist,short *ylist,short start,short	end);
     }

float ErrConstrain = 1.8;

void main(short	argc, char *argv[])
     {
     if	(argc <	3) { cout << "Usage: SPLINE vectorfile splinefile [Threashold]\n"; return; }
     FILE *fi =	fopen(argv[1],"rb");
     FILE *fo =	fopen(argv[2],"wb");
     if	(!fi ||	!fo) { cout << "File open error.\n"; return; }
     if	(argc >= 4) ErrConstrain = (float)atof(argv[3]);
     CObArray ary;
     for (;;)
    	 {
    	 coorarray *ct = new coorarray;
    	 ct->parent = _getw(fi);
    	 if (feof(fi)) break;
    	 get_spline(fi,*ct);
    	 ary.Add(ct);
    	 }
     fclose(fi);
     write_file(fo, ary);
     fclose(fo);
     }

void write_file(FILE *fn, CObArray &ary)
     {
     for (short	i = 0; i < ary.GetSize(); i++)
    	 {
    	 coorarray *tmp	= (coorarray *)ary[i];
    	 _putw(tmp->parent,fn);
    	 _putw(tmp->getlen(),fn);
    	 for (short j =	0; j < tmp->getlen(); j++)
    	     {
    	     _putw((*tmp)[j].flag,fn);
    	     _putw((*tmp)[j].x,fn);
    	     _putw((*tmp)[j].y,fn);
    	     }
    	 }
     }

void get_spline(FILE *fn, coorarray &ct)
     {
     short count = _getw(fn), j;
     coorarray tmp;
     for (short	i = 0; i < count; i++)
    	 {
    	 struct	coor curr;
    	 curr.flag = _getw(fn);
    	 curr.x	= _getw(fn);
    	 curr.y	= _getw(fn);
    	 tmp.add(curr);
    	 }
     for (i = 0; i < count; i =	j)
    	 {
    	 for (j	= i+1; (j < count) && (tmp[j].flag == Scurve); j++);
    	 if (j == i+1) { ct.add(tmp[i]); continue; } //Line seg
    	 short x = tmp[i].x, y = tmp[i].y;
    	 short *xlist =	new short[j-i];
    	 short *ylist =	new short[j-i];
    	 for (short k =	0; k < j-i-1; k++)
    	     { xlist[k]	= tmp[i+1+k].x;	ylist[k] = tmp[i+1+k].y; }
    	 if (j == count) { xlist[k] = tmp[0].x;	ylist[k] = tmp[0].y; }
    	 else {	xlist[k] = tmp[i+1+k].x; ylist[k] = tmp[i+1+k].y; }
    	 get_control(ct,x,y,xlist,ylist,j-i);
    	 delete	xlist; delete ylist;
    	 }
     }

void get_control(coorarray &ct,short x,short y,short *xlist,short *ylist,short len)
     {	    	    	    	    	       //Absolute coor
     for (short	start =	0, end = len-1;start < len;)
    	 {  	    	    	 //len-1 and (x,y) are corner points
    	 if (end - start > LineSample) end = start + LineSample; //Limit data size
    	 if (start || (end < len-1))
    	    {
    	    if (testline(x,y,xlist,ylist,start,end))
    	       { //Tangent line
    	       ct.add(x,y,Scorner);
    	       x = xlist[end]; y = ylist[end];
    	       start = end+1;
    	       end = len-1;
    	       continue;
    	       }
    	    }
    	 if (end - start < 2) //Smallest curve
    	    {
    	    ct.add(x,y,Scorner);
    	    ct.add(xlist[start],ylist[start],Scurve);
    	    if (end > start + 1) ct.add(xlist[start+1],ylist[start+1],Scurve);
    	    x =	xlist[end]; y =	ylist[end];
    	    start = end+1;
    	    end	= len-1;
    	    continue;
    	    }
    	 farray	ftmp(end - start,xlist[end]-x,ylist[end]-y);
    	 for (short i =	start; i < end;	i++)
    	     ftmp.set(i-start,xlist[i]-x,ylist[i]-y);
    	 ftmp.find_opt();
    	 if ((ftmp.max_err > ErrConstrain) || (ftmp.avg_err > ErrConstrain/3))
    	    { end = start + (end-start)*2/3;  continue;}
    	 ct.add(x,y,Scorner);
    	 short x1, y1;
    	 ftmp.get_para(x1,y1);
    	 ct.add(x+x1,y+y1,Scontrol);
    	 x = xlist[end]; y = ylist[end];
    	 start = end+1;
    	 end = len-1;
    	 }
     }

short testline(short x,short y,short *xlist,short *ylist,short start,short end)
     {	    	    //All relative coor
     if	(end-start < 2)	return 1; //Line
     if	((xlist[end] ==	x) && (ylist[end] == y)) return	0; //Curve
     double _x = xlist[end]-x, _y = ylist[end]-y;
     float l = (float)sqrt(_x*_x+_y*_y);
     _x	= xlist[start]-x; _y = ylist[start]-y;
     float la =	(float)sqrt(_x*_x+_y*_y);
     for (short	i = start+1; i <= end; i++)
    	 {
    	 _x = xlist[i] - xlist[i-1];
    	 _y = ylist[i] - ylist[i-1];
    	 la += (float)sqrt(_x*_x+_y*_y);
    	 }
     if	(la/l <	(float)1.001) return 1;	//line
     else return 0;
     }
