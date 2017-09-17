rem [[[ djgcc ]]]
gcc -v mki_plug.c -o mki_pl_g.out
coff2exe mki_pl_g.out
del mki_pl_g.out
