#ifndef WAV_H
#define WAV_H

typedef struct wav_inf_t {
	int			fmtTyp;			// 0:wav 1:aif

	int			chnl;
	int			frames;
	int			smpBit;
	int			smpByt;
	int			rate;
	uint32_t	dataSz;
	uint32_t	dataOfs;
	uint32_t	loopTop;
	uint32_t	loopBtm;
	int			endianTyp;		// 0:little 1:big

	uint32_t	flags;
	uint32_t	dataBlkSz;
	uint32_t	dataOfsOfs;
	int			unkownMark;
} wav_inf_t;

int wav_getHdr(wav_inf_t *wa, const void *mem, int siz);
int wav_setHdr(wav_inf_t *wa, void *mem, int siz);

#endif
