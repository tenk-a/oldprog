#define VHFB_SIZE	0x4000

int  VHFB_Init(	int vvSeg, int mmSeg,unsigned mmSzs,
				int emsHdl, unsigned emsSeg, unsigned emsCnt,
				int pln, int xsz,int ysz,int xstart,int ystart,
				unsigned char far *pal,int asp1,int asp2,
				int bcol,int reductFlg,int loopFlg);
void VHFB_Start(void);
void VHFB_End(int tone);
void VHFB_ShowOn(void);
void VHFB_ShowOff(void);
void VHFB_GStart(void);
void VHFB_GEnd(void);
void VHFB_GCls(void);
void VHFB_SetTone(int tone);
//void VHFB_RevX(unsigned Seg);
//void VHFB_RevY(unsigned Seg);
void VHFB_SetPal(unsigned Seg,int tone);
void VHFB_PutPx(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
void VHFB_PutPxScrn(unsigned Seg,int x0,int y0);
void VHFB_PutPxPart(unsigned Seg);
void VHFB_PutPxBCol(unsigned Seg);
void VHFB_PutPxLoop(unsigned Seg);
void VHFB_SclLoopMode(unsigned Seg, int mode);
void VHFB_SclUp(unsigned Seg, int d);
void VHFB_SclDw(unsigned Seg, int d);
void VHFB_SclLft(unsigned Seg, int d);
void VHFB_SclRig(unsigned Seg, int d);
unsigned long VHFB_GetSclOfs(unsigned Seg);
