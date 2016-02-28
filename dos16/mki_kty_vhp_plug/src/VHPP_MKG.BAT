rem [[[ djgcc ]]]
gcc -v vhp_plug.c -o vhp_pl_g.out
coff2exe vhp_pl_g.out
del vhp_pl_g.out
