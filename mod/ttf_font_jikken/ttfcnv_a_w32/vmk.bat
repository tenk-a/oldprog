rem visual-c++
rem cl -W3 -I. -Yd -GX -Zi ttfcnv_a.cpp
cl -W3 -I. -Ox ttfcnv_a.cpp >err.txt
type err.txt
