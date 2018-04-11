/**
 *	@file	aifwavcv.c
 *	@brief	�Ȉ� aiff->wav �R���o�[�^
 *	@author	tenka@6809.net
 *  @date   2003
 */

#include "subr.h"
#include "wav.h"
#include "aif.h"
#include "wav_resize.h"


typedef enum fmt_typ_t {
	FMT_TYP_WAV  = 0,
	FMT_TYP_AIF  = 1,
} fmt_typ_t;


/// �w�b�_���̕ύX�v�f
typedef struct cnv_opt_t {
	int 		loopFlg;		///< bit 0:�擪��ݒ� 1:�I�[��ݒ� 2:�S�̃��[�v��ݒ�
	uint32_t	loopTop;
	uint32_t	loopBtm;
	int			loopRate;		///< ���[�v�l�̒l�����̃��[�g�Ƃ��āA���ۂ̉��̃��[�g�ɕϊ�
	int 		convSmpRate;	///< �T���v�����[�g�̕ϊ��p
	int			resetSmpRate;	///< �w�b�_�̃T���v�����[�g������������p
	int			inpPcmMode;		///< 1=8bit 2=16bitLittle 3=16BitBig
	int			inpPcmRate;
	int			inpPcmChnl;
	int			vagMode;
	int			infoMode;
} cnv_opt_t;


/// �R�}���h���C���E�I�v�V����
typedef struct opts_t {
	char 		*dstDir;
	char 		*srcDir;
	char		*dstName;
	int			dspInfOnly;
	int			outTyp;
	int			update;
	cnv_opt_t	cnv;
} opts_t;


static void resfile(opts_t *o, const char *name);
static void opts_init(opts_t *o);
static void opts_get(opts_t *o, const char *arg);
static void cnv_main(opts_t *o, char *inm);
static int  cnv_oneFile(const char *srcName, const char *dstName, int typ, const cnv_opt_t *o);
static int  cnv_oneFile(const char *srcName, const char *dstName, int typ, const cnv_opt_t *o);
static void dispHdrInfo(const wav_inf_t *ai, const char *name, const cnv_opt_t *o);
static int8_t *oneResize(wav_inf_t *a, const char *name, const cnv_opt_t *o, int8_t *m, uint32_t dstRate);
static int  usage(void);


/** ���C��
 */
int main(int argc, char *argv[])
{
	static opts_t opts;
	opts_t *o = &opts;
	int i;
	char *p;

	if (argc < 2)
		return usage();

	opts_init(o);

	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			opts_get(o, p);
		} else if (*p == '@') {
			resfile(o, p+1);
		} else {
			cnv_main(o, p);
		}
	}
	return 0;
}


/** ���X�|���X�E�t�@�C��
 */
static void resfile(opts_t *o, const char *name)
{
	char buf[1024*64];
	char *p;

	TXT1_OpenE(name);
	while (TXT1_GetsE(buf, sizeof buf)) {
		p = strtok(buf, " \t\n");
		do {
			if (*p == ';') {
				break;
			} else if (*p == '-') {
				opts_get(o, p);
			} else {
				cnv_main(o, p);
			}
			p = strtok(NULL, " \t\n");
		} while (p);
	}
	TXT1_Close();
}



/** �R�}���h���C���E�I�v�V�����̏�����
 */
static void opts_init(opts_t *o)
{
	memset(o, 0, sizeof *o);
	o->outTyp = FMT_TYP_WAV;
	o->dspInfOnly = 1;
}


/** �R�}���h���C���E�I�v�V�����̎擾
 */
static void opts_get(opts_t *o, const char *arg)
{
	const char *p = arg + 1;

	if (memcmp(arg, "-lpa", 4) == 0) {
		o->cnv.loopFlg |= 4;
	} else if (memcmp(arg, "-lps", 4) == 0) {
		o->cnv.loopFlg |= 1;
		o->cnv.loopTop = strtoul(arg+4, 0, 0);
	} else if (memcmp(arg, "-lpe", 4) == 0) {
		o->cnv.loopFlg |= 2;
		o->cnv.loopBtm = strtoul(arg+4, 0, 0);
	} else if (memcmp(arg, "-lrate", 6) == 0) {
		o->cnv.loopRate = strtoul(arg+6, 0, 0);
	} else if (memcmp(arg, "-d", 2) == 0) {
		o->dstDir = strdup(arg+2);
		file_delLastDirSep(o->dstDir);
	} else if (memcmp(arg, "-s", 2) == 0) {
		o->srcDir = strdup(arg+2);
		file_delLastDirSep(o->srcDir);
	} else if (memcmp(arg, "-o", 2) == 0) {
		o->dstName = strdup(arg+2);
	} else if (memcmp(arg, "-wav", 4) == 0) {
		o->outTyp = FMT_TYP_WAV;
		o->dspInfOnly = 0;
	} else if (memcmp(arg, "-aif", 4) == 0) {
		o->outTyp = FMT_TYP_AIF;
		o->dspInfOnly = 0;
	} else if (memcmp(arg, "-pcm8:", 6) == 0) {
		o->cnv.inpPcmMode = 1;
		o->cnv.inpPcmRate = strtoul(arg+6, (char**)&p, 0);
		o->cnv.inpPcmChnl = (*p == ':') ? strtoul(p+1, (char**)&p, 0) : 1;
	} else if (memcmp(arg, "-pcm16l:", 8) == 0) {
		o->cnv.inpPcmMode = 2;
		o->cnv.inpPcmRate = strtoul(arg+8, (char**)&p, 0);
		o->cnv.inpPcmChnl = (*p == ':') ? strtoul(p+1, (char**)&p, 0) : 1;
	} else if (memcmp(arg, "-pcm16b:", 8) == 0) {
		o->cnv.inpPcmMode = 3;
		o->cnv.inpPcmRate = strtoul(arg+8, (char**)&p, 0);
		o->cnv.inpPcmChnl = (*p == ':') ? strtoul(p+1, (char**)&p, 0) : 1;
	} else if (memcmp(arg, "-rr48000", 8) == 0) {
		o->cnv.convSmpRate    = -2;
	} else if (memcmp(arg, "-rr44100", 8) == 0) {
		o->cnv.convSmpRate    = -1;
	} else if (memcmp(arg, "-rs", 3) == 0) {
		o->cnv.resetSmpRate  = strtoul(arg+3, (char**)&p, 0);
	} else if (memcmp(arg, "-rc", 3) == 0) {
		o->cnv.convSmpRate    = strtoul(arg+2, (char**)&p, 0);
	} else if (*p == 'u') {
		o->update = 1;
	} else if (*p == 'i') {
		o->dspInfOnly = 1;
		++p;
		if (*p != 0) {
			o->cnv.infoMode = strtol(p, 0, 10);
		}
	} else {
		printf("unkown option : %s\n", arg);
		exit(1);
	}
}


/** �t�@�C�����𒲐����āA�P�t�@�C���ϊ��B
 */
static void cnv_main(opts_t *o, char *inm)
{
	char srcName[FILE_NMSZ];
	char dstName[FILE_NMSZ];
	char onmBuf[FILE_NMSZ];
	char *onm = o->dstName;
	const char *ext;
	int  outTyp = o->outTyp;

	switch (outTyp) {
	case FMT_TYP_WAV:  ext = "wav";  break;
	case FMT_TYP_AIF:  ext = "aif";  break;
	default:		   printf("PRGERR:cnv_main\n"); exit(1);
	}

	if (onm == NULL) {	// ���͖��̊g���q��wav�ɂ������̂��o�͖��ɂ���
		onm = onmBuf;
		strcpy(onm, inm);
		file_chgExt(onm, ext);
	} else {	// -o�͈�񂱂�����Ȃ�ŁA���g������A�j������... ���ݎc�邪�蔲��
		//free(o->dstName);
		o->dstName = NULL;
	}

	if (o->srcDir)
		sprintf(srcName, "%s\\%s", o->srcDir, inm);
	else
		sprintf(srcName, "%s", inm);

	if (o->dstDir)
		sprintf(dstName, "%s\\%s", o->dstDir, file_baseName(onm));
	else
		sprintf(dstName, "%s", onm);
	onm = dstName;
	if (o->dspInfOnly) {
		onm = NULL;
	} else if (stricmp(srcName, dstName) == 0) {
		printf("�x��:���̓p�X���Əo�̓p�X�����ꏏ�ł��B(�w�b�_�C���p�ɃG���[�ɂ͂��Ă��܂���)\n");
	}
	if (o->update == 0 || file_fdateCmp(dstName, srcName) < 0) {	//��ɕϊ����Aonam�̓��t���Â����
		cnv_oneFile(srcName, onm, outTyp, &o->cnv);
	}
}


/** 16bit �����̏㉺���ꂩ��
 */
static void swapEndian16(wav_inf_t *a, uint8_t *m)
{
	if (a->smpByt == 2) {
		int i, c;
		int16_t *d = (int16_t*)(m + a->dataOfs);
		for (i = 0; i < a->frames * a->chnl; i++) {
			c = *d;
			c = (uint8_t)(c >> 8) | ((uint8_t)c) << 8;
			*d++ = c;
		}
	}
}


/** 1�t�@�C����ϊ��̎���
 */
static int cnv_oneFile(const char *srcName, const char *dstName, int outTyp, const cnv_opt_t *o)
{
	wav_inf_t	audioInf = {0}, *a = &audioInf;
	char 	tmpName[FILE_NMSZ] = {0};
	char 	bakName[FILE_NMSZ] = {0};
	char 	hdr[4096] = {0};
	char 	*ihdr;
	uint8_t	*m = NULL;
	FILE 	*fp;
	int 	sz = 0;
	int 	rc;
	int		hdrSz;

	// aiff �́A�t�@�C���̃P�c�Ƀ��[�v��񂪂���ꍇ������̂ŁA�S���ǂݍ���(T T)
	if (strnicmp(file_extPtr((char*)srcName), "aif", 3) == 0) {
		sz = file_size(srcName);
		if (sz == 0) {
			printf("%s ��open�ł��Ȃ��� �T�C�Y��0�ł�\n", srcName);
			goto RET0;
		}
		hdrSz = sz + 2048;
		m    = calloc(1, hdrSz+0x4000);
		if (m == NULL) {
			printf("�������s��\n");
			goto RET0;
		}
		ihdr = m;
	} else {
		hdrSz = 4096;
		ihdr  = hdr;
	}

	fp = fopen(srcName, "rb");
	if (fp == NULL) {
		printf("%s ��open�ł��Ȃ�����\n", srcName);
		goto RET0;
	}
	sz = fread(ihdr, 1, hdrSz, fp);
	fclose(fp);
	if (sz < 256) {
		printf("�t�@�C��������������\n");
		goto RET0;
	}
	printf("%-14s : ", srcName);
	if (o->inpPcmMode > 0) {
		int blkSz;
		memset(a, 0, sizeof *a);
		a->fmtTyp = 3;
		a->chnl   = o->inpPcmChnl;
		a->rate   = o->inpPcmRate;
		a->smpByt = 1 + (o->inpPcmMode > 1);
		a->smpBit = a->smpByt * 8;
		a->endianTyp = (o->inpPcmMode == 3);
		fp = fopen(srcName, "rb");
		a->dataSz    = filelength(fileno(fp));
		fclose(fp);
		blkSz = a->smpByt * a->chnl;
		a->frames = a->dataSz / blkSz;
		rc = 1;
	} else {
		rc = wav_getHdr(a, ihdr, sz);
		if (rc == 0) {
			rc = aif_getHdr(a, ihdr, sz);
		}
	}
	if (rc == 0) {
		printf("�Ή�����wav,aif�t�@�C���ł͂���܂���ł���\n");
		goto RET0;
	}

	dispHdrInfo(a, srcName, o);

	if (dstName == NULL)
		goto RET0;


	if (m == 0)
		m = file_load((char*)srcName, 0, 0, &sz);
	if (m == NULL) {
		printf("%s �����[�h�ł��Ȃ�����\n", srcName);
		goto RET0;
	}

	//���͂� aif �ŏo�͂� wav �Ȃ�G���f�B�A���΍��swap
	if (a->endianTyp != 0) {
		swapEndian16(a, m);
		//a->endianTyp = 0;
	}

	if (o->resetSmpRate != 0) {
		a->rate = o->resetSmpRate;
	}

	if (o->convSmpRate != 0) {
		m = (uint8_t*)oneResize(a, srcName, o, (int8_t*)(m), o->convSmpRate);
	}

	if (o->loopFlg) {
		if (o->loopFlg & 4) {
			a->loopTop = 0;
			a->loopBtm = a->frames - 1;
			if (o->vagMode)
				a->loopBtm = a->loopBtm / 28 * 28;
		}
		if (o->loopFlg & 1)
			a->loopTop = o->loopTop;
		if (o->loopFlg & 2)
			a->loopBtm = o->loopBtm;
		if (a->loopTop >= a->loopBtm) {
			a->loopBtm = a->frames - 1;
			if (o->vagMode)
				a->loopBtm = a->loopBtm / 28 * 28;
		}
	}
	if (o->loopRate > 0 && o->loopRate != a->rate) {
		a->loopTop = (uint32_t)((uint64_t)a->loopTop * a->rate / o->loopRate);
		a->loopBtm = (uint32_t)((uint64_t)a->loopBtm * a->rate / o->loopRate);
		if (o->vagMode)
			a->loopBtm = a->loopBtm / 28 * 28;
		//printf("*** newLoop(%8d,%8d)\n", a->loopTop, a->loopBtm);
	}

	//a->fmtTyp = outTyp;
	switch (outTyp) {
	case FMT_TYP_WAV:
		hdrSz = wav_setHdr(a, hdr, hdrSz);
		break;

	case FMT_TYP_AIF:
		hdrSz = aif_setHdr(a, hdr, hdrSz);
		break;

	default:
		hdrSz = -0x8000;
	}
	if (hdrSz < 0) {
		printf("PRGERR:wav_setHdr(%d)\n", hdrSz);
		goto RET0;
	}

	//���͂� wav �ŏo�͂� aif �Ȃ�G���f�B�A���΍��swap
	if (/*a->endianTyp == 0 &&*/outTyp == FMT_TYP_AIF)
		swapEndian16(a, m);

	sprintf(bakName, "%s.bak", dstName);
	sprintf(tmpName, "%s.tmp", dstName);

	if (o->vagMode == 0) {
		fp = fopen(tmpName, "wb");
		if (fp == NULL) {
			printf("%s�̏������݃I�[�v���Ɏ��s���܂���\n",dstName);
			goto RET0;
		}

		// �w�b�_����������
		if (hdrSz > 0) {
			rc = fwrite(hdr, 1, hdrSz, fp);
			if (rc != hdrSz) {
				printf("%s�̏������݂Ɏ��s���܂���\n",dstName);
				goto RET0;
			}
		}

		// ���{�̂���������
		sz = a->dataSz;
		rc = fwrite(m + a->dataOfs, 1, sz, fp);
		if ((uint32_t)rc != sz) {
			printf("%s�̏������݂Ɏ��s���܂���.\n",dstName);
			goto RET0;
		}
		fclose(fp);
		free(m);
	} else {
		free(m);
	}
	remove(bakName);
	rename(dstName, bakName);
	rename(tmpName, dstName);
	return 1;

  RET0:
	if (m)
		free(m);
	return 0;
}


/** �w�b�_���\��
 */
static void dispHdrInfo(const wav_inf_t *a, const char *name, const cnv_opt_t *o)
{
	int vsync = (a->frames*60 + a->rate-1)/a->rate;
	printf("\t%dch %2dbits rate:%5d smp:%8d(%6dvsync) data[sz=%8d ofs=0x%04x]"
		, a->chnl, a->smpBit, a->rate, a->frames, vsync, a->dataSz, a->dataOfs);

	if (a->loopTop == 0 && a->loopBtm == 0) {
		//printf(" noloop");
	} else {
		printf(" loop(%8d,%8d)", a->loopTop, a->loopBtm);
	}
	if (a->dataBlkSz != 0) {
		printf(" block-sz:%d", a->dataBlkSz);
	}
	if (a->dataOfsOfs != 0) {
		printf(" (ofs:%d)", a->dataOfsOfs);
	}
	if (a->unkownMark) {
		printf(" used unkown 'MARK'-Chunk");
	}

	if (o->loopFlg) {
		//enum { K = 28 };
		enum   { K = 1 };
		uint32_t loopTop=0,loopBtm=0;
		if (o->loopFlg & 4) {
			loopTop = 0;
			loopBtm = a->frames - 1;
			if (o->vagMode)
				loopBtm = loopBtm / K * K;
		}
		if (o->loopFlg & 1)
			loopTop = o->loopTop;
		if (o->loopFlg & 2)
			loopBtm = o->loopBtm;
		if (loopTop >= loopBtm) {
			loopBtm = a->frames - 1;
			if (o->vagMode)
				loopBtm = loopBtm / K * K;
		}
		if (o->loopRate > 0 && o->loopRate != a->rate) {
			loopTop = (uint32_t)((uint64_t)loopTop * a->rate / o->loopRate);
			loopBtm = (uint32_t)((uint64_t)loopBtm * a->rate / o->loopRate);
			if (o->vagMode)
				loopBtm = loopBtm / K * K;
		}
		printf(" new-loop(%8d,%8d)", loopTop, loopBtm);
	}

	if (a->smpByt) {
		if ((uint32_t)a->dataSz != (uint32_t)a->smpByt * a->frames * a->chnl) {
			printf(" bad size!(dataSz(%#x) != frames(%d)*bits(%d)/8)", a->dataSz, a->frames, a->smpByt*8);
		}
	}
	
	printf("\n");
}


/** LR����
 */
static void wav_splitLR(int16_t *ldst, int16_t *rdst, const int16_t *src, uint32_t frames)
{
	if (frames == 0)
		return;
	do {
		*ldst++ = *src++;
		*rdst++ = *src++;
	} while (--frames);
	return;
}

/** LR����
 */
static void wav_mergeLR(int16_t *dst, const int16_t *lsrc, const int16_t *rsrc, uint32_t frames)
{
	if (frames == 0)
		return;
	do {
		*dst++ = *lsrc++;
		*dst++ = *rsrc++;
	} while (--frames);
	return;
}


/** dstRate�փf�[�^�ϊ�
 */
static int8_t *oneResize(wav_inf_t *a, const char *name, const cnv_opt_t *o, int8_t *m, uint32_t dstRate)
{
	double   rate;
	uint32_t sz;
	uint32_t dstFrames;
	int16_t *src = (int16_t*)(m + a->dataOfs);
	int16_t *dst = src;
	uint32_t srcFrames = a->frames;
	uint32_t srcRate   = a->rate;
	
	if (a->smpBit != 16) {
		printf("%s :ERROR: �T���v�����[�g�̕ϊ���16�r�b�g�ȊO�͖��Ή�\n", name);
		return m;
	}

	if (a->chnl != 1 && a->chnl != 2) {
		printf("%s :ERROR: �T���v�����[�g�̕ϊ���1or2�`�����l���̂�\n", name);
		return m;
	}
	
	if (dstRate == -1) {	// 11025 �̐����{�ɂ���
		if (srcRate <= 44100 / 4)
			dstRate = 44100 / 4;
		else if (srcRate <= 44100 / 2)
			dstRate = 44100 / 2;
		else if (srcRate <= 44100)
			dstRate = 44100;
		else if (srcRate <= 44100*2)
			dstRate = 44100*2;
		else //x if (srcRate <= 44100*4)
			dstRate = 44100*4;
	} else if (dstRate == -2) {
		if (srcRate <= 48000 / 16)
			dstRate = 48000 / 16;
		else if (srcRate <= 48000 / 8)
			dstRate = 48000 / 8;
		else if (srcRate <= 48000 / 4)
			dstRate = 48000 / 4;
		else if (srcRate <= 48000 / 2)
			dstRate = 48000 / 2;
		else if (srcRate <= 48000)
			dstRate = 48000;
		else if (srcRate <= 48000*2)
			dstRate = 48000*2;
		else //x if (srcRate <= 48000*4)
			dstRate = 48000*4;
	}
	assert( dstRate > 0 );
	assert( srcRate > 0 );
	rate = (double)dstRate / (double)srcRate ;
	if (rate < 0.0001 || rate > 1000 ) {
		printf("%s : �T���v�����[�g�ϊ����ɒ[�Ȓl�ɂȂ�(%d/%d=%f)\n", srcRate, dstRate, rate);
		return m;
	}

	if (dstRate == srcRate) {
		printf("\t���[�g�ϊ� ���� %d:%dframes\n", srcRate, srcFrames );
		return m;
	}

	//x dstFrames = (uint32_t)(((uint64_t)srcFrames * (uint64_t)dstRate + (srcRate-1)) / (uint64_t)srcRate);
	dstFrames = (uint32_t)(((uint64_t)srcFrames * (uint64_t)dstRate) / (uint64_t)srcRate);
	sz = dstFrames * 2;

	printf("\t���[�g�ϊ� %d:%dframes->%d:%dframes\n", srcRate, srcFrames, dstRate, dstFrames );

	// ���ۂ̕ϊ�
	if (a->chnl == 1) {
		dst = calloc(1, sz+0x10000);
		if (dst == 0) {
			printf("%s:ERROR:�������s��\n", name);
			return m;
		}

		wav_resize((int16_t*)dst, dstFrames, (int16_t*)src, srcFrames);

		free( m );

	} else {
		int16_t *lsrc = calloc(1, srcFrames*2 + 0x10000);
		int16_t *rsrc = calloc(1, srcFrames*2 + 0x10000);
		int16_t *ldst = calloc(1, sz   + 0x10000);
		int16_t *rdst = calloc(1, sz   + 0x10000);
		dst  = calloc(1, 2*sz + 0x10000);

		if (ldst == 0 || rdst == 0 || dst == 0 || lsrc == 0 || rsrc == 0) {
			printf("%s:ERROR:�������s��\n", name);
			if (dst)
				free(dst);
			if (lsrc)
				free(lsrc);
			if (rsrc)
				free(rsrc);
			if (ldst)
				free(ldst);
			if (rdst)
				free(rdst);
			return m;
		}

		wav_splitLR((int16_t*)lsrc, (int16_t*)rsrc, (int16_t*)src, srcFrames);
		wav_resize((int16_t*)ldst, dstFrames, (int16_t*)lsrc, srcFrames);
		wav_resize((int16_t*)rdst, dstFrames, (int16_t*)rsrc, srcFrames);
		wav_mergeLR((int16_t*)dst, ldst, rdst, dstFrames);

		free(m);
		if (lsrc)
			free(lsrc);
		if (rsrc)
			free(rsrc);
		if (ldst)
			free(ldst);
		if (rdst)
			free(rdst);

		sz *= 2;
	}

	a->dataSz  = sz;
	a->frames  = dstFrames;
	a->rate    = dstRate;
	a->dataOfs = 0;
	a->loopTop = (uint32_t)(((uint64_t)a->loopTop * dstRate + srcRate-1) / (uint64_t)srcRate);
	a->loopBtm = (uint32_t)(((uint64_t)a->loopBtm * dstRate + 0        ) / (uint64_t)srcRate);

	return (uint8_t*)dst;
}


/** �����\��
 */
static int usage(void)
{
	printf("usage> aifwavcv [-opts] file(s)\n");
	printf(" aif,wav�t�@�C�� �� ���ݕϊ�, �w�b�_�\��.\n"
		   "  -i           ���\��(�f�t�H���g)\n"
		   "  -wav         wav �o��\n"
		   "  -aif         aif �o��\n"
		   "  -s[DIR]      ���̓f�B���N�g��\n"
		   "  -d[DIR]      �o�̓f�B���N�g��\n"
		   "  -o[FNAME]    1�t�@�C���ϊ����̏o�͖�\n"
		   "  -u           �o�͐�t�@�C�������݂���Γ��t��r�ŐV�������̂ݏo��\n"
		   "  -rs[SMP]     �w�b�_�̃T���v������ SMP �ɏ���������\n"
		   "  -rc[SMP]     SMP �T���v���Ƀf�[�^�ϊ�\n"
		   //"  -rr[SMP]     SMP��44100��48000�̂�.���̌n��Ŗ����߂��T���v�����Ƀf�[�^�ϊ�\n"
		   "  -lrate[R]    ���[�v�w��ŗp����T���v�����[�g���w��(�f�[�^���̂łȂ����l��)\n"
		   "  -lps[START]  ���[�v�J�n�ʒu(�T���v����)��}��\n"
		   "  -lpe[END]    ���[�v�I���ʒu(�T���v����)��}��\n"
		   "  -lpa         �S�̃��[�v�w��\n"
		   "  -pcm16l:SMP:CH  CH�`�����l�� SMP Hz �̕����t16bit(LE) pcm �����\n"
		   "  -pcm16b:SMP:CH  CH�`�����l�� SMP Hz �̕����t16bit(BE) pcm �����\n"
		   "  -pcm8:SMP:CH    CH�`�����l�� SMP Hz �̕�����8bit pcm �����\n"
		   " �e�t�H�[�}�b�g�Ή��͕s���S�Ȍ���I�Ȃ���.\n"
		   " �����k, 1ch/2ch, 16bit, ���[�g11025/22050/44100/12000/24000/48000 ��z��.\n"
		   //" (wav,aif ���m�ł�8bit�\. wav ���m�ł͑��̃��[�g���\)\n"
	);
	return 1;
}

