/*--------------------------------------------------------------------------*/
/* 共通定義																	*/
/*--------------------------------------------------------------------------*/

#ifndef DEF_H
#define DEF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <io.h>


#if 0
#ifndef NULL
#define NULL	((void *)0)
#endif
#endif


#if 1
#define	UINT		unsigned
#define	UCHAR		unsigned char
#define	USHORT		unsigned short
#define	ULONG		unsigned long
#define	SCHAR		signed	 char
#endif
#if 1
typedef int 			int_t;
typedef signed char 	int8_t;
typedef short			int16_t;
typedef long			int32_t;
typedef unsigned		uint_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned long	uint32_t;
#endif
#if 1
typedef int 			sint;
typedef signed char 	sint8;
typedef short			sint16;
typedef long			sint32;
typedef unsigned		uint;
typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned long	uint32;
#endif


#define OFFSETOF(t,m)	((long)&(((t*)0)->m))	/* 構造体メンバ名の、オフセット値を求める */

#define ISDIGIT(c)  	(((unsigned)(c) - '0') < 10U)
#define ISLOWER(c)		(((unsigned)(c)-'a') < 26U)
#define TOUPPER(c)  	(ISLOWER(c) ? (c) - 0x20 : (c) )
/*#define ISKANJI(c)	((unsigned)((c)^0x20) - 0xA1 < 0x3C)*/
#define ISKANJI(c)		((unsigned char)(c) >= 0x81 && ((unsigned char)(c) <= 0x9F || ((unsigned char)(c) >= 0xE0 && (unsigned char)(c) <= 0xFC)))
#define ISKANJI2(c) 	((unsigned char)(c) >= 0x40 && (unsigned char)(c) <= 0xfc && (c) != 0x7f)

#define MAX(x, y)		((x) > (y) ? (x) : (y)) /* 最大値 */
#define MIN(x, y)		((x) < (y) ? (x) : (y)) /* 最小値 */
#define ABS(x)			((x) < 0 ? -(x) : (x))	/* 絶対値 */

#define REVW(a)			((((a) >> 8) & 0xff)|(((a) & 0xff) << 8))
#define REVL(a)			( (((a) & 0xff000000) >> 24)|(((a) & 0x00ff0000) >>  8)|(((a) & 0x0000ff00) <<  8)|(((a) & 0x000000ff) << 24) )

#define BT(n)			(1<<(n))
#define	BB(a,b)			((((unsigned char)(a))<<8)+(unsigned char)(b))
#define	WW(a,b)			((((unsigned short)(a))<<16)+(unsigned short)(b))
#define	BBBB(a,b,c,d)	((((unsigned char)(a))<<24)+(((unsigned char)(b))<<16)+(((unsigned char)(c))<<8)+((unsigned char)(d)))

#define	GLB(a)			((unsigned char)(a))
#define	GHB(a)			GLB(((unsigned short)(a))>>8)
#define	GLLB(a)			GLB(a)
#define	GLHB(a)			GHB(a)
#define	GHLB(a)			GLB(((unsigned long)(a))>>16)
#define	GHHB(a)			GLB(((unsigned long)(a))>>24)
#define	GLW(a)			((unsigned short)(a))
#define	GHW(a)			GLW(((unsigned long)(a))>>16)

#define gLb(a)			((signed char)(a))
#define gHb(a)			gLb(((signed short)(a))>>8)
#define gLLb(a) 		gLb(a)
#define gLHb(a) 		gHb(a)
#define gHLb(a) 		gLb(((signed long)(a))>>16)
#define gHHb(a) 		gLb(((signed long)(a))>>24)
#define gLw(a)			((signed short)(a))
#define gHw(a)			gLw(((signed long)(a))>>16)

#define	PEEKB(a)		(*(unsigned char  *)(a))
#define	PEEKW(a)		(*(unsigned short *)(a))
#define	PEEKD(a)		(*(unsigned long  *)(a))
#define	POKEB(a,b)		(*(unsigned char  *)(a) = (b))
#define	POKEW(a,b)		(*(unsigned short *)(a) = (b))
#define	POKED(a,b)		(*(unsigned long  *)(a) = (b))
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((unsigned long)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((unsigned long)(a)+2) << 16) )
#define	PEEKmW(a)		( (PEEKB(a)<<8) | PEEKB((unsigned long)(a)+1) )
#define	PEEKmD(a)		( (PEEKmW(a)<<16) | PEEKmW((unsigned long)(a)+2) )
#define	POKEmW(a,b)		(POKEB((a),GHB(b)), POKEB((ULONG)(a)+1,GLB(b)))
#define	POKEmD(a,b)		(POKEmW((a),GHW(b)), POKEmW((ULONG)(a)+2,GLW(b)))
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((ULONG)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((ULONG)(a)+2,GHW(b)))

#define BTST(a,b)		((a) & (1<<(b)))
#define SWAP_INT(a,b)	do {unsigned long t__; t__ = (unsigned long)(a); (a) = (b); (b) = t__;}while(0)
#define SWAP_PTR(a,b)	do {void *t__; t__ = (void *)(a); (a) = (void *)(b); (b) = (void*)t__;}while(0)
#define SWAP_TYP(y,a,b)	do {y t__; t__ = (a); (a) = (b); (b) = t__;}while(0)



/* メモリ・文字列関係	*/
#define STPCPY(d,s)			(strcpy((d),(s)) + strlen(s))
#define STREND(p)			((p)+ strlen(p))
#define STRINS(d,s) 		(memmove((d)+strlen(s),(d),strlen(d)+1),memcpy((d),(s),strlen(s)))
#define MEMSETW(d, s, sz)	do {short *d__ = (short*)(d); int c__ = (sz); do { *d__++ = (short)(s); } while(--c__); } while (0)
#define MEMSETD(d, s, sz)	do {long  *d__ = (long *)(d); int c__ = (sz); do { *d__++ = (long) (s); } while(--c__); } while (0)
#define MEMCPY2(d, s, sz)	do {short *d__ = (short*)(d); short *s__ = (short*)(s); int c__ = (unsigned)(sz)>>1; do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPY4(d, s, sz)	do {long  *d__ = (long *)(d); long  *s__ = (long *)(s); int c__ = (unsigned)(sz)>>2; do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMRCPY4(d, s, sz)	do {long  *d__ = (long *)(d); long  *s__ = (long*)(s);  int c__ = (unsigned)(sz)>>2; while (--c__ >= 0) { d__[c__] = s__[c__]; } } while (0)

//void MEMCPY16(void *d, void *s, int sz);
//void MEMCPY32(void *d, void *s, int sz);
//void MEMCPY64(void *d, void *s, int sz);
//void MEMCPY128(void *d, void *s, int sz);

//char *strncpyz(char *dst, char *src, int size);


/* デバッグ */

#define	DB			if (debugflag)
extern int			debugflag;
#define DB_M()		if (debugflag)	printf("%-16s %6d\n",__FILE__,__LINE__)

/*------------------------------------------*/

#define STDIN		stdin
#define STDOUT		stdout
#define STDERR		stdout

/*#define FLENGTH(fp)		filelength(fileno((fp)))*/

#define FIL_NMSZ	(1024+16)



/*------------------------------------------*/
/*------------------------------------------*/
/*------------------------------------------*/

#endif /* DEF_H */
