echo [ERROR] >err
gcc -O -DX68K picld.c -lfloatfnc -ldos >>err
type err
