/*--------------------------------------------------------------------------*/
/* ���ʒ�`																	*/
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
#include <limits.h>


/*--------------------------------------------------------------------------*/

// �R���p�C���̈Ⴂ�΍�
#if   defined(_MSC_VER) || defined(__BORLANDC__) || defined(__MINGW32__)
 #define CPU_X86			// ���g���G���f�B�A�������o�C�g�����̃A�h���X�̃A���C�����g�𖳎��ł���Ƃ��ɒ�`
 #define LITTLE_ENDIAN		// ���g���E�G���f�B�A��(�C���e���n�̒l)��CPU�̂Ƃ���`
 //#define BIG_ENDIAN		// �r�b�O�E�G���f�B�A��(���g���[���n�̒l)��CPU�̂Ƃ���`
 #define NO_ALIGN
 #ifndef __cplusplus
  #define inline		__inline
 #endif
#endif



/*--------------------------------------------------------------------------*/
/* ��{�^�� */

#if   defined(_MSC_VER) || defined(__BORLANDC__)
typedef signed char 		int8_t;
typedef short				int16_t;
typedef int					int32_t;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned			uint32_t;
typedef __int64 		 	int64_t;
typedef unsigned __int64	uint64_t;
#else
#include <stdint.h>
#endif



/*--------------------------------------------------------------------------*/
/* windows �݊��̌^����}�N��. �R���\�[���A�v����windows.h ���g��Ȃ��ꍇ�p */

#if	defined(_WINDOWS_) == 0
#define LOBYTE(w)		((uint8_t)(w))
#define HIBYTE(w)		((uint8_t)((unsigned int)(w) >> 8))
#define LOWORD(l)		((uint16_t)(l))
#define HIWORD(l)		((uint16_t)((unsigned int)(l) >> 16))
#define MAKEWORD(a, b)	(LOBYTE(a)|(LOBYTE(b)<<8))
#define MAKELONG(a, b)	(LOWORD(a)|(LOWORD(b)<<16))
#define ZeroMemory(d,n)	(memset((d),0,(n)))
#endif

#ifndef MAX_PATH
#define MAX_PATH		(1024+16)
#endif

//C++�΍�
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

/* �W���֐�/�}�N���̒u�������� */
#define ISDIGIT(c)  	(((unsigned)(c) - '0') < 10U)
#define ISLOWER(c)		(((unsigned)(c)-'a') < 26U)
#define TOUPPER(c)  	(ISLOWER(c) ? (c) - 0x20 : (c) )

#define OFFSETOF(t,m)	((long)&(((t*)0)->m))			/* offsetof�}�N����ansi�K�i�������C�u�����ɂȂ��ꍇ�����邽�� */

#define ISKANJI(c)		((unsigned char)(c) >= 0x81 && ((unsigned char)(c) <= 0x9F || ((unsigned char)(c) >= 0xE0 && (unsigned char)(c) <= 0xFC)))
#define ISKANJI2(c) 	((unsigned char)(c) >= 0x40 && (unsigned char)(c) <= 0xfc && (c) != 0x7f)

#define MAX(a, b)		((a) > (b) ? (a) : (b)) 	/* �ő�l */
#define MIN(a, b)		((a) < (b) ? (a) : (b)) 	/* �ŏ��l */
#define ABS(a)			((a) < 0 ? -(a) : (a))		/* ��Βl */



/* �r�b�g���Z�⃁�����A�N�Z�X, �A���C�����g�΍���� */
#define BT(n)			(1<<(n))
#define BTST(a,b)		((a) & (1<<(b)))
#define	BB(a,b)			((((unsigned char)(a))<<8)+(unsigned char)(b))
#define	WW(a,b)			((((unsigned short)(a))<<16)+(unsigned short)(b))
#define	BBBB(a,b,c,d)	((((unsigned char)(a))<<24)+(((unsigned char)(b))<<16)+(((unsigned char)(c))<<8)+((unsigned char)(d)))
#define	CCCC(a,b,c,d)	((((unsigned char)(d))<<24)+(((unsigned char)(c))<<16)+(((unsigned char)(b))<<8)+((unsigned char)(a)))

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

/* �o�C�g�P�ʂ̃A�h���X����l�����o���������A�N�Z�X�p�}�N�� */
#define	PEEKB(a)		(*(const unsigned char  *)(a))
#define	POKEB(a,b)		(*(unsigned char  *)(a) = (b))

#ifdef CPU_X86
#define	PEEKiW(a)		(*(const unsigned short *)(a))
#define	PEEKiD(a)		(*(const unsigned long  *)(a))
#define PEEKiB3(s)		((*(const unsigned short*)(s)) | ((*(const unsigned char*)((s)+2))<<16))
#define	POKEiW(a,b)		(*(unsigned short *)(a) = (b))
#define	POKEiD(a,b)		(*(unsigned       *)(a) = (b))
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEiW(a,GLW(b)))
#else
#define	PEEKiW(a)		( PEEKB(a) | (PEEKB((const char *)(a)+1)<< 8) )
#define	PEEKiD(a)		( PEEKiW(a) | (PEEKiW((const char *)(a)+2) << 16) )
#define	POKEiW(a,b)		(POKEB((a),GLB(b)), POKEB((char *)(a)+1,GHB(b)))
#define	POKEiD(a,b)		(POKEiW((a),GLW(b)), POKEiW((char *)(a)+2,GHW(b)))
#define PEEKiB3(s)		BBBB(0, ((const unsigned char*)(s))[2], ((const unsigned char*)(s))[1], ((const unsigned char*)(s))[0])
#define POKEiB3(a,b)	(POKEB((a)+2, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a), GLLB(b)))
#endif

#define	PEEKmW(a)		( (PEEKB(a)<<8) | PEEKB((const char *)(a)+1) )
#define	PEEKmD(a)		( (PEEKmW(a)<<16) | PEEKmW((const char *)(a)+2) )
#define PEEKmB3(s)		BBBB(0, ((const unsigned char*)(s))[0], ((const unsigned char*)(s))[1], ((const unsigned char*)(s))[2])
#define	POKEmW(a,b)		(POKEB((a),GHB(b)), POKEB((char *)(a)+1,GLB(b)))
#define	POKEmD(a,b)		(POKEmW((a),GHW(b)), POKEmW((char *)(a)+2,GLW(b)))
#define POKEmB3(a,b)	(POKEB((a)+0, GHLB(b)), POKEB((a)+1,GLHB(b)), POKEB((a)+2, GLLB(b)))

#ifdef BIG_ENDIAN
#define	PEEKW(a)		PEEKmW(a)
#define	PEEKB3(a)		PEEKmB3(a)
#define	PEEKD(a)		PEEKmD(a)
#define	POKEW(a,b)		POKEmW(a,b)
#define	POKEB3(a,b)		POKEmB3(a,b)
#define	POKED(a,b)		POKEmD(a,b)
#else /* LITTLE_ENDIAN */
#define	PEEKW(a)		PEEKiW(a)
#define	PEEKB3(a)		PEEKiB3(a)
#define	PEEKD(a)		PEEKiD(a)
#define	POKEW(a,b)		POKEiW(a,b)
#define	POKEB3(a,b)		POKEiB3(a,b)
#define	POKED(a,b)		POKEiD(a,b)
#endif


#if 1	/* �l�����}�N�� */
#define SWAP_INT(a,b)	do {unsigned long t__; t__ = (unsigned long)(a); (a) = (b); (b) = t__;}while(0)
#define SWAP_PTR(a,b)	do {void *t__; t__ = (void *)(a); (a) = (void *)(b); (b) = (void*)t__;}while(0)
#define SWAP_TYP(y,a,b)	do {y t__; t__ = (a); (a) = (b); (b) = t__;}while(0)
#endif



/*--------------------------------------------------------------------------*/
/* �������E������֌W	*/

#if 1
#define STPCPY(d,s)			(strcpy((d),(s)) + strlen(s))
#define STREND(p)			((p)+ strlen(p))

/* sz �̓o�C�g��(�e��{�^�̃T�C�Y�P��). �A���C�����g�ɒ��� */
#define MEMCPY1(d, s, sz)	do {char  *d__ = (char *)(d); const char  *s__ = (const char *)(s); int c__ = (unsigned)(sz);    do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPY2(d, s, sz)	do {short *d__ = (short*)(d); const short *s__ = (const short*)(s); int c__ = (unsigned)(sz)>>1; do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPY4(d, s, sz)	do {int   *d__ = (int  *)(d); const int   *s__ = (const int  *)(s); int c__ = (unsigned)(sz)>>2; do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMRCPY4(d, s, sz)	do {int   *d__ = (int  *)(d); const int   *s__ = (const int  *)(s); int c__ = (unsigned)(sz)>>2; while (--c__ >= 0) { d__[c__] = s__[c__]; } } while (0)
#define MEMCPY8(d, s, sz)	do {uint64_t *d__ = (uint64_t *)(d); const uint64_t *s__ = (const uint64_t *)(s); unsigned c__ = (unsigned)(sz)>>3; do { *d__++ = *s__++; } while (--c__); } while (0)

/* sz �� �l�̌�. �A���C�����g�ɒ��� */
#define MEMSET(d, s, sz)	do {char  *d__ = (char *)(d); int c__ = (sz); do { *d__++ = (char) (s); } while(--c__); } while (0)
#define MEMSETB(d, s, sz)	do {char  *d__ = (char *)(d); int c__ = (sz); do { *d__++ = (char) (s); } while(--c__); } while (0)
#define MEMSETW(d, s, sz)	do {short *d__ = (short*)(d); int c__ = (sz); do { *d__++ = (short)(s); } while(--c__); } while (0)
#define MEMSETD(d, s, sz)	do {int   *d__ = (int  *)(d); int c__ = (sz); do { *d__++ = (int)  (s); } while(--c__); } while (0)
#define MEMSETQ(d, s, sz)	do {uint64_t *d__ = (uint64_t *)(d); int c__ = (sz); do { *d__++ = (long) (s); } while(--c__); } while (0)
#define MEMCPYB(d, s, sz)	do {char  *d__ = (char *)(d); const char  *s__ = (const char *)(s); int c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPYW(d, s, sz)	do {short *d__ = (short*)(d); const short *s__ = (const short*)(s); int c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPYD(d, s, sz)	do {int   *d__ = (int  *)(d); const int   *s__ = (const int  *)(s); int c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#define MEMCPYQ(d, s, sz)	do {uint64_t *d__ = (uint64_t *)(d); const uint64_t *s__ = (const uint64_t *)(s); unsigned c__ = (unsigned)(sz); do { *d__++ = *s__++; } while (--c__); } while (0)
#endif


/*--------------------------------------------------------------------------*/
/* �t�@�C���֌W�̂��܉�킹�p */
#define STDIN		stdin
#define STDOUT		stdout
#define STDERR		stdout
//#define FILE_LENGTH(fp)		filelength(fileno((fp)))

#define FIL_NMSZ		(2048+2)
#define FILE_NMSZ		(2048+2)



/*--------------------------------------------------------------------------*/
/* �摜�����p */

/* BPP�ɂ�鉡��,�o�C�g���ϊ� */
#define	BPP2BYT(bpp)	(((bpp) > 24) ? 4 : ((bpp) > 16) ? 3 : ((bpp) > 8) ? 2 : 1)
#define	WID2BYT(w,bpp)	(((bpp) > 24) ? (w)<<2 : ((bpp) > 16) ? (w)*3 : ((bpp) > 8) ? (w)<<1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w+1)>>1) : ((bpp) > 1) ? (((w)+3)>>2) : (((w)+7)>>3))
#define	WID2BYT4(w,bpp)	((WID2BYT(w,bpp) + 3) & ~3)
#define	BYT2WID(w,bpp)	(((bpp) > 24) ? (w)>>2 : ((bpp) > 16) ? (w)/3 : ((bpp) > 8) ? (w)>>1 : ((bpp) > 4) ? (w) : ((bpp) > 2) ? ((w)<<1) : ((bpp) > 1) ? ((w)<<2) : ((w)<<3))

#undef RGB
#undef RGBA
#define	ARGB(a,r,g,b)		((((unsigned char)(a))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define	RGBA(r,g,b,a)		((((unsigned char)(a))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define	RGB(r,g,b)			((((unsigned char)(0))<<24)+(((unsigned char)(r))<<16)+(((unsigned char)(g))<<8)+((unsigned char)(b)))
#define	RGB_B(rgb)			((unsigned char)(rgb))
#define	RGB_G(rgb)			((unsigned char)((rgb)>>8))
#define	RGB_R(rgb)			((unsigned char)((rgb)>>16))
#define	RGB_A(rgb)			((unsigned char)((rgb)>>24))

#if 0
#define GR_RGB_888to555(c)		((((c)>>(16-7))&(0xF8<<7))|(((c)>>(8-2))&(0xF8<<2))|((unsigned char)((c)>>3)))
#define GR_RGB_555to888(c)		((((c)&(0xF8<<7))<<(16-7))|(((c)&(0xF8<<2))<<(8-2))|(((unsigned char)(c)<<3)))
#define GR_RGB555(r,g,b)		((((r)&0xF8)<<7)|(((g)&0xF8)<<2)|(((b)>>3)&0x1f))
#define GR_ARGB_8888to1555(c)	((((c)>>16)&0x8000)|(((c)>>(16-7))&(0xF8<<7))|(((c)>>(8-2))&(0xF8<<2))|((unsigned char)((c)>>3)))
#define GR_ARGB_1555to8888(c)	((((c)&0x8000)<<16)|(((c)&(0xF8<<7))<<(16-7))|(((c)&(0xF8<<2))<<(8-2))|(((unsigned char)(c)<<3)))
#define GR_ARGB1555(a,r,g,b)	(((a)<<15)|(((r)&0xF8)<<7)|(((g)&0xF8)<<2)|(((b)>>3)&0x1f))
#endif


/*--------------------------------------------------------------------------*/
/* �f�o�b�O */
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
