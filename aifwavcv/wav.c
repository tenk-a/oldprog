#include "subr.h"
#include "wav.h"


/*---------------------------------------
 WAVE フォーマット

 WAVファイルは少なくとも以下３ブロックからなる
	RIFFチャンク	12バイト
	fmtチャンク		24バイト
	dataチャンク	? バイト

各チャンクは以下の通り

RIFF
	chunkID			4	'RIFF'
	chunkSize		4	12+24+sz
	formType		4	'WAVE'
fmt
	chunkID			4	'fmt '
	chunkSize		4	16
	waveFormatType	2	1=リニアPCM(普通は:-)
	channel			2	1=モノラル 2=ステレオ
	samplesPerSec	4	サンプリング周波数(11025,22050,44100など)
	bytesPerSec		4	=blockSize*samplesPerSec
	blockSize		2	=bitsPerSample*channe/8
	bitsPerSamples	4	量子化ビット数. 普通 8か16
data
	chunkID			4	'data'
	chunkSize		4	sz(データサイズ. 可変長)
	data			sz	WAVデータ

これら以外にもチャンクはあり、とくにfmtとdata
の間に別のものがあることはママあり、ちゃんと
チャンクサイズをみてスキャンする必要がある


-----------------------------------------*/

/// ループ情報. なんかのゲームで勝手採用されているチャンクらしい...のを流用
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


/// memに、wa情報を元にwavファイルヘッダを作成
int wav_setHdr(wav_inf_t *wa, void *mem, int siz)
{
	uint8_t *d = (uint8_t*)mem;
	int	loopChunk = 0;
	int sz, totalSz, blkSz;

	if (siz < 512) {
		// ヘッダサイズとして十分なバイト数があるものとして処理したいので、
		// 少ない場合はやめ
		return 0;
	}
	if (wa->loopBtm > wa->loopTop) {
		loopChunk = sizeof(wav_loopSmpl_t);
	}
	totalSz = 12  +  8+16  +  loopChunk  +  8+wa->dataSz;
	blkSz	= wa->chnl * wa->smpByt;

	// ヘッダ
	POKEmD(d, BBBB('R','I','F','F'));	d+=4;	//	chunkID			4	'RIFF'
	POKEiD(d, totalSz);					d+=4;	//	chunkSize		4	12+24+sz
	POKEmD(d, BBBB('W','A','V','E'));	d+=4;	//	formType		4	'WAVE'
	// 基本情報
	POKEmD(d, BBBB('f','m','t',' '));	d+=4;	//	chunkID			4	'fmt '
	POKEiD(d, 16);						d+=4;	//	chunkSize		4	16
	POKEiW(d, 1);						d+=2;	//	waveFormatType	2	1=リニアPCM(普通は:-)
	POKEiW(d, wa->chnl);				d+=2;	//	channel			2	1=モノラル 2=ステレオ
	POKEiD(d, wa->rate);				d+=4;	//	samplesPerSec	4	サンプリング周波数(11025,22050,44100など)
	POKEiD(d, wa->rate*blkSz);			d+=4;	//	bytesPerSec		4	=blockSize*samplesPerSec
	POKEiW(d, blkSz);					d+=2;	//	blockSize		2	=bitsPerSample*channe/8
	POKEiW(d, wa->smpBit);				d+=2;	//	bitsPerSamples	4	量子化ビット数. 普通 8か16
	// 拡張ループ情報
	if (loopChunk) {
		memcpy(d, &wav_loopSmpl, sizeof(wav_loopSmpl_t));
		POKEiD(&((wav_loopSmpl_t*)d)->start, wa->loopTop);
		POKEiD(&((wav_loopSmpl_t*)d)->end,   wa->loopBtm);
										d += sizeof(wav_loopSmpl_t);
	}
	// データチャンク
	POKEmD(d, BBBB('d','a','t','a'));	d+=4;	//	chunkID			4	'data'
	POKEiD(d, wa->dataSz);				d+=4;	//	chunkSize		4	sz(データサイズ. 可変長)
												//	data			sz	WAVデータ

	sz = d - (uint8_t*)mem;
	return sz;
}




/// サウンド・データ先頭にヘッダ情報があるか調べ、あれば、取得する
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
	//情報取得する
	c = PEEKiW(b+4+4);						//	waveFormatType	2	1=リニアPCM(普通は:-)
	if (c != 1)								// いまのとこリニアPCMしか対応してないの
		return 0;
	wa->chnl = PEEKiW(b+4+4 + 2);			//	channel			2	1=モノラル 2=ステレオ
	wa->rate = PEEKiD(b+4+4 + 2+2);			//	samplesPerSec	4	サンプリング周波数(11025,22050,44100など)
											//	bytesPerSec		4	=blockSize*samplesPerSec
											//	blockSize		2	=bitsPerSample*channe/8
	wa->smpBit = PEEKiW(b+4+4 + 2+2+4+4+2);	//	bitsPerSamples	2	量子化ビット数. 普通 8か16
	wa->smpByt = (wa->smpBit+7) / 8;
	//b += sz;

	// dataチャンクを探す
	do {
		sz	= PEEKiD(b+4);
		b	= b + 8 + sz;		// dataチャンクの先頭を探す
		if (b > e) {
			DBG_F(("WAVヘッダ情報が %d bytes以上ある\n", memSz));
			return 0;
		}
		// ループ位置情報があったのなら取得
		if (PEEKmD(b) == BBBB('s','m','p','l')) {
			int loopTop = PEEKiD(b + 52);
			int loopBtm = PEEKiD(b + 56);
			if (loopTop >= 0 && loopBtm > loopTop) {
				wa->loopTop = loopTop;
				wa->loopBtm = loopBtm;
			}
		}
	} while (PEEKmD(b) != BBBB('d','a','t','a'));
	wa->dataSz = PEEKiD(b+4);			// ストリームのサイズな
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
