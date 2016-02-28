#include <stddef.h>
#include "tenkafun.h"
#include "rpal.h"

int
RPal_Get(int *toon, byte *grb)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) == NULL)
        return -1;
    *toon = p->toon;
    for (i = 0; i < 16; ++i) {
        grb[i * 3 + 0] = p->grb[i][0];
        grb[i * 3 + 1] = p->grb[i][1];
        grb[i * 3 + 2] = p->grb[i][2];
    }
 #if 0
    {
        byte far *r;
        r = (byte far *)p + 0x10;
        printf("%s\n",r);
    }
 #endif
    return 0;
}
