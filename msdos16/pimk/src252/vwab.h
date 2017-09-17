#define VWAB_SIZE	0x4000

int  VWAB_Init( int vvSeg, int mmSeg,unsigned mmSzs,
				int emsHdl, unsigned emsSeg, unsigned emsCnt,
				int pln, int xsz,int ysz,int xstart,int ystart,
				unsigned char far *pal,int asp1,int asp2,
				int bcol,int reductFlg,int loopFlg);
void VWAB_Start(void);
void VWAB_End(int tone);
void VWAB_ShowOn(void);
void VWAB_ShowOff(void);
void VWAB_GStart(void);
void VWAB_GEnd(void);
void VWAB_GCls(void);
void VWAB_SetTone(int tone);
void far VWAB_GetLines(void far *dat,unsigned char far *buf);
void far VWAB_GetLine24(void far *dat,unsigned char far *buf);
void VWAB_RevX(unsigned Seg);
void VWAB_RevY(unsigned Seg);
void VWAB_SetPal(unsigned Seg,int tone);
void VWAB_PutPx(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
void VWAB_PutPxScrn(unsigned Seg,int x0,int y0);
void VWAB_PutPxPart(unsigned Seg);
void VWAB_PutPxBCol(unsigned Seg);
void VWAB_PutPxLoop(unsigned Seg);
void VWAB_SclLoopMode(unsigned Seg, int mode);
void VWAB_SclUp(unsigned Seg, int d);
void VWAB_SclDw(unsigned Seg, int d);
void VWAB_SclLft(unsigned Seg, int d);
void VWAB_SclRig(unsigned Seg, int d);
unsigned long VWAB_GetSclOfs(unsigned Seg);

void VWAB_SetMode(int mode, int hpos, int vpos);
//int  VWAB_CheckBord(void);
int  VWAB_SetLoadMode(int xx,int yy);
