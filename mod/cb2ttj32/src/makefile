all: obj demo utl dynabmp.exe freeman.exe splx.exe spline.exe toglyf.exe get.exe fgbmp.exe add.exe vector.exe

obj: big5.obj array.obj bf_file.obj

utl: readttf.exe engpatch.exe batch.exe ttfstub.exe part_ttf.exe subset.exe

demo: dsp.exe dfm.exe dpt.exe dspx.exe

CL_OPT=/AS /Ox /G2 /W3

dynabmp.exe: dynabmp.cpp
        CL $(CL_OPT) dynabmp.cpp big5

freeman.exe: freeman.cpp
        CL $(CL_OPT) freeman.cpp array safxcr.lib

splx.exe: splx.cpp
        CL $(CL_OPT) splx.cpp array safxcr.lib

spline.exe: spline.cpp
        CL $(CL_OPT) spline.cpp array safxcr.lib

toglyf.exe: toglyf.cpp
        CL $(CL_OPT) toglyf.cpp array safxcr.lib

get.exe: get.cpp
        CL $(CL_OPT) get.cpp bf_file

fgbmp.exe: fgbmp.cpp
        CL $(CL_OPT) fgbmp.cpp big5 fgx

add.exe: add.cpp
        CL $(CL_OPT) add.cpp bf_file

vector.exe: vector.cpp
        CL $(CL_OPT) vector.cpp array safxcr.lib

big5.obj: big5.c
        CL $(CL_OPT) /c big5.c

array.obj: array.cpp
        CL $(CL_OPT) /c array.cpp

bf_file.obj: bf_file.c
        CL $(CL_OPT) /c bf_file.c

readttf.exe: readttf.c
        CL $(CL_OPT) readttf.c bf_file

engpatch.exe: engpatch.cpp
        CL $(CL_OPT) engatch.cpp big5 bf_file

batch.exe: batch.cpp
        CL $(CL_OPT) batch.cpp

ttfstub.exe: ttfstub.cpp
        CL $(CL_OPT) ttfstub.cpp big5 bf_file array safxcr.lib

part_ttf.exe: part_ttf.cpp
        CL $(CL_OPT) part_ttf.cpp big5 bf_file array safxcr.lib

dsp.exe: dsp.cpp
        CL $(CL_OPT) dsp.cpp graphics.lib

dspx.exe: dspx.cpp
        CL $(CL_OPT) dspx.cpp graphics.lib

dpt.exe: dpt.cpp
        CL $(CL_OPT) dpt.cpp graphics.lib

dfm.exe: dfm.cpp
        CL $(CL_OPT) dfm.cpp graphics.lib

subset.exe: subset.cpp
        CL $(CL_OPT) subset.cpp big5


backup:
        xcopy *.cpp b:\ntuttf.bak
        xcopy *.c b:\ntuttf.bak
        xcopy *.h b:\ntuttf.bak
        copy makefile b:\ntuttf.bak
        xcopy *.exe b:\

