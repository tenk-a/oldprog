echo [ERROR] >err
gcc -O -DX68K picsv.c -lfloatfnc -ldos >>err
type err
