#include <stddef.h>
#include <dos.h>
#include <stdlib.h>
#include "va.h"

#define _GSC   0x8f

#if 0
/* va_SceenWidth --------------------------------------------------------------
	mode  b15   0:�����ڰ� (����)           1:�ݸ����ڰ�(����)
	(bx)  b14   0:�ݸ���߰��                1:2�߰��
		  b13   0:���̨����ʂ�\�����Ȃ�   1:����

		  b3    �߰��0�̐����摜�x  0:640   1:320
		  b2    �߰��1�̐����摜�x  0:640   1:320

		  b1-0  �����摜�x         00:400  01:408  10:200  11:204

	col0 (cl)   �߰��0���߸�٥����   1,2,4,5,8,16
	col1 (ch)   �߰��1���߸�٥����   1,2,4,5,8,16  (�ݸ����ڰ݂�2�߰��Ӱ�ނ̂�)
	pcol (dh)   �ݸ����ڰݔ���Ӱ�ނł̸��̨�����̫������ޥ�װ
	dp   (dl)   �����ڰ�Ӱ�ނŕ\��������ڰ݂��w��
				b0  ��ڰ�0 (B)
				b1  ��ڰ�1 (R)
				b2  ��ڰ�2 (G)
				b3  ��ڰ�3 (I)
*/
int
va_ScreenWidth(unsigned mode,int col0,int col1,int pcol,int dp)
{
	union REGS reg;

	reg.h.ah = 0x00;
	reg.x.bx = mode;
	reg.h.cl = col0;
	reg.h.ch = col1;
	reg.h.dh = pcol;
	reg.h.dl = dp;
	return int86(_GSC,&reg,&reg);
}
#endif

/* Va_SceenWidth --------------------------------------------------------------
	mode  0  �����ڰ�    1�߰��  ��𑜓x(200)
		  1  �����ڰ�    1�߰��  ���𑜓x(400)
		  4  �ݸ����ڰ�  1�߰��  ��𑜓x(200)
		  5  �ݸ����ڰ�  1�߰��  ���𑜓x(400)
		  6  �ݸ����ڰ�  2�߰��  ��𑜓x(200)
		  7  �ݸ����ڰ�  2�߰��  ���𑜓x(400)
	w0    �߰��0 �̐����摜�x  320 / 640
	col0  �߰��0 ���߸�٥����  1,2,4,5,8,16
	w1    �߰��1 �̐����摜�x  320 / 640
	col1  �߰��1 ���߸�٥����  1,2,4,5,8,16
*/
int
Va_ScreenWidth(int mode,int w0,int c0,int w1,int c1)
{
	union REGS reg;
	unsigned m;

	m  = (mode & 0x04) ? 0xa000 : 0x2000;
	m |= (mode & 0x02) ? 0x4000 : 0;
	m |= ((mode & 0x01) == 0) ? 0x0002 : 0;
	m |= (w0 == 320)   ? 0x0008 : 0;
	m |= (w1 == 320)   ? 0x0004 : 0;

	reg.h.ah = 0x00;
	reg.x.bx = m;
	reg.h.cl = c0;
	reg.h.ch = c1;
	reg.h.dh = 0;
	reg.h.dl = 0xff;
	return int86(_GSC,&reg,&reg);
}

/* Va_Conpose -----------------------------------------------------------------
	p1  ��1�D����
	p2  ��2�D����
	p3  ��3�D����
	p4  ��4�D����
		  p1-p4�̒l
			0 �ݒ肵�Ȃ�
			1 ÷�ĉ��
			2 ���ײĉ��
			3 ���̨������߰��0
			4 ���̨������߰��1
	c   ���ײĉ�ʂň����ő���گĔԍ�(0-15)
*/
int
Va_Compose(int p1, int p2, int p3, int p4,int c)
{
	union REGS reg;

	reg.h.ah = 0x03;
	reg.h.al = c;
	reg.x.cx = (p1 & 0x07)
			 | ((p2 & 0x07) <<  4)
			 | ((p3 & 0x07) <<  8)
			 | (p4 << 12);
	return int86(_GSC,&reg,&reg);
}

#if 0
/* Va_Palette -----------------------------------------------------------------
	n    ��گĔԍ� 0-31
	data �J���[  GGGG00RRRR0BBBB0
*/
int
Va_Palette(int n,unsigned data)
{
	union REGS reg;

	reg.h.ah = 0x08;
	reg.h.al = (char)n;
	reg.x.cx = data;
	return int86(_GSC,&reg,&reg);
}
#endif


/* Va_PalGRB ------------------------------------------------------------------
	n    ��گĔԍ� 0-31
	g    0-15
	r    0-15
	b    0-15
*/
int
Va_PalGRB(int n,int g,int r,int b)
{
	union REGS reg;

	reg.x.cx = ((g & 0x0f) << 12)
			 | ((r & 0x0f) << 6)
			 | ((b & 0x0f) << 1);
	reg.h.ah = 8;
	reg.h.al = (char)n;
	return int86(0x8f,&reg,&reg);
}


/* Va_PaletteMode -------------------------------------------------------------
	mode    0 ���ׂĂ̍\����ʂ���گľ��1( 0-15)���g�p
			1 ���ׂĂ̍\����ʂ���گľ��2(16-31)���g�p
			2 �\����ʂ̂����P����گľ��2(16-31)���g���A����1(0-15)���g�p
			3 32/256�FӰ��
	page    mode = 2 �̂Ƃ��̂�
			0 ÷�ĉ��
			1 ���ײĉ��
			2 ���̨������߰��0
			3 ���̨������߰��1
	bm      ���ݸӰ��:��گľ��1��2����莞�Ԃ��Ƃɐ؂芷����..1,2���킹�Ă̎���
			0 ���ݸ���Ȃ�
			1 32�ڰ�   (32 * 16ms)
			2 64�ڰ�   (64 * 16ms)
			3 128�ڰ� (128 * 16ms)
	bd      ���ݸ���鎞�Ԃ̔䗦
			   ��گ�A   ��گ�B
			0  12.5%    87.5%
			1  25%      75%
			2  50%      50%
			3  75%      25%
*/
int
Va_PaletteMode(int mode,int page,int bm,int bd)
{
	union REGS reg;

	reg.h.ah = 0x09;
	reg.h.al = (char)( ((mode & 0x03) << 6)
					  |((page & 0x03) << 4)
					  |((bm   & 0x03) << 2)
					  |(bd   & 0x03)         );
	return int86(_GSC,&reg,&reg);
}

#if 0
/* Va_PaletteInit -------------------------------------------------------------
   ��گĂ�����������
*/
int
Va_PaletteInit(void)
{
	union REGS reg;

	reg.h.ah = 0x10;
	return int86(_GSC,&reg,&reg);
}
#endif

#if 0
/* Va_Screen ------------------------------------------------------------------
   f    ��ʕ\�� 0:off 1:on
   act  �����ڰ݂̔����łǂ���ڰ݂������ɂ��邩�w��
   dsp  �����ڰ݂̔����łǂ���ڰ݂�\�����邩�w��
*/
int
Va_Screen(int f,int act,int dsp)
{
	union REGS reg;

	reg.h.ah = 0x11;
	reg.h.al = (f == 0) ? 0 : 1;
	reg.h.dl = (char)(((act & 0x07) << 4) | (dsp & 0x07));
	return int86(_GSC,&reg,&reg);
}
#endif


