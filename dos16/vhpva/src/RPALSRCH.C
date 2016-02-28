#include <stddef.h>
#include <dos.h>
#ifdef LSI_C
  #include <farstr.h>
#endif
#include "tenkafun.h"
#include "rpal.h"

struct MCB_t {
    byte  flag;
    word owner;
    word blksiz;
    byte reserve[11];
};

struct MCB_t far *
_dos_mcbfirst(void)
{
    union REGS reg;
    struct SREGS sreg;

    reg.h.ah = 0x52;
    intdosx(&reg,&reg,&sreg);
 #if 0
    {
        word seg;
        byte far *p;

        p = MAKE_FP(sreg.es, reg.x.bx - 2);
        seg = *((word far *)p);
        printf("%04x:%04x=%08lx %04x %lx\n",sreg.es,reg.x.bx-2,p,
                                            seg,MAKE_FP(seg,0));
    }
 #endif
    return MAKE_FP(*(word far *)MAKE_FP(sreg.es, reg.x.bx - 2), 0);
}

#ifndef LSI_C
static int
far_strcmp(unsigned char far *s1, unsigned char far *s2)
{
    while (*s1 == *s2)
        if (*s1 == '\0')
            return 0;
        else {
            ++s1;
            ++s2;
        }
    if (*s1 > *s2)
        return 1;
    else
        return -1;
}
#endif

struct RAMPAL far *
RPal_Search(void)
{
    struct MCB_t far *p;
    word seg;

    p = _dos_mcbfirst();
    seg = PTR_SEG(p);
    for (;;) {
      #if 0
        printf("pointer = %08lx\nflag = %c\nowner = %x\nsize = %x\n",
            p,p->flag,p->owner,p->blksiz);
      #endif
        if (p->owner != NULL) {
            if (far_strcmp((byte far *)p + 0x10, "pal98 grb") == 0)
                return (struct RAMPAL far *)((byte far *)p + 0x10);
        }
        seg += p->blksiz + 1;
        p = MAKE_FP(seg,0);
        if (p->flag != 'M')
            break;
    }
    return NULL;
}

/* ÇøÇ•Ç¡Ç≠ÇÈÅ[ÇøÇÒ */
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    {
        byte far *r;
        r = (byte far *)p + 0x10;
        printf("%s\n",r);
    }
    return 0;
}


void
RPal_Set(int toon, byte *grb)
{
    struct RAMPAL far *p;
    int  i;

    if ((p = RPal_Search()) != NULL) {
        p->toon = (tˇon > 100) ? 100 : toon;
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


void
main(int argc, char *argv[])
{
    static byte buf[200];
    static byte grb[48];
    static int toon;
    FILE *fp;
    int  i;

    if (RPal_Get(&toon, grb)) {
        printf("èÌíì ﬂ⁄ØƒÇ™Ç†ÇËÇ‹ÇπÇÒ\n");
        exit(1);
    }
    printf("Toon = %d\n",toon);
    for (i = 0; i < 16; ++i)
        printf("G:%02x  R:%02x  B:%02x\n",grb[i*3],grb[i*3+1],grb[i*3+2]);
    
    if (argc < 2)
        return;
    strcpy (buf,argv[1]);
    strcat (buf,".RGB");
    if ((fp = fopen(buf,"rb")) == NULL) {
        printf(".RGBÃß≤ŸÇopenÇ≈Ç´Ç‹ÇπÇÒ");
        exit(1);
    }
    for (i = 0; i < 16; ++i) {
        grb[i*3+1] = fgetc(fp);
        grb[i*3+0] = fgetc(fp);
        grb[i*3+2] = fgetc(fp);
    }
    fclose(fp);
    if (argc > 2 && *(argv[2]) >= '0' && *(argv[2]) <= '9')
        toon = (int)strtol(argv[2],NULL,10);
    if (toon > 100)
        toon = 100;
    RPal_Set(toon,grb);
    if (RPal_Get(&toon, grb)) {
        printf("èÌíì ﬂ⁄ØƒÇ™Ç†ÇËÇ‹ÇπÇÒ\n");
        exit(1);
    }
    printf("Toon = %d\n",toon);
    for (i = 0; i < 16; ++i)
        printf("G:%02x  R:%02x  B:%02x\n",grb[i*3],grb[i*3+1],grb[i*3+2]);
}
#endif
