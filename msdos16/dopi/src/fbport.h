/*
    �e�a�p�̃|�[�g���ӂ̃w�b�_��`
    fb.c�ƈꏏ�Ɏg�����ƁB
    �����ŁA���ӂƂ��āG
    	�i�P�jFB_init();�����s���邱�ƁB
    	�i�Q�j�������݂͂w�x�q�f�a�̑S�Ă��s�����Ƃ�O��Ƃ��邱��
    ���̂Q�ɋC������΁A�g�e�^�r�e�͏������E������ʂ�������
    �܂����������l�ɓ��삷�遃�͂����ł���B
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    * 92/06/22	FB_Pxxxx�̌`�̒�`��FBP_xxxx�ɕς���
    * 93/01/06	FB_xxxx();�̊֐���#define��FBxxxx�ɂȂ�悤�ɂ����B
*/

/*------    �ȉ��� fb.pas ���C���v�������g���鎞�ɏo��������	*/
/*  (*----- �r�e�^�g�e�@�h�^�n�|�[�g��` -----*)    */

#define FBP_cont0   	0xd0
#define FBP_cont1   	0xd2
#define FBP_contHF0 	0xd3	    /*	�g�e�ɂ����Ȃ�����ȃ|�[�g  */
#define FBP_Xreg    	0xd4
#define FBP_Yreg    	0xd6
#define FBP_RedReg  	0xd8
#define FBP_GreReg  	0xda
#define FBP_BluReg  	0xdc
#define FBP_ModReg  	0xde
#define FBP_BnkReg0 	0xec
#define FBP_BnkReg1 	0xee

/*  (*----- �r�e���[�h���W�X�^��` (0DEh) -----*)   */
#define PCONLY	    0x00
#define BOTH	    0x01
#define FBONLY	    0x03    	/* �r�e�Q���ł͂o�b��ʂ��J�b�g�o���Ȃ� */
#define FBOFF	    0x04    	/* HF+�̊g���B�|�[�g��������̂�h��	*/

#define SF_STEALWR  0x80    	/* �X�`�[�����C�g */
#define SF_AUTOINC  0x40    	/* �w���̃I�[�g�C���N�������g */
#define SF_GRAYMODE 0x08

/*  (*----- �g�e���[�h���W�X�^��` (0D0h) -----*)   */
#define HF_IOXRAM   0x01    	/* �A�N�Z�X���@ 0:BANK 1:I/O	*/
#define HF_WRMASK   0x02    	/* ���C�g�}�X�N 0:OFF  1:ON 	*/
#define HF_RGBWR    0x04    	/* RGB��������	0:OFF  1:ON 	*/
#define HF_AUTINC   0x08    	/* BANK����inc	0:OFF  1:ON 	*/
#define HF_CGX98    0x10    	/* HF��ʕ\��	0:98   1:HF 	*/
#define HF_IMPOSE   0x20    	/* �C���|�[�Y	0:OFF  1:ON 	*/
#define HF_EXTON    0x40    	/* ���W�X�^�g�p 0:�s�� 1:�� 	*/
#define HF_XSFTRST  0x80    	/* �\�t�g���Z�b�g   	    	*/

#define NoFrameBuf  0
#define SuperFrame  1
#define HyperFrame  2

/*=============================================================================
    �g�e���W�X�^�֘A�A����ɒ�`    �i�g�e�{�Ɠ��̊g���j
=============================================================================*/
#define HF_REG_BASEX	0x80
#define HF_REG_BASEY	0x193
#define HF_CUR_OFFX 	150
#define HF_CUR_OFFY 	31

#define HF_REG_SCROLLX	4
#define HF_REG_SCROLLY	6

/*=============================================================================
    inline assembler���g���������ȃ|�[�g�o��
=============================================================================*/
/*  tcc�̓f�����\�[�X��������call�𐶐������̂łт����肵�č�����}�N�� */
/*  �g������A�K��#pragma inline��錾���邱��	    	    	    	*/

#define ioutw(l,m) {\
    _AX=(m);\
    asm out l,ax;\
}

#define ioutb(l,m) {\
    _AL=(m);\
    asm out l,al;\
}

#define FBInit()    FB_Init()
#define FBMode(x)   FB_Mode((x))
#define FB_Clear()  FBClear()
