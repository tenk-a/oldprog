#include <stddef.h>
#include <dos.h>
#include <stdlib.h>
#include "va.h"

#define _GSC   0x8f

#if 0
/* va_SceenWidth --------------------------------------------------------------
	mode  b15   0:ﾏﾙﾁﾌﾟﾚｰﾝ (水平)           1:ｼﾝｸﾞﾙﾌﾟﾚｰﾝ(垂直)
	(bx)  b14   0:ｼﾝｸﾞﾙﾍﾟｰｼﾞ                1:2ﾍﾟｰｼﾞ
		  b13   0:ｸﾞﾗﾌｨｯｸ画面を表示しない   1:する

		  b3    ﾍﾟｰｼﾞ0の水平画像度  0:640   1:320
		  b2    ﾍﾟｰｼﾞ1の水平画像度  0:640   1:320

		  b1-0  垂直画像度         00:400  01:408  10:200  11:204

	col0 (cl)   ﾍﾟｰｼﾞ0のﾋﾟｸｾﾙ･ｻｲｽﾞ   1,2,4,5,8,16
	col1 (ch)   ﾍﾟｰｼﾞ1のﾋﾟｸｾﾙ･ｻｲｽﾞ   1,2,4,5,8,16  (ｼﾝｸﾞﾙﾌﾟﾚｰﾝで2ﾍﾟｰｼﾞﾓｰﾄﾞのみ)
	pcol (dh)   ｼﾝｸﾞﾙﾌﾟﾚｰﾝ白黒ﾓｰﾄﾞでのｸﾞﾗﾌｨｯｸｽのﾌｫｱｸﾞﾗﾝﾄﾞ･ｶﾗｰ
	dp   (dl)   ﾏﾙﾁﾌﾟﾚｰﾝﾓｰﾄﾞで表示するﾌﾟﾚｰﾝを指定
				b0  ﾌﾟﾚｰﾝ0 (B)
				b1  ﾌﾟﾚｰﾝ1 (R)
				b2  ﾌﾟﾚｰﾝ2 (G)
				b3  ﾌﾟﾚｰﾝ3 (I)
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
	mode  0  ﾏﾙﾁﾌﾟﾚｰﾝ    1ﾍﾟｰｼﾞ  低解像度(200)
		  1  ﾏﾙﾁﾌﾟﾚｰﾝ    1ﾍﾟｰｼﾞ  高解像度(400)
		  4  ｼﾝｸﾞﾙﾌﾟﾚｰﾝ  1ﾍﾟｰｼﾞ  低解像度(200)
		  5  ｼﾝｸﾞﾙﾌﾟﾚｰﾝ  1ﾍﾟｰｼﾞ  高解像度(400)
		  6  ｼﾝｸﾞﾙﾌﾟﾚｰﾝ  2ﾍﾟｰｼﾞ  低解像度(200)
		  7  ｼﾝｸﾞﾙﾌﾟﾚｰﾝ  2ﾍﾟｰｼﾞ  高解像度(400)
	w0    ﾍﾟｰｼﾞ0 の水平画像度  320 / 640
	col0  ﾍﾟｰｼﾞ0 のﾋﾟｸｾﾙ･ｻｲｽﾞ  1,2,4,5,8,16
	w1    ﾍﾟｰｼﾞ1 の水平画像度  320 / 640
	col1  ﾍﾟｰｼﾞ1 のﾋﾟｸｾﾙ･ｻｲｽﾞ  1,2,4,5,8,16
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
	p1  第1優先画面
	p2  第2優先画面
	p3  第3優先画面
	p4  第4優先画面
		  p1-p4の値
			0 設定しない
			1 ﾃｷｽﾄ画面
			2 ｽﾌﾟﾗｲﾄ画面
			3 ｸﾞﾗﾌｨｯｸ画面ﾍﾟｰｼﾞ0
			4 ｸﾞﾗﾌｨｯｸ画面ﾍﾟｰｼﾞ1
	c   ｽﾌﾟﾗｲﾄ画面で扱う最大ﾊﾟﾚｯﾄ番号(0-15)
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
	n    ﾊﾟﾚｯﾄ番号 0-31
	data カラー  GGGG00RRRR0BBBB0
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
	n    ﾊﾟﾚｯﾄ番号 0-31
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
	mode    0 すべての構成画面がﾊﾟﾚｯﾄｾｯﾄ1( 0-15)を使用
			1 すべての構成画面がﾊﾟﾚｯﾄｾｯﾄ2(16-31)を使用
			2 構成画面のうち１つがﾊﾟﾚｯﾄｾｯﾄ2(16-31)を使い、他は1(0-15)を使用
			3 32/256色ﾓｰﾄﾞ
	page    mode = 2 のときのみ
			0 ﾃｷｽﾄ画面
			1 ｽﾌﾟﾗｲﾄ画面
			2 ｸﾞﾗﾌｨｯｸ画面ﾍﾟｰｼﾞ0
			3 ｸﾞﾗﾌｨｯｸ画面ﾍﾟｰｼﾞ1
	bm      ﾌﾞﾘﾝｸﾓｰﾄﾞ:ﾊﾟﾚｯﾄｾｯﾄ1と2を一定時間ごとに切り換える..1,2合わせての時間
			0 ﾌﾞﾘﾝｸしない
			1 32ﾌﾚｰﾑ   (32 * 16ms)
			2 64ﾌﾚｰﾑ   (64 * 16ms)
			3 128ﾌﾚｰﾑ (128 * 16ms)
	bd      ﾌﾞﾘﾝｸする時間の比率
			   ﾊﾟﾚｯﾄA   ﾊﾟﾚｯﾄB
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
   ﾊﾟﾚｯﾄを初期化する
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
   f    画面表示 0:off 1:on
   act  ﾏﾙﾁﾌﾟﾚｰﾝの白黒でどのﾌﾟﾚｰﾝを書込可にするか指定
   dsp  ﾏﾙﾁﾌﾟﾚｰﾝの白黒でどのﾌﾟﾚｰﾝを表示するか指定
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


