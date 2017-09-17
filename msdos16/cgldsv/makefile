# rules
CC = TCC
ASM = asm
AFLAGS = /ML
CFLAGS = -G -O -ms -d

.c.obj:
	$(CC) $(CFLAGS) -c $<
.asm.obj:
	$(ASM) $(AFLAGS) $*,$*,NUL,NUL
#
OBJ = cgldsv.obj direntry.obj wildcmp.obj
pihdr.exe : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ)
#
cgldsv.obj   : cgldsv.c
direntry.obj : direntry.asm
wildcmp.obj  : wildcmp.asm
