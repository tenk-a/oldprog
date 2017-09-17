/*
    PiHdr
    v1.00  1992.06.30	writen by M.Kitamura
    v1.10  1992.09.14	オプション名変更.
    v1.50  1992.09.15	MAGファイルのヘッダを参照できるようにした.
    v1.51  1992.10.27	ワイルド・カード機能の ^c を削除した.
    	    	    	複数のファイルを一度に変更できるようにした.
    v1.52  1992.11.03	DirEntryGet が複数ファイルの変更に対応していないのを
    	    	    	わすれていた^_^;ので修正
    v1.53  1993.04.03	pi2mag,mag2pi,dopiでサポートしたユーザ名(-u)に対応
    	    	    	とりこんだmagヘッダの扱いをmag2piに合わせた.
    	    	    	-p,-iオプション修正、変更. -yで元の日付を変更しない
    	    	    	始点情報判定を緩くした. -8付加.
    v1.53b 1993.05.21	変更作業をすると、セーバ名が消える場合があった
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "pihdr.h"

#define PIHDR	"PiHdr v1.53b"

/* DIRENTが定義されると、ﾜｲﾙﾄﾞ･ｶｰﾄﾞ展開をasmﾙｰﾁﾝ(TC(++),LSI-C用)で行う.*/
/* 定義をやめると、ﾜｲﾙﾄﾞｶｰﾄﾞ展開できないが TC,LSI-C以外 で再ｺﾝﾊﾟｲﾙ可能なはず?*/
/* #define  DIRENT  */

/* FDATEが定義されると、元の日付を変更しないようにする -y を有効にする */
/* #define FDATE */

#if 1
byte gComment[COMMENT_SIZ+2];
byte gComment2[COMMENT2_SIZ+2];
byte gExBuf[EX_MAX+2];
byte gXComment[X_COMMENT_SIZ+2];
#endif
byte gBuf[BUFSZ+16];
static FILE *gOutFp,*gInFp;
static int  gCmd,gMakRgbFileFlg,gKeyWaitFlg,gOvrFlg;
static byte *gPiName;
static byte *gPrgName;
static int  gMagFlg;
static byte *gMagName;
static byte gMagMsgFlg;
#ifdef FDATE
static unsigned gFdate,gFtime;
static int  gFtimFlg;
#endif
static byte gPalD8[48] = {
    	0x00,0x00,0x00,
    	0x00,0x00,0xff,
    	0xff,0x00,0x00,
    	0xff,0x00,0xff,
    	0x00,0xff,0x00,
    	0x00,0xff,0xff,
    	0xff,0xff,0x00,
    	0xff,0xff,0xff,
    	0x00,0x00,0x00,
    	0x00,0x00,0xff,
    	0xff,0x00,0x00,
    	0xff,0x00,0xff,
    	0x00,0xff,0x00,
    	0x00,0xff,0xff,
    	0xff,0xff,0x00,
    	0xff,0xff,0xff
    };

/*-------------------------------------------------------------------------*/
static void Usage(void)
{
    printf( PIHDR " : Piファイルのヘッダ表示＆変更を行う\n");
    printf("use: PiHdr [-opts] pifile[.Pi]...\n");
    printf(" -t            簡単な表示. オプション無指定ではこれが指定されたことになる\n");
    printf(" -d            コメントの表示           -l            くわしい表示\n");
    printf(" -y            日付を変更しない         -b            .BAKを作成しない\n");
    printf(" -z[0]         変更時, 確認待ちをしない. -z0なら表示も行わない\n");
  #ifdef FDATE
  #endif
    printf(" -n[r1,r2]     縦横ドット比を r1:r2 に変更(省略:-n0,0)\n");
    printf(" -i<name>      セーバ機種名を<name>に変更(name:4バイト)\n");
    printf(" -o[x,y]       始点を変更(省略:-o0,0)\n");
    printf(" -k            機種依存情報を削除（始点情報含む）\n");
    printf(" -h[文字列]    隠れコメントを変更. 文字列省略:隠れコメント削除\n");
    printf(" -u[名前]      ユーザ名を指定. 名前省略:ユーザ名削除\n");
    printf(" -r            .RGB ファイルの作成(16色画像のみ)\n");
    printf(" -p[rgbfile]   rgbfile[.rgb] よりパレットを取得. rgbfile省略:pifile.rgb\n");
    printf(" -p-           パレットを削除し、デフォルト・パレットに変更\n");
    printf(" -8            パレットをデジタル８色に変更\n");
    printf(" -m[magfile]   magfile[.mag]のヘッダよりドット比,パレット,コメント,ユーザ名,\n");
    printf("               始点を得る. magfile省略時:pifile.mag\n");
    printf(" -lm[magfile]  -m と同じ。ついでに mag の情報を表示する\n");
    printf(" -c<file>      コメントを file より読み込み変更\n");
    printf(" =[文字列]     コメントを変更。コマンド・ラインの行末で有効\n");
    printf("               ただし連続する空白は１文字に変換されてしまう\n");
    exit(0);
}

/*- 日付保持のために使用 ----------------------------------------------------*/
#ifdef FDATE
#ifdef __TURBOC__
unsigned _dos_setftime(int hno, unsigned dat, unsigned tim)
{
    union REGS reg;

    reg.x.ax = 0x5701;
    reg.x.bx = hno;
    reg.x.cx = tim;
    reg.x.dx = dat;
    intdos(&reg, &reg);
    if (reg.x.flags & 1) {/* reg.x.flags はTCに依存 */
    	return (unsigned)(errno = reg.x.ax);
    }
    return 0;
}

unsigned _dos_getftime(int hno, unsigned *dat, unsigned *tim)
{
    union REGS reg;

    reg.x.ax = 0x5700;
    reg.x.bx = hno;
    intdos(&reg, &reg);
    if (reg.x.flags & 1) {  /* reg.x.flags はTCに依存 */
    	return (unsigned)(errno = reg.x.ax);
    }
    *tim = reg.x.cx;
    *dat = reg.x.dx;
    return 0;
}
#endif
#endif

/*-------------------------------------------------------------------------*/
void ChgExt(byte *onam, byte *inam, byte *ext)
{
    byte *p;

    p = strncpy(onam,inam,80);
    if (*p == '.')
    	p++;
    if (*p == '.')
    	p++;
    if ((p = strrchr(p,'.')) == NULL) {
    	p = onam;
    	while(*p)
    	    p++;
    }
    if (ext == NULL||*ext == '\0') {
    	*p = '\0';
    } else {
    	*p++ = '.';
    	strcpy(p,ext);
    }
}

#if 1
byte *ChgExtTmp(byte *s, byte *p)
{
    static byte buf[130];

    if (s)
    	ChgExt(buf,s,p);
    return buf;
}
#endif

#if 1
static byte *FName(byte *p)
{
    byte *s;
    byte c;

    s = p;
    while(*s) {
    	c = *s++;
    	if (c == '/' || c == '\\' || c == ':') {
    	    p = s;
    	}
    }
    return p;
}
#endif

FILE *fopen_e(byte *name, byte *mode)
{
    FILE *fp;

    if (name == NULL) {
    	name = "File";
    	goto J1;/*return stdin;*/
    }
    fp = fopen(name,mode);
    if (fp == NULL) {
  J1:
    	printf("\n%s をオープンできません\n",name);
    	exit(errno);
    }
    return fp;
}

void rename_e(byte *old_nam, byte *new_nam)
{
    if (rename(old_nam,new_nam)) {
    	printf("%s を %s に変名できません\n",old_nam,new_nam);
    	exit(1);
    }
}

static void Rgb8to4(byte *pal,byte *rgb)
{
    int i;

    for (i = 0; i < 48; i++) {
    	rgb[i] = (pal[i] >> 4) & 0x0f;
    }
}

static void MakRgbFile(byte *pal,byte *nam)
{
    byte rgb[48];
    FILE *fp;

    Rgb8to4(pal,rgb);
    fp = fopen_e(ChgExtTmp(nam,"RGB"),"wb");
    fwrite(rgb,1,48,fp);
    fclose(fp);
}

/*--- TC用ファイル名のワイルド・カード展開用 --------------------------------*/

#ifdef DIRENT

typedef struct direntry_t {
    struct direntry_t *next;
    byte *name;
} direntry_t;

static direntry_t gDirEntryTop;
static direntry_t *gDirEntryCur;

int  DirEntryGetAll(byte *keyname,word atr)
{
    byte fnambuf[140];
    direntry_t *dp;
    int l;

    l = 0;
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
#endif
/*-------------------------------------------------------------------------*/
static int  GetC1(void)
{
    int t;
    t = fgetc(gInFp);
    if (t < 0) {
    	printf("EOFがおかしな位置に現れた\n");
    	exit (1);
    }
    return t;
}

static word GetC2(void)
{
    int t;

    t = GetC1();
    return t * 0x100 + GetC1();
}

static void HdrErr(byte *msg)
{
    printf("%-8s : %s\n",gPiName,msg);
}

static int  GetHdr(pihdr_t *p,byte *name)
{
    static byte nam[130];
    int i,c;
    int errf = 0;

    memset(p,'\0',sizeof(pihdr_t));
    p->comment	= gComment;
    p->comment2 = gComment2;
    p->ex   	= gExBuf;
    p->name = strncpy(nam,name,128);
    if (GetC2() != 'P'*0x100 + 'i') {
    	HdrErr("Piファイルでない");
    	return -1;
    }
    i = 0;
    while ((c = GetC1()) != 0x1a) {
    	if (c == '\0') {
    	    HdrErr("コメントの途中に文字コード 0 が現れた");
    	} else if (i < COMMENT_SIZ) {
    	    p->comment[i] = c;
    	    i++;
    	}
    }
    p->comment[i] = 0;
    if (i >= COMMENT_SIZ) {
    	HdrErr("コメントが長すぎる");
    	errf = 1;
    }
    if (strncmp(p->comment,"User:",5) == 0) {
    	p->user = p->comment+5;
    	for (i = 0; i < COMMENT_SIZ;i++) {
    	    c = p->user[i];
    	    if (c == '\r' || c == '\n' || c == '\0') {
    	    	break;
    	    }
    	}
    	p->userlen = 5 + i + 2;
    	p->user[i] = 0;
    	if (c == '\r' && p->user[i+1] == '\n') {
    	    ++i;
    	    p->user[i] = 0;
    	}
    	p->comment += 5 + i + 1;
    }
    p->cmtlen = strlen(p->comment);

    i = 0;
    while ((c = GetC1()) != 0x00) {
    	if (i < COMMENT2_SIZ) {
    	    p->comment2[i] = c;
    	    i++;
    	}
    }
    p->comment2[i] = 0;
    if (i >= COMMENT2_SIZ) {
    	errf = 1;
    	HdrErr("隠れコメントが長すぎる");
    	i = 0;
    }
    p->cmt2len = strlen(p->comment2);

    p->mode = GetC1();
    p->r1 = GetC1();
    p->r2 = GetC1();
    p->pln = GetC1();
    if (p->pln != 4 && p->pln != 8) {
    	HdrErr("16色,256色以外のPiファイル？");
    	errf=1;
    }
    p->sv[0] = GetC1();
    p->sv[1] = GetC1();
    p->sv[2] = GetC1();
    p->sv[3] = GetC1();
    p->sv[4] = '\0';
    p->exsiz = GetC2();
    if (p->exsiz) {
    	p->bflg = 1;
    	if  (p->exsiz >= EX_MAX) {
    	    HdrErr("機種依存情報がおおすぎる");
    	    errf = 1;
    	}
    	for (i = 0; i < p->exsiz; i++) {
    	    if (i < EX_MAX)
    	    	p->ex[i] = GetC1();
    	}
    	if (p->exsiz >= 5 /*&& p->ex[0] == 1*/) {
    	    if (p->exsiz == 5 && p->ex[0] == 1)
    	    	p->bflg = 2;
    	    p->bx = p->ex[1]*0x100 + p->ex[2];
    	    p->by = p->ex[3]*0x100 + p->ex[4];
    	}
    }
    p->x_siz = GetC2();
    p->y_siz = GetC2();
    if ((p->mode & 0x80) == 0) {
    	if (p->pln == 4) {
    	    p->pallen = 48;
    	    for (i = 0; i < 48; i ++) {
    	    	p->pal[i] = GetC1();
    	    }
    	} else if (p->pln == 8) {
    	    p->pallen = 256 * 3;
    	    for (i = 0; i < 256 * 3; i++) {
    	    	p->pal[i] = GetC1();
    	    }
    	}
    }
    p->dat = ftell(gInFp);
    p->hdrsiz = (int)p->dat;
    return errf;
}

static void GetRgbFile(byte *p,byte *pal)
{
    FILE *fp;
    int i;
    long l;

    fp = fopen_e(ChgExtTmp(p,"RGB"),"rb");
    l = fread(pal,1,48,fp);
    fclose(fp);
    if (l != 48) {
    	printf("指定された.RGBファイルが48バイトない\n");
    	exit(1);
    }
    for (i = 0;i < 48;i++) {
    	pal[i] <<= 4;
    }
}

static void OptHdr(pihdr_t *p, chgopt_t *q)
{
    int i;

    if (q->sv && q->sv[0]) {
    	if (stricmp(q->sv,p->sv)) {
    	    memcpy(p->sv,q->sv,4);
    	}
    }
    if (p->r1 != q->r1||p->r2 != q->r2) {
    	if (p->r1 == p->r2 || p->r1 == 0 || p->r2 == 0)
    	    p->r1 = p->r2 = 0;
    	if (q->r1 == q->r2 || q->r1 == 0 || q->r2 == 0)
    	    q->r1 = q->r2 = 0;
    	p->r1 = q->r1;
    	p->r2 = q->r2;
    }
    if (q->bflg) {
    	if (q->bflg > 0) {
    	    p->exsiz = 5;
    	    p->ex[0] = 1;
    	    p->ex[1] = q->bx / 0x100;
    	    p->ex[2] = q->bx % 0x100;
    	    p->ex[3] = q->by / 0x100;
    	    p->ex[4] = q->by % 0x100;
    	    p->bflg = 2;
    	} else {
    	    p->exsiz = 0;
    	    p->ex[0] = 0;
    	    p->bflg = 0;
    	    q->bx = q->by = 0;
    	}
    	p->bx = q->bx;
    	p->by = q->by;
    }
    if (q->palfile) {
    	if (*(q->palfile) == '-') {
    	    p->mode |= 0x80;
    	    p->pallen = 0;
    	} else if (*(q->palfile) == '*') {
    	    if (p->pln != 4) {
    	    	printf("16色画像でないので -8オプションを無視します\n");
    	    } else {
    	    	p->mode &= 0x7f;
    	    	p->pallen = 48;
    	    	for (i = 0; i < 48; i++) {
    	    	    p->pal[i] = gPalD8[i];
    	    	}
    	    }
    	} else {
    	    if (p->pln != 4) {
    	    	printf("16色画像でないので .RGBファイルをロードしません\n");
    	    } else {
    	    	if (*(q->palfile) == '\0') {
    	    	    GetRgbFile(p->name,p->pal);
    	    	} else {
    	    	    GetRgbFile(q->palfile,p->pal);
    	    	}
    	    	p->mode &= 0x7f;
    	    	p->pallen = 48;
    	    }
    	}
    } else if (q->palcnt) {
    	p->mode &= 0x7f;
    	if ((q->palcnt==16 && p->pln!=4) || (q->palcnt==256 && p->pln!=8 )) {
    	    printf("プレーン数とパレットの数があいません\n");
    	    exit(1);
    	}
    	p->pallen = q->palcnt * 3;
    	for (i = 0; i < q->palcnt * 3; i++) {
    	    p->pal[i] = q->pal[i];
    	}
    }
    if (q->user) {
    	if (q->user[0] == '\0') {
    	    p->user = NULL;
    	    p->userlen = 0;
    	} else {
    	    p->user = q->user;
    	    p->userlen = 5 + strlen(p->user) + 2;
    	}
    }
    if (q->comment) {
    	p->comment = q->comment;
    	p->cmtlen = strlen(p->comment);
    }
    if (q->comment2) {
    	p->comment2 = q->comment2;
    	p->cmt2len = strlen(p->comment2);
    }
    p->hdrsiz = 2 + p->userlen + p->cmtlen + 1 + p->cmt2len + 1
    	    	+ 1 + 2 + 1 + 4 + 2 + p->exsiz + 2 + 2 + p->pallen;
  /*printf("size %d, dat %d\n",p->hdrsiz,p->dat);*/
}

/*-------------------------------------------------------------------------*/

static void PutC1(int c)
{
    /*printf("%02x ",c);*/
    fputc(c,gOutFp);
}

static void PutC2(word c)
{
    PutC1(c/0x100);
    PutC1(c%0x100);
}

static void PutS(byte *s)
{
    /*printf("%s ",s);*/
    fputs(s,gOutFp);
}

static void PutHdr(pihdr_t *p)
{
    int i;

    PutC1('P');
    PutC1('i');
    if (p->user) {
    	PutS("User:");
    	PutS(p->user);
    	PutC1(0x0d);
    	PutC1(0x0a);
    }
    if (p->cmtlen)
    	PutS(p->comment);
    PutC1(0x1a);
    if (p->cmt2len)
    	PutS(p->comment2);
    PutC1(0x00);
    PutC1(p->mode);
    PutC1(p->r1);
    PutC1(p->r2);
    PutC1(p->pln);
    PutC1(p->sv[0]);
    PutC1(p->sv[1]);
    PutC1(p->sv[2]);
    PutC1(p->sv[3]);
    PutC2(p->exsiz);
    if (p->exsiz) {
    	for (i = 0;i < p->exsiz; i++) {
    	    PutC1(p->ex[i]);
    	}
    }
    PutC2(p->x_siz);
    PutC2(p->y_siz);
    if (p->pallen) {
    	for (i = 0;i < p->pallen; i++) {
    	    PutC1(p->pal[i]);
    	}
    }
}

static void Copy(FILE *ifp,FILE *ofp)
{
    long l;

    for(;;) {
    	l = fread(gBuf,1,BUFSZ,ifp);
    	if (l <= 0)
    	    break;
    	fwrite(gBuf,1,(size_t)l,ofp);
    }
}

static void SetHdr(pihdr_t *p,byte *nam)
{
    static byte  nn[130];

    if (p->dat == p->hdrsiz && gOvrFlg) {
    	gOutFp = fopen_e(nam,"rb+");
    	PutHdr(p);
    	/*fseek(gOutFp,0L,SEEK_END);*/
      #ifdef FDATE
    	if (gFtimFlg)
    	    _dos_setftime(fileno(gOutFp), gFdate, gFtime);
      #endif
    	fclose(gOutFp);
    } else {
    	ChgExt(nn,nam,"$$$");
    	gOutFp = fopen_e(nn,"wb");
    	PutHdr(p);
    	gInFp = fopen_e(nam,"rb");
    	fseek(gInFp,p->dat,SEEK_SET);
    	Copy(gInFp,gOutFp);
    	fclose(gInFp);
      #ifdef FDATE
    	if (gFtimFlg)
    	    _dos_setftime(fileno(gOutFp), gFdate, gFtime);
      #endif
    	fclose(gOutFp);
    	remove(ChgExtTmp(nam,"BAK"));
    	rename_e(nam,ChgExtTmp(nam,"BAK"));
    	rename_e(nn,nam);
    	if (gOvrFlg) {
    	    remove(ChgExtTmp(nam,"BAK"));
    	}
    }
}

static void DspHdrShort(pihdr_t *p)
{
    printf("%-8s %3d %4d*%-4d %2d:%-2d %-4s %4d (%d,%d)-(%d,%d)\n",
    	FName(ChgExtTmp(p->name,NULL)),
    	(p->pln == 8) ? 256:16,
    	p->x_siz,p->y_siz,
    	p->r1,p->r2,
    	p->sv,
    	p->exsiz,
    	p->bx, p->by, p->bx + p->x_siz - 1, p->by + p->y_siz - 1
    	);
}

static void DspHdrComment(pihdr_t *p)
{
    byte tt[56];
    int i;
    byte *s;

    for (s = p->comment, i = 0; *s != '\0' && *s != '\n' && i < 50; i++, s++) {
    	tt[i] = *s;
    }
    tt[i] = '\0';
    printf("%-8s %-18s %s\n",
    	FName(ChgExtTmp(p->name,NULL)),
    	(p->user) ? p->user:"",
    	tt
    	);
}

static void DspHdr(pihdr_t *p,byte *ss)
{
    static byte *pl[] = {"ﾊﾟﾚｯﾄ情報あり","ﾃﾞﾌｫﾙﾄ･ﾊﾟﾚｯﾄ"};
    static byte *kip[] = {
    	"機種依存情報無し","未知の機種依存情報有り","始点情報有り"
    };
    int i;
    byte buf[40];

    sprintf(buf,"(%d,%d)-(%d,%d)",
    	p->bx, p->by, p->bx+p->x_siz-1, p->by+p->y_siz-1);
    printf("%6s  %-34s (ﾍｯﾀﾞｻｲｽﾞ%3dﾊﾞｲﾄ)\n",ss,p->name,p->hdrsiz);
    printf("    ｻｲｽﾞ:%4d*%-4d %3d色  ﾄﾞｯﾄ縦横比%2d:%-2d  ｾｰﾊﾞ名:%-4s\n",
    	p->x_siz,p->y_siz,
    	(p->pln != 8)?16:256,
    	p->r1,p->r2,
    	p->sv);
    printf("    %-21s %s(%dﾊﾞｲﾄ)\n",buf, kip[p->bflg], p->exsiz);

    if ((p->mode & 0x80) == 0 && p->pln == 4) {
    	printf("    n:R G B ");
    	for (i = 0; i < 16;i++) {
    	    printf("  %x:%02x %02x %02x",i,
    	    	p->pal[i*3+0],p->pal[i*3+1],p->pal[i*3+2]);
    	    if ((i % 4) == 3) {
    	    	if (i != 15)
    	    	    printf("\n            ");
    	    }
    	}
    	printf("\n");
    } else {
    	printf("    %s\n",pl[(p->mode&0x80)!=0]);
    }
    if (p->user)
    	printf("    ユーザ名:%s\n",p->user);
    if ((p->comment == NULL || *p->comment == 0) && p->user == NULL)
    	printf("    コメント無し\n");
    else
    	printf("    コメント:%s\n",p->comment);
    if (p->cmt2len) {
    	printf("    隠れコメント:%s\n",p->comment2);
    }
    printf("\n");
}


/*-------------------------------------------------------------------------*/
static void OptErr(void)
{
    printf("%s:オプションがおかしい(-? または - で説明を表示)\n",gPrgName);
    exit(1);
}

static void GetCommentFile(byte *s, byte *fnam, int sz)
{
    FILE *fp;
    byte *p;
    int  l,r;

    fp = fopen_e(fnam,"r");
    p = s;
    r = sz;
    while (r > 0) {
    	p = fgets(p,r,fp);
    	if (p == NULL || p == (void *)EOF)
    	    break;
    	/*printf("%d=%d (%d):%s",l,strlen(p),r,p);*/
    	l = strlen(p);
    	p += l;
    	r -= l;
    	if (l > 0 && *(p-1) == '\n') {
    	    *(p-1) = '\r';
    	    *p = '\n';
    	    ++l;
    	    ++p;
    	    --r;
    	    *p = '\0';
    	}
    }
    fclose (fp);
}

static void Option(byte *s, chgopt_t *q)
{
    word c;
    static char *ss;

    c = *(s++);
    switch(toupper(c)) {
    case 'T':
    	gCmd = 1;
    	break;
    case 'Z':
    	gKeyWaitFlg = 1;
    	if (*s++ == '0')
    	    gKeyWaitFlg = -1;
    	break;
    case 'B':
    	gOvrFlg = 1;
    	break;
    case 'D':
    	gCmd = 4;
    	break;
    case 'R':
    	gMakRgbFileFlg = 1;
    	break;
    case 'U':
    	gCmd = 3;
    	q->user = s;
    	break;
    case 'O':
    	gCmd = 3;
    	q->bflg = 1;
    	if (*s == '\0') {
    	    q->bx = q->by = 0;
    	} else {
    	    q->bx = (int)strtol(s,&ss,0);
    	    s = ss;
    	    if (*s != ',')
    	    	OptErr();
    	    s++;
    	    q->by = (int)strtol(s,&ss,0);
    	}
    	break;
    case 'K':
    	gCmd = 3;
    	q->bflg = -1;
    	break;
    case 'N':
    	gCmd = 3;
    	if (*s == '\0') {
    	    q->r1 = q->r2 = 0;
    	} else {
    	    q->r1 = (int)strtol(s,&ss,0);
    	    s = ss;
    	    if (*s != ',')
    	    	OptErr();
    	    s++;
    	    q->r2 = (int)strtol(s,&ss,0);
    	    if (q->r1 == 0 || q->r2 == 0)
    	    	q->r1 = q->r2 = 0;
    	}
    	break;
    case 'I':
    	gCmd = 3;
    	if (strlen(s) > 4)
    	    OptErr();
    	{
    	    int i;
    	    for (i = 0; i< 4; i++) {
    	    	if (*s)
    	    	    q->sv[i] = s[i];
    	    	else
    	    	    q->sv[i] = ' ';
    	    }
    	}
    	q->sv[4] = '\0';
    	break;
    case 'P':
    	gCmd = 3;
    	q->palfile = s;
    	break;
    case '8':
    	gCmd = 3;
    	q->palfile = "*";
    	break;
    case 'H':
    	gCmd = 3;
    	q->comment2 = s;
    	break;
    case 'C':
    	gCmd = 3;
    	if (*s == '\0')
    	    OptErr();
    	q->comment = gXComment;
    	GetCommentFile(q->comment,s,COMMENT_SIZ-2);
    	break;
    case 'L':
    	gCmd = 2;
    	if (*s == 'M' || *s == 'm') {
    	    gCmd = 3;
    	    gMagFlg = 1;
    	    gMagMsgFlg = 1;
    	    if (*++s)
    	    	gMagName = s;
    	    else
    	    	gMagName = NULL;
    	}
    	break;
    case 'M':
    	gCmd = 3;
    	gMagFlg = 1;
    	if (*s == '\0')
    	    gMagName = NULL;
    	else
    	    gMagName = s;
    	break;
  #ifdef FDATE
    case 'Y':
    	gFtimFlg = 1;
    	break;
  #endif
    case '?':
    case '\0':
    	Usage();
    default:
    	OptErr();
    }
}

static int InputYorN(void)
{
    byte buf[30];
    int c;

    for (;;) {
    	printf("変更してよろしいですか(Y or N) ? ");
    	/*c = fgetc(stdin);*/
    	buf[0] = 0;
    	fgets(buf,28,stdin);
    	c = buf[0];
    	if (c == 'y' || c == 'Y')
    	    return 1;
    	else if (c == 'n' || c == 'N')
    	    return 0;
    }
}


void One(int cmd, byte *name, pihdr_t *p, chgopt_t *q)
{
    int w;

    gInFp = fopen_e(name,"rb");
  #ifdef FDATE
    _dos_getftime(fileno(gInFp), &gFdate, &gFtime);
  #endif
    w = GetHdr(p, name);
    fclose(gInFp);
    if (w < 0)
    	return;
    if (gMakRgbFileFlg) {
    	if ((p->mode & 0x80) == 0 && (p->pln == 4))
    	    MakRgbFile(p->pal,name);
    }
    if (cmd == 1) {
    	DspHdrShort(p);
    } else if (cmd == 4) {
    	DspHdrComment(p);
    } else if (cmd == 2) {
    	DspHdr(p,"[FILE]");
    } else if (cmd == 3 && w == 0) {
    	if (gMagFlg) {
    	    if (gMagName == NULL)
    	    	RdMagHdr(name,q);
    	    else
    	    	RdMagHdr(gMagName,q);
    	    if (gMagMsgFlg)
    	    	PriMagHdr(ChgExtTmp(NULL,NULL),q);
    	}
    	if (gKeyWaitFlg >= 0)
    	    DspHdr(p,"変更前");
    	OptHdr(p,q);
    	if (gKeyWaitFlg >= 0)
    	    DspHdr(p,"変更後");
    	if (gKeyWaitFlg) {
    	    SetHdr(p,name);
    	} else if (InputYorN()) {
    	    SetHdr(p,name);
    	}
    }
}


int main(int argc, byte *argv[])
{
    static pihdr_t pih;
    static chgopt_t pichg;
    static byte keyname[130];
    byte *p;
    int i,f;
  #ifdef DIRENT
    int n;
    static byte fnambuf[130];
  #endif

    gPrgName = argv[0];
    if (argc < 2)
    	Usage();
    /* オプションを取り出す */
    for(f = i = 1; i < argc; i++) {
    	p = argv[i];
    	if (*p == '-') {
    	    Option(++p,&pichg);
    	} else if (*p == '"') { /* コメントの処理 */
    	    gCmd = 3;
    	    pichg.comment = gXComment;
    	    if (*++p) {
    	    	p = stpcpy (pichg.comment,p);
    	    	if (*(p - 1) == '"')
    	    	    --p;
    	    } else
    	    	p = pichg.comment;
    	    *p = '\0';
    	} else if (*p == '=') { /* コメントの処理 */
    	    gCmd = 3;
    	    pichg.comment = gXComment;
    	    if (*++p)
    	    	p = stpcpy (pichg.comment,p);
    	    else
    	    	p = pichg.comment;
    	    for (++i;i < argc;i++) {
    	    	*p++ = ' ';
    	    	p = stpcpy(p,argv[i]);
    	    }
    	    *p = '\0';
    	    break;
    	} else {
    	    f = 0;  /* ファイルが指定されているぞ */
    	}
    }
    if (f) {
    	if (gMagMsgFlg && gMagName) { /* -lm でmag表示のみの処理 */
    	    ChgExt(keyname,gMagName,"mag");
    	  #ifndef DIRENT
    	    memset(&pichg,0x00,sizeof pichg);
    	    RdMagHdr(keyname,&pichg);
    	    PriMagHdr(keyname,&pichg);
    	  #else
    	    if ((n = DirEntryGet(fnambuf,keyname,0)) == 0) {
    	    	do {
    	    	    memset(&pichg,0x00,sizeof pichg);
    	    	    RdMagHdr(fnambuf,&pichg);
    	    	    PriMagHdr(fnambuf,&pichg);
    	    	} while (DirEntryGet(fnambuf,NULL,0) == 0);
    	    }
    	  #endif
    	    exit(0);
    	} else {
    	    printf("%s:ファイル名が指定されていない\n",gPrgName);
    	    exit(1);
    	}
    }
    if (!gMakRgbFileFlg && !gCmd) { /* オプションが指定されていないとき */
    	gCmd = 1;   	    	    /* -t が指定されたことになる */
    }

    if (gCmd == 1) {	/* -t でのヘッダ */
    	printf("FileName Col Size      Dot   Save ExSz\n");
    	printf("-------- --- --------- ----- ---- ---- --------------------------------------\n");
    } else if (gCmd == 4) { /* -d でのヘッダ */
    	printf("FileName UserName           Comment\n");
    	printf("-------- ------------------ -------------------------------------------------\n");
    }

    /* 各ファイルの処理 */
    for(i = 1; i < argc; i++) {
    	gInFp = gOutFp = NULL;
    	p = argv[i];
    	if (*p == '-' || *p == '"')
    	    continue;
    	else if (*p == '=')
    	    break;
    	ChgExt(keyname,p,"pi");
      #ifndef DIRENT
    	One(gCmd, keyname, &pih, &pichg);
      #else /*#ifdef DIRENT */
    	n = DirEntryGetAll(keyname,0);
    	if (n > 0) {
    	    while (--n >= 0) {
    	    	One(gCmd, DirEntryNext(), &pih, &pichg);
    	    }
    	    DirEntryFreeAll();
    	} else {
    	    if (n == 0)
    	    	printf("ファイル %s がみつかりません\n", keyname);
    	    else
    	    	printf("メモリが足りません\n");
    	    return 1;
    	}
      #endif
    }
    return 0;
}
