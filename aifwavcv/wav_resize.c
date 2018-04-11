#include "subr.h"
#include "wav_resize.h"


inline int	ifval(int c, int  t, int  f) {
	c = (c != 0);
	return (t & (-(int)(c))) | (f & ((int)(c) - 1));
}


inline int clamp(int val, int mi, int ma) {
 #if 1
	return (val < mi) ? mi : (val < ma) ? val : ma;
 #else
	int	bMi = (val >= mi);
	int	bMa = (val <= ma);
	return (val & ((-bMi) & (-bMa))) | (mi & (bMi - 1)) | (ma & (bMa - 1));
 #endif
}


// バイリニアを利用した、wav のリサイズ
void wav_resize(int16_t dst[], unsigned dstLen, const int16_t src[], unsigned srcLen)
{
	double	rl0  = (double)srcLen / dstLen;
	int16_t 	ml  = (int16_t)rl0 + 1;			// 何分の一にするか
	int16_t 	dml = dstLen * ml;			// 出力幅の整数倍かつ元サイズよりも大きい値
	double	rl  = (double)srcLen / dml;
	unsigned di;

	if (srcLen == dstLen) {
		memcpy(dst, src, dstLen);
		return;
	}

	if (srcLen == 0 || dstLen == 0)
		return;

	for (di = 0; di < dstLen; di ++) {
		double fc = 0.0;
		unsigned i;
		for (i = di*ml; i < (di+1)*ml; ++i) {
			double si = i * rl - 0.5;
			int16_t i0 = (int16_t)si;
			double kl = si - i0;

			double k0 = (1.0F - kl);
			double k1 = kl;
			int16_t c0 =  src[i0];
			int16_t c1 =  src[i0 + 1];
		 #if 1
			if (i0+1 >= srcLen)
				c1	= c0;
		 #else
			c1     =  ifval((i0+1 < srcLen), c1, c0);
		 #endif
			fc     += c0*k0 + c1*k1;
		}
		{
			int  ic = (int)(fc / ml);
			ic      = clamp(ic, -0x7fff, 0x7fff);
			dst[di] = ic;
		}
	}
}



