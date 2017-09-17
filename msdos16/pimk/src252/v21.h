#define V21_SIZE	0x4000

int  V21_Init(	int vvSeg, int mmSeg,unsigned mmSzs,
				int emsHdl, unsigned emsSeg, unsigned emsCnt,
				int pln, int xsz,int ysz,int xstart,int ystart,
				unsigned char far *pal,int asp1,int asp2,
				int bcol,int reductFlg,int loopFlg);
void V21_Start(void);
void V21_End(int tone);
void V21_End2(int tone);
void V21_ShowOn(void);
void V21_ShowOff(void);
void V21_GStart(void);
void V21_GEnd(void);
void V21_GCls(void);
void V21_SetTone(int tone);
void far V21_GetLines(void far *dat, unsigned char far *buf);
void far V21_GetLine24(void far *dat, unsigned char far *buf);
void V21_RevX(unsigned Seg);
void V21_RevY(unsigned Seg);
void V21_SetPal(unsigned Seg,int tone);
void V21_PutPx(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
void V21_PutPxScrn(unsigned Seg,int x0,int y0);
void V21_PutPxPart(unsigned Seg);
void V21_PutPxBCol(unsigned Seg);
void V21_PutPxLoop(unsigned Seg);
void V21_SclLoopMode(unsigned Seg, int mode);
void V21_SclUp(unsigned Seg, int d);
void V21_SclDw(unsigned Seg, int d);
void V21_SclLft(unsigned Seg, int d);
void V21_SclRig(unsigned Seg, int d);
unsigned long V21_GetSclOfs(unsigned Seg);

void V21_SetMode(int mode);
void V21_SetVflg(int mode);
void V21_SetVflg(int vflg);
void V21_Sw480(int flg);
int  V21_GetYsize(void);

extern BYTE far V21_scnMode;	// 0:16色400l 1:256色400l 2:256色480l $ff:以外
