/*-------------------------------------------------------------------------*/
#define _S  static
typedef unsigned char	byte;
typedef unsigned    	word;
typedef unsigned long	dword;

extern int DirEntryGet(byte far *,byte far *,word);

#define MAKE_FP(s,o)  ((void far *)(((dword)(s)<<16) | ((o)&0xffff)))
#define PTR_OFF(p)  ((unsigned short) (p))
#define PTR_SEG(p)  ((unsigned short)((unsigned long)(void far*)(p) >> 16))
#define HUGE	far

#ifndef toascii /* ctype.h がincludeされていないとき */
  #define toupper(c)  ( ((c) >= 'a' && (c) <= 'z') ? (c) - 0x20 : (c) )
  #define isdigit(c)  ((c) >= '0' && (c) <= '9')
#endif
#ifndef CT_KJ1	/* jctype.h がincludeされていないとき */
  #define iskanji(c)  (((c)>=0x81 && (c)<=0x9f) || ((c)>=0xE0 && (c)<=0xfc))
  #define iskanji2(c) ((c) >= 0x40 && (c) <= 0xfc && (c) != 0x7f)
#endif

#ifdef	EXT
    #define EXTERN
#else
    #define EXTERN  extern
#endif

/*-------------------------------------------------------------------------*/
#define WaitVsync() {while((inp(0x60)&0x20));	while(!(inp(0x60)&0x20));}

#define setGRB(n,g,r,b) do{\
    	      outp(0xa8, (n));\
    	      outp(0xaa, (g));\
    	      outp(0xac, (r));\
    	      outp(0xae, (b));\
    	  }while(0)
struct RAMPAL {
    byte id[10];
    byte tone;
    byte rsv[5];
    byte grb[16][3];
};
unsigned _dos_read(int hno, void far *buf, unsigned siz, unsigned *cnt);
unsigned _dos_write(int hno, void far *buf, unsigned siz, unsigned *cnt);

struct RAMPAL far *RPal_Search(void);
int RPal_Get(word *tone, byte *rgb);
void RPal_Set(int tone, byte *rgb);
void RPal_ChgTone(int tone);

void ActGPage(int n);
void DspGPage(int n);
void DspSwitch(int t,int g);
void DspGrph(int mode);
void DspText(int mode);

void ChgExt(byte *onam, byte *inam, byte *ext);
void far *LtoFP(long);
void far *FPtoFP(void far *);
long CntVsync(void);
int  Chk86(void);
/*-------------------------------------------------------------------------*/
EXTERN byte gChk,gDebSclFlg;

// Ｐiファイル関係
extern int  PiLd_ph;	    	    	//入力する Piファイルのハンドル
extern byte PiLd_dotX,PiLd_dotY;    	//ドット比
extern byte PiLd_plnCnt;    	    	//プレーン数
extern byte PiLd_palMode;   	    	//パレット・モード
extern int  PiLd_sizX,PiLd_sizY;    	//画像サイズ
extern int  PiLd_sizY0;
extern int  PiLd_topX,PiLd_topY;    	//基点
extern int  PiLd_bcol;	    	    	//透明色
extern byte PiLd_macName[5];	    	//セーバ名
extern byte PiLd_pal[16*3]; 	    	//パレット (Display)
extern byte PiLd_pal256[256*3];     	//パレット (data)
extern byte PiLd_colTbl[16*16];     	//展開で使用する色表
extern long PiLd_filSiz;    	    	//ファイルサイズ
extern byte far *PiLd_wkBuf;	    	//展開用のバッファ(仮想の２ライン込)
extern byte far *PiLd_wkBuf2;	    	//PiLd_wkBufで実際のデータの先頭
extern dword PiLd_wkBufMax; 	    	//PiLd_wkBufのサイズ
extern dword PiLd_wkSiz;    	    	//PiLd_wkBufに展開されたサイズ
extern word PiLd_wkBufMaxS; 	    	//PiLd_wkBufのサイズ(ｾｸﾞﾒﾝﾄ単位)
extern int  PiLd_wkFlg;     	    	//PiLd4aを使うかPiLd4bを使うか
extern byte far *PiLd_commentBuf;   	//コメント用のバッファ

void Exit(int);
void DosError(int);

int  PiLd_LdInit(word segsiz);
void PiLd_ColTblInit(void);
int  PiLd_ReadHdr(void);
int  PiLd_Load4a(void);
int  PiLd_Load8(void);
int  PiLd_Load4c(void);
int  PiLd_ReadColor2(int);

void PiLd_RdInitVRam(void);
int  PiLd_RdInit(word siz);
int  PiLd_RdFirst(word hdl);
int  PiLd_RdByte(void);
int  PiLd_RdBit1(void);
int  PiLd_RdPos(void);
int  PiLd_RdColPos(void);
dword PiLd_RdLen(void);

void Decol_Dit16(byte far *p, int w, int h, byte *pal);
void Decol_Col16(byte far *p, int w, int h, byte *pal, byte far *wk,int wksiz);
void Decol_Mono16(byte far *p, int w, int h, byte *pal, byte far *wk,
    	    	     int wksiz, int gosaFlg);

void Gv_Init(void);
int  Gv_Cls(void);
int  Gv_Cnv4a(byte far *p, int xsiz, int ysiz);
int  Gv_Cnv4b(byte far *p, int xsiz, int ysiz);
int  Gv_Dsp(byte far *p,int xsiz,int ysiz,int sx,int sy);
int  Gv_Dsp2(byte far *p,int xsiz,int ysiz,int sx,int sy);
int  Gv_DspP(byte far *p,int xsiz,int ysiz,int sx,int sy,int l);
int  Gv_DspSml(byte far *p,int xsiz,int ysiz);
int  Gv_DspOvrLd(byte far *p,int xsiz,int ysiz,int sx,int sy,int col);
int  Gv_Dsp640(byte far *p,word ysiz);
int  Gv_GUp(int yy);
int  Gv_GDwn(int yy);
int  Gv_GLft(int xx);
int  Gv_GRgt(int xx);
int  Gv_WkCpy(byte far *p,int xsz,int ysz,int x0,int y0,
    	int w,int h,int gx,int gy);
int  Gv_RevX(byte far *p, int xsiz, int ysiz);
int  Gv_RevY(byte far *p, int xsiz, int ysiz);
int  Gv_GetVRam(byte far *p, int xsz, int ysz, int mode);
int  Gv_GetVRamQ(byte far *p, int xsz, int ysz, int mode);
void ror8x8(byte far *);
extern int Gv_btmLine;
extern int Gv_ofsFlg;

extern int Gdc_ofsX,Gdc_ofsY;
void Gdc_Scl(int ddx, int ddy);
void Gdc_SclInit(void);

void Key_BufClr(void);
int  Key_GetShift(void);
int  Key_GetWait(void);
long Key_Get(void);
int  Key_Init(void);

#ifdef HF
extern int  gFBmode;
void Hf_Dsp(byte huge *p, int xsize, int ysize, int sx, int sy);
void Hf_Cls(void);
int Hf_Init(void);  //0:FBがない  1:SF	2:HF
void Hf_Term(void);
void Hf_DspOff(void);
#endif
#ifdef MP
void Mp_Init(void);
void Mp_Term(void);
void Mp_SetPal(byte far *p);
int  Mp_Cnv(byte far *p, int xsiz, int ysiz);
int  Mp_WkCpy(byte far *p,int xsz,int ysz,int x0,int y0,
    	int w,int h,int gx,int gy);
#endif
