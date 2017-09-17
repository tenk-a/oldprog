#define L(a)     (*((unsigned char *)(&(a))))
#define H(a)     (*(((unsigned char *)(&(a)))+1))
#define HX(a)     (*(((unsigned short *)(&(a)))+1))
#define LX(a)     (*((unsigned short *)(&(a))))
#define XOR(a,b) (((a)?1:0)^((b)?1:0))

#define XSIZE 1024
#define YSIZE 1024
#define DESCENT 204
#define VecUnit 12
#define MinFree 20 //5X5 box
#define NoCurCut 6
#define CurveDeg ((float)(3.14159265359/NoCurCut))
#define NegCurveDeg (CurveDeg*(NoCurCut*2-1))
#define NOISE 2
#define MaxSample 50
#define LineSample 40
#define SolTry 5

#ifdef __cplusplus
extern "C"
     {
#endif
     unsigned short debig5(short i); /* big5.c */
     short big5(unsigned char a, unsigned char b);
     unsigned char get8(FILE *fi);
     unsigned short get16(FILE *fi);
     unsigned long get32(FILE *fi);
     void put16(unsigned short s,FILE *fi);
     void put32(unsigned long s,FILE *fi);
#ifdef __cplusplus
     }
#endif

#define SUCC 1
#define FAIL 0

#define Scorner 1
#define Scurve 2
#define Scontrol 3

#define _INC_NTUTTF
