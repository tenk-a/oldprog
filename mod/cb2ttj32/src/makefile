all: obj freeman.exe spline.exe toglyf.exe get.exe add.exe vector.exe utl

obj: sjis.obj array.obj bf_file.obj

utl: readttf.exe engpatch.exe batch.exe ttfstub.exe

CC=CL
#CL_OPT=/W3 /DHANKANA
CL_OPT=/W3
MFC_LIB=mfc42.lib

freeman.exe: freeman.cpp
        $(CC) $(C_OPT) freeman.cpp array.obj $(MFC_LIB)

spline.exe: spline.cpp
        $(CC) $(C_OPT) spline.cpp array.obj $(MFC_LIB)

toglyf.exe: toglyf.cpp
        $(CC) $(C_OPT) toglyf.cpp array.obj $(MFC_LIB)

add.exe: add.cpp
        $(CC) $(C_OPT) add.cpp bf_file.obj $(MFC_LIB)

vector.exe: vector.cpp
        $(CC) $(C_OPT) vector.cpp array.obj $(MFC_LIB)

engpatch.exe: engpatch.cpp
        $(CC) $(C_OPT) /DSJIS engpatch.cpp sjis.obj bf_file.obj

batch.exe: batch.c
        $(CC) $(C_OPT) batch.c winmm.lib

ttfstub.exe: ttfstub.cpp
        $(CC) $(C_OPT) /DSJIS ttfstub.cpp sjis.obj bf_file.obj array.obj $(MFC_LIB)

sjis.obj: sjis.c
        $(CC) $(C_OPT) /c sjis.c

array.obj: array.cpp
        $(CC) $(C_OPT) /c array.cpp

bf_file.obj: bf_file.c
        $(CC) $(C_OPT) /c bf_file.c

# ----
get.exe: get.cpp
        $(CC) $(C_OPT) get.cpp bf_file.obj $(MFC_LIB)

readttf.exe: readttf.c
       $(CC) $(C_OPT) readttf.c bf_file.obj

