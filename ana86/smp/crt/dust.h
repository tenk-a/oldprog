#if 0
int Cn_PutC(unsigned chr);
/*
	���݂̃J�[�\���ʒu�ɕ���chr��\�����A�J�[�\���ʒu����i�߂�
	chr �� 0�`0xff �܂łȂ� ANK, 0x100�ȏ�Ȃ�V�t�g�i�h�r�Ƃ��đS�p��\��
	�ꕔ�R���g���[���R�[�h���T�|�[�g(0x0a,0x0d,0x08,0x09)
	���핶���Ȃ� 0 ���A�s�K�����Ȃ�1��Ԃ��B
 */

int Cn_PutS(unsigned char *str);
/*
	���݂̃J�[�\���ʒu�ɕ�����str ��\�����A������̏I���̎��ɃJ�[�\���ʒu��
	�i�߂�B
	chr �� 0�`0xff �܂łȂ� ANK, 0x100�ȏ�Ȃ�V�t�g�i�h�r�Ƃ��đS�p��\��
	�ꕔ�R���g���[���R�[�h���T�|�[�g(0x0a,0x0d,0x08,0x09)
	���핶���Ȃ� 0 ���A�s�K�����Ȃ�1��Ԃ��B
 */


void Cn_DosPutChr(void);
/*
	DOS �t�@���N�V�����E�R�[�� 0x06 ��p�����R���\�[���ւ� 1�����o��
	ESC�V�[�P���X�ȂǗ��p�����A���p��A�J�[�\���ʒu�����C�u�������̒l����
	���̂ŁA�J�[�\���ʒu��ݒ肷��葱��(Cn_DosGetPos(),Cn_PosXY��) ��
	�J�[�\���ʒu�����킹�Ă��K�v������B
 */

void Cn_DosPutStr(void);
/*
	DOS �t�@���N�V�����E�R�[�� 0x06 ��p�����R���\�[���ւ̕�����o��
	ESC�V�[�P���X�ȂǗ��p�����A���p��A�J�[�\���ʒu�����C�u�������̒l����
	���̂ŁA�J�[�\���ʒu��ݒ肷��葱��(Cn_DosGetPos(),Cn_PosXY��) ��
	�J�[�\���ʒu�����킹�Ă��K�v������B
 */

void Cn_TVRamMode(void);
/*
	PC88VA ��0xA0000����̔z�u���e�L�X�gVRAM �ɂ���B
	98 ���[�h�ł͉������Ȃ��B
 */

#endif

