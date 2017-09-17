bcc32 -4 -Ox -d -tWD -edplddyna.dll dplddyna.c >err.txt
bcc32 -4 -Ox -d -tWD -edpsvdyna.dll dpsvdyna.c >>err.txt
del  *.tds
del  *.obj
del  *.bak
type err.txt
