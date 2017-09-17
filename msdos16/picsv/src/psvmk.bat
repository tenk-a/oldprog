tasm /ML /M wildcmp.asm
tasm /ML /M direntry.asm
REM TC++ v1.0 —p
rem tcc -G -mc -DPC98 -DTCC picsv.c wildcmp.obj direntry.obj
REM TC++ v4.0 —p
tcc -G -mc -DPC98 -DTCC picsv.c wildcmp.obj direntry.obj   noehc.lib
