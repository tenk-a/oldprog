/*
  ＰＣ８８ＶＡ＆ＰＣ９８０１用テキスト関係の手続き

　画面は 80*25 または 80*20 で、座標は左上(0,0)-右下(79,24(19)).
  裏テキストＶRAMはサポートしていない。
  カラーは 0～7 で 98のアトビュレートで指定。

  このライブラリは PC88VA,PC98 両方で実行可能なコマンドを作成することを
  考慮しています。つまり、その分助長になっており、そんなことせずどちらか
  片方のみのばあいにくらべてスピードやサイズは不利ですが、asm で書いたの
  でＣで同様のものを造ることを思えば、まあ無視できる範囲と思います。
  （単にVAの資料なくて、でもって VA が98の int 18h をサポートしてくれて
  　いたのでとりあえずこれで間に会わそうと... PC98 対応はついでです^_^）

  PC88VAと PC98 のみの対応ですのでこれら以外の機種での実行は考慮していま
  せん.

 　関数の引き数にかかれた __PPP__ は無視して空白と思ってください。
 */
/*
  　このライブラリの関数名, 変数名の付けかかたは、Cn_Init() や Cn_flg
  のように
    グループ名 + '_' + 名前 のようになります。
  グループ名は大文字で始まり英数小文字が続く比較的短い名前です。
  後に続く名前は、
    関数名のばあい：英大文字で始まり英数小文字が続き、場合によって大文字
    	    	    でわかりやすいように区切る. 例) Cn_PutStr()
    変数名のばあい：英小文字で始まり英数小文字が続き、場合によって大文字
 　 	    	    でわかりやすいように区切る. 例) Cn_x, Cn_flg
  のようになります。
 */

#ifndef __VA98_H__
#define __VA98_H__

#ifdef __PPP__
    #undef __PPP__
#endif

#ifdef LSI_C
    #define __PPP__ ,...
#else
    #define __PPP__
#endif

/******************************   基本	 *************************************/
extern unsigned char Cn_mode;

int  Cn_Init(void);
/*
    このライブラリを利用するに当たって一番最初に一度だけ指定する.
    VA か 98 かを判別し、 次の値を返す。
    	0: VA以外(98)
    	2: VA
    また、この値は大域変数 Cn_flg に設定され、必要なルーチンが参照する。
    ユーザは Cn_flg を参照するのはよいが書き換えてはいけない。

    PC88VA では MSE のモードを拾得し, その値を保存してから VAスタンダード
    モードに切り換える. ただし、MSEのモードが VAネイティブ・モードの場合
    は al = 1 でdos に復帰.

*/

void Cn_Term(void);
/*
    ライブラリの使用をやめる。
    PC88VA モードでは MSE のモードを元に戻す。
    98 モードでは何もしない。
 */

/******************************   テキスト画面	 *****************************/
extern int Cn_x, Cn_y;
extern unsigned char Cn_chrAttr;

void Cn_ConInit(int mode __PPP__);
/*
    int  mode;	0:98ﾉｰﾏﾙ  1:98ﾊｲﾚｿﾞ  2:PC88VA

    テキスト画面操作関数の初期化。
    このライブラリを使用するにあたって最初に実行。
    	各種変数・モードの初期化。
    	画面モード    80*25モード  座標範囲(左上-右下) (0,0)-(79,24)
    	ｱﾄﾋﾞｭﾚｰﾄ      白
    	カーソル位置  (x,y) = (0,0)
    	カーソル表示  Off
    	テキスト表示  on
    	(VA ではテキスト・アクセス・モード)
    なお、引数の mode はテキストＶRAM の種類を指定する。
　　カーソル位置やアトビュレート値や各種モードを自前で管理しているので、
　　このライブラリの関数を使用中に他のライブラリの関数(printfとかcprintf)
    で画面表示などを行うと管理している値と現在の表示とにずれが生じるので
    (画面表示がおかしくなるでせう)、なるべく混用はさけてください。
 */

void Cn_TextOn(void);
/*
    テキスト表示on
 */

void Cn_TextOff(void);
/*
    テキスト表示off
 */

void Cn_Text20(void);
/*
    縦行数を20行にし、画面をクリアする。カーソル位置を(0,0)に。
 */

void Cn_Text25(void);
/*
    縦行数を25行にし、画面をクリアする。カーソル位置を(0,0)に。
 */

void Cn_FuncKeyOn(void);
/*
    ファンクション・キーを表示
 */

void Cn_FuncKeyOff(void);
/*
    ファンクション・キーを非表示
 */

void Cn_CurOn(void);
/*
    カーソル表示on
 */

void Cn_CurOff(void);
/*
    カーソル表示off
 */

void Cn_CurBlnkOn(void);
/*
    カーソルを点滅する
 */

void Cn_CurBlnkOff(void);
/*
    カーソルを点滅しない
 */

void Cn_PosXY(int x, int y __PPP__);
/*
    カーソル位置を(x,y)にする
 */

unsigned Cn_GetPos(void);
/*
    復帰値 x * 0x100 + y
    x : 0～79
    y : 0～25
 　 また、大域変数 Cn_x,Cn_y にも設定される.
    Cn_x,Cn_y は Cn_GetPos 実行直後でないと正しい値を示さない.
 */

unsigned Cn_ResetPos(void);
/*
    printf など他の手続きでコンソール出力したあとで、カーソル位置を調整
    する。
    復帰値はCn_GetPos と同じ。
 */

void Cn_Color(int c __PPP__);
/*
    int c   カラー 0～7
    文字のアトビュレートのカラーを設定
 */

#define Cn_GetColor()	(Cn_chrAttr >> 5)
/*
int  Cn_GetColor(void);
    現在の文字アトビュレートのカラーを拾得
 */

void Cn_SetAttr(unsigned attr __PPP__);
/*
    文字のアトビュレートを設定
    値は 98 のもの
 */

#define Cn_GetAttr()	(Cn_chrAttr)
/*
unsigned Cn_GetAttr(void);
    現在の文字アトビュレートを拾得
 */

void Cn_SetCrtMode(int mode __PPP__);
/*
    bit0 画面行数     0:25  1:20
    bit1 行文字数     0:80  1:40
    bit2 ｱﾄﾋﾞｭﾚｰﾄ     0:垂線表示  1:簡易グラフ
    bit3 K-CGｱｸｾｽﾓｰﾄﾞ 0:ｺｰﾄﾞｱｸｾｽ  1:ﾄﾞｯﾄｱｸｾｽ
 */

void Cn_GetCrtMode(void);
/*
    bit0 画面行数     0:25  1:20
    bit1 行文字数     0:80  1:40
    bit2 ｱﾄﾋﾞｭﾚｰﾄ     0:垂線表示  1:簡易グラフ
    bit3 K-CGｱｸｾｽﾓｰﾄﾞ 0:ｺｰﾄﾞｱｸｾｽ  1:ﾄﾞｯﾄｱｸｾｽ
    bit7 CRTの種別    0:標準CRT   1:高解像度CRT
 */

int Cn_GetBlock(void far *buf,int x,int y,int w, int h __PPP__);
/*
    テキストVRAMの指定範囲の文字とアトビュレートを buf にコピーする。
    範囲は座標(x,y)の位置の横 w 文字, 縦 h 文字。
    テキストVRAMをそのままコピーしたあと、アトビュレートをコピー。
    buf は w * h * (2+2) バイト以上確保しておくこと。
    復帰値は正常なら 0, 指定のおかしいとき 0以外を帰す。
 */

int Cn_PutBlock(void far *buf,int x, int y, int w, int h __PPP__);
/*
    テキストVRAMの指定範囲にbuf の内容をコピーする。
    buf の内容は Cn_GetBlock() で拾得したもの。
    復帰値は正常なら 0, 指定のおかしいとき 0以外を帰す。
 */

int Cn_ScrlUpY(int y,int n __PPP__);
/*
    テキストVRAMのy行目からn 行の範囲を１行スクロール・アップする
    復帰値は正常なら 0, 指定のおかしいとき 0以外を帰す。
 */

int Cn_ScrlDownY(int y,int n __PPP__);
/*
    テキストVRAMのy行目からn 行の範囲を１行スクロール・ダウンする
    復帰値は正常なら 0, 指定のおかしいとき 0以外を帰す。
 */

void Cn_PutChr(unsigned chr __PPP__);
/*
    現在のカーソル位置に文字chrを表示し、カーソル位置を一つ進める
    chr が 0～0xff までなら ANK, 0x100以上ならシフトＪＩＳとして全角を表示
    コントロールコードや不適文字の処理を一切行わないので注意。
 */

void Cn_PutStr(unsigned char *str __PPP__);
/*
    現在のカーソル位置に文字列str を表示し、文字列の終わりの次にカーソル位置を
    進める。
    chr が 0～0xff までなら ANK, 0x100以上ならシフトＪＩＳとして全角を表示
    コントロールコードや不適文字の処理を一切行わないので注意。
 */

void Cn_PutStrFar(unsigned char far *str __PPP__);
/*
    Cn_PutStr() の far pointer 版
 */


/*******************************   キー入力   *******************************/

unsigned Cn_KeyInit(void);
/*
    キー入力手続きの初期化
 */

unsigned Cn_GetKey(void);
/*
    ９８のキーコードを入力(待たない)
 */

unsigned Cn_GetKeyWait(void);
/*
    ９８のキーコードを入力(キーが押されるまで待つ)
 */

unsigned Cn_GetShift(void);
/*
    シフト・キーの状態を拾得
 */

#endif /* __VA98_H__ */


/******************************  VA  ****************************************/
/* MSE : MSE の ﾓｰﾄﾞ */
#define MSE_PC9801  -2
#define MSE_NON     -1
#define MSE_NATIVE  0
#define MSE_STD     1
#define MSE_TEXT       2
#define MSE_TEXTK   3
#define MSE_MONO    4
#define MSE_GRPH       5

int Va_GetMseMode(void);
/*
    PC88VA の現在の MSE のモードを返します。
    PC98モードのときは 98グラフィック・モードを返します。
 */

void Va_SetMseMode(int mode __PPP__);
/*
    PC88VA のモードを変更します。
    Pc98モードでは何もしない。
 */

void Va_PutStr(unsigned char far *s);
/*
    PC88VA のテキストBIOSを使って文字列を表示。
 */

int  Va_ChkVA(void);
/*
    VA なら 1 を、以外なた 0 を返す
 */

