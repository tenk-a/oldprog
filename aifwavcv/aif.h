/**
 *	@file	aif.h
 *	@brief	AIF‚Ìƒwƒbƒ_î•ñæ“¾
 *	@author	tenka@6809.net
 */
#ifndef AIF_H
#define AIF_H

#if 1
#include "wav.h"
typedef wav_inf_t aif_inf_t;
#else
typedef struct aif_inf_t {
	int			chnl;
	int			frames;
	int			smpBit;
	int			smpByt;
	int			rate;
	uint32_t	dataSz;
	uint32_t	dataOfs;
	uint32_t	loopTop;
	uint32_t	loopBtm;
	uint32_t	dataBlkSz;
	uint32_t	dataOfsOfs;
	//
	int			unkownMark;
} aif_inf_t;
#endif

int aif_getHdr(aif_inf_t *ai, const void *mem, int siz);
int aif_setHdr(aif_inf_t *a, void *mem, int siz);

#endif

