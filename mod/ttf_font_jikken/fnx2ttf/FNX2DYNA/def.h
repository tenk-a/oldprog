/*--------------------------------------------------------------------------*/
/* 共通定義																	*/
/*--------------------------------------------------------------------------*/

#ifndef DEF_H
#define DEF_H

#define CPU_X86			// リトルエンディアンかつ多バイト整数のアドレスのアライメントを無視できるときに定義
#define LITTLE_ENDIAN	// リトル・エンディアン(インテル系の値)なCPUのとき定義
//#define BIG_ENDIAN	// ビッグ・エンディアン(モトローラ系の値)なCPUのとき定義
#define NO_ALIGN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <io.h>
#include <limits.h>


/*--------------------------------------------------------------------------*/

// コンパイラの違い対策
#if   defined(_MSC_VER) || defined(__BORLANDC__) || defined(__LCC__)
 #ifndef CDECL
  #define CDECL			__cdecl
 #endif
 #ifndef FASTCALL
  #define FASTCALL		__fastcall
 #endif
 #ifndef __cplusplus
  #define inline		__inline
 #endif
#elif defined(__MINGW32__)
 #define CDECL
#elif 0	/*defined(__GCC__)*/
 #define CDECL
 #define FASTCALL
 typedef long long __int64;
#else	/* 以外 */
 #define CDECL
 typedef long __int64;
#endif



/*--------------------------------------------------------------------------*/
/* 基本型名 */

#if 1	// C99スタイル
typedef int 			int_t;
typedef signed char 	int8_t;
typedef short			int16_t;
typedef int				int32_t;
typedef unsigned		uint_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned		uint32_t;
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(__LCC__)
typedef __int64 		 int64_t;
typedef unsigned __int64 uint64_t;
#elif 1	/*defined(__GCC__)*/
typedef long long			int64_t;
typedef unsigned long long	uint64_t;
#else
typedef long 				int64_t;
typedef unsigned long 		uint64_t;
#endif
#endif

#if 1 /* */
typedef int 			Int;
typedef signed char 	Sint8;
typedef short			Sint16;
typedef int				Sint32;
typedef unsigned		Uint;
typedef unsigned char	Uint8;
typedef unsigned short	Uint16;
typedef unsigned		Uint32;
#if defined(_MSC_VER)||defined(__BORLANDC__)||defined(__LCC__)
typedef __int64			 Sint64;
typedef unsigned __int64 Uint64;
#elif 1	/*defined(__GCC__)*/
typedef long long		Sint64;
typedef unsigned long long Uint64;
#else
typedef long			Sint64;
typedef unsigned long	Uint64;
#endif
#endif


//#ifndef M
//#define M extern
//#endif


/*--------------------------------------------------------------------------*/
/* windows 互換の型名やマクロ. コンソールアプリでwindows.h を使わない場合用 */

#if	defined(_WINDOWS_) == 0
typedef unsigned int	UINT;
typedef unsigned char	BYTE;
typedef unsigned short  WORD;
typedef unsigned        DWORD;
//typedef unsigned __int64 QWORD;
typedef int				BOOL;
#define TRUE			1
#define FALSE			0
#define LOBYTE(w)		((BYTE)(w))
#define HIBYTE(w)		((BYTE)((int)(w) >> 8))
#define LOWORD(l)		((WORD)(l))
#define HIWORD(l)		((WORD)((int)(l) >> 16))
#define MAKEWORD(a, b)	(LOBYTE(a)|(LOBYTE(b)<<8))
#define MAKELONG(a, b)	(LOWORD(a)|(LOWORD(b)<<16))
#define ZeroMemory(d,n)	(memset((d),0,(n)))
#endif

#ifndef MAX_PATH
#define MAX_PATH		(1024+16)
#endif

//C++対策
#ifdef __cplusplus
  #ifndef EXTERN_C
	#define EXTERN_C	extern "C"
  #endif
#else
  #ifndef EXTERN_C
	#define EXTERN_C	extern
  #endif
#endif



/*--------------------------------------------------------------------------*/

/* 標準関数/マクロの置き換え物 */
#define ISDIGIT(c)  	(((unsigned)(c) - '0') < 10U)
#define ISLOWER(c)		(((unsigned)(c)-'a') < 26U)
#define TOUPPER(c)  	(ISLOWER(c) ? (c) - 0x20 : (c) )

#define OFFSETOF(t,m)	((long)&(((t*)0)->m))			/* offsetofマクロはansi規格だがライブラリにない場合もあるため */

#define ISKANJI(c)		((unsigned char)(c) >= 0x81 && ((unsigned char)(c) <= 0x9F || ((unsigned char)(c) >= 0xE0 && (unsigned char)(c) <= 0xFC)))
#define ISKANJI2(c) 	((unsigned char)(c) >= 0x40 && (unsigned char)(c) <= 0xfc && (c) != 0x7f)

#define MAX(a, b)		((a) > (b) ? (a) : (b)) 	/* 最大値 */
#define MIN(a, b)		((a) < (b) ? (a) : (b)) 	/* 最小値 */
#define ABS(a)			((a) < 0 ? -(a) : (a))		/* 絶対値 */



/* ビット演算やメモリアクセス, アライメント対策もの */
#define BT(n)			(1<<(n))
#define BTST(a,b)		((a) & (1<<(b)))
#define	BB(a,b)			((((unsigned char)(a))<<8)+(unsigned char)(b))
#define	WW(a,b)			((((unsigned short)(a))<<16)+(unsigned short)(b))
#define	BBBB(a,b,c,d)	((((unsigned char)(a))<<24)+(((unsigned char)(b))<<16)+(((unsigned char)(c))<<8)+((unsigned char)(d)))

#define REVW(a)			((((a) >> 8) & 0xff)|(((a) & 0xff) << 8))
#define REVL(a)			( (((a) & 0xff000000) >> 24)|(((a) & 0x00ff0000) >>  8)|(((a) & 0x0000ff00) <<  8)|(((a) & 0x000000ff) << 24) )

#if 1
#define gLb(a)			((signed char)(a))
#define gHb(a)			gLb(((signed short)(a))>>8)
#define gLLb(a) 		gLb(a)
#define gLHb(a) 		gHb(a)
#define gHLb(a) 		gLb(((signed long)(a))>>16)
#define gHHb(a) 		gLb(((signed long)(a))>>24)
#define gLw(a)			((signed short)(a))
#define gHw(a)			gLw(((signed long)(a))>>16)
#endif

#if 1
#define	GLB(a)			((unsigned char)(a))
#define	GHB(a)			GLB(((unsigned short)(a))>>8)
#define	GLLB(a)			GLB(a)
#define	GLHB(a)			GHB(a)
#define	GHLB(a)			GLB(((unsigned long)(a))>>16)
#define	GHHB(a)			GLB(((unsigned long)(a))>>24)
#define	GLW(a)			((unsigned short)(a))
#define	GHW(a)			GLW(((unsigned long)(a))>>16)
#endif

/* バイト単位のアドレスから値を取り出すメモリアクセス用マクロ */
#define	PEEKB(a)		(*(const unsigned char  *)(a))
#define	POKEB(a,b)		(*(unsigned char  *)(a) = (b))

#ifdef CPU_X86
#define	PEEKiW(a)		(*(const unsigned short *)(a))
#define	PEEKiD(a)		(*(const unsigned long  *)(a))
#define	POKEiW(a,b)		(*(unsigned short *)(a) = (b))
#define	POKEiD(a,b)		(*(unsigned       *)(a) = (b))
#else
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#endif

#define	PEEKmW(a)		( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )
#define	PEEKmD(a)		( (PEEKmW(a)<<16) | PEEKmW((const char *)(a)+2) )
#define	POKEmW(a,b)		(POKEB((a),GHB(b)), POKEB((char *)(a)+1,GLB(b)))
#define	POKEmD(a,b)		(POKEmW((a),GHW(b)), POKEmW((char *)(a)+2,GLW(b)))

#ifdef BIG_ENDIAN
#define	PEEKW(a)		PEEKmW(a)
#define	PEEKD(a)		PEEKmD(a)
#define	POKEW(a,b)		POKEmW(a,b)
#define	POKED(a,b)		POKEmD(a,b)
#else /* LITTLE_ENDIAN */
#define	PEEKW(a)		PEEKiW(a)
#define	PEEKD(a)		PEEKiD(a)
#define	POKEW(a,b)		POKEiW(a,b)
#define	POKED(a,b)		POKEiD(a,b)
#endif


#if 1	/* 値交換マクロ */
#define SWAP_INT(a,b)	do {unsigned long t__; t__ = (unsigned long)(a); (a) = (b); (b) = t__;}while(0)
#define SWAP_PTR(a,b)	do {void *t__; t__ = (void *)(a); (a) = (void *)(b); (b) = (void*)t__;}while(0)
#define SWAP_TYP(y,a,b)	do {y t__; t__ = (a); (a) = (b); (b) = t__;}while(0)
#endif



/*--------------------------------------------------------------------------*/
/* メモリ・文字列関係	*/

#if 1
#define STPCPY(d,s)			(strcpy((d),(s)) + strlen(s))
#define STREND(p)			((p)+ strlen(p))

/* sz はバイト数(各基本型のサイズ単位). アライメントに注意 */
#define MEMCPY1(d, s, sz)	do {char  *d__ = (char *)(d); const char  *s__ = (const char *)(s); int c__ = (unsigned)(sz);    do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPY2(d, s, sz)	do {short *d__ = (short*)(d); const short *s__ = (const short*)(s); int c__ = (unsigned)(sz)>>1; do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPY4(d, s, sz)	do {int   *d__ = (int  *)(d); const int   *s__ = (const int  *)(s); int c__ = (unsigned)(sz)>>2; do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMRCPY4(d, s, sz)	do {int   *d__ = (int  *)(d); const int   *s__ = (const int  *)(s); int c__ = (unsigned)(sz)>>2; while (--c__ >= 0) { d__[c__] = s__[c__]; } } while (0)
#define MEMCPY8(d, s, sz)	do {__int64 *d__ = (__int64 *)(d); const __int64 *s__ = (const __int64 *)(s); unsigned c__ = (unsigned)(sz)>>3; do { *d__++ = *s__++; } while (--c__); } while (0)

/* sz は 値の個数. アライメントに注意 */
#define MEMSET(d, s, sz)	do {char  *d__ = (char *)(d); int c__ = (sz); do { *d__++ = (char) (s); } while(--c__); } while (0)
#define MEMSETB(d, s, sz)	do {char  *d__ = (char *)(d); int c__ = (sz); do { *d__++ = (char) (s); } while(--c__); } while (0)
#define MEMSETW(d, s, sz)	do {short *d__ = (short*)(d); int c__ = (sz); do { *d__++ = (short)(s); } while(--c__); } while (0)
#define MEMSETD(d, s, sz)	do {int   *d__ = (int  *)(d); int c__ = (sz); do { *d__++ = (int)  (s); } while(--c__); } while (0)
#define MEMSETQ(d, s, sz)	do {__int64 *d__ = (__int64 *)(d); int c__ = (sz); do { *d__++ = (long) (s); } while(--c__); } while (0)
#define MEMCPYB(d, s, sz)	do {char  *d__ = (char *)(d); const char  *s__ = (const char *)(s); int c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPYW(d, s, sz)	do {short *d__ = (short*)(d); const short *s__ = (const short*)(s); int c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPYD(d, s, sz)	do {int   *d__ = (int  *)(d); const int   *s__ = (const int  *)(s); int c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPYQ(d, s, sz)	do {__int64 *d__ = (__int64 *)(d); const __int64 *s__ = (const __int64 *)(s); unsigned c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#endif


/*--------------------------------------------------------------------------*/
/* ファイル関係のつじつま会わせ用 */
#define STDIN		stdin
#define STDOUT		stdout
#define STDERR		stdout
//#define FILE_LENGTH(fp)		filelength(fileno((fp)))

#define FIL_NMSZ		(1024+16)



/*--------------------------------------------------------------------------*/
/* 画像処理用 */

/* BPPによる横幅,バイト数変換 */
#define	BPP2BYT(bpp)	(((bpp) > 24) ? 4 : ((bpp) > 16) ? 3 : ((bpp) > 8) ? 2 : 1)
#define	WID2BYT(w,bpp)	(((bpp) > 24) ? (w)<<2 : ((bpp) > 16) ? (w)*3 : ((bpp) > 8) ? (w)<<1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w+1)>>1) : ((bpp) > 1) ? ((w+3)>>2) : ((w+7)>>3))
#define	WID2BYT4(w,bpp)	((WID2BYT(w,bpp) + 3) & ~3)
#define	BYT2WID(w,bpp)	(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w)<<1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))

#if 0
#define	GR_ARGB(a,r,g,b)		((((unsigned char)(a))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define	GR_RGBA(r,g,b,a)		((((unsigned char)(a))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define	GR_RGB(r,g,b)			((((unsigned char)(0))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define GR_RGB_888to555(c)		((((c)>>(16-7))&(0xF8<<7))|(((c)>>(8-2))&(0xF8<<2))|((unsigned char)((c)>>3)))
#define GR_RGB_555to888(c)		((((c)&(0xF8<<7))<<(16-7))|(((c)&(0xF8<<2))<<(8-2))|(((unsigned char)(c)<<3)))
#define GR_RGB555(r,g,b)		((((r)&0xF8)<<7)|(((g)&0xF8)<<2)|(((b)>>3)&0x1f))
#define GR_ARGB_8888to1555(c)	((((c)>>16)&0x8000)|(((c)>>(16-7))&(0xF8<<7))|(((c)>>(8-2))&(0xF8<<2))|((unsigned char)((c)>>3)))
#define GR_ARGB_1555to8888(c)	((((c)&0x8000)<<16)|(((c)&(0xF8<<7))<<(16-7))|(((c)&(0xF8<<2))<<(8-2))|(((unsigned char)(c)<<3)))
#define GR_ARGB1555(a,r,g,b)	(((a)<<15)|(((r)&0xF8)<<7)|(((g)&0xF8)<<2)|(((b)>>3)&0x1f))
#endif


/*--------------------------------------------------------------------------*/
/* デバッグ */
#if 0
//#include "dbg.h"
#else
extern int			debugflag;
#define	DB			if (debugflag)
//#define DB_M()	if (debugflag)	printf("%-16s %6d\n",__FILE__,__LINE__)
#endif




/*--------------------------------------------------------------------------*/


/*------------------------------------------*/
/*------------------------------------------*/
/*------------------------------------------*/

#endif /* DEF_H */
