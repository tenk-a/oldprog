bcc32 -4 -Ox -tWD -eifdyna.spi ifdyna.c >err.txt
del  *.tds
del  *.obj
del  *.bak
type err.txt
