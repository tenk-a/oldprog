/*
  �o�b�W�W�u�`���o�b�X�W�O�P�p�e�L�X�g�֌W�̎葱��

�@��ʂ� 80*25 �܂��� 80*20 �ŁA���W�͍���(0,0)-�E��(79,24(19)).
  ���e�L�X�g�uRAM�̓T�|�[�g���Ă��Ȃ��B
  �J���[�� 0�`7 �� 98�̃A�g�r�����[�g�Ŏw��B

  ���̃��C�u������ PC88VA,PC98 �����Ŏ��s�\�ȃR�}���h���쐬���邱�Ƃ�
  �l�����Ă��܂��B�܂�A���̕������ɂȂ��Ă���A����Ȃ��Ƃ����ǂ��炩
  �Е��݂̂̂΂����ɂ���ׂăX�s�[�h��T�C�Y�͕s���ł����Aasm �ŏ�������
  �łb�œ��l�̂��̂𑢂邱�Ƃ��v���΁A�܂������ł���͈͂Ǝv���܂��B
  �i�P��VA�̎����Ȃ��āA�ł����� VA ��98�� int 18h ���T�|�[�g���Ă����
  �@�����̂łƂ肠��������ŊԂɉ�킻����... PC98 �Ή��͂��łł�^_^�j

  PC88VA�� PC98 �݂̂̑Ή��ł��̂ł����ȊO�̋@��ł̎��s�͍l�����Ă���
  ����.

 �@�֐��̈������ɂ����ꂽ __PPP__ �͖������ċ󔒂Ǝv���Ă��������B
 */
/*
  �@���̃��C�u�����̊֐���, �ϐ����̕t���������́ACn_Init() �� Cn_flg
  �̂悤��
    �O���[�v�� + '_' + ���O �̂悤�ɂȂ�܂��B
  �O���[�v���͑啶���Ŏn�܂�p����������������r�I�Z�����O�ł��B
  ��ɑ������O�́A
    �֐����̂΂����F�p�啶���Ŏn�܂�p���������������A�ꍇ�ɂ���đ啶��
    	    	    �ł킩��₷���悤�ɋ�؂�. ��) Cn_PutStr()
    �ϐ����̂΂����F�p�������Ŏn�܂�p���������������A�ꍇ�ɂ���đ啶��
 �@ 	    	    �ł킩��₷���悤�ɋ�؂�. ��) Cn_x, Cn_flg
  �̂悤�ɂȂ�܂��B
 */

#ifndef __VA98_H__
#define __VA98_H__

#ifdef __PPP__
    #undef __PPP__
#endif

#ifdef LSI_C
    #define __PPP__ ,...
#else
    #define __PPP__
#endif

/******************************   ��{	 *************************************/
extern unsigned char Cn_mode;

int  Cn_Init(void);
/*
    ���̃��C�u�����𗘗p����ɓ������Ĉ�ԍŏ��Ɉ�x�����w�肷��.
    VA �� 98 ���𔻕ʂ��A ���̒l��Ԃ��B
    	0: VA�ȊO(98)
    	2: VA
    �܂��A���̒l�͑��ϐ� Cn_flg �ɐݒ肳��A�K�v�ȃ��[�`�����Q�Ƃ���B
    ���[�U�� Cn_flg ���Q�Ƃ���̂͂悢�����������Ă͂����Ȃ��B

    PC88VA �ł� MSE �̃��[�h���E����, ���̒l��ۑ����Ă��� VA�X�^���_�[�h
    ���[�h�ɐ؂芷����. �������AMSE�̃��[�h�� VA�l�C�e�B�u�E���[�h�̏ꍇ
    �� al = 1 ��dos �ɕ��A.

*/

void Cn_Term(void);
/*
    ���C�u�����̎g�p����߂�B
    PC88VA ���[�h�ł� MSE �̃��[�h�����ɖ߂��B
    98 ���[�h�ł͉������Ȃ��B
 */

/******************************   �e�L�X�g���	 *****************************/
extern int Cn_x, Cn_y;
extern unsigned char Cn_chrAttr;

void Cn_ConInit(int mode __PPP__);
/*
    int  mode;	0:98ɰ��  1:98ʲڿ�  2:PC88VA

    �e�L�X�g��ʑ���֐��̏������B
    ���̃��C�u�������g�p����ɂ������čŏ��Ɏ��s�B
    	�e��ϐ��E���[�h�̏������B
    	��ʃ��[�h    80*25���[�h  ���W�͈�(����-�E��) (0,0)-(79,24)
    	���ޭڰ�      ��
    	�J�[�\���ʒu  (x,y) = (0,0)
    	�J�[�\���\��  Off
    	�e�L�X�g�\��  on
    	(VA �ł̓e�L�X�g�E�A�N�Z�X�E���[�h)
    �Ȃ��A������ mode �̓e�L�X�g�uRAM �̎�ނ��w�肷��B
�@�@�J�[�\���ʒu��A�g�r�����[�g�l��e�탂�[�h�����O�ŊǗ����Ă���̂ŁA
�@�@���̃��C�u�����̊֐����g�p���ɑ��̃��C�u�����̊֐�(printf�Ƃ�cprintf)
    �ŉ�ʕ\���Ȃǂ��s���ƊǗ����Ă���l�ƌ��݂̕\���Ƃɂ��ꂪ������̂�
    (��ʕ\�������������Ȃ�ł���)�A�Ȃ�ׂ����p�͂����Ă��������B
 */

void Cn_TextOn(void);
/*
    �e�L�X�g�\��on
 */

void Cn_TextOff(void);
/*
    �e�L�X�g�\��off
 */

void Cn_Text20(void);
/*
    �c�s����20�s�ɂ��A��ʂ��N���A����B�J�[�\���ʒu��(0,0)�ɁB
 */

void Cn_Text25(void);
/*
    �c�s����25�s�ɂ��A��ʂ��N���A����B�J�[�\���ʒu��(0,0)�ɁB
 */

void Cn_FuncKeyOn(void);
/*
    �t�@���N�V�����E�L�[��\��
 */

void Cn_FuncKeyOff(void);
/*
    �t�@���N�V�����E�L�[���\��
 */

void Cn_CurOn(void);
/*
    �J�[�\���\��on
 */

void Cn_CurOff(void);
/*
    �J�[�\���\��off
 */

void Cn_CurBlnkOn(void);
/*
    �J�[�\����_�ł���
 */

void Cn_CurBlnkOff(void);
/*
    �J�[�\����_�ł��Ȃ�
 */

void Cn_PosXY(int x, int y __PPP__);
/*
    �J�[�\���ʒu��(x,y)�ɂ���
 */

unsigned Cn_GetPos(void);
/*
    ���A�l x * 0x100 + y
    x : 0�`79
    y : 0�`25
 �@ �܂��A���ϐ� Cn_x,Cn_y �ɂ��ݒ肳���.
    Cn_x,Cn_y �� Cn_GetPos ���s����łȂ��Ɛ������l�������Ȃ�.
 */

unsigned Cn_ResetPos(void);
/*
    printf �ȂǑ��̎葱���ŃR���\�[���o�͂������ƂŁA�J�[�\���ʒu�𒲐�
    ����B
    ���A�l��Cn_GetPos �Ɠ����B
 */

void Cn_Color(int c __PPP__);
/*
    int c   �J���[ 0�`7
    �����̃A�g�r�����[�g�̃J���[��ݒ�
 */

#define Cn_GetColor()	(Cn_chrAttr >> 5)
/*
int  Cn_GetColor(void);
    ���݂̕����A�g�r�����[�g�̃J���[���E��
 */

void Cn_SetAttr(unsigned attr __PPP__);
/*
    �����̃A�g�r�����[�g��ݒ�
    �l�� 98 �̂���
 */

#define Cn_GetAttr()	(Cn_chrAttr)
/*
unsigned Cn_GetAttr(void);
    ���݂̕����A�g�r�����[�g���E��
 */

void Cn_SetCrtMode(int mode __PPP__);
/*
    bit0 ��ʍs��     0:25  1:20
    bit1 �s������     0:80  1:40
    bit2 ���ޭڰ�     0:�����\��  1:�ȈՃO���t
    bit3 K-CG����Ӱ�� 0:���ޱ���  1:�ޯı���
 */

void Cn_GetCrtMode(void);
/*
    bit0 ��ʍs��     0:25  1:20
    bit1 �s������     0:80  1:40
    bit2 ���ޭڰ�     0:�����\��  1:�ȈՃO���t
    bit3 K-CG����Ӱ�� 0:���ޱ���  1:�ޯı���
    bit7 CRT�̎��    0:�W��CRT   1:���𑜓xCRT
 */

int Cn_GetBlock(void far *buf,int x,int y,int w, int h __PPP__);
/*
    �e�L�X�gVRAM�̎w��͈͂̕����ƃA�g�r�����[�g�� buf �ɃR�s�[����B
    �͈͍͂��W(x,y)�̈ʒu�̉� w ����, �c h �����B
    �e�L�X�gVRAM�����̂܂܃R�s�[�������ƁA�A�g�r�����[�g���R�s�[�B
    buf �� w * h * (2+2) �o�C�g�ȏ�m�ۂ��Ă������ƁB
    ���A�l�͐���Ȃ� 0, �w��̂��������Ƃ� 0�ȊO���A���B
 */

int Cn_PutBlock(void far *buf,int x, int y, int w, int h __PPP__);
/*
    �e�L�X�gVRAM�̎w��͈͂�buf �̓��e���R�s�[����B
    buf �̓��e�� Cn_GetBlock() �ŏE���������́B
    ���A�l�͐���Ȃ� 0, �w��̂��������Ƃ� 0�ȊO���A���B
 */

int Cn_ScrlUpY(int y,int n __PPP__);
/*
    �e�L�X�gVRAM��y�s�ڂ���n �s�͈̔͂��P�s�X�N���[���E�A�b�v����
    ���A�l�͐���Ȃ� 0, �w��̂��������Ƃ� 0�ȊO���A���B
 */

int Cn_ScrlDownY(int y,int n __PPP__);
/*
    �e�L�X�gVRAM��y�s�ڂ���n �s�͈̔͂��P�s�X�N���[���E�_�E������
    ���A�l�͐���Ȃ� 0, �w��̂��������Ƃ� 0�ȊO���A���B
 */

void Cn_PutChr(unsigned chr __PPP__);
/*
    ���݂̃J�[�\���ʒu�ɕ���chr��\�����A�J�[�\���ʒu����i�߂�
    chr �� 0�`0xff �܂łȂ� ANK, 0x100�ȏ�Ȃ�V�t�g�i�h�r�Ƃ��đS�p��\��
    �R���g���[���R�[�h��s�K�����̏�������؍s��Ȃ��̂Œ��ӁB
 */

void Cn_PutStr(unsigned char *str __PPP__);
/*
    ���݂̃J�[�\���ʒu�ɕ�����str ��\�����A������̏I���̎��ɃJ�[�\���ʒu��
    �i�߂�B
    chr �� 0�`0xff �܂łȂ� ANK, 0x100�ȏ�Ȃ�V�t�g�i�h�r�Ƃ��đS�p��\��
    �R���g���[���R�[�h��s�K�����̏�������؍s��Ȃ��̂Œ��ӁB
 */

void Cn_PutStrFar(unsigned char far *str __PPP__);
/*
    Cn_PutStr() �� far pointer ��
 */


/*******************************   �L�[����   *******************************/

unsigned Cn_KeyInit(void);
/*
    �L�[���͎葱���̏�����
 */

unsigned Cn_GetKey(void);
/*
    �X�W�̃L�[�R�[�h�����(�҂��Ȃ�)
 */

unsigned Cn_GetKeyWait(void);
/*
    �X�W�̃L�[�R�[�h�����(�L�[���������܂ő҂�)
 */

unsigned Cn_GetShift(void);
/*
    �V�t�g�E�L�[�̏�Ԃ��E��
 */

#endif /* __VA98_H__ */


/******************************  VA  ****************************************/
/* MSE : MSE �� Ӱ�� */
#define MSE_PC9801  -2
#define MSE_NON     -1
#define MSE_NATIVE  0
#define MSE_STD     1
#define MSE_TEXT       2
#define MSE_TEXTK   3
#define MSE_MONO    4
#define MSE_GRPH       5

int Va_GetMseMode(void);
/*
    PC88VA �̌��݂� MSE �̃��[�h��Ԃ��܂��B
    PC98���[�h�̂Ƃ��� 98�O���t�B�b�N�E���[�h��Ԃ��܂��B
 */

void Va_SetMseMode(int mode __PPP__);
/*
    PC88VA �̃��[�h��ύX���܂��B
    Pc98���[�h�ł͉������Ȃ��B
 */

void Va_PutStr(unsigned char far *s);
/*
    PC88VA �̃e�L�X�gBIOS���g���ĕ������\���B
 */

int  Va_ChkVA(void);
/*
    VA �Ȃ� 1 ���A�ȊO�Ȃ� 0 ��Ԃ�
 */

