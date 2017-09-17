/*-------------------------------------------------------------------------*/
typedef unsigned char	byte;
typedef unsigned    	word;
typedef unsigned long	dword;
#ifndef toascii /* ctype.h ‚ªinclude‚³‚ê‚Ä‚¢‚È‚¢‚Æ‚« */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif
#ifndef CT_KJ1	/* jctype.h ‚ªinclude‚³‚ê‚Ä‚¢‚È‚¢‚Æ‚« */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif
#define pr(s) fputs(s,stdout)

extern int DirEntryGet(byte far *,byte far *,word);
/*extern byte *DirEntrykey,DirEntrydir[200];*/

/*---------------------------------------------------------------------------*/
struct pihdr_t {
    byte *name;
    int  mode;
    int  pln;
    int  r1,r2;
    int  x_siz,y_siz;
    int  bx,by,bflg;
    int  exsiz;
    byte sv[5];
    byte *comment;
    byte *comment2;
    byte *user;
    byte *ex;
    word cmtlen;
    word cmt2len;
    word userlen;
    long dat;
    int  hdrsiz;
    int  pallen;
    byte pal[3*256];
};
typedef struct pihdr_t pihdr_t;

struct chgopt_t {
    int  bflg;
    int  bx,by;
    int  xsiz,ysiz;
    int  r1,r2;
    byte *palfile;
    byte sv[5];
    byte *comment;
    byte *comment2;
    byte *user;
    int  palcnt;
    byte pal[3*256];
};
typedef struct chgopt_t chgopt_t;

#define COMMENT_SIZ  0x1000U
#define COMMENT2_SIZ 0x200U
#define EX_MAX 0x600U
#define X_COMMENT_SIZ 0x800U
#if 1
  #define BUFSZ     (20*1024)
  extern byte gComment[COMMENT_SIZ+2];
  extern byte gComment2[COMMENT2_SIZ+2];
  extern byte gExBuf[EX_MAX+2];
  extern byte gXComment[X_COMMENT_SIZ+2];
#else
  #define BUFSZ     (COMMENT_SIZ+COMMENT2_SIZ+EX_MAX+X_COMMENT_SIZ)
  #define gComment  gBuf
  #define gComment2 (gBuf+COMMENT_SIZ+2)
  #define gExBuf    (gBuf+COMMENT_SIZ+2+COMMENT2_SIZ+2)
  #define gXComment (gBuf+COMMENT_SIZ+2+COMMENT2_SIZ+2+EX_MAX+2)
#endif
extern byte gBuf[BUFSZ+16];

void ChgExt(byte *onam, byte *inam, byte *ext);
byte *ChgExtTmp(byte *s, byte *p);
FILE *fopen_e(byte *name, byte *mode);
void rename_e(byte *old_nam, byte *new_nam);
int RdMagHdr(byte *magname, chgopt_t *p);
void PriMagHdr(byte *name,chgopt_t *q);
