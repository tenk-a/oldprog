@echo off
if exist %1 goto RU
echo usage: bug2mag errmag[.mag]
echo 	v0.98以前のmgが誤生成した 256色MAGを生成しなおす.
echo 	指定するﾌｧｲﾙは誤生成256色MAG画像のみで正常な画像を指定しないこと.
goto :END
:RU
mg -mb .mag.pmt %1
mg -skPMT .pmt.mag %1
del *.pmt
:END
