/*
    ＦＢ用のポート周辺のヘッダ定義
    fb.cと一緒に使うこと。
    ここで、注意として；
    	（１）FB_init();を実行すること。
    	（２）書き込みはＸＹＲＧＢの全てを行うことを前提とすること
    この２つに気をつければ、ＨＦ／ＳＦは初期化・特殊効果を除いて
    まったく同じ様に動作する＜はず＞である。
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    * 92/06/22	FB_Pxxxxの形の定義をFBP_xxxxに変えた
    * 93/01/06	FB_xxxx();の関数を#defineでFBxxxxになるようにした。
*/

/*------    以下は fb.pas をインプリメントする時に出来たもの	*/
/*  (*----- ＳＦ／ＨＦ　Ｉ／Ｏポート定義 -----*)    */

#define FBP_cont0   	0xd0
#define FBP_cont1   	0xd2
#define FBP_contHF0 	0xd3	    /*	ＨＦにしかない特殊なポート  */
#define FBP_Xreg    	0xd4
#define FBP_Yreg    	0xd6
#define FBP_RedReg  	0xd8
#define FBP_GreReg  	0xda
#define FBP_BluReg  	0xdc
#define FBP_ModReg  	0xde
#define FBP_BnkReg0 	0xec
#define FBP_BnkReg1 	0xee

/*  (*----- ＳＦモードレジスタ定義 (0DEh) -----*)   */
#define PCONLY	    0x00
#define BOTH	    0x01
#define FBONLY	    0x03    	/* ＳＦ２ΣではＰＣ画面をカット出来ない */
#define FBOFF	    0x04    	/* HF+の拡張。ポートが当たるのを防ぐ	*/

#define SF_STEALWR  0x80    	/* スチールライト */
#define SF_AUTOINC  0x40    	/* Ｘ軸のオートインクリメント */
#define SF_GRAYMODE 0x08

/*  (*----- ＨＦモードレジスタ定義 (0D0h) -----*)   */
#define HF_IOXRAM   0x01    	/* アクセス方法 0:BANK 1:I/O	*/
#define HF_WRMASK   0x02    	/* ライトマスク 0:OFF  1:ON 	*/
#define HF_RGBWR    0x04    	/* RGB同時書込	0:OFF  1:ON 	*/
#define HF_AUTINC   0x08    	/* BANK自動inc	0:OFF  1:ON 	*/
#define HF_CGX98    0x10    	/* HF画面表示	0:98   1:HF 	*/
#define HF_IMPOSE   0x20    	/* インポーズ	0:OFF  1:ON 	*/
#define HF_EXTON    0x40    	/* レジスタ使用 0:不可 1:可 	*/
#define HF_XSFTRST  0x80    	/* ソフトリセット   	    	*/

#define NoFrameBuf  0
#define SuperFrame  1
#define HyperFrame  2

/*=============================================================================
    ＨＦレジスタ関連、さらに定義    （ＨＦ＋独特の拡張）
=============================================================================*/
#define HF_REG_BASEX	0x80
#define HF_REG_BASEY	0x193
#define HF_CUR_OFFX 	150
#define HF_CUR_OFFY 	31

#define HF_REG_SCROLLX	4
#define HF_REG_SCROLLY	6

/*=============================================================================
    inline assemblerを使った高速なポート出力
=============================================================================*/
/*  tccの吐いたソースを見たらcallを生成したのでびっくりして作ったマクロ */
/*  使ったら、必ず#pragma inlineを宣言すること	    	    	    	*/

#define ioutw(l,m) {\
    _AX=(m);\
    asm out l,ax;\
}

#define ioutb(l,m) {\
    _AL=(m);\
    asm out l,al;\
}

#define FBInit()    FB_Init()
#define FBMode(x)   FB_Mode((x))
#define FB_Clear()  FBClear()
