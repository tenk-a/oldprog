/*
 * NIFのDLﾘｽﾄの整形ﾌﾟﾛｸﾞﾗﾑ  Ver. 1.06
 * ver. 1.00 1991/07/21
 * ver. 1.01 1991/07/25   MS-DOSﾌｧｲﾙ名のﾁｪｯｸが甘かった('.'までにある' 'のﾁｪｯｸ)
 * ver. 1.02 1992/09/20   桁変更モードをなくした。
 * ver. 1.03 1992/10/30   不用行を削除するようにし、-p で従来通り残すようにした
 * ver. 1.04 1992/11/05   ワイルドカード対応
 * ver. 1.05 1992/11/05   ソート＆重複行削除
 * ver. 1.06 1992/11/29   -dで日付時間比較を行なう
 */

#define NIFDL	"NifDlTtl v1.06"
#define DATEFILE "NifDlTtl.dat"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <jctype.h>
#include <string.h>
#include <jstring.h>
#include <signal.h>
#include <dos.h>
#include "tenkdefs.h"
#include "jstr.h"

#define NAM_SIZE 85
#define BUF_SIZE 1024

#define Pr(s)	    printf(s)
#define PrExit(s)  (printf(s),exit(1))

int gDebFlg;
#define MSG(s)	    ((gDebFlg)? printf(s) : 0)
#define MSGF(s)     ((gDebFlg)? printf s  : 0)

void Usage(void)
{
    Pr(NIFDL " : NIFTYのDLリストを整形(説明は半角にできれば半角にする)\n");
    Pr("use: nifdlttl [-opts] file...\n");
    Pr(" -s     ソート＆重複行削除を行なう\n");
    Pr(" -m     '拡張子付MSDOS式ファイル名 +説明'をそろえる\n");
    Pr(" -u     -mの動作かつファイル名を大文字化する\n");
    Pr(" -e     -mの動作かつ拡張子をそろえる\n");
    Pr(" -r     参照数を０にする\n");
    Pr(" -p     不用行を削除しない\n");
    Pr(" -d     カレントディレクトリの'nifdlttl.dat'の日付より新しいファイルのみ処理\n");
    Pr(" -v     作業中のファイル名の表示\n");
    Pr(" -o     入力を BAKファイルにし、そのファイル名で出力\n");
    Pr(" -oPATH 出力を PATH にする\n");
    exit(0);
}

/*---------------------------------------------------------------------------*/
unsigned _dos_getftime(int hno, unsigned *date, unsigned *time)
{
    union REGS reg;

    reg.x.ax = 0x5700;
    reg.x.bx = hno;
    intdos(&reg, &reg);
    *time = reg.x.cx;
    *date = reg.x.dx;
    if (reg.x.flags & 1) {
    	*time = *date = 0;
    	return (unsigned)(errno = reg.x.ax);
    }
    return 0;
}

/*---------------------------------------------------------------------------*/
typedef struct direntry_t {
    struct direntry_t *next;
    byte *name;
} direntry_t;

_S direntry_t gDirEntryTop;
_S direntry_t *gDirEntryCur;

int  DirEntryGetAll(byte *keyname, word atr)
{
    byte fnambuf[140];
    direntry_t *dp;
    int l;

    l = 0;
    gDirEntryTop.next = NULL;
    dp = &gDirEntryTop;
    if (DirEntryGet(fnambuf,keyname,atr) == 0) {
    	do {
    	    dp->next = malloc(sizeof(direntry_t));
    	    dp = dp->next;
    	    if (dp == NULL)
    	    	return -1;
    	    dp->next = NULL;
    	    dp->name = strdup(fnambuf);
    	    if (dp->name == NULL)
    	    	return -1;
    	    l++;
    	} while (DirEntryGet(fnambuf,NULL,atr) == 0);
    }
    gDirEntryCur = gDirEntryTop.next;
    return l;
}

void DirEntryFreeAll(void)
{
    direntry_t *p,*np;

    p = gDirEntryTop.next;
    while (p != NULL) {
    	np = p->next;
    	free(p->name);
    	free(p);
    	p = np;
    }
}

byte *DirEntryNext(void)
{
    byte *p;

    if (gDirEntryCur == NULL)
    	return NULL;
    p = gDirEntryCur->name;
    gDirEntryCur = gDirEntryCur->next;
    return p;
}
/*---------------------------------------------------------------------------*/
typedef struct st_t {
    struct st_t *lnk[2];
    int  flg;
    byte *obj;
} st_t;

_S st_t *gStCur;
_S byte *gStObj;

_S st_t *StNew(void)
{
    st_t *sp;

    sp = malloc(sizeof(st_t));
    sp->lnk[0] = sp->lnk[1] = NULL;
    sp->flg = 0;
    sp->obj = NULL;
    return sp;
}

_S int StCmp(byte *p, byte *q)
{
    int a;
    if (*p == '-')
    	*p = 0xff;
    if (*q == '-')
    	*q = 0xff;
    if (*q == 0xff && *p == 0xff) {
    	byte *s=p,*t=q;
    	while (*s == *t && *s != ':' && *s != '\0') {
    	    ++s;
    	    ++t;
    	}
    	a = (*t - *s);
    } else {
    	a = -memcmp(p,q,31);
    }
    if (*p == 0xff)
    	*p = '-';
    if (*q == 0xff)
    	*q = '-';
    return a;
}

_S int StIns2(st_t *pp, int  lrFlg)
{
    byte l,r,nl,nr;
    int  a;
    st_t *p,*lp,*lrp,*lrlp,*lrrp;

 //MSGF(("pp=%p  ",pp));
    if (pp == NULL)
    	return 0;
    p = pp->lnk[lrFlg];
 //MSGF(("p=%p	",p));
    if (p == NULL) {
    	gStCur = p = pp->lnk[lrFlg] = StNew();
    	p->obj = strdup(gStObj);
 //MSGF(("obj=%p\n",p->obj));
    	return 3;
    }
    a = StCmp(gStObj,p->obj);
 //MSGF(("a=%d\n",a));
    if (a == 0) {
    	{
    	    int len0,len1;
    	    len0 = strlen(p->obj);
    	    len1 = strlen(gStObj);
    	    if (len0 < len1) {
    	    	free(p->obj);
    	    	p->obj = strdup(gStObj);
    	    } else if (len0 == len1) {
    	    	if (strcmp(p->obj,gStObj) < 0) {
    	    	    free(p->obj);
    	    	    p->obj = strdup(gStObj);
    	    	}
    	    }
    	}
    	gStCur = p;
    	return 0;
    }
    if (a < 0) {
    	l = 0;	r = 1;	nl = 1; nr = 2;
    } else {
    	l = 1;	r = 0;	nl = 2; nr = 1;
    }

    a = StIns2(p,l);
    if (a == 0) {
    	return 0;
    }
    if (p->flg == nr) {
    	p->flg = 0;
    	return 0;
    } else if (p->flg == 0) {
    	p->flg = nl;
    	return p->flg;
    }
    if (a == nl) {
    	lp = p->lnk[l];
    	lrp = lp->lnk[r];
    	p->lnk[l] = lrp;
    	p->flg = 0;
    	lp->lnk[r] = p;
    	lp->flg = 0;
    	pp->lnk[lrFlg] = lp;
    } else if (a == nr) {
    	lp = p->lnk[l];
    	lrp = lp->lnk[r];
    	lrlp = lrp->lnk[l];
    	lrrp = lrp->lnk[r];
    	pp->lnk[lrFlg] = lrp;
    	p->lnk[l]   = lrrp;
    	lp->lnk[r]  = lrlp;
    	lrp->lnk[l] = lp;
    	lrp->lnk[r] = p;
    	lp->flg = p->flg = 0;
    	if (lrp->flg == nl)
    	    p->flg = nr;
    	else if (lrp->flg == nr)
    	    lp->flg = nl;
    	lrp->flg = 0;
    } else {
    	PrExit("PRGERR:StIns2 flg = 3\n");
    }
    return 0;
}

st_t *StIns(st_t **root, byte *obj)
{
    static st_t dmy;

    dmy.lnk[0] = *root;
    gStObj = obj;
 /*MSGF(("root0=%p  ",*root));*/
    StIns2(&dmy, 0);
    *root = dmy.lnk[0];
 /*MSGF(("root1=%p\n",*root));*/
    return gStCur;
}

void StDelAll(st_t *sp)
{
    if (sp == NULL)
    	return;
    if (sp->lnk[0])
    	StDelAll(sp->lnk[0]);
    if (sp->lnk[1])
    	StDelAll(sp->lnk[1]);
    free(sp->obj);
    free(sp);
}

void StPutsAll(st_t *sp,FILE *fp,int n)
{
    static byte buf[40];

    if (sp == NULL)
    	return;
    if (sp->lnk[0])
    	StPutsAll(sp->lnk[0],fp,n+1);
    if (gDebFlg) {
    	strncpy(buf,sp->obj,38);
    	printf("%*s%s\n",n*2,"",buf);
    } else
    	fputs(sp->obj,fp);
    if (sp->lnk[1])
    	StPutsAll(sp->lnk[1],fp,n+1);
}

/*---------------------------------------------------------------------------*/

void rename_e(byte *old_nam, byte *new_nam)
{
    if (rename(old_nam,new_nam)) {
    	fprintf(stderr,"%s を %s に変名できません\n",old_nam,new_nam);
    	exit(errno);
    }
}

FILE *fopen_e(byte *name, byte *mode)
{
    FILE *fp;

    fp = fopen(name,mode);
    if (fp == NULL) {
    	printf("\n %s をオープンできません\n",name);
    	exit(errno);
    }
    return fp;
}

void fputs_e(byte *buf, FILE *fp)
{
    int k;

    k = fputs(buf,fp);
    if (k < 0)
    	PrExit("出力でエラーが発生した\n");
}

void ChgExt(byte *onam, byte *inam, byte *ext)
{
    byte *p;

    p = strncpy(onam,inam,80);
    if (*p == '.')
    	p++;
    if (*p == '.')
    	p++;
    if ((p = strrchr(p,'.')) != NULL)
    	++p;
    else {
    	p = onam;
    	while(*(p++))
    	    ;
    	*(p - 1) = '.';
    }
    strcpy(p,ext);
}

/*---------------------------------------------------------------------------*/
void Cnv(byte *msgp, int fmtFlg)
{
    byte *p;
    int  i,j,l;
    static byte buf[80];
    static byte name[20];
    static byte ext[4];

    if ((p = jstrchr(msgp,'.')) == NULL || fmtFlg == 0)
    	return;
    l = (int)(p - msgp);
    if (l <= 0||l >9||(isgrkana(*(p+1)) == 0 &&iskanji(*(p+1)) == 0)
    	/*|| (isspace(*(p+4))==0&&isspace(*(p+3))==0&&isspace(*(p+2))==0)*/)
    	return;
    while (--l && msgp[l] == ' ')
    	;
    ++l;
    for (i = 0;i < l;i++ )
    	if (msgp[i] == ' ')
    	    return;
    strncpy(name,msgp,l);
    name[l] = '\0';
    p++;
    for (i = 0;i < 3;i++,p++) {
    	if (isspace(*p) == 0)
    	    ext[i] = *p;
    	else {
    	    break;
    	}
    }
    ext[i] = '\0';
    if (isspace(*p) == 0)
    	return;
    if (fmtFlg & 0x02) {
    	jstruplow(name,A2UPPER);
    	jstruplow(ext,A2UPPER);
    }
    p = jstrskip(p," \t");
    l = strlen(p);
    if (l <= 41 - 13) {
    	if (fmtFlg & 0x04) {
    	    sprintf(buf,"%-8s.%-3s ",name,ext);
    	} else {
    	    strcat(name,".");
    	    strcat(name,ext);
    	    sprintf(buf,"%-12s ",name);
    	}
    } else {
    	strcat(name,".");
    	strcat(name,ext);
    	sprintf(buf,"%s ",name);
    	j = strlen(buf);
    	l = 41 - (l + j);
    	/*printf("%% %d %s(%d) %s(%d)\n",l,buf,j,p,strlen(p));*/
    	if (l > 0) {
    	    for (i = 0; i < l; i++) {
    	    	buf[j+i] = ' ';
    	    	buf[j+i+1] = '\0';
    	    }
    	}
    }
    strcat(buf,p);
    strcpy(msgp,buf);
}

int DlLin(byte *obuf,byte *ibuf,int fmtFlg,int refClrFlg)
{
    byte *p,*s;
    int  i;
    byte *nump;
    byte *idp;
    byte *datp;
    byte *sizp;
    byte *cntp;
    byte *typp;
    byte *mesp;

    nump = ibuf;
    idp  = ibuf+6;
    datp = ibuf+15;
    sizp = ibuf+24;
    cntp = ibuf+32;
    typp = ibuf+37;
    mesp = ibuf+39;

    /* num */
    for (p = nump,i = 0;*p == ' ' && i < 4;i++,p++)
    	;
    if (i == 4)
    	return 0;
    for (;i < 4;i++,p++)
    	if (*p <'0' || *p > '9')
    	    return 0;
    /* id */
    for (p = idp,i = 0;i < 3;i++,p++)
    	if (*p < 'A' || *p > 'Z')
    	    return 0;
    for(;i < 8;i++,p++)
    	if (*p <'0' || *p > '9')
    	    return 0;
    /* dat */
    for (p = datp,i=0;i < 2;i++,p++)
    	if (*p <'0' || *p > '9')
    	    return 0;
    if (*(p++) != '/')
    	return 0;
    for (i=0;i < 2;i++,p++)
    	if (*p <'0' || *p > '9')
    	    return 0;
    if (*(p++) != '/')
    	return 0;
    for(i=0;i < 2;i++,p++)
    	if (*p <'0' || *p > '9')
    	    return 0;
    /* siz */
    for (p = sizp,i = 0;*p == ' ' && i < 7;i++,p++)
    	;
    if (i == 7)
    	return 0;
    for (;i < 7;i++,p++)
    	if (*p <'0' || *p > '9')
    	    return 0;
    /* T or B */
    if (*typp != 'B' && *typp != 'T')
    	return 0;

    /* cnt */
    for(p = cntp,i = 0;*p == ' ' && i < 4;i++,p++)
    	;
    if (i == 4)
    	return 0;
    for(;i < 4;i++,p++)
    	if (*p <'0' || *p > '9')
    	    return 0;

    ibuf[4]  = '\0';
    ibuf[14] = '\0';
    ibuf[23] = '\0';
    ibuf[31] = '\0';
    ibuf[36] = '\0';
    ibuf[38] = '\0';

    if (refClrFlg) {
    	cntp = "   0";
    }
    sprintf(obuf,"%s  %s %s %s %s %s ",nump,idp,datp,sizp,cntp,typp);
    s = obuf + strlen(obuf);
    p = jstrskip(mesp," \t　");
    if (p == NULL)
    	p = mesp;
    jstrcnv(s,p,JTOPUNS|JTODGT|JTOALPH|JTOKPUN|JTOKATA);

    if (fmtFlg)
    	Cnv(s,fmtFlg);
    return 1;
}


void InpDl(FILE *fp, FILE *ofp, int fmtFlg, int refClrFlg, int undelLineFlg)
{
    static byte ibuf[BUF_SIZE];
    static byte obuf[BUF_SIZE + 100];
    byte *p;

    while (fgets(ibuf,BUF_SIZE,fp)) {
    	jstrtrim(ibuf);
    	if ( strlen(ibuf) > 39 &&
    	    ibuf[5] == ' ' && ibuf[38] == ' ' && ibuf[36] == ' '
    	    && ibuf[4] == ' '  && ibuf[14] == ' ' && ibuf[23] == ' '
    	    && ibuf[31] == ' ' && DlLin(obuf,ibuf,fmtFlg,refClrFlg))
    	{
    	    p = obuf;
    	} else if (ibuf[0] == '-' && ibuf[1] == ' ') {
    	    p = ibuf;
    	} else if (strncmp(ibuf,"番号 ",5) == 0
    	    && strstr(ibuf+5,"ID ") != NULL
    	    && strstr(ibuf+8,"登録日付") != NULL)
    	{
    	    if (undelLineFlg == 0)
    	    	continue;
    	    fputs_e("番号  ID       登録日付    ﾊﾞｲﾄ 参照   ﾃﾞｰﾀ名\n",ofp);
    	    continue;
    	} else {
    	    if (undelLineFlg == 0)
    	    	continue;
    	    p = ibuf;
    	}
    	jstrtrim(p);
    	fputs_e(p,ofp);
    }
    if (ferror(fp))
    	PrExit("入力でエラーが発生した\n");
}


void InpDl2(FILE *fp, FILE *ofp, int fmtFlg, int refClrFlg, int undelLineFlg)
{
    static byte ibuf[BUF_SIZE];
    static byte obuf[BUF_SIZE + 100];
    static st_t *root;
    byte *p;
    int  i;

    root = NULL;
    while (fgets(ibuf,BUF_SIZE,fp)) {
    	jstrtrim(ibuf);
    	i = strlen(ibuf);
    	if ( i > 39 && i < 82 &&
    	    ibuf[5] == ' ' && ibuf[38] == ' ' && ibuf[36] == ' '
    	    && ibuf[4] == ' '  && ibuf[14] == ' ' && ibuf[23] == ' '
    	    && ibuf[31] == ' ' && DlLin(obuf,ibuf,fmtFlg,refClrFlg))
    	{
    	    p = obuf;
    	} else if (ibuf[0] == '-' && ibuf[1] == ' ') {
    	    p = ibuf;
    	} else {
    	    if (undelLineFlg == 0)
    	    	continue;
    	    p = ibuf;
    	}
    	jstrtrim(p);
    	StIns(&root,p);
    }
    if (ferror(fp))
    	PrExit("入力でエラーが発生した\n");
    StPutsAll(root,ofp,0);
    StDelAll(root);
}

/*---------------------------------------------------------------------------*/
typedef struct opts_t {
    byte outputMode,fmtFlg,refClrFlg,undelLineFlg,sortFlg,vvvFlg,timeFlg;
    byte *outputName;
} opts_t;

void OptsErr(void)
{
    PrExit("オプション指定がおかしい\n");
}

void Option(byte *p, opts_t *o)
{
    int c;

    if (*p == '\0')
    	Usage();
    for (;;) {
    	c = *p++;
    	c = toupper(c);
    	if (c == '\0')
    	    break;
    	switch(c) {
    	case 'Z':
    	    gDebFlg = 1;
    	    break;
    	case 'S':
    	    o->sortFlg = 1;
    	    if (*p == '-')
    	    	p++,o->sortFlg = 0;
    	    break;
    	case 'R':
    	    o->refClrFlg = 1;
    	    if (*p == '-')
    	    	p++,o->refClrFlg = 0;
    	    break;
    	case 'M':
    	    o->fmtFlg |= 0x01;
    	    break;
    	case 'U':
    	    o->fmtFlg |= 0x03;
    	    break;
    	case 'E':
    	    o->fmtFlg |= 0x05;
    	    break;
    	case 'P':
    	    o->undelLineFlg = 1;
    	    break;
    	case 'D':
    	    o->timeFlg = 1;
    	    break;
    	case 'O':
    	    o->outputMode = 1;
    	    if (*p == '=')
    	    	p++;
    	    if (*p != '\0') {
    	    	o->outputName = p;
    	    	o->outputMode = 2;
    	    }
    	    goto OPTS_LOOPOUT;
    	case 'V':
    	    o->vvvFlg = 1;
    	    break;
    	case '?':
    	    Usage();
    	default:
    	    OptsErr();
    	}
    }
OPTS_LOOPOUT:
    ;
}

void KeyBrk(int sig)
{
    sig = 1;
    exit(sig);
}

int main(int argc, byte *argv[])
{
    static byte bakName[NAM_SIZE];
    static byte tmpName[NAM_SIZE];
    static opts_t opts;
    static word tim,dat;
    dword  datim;
    FILE *fp,*ofp;
    byte *p;
    int i,n;

    if (argc < 2)
    	Usage();
    signal(SIGINT,KeyBrk);
    //opts.sortFlg = 1;
    for (n = i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-')
    	    Option(p+1,&opts);
    	else
    	    n = 0;
    }
    if (n) {
    	printf("ファイルを指定してね\n");
    	exit(1);
    }
    if (opts.timeFlg) {
    	fp = fopen(DATEFILE,"r");
    	if (fp == NULL)
    	    tim = dat = 0;
    	else {
    	    _dos_getftime(fileno(fp),&dat,&tim);
    	    fclose(fp);
    	}
    	datim = dat * 0x10000L + tim;
    }
    if (opts.outputMode == 2)
    	ofp = fopen_e(opts.outputName,"w");
    else
    	ofp = stdout;
    for (i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-')
    	    continue;
    	n = DirEntryGetAll(p,0);
    	if (n == 0) {
    	    printf("%s はありません\n",p);
    	    exit(1);
    	}
    	else if (n < 0)
    	    PrExit("メモリが足りません\n");
    	while (--n >= 0) {
    	    p = DirEntryNext();
    	    /*printf("[%s]\n",p);*/
    	    fp = fopen_e(p,"r");
    	    if (opts.timeFlg) {
    	    	_dos_getftime(fileno(fp), &dat, &tim);
    	    	if (datim >= dat * 0x10000L+tim) {
    	    	    fclose(fp);
    	    	    continue;
    	    	}
    	    }
    	    if (opts.outputMode == 1) {
    	    	ChgExt(tmpName,p,"tmp");
    	    	ofp = fopen_e(tmpName,"w");
    	    }
    	    if (opts.vvvFlg)
    	    	printf("[%s]\n",p);
    	    if (opts.sortFlg)
    	    	InpDl2(fp,ofp, opts.fmtFlg, opts.refClrFlg,opts.undelLineFlg);
    	    else
    	    	InpDl(fp, ofp, opts.fmtFlg, opts.refClrFlg,opts.undelLineFlg);
    	    fclose(fp);

    	    if (opts.outputMode == 1) {
    	    	fclose(ofp);
    	    	ChgExt(bakName,p,"bak");
    	    	unlink(bakName);
    	    	rename_e(p,bakName);
    	    	rename_e(tmpName,p);
    	    }
    	}
    	DirEntryFreeAll();
    }
    if (opts.outputMode == 2)
    	fclose(ofp);
    if (opts.timeFlg) {
    	fp = fopen(DATEFILE,"w");
    	if (fp != NULL) {
    	    fclose(fp);
    	}
    }
    return 0;
}
