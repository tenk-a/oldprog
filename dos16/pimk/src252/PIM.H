	// n < N_WAB ‚Æ‚©‚µ‚Ä‘å¬”äŠr‚µ‚Ä‚é‚Ì‚ÅA’l‚â‡”Ô‚ð‚©‚¦‚Ä‚Í‚¢‚¯‚È‚¢^^;
#define N_S 	0
#define N_WL	1
#define N_MP	2
#define N_WAB	3
#define N_21	4
#define N_HF	5
#define N_SF	6
#define N_HFB	7
#define N_CNT	8
void PriMsgSet(char far *fname,int xsz,int ysz, int x0,int y0, int pln,
				int asp1,int asp2, char far *saver,
				char far *artist, char far *cmt);
void PriCmt(void);
//int PriDoc(void);
//void PriHlp(int tone);

int KeyLoop(unsigned n16seg,VVF *p,int keyWaitFlg,
			int tone, int loopFlg,int kabeFlg);
