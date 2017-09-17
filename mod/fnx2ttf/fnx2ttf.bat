@echo off

set TTF=jpnzn24x11.ttf
set SRC=jpnzn24x.x11
set NAME="jpnzn24x11 sample"
set JNAME="jpnzn24x11 �T���v��"

rem -m0 �Ȃ�P���Ȋg��.  -m1 �Ȃ�A������ƕό`.
echo fnx2dyna -m0 %SRC% %%1 -otmp.dyn > loop.bat
echo freeman  tmp.dyn  tmp.fre >> loop.bat
echo vector   tmp.fre tmp.vec >> loop.bat
echo spline   tmp.vec tmp.spl >> loop.bat
echo toglyf   tmp.spl tmp.glf >> loop.bat
echo add %TTF% tmp.glf >> loop.bat

ttfstub %TTF% %NAME% %JNAME%
batch loop.bat 0 8835
engpatch %TTF%
