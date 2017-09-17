bcc32 -Ox -d -ewav_hdr.exe wav_hdr.c subr.c >err.txt
del *.obj
del *.tds
type err.txt
