LIBNAME = ..\myl
OBJ	=		\
	f_delete.obj	\
	f_fulpth.obj	\
	f_getatr.obj	\
	f_getcwd.obj	\
	f_getdrv.obj	\
	f_mkdir.obj	\
	f_rmdir.obj	\
	f_setatr.obj	\
	f_setcwd.obj	\
	f_setdrv.obj	\
	f_wccmp.obj	\
	h_close.obj	\
	h_creat.obj	\
	h_creatn.obj	\
	h_dup.obj	\
	h_dup2.obj	\
	h_open.obj	\
	h_read.obj	\
	h_seek.obj	\
	h_write.obj	\

# --------------------------------------------------------------------------- #

.ana.obj:
	ANA -q1 -c -o$(TMP) -1 -dFC= $<
	tasm /ML $(TMP)$*,$*,NUL,NUL

$(LIBNAME).lib : $(OBJ)
	tlib $(LIBNAME) /c /e @myl.lnk

# --------------------------------------------------------------------------- #

f_delete.obj	: f_delete.ana
f_fulpth.obj	: f_fulpth.ana
f_getatr.obj	: f_getatr.ana
f_getcwd.obj	: f_getcwd.ana
f_getdrv.obj	: f_getdrv.ana
f_mkdir.obj	: f_mkdir.ana
f_rmdir.obj	: f_rmdir.ana
f_setatr.obj	: f_setatr.ana
f_setcwd.obj	: f_setcwd.ana
f_setdrv.obj	: f_setdrv.ana
f_wccmp.obj	: f_wccmp.ana
h_close.obj	: h_close.ana
h_creat.obj	: h_creat.ana
h_creatn.obj	: h_creatn.ana
h_dup.obj	: h_dup.ana
h_dup2.obj	: h_dup2.ana
h_open.obj	: h_open.ana
h_read.obj	: h_read.ana
h_seek.obj	: h_seek.ana
h_write.obj	: h_write.ana
