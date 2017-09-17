# TC(++)�ŃR���p�C�����邽�߂� makefile
ASM = asm /ML
CC = TCC -G -O -ms -d -DDIRENT -DFDATE

.c.obj:
	$(CC) -c $<
.asm.obj:
	$(ASM) $*,$*,NUL,NUL
#
OBJ = pihdr.obj rdmaghdr.obj direntry.obj wildcmp.obj
pihdr.exe : $(OBJ)
	$(CC) $(OBJ)
#
pihdr.obj    : pihdr.c    pihdr.h
rdmaghdr.obj : rdmaghdr.c pihdr.h
direntry.obj : direntry.ana
wildcmp.obj  : wildcmp.ana
