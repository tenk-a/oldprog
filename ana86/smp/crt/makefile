# rules
CC = TCC
CPP = CPP
ASM = TASM
ANA = ANA
AFLAGS = -Ml
CFLAGS =
ANAFLAGS =
CPPFLAGS =
.ana.obj:
	$(ANA) $(ANAFLAGS) -o$*.asm $<
	$(ASM) $(AFLAGS) $*.asm,,,;
	del $*.asm
.ana.asm:
	$(ANA) $(ANAFLAGS) -o $<
.asm.obj:
	$(ASM) $(AFLAGS) $<
.c.obj:
	$(CC) $(CFLAGS) -c $<
#
#
#
OBJ = tst.obj va98.obj key98.obj  va.obj

tst.exe : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ)
#
#
tst.obj   : tst.c
va98.obj  : va98.ana
key98.obj : key98.ana
va.obj    : va.ana
