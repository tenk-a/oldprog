 *- コンパイル方法 ------------------------------------------------------------
 * LSI-C V3.2試食版 : lcc vhload.c gl98.c -lnoexpand.obj -k/stack:20000
 * TC      V2.0c    : tcc -O -G -w-par vhload.c gl98.c dos_read.c dos_find.c
 *- VA ------------------------------------------------------------------------
 * lcc -DPC88VA vhload.c glva.c vag.c msemode.c noexpand.obj -k/stack:20000
