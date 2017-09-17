typedef short SHORT;
typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;


#ifndef CT_KJ1	/* jctype.h ��include����Ă��Ȃ��Ƃ� */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif

#ifndef toascii /* ctype.h ��include����Ă��Ȃ��Ƃ� */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif

#define FNAMESIZE   128
#define WRTBUFSIZE  0x6000


#ifdef __TURBOC__   /* TC(++),BC++�p����... ���Ԃ� QC �ł����v����...?    */
    /*MSDOS�Ȋ��Ȃ�*/
    #define FAR_ALLOC	/* calloc�̑����farcalloc ���g���ꍇ, ��`	    */
    //#define DIRENTRY/* ܲ��ޥ���ޑΉ���̧�ٖ��擾ٰ�݂��g��.	    	    */
    #define FDATEGET/* �t�@�C���̓��t��ۑ�����I�v�V������t��     	    */
#else
    #define huge
    #define far
#endif

/*---------------------------------------------------------------------------*/
/*  	    	    ���@�ʁ@	    	    	    	    	    	     */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef FAR_ALLOC
#include <alloc.h>
#endif
#if 0
    #define KEYBRK  /* CTRL-C �� abort �ɂ���...? */
    #include <signal.h>
#endif


#define FNAMESIZE   128

