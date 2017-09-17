#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "def.h"
#include "gf.h"

GF far * far Q0_Open(void far *gf, char far *name)
{
    char hdrnam[400];
    char *p;
    int  hdl;
    int  i;
    GF far *g;

    p = hdrnam;
    g = gf;
    for (i = 0; i < 4096; i++) ((char far *)gf)[i] = 0;
    for (i = 0; name[i]; i++) g->fileName[i] = hdrnam[i] = name[i];
    hdrnam[i] = 0;
    FIL_ChgExt(hdrnam, "FAL");
    hdl = _open(hdrnam,0);
    if (hdl < 0) {
    	FIL_ChgExt(hdrnam, "IPR");
    	hdl = _open(hdrnam,0);
    }
    if (hdl >= 0) {
    	_read(hdl, p, 400);
    	if (*p == 'F') {
    	    p = (char *)memchr(p,'\n', 398) + 1;
    	}
    	g->xsize = (int)strtol(p,&p,10);
    	g->ysize = (int)strtol(p,&p,10);
    	g->xstart = (int)strtol(p,&p,10);
    	g->ystart = (int)strtol(p,&p,10);
    	_close(hdl);
    } else {
    	g->xsize = 640;
    	g->ysize = 400;
    	g->xstart = 0;
    	g->ystart = 0;
    }
    g->pln = 24;
    g->color = 0;
    if ((hdl = _open(name, 0)) < 0)
    	return (void far *)1;
    g->filHdl = hdl;
    return g;
}

int far Q0_Close(void far *g)
{
    _close(((GF far *)g)->filHdl);
    return 0;
}
