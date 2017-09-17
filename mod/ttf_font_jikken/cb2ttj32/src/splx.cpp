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
     void get_para(short &x1, short &y1, short &x2, short &y2)
    	  {
    	  x1 = (short)(px1*cosx-py1*sinx+0.5);
    	  y1 = (short)(py1*cosx+px1*sinx+0.5);
    	  x2 = (short)(px2*cosx-py2*sinx+0.5);
    	  y2 = (short)(py2*cosx+px2*sinx+0.5);
    	  }
     short almost_line()
    	  { return ((fabs(py1) < NOISE/2) && (fabs(py2)	< NOISE/2))?1:0; }
     float sq_err, max_err, avg_err;
     short maxerr_ptr;
private:
     void init_para()
    	  {
    	  px1 =	xlist[len/3];	py1 = ylist[len/3];
    	  px2 =	xlist[len*2/3];	py2 = ylist[len*2/3];
    	  }
     float compute_SE();
     void search_MSE(float step);
     void null_op();
     float lx, px1, py1, px2, py2, cosx, sinx;
     float *xlist, *ylist;
     short len;
     };

inline float farray::compute_SE()
     {
     float _err	= 0.0;
     float t = 0, dt = (float)4/(float)(len+1);
     float _c1 = px1/2,	      _b1 = px1,       _a1 = px2/2 - px1;
     float _c2 = (px2+px1)/2, _b2 = px2	- px1, _a2 = (lx+px1)/2	- px2;
     float _da1	= 2*_a1, _da2 =	2*_a2;
     float _a3 = (px2+lx)/2, _b3 = (lx-px2)/2; //X invariant
     float _ya1	= py2/2-py1, _yc1 = py1/2, _ya3	= py2/2;
     float _ya2	= py1/2-py2, _yb2 = py2-py1, _yc2 = (py1+py2)/2; //Y inv
     for (short	k = 0; k < len;	k++)
    	 {
    	 float x = xlist[k], __c1 = _c1	- x, __c2 = _c2	- x, __c3 = _a3	- x;
    	 float low = t,	sup = 4, r;
    	 t += dt; //default
    	 if (t < 1) r =	t*_c1-x;
    	 else if (t < 2) { float _t = t	- 1; r = _t*(_a1*_t+_b1)+__c1; }
    	 else if (t < 3) { float _t = t	- 2; r = _t*(_a2*_t+_b2)+__c2; }
    	 else r	= __c3+_b3*(t-3);
    	 short try_no =	1;
    	 while (fabs(r)	> Tolence) //Newton methid
    	       {
    	       float rd;
    	       if (t < 1) rd = _c1;
    	       else if (t < 2) rd = _da1*(t-1)+px1;
    	       else if (t < 3) rd = _da2*(t-2)+_b2;
    	       else rd = _b3;
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
    	       if (t < 1) r = t*_c1-x;
    	       else if (t < 2) { float _t = t -	1; r = _t*(_a1*_t+_b1)+__c1;}
    	       else if (t < 3) { float _t = t -	2; r = _t*(_a2*_t+_b2)+__c2;}
    	       else r =	__c3+_b3*(t-3);
    	       }
    	 float ny = -1 * ylist[k];
    	 if (t < 1) ny += _yc1*t;
    	 else if (t < 2) { float _t = t	- 1; ny	+= _t*(_ya1*_t+py1)+_yc1;}
    	 else if (t < 3) { float _t = t	- 2; ny	+= _t*(_ya2*_t+_yb2)+_yc2;}
    	 else ny += _ya3*(4-t);	//ny +=	py2/2-py2*(t-3)/2;
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

void farray::search_MSE(float step)
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
    	if (!(obd & MoveDown2))
    	   {
    	   px2 += step;	py2 += step;
    	   float sq_tmp=compute_SE();
    	   px2 -= step;	py2 -= step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd = MoveUp2;}
    	   }
    	if (!(sbd & MoveUp2))
    	   {
    	   px2 -= step;	py2 -= step;
    	   float sq_tmp=compute_SE();
    	   px2 += step;	py2 += step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd=MoveDown2;}
    	   }
    	if (!(obd & MoveRite1))
    	   {
    	   px1 -= step;	py1 += step;
    	   float sq_tmp=compute_SE();
    	   px1 += step;	py1 -= step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd=MoveLeft1;}
    	   }
    	if (!(obd & MoveRite2))
    	   {
    	   px2 -= step;	py2 += step;
    	   float sq_tmp=compute_SE();
    	   px2 += step;	py2 -= step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd=MoveLeft2;}
    	   }
    	if (!(obd & MoveLeft1))
    	   {
    	   px1 += step;	py1 -= step;
    	   float sq_tmp=compute_SE();
    	   px1 -= step;	py1 += step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd=MoveRite1;}
    	   }
    	if (!(obd & MoveLeft2))
    	   {
    	   px2 += step;	py2 -= step;
    	   float sq_tmp=compute_SE();
    	   px2 -= step;	py2 += step;
    	   if (sq_tmp <	_err - Tolence)	{_err =	sq_tmp;	sbd=MoveRite2;}
    	   }
    	switch (sbd)
    	       {
    	       case MoveUp1:   px1 += step; py1	+= step; break;
    	       case MoveDown1: px1 -= step; py1	-= step; break;
    	       case MoveLeft1: px1 -= step; py1	+= step; break;
    	       case MoveRite1: px1 += step; py1	-= step; break;
    	       case MoveUp2:   px2 += step; py2	+= step; break;
    	       case MoveDown2: px2 -= step; py2	-= step; break;
    	       case MoveLeft2: px2 -= step; py2	+= step; break;
    	       case MoveRite2: px2 += step; py2	-= step; break;
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
//     short testline(short x, short y,	coorarray &lt);
     short testline(short x,short y,short *xlist,short *ylist,short start,short	end);
     }

float ErrConstrain = 1.8;

void main(short	argc, char *argv[])
     {
     if	(argc <	3) { cout << "Usage: SPLX vectorfile splinefile [Threashold]\n"; return; }
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
    	 if (end - start > MaxSample) end = start + MaxSample; //Limit data size
    	 if (start || (end < len-1))
    	    {
    	    //coorarray	tmp;
    	    //for (short i = start; i <	end; i++) tmp.add(xlist[i]-x,ylist[i]-y,Scurve);
    	    //if (testline(xlist[end]-x,ylist[end]-y,tmp))
    	    if (testline(x,y,xlist,ylist,start,end))
    	       { //Tangent line
    	       ct.add(x,y,Scorner);
    	       if (end-start > LineSample)
    	    	  {
    	    	  x = xlist[start+LineSample];
    	    	  y = ylist[start+LineSample];
    	    	  start	+= LineSample+1;
    	    	  }
    	       else {
    	    	    x =	xlist[end]; y =	ylist[end];
    	    	    start = end+1;
    	    	    }
    	       end = len-1;
    	       continue;
    	       }
    	    }
    	 if (end - start < 3) //Smallest curve
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
    	 short x1, x2, y1, y2;
    	 ftmp.get_para(x1,y1,x2,y2);
    	 ct.add(x+x1,y+y1,Scontrol);
    	 ct.add(x+x2,y+y2,Scontrol);
    	 x = xlist[end]; y = ylist[end];
    	 start = end+1;
    	 end = len-1;
    	 }
     }

short testline(short x,short y,short *xlist,short *ylist,short start,short end)
//short	testline(short x, short	y, coorarray &lt)
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
     //_x = x -	lt[i-1].x;
     //_y = y -	lt[i-1].y;
     //la += (float)sqrt(_x*_x+_y*_y);
     if	(la/l <	(float)1.001) return 1;	//line
     else return 0;
     }
