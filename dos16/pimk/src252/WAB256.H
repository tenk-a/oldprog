void WAB256_Sw2000(int);
void WAB256_GetScreenMode(void);
void WAB256_SetBank(int bnk);
unsigned char far *WAB256_LineAddr(int yy);
void WAB256_Init(int mode,int hpos,int vpos);
void WAB256_ShowOn(void);
void WAB256_ShowOff(void);
void WAB256_AccessOn(void);
void WAB256_AccessOff(void);
void WAB256_Cls(void);
void WAB256_SetPal(int tone, unsigned char far *pal);
void WAB256_SetTone(int tone);
unsigned char far *WAB256_PutXYN(int x,int y,int len, void far *pix);
