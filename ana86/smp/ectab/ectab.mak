ASM = ASM /ML
ANA = ANA -q1 -c -o
LINK = tlink /c /x
# MAKEILINE = 0+

.ana.obj:
	$(ANA) $<
	$(ASM) $*,$*,NUL,NUL
.asm.obj:
	$(ASM) $*,$*,NUL,NUL
.ana.asm:
	$(ANA) $<
#
ectab.exe :	ectab.obj	cmdlineb.obj	readln.obj	writln.obj \
		putstr.obj	direntry.obj	wildcmp.obj	chgext.obj \
		moveopt.obj	str.obj 	tab2spc.obj	spc2tab.obj \
		ltoa.obj	atol.obj	strncut.obj	strtrim.obj \
		strlen.obj	struplow.obj	dectrl.obj	nilchk.obj \
		sepline.obj
#	$(LINK) @${$#},$*.exe
	$(LINK) @ectab.lnk,$*.exe

# みてのとうり依存関係を記してないので変更したときは気を付けること...
ectab.obj    : ectab.ana
lincnt.obj   : lincnt.ana
dirlist.obj  : dirlist.ana
dl.obj       : dl.ana
direntry.obj : direntry.ana
cmdlineb.obj : cmdlineb.ana
putstr.obj   : putstr.ana
readln.obj   : readln.ana
writln.obj   : writln.ana
moveopt.obj  : moveopt.ana
chgext.obj   : chgext.ana
str.obj      : str.ana
tab2spc.obj  : tab2spc.ana
str2tab.obj  : str2tab.ana
ltoa.obj     : ltoa.ana
atol.obj     : atol.ana
wildcmp.obj  : wildcmp.ana
strncut.obj  : strncut.ana
strtrim.obj  : strtrim.ana
nilchk.obj   : nilchk.ana
sepline.obj  : sepline.ana
strlen.obj   : strlen.ana
struplow.obj : struplow.ana
dectrl.obj   : dectrl.ana
