rem msdos ana -q1 -c -o -1 %1.ana
ana -q1 -c -o -1 %1.ana
wasm -1 -j -ml -fo=%1.obj %1.asm
