/*
    安易wavヘッダ表示
 */

#include "subr.h"



/*-------------------------------------------------------------------------*/

volatile void Usage(void)
{
    puts(
    	"usage> wav_hdr [-opts] file[.wav]   // v0.02   " __DATE__ "  by tenk*\n"
    	"  WAVファイルのヘッダ情報を表示する\n"
    	" -diDIR    入力ディレクトリ\n"
    	" @RESFILE  レスポンスファイル入力\n"
    	);
    exit(1);
}




/*---------------------------------------------------------------------------*/
/* 変数/型定義	*/

typedef struct opts_t {
    char    *odir;
    char    *idir;
    int     bytSwp;
    int     vflg;
} opts_t;

static opts_t	opts;
static slist_t	*flist_root;


/*-------------------------------------------------------------------------*/
void OneFile(char *inm, char *onm, opts_t *o);
void opts_init(void);
void opts_get(char *a);
void resfile_get(char *name);


int main(int argc, char *argv[])
{
    int     i,filNum;
    char    *p;
    slist_t   *fl;

    if (argc < 2)
    	Usage();

    /* オプション要素の初期化 */
    opts_init();

    /* コマンドライン解析 */
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-') {
    	    opts_get(p);
    	} else if (*p == '@') {
    	    resfile_get(p+1);
    	} else {
    	    slist_add(&flist_root, p);
    	}
    }

    /* パラメータの整合チェック */
    if (flist_root == NULL) {
    	printfExit("ファイル名を指定してください\n");
    }

    for (i = 0, fl = flist_root; fl; fl = fl->link, i++) {
    	DB printf("%3d %s\n", i, fl->s);
    }

    /* 指定ファイルごとに変換 */
    for (i = 0, fl = flist_root; fl; fl = fl->link, i++) {
    	char	onm[FIL_NMSZ], inm[FIL_NMSZ];
    	// 入力ファイル名
    	onm[0] = inm[0];
    	if (opts.idir) {
    	    sprintf(inm, "%s\\%s", opts.idir, fl->s);
    	} else {
    	    strcpy(inm, fl->s);
    	}
    	FIL_AddExt(inm, "WAV");
      #if 1
    	// 出力ファイル名
    	if (opts.odir) {
    	    sprintf(onm, "%s\\%s", opts.odir, fl->s);
    	} else {
    	    strcpy(onm, fl->s);
    	}
    	FIL_ChgExt(onm, "PCM");
      #endif
    	// 変換
    	//printf("%s -> %s\r", inm, onm);
    	OneFile(inm, onm, &opts);
    }
    return 0;
}



/*---------------------------------------------------------------------------*/

void resfile_get(char *name)
{
    /* レスポンスファイルの読み込み */
    static char buf[1024*32-4];
    char *p;

    TXT1_OpenE(name);
    while (TXT1_GetsE(buf, sizeof buf)) {
    	p = strtok(buf, " \t\n");
    	do {
    	    if (*p == '-') {
    	    	opts_get(p);
    	    } else {
    	    	slist_add(&flist_root, p);
    	    }
    	    p = strtok(NULL, " \t\n");
    	} while (p);
    }
    TXT1_Close();
}




/*---------------------------------------------------------------------------*/


void opts_init(void)
{
    opts.odir	    = NULL;
    opts.idir	    = NULL;
    opts.vflg	    = 0;
    opts.bytSwp     = 0;
}


void opts_get(char *a)
{
    /* オプションの処理 */
    char *p;
    int c;

    p = a;
    p++;
    c = *p++;
    c = TOUPPER(c);
    switch (c) {
    case 'V':
    	opts.vflg = (*p == '-') ? 0 : 1;
    	break;
    case 'D':
    	c = *p++;
    	c = TOUPPER(c);
    	switch (c) {
    	case 'I':
    	    opts.idir	    = FIL_DelLastDirSep(strdupE(p));
    	    break;
    	case 'O':
    	    opts.odir	    = FIL_DelLastDirSep(strdupE(p));
    	    break;
    	default:
    	    goto OPT_ERR;
    	}
    	break;

    case 'Z':
    	debugflag   	= (*p == '-') ? 0 : 1;
    	break;

    case '\0':
    case '?':
    	Usage();
    default:
    	goto OPT_ERR;
    }
    return;

 OPT_ERR:
    printfExit("Incorrect command line option : %s\n", a);
}



/*---------------------------------------------------------------------------*/

typedef struct wav_chank_fmt_t {
    //Uint32	chunkID;    	//  4	'fmt '
    //Uint32	chunkSize;  	//  4	16
    int16_t 	waveFormatType; //  2	1=リニアPCM(普通は:-)
    int16_t 	channel;    	//  2	1=モノラル 2=ステレオ
    int32_t 	samplesPerSec;	//  4	サンプリング周波数(11025,22050,44100など)
    int32_t 	bytesPerSec;	//  4	=blockSize*samplesPerSec
    uint16_t	blockSize;  	//  2	=bitsPerSample*channe/8
    uint16_t	bitsPerSamples; //  2	量子化ビット数. 普通 8か16
} wav_chank_fmt_t;


int getWavFmt(wav_chank_fmt_t *wh, void *wavdata, int *asz);


void OneFile(char *inm, char *onm, opts_t *o)
{
    int n,isz,sz;
    char *wav;
    wav_chank_fmt_t wh;

    wav = FIL_LoadE(inm, NULL, 0, &isz);
    n = getWavFmt(&wh, wav, &sz);
    if (n) {
    	printf("ERROR: %s は WAV ファイルでない\n", inm);
    	return;
    }
    freeE(wav);
    printf("%-16s(%8x)", inm, isz);
    printf("\t%s", (wh.waveFormatType == 1) ? "リニアPCM" : "特殊なPCM");
    printf("  %s", (wh.channel == 1) ? "モノラル  " : (wh.channel == 2) ? "ステレオ" : "??????  ");
    printf("  %2dbits", wh.bitsPerSamples);
    printf("  %5dHz  0x%06xbytes", wh.samplesPerSec, sz);
    printf("\n");
}




/*-------------------------------------------------------------------------*/

/*---------------------------------------
 WAVE フォーマット

 WAVファイルは少なくとも以下３ブロックからなる
    RIFFチャンク    12バイト
    fmtチャンク     24バイト
    dataチャンク    ? バイト

各チャンクは以下の通り

RIFF
    chunkID 	    4	'RIFF'
    chunkSize	    4	12+24+sz
    formType	    4	'WAVE'
fmt
    chunkID 	    4	'fmt '
    chunkSize	    4	16
    waveFormatType  2	1=リニアPCM(普通は:-)
    channel 	    2	1=モノラル 2=ステレオ
    samplesPerSec   4	サンプリング周波数(11025,22050,44100など)
    bytesPerSec     4	=blockSize*samplesPerSec
    blockSize	    2	=bitsPerSample*channe/8
    bitsPerSamples  4	量子化ビット数. 普通 8か16
data
    chunkID 	    4	'data'
    chunkSize	    4	sz(データサイズ. 可変長)
    data    	    sz	WAVデータ

これら以外にもチャンクはあり、とくにfmtとdata
の間に別のものがあることはママあり、ちゃんと
チャンクサイズをみてスキャンする必要がある


-----------------------------------------*/



int getWavFmt(wav_chank_fmt_t *wh, void *wavdata, int *asz)
{
    /* サウンドの設定 */
    uint8_t *b = (uint8_t*)wavdata, *d, *m;
    int     sz;

    if (PEEKmD(b) == BBBB('R','I','F','F')) {
    	b += 12;
    }
    if (PEEKmD(b) != BBBB('f','m','t',' ')) {
    	return -1;
    }
    // この時点で b は fmtチャンクの先頭アドレス
    memcpy(wh, b+8, sizeof *wh);

    // dataチャンクを探す
    do {
    	sz  	    	= *(uint32_t*)(b+4);
    	b   	    	= b + 8 + sz;	    // dataチャンクの先頭を探す
    } while (PEEKmD(b) != BBBB('d','a','t','a'));
    sz = *(uint32_t*)(b+4);
    *asz = sz;
    return 0;
}



