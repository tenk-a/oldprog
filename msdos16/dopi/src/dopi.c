/*
    DoPI.exe  Pi�摜���[�_
    ver 1.11
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include <conio.h>
#include <alloc.h>
#include <signal.h>
#include <pc98.h>
#include "dopi.h"
#define pr(s)	printf(s)

    byte *gNameVer = "DoPI v1.11";  //����";

#define ATMO	1

//#define PILD4C    1
#define DEB_DMP 1
#ifdef DEB_DMP
_S byte gDmpFlg;    	    //�f�o�b�O�p. �W�J�f�[�^��DUMP���邩�ǂ���
#endif
#ifdef PILD4C	    	    //���͂₱�̃��[�`���͎g���Ȃ��Ȃ��Ă���
_S byte gPiLd4cFlg = 0;     //C�ŏ����ꂽ�W�J���[�`�����g�p���邩�ǂ���
#endif
#define DEBSCL
byte gDebSclFlg;    	    //�X�N���[���̃f�o�b�O�p...


#define DFLT_TOON   50
byte oDfltTone = DFLT_TOON; //-v,-lv�ł̏ȗ����̃g�[��

byte *gPrgName;     	    //�N�������Ƃ��̃R�}���h��
byte gChk;  	    	    //�f�o�b�N�p�t���O
byte gChkPiLd4a;    	    //PiLd4a,PiLd4b�̃f�o�b�O�p�t���O
word gPalTone;	    	    //�p���b�g�̃g�[��
long PiLd_filSiz;   	    //���[�h����Pi�摜�̃t�@�C���T�C�Y
_S int g256mode;    	    //color 256
#ifdef	HF
//int  gFBmode;     	    //
_S byte gHfMode = 0;	    //HyPER-FRAME+ ���g��
_S byte gEx256Flg = 0;	    //1=�O��̕\���͂g�e�ɕ\������. 2=PC386M(P)
#endif
_S int	gBx,gBy;    	    //-o �ł̊�_
_S int	gSx,gSy;    	    //�摜�̕\���ʒu
_S int	gL200flg;   	    //200���C���摜���ǂ���
_S int	gFx,gFy;    	    //�X�N���[���ł��邩�ǂ����̃t���O
_S word gEndPalTone;	    //�I�����̃p���b�g�̃g�[��
_S byte gEndPalToneFlg;     // -lv ���w�肳�ꂽ���ǂ���
_S byte gEndGrphOffFlg;     // -lvf ���w�肳�ꂽ���ǂ���
_S byte gKeyWaitFlg;	    //�L�[���̓��[�h�ɂ��邩�ǂ���
_S byte gClsFlg;    	    //�\�����ɉ�ʏ������邩�ǂ���
_S byte gBxByMode;  	    //-o ���w�肳�ꂽ���ǂ���, -oa���ǂ���
_S byte gOvrLdFlg;  	    //��ʍ������邩�ǂ���
_S byte gOvrLdCol;  	    //��ʍ����ł̓����F
_S byte gOvrLdFlg2; 	    //Pi�w�b�_�ɖ��ߍ��܂ꂽ�����F�𖳎����邩�ǂ���
_S byte gUraUseMode;	    //��VRAM �ɏ������݂��s�Ȃ����ǂ���
_S byte gUraWrtMode;	    //-lb[N] �� N.
_S byte gVRamGetNo; 	    //-kg[N] �� N.
_S byte gPalFlg;    	    //-v ���w�肳�ꂽ���ǂ���
_S byte gUraLoadFlg;	    //-b �ŗ�VRAM�Ƀ��[�h���邩�ǂ���
_S byte gGdcSclFlg; 	    //GDC�X�N���[�����\�t�g�E�F�A�]����
_S byte gSclBakFlg; 	    //�X�N���[���Ńo�b�N���Ďn�_�ɖ߂邩
_S byte gSclLoopFlg;	    //�㉺���E�Œ[�܂ł����Ƃ����Α��ɍs�����~�܂邩
_S byte gSmlFlg;    	    //�k���\�����邩�ǂ���
_S byte gTxtFlg;    	    //-t:���[�h�t�@�C�����A�R�����g�\�������邩�ǂ���
_S byte gInfoFlg;   	    //-i:�w�b�_��\�����邩
_S int	gMieMode;   	    //-a:���h����\�� 1,2
_S byte gMie2Mode;  	    //-a:���h����\�� 3,4
_S byte gRdBufVRamFlg;	    //���̓o�b�t�@��VRAM�ɂƂ邩�ǂ���
_S byte gToneDownFlg;	    //��ʏ����Ńt�B�[�h�A�E�g���邩�ǂ���
_S byte gToneUpFlg; 	    //-a:���h����\�� 5  �t�B�[�h�C��
_S byte gI8086Flg;  	    //CPU��8086(V30)��186�ȏォ
_S int	gVsWaitCnt; 	    //VSYNC�̐M�����݂Ĉ�莞�Ԃɉ��񃋁[�v�ł�����
_S int	gVsWaitCnt0;	    //-w[N] �ł� N
_S int	gVsWaitCnt1;	    //-ws[N,M] �ł� N
_S int	gVsWaitCnt2;	    //-ws[N,M] �ł� M

_S byte g256col16pal[48] = {
     0, 0, 0,
     0, 0,15,
    15, 0, 0,
    15, 0,15,
     0, 5, 0,
     0, 5,15,
    15, 5, 0,
    15, 5,15,
     0,10, 0,
     0,10,15,
    15,10, 0,
    15,10,15,
     0,15, 0,
     0,15,15,
    15,15, 0,
    15,15,15
};
/*---------------------------------------------------------------------------*/
void TCls(void)
    // �e�L�X�g��ʃN���A
{
    if (gChk)
    	return;
    printf ("\x1b[2J\x1b[>5h\x1b[>1h");/*÷�ĉ�ʸر;����off;PF�\��off*/
}

void Exit(int n)
    // �v���O�����I��
{
    if (Gdc_ofsX || Gdc_ofsY) { //�uRAM�\����GDC�����������܂܂Ȃ�A����
    	int j;
    	j = 50;
    	while (--j > 0)
    	    WaitVsync();
    	Gdc_SclInit();
    }
    ActGPage(0);
    DspGPage(0);
    printf("\x1b[>5l\x1b[>1l");/*����on;PF�\��on*/
    exit(n);
}

void DosError(int n)
    // �f�o�b�O�p�̕\��&�I��
{
    if (n >= 0)
    	printf("%s : Dos Function Error #%d\n",gPrgName,n);
    else
    	n = 1;
    Exit(n);
}

void DbgPutWord(int n)
    // �f�o�b�O�p�̕\��
{
    printf(" %d(%x) ",n,n);
}

void PriMsg(byte *s)
    // ���b�Z�[�W�\��
{
    printf("%s : %s",gPrgName,s);
}

void PriExit(byte *s)
    // ���b�Z�[�W��\�����ďI��(1)
{
    printf("%s : %s",gPrgName,s);
    Exit(1);
}

void DmyProc2(void)
    // �Ӗ����Ȃ���������Ȃ����A�󃋁[�v�p�̃_�~�[�֐�
{
    ;
}

void DmyProc(void)
    // �Ӗ����Ȃ���������Ȃ����A�󃋁[�v�p�̃_�~�[�֐�
{
    DmyProc2();
}

#if 1
#define VsyncWait(n)	delay(n)
#else
void VsyncWait(int n)
    // -w , @w �ł̎��ԑ҂�
{
    long l;
    Key_BufClr();
    while (n) {
    	l = Key_Get();
    	if (l & 0xffff0000L) {
    	    l &= 0xFFFF;
    	    if (l == 0x001b||(l & 0xff) == 0x03)
    	    	Exit(1);
    	    else if (l == 0x1c0d || l == 0x3420)
    	    	return;
    	}
    	WaitVsync();
    	n--;
    }
}
#endif

void VsWaitInit(void)
    // -ws �̎��ԑ҂��悤���[�`���̏�����
{
    gVsWaitCnt = (int)CntVsync();
    gVsWaitCnt2 = gVsWaitCnt1 = 0;
  #if 0
    if (gVsWaitCnt < 20)
    	PriExit("PrgErr:CntVsync����������\n");
    /*else if (gVsWaitCnt > 120)
    	gVsWaitCnt1 = 1;*/
  #endif
}

void VsWait(int n)
    // -wsN,M �� N �̒l�̎��ԑ҂�
{
    volatile long l;
    l = (long)gVsWaitCnt * (long)n;
    while (l) {
    	--l;
    	DmyProc();
    }
}

void VsWait2(void)
    // -wsN,M �� M �̒l�̎��ԑ҂�
{
    volatile long l;
    l = gVsWaitCnt * gVsWaitCnt2;
    l >>= 2;
    while (l) {
    	--l;
    	DmyProc();
    }
}

/*---------------------------------------------------------------------------*/

void GInit(void)
    // VRAM�̏�Ԃ̏�����
{
    union   REGS    regs;

    ActGPage(0);
    regs.h.ah = 0x42;
    if (gL200flg==1
    || (gL200flg==0&& PiLd_dotX && PiLd_dotY && (PiLd_dotX/PiLd_dotY >= 2))) {
    	regs.h.ch = 0x80;
    	Gv_btmLine = 200;
    	if (gSmlFlg)
    	    regs.h.ch = 0xC0;
    } else {
    	regs.h.ch = 0xC0;
    	Gv_btmLine = 400;
    }
    int86( 0x18, &regs, &regs);

    regs.h.ah = 0x40;
    int86( 0x18, &regs, &regs);
    outp( 0x6a, 1); //analog mode
    Gdc_SclInit();
}

void ToneDown(int n)
    // -cp �ł̉�ʂ��g�[���_�E���i�t�B�[�h�A�E�g�j
{
    int i;

    if (gPalTone > n) {
    	for (i = gPalTone; i >= n; i--) {
    	    RPal_ChgTone(i);
    	    if (i & 0x01)
    	    	WaitVsync();
    	}
    } else {
    	RPal_ChgTone(n);
    }
}

void ToneUp(int n)
    // -a5 �ł̉�ʂ��g�[���A�b�v�i�t�B�[�h�C���j
{
    int i;

    for (i = 0; i <= n; i++) {
    	RPal_ChgTone(i);
    	if (i & 0x01)
    	    WaitVsync();
    }
}

void GCls(void)
    // �O���t�B�b�N��ʏ���
{
    Gv_Cls();
    if (gUraUseMode) {
    	ActGPage(1);
    	Gv_Cls();
    	ActGPage(0);
    }
  #ifdef HF
    if (gHfMode) {
    	/*gHfMode =*/ Hf_Init();
    	Hf_Cls();
    	Hf_Term();
    }
  #endif
}

#if 0
void PriColTbl(void)
{
    if (gChk) {
    	byte *p;
    	int  l;
    	p = PiLd_colTbl;
    	for (l = 0;l < 256;l++) {
    	    if ((l % 16) == 0) {
    	    	printf("\n: ");
    	    }
    	    printf("%02x ",*p++);
    	}
    	printf("\n");
    }
}
#endif

#ifdef DEB_DMP
_S void MemDmp(void far *adr, dword len)
    // �f�o�b�O�p�B�w��A�h���X�̃_���v���s�Ȃ�
{
    byte huge *dp;
    dword l;

    dp = adr;
    dp++;dp--;
    for (l = 0;l < len;l++) {
    	if ((l % 20) == 0) {
    	    if ((l % 320) == 0) {
    	    	printf("\n");
    	    	if (gKeyWaitFlg) {
    	    	    if (getch() == 0x1b)
    	    	    	return;
    	    	}
    	    }
    	    printf("\n%08x ",l);
    	}
    	printf("%02x ",*dp++);
    }
    printf("\n");
}
#endif

void PriHdr(byte *name,int cf)
    // �w�b�_���̕\��
{
    typedef int (*pri_t)(char *, ...);
    pri_t pri[2] = {(pri_t)printf, (pri_t)cprintf};
    pri_t prf;
    static byte *crlf[2] = {"\n","\r\n"};
    int i,n;

    prf = pri[cf];
    prf("%s\r\n",name);
    prf("  �h�b�g��: %d:%d",PiLd_dotX,PiLd_dotY);
    prf("  �T�C�Y %d*%d",PiLd_sizX,PiLd_sizY0);
    prf("  �ȗ������W(%d,%d)-(%d,%d)",
    	PiLd_topX,PiLd_topY,PiLd_topX+PiLd_sizX-1,PiLd_topY+PiLd_sizY0-1);
    prf(crlf[cf]);
    prf("  �Z�[�o��:%s",PiLd_macName);
    prf("  %d�F(%d�v���[��)", ((PiLd_plnCnt == 8) ? 256 : 16), PiLd_plnCnt);
    prf("  �p���b�g���[�h:%02x",PiLd_palMode);
    prf(crlf[cf]);
    n = 16;
    if (cf == 0 && PiLd_plnCnt == 8)
    	n = 256;
    for (i=0; i < n; i++) {
    	prf("\t%02x: %02x %02x %02x",
    	    i,PiLd_pal256[i*3],PiLd_pal256[i*3+1],PiLd_pal256[i*3+2]);
    	if (i % (4) == 3)
    	    prf(crlf[cf]);
    }
}

void HdrErrMsg(int f)
{
    byte *msg[] = {
    	"�o���t�@�C���łȂ�",
    	"256�F�摜�� -lbu ���w�肵�Ȃ��ƕ\���ł��Ȃ�",
    	"���̉摜�T�C�Y�ɂ͑Ή����Ă��Ȃ�",
    	"�摜���傫������(������������Ȃ�)",
    };
    PriMsg(msg[f-1]);
}

void Disp(int sx,int sy,int ovrf)
    // �摜��\��.
    // ���ۂ̕\���� Gv_Dsp???�̊֐����s�Ȃ�
{
    int f;

    sx = -sx;
    sy = -sy;
    Gdc_SclInit();
    f = 0;
    if (gUraUseMode || gUraWrtMode) {
    	if (Gv_btmLine == 200)
    	    f = 1;
    	Gv_btmLine = 400;
    }
    if (ovrf>0) {
    	Gv_DspOvrLd(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy,gOvrLdCol);
    } else if (gMieMode) {
    	Gv_DspP(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy,gMieMode);
    } else if (PiLd_sizX == 640 && sx == 0 && sy == 0) {
    	Gv_Dsp640(PiLd_wkBuf2,PiLd_sizY);
    } else {
    	Gv_Dsp(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy);
    }
    if (gUraUseMode && sx == 0 && sy == 0) {
    	ActGPage(1);
    	if (f == 0)
    	    DspGPage(1);
    	if(gUraUseMode == 1)
    	    sy -= 400;
    	else if (gUraUseMode == 2)
    	    sx -= 640;
    	if (ovrf>0) {
    	    Gv_DspOvrLd(PiLd_wkBuf2,
    	    	PiLd_sizX,PiLd_sizY,sx,sy,gOvrLdCol);
    	} else if (gMieMode) {
    	    Gv_DspP(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy,gMieMode);
    	} else {
    	    Gv_Dsp(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY,sx,sy);
    	}
    	ActGPage(0);
    	if (f == 0)
    	    DspGPage(0);
    }
    if (f)
    	Gv_btmLine = 200;
}

void DispL(int sx,int sy)
    // �摜�̕\���B�X�N���[���ŕs�A���ɂȂ���VRAM��Ԃ�����������
    // �w�肳�ꂽ�ʒu�̉摜��\��
{
    int f,sizx;

    if (PiLd_sizX < 640 && PiLd_sizY < Gv_btmLine) {
    	Disp(sx<<3,sy,0);
    	return;
    }
    Gdc_SclInit();
    if (sx < 0 || sy < 0)
    	sx = sy = 0;
    f = 0;
    if (gUraUseMode || gUraWrtMode) {
    	if (Gv_btmLine == 200)
    	    f = 1;
    	Gv_btmLine = 400;
    }
    sizx=PiLd_sizX>>3;
    Gv_WkCpy(PiLd_wkBuf2,sizx, PiLd_sizY,sx, sy,
    	80,400,0,0);
    if (gUraUseMode) {
    	ActGPage(1);
    	if (f == 0)
    	    DspGPage(1);
    	if(gUraUseMode == 1)
    	    sy += 400;
    	else if (gUraUseMode == 2)
    	    sx += 80;
    	Gv_WkCpy(PiLd_wkBuf2, sizx, PiLd_sizY,sx%sizx, sy%PiLd_sizY,
    	    80,400,0,0);
    	ActGPage(0);
    	if (f == 0)
    	    DspGPage(0);
    }
    if (f)
    	Gv_btmLine = 200;
}


/*---------------------------------------------------------------------------*/
void KeyUsage(int cf)
{
    byte *keyUsageMsg[] = {
    	"ESC      DoPI ���I��",
    	"HELP     ���̐���(�����ɂ� ������xHELP�������� TAB ������)",
    	"TAB =    �w�b�_������ �̕\��",
    	"CR SPC   ���̉摜",
    	"HOME/CLR �n�_��(0,0)�ɂ��čĕ\��(-lb�̉e������)",
    	"�������� ��ʂɕ\���ł��Ȃ�����������\��(�X�N���[��)",
    	"         �X�N���[���͉�ʔ͈͂��z����摜�Ŏn�_��(0,0)�̂Ƃ��̂݉�",
    	"0�`6     �X�N���[���̈ړ���",
    	"         0:1�ޯ� 1:2�ޯ� 2:4�ޯ� 3:8�ޯ� 4:16�ޯ� 5:32�ޯ�  6:64�ޯ�",
    	"INS      �p���b�g�̃g�[����10���A�b�v(0-200%)",
    	"DEL      �p���b�g�̃g�[����10���_�E��(0-200%)",
    	"/        ��ʒ��x���͈͂��z����摜�Ȃ�A�X�N���[���Ń��[�v����",
    	"-        ���̂܂܂̍��W�ōĕ\��(-lb�w�莞�̂�. ����ʂ�`������)",
    	"[        �㉺���]�i���C����������̃f�[�^�ɑ΂��čs���j",
    	"]        ���E���]�i�@�@�@�@�@�@�@�V                  �j",
    	"\\        �X�N���[�����@�̐ؑ�(GDC���p/��ĳ���]��)",
    	",        �k���\��(1/4). �����L�[�������Ζ߂�",
    	"^        ��ʂ�400line�̎��A����ʂ�����. �����L�[�������Ζ߂�",
    	"@        ���݂�VRAM������o�b�t�@�Ɏ�荞��",
    	NULL
    };
    typedef int (*pri_t)(char *, ...);
    pri_t pri[2] = {(pri_t)printf, (pri_t)cprintf};
    pri_t prf;
    byte *crlf[2] = {"\n","\r\n"};
    byte **p;

    prf = pri[cf];
    prf("%s �ŃL�[���͎��Ɏg����L�[",gNameVer);
    prf(crlf[cf]);
    for (p = keyUsageMsg; *p != NULL; p++) {
    	prf(*p);
    	prf(crlf[cf]);
    }
}

int KeyMode(byte *name,int sx, int sy)
    // �L�[����
{
    int  i,f,g,xx,yy,mode,xsiz,tone;
    word c;
    //int  sav_cbrk;
    static int ddy = 16;
    static byte *keyGmsg[] = {
    	"640*400","640*800","1280*400","400*640","800*640","400*1280"
    };

    //sav_cbrk = getcbrk();
    //setcbrk(0);
    gVRamGetNo = 0;
    tone = gPalTone;
    xsiz = PiLd_sizX>>3;
    sx >>= 3;
    f = (gFx && (xsiz - sx >= 80));
    g = (gFy && (PiLd_sizY - sy >= Gv_btmLine));
    xx = (xsiz >= 80) ? 80 : xsiz;
    yy = (PiLd_sizY >= Gv_btmLine) ? Gv_btmLine : PiLd_sizY;
    mode = 0;
    if ((sx && g && !f) || (sy && f && !g))
    	f = g = 0;
    for (;;) {
    	if (mode) {
    	    gotoxy(1,25);
    	    cprintf("(%3d,%3d) Skip:%d  tone:%3d%%   %s    ",
    	    	sx*8,sy,ddy,tone,(gSclLoopFlg) ? "loop":"    ");
    	}

    	Gv_ofsFlg = 0;
    	if (gGdcSclFlg && Gv_btmLine != 200) {	//GDC�X�N���[��
    	    Key_BufClr();
    	    c = pc98key(0x04,0x07);
    	    if (c & 0x04) { //UP
    	    	if (g == 0)
    	    	    continue;
    	    	i = ddy;
    	    	do {
    	    	    if (--sy < 0) {
    	    	    	if (gSclLoopFlg == 0) {
    	    	    	    sy = 0;
    	    	    	    goto JJ0;
    	    	    	}
    	    	    	sy += PiLd_sizY;
    	    	    }
    	    	    Gdc_Scl(0,-1);
    	    	    Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx, sy,xx, 1,
    	    	    	Gdc_ofsX, Gdc_ofsY);
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    } else if (c & 0x20) { //DOWN
    	    	if (g == 0)
    	    	    continue;
    	    	i = ddy;
    	    	do {
    	    	    if (sy + Gv_btmLine >= PiLd_sizY && gSclLoopFlg == 0) {
    	    	    	sy = PiLd_sizY - Gv_btmLine;
    	    	    	goto JJ0;
    	    	    }
    	    	    Gdc_Scl(0,1);
    	    	    Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx,
    	    	    	(sy+Gv_btmLine)%PiLd_sizY,  xx, 1,
    	    	    	Gdc_ofsX, (Gdc_ofsY + Gv_btmLine - 1)%400);
    	    	    if (++sy >= PiLd_sizY)
    	    	    	sy -= PiLd_sizY;
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    } else if (c & 0x10) { //Right
    	    	if (f == 0)
    	    	    goto JJ0;
    	    	i = ddy >> 3;
    	    	if (i < 2||((sx & 0x01) == 0))
    	    	    i = 2;
    	    	do {
    	    	    if (sx + 80 >= xsiz && gSclLoopFlg == 0) {
    	    	    	sx = xsiz - 80;
    	    	    	goto JJ0;
    	    	    }
    	    	    Gdc_Scl(1,0);
    	    	    Gv_ofsFlg = (Gdc_ofsX != 0);
    	    	    Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,
    	    	    	(sx + 80)%xsiz, sy, 1, yy,
    	    	    	(Gdc_ofsX + 80 - 1)%80, Gdc_ofsY);
    	    	    Gv_ofsFlg = 0;
    	    	    sx += 1;
    	    	    if (sx >= xsiz)
    	    	    	sx -= xsiz;
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    } else if (c & 0x08) { //Left
    	    	if (f == 0)
    	    	    continue;
    	    	i = ddy >> 3;
    	    	if (i < 2||((sx & 0x01) == 0))
    	    	    i = 2;
    	    	do {
    	    	    sx -= 1;
    	    	    if (sx < 0) {
    	    	    	if (gSclLoopFlg == 0) {
    	    	    	    sx = 0;
    	    	    	    goto JJ0;
    	    	    	}
    	    	    	sx += xsiz;
    	    	    }
    	    	    Gdc_Scl(-1,0);
    	    	    Gv_WkCpy(PiLd_wkBuf2,xsiz,PiLd_sizY,
    	    	    	sx,sy,1,yy,Gdc_ofsX,Gdc_ofsY);
    	    	    VsWait2();
    	    	} while (--i);
    	    	continue;
    	    JJ0:
    	    	continue;
    	    }
    	    c = Key_GetWait();
    	} else {    //�\�t�g�E�F�A�]��
    	    int dd,ddx;

    	    Key_BufClr();
    	    c = Key_GetWait();
    	    switch (c >> 8) {
    	    case 0x3A://UP
    	    	if (g == 0)
    	    	    break;
    	    	if (gSclLoopFlg) {
    	    	    dd = ddy;
    	    	} else if (sy > 0) {
    	    	    dd = (sy > ddy) ? ddy : sy;
    	    	} else
    	    	    break;
    	    	sy -= dd;
    	    	if (sy < 0)
    	    	    sy += PiLd_sizY;
    	    	Gv_GDwn(dd);
    	    	Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx, sy, xx, dd,
    	    	    Gdc_ofsX, Gdc_ofsY);
    	    	break;
    	    case 0x3D://Dwn
    	    	if (g == 0)
    	    	    break;
    	    	if (gSclLoopFlg) {
    	    	    dd = ddy;
    	    	} else {
    	    	    dd = PiLd_sizY - (sy + Gv_btmLine);
    	    	    if (dd < 0)
    	    	    	break;
    	    	    if (dd > ddy)
    	    	    	dd = ddy;
    	    	}
    	    	Gv_GUp(dd);
    	    	Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,sx,
    	    	    (sy+Gv_btmLine)%PiLd_sizY, xx, dd,
    	    	    Gdc_ofsX, (Gdc_ofsY + Gv_btmLine - dd)%400);
    	    	sy += dd;
    	    	if (sy >= PiLd_sizY)
    	    	    sy -= PiLd_sizY;
    	    	break;
    	    case 0x3C://Rgt
    	    	if (f == 0)
    	    	    break;
    	    	ddx = (ddy < 8) ? 1 : (ddy >> 3);
    	    	if (gSclLoopFlg) {
    	    	    dd = ddx;
    	    	} else if (sx + 80 < xsiz) {
    	    	    dd = xsiz - (sx + 80);
    	    	    if (dd > ddx)
    	    	    	dd = ddx;
    	    	} else
    	    	    break;
    	    	Gv_GLft(dd);
    	    	Gv_ofsFlg = (Gdc_ofsX != 0);
    	    	Gv_WkCpy(PiLd_wkBuf2, xsiz, PiLd_sizY,
    	    	    (sx + 80)%xsiz, sy, dd, yy,
    	    	    (Gdc_ofsX + 80 - dd)%80, Gdc_ofsY);
    	    	sx += dd;
    	    	if (sx >= xsiz)
    	    	    sx -= xsiz;
    	    	break;
    	    case 0x3B://Lft
    	    	if (f == 0)
    	    	    break;
    	    	ddx = (ddy < 8) ? 1 : (ddy >> 3);
    	    	if (gSclLoopFlg) {
    	    	    dd = ddx;
    	    	} else if (sx > 0) {
    	    	    dd = sx;
    	    	    if (dd > ddx)
    	    	    	dd = ddx;
    	    	} else
    	    	    break;
    	    	sx -= dd;
    	    	if (sx < 0)
    	    	    sx += xsiz;
    	    	Gv_GRgt(dd);
    	    	Gv_WkCpy(PiLd_wkBuf2,xsiz,PiLd_sizY,
    	    	    sx,sy,dd,yy,Gdc_ofsX,Gdc_ofsY);
    	    	break;
    	    default:
    	    	goto JJ2;
    	    }
    	    continue;
    	}

    JJ2:
    	switch (c >> 8) {
    	case 0x00://ESC 00:1B
    	    //if (Gdc_ofsY||Gdc_ofsX) {
    	    //	DispL(sx,sy);
    	    //}
    	    c = -1;
    	    goto LOOP_OUT;
    	case 0x1C://CR
    	case 0x34://SPC
    	    //if (Gdc_ofsY||Gdc_ofsX) {
    	    //	DispL(sx,sy);
    	    //}
    	    c = 0;
    	    goto LOOP_OUT;
    	case 0x3E://CLR 3E:00 (1A)
    	    sx=sy=0;
    	    GCls();
    	    Disp(0,0,0);
    	    f = (gFx && (xsiz - sx >= 80));
    	    g = (gFy && (PiLd_sizY - sy >= Gv_btmLine));
    	    break;
    	case 0x3F://HELP
    	    if (mode != 3) {
    	    	mode = 3;
    	    	RPal_ChgTone(50);
    	    	//RPal_Set(50,PiLd_pal);
    	    	TCls();
    	    	KeyUsage(1);
    	    	break;
    	    }
    	case 0x0F://TAB
    	TAB:
    	    ++mode;
    	    if (mode >= 3)
    	    	mode = 0;
    	    switch (mode) {
    	    case 0:
    	    	RPal_Set(tone,PiLd_pal);
    	    	TCls();
    	    	break;
    	    case 1:
    	    	RPal_ChgTone(50);
    	    	//RPal_Set(50,PiLd_pal);
    	    	TCls();
    	    	PriHdr(name,1);
    	    	if (*PiLd_commentBuf) {
    	    	    cprintf("\r\n�R�����g����");
    	    	} else {
    	    	    cprintf("\r\n�R�����g�Ȃ�");
    	    	    ++mode;
    	    	}
    	    	break;
    	    case 2:
    	    	if (*PiLd_commentBuf) {
    	    	    RPal_ChgTone(50);
    	    	    //RPal_Set(50,PiLd_pal);
    	    	    TCls();
    	    	    printf("%Fs",PiLd_commentBuf);
    	    	}
    	    }
    	    break;
    	case 0x38://INS
    	    tone += 10;
    	    if (tone > 200)
    	    	tone = 200;
    	    if (mode == 0)
    	    	RPal_ChgTone(tone);
    	    break;
    	case 0x39://DEL
    	    tone -= 10;
    	    if (tone < 0)
    	    	tone = 0;
    	    if (mode == 0)
    	    	RPal_ChgTone(tone);
    	    break;
    	default:
    	    goto JJ4;
    	}
    	continue;

    JJ4:
    	c = c & 0xff;
    	c = toupper(c);
    	switch(c) {
    	case '0':ddy = 1;   break;
    	case '1':ddy = 2;   break;
    	case '2':ddy = 4;   break;
    	case '3':ddy = 8;   break;
    	case '4':ddy = 16;  break;
    	case '5':ddy = 32;  break;
    	case '6':ddy = 64;  break;
    	case '=':
    	    goto TAB;
    	case '-':
    	    DispL(sx,sy);
    	    break;
    	case '/':
    	    if (f == 0 && g == 0)
    	    	break;
    	    ++gSclLoopFlg;
    	    gSclLoopFlg &= 0x01;
    	    if (gSclLoopFlg == 0
    	    	&& (sx + 80 >= xsiz ||sy+Gv_btmLine > PiLd_sizY)) {
    	    	sx=sy=0;
    	    	Disp(0,0,0);
    	    }
    	    break;
    	case '\\':
    	    if (f == 0 && g == 0)
    	    	break;
    	    ++gGdcSclFlg;
    	    gGdcSclFlg &= 0x01;
    	    //sx = sy = 0;
    	    if (sx & 0x01) {
    	    	if (sx > 0)
    	    	    --sx;
    	    	else
    	    	    ++sx;
    	    }
    	    DispL(sx,sy);
    	    break;
    	case '['://�Ô��]
    	    Gv_RevY(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY);
    	    sx = sy = 0;
    	    DispL(sx,sy);
    	    break;
    	case ']'://ֺ���]
    	    Gv_RevX(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY);
    	    sx = sy = 0;
    	    DispL(sx,sy);
    	    break;
    	/*case '':
    	    Gv_RePos(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,sx,sy);
    	    sx = sy = 0;
    	    DispL(sx,sy);
    	    break;*/
    	case ','://�k���\��
    	    gSmlFlg = 1;
    	    GInit();
    	    GCls();
    	    Gv_DspSml(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY);
    	    Key_GetWait();
    	    gSmlFlg = 0;
    	    GInit();
    	    DispL(sx,sy);
    	    break;
    	case '^'://��vram��`��
    	    if (Gv_btmLine != 200) {
    	    	ActGPage(1);
    	    	DspGPage(1);
    	    	Key_GetWait();
    	    	ActGPage(0);
    	    	DspGPage(0);
    	    }
    	    break;
    	case '@':
    	    RPal_ChgTone(50);
    	    TCls();
    	    cprintf("��荞�ރT�C�Y��ԍ�1�`6�Ŏw��."\
    	    	    " �L�����Z���͑��̃L�[. (4�`6��90�x�E�ɉ�])\r\n");
    	    cprintf("    1:640*400  2:640*800  3:1280*400  4:400*640"\
    	    	    "  5:800*640  6:400*1280\r\n");
    	    c = 0xff & Key_GetWait();
    	    TCls();
    	    if (c >= '1' && c <= '6') {
    	    	DispL(sx,sy);
    	    	cprintf("%s",keyGmsg[c-'1']);
    	    	gVRamGetNo = c - '0';
    	    	c = 0;
    	    	goto LOOP_OUT;
    	    } else {
    	    	RPal_ChgTone(tone);
    	    }
    	    break;
    	}/* end of switch */
    } /* InKey Loop */

 LOOP_OUT:
  #if 1
    if ((Gdc_ofsY||Gdc_ofsX)) {
    	DispL(sx,sy);
    }
  #endif
    //setcbrk(sav_cbrk);
    return c;
}

/*---------------------------------------------------------------------------*/

void DispInit(void)
    // �摜���Ƃ̉�ʂ̏�����. ���W�A�͈͂̐ݒ�.
{
    GInit();
    RPal_Set(gPalTone,PiLd_pal);
    gUraUseMode = 0;
    if (gUraWrtMode && gSmlFlg == 0) {
    	if (gUraWrtMode > 1 && PiLd_sizX > 640)
    	    gUraUseMode = 2;
    	else if (PiLd_sizY > 400)
    	    gUraUseMode = 1;
    }
  #ifdef MP
    if (g256mode == 386 && PiLd_plnCnt == 8)
    	gUraUseMode = 3;
  #endif
    switch (gBxByMode) {
    case 1:
    	gSx = -gBx;
    	gSy = -gBy;
    	break;
    case 2:
    	gSx = -(PiLd_topX+gBx);
    	gSy = -(PiLd_topY+gBy);
    default:
    	gSx = -PiLd_topX;
    	gSy = -PiLd_topY;
    }
    gFx = (PiLd_sizX > 640) ? 2 : (PiLd_sizX == 640) ? 1 : 0;
    gFy = (PiLd_sizY > Gv_btmLine) ? 2 : (PiLd_sizY == Gv_btmLine) ? 1 : 0;
    if (PiLd_sizX - gSx < 0)
    	gSx = PiLd_sizX - 640;	//gSx = (gFx) ? (PiLd_sizX - 640):0;
    if (PiLd_sizY - gSy < 0)
    	gSy = PiLd_sizY-Gv_btmLine;//gSy = (gFy) ? (PiLd_sizY - Gv_btmLine):0;
}

void MemErr(void)
{
    printf("������������܂���\n");
    Exit(1);
}

void VramGetDisp(int n)
    // -kq[N] �ł̉摜�捞�݁��\��
{
    static word dmy;
    static int	xx[7] = {0,640,640,1280,400,800, 400};
    static int	yy[7] = {0,400,800, 400,640,640,1280};
    static int	ura[7]= {0,  0,  1,   2,  0,  1,   2};

  J0:
    gUraWrtMode = ura[n];
    PiLd_wkBuf2 = PiLd_wkBuf + xx[n];
    PiLd_sizX = xx[n];
    PiLd_sizY = yy[n];
    if ( (dword)PiLd_sizX*PiLd_sizY/2 +(dword)PiLd_sizX*2 > PiLd_wkBufMaxS*16L)
    	MemErr();
    PiLd_topX = PiLd_topY = 0;
    PiLd_plnCnt = 4;
    PiLd_palMode = 0;
    PiLd_dotX = PiLd_dotY = 0;
    strcpy(PiLd_macName,"DoPI");
    if (RPal_Get(&dmy,PiLd_pal)) {
    	PriExit("�풓�p���b�g���g�ݍ��܂�Ă��܂���");
    }
    if (n < 4)
    	Gv_GetVRam(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,n);
    else
    	Gv_GetVRamQ(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY>>3,n - 4);
    DispInit();
    Disp(gSx,gSy,0);
    TCls();
    gVRamGetNo = 0;
    KeyMode("��ʎ�荞��",gSx,gSy);
    gUraLoadFlg = gOvrLdFlg = 0;
    if (gClsFlg == 1)
    	gClsFlg = 0;
    if (gKeyWaitFlg == 1)
    	gKeyWaitFlg = 0;
    if (gVRamGetNo) {
    	n = gVRamGetNo;
    	goto J0;
    }
}

int KeyWaitSml(void)
{
    if (gKeyWaitFlg) {
    	int c;
    	do {
    	    c = getch();
    	    if (c == 0x1b)
    	    	return 1;
    	} while (c != ' ' && c != 0x0d);
    } else {
    	if (gVsWaitCnt0)
    	    VsyncWait(gVsWaitCnt0);
    }
    return 0;
}

#if defined(HF)||defined(MP)
int KeyWaitHF(void)
{
    static ddy = 64;

    //cprintf("*");
    if (gKeyWaitFlg) {
    	word c;
    	for (; ;) {
    	    Key_BufClr();
    	    c = Key_GetWait();
    	    //cprintf("%04x ",c);
    	    switch(c >> 8) {
    	    case 0x3a://UP
    	    	gSy -= ddy;
    	    	if (gSy < 0)
    	    	    gSy = 0;
    	    	return -1;
    	    case 0x3d://DOWN
    	    	if (PiLd_sizY > 400) {
    	    	    gSy += ddy;
    	    	    if (gSy > PiLd_sizY - 400)
    	    	    	gSy = PiLd_sizY - 400;
    	    	}
    	    	return -1;
    	    case 0x3c://RIGHT
    	    	if (PiLd_sizX > 640) {
    	    	    gSx += ddy;
    	    	    if (gSx > PiLd_sizX - 640)
    	    	    	gSx = PiLd_sizX - 640;
    	    	}
    	    	return -1;
    	    case 0x3b://LEFT
    	    	gSx -= ddy;
    	    	if (gSx < 0)
    	    	    gSx = 0;
    	    	return -1;
    	    case 0x00://ESC 00:1B
    	    	return 1;
    	    case 0x3E://CLR 3E:00 (1A)
    	    	GCls();
    	    	return -2;
    	    case 0x1C://CR
    	    case 0x34://SPC
    	    	return 0;
    	    }
    	    switch(c&0xff) {
    	    case '0':ddy = 1;	break;
    	    case '1':ddy = 2;	break;
    	    case '2':ddy = 4;	break;
    	    case '3':ddy = 8;	break;
    	    case '4':ddy = 16;	break;
    	    case '5':ddy = 32;	break;
    	    case '6':ddy = 64;	break;
    	    case '7':ddy = 128; break;
    	    case '8':ddy = 256; break;
    	    }
    	}
    } else {
    	if (gVsWaitCnt0)
    	    VsyncWait(gVsWaitCnt0);
    }
    return 0;
}
#endif

int  PiDisp(byte *loadName)
    // ��ʂ̕\��
{
    int xx;

    xx = PiLd_sizX>>3;
    xx = (xx >= 80) ? 80 : xx;
  #ifdef HF
    if (gEx256Flg == 1) {
    	Hf_Term();
    }
  #endif
  #ifdef MP
    if (gEx256Flg == 2) {
    	Mp_Term();
    }
  #endif
  #if defined(HF)||defined(MP)
    gEx256Flg = 0;
  #endif
    if (gUraLoadFlg == 0) { // �\��ʂɕ\������
    	if (gToneDownFlg)
    	    ToneDown(0);
    	if (gClsFlg)
    	    GCls();
    	DispInit();
    	if (gSmlFlg) {//�k���\�����w�肳��Ă���Ƃ�
    	    if (gToneUpFlg)
    	    	RPal_ChgTone(0);
    	    Gv_DspSml(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY);
    	    TCls();
    	    if (gToneUpFlg)
    	    	ToneUp(gPalTone);
    	    if (KeyWaitSml())
    	    	return 1;
  #ifdef    HF
    	} else if (PiLd_plnCnt == 8 && gHfMode) {
    	    int c,sx,sy;
    	    TCls();
    	    Gv_Cls();
    	    if (gUraUseMode) {
    	    	ActGPage(1);
    	    	Gv_Cls();
    	    	ActGPage(0);
    	    }
    	    /*gHfMode =*/ Hf_Init();
    	    gEx256Flg = 1;
    	    sx = sy = 0x7fff;
    	    c = -2;
    	    do {
    	    	if (c != -1 ||(sx != gSx || sy != gSy)) {
    	    	    Hf_Dsp(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, gSx, gSy);
    	    	    sx = gSx;sy = gSy;
    	    	}
    	    } while ((c = KeyWaitHF()) < 0);
    	    return c;
  #endif
  #ifdef    MP
    	} else if (PiLd_plnCnt == 8 && g256mode == 386) {
    	    int     c,bx,by,yy,sx,sy;
    	    TCls();
    	    Mp_Init();
    	    Mp_SetPal(PiLd_pal256);
    	    if (gSx >= 0) {
    	    	bx = 0;
    	    } else {
    	    	bx = -(gSx>>3),gSx = 0;
    	    }
    	    if (gSy >= 0) {
    	    	by = 0;
    	    } else {
    	    	by = -gSy,gSy = 0;
    	    }
    	    gEx256Flg = 2;
    	    sx = sy = 0x7fff;
    	    c = -2;
    	    GCls();
    	    do {
    	    	if (c != -1 || (sx != gSx || sy != gSy)) {
    	    	    yy = (PiLd_sizY >= 400) ? 400 : PiLd_sizY;
    	    	    Mp_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,
    	    	    	gSx>>3, gSy, xx,/*400*/yy, bx%80,by%Gv_btmLine);
    	    	    sx = gSx;sy = gSy;
    	    	    bx = by = 0;
    	    	}
    	    } while ((c = KeyWaitHF()) < 0);
    	    return c;
  #endif
    	} else {
    	    if (gMie2Mode && gSx == 0 && gSy == 0 && gUraWrtMode == 0
    	    	&& gOvrLdFlg <= 0 && gGdcSclFlg && Gv_btmLine != 200
    	    	&& PiLd_sizY >= Gv_btmLine) {//���h���胂�[�h�Ƃ��̃X�N���[��
    	    	int i;
    	    	if (gMie2Mode == 1) {
    	    	    for (i = 0; i < Gv_btmLine; i++) {
    	    	    	Gdc_Scl(0,1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }
    	    	} else {
    	    	    for (i = PiLd_sizY; --i >= 0/*Gv_btmLine*/;) {
    	    	    	Gdc_Scl(0,-1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i % Gv_btmLine);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }
    	    	    /*for (i = PiLd_sizY-Gv_btmLine; --i >= 0;) {
    	    	    	Gdc_Scl(0,-1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }*/
    	    	}
    	    } else {//���ʂɕ\��
    	    	if (gToneUpFlg)
    	    	    RPal_ChgTone(0);
    	    	Disp(gSx,gSy,gOvrLdFlg);
    	    	if (gToneUpFlg)
    	    	    ToneUp(gPalTone);
    	    }
    	    if (/*gKeyWaitFlg == 0 &&*/ gSx == 0 && gSy == 0 && gUraWrtMode==0
    	    	&& gOvrLdFlg <= 0 && PiLd_sizY > Gv_btmLine && gGdcSclFlg
    	    	&& Gv_btmLine != 200 && gMie2Mode != 2) {
    	    	    //400���C���ȏ�̉摜�̂Ƃ��̃X�N���[��
    	    	int i;
    	    	for (i = Gv_btmLine; i < PiLd_sizY; i++) {
    	    	    Gdc_Scl(0,1);
    	    	    Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	xx, 1, 0, (i - Gv_btmLine)%Gv_btmLine);
    	    	    	//xx, 1, Gdc_ofsX, Gdc_ofsY);
    	    	    VsWait(gVsWaitCnt1);
    	    	}
    	    	if (gSclBakFlg) {
    	    	    for (i = 0;i < 6;i++)
    	    	    	WaitVsync();
    	    	    for (i = PiLd_sizY-Gv_btmLine; --i >= 0;) {
    	    	    	Gdc_Scl(0,-1);
    	    	    	Gv_WkCpy(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY,0,i,
    	    	    	    xx, 1, 0, i % Gv_btmLine);
    	    	    	    //xx, 1, Gdc_ofsX, Gdc_ofsY);
    	    	    	VsWait(gVsWaitCnt1);
    	    	    }
    	    	} else {
    	    	    gSy = Gv_btmLine;
    	    	}
    	    }
    	    TCls();
    	    if (gKeyWaitFlg) {	//�L�[���̓��[�h
    	    	if (KeyMode(loadName,gSx,gSy))
    	    	    return 1;
    	    } else {
    	    	if (gVsWaitCnt0)    //�\����̎��ԑ҂�
    	    	    VsyncWait(gVsWaitCnt0);
    	    }
    	}
    } else {	// -b filename �ł̗�VRAM�ւ̃��[�h
    	DispInit();
    	DspGPage(1);
    	ActGPage(1);
    	if (gClsFlg)
    	    Gv_Cls();
    	if (gSmlFlg)
    	    Gv_DspSml(PiLd_wkBuf2,PiLd_sizX,PiLd_sizY);
    	else
    	    Disp(gSx,gSy,gOvrLdFlg);
    	TCls();
    	DspGPage(0);
    	ActGPage(0);
    }
    gUraLoadFlg = gOvrLdFlg = 0;
    if (gClsFlg == 1)
    	gClsFlg = 0;
    if (gKeyWaitFlg == 1)
    	gKeyWaitFlg = 0;
    return 0;
}

int PiLoad(byte *name)
{
    FILE *fp;
    int f;
    int c256mode;

    fp = fopen(name,"rb");
    if (fp == NULL) {
    	printf("%s �̓I�[�v���ł��܂���ł���\n",name);
    	return 1;
    }
    if (gRdBufVRamFlg)
    	ActGPage(1);
    PiLd_RdFirst(fileno(fp));
    PiLd_filSiz = filelength(PiLd_ph);
    PiLd_ColTblInit(); /* PriColTbl();*/
    f = PiLd_ReadHdr(); // �w�b�_��ǂݍ��ݐݒ�
    c256mode = 0;
    if (PiLd_plnCnt == 8) {
    	if (gRdBufVRamFlg) {
    	    c256mode = g256mode+1;
  #ifdef HF
    	    if (gHfMode)
    	    	c256mode = -1;
  #endif
    	} else
    	    f = 2;
    }
    if (PiLd_bcol && gOvrLdFlg2 == 0) {
    	gOvrLdFlg = 1;
    	gOvrLdCol = PiLd_bcol - 1;
    }
    if (f != 1 && gInfoFlg) {	// -i �ł̃w�b�_���̕\���݂̂̂Ƃ�
    	PriHdr(name,0);
    	if (*PiLd_commentBuf)
    	    printf("%Fs\n\n",PiLd_commentBuf);
    	else
    	    printf("\n");
    	fclose(fp);
    	return 1;
    }
    if (f) {
    	HdrErrMsg(f);
    	return 1;
    }
    TCls();
    if (gTxtFlg) {
    	printf("%s",name);
    	if (*PiLd_commentBuf) {
    	    if (strncmp(PiLd_commentBuf, "User:", 5) == 0) {
    	    	printf (" : %Fs\n", &PiLd_commentBuf[5]);
    	    } else {
    	    	printf ("\n%Fs\n", PiLd_commentBuf);
    	    }
    	} else {
    	    printf("\n");
    	}
    }
    if (gChk) {
    	PriHdr(name,0);
    }
    if (PiLd_sizY != PiLd_sizY0) {
    	printf("\n���������m�ۂł��Ȃ��̂� %d �s�܂ł����\���ł��܂���\n",PiLd_sizY);
    }
  #ifdef PILD4C
    if (gPiLd4cFlg)
    	PiLd_Load4c();
    else
  #else
    // Pi�f�[�^��W�J
    if (c256mode == 0) {
    	PiLd_Load4a();
    } else {
    	PiLd_Load8();
    }
    if (gChk)
    	printf("PiLd end\n");
  #endif
    fclose(fp);
  #ifdef DEB_DMP
    if (gDmpFlg) {
    	MemDmp(PiLd_wkBuf, 1024);
    	MemDmp(PiLd_wkBuf2, 1024);
    	//MemDmp(PiLd_wkBuf, PiLd_wkSiz);
    }
  #endif
    // �W�J�����f�[�^������ɕ\�����₷���悤�ɕϊ�
    switch(c256mode) {
    case 0:
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4a(PiLd_wkBuf2,PiLd_sizX>>3,PiLd_sizY);
    	break;
    case 1:
    	Decol_Dit16(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, PiLd_pal256);
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4b(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	memcpy(PiLd_pal,g256col16pal,16*3);
    	break;
    case 2:
    case 3:
    	Decol_Mono16(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, PiLd_pal256,
    	    	    PiLd_wkBuf, PiLd_sizX * 2, g256mode-1);
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4b(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	break;
    case 4:
    	Decol_Col16(PiLd_wkBuf2, PiLd_sizX, PiLd_sizY, PiLd_pal256,
    	    	    (void far *)0xA8000000L, PiLd_sizX * 2 * 4 * 3);
    	if (gRdBufVRamFlg)
    	    ActGPage(0);
    	Gv_Cnv4b(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	memcpy(PiLd_pal,g256col16pal,16*3);
    	break;
  #ifdef    MP
    case 386+1: //PC386M&P
    	Mp_Cnv(PiLd_wkBuf2, PiLd_sizX>>3, PiLd_sizY);
    	break;
  #endif
    default:
    	;   	//HF(SF)
    }

  #ifdef DEB_DMP
    if (gDmpFlg)
    	MemDmp(PiLd_wkBuf, PiLd_wkSiz);
  #endif
    if (gChk)
    	printf("Gv_Cnv4 end\n");
    return 0;
}

/*---------------------------------------------------------------------------*/
_S void OptsRngErr(byte *s)
{
    printf("-%s �I�v�V�����Ŏw��ł��Ȃ��l���w�肳��Ă���\n",s);
    Exit(1);
}

_S void OptsErr(byte *s)
{
    printf("%s �I�v�V�����̎w�肪��������\n",s);
    Exit(1);
}


_S void Usage(void)
{
    printf("use: dopi [-opts] file(s) �^ %s By �Ă񂩁�  Pi���:��Ȃ����펁\n",gNameVer);
    pr("�E�R�}���h���C���̐擪���珇�ԂɎ��s. �I�v�V�����A�t�@�C���̏��Ԃɒ���.\n");
    pr("�E��VRAM����Ɨp�������Ƃ��Ďg�p����̂Œ���.\n");
    pr("�E�L�[���͑҂���HELP�L�[�������ƁA�g����L�[�̐�����\��.\n");
    pr("@FILE    FILE ���I�v�V�����E�t�@�C����������\n");
    //pr("-??	   ���̑��̃I�v�V����(-a,-cp,-i,-kg,-lbu,-ll,-ly,-w,-ws,-wh)�̃w���v\n");
    pr("-?k      �L�[���͑҂��̎��Ɏg����L�[�̕\��\n");
    pr("-k[0|1]  �\�����ɃL�[���͑҂�. -k1 �Ȃ�P�x����. -k0 �ŃI�t\n");
    pr("-c[0|1]  �\���ɐ悾���Ė����ʏ���. -c1 �Ȃ炻�̂Ƃ��P�x����. -c0�ŃI�t\n");
    pr("-t[1]    �t�@�C�����A�R�����g��\�����Ȃ�. -t1 �ŕ\������\n");
    pr("-v[N]    �p���b�g�̃g�[��. N:0�`200%%\n");
    pr("-lv[N]   �I�����̃p���b�g�̃g�[��. N:0�`200%%\n");
    pr("-lvf     �I�����ɉ�ʕ\���I�t\n");
    pr("-i       �w�b�_����\��(�摜�\����)\n");
    pr("-b  �@�@ ����̃t�@�C���𗠉�ʂɕ\��\n");
    pr("-lb[N]   ����摜�̂Ƃ�����ʂɕ\��. N:= 0:���Ȃ� 1:�c��D�� 2:����D��\n");
    pr("-lr[0]   �k���\��(�P�^�S). �n�_����. -lr0 ���w�肳���ƃI�t\n");
    pr("-lm[0-3] 256�F�摜�̕\�����@. 0:�ި�� 1:�蔲�ɸ� 2:�덷�g�U�ɸ� 3:�덷�g�U�װ\n");
    pr("-o[X,Y]  ��ʂ̕\���J�n�ʒu. �摜�f�[�^�̎n�_���Ƃ̍��v\n");
    pr("-oa[X,Y] ��ʂ̕\���J�n�ʒu. �摜�f�[�^�̎n�_���͖���\n");
    pr("         X:-1300�ʁ`632 ���� 8�h�b�g�P��. Y:-1300�ʁ`399\n");
    pr("         ���̒l�̂΂����A���̕����͕\�����ꂸ 0����̕�����\��\n");
    pr("         ���W�l�̌���'o'��t����ƃo�C�g�P�ʂŎw��(��:-o11o,4 �Ȃ� -o88,4)\n");
    {
    	pr("[more]");
    	getch();
    	pr("\b\b\b\b\b\b\      \b\b\b\b\b\b");
    }
    pr("-a[0-5]  ���h�\��(^_^?; -a0:�I�t 1-5:�����Ă�\n");
    pr("-cp      -c �Ɠ��������A�����O�Ƀp���b�g���g�[���E�_�E��\n");
    pr("-kg[N]   ���݂�VRAM���o�b�t�@�Ɏ�荞��ŃL�[���͑҂�\n");
    pr("         N= 1:640*400 2:640*800 3:1280*400 4:400*640 5:800*640 6:400*1280\n");
    pr("-lbu0    ��VRAM����ƃo�b�t�@�Ɏg�p���Ȃ��i256�F�摜�����[�h�ł��Ȃ��Ȃ�j\n");
    pr("-lc[C]   ����̃t�@�C���̐FC �𓧖��F����.C:0�`15.(16�F�摜�̂�)\n");
    pr("         -lc- �ȗ��������F�������I�ɋ֎~\n");
    pr("-ll[0]   �X�N���[���Ŕ͈͂��z�����Ƃ����Α��֑���. -ll0 �ŃI�t\n");
    pr("-ly<0-2> �X�N���[�����@. 0:�\�t�g�E�F�A�]�� 1:�f�c�b���p(Back) 2:�f�c�b���p\n");
    pr("         -ly1 �́A400���C�����z����摜�͉��܂ŕ\����A��ɖ߂�\n");
    pr("-w[N]    �L�[���͑҂��łȂ��Ƃ��A��ʕ\����AN�����҂�\n");
    pr("-ws[N,M] �X�N���[���̃E�F�C�g. N:�L�[�҂��łȂ��Ƃ�  M:�L�[�҂��̂Ƃ�\n");
    pr("         -w,-ws �ł�N,M�̒l�͋@�킲�ƂɈႤ�̂ŁA���ۂɎw�肵�Ē�������n��\n");
    pr("         �������AV30 �ł� -w �̒l�͔����ɁA-ws �̒l�͖�������\n");
    pr("-wh      V30�ł� -w,-ws �̎w������̂܂܏�������\n");
  #ifdef HF
    pr("-h       256�F�摜�̂Ƃ��AHF(ʲ�߰�ڰ�)�ɕ\��(����)\n");
  #endif
  #ifdef MP
    pr("-lmm     256�F�摜�̂Ƃ��APC386M,P�Ȃ�256�F��ʂŕ\��(����)\n");
  #endif
    //pr(" -z	  �������Ăˁi�f�o�b�O�p�j\n");
    Exit(0);
}

#if 0
_S void Usage2(void)
{
    printf("%s �� -? �ŕ\��������Ȃ������I�v�V����\n",gNameVer);
    Exit(0);
}
#endif

_S void Option(byte *p)
    //�I�v�V����������
{
    static byte *pp;
    int  c;

    c = *p++;
    c = toupper(c);
    switch (c) {
    case 'B':
    	gUraLoadFlg = 1;
    	break;
    case 'I':
    	gInfoFlg = 1;
    	break;
    case 'C':
    	gClsFlg = -1;
    	if (*p == '1')
    	    gClsFlg = 1;
    	else if (*p == '-' || *p == '0')
    	    gClsFlg = 0;
    	else if (*p == 'P' || *p == 'p') {
    	    gToneDownFlg = 1;
    	    if (*p == '-' || *p == '0')
    	    	gToneDownFlg = 0;
    	} else if (*p && *p != ' ')
    	    OptsErr("-c");
    	break;
    case 'K':
    	gKeyWaitFlg = -1;
    	c = *p++;
    	c = toupper(c);
    	if (c == '?') {
    	    KeyUsage(0);
    	    Exit(0);
    	} else if (c == '-'||c=='0') {
    	    gKeyWaitFlg = 0;
    	} else if (c == '1') {
    	    gKeyWaitFlg = 1;
    	} else if (c == 'G') {
    	    gKeyWaitFlg = 1;
    	    c = *p++;
    	    c = (c=='-') ? '0' : (c==' '||c=='\0') ? '1' : c;
    	    if (c >= '0' && c <= '6')
    	    	gVRamGetNo = c - '0';
    	    else
    	    	OptsRngErr("kg");
    	} else if (c && c != ' ')
    	    OptsErr("-k");
    	break;
    case 'L':
    	c = *p++;
    	c = toupper(c);
    	if (c == 'V') {
    	    if (*p == 'F' || *p == 'f') {
    	    	gEndGrphOffFlg = 1;
    	    }else{
    	    	gEndPalToneFlg = 1;
    	    	gEndPalTone = oDfltTone;
    	    	if (*p && *p != ' ') {
    	    	    gEndPalTone = (word)strtoul(p,&(char *)pp,0);
    	    	    if (gEndPalTone > 200)
    	    	    	OptsRngErr("lv");
    	    	    p = pp;
    	    	}
    	    	if (*p == ',') {
    	    	    p++;
    	    	    oDfltTone = (word)strtoul(p,&(char *)pp,0);
    	    	    if (oDfltTone > 200)
    	    	    	OptsRngErr("lv");
    	    	}
    	    }
    	} else if (c == 'B') {
    	    c = *p++;
    	    c = toupper(c);
    	    c = (c == '\0'||c==' ') ? '1' : (c=='-') ? '0' : c;
    	    if (c >= '0' && c <= '2') {
    	    	gUraWrtMode = c - '0';
    	    } else if (c == 'U') {
    	    	;
    	    } else {
    	    	OptsRngErr("lb");
    	    }
    	} else if (c == 'C') {
    	    if (*p == '\0' || *p == ' ') {
    	    	gOvrLdFlg = 1;
    	    	gOvrLdCol = 0;
    	    } else if (*p == '-') {
    	    	gOvrLdFlg2 = 1;
    	    	gOvrLdCol = 0;
    	    } else {
    	    	gOvrLdFlg = 1;
    	    	gOvrLdCol = (int)strtol(p,NULL,0);
    	    }
    	    if (gOvrLdCol < 0 || gOvrLdCol > 15)
    	    	OptsRngErr("lc");
    	} else if (c == 'Y') {
    	    switch (*p) {
    	    case '0':
    	    	gGdcSclFlg = 0;
    	    	break;
    	    case ' ':
    	    case '\0':
    	    case '1':
    	    	gSclBakFlg = 1;
    	    	gGdcSclFlg = 1;
    	    	break;
    	    case '2':
    	    	gSclBakFlg = 0;
    	    	gGdcSclFlg = 1;
    	    	break;
    	    default:
    	    	OptsErr("-ly");
    	    }
    	} else if (c == 'M') {
    	    if (*p == '\0' || *p == ' ')
    	    	g256mode = 0;
  #ifdef    MP
    	    else if (*p == 'M'||*p == 'm')
    	    	g256mode = 386;
  #endif
    	    else {
    	    	g256mode = (int)strtol(p,NULL,0);
    	    	if (g256mode < 0 || g256mode > 3)
    	    	    OptsRngErr("d");
    	    }
    	} else if (c == 'L') {
    	    gSclLoopFlg = (*p == '-'||*p == '0') ? 0 : 1;
    	} else if (c == 'R') {
    	    gSmlFlg = (*p == '-'||*p == '0') ? 0 : 1;
    	} else if (c && c != ' ') {
    	    OptsErr("-l");
    	}
    	break;
    /*case 'A':
    	gBxByMode = 1;
    	goto OPT_OA:*/
    case 'O':
    	gBxByMode = 2;
    //OPT_OA:
    	gBx = gBy = 0;
    	if (*p == 'a' || *p=='A')
    	    p++,gBxByMode = 1;
    	if (*p == '\0'||*p == ' ')
    	    break;
    	if (*p == '_') {
    	    gBx = -0x7fff;
    	    ++p;
    	} else if (*p != ',') {
    	    gBx = (word)strtol(p,&(char *)pp,0);
    	    p = pp;
    	    if (*p == 'o' || *p == 'O')
    	    	p++,gBx *= 8;
    	    if (gBx > 79*8 || gBx < -1900 || gBx &0x07)
    	    	OptsRngErr("o");
    	}
    	if (*p++ != ',')
    	    break;
    	if (*p == '_') {
    	    gBy = -0x7fff;
    	    ++p;
    	} else {
    	    gBy = (word)strtol(p,&(char *)pp,0);
    	    p = pp;
    	    if (*p == 'o' || *p == 'O')
    	    	p++,gBy *= 8;
    	    if (gBy > 399 || gBy < -1900)
    	    	OptsRngErr("o");
    	}
    	break;
    case 'T':
    	gTxtFlg = 0;
    	if (*p == '1')
    	    gTxtFlg = 1;
    	break;
    case 'V':
    	gPalFlg = 1;
    	gPalTone = oDfltTone;
    	if (*p == '\0' || *p == ' ')
    	    break;
    	gPalTone = (word)strtoul(p,&(char *)pp,0);
    	if (gPalTone > 200)
    	    OptsRngErr("v");
    	break;
    case 'A':
    	gToneUpFlg = gMieMode = gMie2Mode = 0;
    	switch (*p) {
    	case '1':   gMieMode = 2;   break;
    	case '2':   gMieMode = 4;   break;
    	case '3':   gMie2Mode = 1;  break;
    	case '4':   gMie2Mode = 2;  break;
    	case '5':   gToneUpFlg = 1; break;
    	}
    	break;
    case 'W':
    	if (*p == '\0' || *p == ' ') {
    	    gVsWaitCnt0 = 0;
    	} else if (*p >= '0' && *p <= '9') {
    	    gVsWaitCnt0 = (int)strtoul(p,NULL,0);
    	    if (gI8086Flg)
    	    	gVsWaitCnt0 >>= 1;
    	} else if (*p == 'S' || *p == 's') {
    	    p++;
    	    gVsWaitCnt2 = 0;
    	    gVsWaitCnt1 = (int)strtoul(p,&(char *)pp,0);
    	    p = pp;
    	    if (*p == ',') {
    	    	p++;
    	    	gVsWaitCnt2 = (int)strtoul(p,NULL,0);
    	    }
    	    if (gI8086Flg)
    	    	gVsWaitCnt1 = gVsWaitCnt2 = 0;
    	} else if (*p == 'H' || *p == 'h') {
    	    gI8086Flg = 0;
    	    if (*++p == '-')
    	    	gI8086Flg = 1;
    	} else {
    	    OptsErr("-ws");
    	}
    	break;
  #ifdef    HF
    case 'H':
    	gHfMode = 1;	//HyPER-FRAME+ mode
    	//gFBmode = 0;
    	break;
    //case 'F':
    //	gHfMode = 1;	//HyPER-FRAME+ mode
    //	gFBmode = 1;
    //	break;
  #endif
    case 'Z':
    	break;
    case '?':
    	/*if (*p == 'k' || *p == 'K') {
    	    KeyUsage(0);
    	    Exit(0);
    	}*/
    case '\0':
    	Usage();
    default:
    	printf("-%c �͒m��Ȃ��I�v�V������."
    	    "- �� -? �Ő��������Ă�\n",c);
    	Exit(1);
    }
}

_S void Option0(byte *p)
    // -? �� -lbu �̃I�v�V����������
{
    int  c;

    c = *p++;
    c = toupper(c);
    switch (c) {
    case 'Z':
    	c = *p++;
    	c = toupper(c);
    	if (c == 'S')
    	    gDebSclFlg = 1;
    	else if (c == 'Z')
    	    gChkPiLd4a = 1;
      #ifdef PILD4C
    	else if (c == 'C')
    	    gPiLd4cFlg = 1;
      #endif
      #ifdef DEB_DMP
    	else if (c == 'D')
    	    gDmpFlg = 1;
      #endif
    	else {
    	    gChk = 1;
    	}
    	break;
    case 'L':
    	if (*p == 'b' || *p == 'B'){
    	    ++p;
    	    if(*p == 'u' || *p == 'U') {
    	    	gRdBufVRamFlg = 1;
    	    	++p;
    	    	if (*p == '-' || *p == '0')
    	    	    gRdBufVRamFlg = 0;
    	    }
    	}
    	break;
    case 'K':
    	if (*p == '?') {
    	    KeyUsage(0);
    	    Exit(0);
    	}
    	break;
    case '?':
    	if (*p == '?') {
    	    //Usage2();
    	} else if (*p == 'k' || *p == 'K') {
    	    KeyUsage(0);
    	    Exit(0);
    	}
    case '\0':
    	Usage();
    }
}

byte *FGetTok(byte **wk, FILE *fp)
    // �t�@�C�����g�[�N���𓾂�
{
    static byte buf[300+40];
    byte *p,*s;

    if (*wk == NULL) {
  JJJ:
    	*wk = p = fgets(buf,300,fp);
    	if (p == NULL)
    	    return NULL;
    } else
    	p = *wk;
    for (;;) {
    	if (*p == '\0' || *p == '#') {
    	    goto JJJ;
    	} else if (*p > ' '/* && *p != ','*/) {
    	    break;
    	}
    	++p;
    }
    s = p;
    for (;;) {
    	if (*p == '\0') {
    	    *wk = NULL;
    	    return s;
    	} else if (*p <= ' ') {
    	    *p = '\0';
    	    *wk = p+1;
    	    return s;
    	}
    	++p;
    }
}

#ifdef ATMO
void AtMkOdr(byte *p,byte **ap,FILE *fp)
    // @���߂̏���
{
    static byte col[8] = {0,1,4,5,2,3,6,7};
    int x,y,c;

    c = *p;
    switch (c) {
    case 'k':
    	getch();
    	break;
    case 'i':
    	TCls();
    	break;
    case 'p':
    case 'd':
    	p = *ap;
    	if (*p == '"') {
    	    p++; (*ap)++; x = '"';
    	} else if (*p == '\'') {
    	    p++; (*ap)++; x = '\'';
    	} else
    	    x = '\n';
    	while (*p != '\0') {
    	    if (*p == x) {
    	    	*p++ = '\0';
    	    	break;
    	    }
    	    p++;
    	}
    	cprintf ("%s",*ap);
    	*ap = p;
    	if (*p == '\0')
    	    *ap = NULL;
    	if (c == 'p')
    	    cprintf("\r\n");
    	break;
    case 'l':
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@l");
    	x = (int)strtol(p,NULL,0);
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@l");
    	y = (int)strtol(p,NULL,0);
    	gotoxy(x,y);
    	break;
    case 'c':
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@c");
    	x = (int)strtol(p,NULL,0);
    	y = x & 0x0f;
    	x &= 0xff00;
    	textattr(x+col[y]);
    	break;
    case 'w':
    	if ((p = FGetTok(ap,fp)) == NULL)
    	    OptsErr("@w");
    	x = (int)strtol(p,NULL,0);
    	VsyncWait(x);
    	break;
    default:
    	OptsErr("@");
    }
}
#endif

int FilOpt(byte *name)
    // �t�@�C�����I�v�V�����A�t�@�C��������͂��Ď��s
{
    FILE *fp;
    byte *p;
    static byte *aFGetTokWk = NULL;
    static byte tmpName[140];

    fp = fopen(name,"r");
    if (fp == NULL) {
    	printf("%s �̓I�[�v���ł��܂���ł���\n",name);
    	return 1;
    }
    while ((p = FGetTok(&aFGetTokWk,fp)) != NULL) {
    	if (*p == '-') {
    	    Option(p+1);
    	    if (gVRamGetNo)
    	    	VramGetDisp(gVRamGetNo);
    	    gVRamGetNo = 0;
    #ifdef ATMO
    	} else if (*p == '@') {
    	    AtMkOdr(p+1,&aFGetTokWk,fp);
    #endif
    	} else {
    	    ChgExt(tmpName,p,"Pi");
    	    ActGPage(0);
    	    if (PiLoad(tmpName) == 0) {
    	    	if (PiDisp(tmpName))
    	    	    return 1;
    	    	if (gVRamGetNo) {
    	    	    VramGetDisp(gVRamGetNo);
    	    	    gVRamGetNo = 0;
    	    	}
    	    }
    	}
    }
    fclose(fp);
    return 0;
}

void KeyBrk(int sig)
    // Stop-Key , ctrl-c ���荞�ݗp
{
    if (gEndPalToneFlg)
    	RPal_Set(gEndPalTone,PiLd_pal);
    sig = 1;
    Exit(sig);
}

void main(int argc,byte *argv[])
{
    byte *p;
    int  i,n;
    static byte tmpName[140];
    static byte loadName[150];
    static word dmy;
    word  freememszs,inbufszs,ldbufszs;

    Gv_Init();
    signal(SIGINT,KeyBrk);
    gPrgName = argv[0];
    strlwr(gPrgName);
    gPalTone = 100;
    //gUraWrtMode = 0;
    gGdcSclFlg = 1;
    gTxtFlg = 1;
    gSclBakFlg = 1;
    g256mode = 0;   	//-lm0
    gRdBufVRamFlg = 1;	//-lbu
    if (argc < 2)
    	Usage();
    VsWaitInit();
    gI8086Flg = (Chk86()) ? 0 : 1;

    //���ϐ� DOPI ���I�v�V��������
    p = getenv("DOPI");
    if (p) {
    	while (*p) {
    	    //printf("DOPI=%s\n",p);
    	    while(*p == ' ' || *p == '\t')
    	    	p++;
    	    if (*(p++) == '-') {
    	    	Option0(p);
    	    	Option(p);
    	    }
    	    while(*p && *p != ' ' && *p != '\t')
    	    	p++;
    	}
    }

    // -? �� -lbu �p�ɃI�v�V���������
    for (n = i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p != '-') {
    	    n = 0;
    	} else {
    	    Option0(p+1);
    	}
    }

    // �R�}���h���C���ɃI�v�V���������w�肳��Ă��Ȃ��Ƃ�
    if (n) {
    	for (i = 1; i < argc; i++) {
    	    p = argv[i];
    	    if (*p == '-') {
    	    	Option(p + 1);
    	    }
    	}
    	if (gPalFlg) {
    	    RPal_ChgTone(gPalTone);
    	}
      #if defined(MP)
    	if (g256mode == 386) {
    	    Mp_Init();
    	    gUraWrtMode = 3;
    	}
      #endif
    	if (gClsFlg) {
    	    gUraUseMode = gUraWrtMode;
    	    if (gToneDownFlg)
    	    	ToneDown(0);
    	    GCls();
    	}
    	if (gVRamGetNo == 0)
    	    Exit(0);
    }

    // �摜�W�J�p�̃o�b�t�@�̊m��
    freememszs = allocmem(0xffff,&dmy);
    if (freememszs < 0xA00) {
    	MemErr();
    } else if (freememszs < 0x2A00) {
    	inbufszs = 0x400;   //0x400*0x10 = 0x4000 = 16K
    	ldbufszs = freememszs - 0x400 - 0x80;
    } else if (freememszs < 0x3700) {
    	inbufszs = freememszs - 0x2800 - 0x80;
    	ldbufszs = 0x2800;
    } else {
    	inbufszs = 0xD80;
    	ldbufszs = freememszs - 0xD80 - 0x80;
    }
    if (gRdBufVRamFlg) {
    	inbufszs = 0;
    	ldbufszs = freememszs - 0x80;
    }
    if (gChk) {
    	printf("mem:%04x0(%ld)  ����buf:%04x0(%ld)  �摜buf:%04x0(%ld)\n",
    	    freememszs,freememszs*16L, inbufszs,inbufszs*16L,ldbufszs,
    	    ldbufszs*16L);
    	getch();
    }
    if (gRdBufVRamFlg) {
    	PiLd_RdInitVRam();
    } else {
    	if ((PiLd_RdInit(inbufszs*16)) != 0)
    	    MemErr();
    }

    if ((PiLd_LdInit(ldbufszs)) != 0)
    	MemErr();

    // �R�}���h���C���Ƀt�@�C�����Ȃ��� -kg[N]���w�肳�ꂽ�Ƃ�
    if (gVRamGetNo) {
    	VramGetDisp(gVRamGetNo);
    	goto EXIT_MAIN;
    }

    // �t�@�C���A�I�v�V�����̏���
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-') {    	    // �I�v�V����
    	    Option(p + 1);
    	    if (gVRamGetNo)
    	    	VramGetDisp(gVRamGetNo);
    	    gVRamGetNo = 0;
    	} else if (*p == '@') {     // @FILE
    	    if (FilOpt(p+1))
    	    	goto EXIT_MAIN;
    	} else {    	    	    // �t�@�C����
    	    ChgExt(tmpName,p,"pi");
    	    n = DirEntryGet(loadName,tmpName,0);
    	    if (n == 0) {
    	    	do {
    	    	    ActGPage(0);
    	    	    if (PiLoad(loadName) == 0) {
    	    	    	if (PiDisp(loadName))
    	    	    	    goto EXIT_MAIN;
    	    	    	if (gVRamGetNo) {
    	    	    	    VramGetDisp(gVRamGetNo);
    	    	    	    gVRamGetNo = 0;
    	    	    	}
    	    	    }
    	    	} while ((n = DirEntryGet(loadName,NULL,0)) == 0);
    	    } else {
    	    	printf(" %s ��������Ȃ��ł�\n",tmpName);
    	    }
    	}
    }
  EXIT_MAIN:
    if (gEndPalToneFlg) {
    	if (gEx256Flg && gEndPalTone < 100) {
    	    gEndGrphOffFlg = 1;
    	} else {
    	    if (gToneDownFlg)
    	    	ToneDown(gEndPalTone);
    	    else
    	    	RPal_Set(gEndPalTone,PiLd_pal);
    	}
    }
    if (gEndGrphOffFlg) {
      #ifdef HF
    	if (gHfMode) {
    	    Hf_DspOff();
    	    outp( 0x6a, 1); //analog mode
    	}
      #endif
    	_AX = 0x4100;
    	geninterrupt(0x18);
    	//outp( 0x6a, 1);   //analog mode
    }
    Exit(0);
}
