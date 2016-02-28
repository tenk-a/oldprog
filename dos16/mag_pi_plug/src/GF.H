/*
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
	int  rsv;				//
	int  filHdl;			// ファイル・ハンドル
	char saverName[4+2];	// 機種名
	char artist[18+2];		// 作者名
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
	必要ﾒﾓﾘは64Kﾊﾞｲﾄ (256色展開時はさらに64Kで 128Kﾊﾞｲﾄ).
	GF構造体をアクセスする場合は復帰値のポインタで行なうこと。
	ret:構造体へのポインタ(ｵﾌｾｯﾄ0) ｾｸﾞﾒﾝﾄが0ならｴﾗｰ(ｵﾌｾｯﾄ値がｴﾗｰNo)
  */

int far PI_Load(void far *gf,
			 void far (*putLine)
				(void far *dat, unsigned char far *lineBuf),
			 void far *putLinDat,
			 int ymax
			 );
 /* Pi画像の展開. １行出力ﾙｰﾁﾝを渡す.
	1行出力ルーチンをＣで書くときは _loadds を指定する必要があるかも
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
	1行出力ルーチンをＣで書くときは _loadds を指定する必要があるかも
	ret ax:error no
  */

int far MAG_Close(void far *gf);
 /* MAG ﾌｧｲﾙのclose
	gf の解放はユーザが行なうこと
	ret ax:error no
  */
