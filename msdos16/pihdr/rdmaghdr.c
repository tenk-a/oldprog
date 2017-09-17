#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "pihdr.h"

static FILE *gMagFp;

static int  GetM1(void)
{
    int t;
    t = fgetc(gMagFp);
    if (t < 0) {
    	printf("EOFがおかしな位置に現れた\n");
    	exit (1);
    }
    return t;
}

static word GetM2(void)
{
    int t;

    t = GetM1();
    return GetM1()*0x100 + t;
}

#if 1
void PriMagHdr(byte *name,chgopt_t *q)
{
    int i;
    printf("[FILE] %s\n",name);
    printf("    ドット比 %d:%d",q->r1,q->r2);
    printf("  サイズ %d*%d",q->xsiz,q->ysiz);
    printf("  基点(%d,%d)",q->bx,q->by);
    printf("  %s\n",(q->palcnt == 16) ?" 16色" : "256色");
    printf("    n:R G B ");
    for (i = 0; i < q->palcnt;i++) {
    	printf("  %x:%02x %02x %02x",i,
    	    q->pal[i*3+0],q->pal[i*3+1],q->pal[i*3+2]);
    	if ((i % 4) == 3) {
    	    printf("\n            ");
    	}
    }
    printf("\n");
    printf("%s\n\n",q->comment);
}
#endif

int RdMagHdr(byte *magname, chgopt_t *q)
{
    int i,c;
    byte *s;
    static byte user[20];

    gMagFp = fopen_e (ChgExtTmp(magname,"MAG"),"rb");
    if	 (GetM1() != 'M'|| GetM1() != 'A'|| GetM1() != 'K'|| GetM1() != 'I'
    	||GetM1() != '0'|| GetM1() != '2'|| GetM1() != ' '|| GetM1() != ' ') {
    	printf("%s はMAGファイルでない\n",magname);
    	exit (1);
    }
    for (i = 0;i < 4;i++)
    	q->sv[i] = GetM1();
    q->sv[4] = 0;
    GetM1();
    q->user = s = user;
    for (i = 0;i < 18; i++) {
    	c = GetM1();
    	if (c == 0x1a)
    	    break;
    	*s++ = c;
    }
    while (i-- > 0 && *(s - 1) == ' ')
    	s--;
    *s = '\0';
    if (strlen(user) == 0 || strcmp(user,">謎<") == 0)
    	q->user = 0;
    q->comment = s = gXComment;
    if (c != 0x1a)
    	c = GetM1();
    i = 0;
    if (c != 0x1a) {
    	if (c == ' ' || c == '\t')
    	    c = GetM1();
    	while (c != 0x1a && c != '\0') {
    	    if (i < X_COMMENT_SIZ-28) {
    	    	*s++ = c;
    	    	i++;
    	    }
    	    c = GetM1();
    	}
    }
    *s = '\0';
    for (i = 0; i < 3;i++)
    	GetM1();
    c = GetM1();
    if (c & 0x01) {
    	q->r1 = 2;
    	q->r2 = 1;
    } else {
    	q->r1 = q->r2 = 0;
    }
    q->bx = GetM2();
    q->by = GetM2();
    if (q->bx == 0 && q->by == 0)
    	q->bflg = -1;
    else
    	q->bflg = 1;
    q->xsiz = GetM2() + 1 - q->bx;
    q->ysiz = GetM2() + 1 - q->by;
    for (i = 0; i < 20;i++)
    	GetM1();
    q->palcnt = c = (c & 0x80)? 256 : 16;
    for (i = 0; i < c;i++) {
    	q->pal[i*3 + 1] = GetM1();
    	q->pal[i*3 + 0] = GetM1();
    	q->pal[i*3 + 2] = GetM1();
    }
    fclose(gMagFp);
  #if 0
    PriMagHdr(ChgExtTmp(magname,"MAG"),q);
  #endif
    return 0;
}
