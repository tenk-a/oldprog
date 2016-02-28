@echo off
gcc -v picsv.c -o picsv32.out
coff2exe picsv32.out
del picsv32.out
