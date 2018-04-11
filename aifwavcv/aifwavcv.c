/**
 *	@file	aifwavcv.c
 *	@brief	簡易 aiff->wav コンバータ
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


/// ヘッダ情報の変更要素
typedef struct cnv_opt_t {
	int 		loopFlg;		///< bit 0:先頭を設定 1:終端を設定 2:全体ループを設定
	uint32_t	loopTop;
	uint32_t	loopBtm;
	int			loopRate;		///< ループ値の値をこのレートとして、実際の音のレートに変換
	int 		convSmpRate;	///< サンプルレートの変換用
	int			resetSmpRate;	///< ヘッダのサンプルレートを書き換える用
	int			inpPcmMode;		///< 1=8bit 2=16bitLittle 3=16BitBig
	int			inpPcmRate;
	int			inpPcmChnl;
	int			vagMode;
	int			infoMode;
} cnv_opt_t;


/// コマンドライン・オプション
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


/** メイン
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


/** レスポンス・ファイル
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



/** コマンドライン・オプションの初期化
 */
static void opts_init(opts_t *o)
{
	memset(o, 0, sizeof *o);
	o->outTyp = FMT_TYP_WAV;
	o->dspInfOnly = 1;
}


/** コマンドライン・オプションの取得
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


/** ファイル名を調整して、１ファイル変換。
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

	if (onm == NULL) {	// 入力名の拡張子をwavにしたものを出力名にする
		onm = onmBuf;
		strcpy(onm, inm);
		file_chgExt(onm, ext);
	} else {	// -oは一回こっきりなんで、一回使ったら、破棄する... ごみ残るが手抜き
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
		printf("警告:入力パス名と出力パス名が一緒です。(ヘッダ修正用にエラーにはしていません)\n");
	}
	if (o->update == 0 || file_fdateCmp(dstName, srcName) < 0) {	//常に変換か、onamの日付が古ければ
		cnv_oneFile(srcName, onm, outTyp, &o->cnv);
	}
}


/** 16bit 整数の上下入れかえ
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


/** 1ファイルを変換の実際
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

	// aiff は、ファイルのケツにループ情報がある場合があるので、全部読み込む(T T)
	if (strnicmp(file_extPtr((char*)srcName), "aif", 3) == 0) {
		sz = file_size(srcName);
		if (sz == 0) {
			printf("%s をopenできないか サイズが0です\n", srcName);
			goto RET0;
		}
		hdrSz = sz + 2048;
		m    = calloc(1, hdrSz+0x4000);
		if (m == NULL) {
			printf("メモリ不足\n");
			goto RET0;
		}
		ihdr = m;
	} else {
		hdrSz = 4096;
		ihdr  = hdr;
	}

	fp = fopen(srcName, "rb");
	if (fp == NULL) {
		printf("%s をopenできなかった\n", srcName);
		goto RET0;
	}
	sz = fread(ihdr, 1, hdrSz, fp);
	fclose(fp);
	if (sz < 256) {
		printf("ファイルが小さすぎる\n");
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
		printf("対応したwav,aifファイルではありませんでした\n");
		goto RET0;
	}

	dispHdrInfo(a, srcName, o);

	if (dstName == NULL)
		goto RET0;


	if (m == 0)
		m = file_load((char*)srcName, 0, 0, &sz);
	if (m == NULL) {
		printf("%s をロードできなかった\n", srcName);
		goto RET0;
	}

	//入力が aif で出力が wav ならエンディアン対策でswap
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

	//入力が wav で出力が aif ならエンディアン対策でswap
	if (/*a->endianTyp == 0 &&*/outTyp == FMT_TYP_AIF)
		swapEndian16(a, m);

	sprintf(bakName, "%s.bak", dstName);
	sprintf(tmpName, "%s.tmp", dstName);

	if (o->vagMode == 0) {
		fp = fopen(tmpName, "wb");
		if (fp == NULL) {
			printf("%sの書き込みオープンに失敗しました\n",dstName);
			goto RET0;
		}

		// ヘッダを書き込む
		if (hdrSz > 0) {
			rc = fwrite(hdr, 1, hdrSz, fp);
			if (rc != hdrSz) {
				printf("%sの書き込みに失敗しました\n",dstName);
				goto RET0;
			}
		}

		// 音本体を書き込む
		sz = a->dataSz;
		rc = fwrite(m + a->dataOfs, 1, sz, fp);
		if ((uint32_t)rc != sz) {
			printf("%sの書き込みに失敗しました.\n",dstName);
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


/** ヘッダ情報表示
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


/** LR分離
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

/** LR合体
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


/** dstRateへデータ変換
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
		printf("%s :ERROR: サンプルレートの変換は16ビット以外は未対応\n", name);
		return m;
	}

	if (a->chnl != 1 && a->chnl != 2) {
		printf("%s :ERROR: サンプルレートの変換は1or2チャンネルのみ\n", name);
		return m;
	}
	
	if (dstRate == -1) {	// 11025 の整数倍にする
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
		printf("%s : サンプルレート変換が極端な値になる(%d/%d=%f)\n", srcRate, dstRate, rate);
		return m;
	}

	if (dstRate == srcRate) {
		printf("\tレート変換 無し %d:%dframes\n", srcRate, srcFrames );
		return m;
	}

	//x dstFrames = (uint32_t)(((uint64_t)srcFrames * (uint64_t)dstRate + (srcRate-1)) / (uint64_t)srcRate);
	dstFrames = (uint32_t)(((uint64_t)srcFrames * (uint64_t)dstRate) / (uint64_t)srcRate);
	sz = dstFrames * 2;

	printf("\tレート変換 %d:%dframes->%d:%dframes\n", srcRate, srcFrames, dstRate, dstFrames );

	// 実際の変換
	if (a->chnl == 1) {
		dst = calloc(1, sz+0x10000);
		if (dst == 0) {
			printf("%s:ERROR:メモリ不足\n", name);
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
			printf("%s:ERROR:メモリ不足\n", name);
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


/** 説明表示
 */
static int usage(void)
{
	printf("usage> aifwavcv [-opts] file(s)\n");
	printf(" aif,wavファイル を 相互変換, ヘッダ表示.\n"
		   "  -i           情報表示(デフォルト)\n"
		   "  -wav         wav 出力\n"
		   "  -aif         aif 出力\n"
		   "  -s[DIR]      入力ディレクトリ\n"
		   "  -d[DIR]      出力ディレクトリ\n"
		   "  -o[FNAME]    1ファイル変換時の出力名\n"
		   "  -u           出力先ファイルが存在すれば日付比較で新しい時のみ出力\n"
		   "  -rs[SMP]     ヘッダのサンプル情報を SMP に書き換える\n"
		   "  -rc[SMP]     SMP サンプルにデータ変換\n"
		   //"  -rr[SMP]     SMPは44100か48000のみ.その系列で満し近いサンプル数にデータ変換\n"
		   "  -lrate[R]    ループ指定で用いるサンプルレートを指定(データ自体でなく数値の)\n"
		   "  -lps[START]  ループ開始位置(サンプル数)を挿入\n"
		   "  -lpe[END]    ループ終了位置(サンプル数)を挿入\n"
		   "  -lpa         全体ループ指定\n"
		   "  -pcm16l:SMP:CH  CHチャンネル SMP Hz の符号付16bit(LE) pcm を入力\n"
		   "  -pcm16b:SMP:CH  CHチャンネル SMP Hz の符号付16bit(BE) pcm を入力\n"
		   "  -pcm8:SMP:CH    CHチャンネル SMP Hz の符号無8bit pcm を入力\n"
		   " 各フォーマット対応は不完全な限定的なもの.\n"
		   " 無圧縮, 1ch/2ch, 16bit, レート11025/22050/44100/12000/24000/48000 を想定.\n"
		   //" (wav,aif 同士では8bit可能. wav 同士では他のレートも可能)\n"
	);
	return 1;
}

