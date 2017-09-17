typedef unsigned char UCHAR;
typedef unsigned int  USHORT;
typedef unsigned int  UINT;
typedef unsigned long ULONG;
typedef signed char   SCHAR;
typedef signed int    SSHORT;
typedef signed int    SINT;
typedef signed long   SLONG;


typedef struct VESA_INFO {
    UCHAR  id[4];   	    	//00	'VESA'
    USHORT version; 	    	//04
    UCHAR far *oemNames;    	//06
    UCHAR flags[4]; 	    	//0A
    USHORT far *videoModeTbl;	//0E
    USHORT totalMem;	    	//12
    //UCHAR rsv[242];	    	//14
} VESA_INFO;

typedef struct VESA_MODEINFO {
    USHORT mode;    	    	//00
    UCHAR  winAatr; 	    	//02
    UCHAR  winBatr; 	    	//03
    USHORT winPagSiz;	    	//04	N Kbytes
    USHORT winSiz;  	    	//06	N Kbytes
    USHORT winAseg; 	    	//08
    USHORT winBseg; 	    	//0A
    ULONG  winAdr;  	    	//0C
    USHORT scanlineBytes;   	//10
    USHORT gxsz;    	    	//12
    USHORT gysz;    	    	//14
    UCHAR  chrXsz;  	    	//16
    UCHAR  chrYsz;  	    	//17
    UCHAR  plnCnt;  	    	//18
    UCHAR  pbits;   	    	//19
    UCHAR  bnkCnt;  	    	//1A
    UCHAR  type;    	    	//1B
    UCHAR  bnkSz;   	    	//1C
    UCHAR  imgPagCnt;	    	//1D
    UCHAR  rsv1;    	    	//1E = 1
    UCHAR  mskBitsR;	    	//1F
    UCHAR  mskBitPosR;	    	//20
    UCHAR  mskBitsG;	    	//21
    UCHAR  mskBitPosG;	    	//22
    UCHAR  mskBitsB;	    	//23
    UCHAR  mskBitPosB;	    	//24
    UCHAR  mskBitsA;	    	//25
    UCHAR  mskBitPosA;	    	//26
    UCHAR  drctColAtr;	    	//27
    UCHAR  rsv[216];	    	//28
} VESA_MODEINFO;


int far VESA_GetInfo(void far *infp);
int far VESA_GetModeInfo(USHORT mode, void far *minfp);
int far VESA_SetMode(USHORT mode);
int far VESA_GetMode(void);
int far VESA_GetVideoStatSize(USHORT stt);
int far VESA_GetVideoStat(USHORT stt, void far *sttBuf);
int far VESA_SetVideoStat(USHORT stt, void far *sttBuf);
int far VESA_SetWinPos(USHORT num, USHORT pos);
int far VESA_GetWinPos(USHORT num);
int far VESA_SetLogicalXsize(USHORT xsz);
int far VESA_GetLogicalXsize(void);
int far VESA_SetOrgPos(USHORT x0, USHORT y0);
int far VESA_GetOrgPos(void);
int far VESA_SetPalMode(USHORT palbit);
int far VESA_GetPalMode(void);

enum {M320x200=0x01,M640x480=0x02,M800x600=0x04,M1024x768=0x08,M1280x1024=0x10};
extern UCHAR far VESA_m8s, far VESA_m15s, far VESA_m16s, far VESA_m24s;
extern VESA_INFO far VESA_info;
extern VESA_MODEINFO far VESA_modeInfo;

int VESA_Init();
