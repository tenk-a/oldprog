# rules
CC = cl
CFLAGS = -Zi -MTd -W4 -wd4057 -wd4244 -wd4996 -wd4305 -wd4245 -wd4127 -D_CRT_SECURE_NO_WARNINGS
.c.obj:
	$(CC) $(CFLAGS) -c $<
#
#
#
OBJ = ana.obj decl.obj stat.obj expr.obj gen.obj out.obj sym.obj mem.obj

ana.exe : $(OBJ)
	$(CC) $(CFLAGS) -Feana.exe $(OBJ)
	copy ana.exe ..\ana.exe
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
