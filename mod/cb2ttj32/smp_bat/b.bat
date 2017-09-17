set TTF=jpnzn24t.ttf
set SRC=jpnzn24x.x11
set NAME="jpnzn24.x11.smp1"
set JNAME="jpnzn24.x11.smp1"

fnx2dyna -m0 %SRC% %1 -otmp.dyn
freeman  tmp.dyn  tmp.fre
vector   tmp.fre tmp.vec
spline   tmp.vec tmp.spl
toglyf   tmp.spl tmp.glf
rem add %TTF% tmp.glf
