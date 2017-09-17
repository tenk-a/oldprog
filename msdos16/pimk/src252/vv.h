#define VV_SIZE 0x4000

typedef struct VV {
    int pln;	//4:16	8:256�F
    int xsize;	    	//�摜����
    int ysize;	    	//    �c��
    int xstart;     	//�\���J�n���W	X
    int ystart;     	//  	    	Y
    int xgscrn;     	//��ʉ���
    int ygscrn;     	//    �c��
    int xgsize;     	//���ۂɕ\������Ƃ��̉���  xxgsize
    int ygsize;     	//  	    	    	    ygsize
    int xgstart;    	//��ʂł̎��ۂ̕\���J�n���W  X
    int ygstart;    	//  	    	    	    �@Y
    int xvsize;     	//���zVRAM����
    int yvsize;     	//  	  �c��
    int xvstart;    	//���zVRAM�̕\��(�]��)�J�n�ʒu(��۰ق����Ƃ���) X
    int yvstart;    	//  	    	    	    	    	    	Y
    //-------------------------------------------------------
    unsigned char far *palette;     	//�p���b�g�E�e�[�u���ւ̃A�h���X
    int aspect1;    	//�h�b�g�̉���
    int aspect2;    	//�h�b�g�̏c��
    int overlayColor;	//�w�i�F

    int ycur;	    	//PutLine �ł̌��݂� Y ���W
    int l200flg;    	//������ 200L �摜��
    int x68flg;     	//�A�X�y�N�g�䂪 x68 �̏ꍇ��
    int reductFlg;  	//�k�����[�h	 0x02:1/4  0x06:1/16
    int reductFlgY; 	//�k�����[�h����Y�X�L�b�v�p�}�X�N
    int ycurReduct; 	//1/16�k�����[�h�ł�PutLine �ł̌��݂� Y ���W
    int loopFlg;    	//���肸�胋�[�v�E���[�h��
    int maxLine;    	//�W�Jٰ�݂��W�J�ł���ő�s��
    //-------------------------------------------------------
    int pgLine;     	//�y�[�W����̍s��
    int pgNo;	    	//�y�[�W�ԍ�
    int pgCnt;	    	//�K�v�y�[�W��
    unsigned pgCurSeg;	//���݂̃y�[�W�̃Z�O�����g
    int pgMmSeg;    	//�y�[�W�Ɏg�����C���������̐擪
    unsigned pgMmSzs;	//�y�[�W�Ɏg���郁�C���E�������̃p���O���t��
    int pgMmCnt;    	//�y�[�W��
    int pgEmsHdl;   	//EMS �̃n���h��
    unsigned pgEmsSeg;	//EMS �̃Z�O�����g
    int pgEmsCnt;   	//EMS �̃y�[�W��
    int pgEmsNo;    	//EMS �̌��݂̃y�[�W�ԍ�
    int pgMaxLine;  	//�ő�s��
    int pglcnt;     	//�s�J�E���^
    int pgPtr;	    	//�y�[�W���̃|�C���^
    int pgXsize;    	//�P�s�̃T�C�Y(�o�C�g��)
    int pgXsizeSubOfs;	//=xxsize*3-1(xxsize*7-1)
    //-------------------------------------------------------
    // 8�h�b�g�Ԋu�̍��W
    int xxvsize;    	//���zVRAM����
    int xofsL;
    int xofsR;
    int xmskL;
    int xmskR;
    int xmskFlg;
    unsigned char  bcolptn[8];	//�w�i�F�̃p�^�[��
    //-------------------------------------------------------
    int ditFlg;     	//���F���邩�ǂ���
    int gvOfs;	    	//�Y���Y���Ŏg��(�\���ʒu�̒���)
    //--------------------------------------------------
    unsigned char dummy[10/*8*/];
    unsigned char pbuf[1280*4+2];
    unsigned char pbuf2[1280*4+10];
} VV;

typedef struct VVF {
    int  no;
    WORD memSize;
    int  (*Init)(   int vvSeg, int mmSeg, unsigned mmSzs,
    	    	    int emsHdl, unsigned emsSeg, unsigned emsCnt,
    	    	    int pln, int xsz,int ysz,int xstart,int ystart,
    	    	    unsigned char far *pal,int asp1,int asp2,
    	    	    int bcol,int reductFlg,int loopFlg);
    void (*Start)(void);    	    	//�v���O�����J�n���̏���
    void (*End)(int tone);  	    	//�v���O�����I�����̂��ƌォ���Â�
    void (*GStart)(void);   	    	//�O���t�B�b�N�J�n
    void (*GEnd)(void);     	    	//�O���t�B�b�N�I��
    void (*GCls)(void);     	    	//�O���t�B�b�N�E�N���A
    void (*ShowOn)(void);   	    	//grphic display on
    void (*ShowOff)(void);  	    	//grphic display off
    void (*SetTone)(int tone);	    	//�g�[���ύX
    void far (*GetLines)(void far *dat,unsigned char far *buf);
    	    	    	    	    	//  �s�N�Z���E�f�[�^���� 4,8�r�b�g�F
    void far (*GetLine24)(void far *dat,unsigned char far *buf);
    	    	    	    	    	//  �s�N�Z���E�f�[�^���� 24�r�b�g�F
    void (*RevX)(unsigned Seg);     	//�����]
    void (*RevY)(unsigned Seg);     	//�c���]
    void (*SetPal)(unsigned Seg,int tone);//�p���b�g�ݒ�
    void (*PutPx)(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
    	    	    	    	    	//����ʂɉ��zVRAM�̓��e��\��
    void (*PutPxScrn)(unsigned Seg,int x0,int y0);
    	    	    	    	    	//�X�N���[���T�C�Y�\��
    void (*PutPxPart)(unsigned Seg);	//�n�_�t�摜�\��
    void (*PutPxBCol)(unsigned Seg);	//�����F�Ή��\��
    void (*PutPxLoop)(unsigned Seg);	//kabe
    //-------------------------
    void (*SclLoopMode)(unsigned Seg,int mode);
    	    	    	    	    	//0:�Y���Y���Ń��[�v���Ȃ� 1:����
    void (*SclUp)(unsigned Seg, int d); //��X�N���[��
    void (*SclDw)(unsigned Seg, int d); //���X�N���[��
    void (*SclLft)(unsigned Seg, int d);//���X�����[��
    void (*SclRig)(unsigned Seg, int d);//�E�X�N���[��
    unsigned long(*GetSclOfs)(unsigned Seg);//VRAM�擪����̉�ʕ\���ʒu�̵̾��
} VVF;

void Vv_Non(void);
void Vv_RevY(unsigned Seg);
void Vv_SclLoopMode(unsigned Seg,int flg);
void Vv_PriInfo(unsigned Seg);
void far Vv_GetLine24(void far *dat ,unsigned char far *buf);
