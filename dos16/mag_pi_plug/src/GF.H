/*
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
	int  rsv;				//
	int  filHdl;			// �t�@�C���E�n���h��
	char saverName[4+2];	// �@�햼
	char artist[18+2];		// ��Җ�
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
	�K�v��؂�64K�޲� (256�F�W�J���͂����64K�� 128K�޲�).
	GF�\���̂��A�N�Z�X����ꍇ�͕��A�l�̃|�C���^�ōs�Ȃ����ƁB
	ret:�\���̂ւ̃|�C���^(�̾��0) �����Ă�0�Ȃ�װ(�̾�Ēl���װNo)
  */

int far PI_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
 /* Pi�摜�̓W�J. �P�s�o��ٰ�݂�n��.
	1�s�o�̓��[�`�����b�ŏ����Ƃ��� _loadds ���w�肷��K�v�����邩��
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
	1�s�o�̓��[�`�����b�ŏ����Ƃ��� _loadds ���w�肷��K�v�����邩��
	ret ax:error no
  */

int far MAG_Close(void far *gf);
 /* MAG ̧�ق�close
	gf �̉���̓��[�U���s�Ȃ�����
	ret ax:error no
  */
