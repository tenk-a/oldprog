fnx2dyna -m1 jpnzn24x.x11 %1 -otmp.dyn 
freeman  tmp.dyn  tmp.fre 
vector   tmp.fre tmp.vec 
spline   tmp.vec tmp.spl 
toglyf   tmp.spl tmp.glf 
add jpnzn24m2.ttf tmp.glf 
