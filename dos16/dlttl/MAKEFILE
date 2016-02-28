CC = tcc
ASM = asm
CFLAGS = -O -d -G -ml
AFLAGS = /ML

.c.obj:
	$(CC) $(CFLAGS) -c $<
.asm.obj:
        $(ASM) $(AFLAGS) $*,$*,NUL,NUL

#
OBJ=nifdlttl.obj jstrcnv.obj jstruplo.obj jstrtrim.obj wildcmp.obj direntry.obj
nifdlttl.exe : $(OBJ)
	tcc $(CFLAGS) $(OBJ)
#
nifdlttl.obj : nifdlttl.c
jstrcnv.obj  : jstrcnv.c
jstruplo.obj : jstruplo.c
jstrtrim.obj : jstrtrim.c
wildcmp.obj  : wildcmp.asm
direntry.obj : direntry.asm
