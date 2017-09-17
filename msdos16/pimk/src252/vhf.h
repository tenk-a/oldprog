#define VHF_SIZE	0x4000

int  VHF_Init(	int vvSeg, int mmSeg,unsigned mmSzs,
				int emsHdl, unsigned emsSeg, unsigned emsCnt,
				int pln, int xsz,int ysz,int xstart,int ystart,
				unsigned char far *pal,int asp1,int asp2,
				int bcol,int reductFlg,int loopFlg);
void VHF_Start(void);
void VHF_End(int tone);
void VHF_ShowOn(void);
void VHF_ShowOff(void);
void VHF_GStart(void);
void VHF_GEnd(void);
void VHF_GCls(void);
void VHF_SetTone(int tone);
void far VHF_GetLines(void far *dat ,unsigned char far *buf);
void far VHF_GetLine24(void far *dat ,unsigned char far *buf);
void VHF_RevX(unsigned Seg);
void VHF_RevY(unsigned Seg);
void VHF_SetPal(unsigned Seg,int tone);
void VHF_PutPx(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
void VHF_PutPxScrn(unsigned Seg,int x0,int y0);
void VHF_PutPxPart(unsigned Seg);
void VHF_PutPxBCol(unsigned Seg);
void VHF_PutPxLoop(unsigned Seg);
void VHF_SclLoopMode(unsigned Seg, int mode);
void VHF_SclUp(unsigned Seg, int d);
void VHF_SclDw(unsigned Seg, int d);
void VHF_SclLft(unsigned Seg, int d);
void VHF_SclRig(unsigned Seg, int d);
unsigned long VHF_GetSclOfs(unsigned Seg);

int  VHF_InitSF(int vvSeg, int mmSeg,unsigned mmSzs,
				int emsHdl, unsigned emsSeg, unsigned emsCnt,
				int pln, int xsz,int ysz,int xstart,int ystart,
				unsigned char far *pal,int asp1,int asp2,
				int bcol,int reductFlg,int loopFlg);
