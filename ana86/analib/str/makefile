LIBNAME = ..\myl
OBJ	=		\
	str.obj	\
	s_atol.obj	\
	s_cat.obj	\
	s_chr.obj	\
	s_chrj.obj	\
	s_cknilj.obj	\
	s_cmp.obj	\
	s_cmpj.obj	\
	s_cpy.obj	\
	s_dctrlj.obj	\
	s_detab.obj	\
	s_dlnilj.obj	\
	s_end.obj	\
	s_htol.obj	\
	s_icmp.obj	\
	s_icmpj.obj	\
	s_len.obj	\
	s_lwr.obj	\
	s_lwrj.obj	\
	s_mcpyl.obj	\
	s_memchr.obj	\
	s_memcmp.obj	\
	s_memcpy.obj	\
	s_memmov.obj	\
	s_memset.obj	\
	s_ncmp.obj	\
	s_ncpy.obj	\
	s_ncutj.obj	\
	s_pcpy.obj	\
	s_flagj.obj	\
	s_rchr.obj	\
	s_rchrj.obj	\
	s_seplin.obj	\
	s_tab.obj	\
	s_trimj.obj	\
	s_uplowj.obj	\
	s_upr.obj	\
	s_uprj.obj	\

# --------------------------------------------------------------------------- #

.ana.obj:
	ANA -q1 -c -o$(TMP) -1 -dFC= $<
	asm /ML $(TMP)$*,$*,NUL,NUL

.as.obj:
	asm /ML $*.as,$*,NUL,NUL

$(LIBNAME).lib : $(OBJ)
	tlib $(LIBNAME) /c /e @myl.lnk

# --------------------------------------------------------------------------- #

str.obj	: str.ana
s_atol.obj	: s_atol.ana
s_cat.obj	: s_cat.ana
s_chr.obj	: s_chr.ana
s_chrj.obj	: s_chrj.ana
s_cknilj.obj	: s_cknilj.ana
s_cmp.obj	: s_cmp.ana
s_cmpj.obj	: s_cmpj.ana
s_cpy.obj	: s_cpy.ana
s_dctrlj.obj	: s_dctrlj.ana
s_detab.obj	: s_detab.ana
s_dlnilj.obj	: s_dlnilj.ana
s_end.obj	: s_end.ana
s_htol.obj	: s_htol.ana
s_icmp.obj	: s_icmp.ana
s_icmpj.obj	: s_icmpj.ana
s_len.obj	: s_len.ana
s_lwr.obj	: s_lwr.ana
s_lwrj.obj	: s_lwrj.ana
s_mcpyl.obj	: s_mcpyl.ana
s_memchr.obj	: s_memchr.ana
s_memcmp.obj	: s_memcmp.ana
s_memcpy.obj	: s_memcpy.ana
s_memmov.obj	: s_memmov.ana
s_memset.obj	: s_memset.ana
s_ncmp.obj	: s_ncmp.ana
s_ncpy.obj	: s_ncpy.ana
s_ncutj.obj	: s_ncutj.ana
s_pcpy.obj	: s_pcpy.ana
s_flagj.obj	: s_flagj.ana
s_rchr.obj	: s_rchr.ana
s_rchrj.obj	: s_rchrj.ana
s_seplin.obj	: s_seplin.ana
s_tab.obj	: s_tab.ana
s_trimj.obj	: s_trimj.ana
s_uplowj.obj	: s_uplowj.ana
s_upr.obj	: s_upr.ana
s_uprj.obj	: s_uprj.ana
