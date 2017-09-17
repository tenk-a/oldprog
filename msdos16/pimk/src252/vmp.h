#define VMP_SIZE    0x4000
#define VWL_SIZE    (0x4000U+0x3010U*2+10)

int VMP_Init(	int vvSeg, int mmSeg,unsigned mmSzs,
    	    	int emsHdl, unsigned emsSeg, unsigned emsCnt,
    	    	int pln, int xsz,int ysz,int xstart,int ystart,
    	    	unsigned char far *pal,int asp1,int asp2,
    	    	int bcol,int reductFlg,int loopFlg);
void VMP_Start(void);
void VMP_End(int tone);
void VMP_GStart(void);
void VMP_GEnd(void);
void VMP_ShowOn(void);
void VMP_ShowOff(void);
void VMP_GCls(void);
void VMP_SetTone(int tone);
void far VMP_GetLines(void far *dat, unsigned char far *buf);
void VMP_RevX(unsigned Seg);
void VMP_RevY(unsigned Seg);
void VMP_SetPal(unsigned Seg,int tone);
void VMP_PutPx(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
void VMP_PutPxScrn(unsigned Seg,int x0,int y0);
void VMP_PutPxPart(unsigned Seg);
void VMP_PutPxBCol(unsigned Seg);
void VMP_PutPxLoop(unsigned Seg);
void VMP_SclLoopMode(unsigned Seg, int mode);
void VMP_SclUp(unsigned Seg, int d);
void VMP_SclDw(unsigned Seg, int d);
void VMP_SclLft(unsigned Seg, int d);
void VMP_SclRig(unsigned Seg, int d);
unsigned long VMP_GetSclOfs(unsigned Seg);

int  VMP_InitWl(    int vvSeg, int mmSeg,unsigned mmSzs,
    	    	int emsHdl, unsigned emsSeg, unsigned emsCnt,
    	    	int pln, int xsz,int ysz,int xstart,int ystart,
    	    	unsigned char far *pal,int asp1,int asp2,
    	    	int bcol,int reductFlg,int loopFlg);
void VMP_StartWl(void);
void VMP_EndWl(int tone);
void VMP_GStartWl(void);
void VMP_GEndWl(void);
void VMP_SetToneWl(int tone);
void VMP_SetPalWl(unsigned Seg,int tone);
void far VMP_GetLineWl(void far *dat, unsigned char far *buf);
