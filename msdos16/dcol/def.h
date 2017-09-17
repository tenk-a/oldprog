typedef short SHORT;
typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;


#ifndef CT_KJ1	/* jctype.h がincludeされていないとき */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif

#ifndef toascii /* ctype.h がincludeされていないとき */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif

#define FNAMESIZE   128
#define WRTBUFSIZE  0x6000


#ifdef __TURBOC__   /* TC(++),BC++用かな... たぶん QC でも大丈夫かも...?    */
    /*MSDOSな環境なら*/
    #define FAR_ALLOC	/* callocの代わりにfarcalloc を使う場合, 定義	    */
    //#define DIRENTRY/* ﾜｲﾙﾄﾞ･ｶｰﾄﾞ対応のﾌｧｲﾙ名取得ﾙｰﾁﾝを使う.	    	    */
    #define FDATEGET/* ファイルの日付を保存するオプションを付加     	    */
#else
    #define huge
    #define far
#endif

/*---------------------------------------------------------------------------*/
/*  	    	    共　通　	    	    	    	    	    	     */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef FAR_ALLOC
#include <alloc.h>
#endif
#if 0
    #define KEYBRK  /* CTRL-C で abort 可にする...? */
    #include <signal.h>
#endif


#define FNAMESIZE   128

