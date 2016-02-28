/*
	Pi 展開
		writen by M.Kitamura(Tenk*)
		1993/12/11
 */

typedef struct GF {
	//char dummy[16];		// セグメント境界調整用
	/*-------------*/
	int  pln;				// 色のビット数(4 or 8)
	int  xsize;				// 横幅(ドット)
	int  ysize;				// 縦幅(ドット)
	int  xstart;			// 始点x
	int  ystart;			// 始点y
	int  aspect1;			// アスペクト比x
	int  aspect2;			// アスペクト比y
	int  overlayColor;		// 透明色
	int  color;				// パレットの使用個数
	int  palBit;			// パレットのビット数
	int  exSize;			// pi拡張領域サイズ
	int  dfltPalFlg;		// piデフォルトパレットフラグ
	int  loopMode;			// piループモード(pimkの実験/隠しTAG)
	int  filHdl;			// ファイル・ハンドル
	BYTE saverName[4+2];	// 機種名
	BYTE artist[18+2];		// 作者名
	/*-------------*/
	char dummy2[256-(14*2+6+20)];
	/*-------------*/
	char palette[256*3];
	char fileName[254+2];
	char comment[2048+510+2];
} GF;

/*- PI ----------------------------------------------------------------------*/
GF far * far PI_Open(void far *buf, char far *fileName);
 /* fileNameをopen して、ヘッダを読み込む. bufで使用するメモリを指定.
	ﾒﾓﾘは呼び出し側で事前に確保しておくこと.
	必要ﾒﾓﾘは64Kﾊﾞｲﾄ.
	GF構造体をアクセスする場合は復帰値のポインタで行なうこと。
	256色画のﾛｰﾄﾞではさらに64Kﾊﾞｲﾄ確保してPI_SetC256bufで設定する必要がある
	ret:構造体へのポインタ(ｵﾌｾｯﾄ0) ｾｸﾞﾒﾝﾄが0ならｴﾗｰ(ｵﾌｾｯﾄ値がｴﾗｰNo)
  */

void far PI_SetC256buf(unsigned bufseg);
 /* 256色展開時に必要な64Kﾊﾞｲﾄ色表のﾒﾓﾘを指定.
	ﾒﾓﾘは事前に確保すること. PI_Load を実行する前に設定すること.
	ret ax:error no
  */

int far PI_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
 /* Pi画像の展開. １行出力ﾙｰﾁﾝを渡す
	ret ax:error no
  */

int far PI_Close(void far *gf);
 /* Pi ﾌｧｲﾙのclose
	gf の解放はユーザが行なうこと
	ret ax:error no
  */

/*- MAG ---------------------------------------------------------------------*/
GF far * far MAG_Open(void far *gf, char far *fileName);
 /* fileNameをopen して、ヘッダを読み込む. gf で使用するメモリを指定.
	ﾒﾓﾘは呼び出し側で事前に確保しておくこと.
	必要ﾒﾓﾘは128Kﾊﾞｲﾄ.
	GF構造体をアクセスする場合は復帰値のポインタで行なうこと。
	ret:構造体へのポインタ(ｵﾌｾｯﾄ0) ｾｸﾞﾒﾝﾄが0ならｴﾗｰ(ｵﾌｾｯﾄ値がｴﾗｰNo)
  */

int far MAG_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
 /* MAG画像の展開. １行出力ﾙｰﾁﾝを渡す
	ret ax:error no
  */

int far MAG_Close(void far *gf);
 /* MAG ﾌｧｲﾙのclose
	gf の解放はユーザが行なうこと
	ret ax:error no
  */
/*- PMT & DJP ---------------------------------------------------------------*/
GF far * far PMT_Open(void far *gf, char far *fileName);	// gf:要64Kﾊﾞｲﾄ
GF far * far DJP_Open(void far *gf, char far *fileName);	// gf:要64Kﾊﾞｲﾄ
int far DJP_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
int far DJP_Close(void far *gf);
void far DJP_AutoChk16(int sw);
	 /* パレット 16～255 が全て 0なら１６色画像としてあつかう */
/*- Q0 ---------------------------------------------------------------------*/
GF far * far Q0_Open(void far *gf, char far *fileName);		// gf:要64Kﾊﾞｲﾄ
int far Q0_Close(void far *gf);
/*- BMP ---------------------------------------------------------------------*/
GF far * far BMP_Open(void far *gf, char far *fileName);	// gf:要64Kﾊﾞｲﾄ
int far BMP_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
int far BMP_Close(void far *gf);
