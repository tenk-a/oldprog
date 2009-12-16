# rules
CC = BCC
ASM = TASM
AFLAGS = -ml -m
CFLAGS = -ml -d -Ox -G -K
.asm.obj:
	$(ASM) $(AFLAGS) $<
.c.obj:
	$(CC) $(CFLAGS) -c $<
#
#
#
OBJ = ana.obj decl.obj stat.obj expr.obj gen.obj out.obj sym.obj mem.obj

ana.exe : $(OBJ)
	$(CC) $(CFLAGS) -eANA.EXE $(OBJ)
	copy ana.exe c:\cmds\ana.exe
#
#
ana.obj  : ana.c  tenkdefs.h anadef.h
sym.obj  : sym.c  tenkdefs.h # anadef.h
decl.obj : decl.c tenkdefs.h # anadef.h
gen.obj  : gen.c  tenkdefs.h # anadef.h
stat.obj : stat.c tenkdefs.h # anadef.h
expr.obj : expr.c tenkdefs.h # anadef.h
out.obj  : out.c  tenkdefs.h # anadef.h
mem.obj  : mem.c  tenkdefs.h # anadef.h
