bcc32 -Ox -d -ewav2pcm.exe wav2pcm.c subr.c >err.txt
del *.obj
del *.tds
type err.txt
