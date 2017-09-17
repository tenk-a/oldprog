#define V98_SIZE	0x4000

int V98_Init(	int vvSeg, int mmSeg,unsigned mmSzs,
				int emsHdl, unsigned emsSeg, unsigned emsCnt,
				int pln, int xsz,int ysz,int xstart,int ystart,
				unsigned char far *pal,int asp1,int asp2,
				int bcol,int reductFlg,int loopFlg);
void V98_Start(void);
void V98_End(int tone);
void V98_GStart(void);
void V98_GEnd(void);
void V98_ShowOn(void);
void V98_ShowOff(void);
void V98_GCls(void);
void V98_SetTone(int tone);
void far V98_GetLines(void far *dat,unsigned char far *buf);
void far V98_GetLine24(void far *dat,unsigned char far *buf);
void V98_RevX(unsigned Seg);
void V98_RevY(unsigned Seg);
void V98_SetPal(unsigned Seg,int tone);
void V98_PutPx(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
void V98_PutPxScrn(unsigned Seg,int x0,int y0);
void V98_PutPxPart(unsigned Seg);
void V98_PutPxBCol(unsigned Seg);
void V98_PutPxLoop(unsigned Seg);
void V98_SclLoopMode(unsigned Seg, int mode);
void V98_SclUp(unsigned Seg, int d);
void V98_SclDw(unsigned Seg, int d);
void V98_SclLft(unsigned Seg, int d);
void V98_SclRig(unsigned Seg, int d);
unsigned long V98_GetSclOfs(unsigned Seg);

void V98_SetDitMode(int vvSeg, int force256flg);
