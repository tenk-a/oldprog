tcc -mc -O -G -d -DWILDC -DFBUF tlcut.c tree.c direntry.obj wildcmp.obj
echo off
rem TC 以外でｺﾝﾊﾟｲﾙする場合は, -DWILDC direntry.obj wildcmp.obj を指定しない
rem (ﾜｲﾙﾄﾞｶｰﾄﾞﾙｰﾁﾝを使わなくする)
rem スモール・モデルでコンパイルする場合は -DFBUF を指定しない
