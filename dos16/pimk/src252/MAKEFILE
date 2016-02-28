ASM = tasm /ML /M
#CC = TCC -ms -a- -G -O -1 -f-
CC = BCC -ms -a- -G -1 -f- -Ox
ANA = ana -DTURBO_C -ms -1 -c
# ŠÂ‹«•Ï”TMP‚ÌÅŒã‚Ì•¶Žš‚Í'\'‚Å‚È‚¢‚Æ‚¢‚¯‚È‚¢B
ASMDIR=$(TMP)\\

.c.obj:
	$(CC) -c $<
.ana.obj:
	$(ANA) -o$(ASMDIR) $<
	$(ASM) $(ASMDIR)$*.asm,$*,NUL,NUL
#.asm.obj:
#	$(ASM) $*,$*,NUL,NUL

LIB =	noehs.lib

OBJ =	\
	gomi.obj	fullpath.obj	dirname.obj	wildcmp.obj	\
	direntry.obj	rpal.obj	ems.obj		pri.obj		\
	key.obj		text.obj	sub.obj		rrpal.obj	\
	fil.obj								\
	piload.obj	magload.obj	djpload.obj	q0load.obj	\
	bmpload.obj	pmtload.obj					\
	vv.obj		vvw.obj		pimk.obj	keyloop.obj	\
	v98.obj		vmp.obj		v21.obj				\
	wab256.obj	vwab.obj					\
	fb.obj		vhf.obj		hf_sub.obj			\

#	vfb.obj		vhfb.obj	\



pimk.exe : $(OBJ)
	$(CC) -epimk.exe $(OBJ) $(LIB)
	copy pimk.exe \cgsl\pimk.exe

pimk.obj	: pimk.c
piload.obj	: piload.ana
magload.obj	: magload.ana
djpload.obj	: djpload.ana
q0load.obj	: q0load.c
bmpload.obj	: bmpload.ana
pmtload.obj	: pmtload.ana
keyloop.obj	: keyloop.c

vv.obj		: vv.ana
vvw.obj		: vvw.ana
v98.obj		: v98.ana
vmp.obj		: vmp.ana
vwab.obj	: vwab.ana
wab256.obj	: wab256.ana
v21.obj		: v21.ana

vhf.obj		: vhf.ana
# vfb.obj	: vfb.ana
# vhfb.obj	: vhfb.ana
hf_sub.obj	: hf_sub.ana
fb.obj		: fb.c
	BCC -ms -a- -G -O  -S $<
	$(ASM) $*.asm,$*,NUL,NUL

sub.obj		: sub.c
ems.obj		: ems.ana
key.obj		: key.ana
text.obj	: text.ana
rpal.obj	: rpal.ana
rrpal.obj	: rrpal.ana
pri.obj		: pri.ana

fil.obj		: fil.ana
fullpath.obj	: fullpath.c
dirname.obj	: dirname.c
direntry.obj	: direntry.ana
wildcmp.obj	: wildcmp.ana
gomi.obj	: gomi.c
