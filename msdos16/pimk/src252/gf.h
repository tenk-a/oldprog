/*
	Pi �W�J
		writen by M.Kitamura(Tenk*)
		1993/12/11
 */

typedef struct GF {
	//char dummy[16];		// �Z�O�����g���E�����p
	/*-------------*/
	int  pln;				// �F�̃r�b�g��(4 or 8)
	int  xsize;				// ����(�h�b�g)
	int  ysize;				// �c��(�h�b�g)
	int  xstart;			// �n�_x
	int  ystart;			// �n�_y
	int  aspect1;			// �A�X�y�N�g��x
	int  aspect2;			// �A�X�y�N�g��y
	int  overlayColor;		// �����F
	int  color;				// �p���b�g�̎g�p��
	int  palBit;			// �p���b�g�̃r�b�g��
	int  exSize;			// pi�g���̈�T�C�Y
	int  dfltPalFlg;		// pi�f�t�H���g�p���b�g�t���O
	int  loopMode;			// pi���[�v���[�h(pimk�̎���/�B��TAG)
	int  filHdl;			// �t�@�C���E�n���h��
	BYTE saverName[4+2];	// �@�햼
	BYTE artist[18+2];		// ��Җ�
	/*-------------*/
	char dummy2[256-(14*2+6+20)];
	/*-------------*/
	char palette[256*3];
	char fileName[254+2];
	char comment[2048+510+2];
} GF;

/*- PI ----------------------------------------------------------------------*/
GF far * far PI_Open(void far *buf, char far *fileName);
 /* fileName��open ���āA�w�b�_��ǂݍ���. buf�Ŏg�p���郁�������w��.
	��؂͌Ăяo�����Ŏ��O�Ɋm�ۂ��Ă�������.
	�K�v��؂�64K�޲�.
	GF�\���̂��A�N�Z�X����ꍇ�͕��A�l�̃|�C���^�ōs�Ȃ����ƁB
	256�F���۰�ނł͂����64K�޲Ċm�ۂ���PI_SetC256buf�Őݒ肷��K�v������
	ret:�\���̂ւ̃|�C���^(�̾��0) �����Ă�0�Ȃ�װ(�̾�Ēl���װNo)
  */

void far PI_SetC256buf(unsigned bufseg);
 /* 256�F�W�J���ɕK�v��64K�޲ĐF�\����؂��w��.
	��؂͎��O�Ɋm�ۂ��邱��. PI_Load �����s����O�ɐݒ肷�邱��.
	ret ax:error no
  */

int far PI_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
 /* Pi�摜�̓W�J. �P�s�o��ٰ�݂�n��
	ret ax:error no
  */

int far PI_Close(void far *gf);
 /* Pi ̧�ق�close
	gf �̉���̓��[�U���s�Ȃ�����
	ret ax:error no
  */

/*- MAG ---------------------------------------------------------------------*/
GF far * far MAG_Open(void far *gf, char far *fileName);
 /* fileName��open ���āA�w�b�_��ǂݍ���. gf �Ŏg�p���郁�������w��.
	��؂͌Ăяo�����Ŏ��O�Ɋm�ۂ��Ă�������.
	�K�v��؂�128K�޲�.
	GF�\���̂��A�N�Z�X����ꍇ�͕��A�l�̃|�C���^�ōs�Ȃ����ƁB
	ret:�\���̂ւ̃|�C���^(�̾��0) �����Ă�0�Ȃ�װ(�̾�Ēl���װNo)
  */

int far MAG_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
 /* MAG�摜�̓W�J. �P�s�o��ٰ�݂�n��
	ret ax:error no
  */

int far MAG_Close(void far *gf);
 /* MAG ̧�ق�close
	gf �̉���̓��[�U���s�Ȃ�����
	ret ax:error no
  */
/*- PMT & DJP ---------------------------------------------------------------*/
GF far * far PMT_Open(void far *gf, char far *fileName);	// gf:�v64K�޲�
GF far * far DJP_Open(void far *gf, char far *fileName);	// gf:�v64K�޲�
int far DJP_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
int far DJP_Close(void far *gf);
void far DJP_AutoChk16(int sw);
	 /* �p���b�g 16�`255 ���S�� 0�Ȃ�P�U�F�摜�Ƃ��Ă����� */
/*- Q0 ---------------------------------------------------------------------*/
GF far * far Q0_Open(void far *gf, char far *fileName);		// gf:�v64K�޲�
int far Q0_Close(void far *gf);
/*- BMP ---------------------------------------------------------------------*/
GF far * far BMP_Open(void far *gf, char far *fileName);	// gf:�v64K�޲�
int far BMP_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
int far BMP_Close(void far *gf);
