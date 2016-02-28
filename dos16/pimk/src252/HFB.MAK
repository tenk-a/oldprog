ASM = asm /ML
#CC = TCC -ms -a- -G -O -1 -DWAB21
CC = TCC -ms -a- -G -O -1 -DEX_HF
#ANA = ana -DTURBO_C -ms -1 -c -DWAB21
ANA = ana -DTURBO_C -ms -1 -c
LIB = masters.lib
ASMDIR=$(TMP)

.c.obj:
	$(CC) -c $<
.ana.obj:
	$(ANA) -o$(ASMDIR) $<
	$(ASM) $(ASMDIR)$*.asm,$*,NUL,NUL
#.asm.obj:
#	$(ASM) $*,$*,NUL,NUL

OBJ =	\
	gomi.obj	fullpath.obj	dirname.obj	wildcmp.obj	\
	direntry.obj	rpal.obj	ems.obj		pri.obj		\
	key.obj		text.obj	sub.obj		rrpal.obj	\
	piload.obj	vv.obj		v98.obj				\
	pimk.obj	keyloop.obj					\
	vmp.obj		v21.obj		wab256.obj	vwab.obj	\
	fb.obj		vhf.obj		hf_sub.obj			\
	vfb.obj		vhfb.obj	\



pimk.exe : $(OBJ)
	$(CC) -ePIMK.EXE $(OBJ) $(LIB)

pimk.obj	: pimk.c
piload.obj	: piload.ana
keyloop.obj	: keyloop.c

vv.obj		: vv.ana
v98.obj		: v98.ana
vmp.obj		: vmp.ana
vwab.obj	: vwab.ana
wab256.obj	: wab256.ana
v21.obj		: v21.ana

vhf.obj		: vhf.ana
vfb.obj		: vfb.ana
vhfb.obj	: vhfb.ana
hf_sub.obj	: hf_sub.ana
fb.obj		: fb.c
	TCC -ms -a- -G -O  -S $<
	$(ASM) $*.asm,$*,NUL,NUL

sub.obj		: sub.c
ems.obj		: ems.ana
key.obj		: key.ana
text.obj	: text.ana
rpal.obj	: rpal.ana
rrpal.obj	: rrpal.ana
pri.obj		: pri.ana

fullpath.obj	: fullpath.c
dirname.obj	: dirname.c
direntry.obj	: direntry.ana
wildcmp.obj	: wildcmp.ana
gomi.obj	: gomi.c
