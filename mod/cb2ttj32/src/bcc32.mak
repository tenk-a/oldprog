#
# Makefile for Borland C++ (bcc32)
#

BCCDIR=c:\bc5
CC=$(BCCDIR)\bin\bcc32
TLIB=$(BCCDIR)\bin\tlib
TLINK=$(BCCDIR)\bin\tlink32
INCDIR=$(BCCDIR)\include
LIBDIR=$(BCCDIR)\lib
RM=del

CFLAGS=-Obe -Z -DSJIS -I. -I$(INCDIR) -I$(INCDIR)\mfc -L$(LIBDIR)

obj=big5.obj array.obj bf_file.obj


all: ttfstub.exe vflibbmp.exe

.c.obj:
	$(CC) $(CFLAGS) -c $<

.cpp.obj:
	$(CC) $(CFLAGS) -c $<

ttfstub.exe: ttfstub.obj $(obj)
	$(TLINK) /Tpe @&&|
c0x32 $**
$*
nul
bfc40 cw32 import32
|

### VFlib
VFLIBDIR=..\VFlib2-2.24.0\src
VFLIB=$(VFLIBDIR)\VFlib.lib
### FreeType
FTLIBDIR=..\freetype-1.1\lib
FTLIB=$(FTLIBDIR)\freetype.lib

vflibbmp.exe: vflibbmp.c
	$(CC) -Obe -Z -I. -I$(INCDIR) -I$(VFLIBDIR) -L$(LIBDIR) @&&|
$**
$(VFLIB)
$(FTLIB)
|

clean:
	$(RM) *.obj
	$(RM) ttfstub.exe
	$(RM) vflibbmp.exe
