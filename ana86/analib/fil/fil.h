/*
	module fil
 // dosｺｰﾙそのままの場合は基本的にエラーコードaxは負に反転して返す
*/


/*
 * DOSハンドル系.
 */
const int STDIN=0,STDOUT=1,STDERR=2;
I	fil_open(B F *fname, W mode);
	//ret ax:ﾌｧｲﾙ･ﾊﾝﾄﾞﾙ  負値(-1～-100位^^;):dosｴﾗｰｺｰﾄﾞ
I	fil_close(int hdl);
	//ret ax 0:ok  負値:dosｴﾗｰｺｰﾄﾞ
I	fil_creat(B F *fname, W fattr);
	//ret ax:ﾌｧｲﾙ･ﾊﾝﾄﾞﾙ  負値:dosｴﾗｰｺｰﾄﾞ
I	fil_creatnew(B F *fname, W fattr);
	//ret ax:ﾌｧｲﾙ･ﾊﾝﾄﾞﾙ  負値:dosｴﾗｰｺｰﾄﾞ
W	fil_read(I hdl, B F *buf, W siz);
	//ret ax:実際に読み込んだサイズ（又はdosエラーコード）
	//       -1～-100位:dosエラーコード. だから siz は 63K位までにしとこう
W	fil_write(I hdl,B F *buf, W siz);
	//ret ax:実際に書き込んだサイズ（又はdosエラーコード）
D	fil_seek(I hdl, D ofs, I posmode);
	//arg posmode  0:ﾌｧｲﾙ先頭  1:ｶﾚﾝﾄ位置  2:ﾌｧｲﾙ末
	//ret dx.ax  ﾌｧｲﾙ先頭からのｵﾌｾｯﾄ. -1=エラーがあった
I	fil_dup(I hdl);
	//ret ax:新しいﾊﾝﾄﾞﾙ  負値:dosエラーコード
I	fil_dup2(I hdl1, I hdl2);
	//ret ax:新しいﾊﾝﾄﾞﾙ(hdl2)  負値:dosエラーコード

/*
 * ディスク(ﾃﾞｨﾚｸﾄﾘ(ｴﾝﾄﾘ))関係
 */
I	fil_delete(fname:d);cdecl
	//ret ax:0=ok  負値:dosエラーコード
I	fil_getattr(fname:d);cdecl
	//ret ax:ﾌｧｲﾙ属性   負値:dosｴﾗｰｺｰﾄﾞ
I	fil_setattr(fname:d, attr:w);cdecl
	//ret ax:0=ok   負値:dosｴﾗｰｺｰﾄﾞ
I	fil_getdrive();cdecl
	//	ret ax:0=A: 1=B: ...
I	fil_setdrive(drv:w);cdecl
	//	drv 1=A:  2=B: ...
	//	ret ax:設定された最大ドライブ番号(0=A: 1=B: ...)
I	fil_mkdir(dir:d);cdecl
	//	ret ax:0=ok	負値:dosエラーコード
I	fil_rmdir(dir:d);cdecl
	//	ret ax:0=ok	負値:dosエラーコード
I	fil_setcdir(dir:d);cdecl
	//ｶﾚﾝﾄ･ﾃﾞｨﾚｸﾄﾘの指定(ﾄﾞﾗｲﾌﾞ変更なし)
	//	ret ax:0=ok	負値:dosエラーコード
I	fil_setcwd(dir:d);cdecl
	//ｶﾚﾝﾄ･ﾃﾞｨﾚｸﾄﾘの指定(ﾄﾞﾗｲﾌﾞ変更有り)
	//	ret ax:0=ok	負値:dosエラーコード
I	fil_getcdir(drv:w,dir:d);cdecl
	// 指定ﾄﾞﾗｲﾌﾞのカレント・ディレクトリの取得
	//	drv 0=ｶﾚﾝﾄ  1=A:  2=B: ...
	//	ret ax:0=ok	負値:dosエラーコード
I	fil_getcwd(dir:d);cdecl
	// カレントドライブのカレント・ディレクトリの取得
	//	ret ax:0=ok	負値:dosエラーコード

/*
 * ファイル名文字列関係
 */

endmodule
