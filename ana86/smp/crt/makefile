# rules
CC = TCC
CPP = CPP
ASM = TASM
ANA = ANA
AFLAGS = -Ml -Zi
CFLAGS = -v
ANAFLAGS =
CPPFLAGS =
.asm.obj:
	$(ASM) $(AFLAGS) $<
.ana.asm:
	$(ANA) $(ANAFLAGS) -o $<
.ana.obj:
	$(ANA) $(ANAFLAGS) -o$*.asm $<
	$(ASM) $(AFLAGS) $*.asm,,,;
	del $*.asm
.anp.obj:
	$(CPP) $(CPPFLAGS) -o$*.ana $<
	$(ANA) $(ANAFLAGS) -o $*.ana
	$(ASM) $(AFLAGS) $*.asm,,,;
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
