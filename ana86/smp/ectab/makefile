ASM = ASM /ML
ANA = ANA -q1 -c -o
LINK = tlink
.ana.obj:
	$(ANA) $<
	$(ASM) $*,$*,NUL,NUL
.asm.obj:
	$(ASM) $*,$*,NUL,NUL
.ana.asm:
	$(ANA) $<
#
program : dirlist.exe lincnt.exe dmp.exe

DLOBJ = dirlist.obj direntry.obj cmdlinea.obj putstr.obj wildcmp.obj moveopt.obj sepline.obj 

LCOBJ = lincnt.obj direntry.obj cmdlinea.obj readln.obj wildcmp.obj putstr.obj ltoa.obj moveopt.obj sepline.obj

DMPOBJ = dmp.obj ltoh.obj direntry.obj cmdlinea.obj readln.obj wildcmp.obj putstr.obj moveopt.obj sepline.obj strlen.obj htol.obj
DMPS = dmp ltoh direntry cmdlinea readln wildcmp putstr moveopt sepline strlen htol

dirlist.exe : $(DLOBJ)
	$(LINK) $(DLOBJ),$*,NUL
lincnt.exe : $(LCOBJ)
	$(LINK) $(LCOBJ),$*,NUL
dmp.exe : $(DMPOBJ)
	$(LINK) $(DMPS),$*,NUL
	
#
lincnt.obj   : lincnt.ana
dirlist.obj  : dirlist.ana
direntry.obj : direntry.ana
cmdlinea.obj : cmdlinea.ana
putstr.obj   : putstr.ana
readln.obj   : readln.ana
writln.obj   : writln.ana
moveopt.obj  : moveopt.ana
chgext.obj   : chgext.ana
str.obj      : str.ana
ltoa.obj     : ltoa.ana
atol.obj     : atol.ana
wildcmp.obj  : wildcmp.ana
sepline.obj  : sepline.ana
ltoh.obj     : ltoh.ana
htol.obj     : htol.ana
