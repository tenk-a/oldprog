@echo off
gcc -v picld.c -o picld32.out
coff2exe picld32.out
del picld32.out
