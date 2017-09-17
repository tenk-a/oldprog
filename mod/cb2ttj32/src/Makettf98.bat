@echo off


set TTF=98ank.ttf
set SRC=ank.cg
set NAME="98 ANK"
set JNAME="98 ANK"


echo @echo off > loop.bat
echo 98cgbmp %SRC% %%1 tmp.bmp 0 >> loop.bat
echo freeman  tmp.bmp tmp.fre >> loop.bat
echo vector   tmp.fre tmp.vec >> loop.bat
echo spline   tmp.vec tmp.spl >> loop.bat
echo toglyf   tmp.spl tmp.glf >> loop.bat
echo add %TTF% tmp.glf >> loop.bat


ttfstubank %TTF% %NAME% %JNAME%
batch loop.bat 0 256
engpatch %TTF%



set TTF=98gothic.ttf
set SRC=kanji.cg
set NAME="98 Gothic"
set JNAME="98 ƒSƒVƒbƒN"


echo @echo off > loop.bat
echo 98cgbmp %SRC% %%1 tmp.bmp 1 >> loop.bat
echo freeman  tmp.bmp tmp.fre >> loop.bat
echo vector   tmp.fre tmp.vec >> loop.bat
echo spline   tmp.vec tmp.spl >> loop.bat
echo toglyf   tmp.spl tmp.glf >> loop.bat
echo add %TTF% tmp.glf >> loop.bat


ttfstub %TTF% %NAME% %JNAME%
batch loop.bat 0 8836
engpatch %TTF%
