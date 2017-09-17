#define VV_SIZE 0x4000

typedef struct VV {
    int pln;	//4:16	8:256色
    int xsize;	    	//画像横幅
    int ysize;	    	//    縦幅
    int xstart;     	//表示開始座標	X
    int ystart;     	//  	    	Y
    int xgscrn;     	//画面横幅
    int ygscrn;     	//    縦幅
    int xgsize;     	//実際に表示するときの横幅  xxgsize
    int ygsize;     	//  	    	    	    ygsize
    int xgstart;    	//画面での実際の表示開始座標  X
    int ygstart;    	//  	    	    	    　Y
    int xvsize;     	//仮想VRAM横幅
    int yvsize;     	//  	  縦幅
    int xvstart;    	//仮想VRAMの表示(転送)開始位置(ｽｸﾛｰﾙしたときの) X
    int yvstart;    	//  	    	    	    	    	    	Y
    //-------------------------------------------------------
    unsigned char far *palette;     	//パレット・テーブルへのアドレス
    int aspect1;    	//ドットの横比
    int aspect2;    	//ドットの縦比
    int overlayColor;	//背景色

    int ycur;	    	//PutLine での現在の Y 座標
    int l200flg;    	//いわゆる 200L 画像か
    int x68flg;     	//アスペクト比が x68 の場合か
    int reductFlg;  	//縮小モード	 0x02:1/4  0x06:1/16
    int reductFlgY; 	//縮小モード時のYスキップ用マスク
    int ycurReduct; 	//1/16縮小モードでのPutLine での現在の Y 座標
    int loopFlg;    	//ずりずりループ・モードか
    int maxLine;    	//展開ﾙｰﾁﾝが展開できる最大行数
    //-------------------------------------------------------
    int pgLine;     	//ページ当りの行数
    int pgNo;	    	//ページ番号
    int pgCnt;	    	//必要ページ数
    unsigned pgCurSeg;	//現在のページのセグメント
    int pgMmSeg;    	//ページに使うメインメモリの先頭
    unsigned pgMmSzs;	//ページに使えるメイン・メモリのパラグラフ数
    int pgMmCnt;    	//ページ数
    int pgEmsHdl;   	//EMS のハンドル
    unsigned pgEmsSeg;	//EMS のセグメント
    int pgEmsCnt;   	//EMS のページ数
    int pgEmsNo;    	//EMS の現在のページ番号
    int pgMaxLine;  	//最大行数
    int pglcnt;     	//行カウンタ
    int pgPtr;	    	//ページ中のポインタ
    int pgXsize;    	//１行のサイズ(バイト数)
    int pgXsizeSubOfs;	//=xxsize*3-1(xxsize*7-1)
    //-------------------------------------------------------
    // 8ドット間隔の座標
    int xxvsize;    	//仮想VRAM横幅
    int xofsL;
    int xofsR;
    int xmskL;
    int xmskR;
    int xmskFlg;
    unsigned char  bcolptn[8];	//背景色のパターン
    //-------------------------------------------------------
    int ditFlg;     	//減色するかどうか
    int gvOfs;	    	//ズリズリで使う(表示位置の調整)
    //--------------------------------------------------
    unsigned char dummy[10/*8*/];
    unsigned char pbuf[1280*4+2];
    unsigned char pbuf2[1280*4+10];
} VV;

typedef struct VVF {
    int  no;
    WORD memSize;
    int  (*Init)(   int vvSeg, int mmSeg, unsigned mmSzs,
    	    	    int emsHdl, unsigned emsSeg, unsigned emsCnt,
    	    	    int pln, int xsz,int ysz,int xstart,int ystart,
    	    	    unsigned char far *pal,int asp1,int asp2,
    	    	    int bcol,int reductFlg,int loopFlg);
    void (*Start)(void);    	    	//プログラム開始時の処理
    void (*End)(int tone);  	    	//プログラム終了時のあと後かたづけ
    void (*GStart)(void);   	    	//グラフィック開始
    void (*GEnd)(void);     	    	//グラフィック終了
    void (*GCls)(void);     	    	//グラフィック・クリア
    void (*ShowOn)(void);   	    	//grphic display on
    void (*ShowOff)(void);  	    	//grphic display off
    void (*SetTone)(int tone);	    	//トーン変更
    void far (*GetLines)(void far *dat,unsigned char far *buf);
    	    	    	    	    	//  ピクセル・データ入力 4,8ビット色
    void far (*GetLine24)(void far *dat,unsigned char far *buf);
    	    	    	    	    	//  ピクセル・データ入力 24ビット色
    void (*RevX)(unsigned Seg);     	//横反転
    void (*RevY)(unsigned Seg);     	//縦反転
    void (*SetPal)(unsigned Seg,int tone);//パレット設定
    void (*PutPx)(unsigned Seg,int gx,int gy,int x0,int y0,int xsz,int ysz);
    	    	    	    	    	//実画面に仮想VRAMの内容を表示
    void (*PutPxScrn)(unsigned Seg,int x0,int y0);
    	    	    	    	    	//スクリーンサイズ表示
    void (*PutPxPart)(unsigned Seg);	//始点付画像表示
    void (*PutPxBCol)(unsigned Seg);	//透明色対応表示
    void (*PutPxLoop)(unsigned Seg);	//kabe
    //-------------------------
    void (*SclLoopMode)(unsigned Seg,int mode);
    	    	    	    	    	//0:ズリズリでループしない 1:する
    void (*SclUp)(unsigned Seg, int d); //上スクロール
    void (*SclDw)(unsigned Seg, int d); //下スクロール
    void (*SclLft)(unsigned Seg, int d);//左スルロール
    void (*SclRig)(unsigned Seg, int d);//右スクロール
    unsigned long(*GetSclOfs)(unsigned Seg);//VRAM先頭からの画面表示位置のｵﾌｾｯﾄ
} VVF;

void Vv_Non(void);
void Vv_RevY(unsigned Seg);
void Vv_SclLoopMode(unsigned Seg,int flg);
void Vv_PriInfo(unsigned Seg);
void far Vv_GetLine24(void far *dat ,unsigned char far *buf);
