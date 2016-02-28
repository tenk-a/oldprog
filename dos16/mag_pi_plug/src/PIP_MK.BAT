rem ana -1 -c2 -o. piload.ana
tasm /ML /M piload.asm
bcc -ml -1 -Ox -f- pi_plug.c piload.obj noehl.lib

