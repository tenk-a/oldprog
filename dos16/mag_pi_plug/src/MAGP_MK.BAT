rem ana -1 -c2 -o. magload.ana
tasm /ML /M magload.asm
bcc -ml -1 -Ox -f- mag_plug.c magload.obj noehl.lib
