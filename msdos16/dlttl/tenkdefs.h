#define _S	static
typedef unsigned char	byte;
typedef unsigned		word;
typedef unsigned long	dword;
#ifndef	toascii	/* ctype.h ‚ªinclude‚³‚ê‚Ä‚¢‚È‚¢‚Æ‚« */
  #define toupper(c)  (	((c) >=	'a'	&& (c) <= 'z') ? (c) - 0x20	: (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <=	'9')
#endif
#ifndef	CT_KJ1	/* jctype.h	‚ªinclude‚³‚ê‚Ä‚¢‚È‚¢‚Æ‚« */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f)	|| ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) !=	0x7f)
#endif
extern int DirEntryGet(byte far *,byte far *,word);
/*extern byte *DirEntrykey,DirEntrydir[200];*/
#define REGI
#define DIRE
