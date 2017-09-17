/*
    ����wav�w�b�_�\��
 */

#include "subr.h"



/*-------------------------------------------------------------------------*/

volatile void Usage(void)
{
    puts(
    	"usage> wav_hdr [-opts] file[.wav]   // v0.02   " __DATE__ "  by tenk*\n"
    	"  WAV�t�@�C���̃w�b�_����\������\n"
    	" -diDIR    ���̓f�B���N�g��\n"
    	" @RESFILE  ���X�|���X�t�@�C������\n"
    	);
    exit(1);
}




/*---------------------------------------------------------------------------*/
/* �ϐ�/�^��`	*/

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

    /* �I�v�V�����v�f�̏����� */
    opts_init();

    /* �R�}���h���C����� */
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

    /* �p�����[�^�̐����`�F�b�N */
    if (flist_root == NULL) {
    	printfExit("�t�@�C�������w�肵�Ă�������\n");
    }

    for (i = 0, fl = flist_root; fl; fl = fl->link, i++) {
    	DB printf("%3d %s\n", i, fl->s);
    }

    /* �w��t�@�C�����Ƃɕϊ� */
    for (i = 0, fl = flist_root; fl; fl = fl->link, i++) {
    	char	onm[FIL_NMSZ], inm[FIL_NMSZ];
    	// ���̓t�@�C����
    	onm[0] = inm[0];
    	if (opts.idir) {
    	    sprintf(inm, "%s\\%s", opts.idir, fl->s);
    	} else {
    	    strcpy(inm, fl->s);
    	}
    	FIL_AddExt(inm, "WAV");
      #if 1
    	// �o�̓t�@�C����
    	if (opts.odir) {
    	    sprintf(onm, "%s\\%s", opts.odir, fl->s);
    	} else {
    	    strcpy(onm, fl->s);
    	}
    	FIL_ChgExt(onm, "PCM");
      #endif
    	// �ϊ�
    	//printf("%s -> %s\r", inm, onm);
    	OneFile(inm, onm, &opts);
    }
    return 0;
}



/*---------------------------------------------------------------------------*/

void resfile_get(char *name)
{
    /* ���X�|���X�t�@�C���̓ǂݍ��� */
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
    /* �I�v�V�����̏��� */
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
    int16_t 	waveFormatType; //  2	1=���j�APCM(���ʂ�:-)
    int16_t 	channel;    	//  2	1=���m���� 2=�X�e���I
    int32_t 	samplesPerSec;	//  4	�T���v�����O���g��(11025,22050,44100�Ȃ�)
    int32_t 	bytesPerSec;	//  4	=blockSize*samplesPerSec
    uint16_t	blockSize;  	//  2	=bitsPerSample*channe/8
    uint16_t	bitsPerSamples; //  2	�ʎq���r�b�g��. ���� 8��16
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
    	printf("ERROR: %s �� WAV �t�@�C���łȂ�\n", inm);
    	return;
    }
    freeE(wav);
    printf("%-16s(%8x)", inm, isz);
    printf("\t%s", (wh.waveFormatType == 1) ? "���j�APCM" : "�����PCM");
    printf("  %s", (wh.channel == 1) ? "���m����  " : (wh.channel == 2) ? "�X�e���I" : "??????  ");
    printf("  %2dbits", wh.bitsPerSamples);
    printf("  %5dHz  0x%06xbytes", wh.samplesPerSec, sz);
    printf("\n");
}




/*-------------------------------------------------------------------------*/

/*---------------------------------------
 WAVE �t�H�[�}�b�g

 WAV�t�@�C���͏��Ȃ��Ƃ��ȉ��R�u���b�N����Ȃ�
    RIFF�`�����N    12�o�C�g
    fmt�`�����N     24�o�C�g
    data�`�����N    ? �o�C�g

�e�`�����N�͈ȉ��̒ʂ�

RIFF
    chunkID 	    4	'RIFF'
    chunkSize	    4	12+24+sz
    formType	    4	'WAVE'
fmt
    chunkID 	    4	'fmt '
    chunkSize	    4	16
    waveFormatType  2	1=���j�APCM(���ʂ�:-)
    channel 	    2	1=���m���� 2=�X�e���I
    samplesPerSec   4	�T���v�����O���g��(11025,22050,44100�Ȃ�)
    bytesPerSec     4	=blockSize*samplesPerSec
    blockSize	    2	=bitsPerSample*channe/8
    bitsPerSamples  4	�ʎq���r�b�g��. ���� 8��16
data
    chunkID 	    4	'data'
    chunkSize	    4	sz(�f�[�^�T�C�Y. �ϒ�)
    data    	    sz	WAV�f�[�^

�����ȊO�ɂ��`�����N�͂���A�Ƃ���fmt��data
�̊Ԃɕʂ̂��̂����邱�Ƃ̓}�}����A������
�`�����N�T�C�Y���݂ăX�L��������K�v������


-----------------------------------------*/



int getWavFmt(wav_chank_fmt_t *wh, void *wavdata, int *asz)
{
    /* �T�E���h�̐ݒ� */
    uint8_t *b = (uint8_t*)wavdata, *d, *m;
    int     sz;

    if (PEEKmD(b) == BBBB('R','I','F','F')) {
    	b += 12;
    }
    if (PEEKmD(b) != BBBB('f','m','t',' ')) {
    	return -1;
    }
    // ���̎��_�� b �� fmt�`�����N�̐擪�A�h���X
    memcpy(wh, b+8, sizeof *wh);

    // data�`�����N��T��
    do {
    	sz  	    	= *(uint32_t*)(b+4);
    	b   	    	= b + 8 + sz;	    // data�`�����N�̐擪��T��
    } while (PEEKmD(b) != BBBB('d','a','t','a'));
    sz = *(uint32_t*)(b+4);
    *asz = sz;
    return 0;
}



