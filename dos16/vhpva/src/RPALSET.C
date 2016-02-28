#include <stddef.h>
#include <dos.h>
#include "tenkafun.h"
#include "rpal.h"
#ifdef PC88VA	/* ‰‘ãva‚Å‚Írpal‚ÍŽg‚¦‚È‚¢...va2,va3‚Í‚Ç‚¤‚©‚µ‚ñ‚È‚¢ */
  #include "va.h"
  #define setGRB(n,g,r,b) Va_PalGRB((n),(g),(r),(b))
#else /* 98 */
  #define setGRB(n,g,r,b) do{\
              outp(0xa8, (n));\
              outp(0xaa, (g));\
              outp(0xac, (r));\
              outp(0xae, (b));\
          }while(0)
#endif

void
RPal_Set(int toon, byte *grb)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) != NULL) {
        p->toon = (toon > 100) ? 100 : toon;
        for (i = 0; i < 16; ++i) {
            p->grb[i][0] = grb[i*3 + 0];
            p->grb[i][1] = grb[i*3 + 1];
            p->grb[i][2] = grb[i*3 + 2];
        }
    }
    for (i = 0; i < 16; ++i) {
        setGRB(i,grb[i*3 + 0] * toon / 100,
                 grb[i*3 + 1] * toon / 100,
                 grb[i*3 + 2] * toon / 100);
    }
}

