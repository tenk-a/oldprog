CC = TCC -a- -G -O -1 -ms -d -DHF -DMP
##ASM = r86
ASM = asm /ML
##ANA = ANA -c -DTURBO_C -q1 -1 -ar
ANA = ANA  -DTURBO_C -q1 -1

.ana.obj:
	$(ANA) -s$*.a $<
	$(ASM) $*.a,$*,NUL,NUL
##	$(ANA) -s$*.a $<
##	$(ASM) $*.a
.asm.obj:
	$(ASM) $*.asm,$*,NUL,NUL
.c.obj:
	$(CC) -c $<


OBJ = dopi.obj sub.obj gvsub.obj gdcscl.obj pi.obj readdata.obj pild4a.obj pild8.obj decol.obj gvdsp.obj subr.obj direntry.obj wildcmp.obj    pri.obj ltoa.obj ltoh.obj hf.obj fb.obj mp.obj

dopi.exe : $(OBJ)
#	$(CC) @${$#} hfts.lib
	$(CC) @dopi.lnk


dopi.obj     : dopi.c
sub.obj	     : sub.c
gvsub.obj    : gvsub.c
gdcscl.obj   : gdcscl.ana
pi.obj       : pi.ana
pild4a.obj   : pild4a.ana
pild8.obj    : pild8.ana
decol.obj    : decol.ana
readdata.obj : readdata.ana
gvdsp.obj    : gvdsp.ana
subr.obj     : subr.ana
wildcmp.obj  : wildcmp.ana
direntry.obj : direntry.ana

ltoa.obj     : ltoa.ana
ltoh.obj     : ltoh.ana
pri.obj      : pri.ana

mp.obj       : mp.ana
hf.obj	     : hf.c
# fb.obj     : fb.asm
fb.obj       : fb.c
