# LSI-C86試食版でコンパイルするための makefile
ASM = r86
CC = lcc -O -DDIRENT -DFDATE

.c.obj:
	$(CC) -c $<
.a86.obj:
	$(ASM) $<
#
OBJ = pihdr.obj rdmaghdr.obj direntry.obj wildcmp.obj
pihdr.exe : $(OBJ)
	$(CC) $(OBJ) -lnoexpand.obj
#
pihdr.obj    : pihdr.c    pihdr.h
rdmaghdr.obj : rdmaghdr.c pihdr.h
direntry.obj : direntry.a86
wildcmp.obj  : wildcmp.a86
