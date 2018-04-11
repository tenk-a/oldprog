#include "subr.h"
#include "aif.h"

static uint8_t *aif_memEnd;
static int aif_err;


static int aif_getMrkInf(aif_inf_t *ai, const uint8_t *mem, int memSz);
static int aif_getRateEasy(uint8_t **a_mem);
static long double aif_getF80(uint8_t **a_mem);


//#ifdef MK_AIF_HDR	// �ȈՂɃc�[��������Ƃ�
#undef DBG_F
#define DBG_F(x)	printf	x
//#endif


/// 2�o�C�g�������擾���|�C���^�X�V
static inline int aif_getW(uint8_t **a_mem)
{
	uint8_t *s = *a_mem;
	int c = 0;
	if (s + 2 < aif_memEnd) {
		c = (uint16_t)PEEKmW(s);
		s += 2;
		*a_mem = s;
	} else {
		aif_err = 1;
	}
	return c;
}

/// 4�o�C�g�������擾���|�C���^�X�V
static inline uint32_t aif_getD(uint8_t **a_mem)
{
	uint8_t *s = *a_mem;
	int c = 0;
	if (s + 4 < aif_memEnd) {
		c = (uint32_t)PEEKmD(s);
		s += 4;
		*a_mem = s;
	} else {
		aif_err = 1;
	}
	return c;
}


/// �������ɒu���ꂽ aiff�̃w�b�_��������͂��ď���Ԃ�
///	- ai	�擾���
///	- mem	�`�F�b�N���郁����
///	- siz	�L���ȃ������͈�
/// - ���A�n 0:�G���[ 1:�擾
int aif_getHdr(aif_inf_t *ai, const void *mem, int memSz)
{
	uint8_t *s = (uint8_t*)mem;
	int		c;
	int		totalSize;
	int		chunkSize;
	int		sz;

	aif_err    = 0;
	aif_memEnd = s + memSz;

	memset(ai, 0, sizeof *ai);

	// FORM�`�����N
	c = aif_getD(&s);						// 0-3
	if (c != BBBB('F','O','R','M')) {
		//DBG_F(("�擪�� 'FORM' �`�����N���Ȃ�\n"));
		return 0;
	}
	totalSize = aif_getD(&s);				// 4-7
	c = aif_getD(&s);						// 8-11
	if (c != BBBB('A','I','F','F')) {
		DBG_F(("AIFF�łȂ�\n"));
		return 0;
	}

	// COMM �`�����N
	c = aif_getD(&s);						// 12-15
	if (c != BBBB('C','O','M','M') || aif_err) {
		DBG_F(("COMM�`�����N���Ȃ�\n"));
		return 0;
	}
	chunkSize = aif_getD(&s);				// 16-19
	if (chunkSize != 18) {
		DBG_F(("COMM�`�����N�̃T�C�Y���z��O�ł�(18�Ƃ�������%d������)", chunkSize));
		if (chunkSize < 18)
			return 0;
	}
	ai->chnl		= aif_getW(&s);			// 20-21
	ai->frames 		= aif_getD(&s);			// 22-25
	ai->smpBit  = c = aif_getW(&s);			// 26-27	�P�T���v���̃r�b�g��(1�`32)
	ai->smpByt		= (c <= 8) ? 1 : (c <= 16) ? 2 : (c <= 24) ? 3 : 4;
  #if 0
	ai->rate		= aif_getF80(&s);		// 28-37
  #else
	ai->rate		= aif_getRateEasy((uint8_t**)&s);	// 28-37 �ȈՂȃ��[�g�擾
  #endif
	// 38�o�C�g�ځ`
	sz = 0;
	{
		uint8_t *p = memStr(s, "MARK", aif_memEnd - s);
		if (p != NULL) {
			int	rc  = aif_getD(&p);
			int	psz = aif_getD(&p);
			rc = aif_getMrkInf(ai, p, psz);
			ai->unkownMark += (rc == 0);
		}
	}

	do {
		s += sz;
		c  = aif_getD(&s);
		sz = aif_getD(&s);
		if (c == BBBB('M','A','R','K')) {
			int rc = aif_getMrkInf(ai, s, sz);
			ai->unkownMark += (rc == 0);
		}
	} while (c && sz && c != BBBB('S','S','N','D') && aif_err == 0);
	if (aif_err) {
		DBG_F(("�w�b�_���傫�����邩�ASSND���Ȃ�\n"));
		return 0;
	}

  #if 0
	c = ai->smpByt * ai->frames * ai->chnl;
	if (sz != c) {
		if (sz > c) {
			sz = c;
		} else {
			DBG_F(("�t���[���� %d ���̃f�[�^������܂���(%d < %d)\n", ai->frames, sz, c));
		}
	}
  #endif

	ai->dataOfsOfs	= aif_getD(&s);
	ai->dataBlkSz	= aif_getD(&s);
	ai->dataSz		= sz - 2*4;
	ai->dataOfs		= s - mem;
	ai->endianTyp	= 1;
	ai->fmtTyp		= 1;
	return 1;
}


#if 1
#include <math.h>
#define UnsignedToFloat(u)         (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)
#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif /*HUGE_VAL*/

/// ���[�g�擾�B�܂��߂�IEEE80�r�b�g�{���x���������_�����擾
static long double aif_getF80(uint8_t **a_mem)
{
	uint8_t *s = *a_mem;
	long double f;
	int			c, expon, hiMant, loMant;

	c		= aif_getW(&s);	// 28-29
	expon 	= c & 0x7fff;
	hiMant	= aif_getD(&s);	// 30-33
	loMant	= aif_getD(&s);	// 34-37
	if (expon == 0 && hiMant == 0 && loMant == 0) {
		f = 0;
	} else {
		if (expon == 0x7fff) {
			f = HUGE_VAL;
		} else {
			expon -= 16383;
			f  = ldexp(UnsignedToFloat(hiMant), (expon -= 31));
			f += ldexp(UnsignedToFloat(loMant), (expon -= 32));
		}
	}
	if (c & 0x80) {
		f = -f;
	}
	*a_mem = s;
	return f;
}
#endif


/// ���[�g�擾�ȈՔŁB
static int aif_getRateEasy(uint8_t **a_mem)
{
	uint8_t *s = *a_mem;
	int c, baseRate, rate, div;

	c        = aif_getW(&s);
	baseRate = aif_getW(&s);	// 44100 or 48000 ��z��...
	if (baseRate != 44100 && baseRate != 48000) {
		DBG_F(("�x��:���[�g��44100 or 48000�̌n��łȂ�...\n"));
	}
	switch (c) {
	case 0x400c:			// 11025Hz or 12000Hz
		div = 4;
		break;
	case 0x400d:			// 22050Hz or 24000Hz
		div = 2;
		break;
	case 0x400e:			// 44100Hz or 48000Hz
		div = 1;
		break;
	default:
		DBG_F(("���Ή��̃T���v�����O���[�g�Ȃ̂ł�...(0x%x,%d)\n",c,baseRate));
		div = 1;
		baseRate = 0;
	}
	rate = baseRate / div;
	s += 6;
	*a_mem = s;

	return rate;
}


/// �}�[�J�[���擾...���[�v�ʒu�擾�̂��߂̎蔲����^^;
static int aif_getMrkInf(aif_inf_t *ai, const uint8_t *mem, int memSz)
{
	const uint8_t *s = (const uint8_t*)mem;
  #if 1
	int n, i, val;
	int loopTop = -1;
	int loopBtm = -1;

	n = PEEKmW(s); s+= 2;
	for (i = 0; i < n; i++) {
		val = PEEKmD(s + 2);
		if (memcmp(s+6+1, "beg loop", 8) == 0) {
			if (loopTop >= 0 && loopTop != val)
				DBG_F(("������loop�J�n�ʒu������A���A�l���������܂�(%d,%d)\n", loopTop,val));
			loopTop = val;
			ai->loopTop = val;
			s += 16;
		} else if (memcmp(s+6+1, "end loop", 8) == 0) {
			if (loopBtm >= 0 && loopBtm != val)
				DBG_F(("������loop�J�n�ʒu������A���A�l���������܂�(%d,%d)\n", loopBtm,val));
			loopBtm = val;
			ai->loopBtm = val;
			s += 16;
		} else if (PEEKmW(s) == 1 && memcmp(s+2+4+1, "Loop", 5) == 0) {
			if (loopTop >= 0 && loopTop != val)
				DBG_F(("������loop�J�n�ʒu������A���A�l���������܂�(%d,%d)\n", loopTop,val));
			loopTop = val;
			ai->loopTop = val;
			s += 12;
		} else if (PEEKmW(s) == 2 && memcmp(s+2+4+1, "Loop", 5) == 0) {
			if (loopBtm >= 0 && loopBtm != val)
				DBG_F(("������loop�J�n�ʒu������A���A�l���������܂�(%d,%d)\n", loopBtm,val));
			loopBtm = val;
			ai->loopBtm = val;
			s += 12;
		} else {
			DBG_F(("�x��:MARK�̂Ȃ��Ƀ��[�v�ȊO�̏�񂪂���\n"));
			return 0;
		}
	}
  #else
	if (memcmp(s+9, "beg loop",8) != 0) {
		//DBG_F(("�x��:MARK�͂��邪���[�v�łȂ�\n"));
		return 0;
	}
	ai->loopTop   = PEEKmD(s+4);
	ai->loopBtm   = PEEKmD(s+20);
  #endif
	return 1;
}




//-------------------------------------------------

typedef struct aif_loopSmpl_t {
	// INST
	char	instId[4];
	char	instSz[4];
	char	instBaseNote[1];
	char	instDetune[1];
	char 	lowNote[1];
	char 	highNote[1];
	char 	lowVelocity[1];
	char 	highVelocity[1];
	char 	gain[2];
	char 	sustainLoop_plyMd[2];
	char 	sustainLoop_start[2];
	char 	sustainLoop_end[2];
	char 	releaseLoop_plyMd[2];
	char 	releaseLoop_start[2];
	char 	releaseLoop_end[2];

	// MARK
	char	markId[4];
	char	markSz[4];
	char	markNum[2];

	char	startId[2];
	char	start[4];
	char	startNameLen[1];
	char	startName[9];

	char	endId[2];
	char	end[4];
	char	endNameLen[1];
	char	endName[9];
} aif_loopSmpl_t;


static aif_loopSmpl_t aif_loopSmpl = {
	//
	{'I','N','S','T'},
	{0,0,0,20},
	0x3c, 0, 0, 0x7f, 0, 0x7f,
	{0,0},
	{0,1},{0,0},{0,1},
	{0,0},{0,0},{0,0},
	//
	{'M','A','R','K'},
	{0,0,0,0x22},
	{0,2},
	{0,0}, {0,0,0,0}, 8, "beg loop",
	{0,1}, {0,0,0,0}, 8, "end loop",
};


static void aif_setRate(uint8_t *d, int rate);


/// mem�ɁAa��������aif�t�@�C���w�b�_���쐬
int aif_setHdr(aif_inf_t *a, void *mem, int siz)
{
	uint8_t *d = (uint8_t*)mem;
	int	loopChunk = 0;
	int sz, totalSz;

	if (siz < 512) {
		// �w�b�_�T�C�Y�Ƃ��ď\���ȃo�C�g����������̂Ƃ��ď����������̂ŁA
		// ���Ȃ��ꍇ�͂��
		return 0;
	}
	if (a->loopBtm > a->loopTop) {
		loopChunk = sizeof(aif_loopSmpl_t);
	}
	totalSz = 12  +  8+18  +  loopChunk  +  8+a->dataSz;

	// �w�b�_
	POKEmD(d, BBBB('F','O','R','M'));	d+=4;	//	chunkID			4	'FORM'
	POKEmD(d, totalSz);					d+=4;	//	chunkSize		4	12+8+18+(loopChunk)+8+sz
	POKEmD(d, BBBB('A','I','F','F'));	d+=4;	//	formType		4	'AIFF'
	// ��{���
	POKEmD(d, BBBB('C','O','M','M'));	d+=4;	//	chunkID			4	'COMM'
	POKEmD(d, 18);						d+=4;	//	chunkSize		4	18
	POKEmW(d, a->chnl);					d+=2;	//	channel			2	1=���m���� 2=�X�e���I
	POKEmD(d, a->frames);				d+=4;	//	sampleFrames	4	�T���v���E�t���[����
	POKEmW(d, a->smpBit);				d+=2;	//	bitsPerSamples	2	�r�b�g/�T���v��
	aif_setRate(d, a->rate);			d+=10;	//	samplePerSec	10	�T���v��/�b (Float80)

	// �g�����[�v���
	if (loopChunk) {
		memcpy(d, &aif_loopSmpl, sizeof(aif_loopSmpl_t));
		POKEmD(&((aif_loopSmpl_t*)d)->start, a->loopTop);
		POKEmD(&((aif_loopSmpl_t*)d)->end,   a->loopBtm);
										d += sizeof(aif_loopSmpl_t);
	}
	// �f�[�^�`�����N
	POKEmD(d, BBBB('S','S','N','D'));	d+=4;	//	chunkID			4	'data'
	POKEmD(d, a->dataSz+8);				d+=4;	//	chunkSize		4	sz(�f�[�^�T�C�Y. �ϒ�)
												//	data			sz	WAV�f�[�^
	POKEmD(d, 0);						d+=4;
	POKEmD(d, 0);						d+=4;

	sz = d - (uint8_t*)mem;
	return sz;
}



static void aif_setRate(uint8_t *d, int rate)
{
	int baseRate = 44100;
	int ex       = 0x400e;

	switch (rate) {
	case 11025:	baseRate = 44100; ex = 0x400c; break;
	case 12000:	baseRate = 48000; ex = 0x400c; break;
	case 22050:	baseRate = 44100; ex = 0x400d; break;
	case 24000:	baseRate = 48000; ex = 0x400d; break;
	case 44100:	baseRate = 44100; ex = 0x400e; break;
	case 48000:	baseRate = 48000; ex = 0x400e; break;
	default:
		printf("%d �͖��Ή��̃T���v�����O�E���[�g�ł�\n", rate);
	}

	POKEmW(d, ex);			d += 2;
	POKEmW(d, baseRate);	d += 2;
	POKEmW(d, 0);			d += 2;
	POKEmD(d, 0);			//d += 4;
}






// �ȈՃe�X�g�v���O������
#ifdef MK_AIF_HDR

int main(int argc, char *argv[])
{
	int i,c;
	char *p;
	if (argc < 2) {
		printf("aif_hdr filename\n");
	}

	for (i = 1; i < argc; i++) {
		static char buf[0x8000];
		aif_inf_t	aifinf, *ai = &aifinf;
		FILE 	*fp;
		p = argv[i];
		printf("%-14s : ", p);
		fp = fopen(p, "rb");
		if (fp == NULL) {
			printf("open�ł��܂���\n", p);
			continue;
		}
		c = fread(buf, 1, 0x8000, fp);
		fclose(fp);
		c = aif_getHdr(ai, buf, c);
		if (c) {
			printf(" %dch %2dbits rate:%d frm:%6d data[sz=%6d ofs=0x%04x]"
				, ai->chnl, ai->smpBit, ai->rate, ai->frames, ai->dataSz, ai->dataOfs);
			if (ai->loopTop == 0 && ai->loopBtm == 0) {
				//printf(" noloop");
			} else {
				printf(" loop(%6d,%6d)", ai->loopTop, ai->loopBtm);
			}
			if (ai->dataBlkSz != 0) {
				printf(" blkSz:%d", ai->dataBlkSz);
			}
			if (ai->dataOfsOfs != 0) {
				printf(" (ofs:%d)", ai->dataOfsOfs);
			}
			if (ai->unkownMark) {
				printf(" used unkown 'MARK'-Chunk");
			}
			if ((uint32_t)ai->dataSz != (uint32_t)ai->smpByt * ai->frames) {
				printf(" bad size!(dataSz != frames*bits/8)");
			}
			printf("\n");
		}
	}
	return 0;
}

#endif

