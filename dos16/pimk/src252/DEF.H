#ifndef _DEF_H_
#define _DEF_H_

/* 一般的？な定義 */
#define VOID	void
typedef unsigned WORD;
typedef unsigned char BYTE;
typedef unsigned long DWORD;

#if 1//ndef toascii /* ctype.h がincludeされていないとき */
  #define toascii(c)	((c) & 0x7f)
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
  #define islower(c)  ((c) >= 'a' && (c) <= 'z'))
#endif
#if 1//ndef CT_KJ1	/* jctype.h がincludeされていないとき */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif

#ifdef DEBUG
#define D(x)	x
#define ASSERT(p)	((p) ? (void)0 : ( (void) printf( \
					"%12s %d : Assertion failed (%s)\n", \
					__FILE__, __LINE__, #p ), (void)exit(1) ) )
#else
#define D(x)
#define ASSERT(p)	((void)0)
#endif

/*-----------------------------------------*/
#define MEM_Alloc(sz)		farmalloc(sz)		//farヒープメモリ確保
#define MEM_Calloc(n,sz)	farcalloc(n,sz)		//farヒープメモリ確保（配列型）
#define MEM_Free(p)			farfree(p)			//farヒープメモリ解放
#define MEM_CoreLeft()		farcoreleft()		//farヒープメモリの空き容量
void far *MEM_Allocz(long sz);					//ｵﾌｾｯﾄ値が0 の MEM_Alloc
void far *MEM_Callocz(int n,int sz);			//ｵﾌｾｯﾄ値が0 の MEM_CAlloc
int		MEM_Freez(void far *);					//ｵﾌｾｯﾄ値が0 の MEM_Free
unsigned long MEM_CoreLeftz(void);				//ｵﾌｾｯﾄ値が0 の MEM_CoreLeft
unsigned MEM_Allocp(unsigned sz);				//パラグラフ単位のメモリ確保
int		MEM_Freep(unsigned segm);				//パラグラフ単位のメモリ解放
unsigned MEM_CoreLeftp(void);					//パラグラフ単位の空き容量
/*-----------------------------------------*/

void Grph_Show(int mode);
void Grph_DspPage(int n);
void Grph_ActPage(int n);
void Grph_Analog(void);
#define WAIT_VSYNC()	{while((inp(0x60)&0x20));	while(!(inp(0x60)&0x20));}

/*-----------------------------------------*/

#include "fil.h"

#endif	/* _DEF_H_ */
