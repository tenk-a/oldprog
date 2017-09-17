#ifndef _TENKDEFS_H_
#define _TENKDEFS_H_

#ifdef OS9
/* os9/09 MW-C*/
  typedef char  	byte;
  typedef unsigned  word;
  typedef short 	inte;
  typedef long  	dword;
  #define REGI  	register
  #define DIRE  	direct
  #define __(a) 	()
  #define C_DIRSP	'/'
#else
/* MS-DOS */
  #define REGI
  #define DIRE
  #define __(a) 	a
  #define C_DIRSP	'\\'

  typedef unsigned char byte;
  typedef unsigned short	word;
  typedef short 		inte;
  typedef unsigned long dword;
  typedef long double	ldouble;
  typedef void far *void_fp;
  typedef byte far *byte_fp;
  typedef word far *word_fp;

  #define MAKE_FP(s,o)  ((void far *)(((lword)(s)<<16) | ((o)&0xffff)))
  #define PTR_OFF(p)	((word) (p))
  #define PTR_SEG(p)	((word)((lword)(void far *)(p) >> 16))
  #define htoi(s)		((inte)strtoul((s),(char **)0,16))
#endif

/*  */
#define global
#define C_CR		0x0d
#define C_LF		0x0a
#define eputs(s)	fputs((s),stdout)
#define ep_exit(s)  (fputs((s),stdout),exit(1))

#ifndef toascii /* ctype.h ‚ªinclude‚³‚ê‚Ä‚¢‚È‚¢‚Æ‚« */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif
#ifndef CT_KJ1  /* jctype.h ‚ªinclude‚³‚ê‚Ä‚¢‚È‚¢‚Æ‚« */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif

#ifdef  EXT
	#define EXTERN
#else
	#define EXTERN  extern
#endif

#ifdef DEBUG
EXTERN word gDebFlg_;
#define DEBUG_MODE(n) (gDebFlg_ = (n))
#define CHK_DEBUG_MODE() (gDebFlg_)
#define MSG(s)  ((gDebFlg_)? printf("%s\n",s) : 0)
#define MSGF(s) ((gDebFlg_)? (printf s):0)
#define PRNF(s) (printf s)
#define TRACE		((gDebFlg_) ? printf("%s %d\n",__FILE__,__LINE__) : 0)
#define TRACES(s) ((gDebFlg_)? printf("%s %d : %s\n",__FILE__,__LINE__,s) : 0)
#define TRACEF(s) ((gDebFlg_)?(printf("%s %d :",__FILE__,__LINE__),printf s):0)
#else
#define DEBUG_MODE(n)
#define MSG(s)
#define MSGF(s)
#define PRNF(s)
#define TRACE
#define TRACES(s)
#define TRACEF(s)
#endif

#endif  /* End Of _TENKDEFS_H_ */
