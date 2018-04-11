#include "subr.h"
#include "wav.h"


/*---------------------------------------
 WAVE �t�H�[�}�b�g

 WAV�t�@�C���͏��Ȃ��Ƃ��ȉ��R�u���b�N����Ȃ�
	RIFF�`�����N	12�o�C�g
	fmt�`�����N		24�o�C�g
	data�`�����N	? �o�C�g

�e�`�����N�͈ȉ��̒ʂ�

RIFF
	chunkID			4	'RIFF'
	chunkSize		4	12+24+sz
	formType		4	'WAVE'
fmt
	chunkID			4	'fmt '
	chunkSize		4	16
	waveFormatType	2	1=���j�APCM(���ʂ�:-)
	channel			2	1=���m���� 2=�X�e���I
	samplesPerSec	4	�T���v�����O���g��(11025,22050,44100�Ȃ�)
	bytesPerSec		4	=blockSize*samplesPerSec
	blockSize		2	=bitsPerSample*channe/8
	bitsPerSamples	4	�ʎq���r�b�g��. ���� 8��16
data
	chunkID			4	'data'
	chunkSize		4	sz(�f�[�^�T�C�Y. �ϒ�)
	data			sz	WAV�f�[�^

�����ȊO�ɂ��`�����N�͂���A�Ƃ���fmt��data
�̊Ԃɕʂ̂��̂����邱�Ƃ̓}�}����A������
�`�����N�T�C�Y���݂ăX�L��������K�v������


-----------------------------------------*/

/// ���[�v���. �Ȃ񂩂̃Q�[���ŏ���̗p����Ă���`�����N�炵��...�̂𗬗p
typedef struct wav_loopSmpl_t {
	uint8_t  smpl[48+4];
	uint32_t start;
	uint32_t end;
	uint8_t  dmy[8];
} wav_loopSmpl_t;

static wav_loopSmpl_t wav_loopSmpl = {
	{
		's', 'm', 'p', 'l',
		0x3c,0,0,0,0,0,0,0,0,0,0,0,0x27,0xb1,0,0,
		0x3c,0,0,0,0,0,0,0,0,0,0,0,   0,   0,0,0,
		   1,0,0,0,0,0,0,0,0,0,0,0,   0,   0,0,0,
	},
	0,		// START
	0,		// END
	{0,0,0,0,0,0,0,0}
};


/// mem�ɁAwa��������wav�t�@�C���w�b�_���쐬
int wav_setHdr(wav_inf_t *wa, void *mem, int siz)
{
	uint8_t *d = (uint8_t*)mem;
	int	loopChunk = 0;
	int sz, totalSz, blkSz;

	if (siz < 512) {
		// �w�b�_�T�C�Y�Ƃ��ď\���ȃo�C�g����������̂Ƃ��ď����������̂ŁA
		// ���Ȃ��ꍇ�͂��
		return 0;
	}
	if (wa->loopBtm > wa->loopTop) {
		loopChunk = sizeof(wav_loopSmpl_t);
	}
	totalSz = 12  +  8+16  +  loopChunk  +  8+wa->dataSz;
	blkSz	= wa->chnl * wa->smpByt;

	// �w�b�_
	POKEmD(d, BBBB('R','I','F','F'));	d+=4;	//	chunkID			4	'RIFF'
	POKEiD(d, totalSz);					d+=4;	//	chunkSize		4	12+24+sz
	POKEmD(d, BBBB('W','A','V','E'));	d+=4;	//	formType		4	'WAVE'
	// ��{���
	POKEmD(d, BBBB('f','m','t',' '));	d+=4;	//	chunkID			4	'fmt '
	POKEiD(d, 16);						d+=4;	//	chunkSize		4	16
	POKEiW(d, 1);						d+=2;	//	waveFormatType	2	1=���j�APCM(���ʂ�:-)
	POKEiW(d, wa->chnl);				d+=2;	//	channel			2	1=���m���� 2=�X�e���I
	POKEiD(d, wa->rate);				d+=4;	//	samplesPerSec	4	�T���v�����O���g��(11025,22050,44100�Ȃ�)
	POKEiD(d, wa->rate*blkSz);			d+=4;	//	bytesPerSec		4	=blockSize*samplesPerSec
	POKEiW(d, blkSz);					d+=2;	//	blockSize		2	=bitsPerSample*channe/8
	POKEiW(d, wa->smpBit);				d+=2;	//	bitsPerSamples	4	�ʎq���r�b�g��. ���� 8��16
	// �g�����[�v���
	if (loopChunk) {
		memcpy(d, &wav_loopSmpl, sizeof(wav_loopSmpl_t));
		POKEiD(&((wav_loopSmpl_t*)d)->start, wa->loopTop);
		POKEiD(&((wav_loopSmpl_t*)d)->end,   wa->loopBtm);
										d += sizeof(wav_loopSmpl_t);
	}
	// �f�[�^�`�����N
	POKEmD(d, BBBB('d','a','t','a'));	d+=4;	//	chunkID			4	'data'
	POKEiD(d, wa->dataSz);				d+=4;	//	chunkSize		4	sz(�f�[�^�T�C�Y. �ϒ�)
												//	data			sz	WAV�f�[�^

	sz = d - (uint8_t*)mem;
	return sz;
}




/// �T�E���h�E�f�[�^�擪�Ƀw�b�_��񂪂��邩���ׁA����΁A�擾����
///
int wav_getHdr(wav_inf_t *wa, const void *mem, int memSz)
{
	const uint8_t *b = (const uint8_t*)mem;
	const uint8_t *e = b + memSz;
	int  sz, hdrSz, c, blkSz;

	memset(wa, 0, sizeof(*wa));

	if (PEEKmD(b) != BBBB('R','I','F','F')) {
		return 0;
	}
	if (PEEKmD(b+8) != BBBB('W','A','V','E')) {
		return 0;
	}
	b += 12;
	if (PEEKmD(b) != BBBB('f','m','t',' ')) {
		return 0;
	}
	//sz = PEEKiD(b + 4);
	//���擾����
	c = PEEKiW(b+4+4);						//	waveFormatType	2	1=���j�APCM(���ʂ�:-)
	if (c != 1)								// ���܂̂Ƃ����j�APCM�����Ή����ĂȂ���
		return 0;
	wa->chnl = PEEKiW(b+4+4 + 2);			//	channel			2	1=���m���� 2=�X�e���I
	wa->rate = PEEKiD(b+4+4 + 2+2);			//	samplesPerSec	4	�T���v�����O���g��(11025,22050,44100�Ȃ�)
											//	bytesPerSec		4	=blockSize*samplesPerSec
											//	blockSize		2	=bitsPerSample*channe/8
	wa->smpBit = PEEKiW(b+4+4 + 2+2+4+4+2);	//	bitsPerSamples	2	�ʎq���r�b�g��. ���� 8��16
	wa->smpByt = (wa->smpBit+7) / 8;
	//b += sz;

	// data�`�����N��T��
	do {
		sz	= PEEKiD(b+4);
		b	= b + 8 + sz;		// data�`�����N�̐擪��T��
		if (b > e) {
			DBG_F(("WAV�w�b�_��� %d bytes�ȏ゠��\n", memSz));
			return 0;
		}
		// ���[�v�ʒu��񂪂������̂Ȃ�擾
		if (PEEKmD(b) == BBBB('s','m','p','l')) {
			int loopTop = PEEKiD(b + 52);
			int loopBtm = PEEKiD(b + 56);
			if (loopTop >= 0 && loopBtm > loopTop) {
				wa->loopTop = loopTop;
				wa->loopBtm = loopBtm;
			}
		}
	} while (PEEKmD(b) != BBBB('d','a','t','a'));
	wa->dataSz = PEEKiD(b+4);			// �X�g���[���̃T�C�Y��
	b += 8;
	hdrSz       = (uint8_t*)b - (uint8_t*)mem;
	wa->dataOfs = hdrSz;
	blkSz = wa->smpByt * wa->chnl;
	if (blkSz > 0)
		wa->frames = wa->dataSz / blkSz;

	wa->endianTyp	= 0;
	wa->fmtTyp		= 0;

	return hdrSz;
}
