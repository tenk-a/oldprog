#define TX_BLACK    0x01    /* �e�L�X�g���� */
#define TX_BLUE     0x21
#define TX_RED	    0x41
#define TX_MAGENTA  0x61
#define TX_GREEN    0x81
#define TX_CYAN     0xa1
#define TX_YELLOW   0xc1
#define TX_WHITE    0xe1
#define TX_BLINK     2
#define TX_REVERSE   4
#define TX_UNDERLINE 8

void Text_Sw(int);
void Text_CursorSw(int);
void Text_CursorBlinkSw(int);
void Text_PFKeySw(int);
void Text_Cls(void);

void Text_Color(int col);

void Text_Box(int x0,int y0, int x1, int y1, int chr, int col);
void Text_BoxFull(int x0,int y0, int x1, int y1, int chr, int col);
void Text_BoxLine(int x0,int y0, int x1, int y1, int flg, int col);

unsigned Text_Locate(int x,int y);
unsigned Text_PutChr(unsigned chr);
unsigned Text_PutCr(void);
char far *Text_PutStr(char far *str);
char far *Text_PutCStr(int col, char far *str);
char far *Text_PutXYCS(int x, int y, int col, char far *str);

void Text_Range(int xsz, int ysz, int x0,int y0);
unsigned Text_LocateAbs(int x,int y);

/*
    col : 0x03:bit 0,1,2    �F�ԍ�0�`7	0:�� 1:�� 2:�� 3:�� 4:�� 5:�� 6:�� 7:��
    	  0x10:bit 4	    �u�����N	on(1)/off(0)
    	  0x20:bit 5	    ���o�[�X	on(1)/off(0)
    	  0x40:bit 6	    ���ްײ�	on(1)/off(0)
*/
