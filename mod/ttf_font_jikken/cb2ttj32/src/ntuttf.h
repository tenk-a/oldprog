#ifndef	_INC_NTUTTF
#define	_INC_NTUTTF

#include <stdio.h>

//
#define	L(a)	 ((unsigned char)(a))	//#define L(a)	   (*((unsigned	char *)(&(a))))
#define	H(a)	 L((a)>>8)  	    	//#define H(a)	   (*(((unsigned char *)(&(a)))+1))
#define	LX(a)	 ((unsigned short)(a))	//#define LX(a)	    (*((unsigned short *)(&(a))))
#define	HX(a)	 LX((a)>>16)	    	//#define HX(a)	    (*(((unsigned short	*)(&(a)))+1))
#define	XOR(a,b) (((a)?1:0)^((b)?1:0))

// 16ビットDOSな関数の辻褄合わせ
#define	__far
#define	__huge
#define	__near
#define	_ffree	    free
#define	_fmalloc    malloc
#define	_fcalloc    calloc
#define	_frealloc   realloc
#define	_hfree	    free
#define	_halloc	    calloc
#define	_hmemset    memset
#define	_fmemset    memset

static __inline	int __rotr(short m, int	sh) {
    int	a = (unsigned short)m >>sh, b =	(unsigned short)(m << (16-sh));
    return a|b;
}
#define	_rotr(m,c)  __rotr(m,c)

static __inline	void __putw(int	a, FILE	*fn)  {(fputc(L(a), (fn)), fputc(H(a), (fn)));}
#define	_putw(a, fn)  __putw(a,	fn)

static __inline	int __getw(FILE	*fn) {int a = (unsigned	char)fgetc(fn);	return a | (fgetc(fn)<<8);}
#define	_getw(fn)   __getw(fn)

#define	XSIZE	    1024
#define	YSIZE	    1024
#define	DESCENT	    204
#define	VecUnit	    12
#define	MinFree	    20 //5X5 box
#define	NoCurCut    6
#define	CurveDeg    ((float)(3.14159265359/NoCurCut))
#define	NegCurveDeg (CurveDeg*(NoCurCut*2-1))
#define	NOISE	    2
#define	MaxSample   50
#define	LineSample  40
#define	SolTry	    5

#ifdef __cplusplus
extern "C"
     {
#endif
     //	big-endian な値でファイル入出力
     unsigned char get8(FILE *fi);
     unsigned short get16(FILE *fi);
     unsigned long get32(FILE *fi);
     void put16(unsigned short s,FILE *fi);
     void put32(unsigned long s,FILE *fi);

  #ifdef SJIS
     int jis2sjis(int c);
     int sjis2jis(int c);
     unsigned short desjis(short i);
     short sjis(unsigned char a, unsigned char b);
     int j2unicode(const char *sjis, char ucf16[], int len);
  #else
     unsigned short debig5(short i); /*	big5.c */
     short big5(unsigned char a, unsigned char b);
  #endif
#ifdef __cplusplus
     }
#endif

#define	SUCC	    1
#define	FAIL	    0

#define	Scorner	    1
#define	Scurve	    2
#define	Scontrol    3

#endif
