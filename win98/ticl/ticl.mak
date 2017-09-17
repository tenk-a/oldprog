# ---------------------------------------------------------------------------
VERSION = BCB.03
# ---------------------------------------------------------------------------
!ifndef BCB
BCB = $(MAKEDIR)\..
!endif
# ---------------------------------------------------------------------------
PROJECT = TICL.exe
OBJFILES = TICL.obj Unit1.obj mycalc.obj subr.obj
RESFILES = TICL.res
RESDEPEN = $(RESFILES) Unit1.dfm
LIBFILES = 
LIBRARIES = vcldbx35.lib vcldb35.lib vclx35.lib vcl35.lib
SPARELIBS = vcl35.lib vclx35.lib vcldb35.lib vcldbx35.lib
PACKAGES =
PATHASM = .;
PATHCPP = .;
PATHPAS = .;
PATHRC = .;
DEBUGLIBPATH = $(BCB)\lib\debug
RELEASELIBPATH = $(BCB)\lib\release
DEFFILE =
# ---------------------------------------------------------------------------
CFLAG1 = -Od -Hc -w -Ve -r- -k -y -v -vi- -c -b- -w-par -w-inl -Vx
CFLAG2 = -I$(BCB)\include;$(BCB)\include\vcl -H=$(BCB)\lib\vcld.csm
CFLAG3 =
PFLAGS = -AWinTypes=Windows;WinProcs=Windows;DbiTypes=BDE;DbiProcs=BDE;DbiErrs=BDE \
  -U$(BCB)\lib\obj;$(BCB)\lib;$(DEBUGLIBPATH) -I$(BCB)\include;$(BCB)\include\vcl \
  -$Y -$W -$O- -v -JPHNV -M
RFLAGS = -i$(BCB)\include;$(BCB)\include\vcl
AFLAGS = /i$(BCB)\include /i$(BCB)\include\vcl /mx /w2 /zd
LFLAGS = -L$(BCB)\lib\obj;$(BCB)\lib;$(DEBUGLIBPATH) -aa -Tpe -x -v
IFLAGS =
LINKER = ilink32
# ---------------------------------------------------------------------------
ALLOBJ = c0w32.obj sysinit.obj $(OBJFILES)
ALLRES = $(RESFILES)
ALLLIB = $(LIBFILES) $(LIBRARIES) import32.lib cp32mt.lib
# ---------------------------------------------------------------------------
.autodepend

!ifdef IDEOPTIONS

[Version Info]
IncludeVerInfo=0
AutoIncBuild=0
MajorVer=1
MinorVer=0
Release=0
Build=0
Debug=0
PreRelease=0
Special=0
Private=0
DLL=0
Locale=1041
CodePage=932

[Debugging]
DebugSourceDirs=

[Parameters]
RunParams=
HostApplication=

!endif

$(PROJECT): $(OBJFILES) $(RESDEPEN) $(DEFFILE)
    $(BCB)\BIN\$(LINKER) @&&!
    $(LFLAGS) +
    $(ALLOBJ), +
    $(PROJECT),, +
    $(ALLLIB), +
    $(DEFFILE), +
    $(ALLRES) 
!

.pas.hpp:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.pas.obj:
    $(BCB)\BIN\dcc32 $(PFLAGS) { $** }

.cpp.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $* 

.c.obj:
    $(BCB)\BIN\bcc32 $(CFLAG1) $(CFLAG2) -o$* $**

.rc.res:
    $(BCB)\BIN\brcc32 $(RFLAGS) $<
#-----------------------------------------------------------------------------
