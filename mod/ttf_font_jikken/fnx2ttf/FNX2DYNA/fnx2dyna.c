/**
 *  @file   fnx2dyna.c
 *  @brief  FontX2形式のフォントファイルから、cb2ttjで使われるdyna形式の画像を生成する.
 *  @author tenk* (Masashi KITAMURA)
 *  @date   2001,2002,2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "subr.h"
#include "bmp.h"



static void dyna_fnt_save(char *name, Uint8 *buf, int w, int h);
static void fnt_resize1024x1024(Uint8 *dst, int	lw, int	lh, Uint8 *src,	int fw,	int fh);
static void fnt_resize1024x1024_2(Uint8	*dst, int lw, int lh, Uint8 *src, int fw, int fh);
static void fnt_resize1024x1024_3(Uint8	*dst, int dw, int dh, Uint8 *src, int fw, int fh);
static void fnt_resize1024x1024_4(Uint8	*dst, int dw, int dh, Uint8 *src, int fw, int fh);

static void resize_biliner(void	*dst0, int dstW, int dstH, const void *src0, int srcW, int srcH);

static void fnt_8to1(Uint8 *src, int fw, int fh, Uint8 *dst);
static void fnt_1to8(Uint8 *src, int fw, int fh, Uint8 *dst);
static void DoDo(char *name, int st, int en, int styl, int bmpFlg, char	*outname);
static int jis2sjis(int	c);
static int sjis2jis(int	c);


#define	JIS2NO(j)  (((((j) >> 8) & 0x7f) - 0x21) * 94 +	(((j) &	0x7f) -	0x21))


static int vflg	= 0;




static void Usage(void)
{
    printf(
    	"usage>fnx2dyna [-opts] fontx2name no [no2]  // v0.53 "	__DATE__ "  by tenk*\n"
    	" fontx2 形式フォントから cb2ttj用 DYNA画像生成\n"
    	" no (から、あればno2まで) のフォントを 1024x1024 にして出力\n"
    	" -v     余計なメッセージ表示\n"
    	" -mN    拡大モード。0:単純(通常). 1:ちょっと改変。\n"
    	" -b     cb2ttj用のビットマップでなくMS-WIN な BMP 出力\n"
    	" -u     存在しないフォントは生成しない\n"
    	" -j     指定番号は cb2ttj用でなく JIS(EUC)コード\n"
    	" -z     指定番号は cb2ttj用でなく シフトJISコード\n"
    	" -oFILE 出力ファイル名を FILE にする\n"
    );
    exit(1);
}



int main(int argc, char	*argv[])
{
    char    *name = NULL, *outname = NULL;
    char    *p;
    int	    c,i, no = -1, en = -1, styl	= 0, bmpFlg=0, mode = 0, uflg =	0;

    if (argc < 2)
    	Usage();

    for	(i = 1;	i < argc; i++) {
    	p = argv[i];
    	if (*p == '-') {
    	    p++;
    	    c =	*p++;
    	    c =	toupper(c);
    	    switch (c) {
    	    case 'B':
    	    	bmpFlg = (*p !=	'-');
    	    	break;
    	    case 'M':
    	    	styl = strtol(p,0,0);
    	    	break;
    	    case 'V':
    	    	vflg = 1;
    	    	if (*p)
    	    	    vflg = strtol(p,0,0);
    	    	break;
    	    case 'U':
    	    	uflg = (*p != '-');
    	    	break;
    	    case 'J':
    	    	mode = 1;
    	    	break;
    	    case 'Z':
    	    	mode = 2;
    	    	break;
    	    case 'O':
    	    	outname	= strdupE(p);
    	    	break;
    	    case '\0':
    	    	break;
    	    case 'H':
    	    case '?':
    	    	Usage();
    	    default:
    	    	printf("Incorrect command line option : %s\n", argv[i]);
    	    	return 1;
    	    }
    	} else if (name	== NULL) {
    	    name = strdupE(p);
    	} else if (no <	0) {
    	    if (ISKANJI(*p)) {
    	    	c  = ((Uint8)*p<<8) | (Uint8)p[1];
    	    	c  = sjis2jis(c);
    	    	c  = JIS2NO(c);
    	    } else {
    	    	c = strtoul(p, 0, 0);
    	    	if (mode == 2) {
    	    	    c =	sjis2jis(c);
    	    	}
    	    	if (mode) {
    	    	    c =	JIS2NO(c);
    	    	}
    	    }
    	    no = c;
    	} else if (en <	0) {
    	    if (ISKANJI(*p)) {
    	    	c  = ((Uint8)*p<<8) | (Uint8)p[1];
    	    	c  = sjis2jis(c);
    	    } else {
    	    	c = strtoul(p, 0, 0);
    	    	if (mode == 2) {
    	    	    c =	sjis2jis(c);
    	    	}
    	    	if (mode) {
    	    	    c =	JIS2NO(c);
    	    	}
    	    }
    	    en = c;
    	}
    }

    if (no < 0)	{
    	err_exit("番号が不正です(%d)\n", no);
    }
    if (en < no) {
    	en = no;
    }

    DoDo(name, no, en, styl, bmpFlg|(uflg<<1), outname);
    return 0;
}



static char 	fontName[12];
static int  	kcode_rng[256][3];
static char 	fnt[1024 * 1024	+ 4096];
static char 	pix[1024 * 1024	+ 4096];
static Uint8	wk_buf2[1024*1024+1024];
static Uint8	wk_buf3[1024*1024+1024];



static void DoDo(char *name, int st, int en, int styl, int flags, char *outname)
{
    int	    c,top, j,n,m,i,fw,fh,fsz,ty,tblNum;
    Uint8   buf[4096];
    FILE    *fp;

    //FoNTx2 ファイルのヘッダ読み込み
    fp = fopenE(name, "rb");
    fread(buf, 1,17, fp);
    if (memcmp(buf, "FONTX2", 6) != 0) {
    	err_exit("%s は Fontx2ファイルでない\n", name);
    }
    memcpy(fontName, buf+6, 8);
    fontName[8]	= 0;
    fw = buf[14];
    fh = buf[15];
    fsz	= ((fw + 7) / 8) * fh;
    if (fw == 0	|| fh == 0) {
    	err_exit("フォントサイズが不正 %d*%d\n",fw,fh);
    }
    ty = buf[16];
    memset(kcode_rng, 0, sizeof(kcode_rng));

    if (vflg) {
    	printf("[%s] %d*%d %dbytes %s\n",fontName,fw,fh,fsz,ty?"全角":"半角");
    }

    if (ty != 0) {  //2バイトコードのとき
    	tblNum = fgetc(fp);
    	fread(buf, 4, tblNum, fp);
    	n = 0;
    	for (i = 0; i <	tblNum;	i++) {
    	    kcode_rng[i][0] = PEEKiW(&buf[i*4+0]);
    	    kcode_rng[i][1] = PEEKiW(&buf[i*4+2]);
    	    kcode_rng[i][2] = n;
    	    n += kcode_rng[i][1] - kcode_rng[i][0] + 1;
    	    if (vflg > 1) {
    	    	printf("    %3d %04x-%04x (%04x)\n", i,	kcode_rng[i][0], kcode_rng[i][1], kcode_rng[i][2]);
    	    }
    	}
    	top = 18 + tblNum * 4;
    } else {	    // 1バイト文字のとき
    	//tblNum = 1;
    	//kcode_rng[0][0] =   0;
    	//kcode_rng[0][1] = 255;
    	top = 17;
    }

    // 指定文字分生成
    for	(n = st; n <= en; n++) {
    	memset(fnt, 0, fw * fh);
    	memset(pix, 0, 1024 * 1024);

    	// JISコードを求める
    	if (ty)	{
    	    j =	(((n / 94) + 0x21) << 8) | ((n % 94) + 0x21);

    	    // シフトJISコードを求める
    	    c =	jis2sjis(j);

    	    // テーブルを検索
    	    m =	-1; //見つからない場合は先頭を代用
    	    for	(i = 0;	i < tblNum; i++) {
    	    	if (kcode_rng[i][0] <= c && c <= kcode_rng[i][1]) {
    	    	    //m	= c - kcode_rng[i][0];
    	    	    m =	kcode_rng[i][2]	+ c - kcode_rng[i][0];
    	    	    break;
    	    	}
    	    }
    	    if (vflg) {
    	    	printf("%8d> JIS=0x%x SJIS=0x%x no=0x%x\n",n,j,c, m);
    	    }
    	    if (m < 0) {
    	    	if (flags & 2)
    	    	    continue;
    	    	//m = 0;
    	    }
    	} else {
    	    //c	= n;
    	    m =	n;
    	    if (vflg) {
    	    	printf("[%3d] 0x%x\n",n, m);
    	    }
    	}

    	// フォントデータ取り出し
    	if (m >= 0) {
    	    fseek(fp, top + m *	fsz, SEEK_SET);
    	    fread(buf, 1, fsz, fp);

    	    fnt_1to8(buf, fw, fh, fnt);

    	    // 変換作業
    	    switch (styl) {
    	    case 0:  fnt_resize1024x1024  (pix,	1024, 1024, fnt, fw, fh); break;
    	    case 1:  fnt_resize1024x1024_2(pix,	1024, 1024, fnt, fw, fh); break;
    	    case 2:  fnt_resize1024x1024_3(pix,	1024, 1024, fnt, fw, fh); break;
    	    default: fnt_resize1024x1024_4(pix,	1024, 1024, fnt, fw, fh); break;
    	    }
    	} else {
    	    //m	= 0;
    	    //memset(fnt, 0, fw	* fh);
    	    //memset(pix, 0, 1024 * 1024);
    	}

    	// 画像ファイル生成
    	if (outname) {
    	    if (st == en)
    	    	sprintf(buf, "%s", outname);
    	    else
    	    	sprintf(buf, "%s%04d.%s", outname, n, (flags&1)?"bmp":"dyn");
    	} else {
    	    sprintf(buf, "%04d.%s", n, (flags&1)?"bmp":"dyn");
    	}
    	if (flags & 1) {
    	    static int clut[2] = {0, 0xFFFFFF};
    	    m =	bmp_write(fnt, 1024, 1024, 1, pix, 1024, 8, clut, 1);
    	    FIL_SaveE(buf, fnt,	m);
    	} else {
    	    //fnt_8to1(pix, 1024, 1024,	fnt);
    	    dyna_fnt_save(buf, pix, 1024, 1024);
    	}
    }
    fclose(fp);
}


static void fnt_1to8(Uint8 *src, int fw, int fh, Uint8 *dst)
{
    static int msk[] = {0x80, 0x40, 0x20, 0x10,	8, 4, 2, 1};
    int	x,y;

    for	(y = 0;	y < fh;	y++) {
    	for (x = 0; x <	fw; x++) {
    	    dst[y*fw+x]	= ((src[((fw+7)/8)*y + (x >> 3)] & msk[x&7]) !=	0);
    	}
    }

    if (vflg > 2) {
    	printf("\n");
    	for (y = 0; y <	fh; y++) {
    	    for	(x = 0;	x < fw;	x++) {
    	    	printf("%s", dst[y*fw+x]?"*":"･");
    	    }
    	    printf("\n");
    	}
    }
}



static void fnt_8to1(Uint8 *src, int fw, int fh, Uint8 *dst)
{
    int	x,y, c,	k = 200;

    for	(y = 0;	y < fh;	y++) {
    	for (x = 0; x <	fw; x+=8) {
    	    c  = (x   <	fw && (src[y*fw+x+0]>k)) ? 0x80:0;
    	    c |= (x+1 <	fw && (src[y*fw+x+1]>k)) ? 0x40:0;
    	    c |= (x+2 <	fw && (src[y*fw+x+2]>k)) ? 0x20:0;
    	    c |= (x+3 <	fw && (src[y*fw+x+3]>k)) ? 0x10:0;
    	    c |= (x+4 <	fw && (src[y*fw+x+4]>k)) ? 0x08:0;
    	    c |= (x+5 <	fw && (src[y*fw+x+5]>k)) ? 0x04:0;
    	    c |= (x+6 <	fw && (src[y*fw+x+6]>k)) ? 0x02:0;
    	    c |= (x+7 <	fw && (src[y*fw+x+7]>k)) ? 0x01:0;
    	    dst[((fw+7)/8)*y + (x >> 3)] = c;
    	}
    }
}



static void fnt_resize1024x1024(Uint8 *dst, int	lw, int	lh, Uint8 *src,	int fw,	int fh)
{
    int	l, dw, dh, rw,rh, x0,y0, ofs,c,x,y,u,v;

    if (fw >= fh) {
    	l = fw;
    	u = 0;
    	v = fw-fh;
    } else {
    	l = fh;
    	u = fh-fw;
    	v = 0;
    }
    dw = lw / l;
    dh = lh / l;
    rw = lw - dw * fw;
    rh = lh - dh * fh;
    x0 = rw / 2;
    y0 = rh / 2;

    // printf("%d*%d <=	%d*%d\n", lw, lh,fw,fh);
    // printf("l=%d uv=%d,%d\n", l, u, v);
    // printf("dh,dw=%d	rh,rw=%d x0,y0=%d,%d\n",dh,rh,x0,y0);

    for	(y = 0;	y < fh;	y++) {
    	for (x = 0; x <	fw; x++) {
    	    c =	src[y*fw+x];
    	    // printf("(%d,%d)=%d\t", x,y,c);
    	    ofs	= (y0+y*dh+0)*lw + (x0+x*dw+0);
    	    for	(v = 0;	v < dh;	v++) {
    	    	for (u = 0; u <	dw; u++) {
    	    	    dst[ofs + v*lw+u] =	c;
    	    	}
    	    }
    	}
    	// printf("\n");
    }
}



static void fnt_resize1024x1024_2(Uint8	*dst, int lw, int lh, Uint8 *src, int fw, int fh)
{
    int	l, i, dw, dh, rw,rh, x0,y0, ofs,c,x,y,u,v;
    #undef S
    #define S(x,y)  ((x) >= 0 && (y) >=	0 && (x) < fw && (y) < fh && src[(y)*fw+(x)])

    if (fw >= fh) {
    	l = fw;
    	u = 0;
    	v = fw-fh;
    } else {
    	l = fh;
    	u = fh-fw;
    	v = 0;
    }
    dw = lw / l;
    dh = lh / l;
    rw = lw - dw * fw;
    rh = lh - dh * fh;
    x0 = rw / 2;
    y0 = rh / 2;

    // printf("%d,%d, %d,%d\n",	fw,fh, dw,dh);
    for	(y = 0;	y < fh;	y++) {
    	for (x = 0; x <	fw; x++) {
    	    c =	S(x,y);
    	    if (c) {
    	    	c = 0xFF;
    	    	if (S(x-1,y) ==	0 && S(x-1,y-1)	== 0 &&	S(x, y-1) == 0)	{
    	    	    c &= ~8;
    	    	}
    	    	if (S(x+1,y) ==	0 && S(x+1,y-1)	== 0 &&	S(x, y-1) == 0)	{
    	    	    c &= ~4;
    	    	}
    	    	if (S(x-1,y) ==	0 && S(x-1,y+1)	== 0 &&	S(x, y+1) == 0)	{
    	    	    c &= ~2;
    	    	}
    	    	if (S(x+1,y) ==	0 && S(x+1,y+1)	== 0 &&	S(x, y+1) == 0)	{
    	    	    c &= ~1;
    	    	}
    	    } else {
    	    	if (S(x-1,y) &&	S(x, y-1)) {
    	    	    c |= 0x8;
    	    	}
    	    	if (S(x+1,y) &&	S(x, y-1)) {
    	    	    c |= 0x4;
    	    	}
    	    	if (S(x-1,y) &&	S(x, y+1)) {
    	    	    c |= 0x2;
    	    	}
    	    	if (S(x+1,y) &&	S(x, y+1)) {
    	    	    c |= 1;
    	    	}
    	    }
    	    if (c == 0)
    	    	continue;
    	    ofs	= (y0+y*dh+0)*lw + (x0+x*dw+0);
    	    i	= dw/2;
    	    l	= (dw+1)/2;
    	    if (c & 0xF0) {
    	    	int px = 0;
    	    	int pl = l;
    	    	for (v = 0; v <	l; v++)	{
    	    	    for	(u = 0;	u < pl;	u++) {
    	    	    	dst[ofs	+ (l-1-v)*lw+(px+u)] = 0xFF;
    	    	    	dst[ofs	+ (l-1-v)*lw+(i	+u)] = 0xFF;
    	    	    	dst[ofs	+ (i+v)*lw+(px+u)] = 0xFF;
    	    	    	dst[ofs	+ (i+v)*lw+(i +u)] = 0xFF;
    	    	    }
    	    	    pl--;
    	    	    px++;
    	    	}
    	    }
    	    if (c & 0x08) {
    	    	int pl = l;
    	    	for (v = 0; v <	l; v++)	{
    	    	    for	(u = 0;	u < pl;	u++) {
    	    	    	dst[ofs	+ v*lw+u] = 0xFF;
    	    	    }
    	    	    pl--;
    	    	}
    	    }
    	    if (c & 0x04) {
    	    	int px = i;
    	    	int pl = l;
    	    	for (v = 0; v <	l; v++)	{
    	    	    for	(u = 0;	u < pl;	u++) {
    	    	    	dst[ofs	+ v*lw+px+u] = 0xFF;
    	    	    }
    	    	    pl -= 1;
    	    	    px += 1;
    	    	}
    	    }
    	    if (c & 0x02) {
    	    	int px = 0;
    	    	int pl = 1;
    	    	for (v = i; v <	dw; v++) {
    	    	    for	(u = 0;	u < pl;	u++) {
    	    	    	dst[ofs	+ v*lw+px+u] = 0xFF;
    	    	    }
    	    	    pl += 1;
    	    	}
    	    }
    	    if (c & 0x01) {
    	    	int px = dw-1;
    	    	int pl = 1;
    	    	for (v = i; v <	dw; v++) {
    	    	    for	(u = 0;	u < pl;	u++) {
    	    	    	dst[ofs	+ v*lw+px+u] = 0xFF;
    	    	    }
    	    	    pl += 1;
    	    	    px -= 1;
    	    	}
    	    }
    	}
    }
    #undef S
}



static void fnt_resize1024x1024_3(Uint8	*dst, int dw, int dh, Uint8 *src, int fw, int fh)
{
    int	fw2=fw*2, fh2=fh*2;
    // int fw2=dw/4, fh2=dh/4;
    memset(wk_buf2,0,sizeof wk_buf2);
    fnt_resize1024x1024(wk_buf2, fw2, fh2, src,	fw, fh);
    fnt_resize1024x1024_2(dst, dw, dh, wk_buf2,	fw2, fh2);
}



static void fnt_resize1024x1024_4(Uint8	*dst, int dw, int dh, Uint8 *src, int fw, int fh)
{
    int	fw2=fw *2, fh2=fh *2;
    int	fw3=fw2*2, fh3=fh2*2;

    memset(wk_buf2,0,sizeof wk_buf2);
    fnt_resize1024x1024(wk_buf2, fw2, fh2, src,	fw, fh);
    memset(wk_buf3,0,sizeof wk_buf3);
    fnt_resize1024x1024_2(wk_buf3, fw3,	fh3, wk_buf2, fw2, fh2);
    fw2	= fw3*2, fh2 = fh3*2;
    memset(wk_buf2,0,sizeof wk_buf2);
    fnt_resize1024x1024(wk_buf2, fw2, fh2, wk_buf3, fw3, fh3);
    //x	fw3 = fw2*2, fh3 = fh2*2;
    fnt_resize1024x1024_2(dst, dw, dh, wk_buf2,	fw2, fh2);
}



static void dyna_fnt_save(char *name, Uint8 *buf, int w, int h)
{
    FILE *fp;
    int	x,y,l,s;

    fp = fopenE(name, "wb");
    fwrite("DYNA", 1, 4, fp);
    for	(y = 0;	y < h; y++) {
    	s = 0;
    	l = 0;
    	for (x = 0; x <	w; x++)	{
    	    if (buf[y*w	+ x]) {
    	    	l++;
    	    } else {
    	    	if (l >	0) {
    	    	    fputc2iE(y,	fp);
    	    	    fputc2iE(s,	fp);
    	    	    fputc2iE(s + l, fp);
    	    	    l =	0;
    	    	}
    	    	s = x;
    	    }
    	}
      #if 1 // 2007-05-16 これを忘れていたため、どっとが右端に接した場合、それが欠落してしまうバグになっていた...m(_ _)m
    	if (l >	0) {
    	    fputc2iE(y,	fp);
    	    fputc2iE(s,	fp);
    	    fputc2iE(s + l, fp);
    	    l =	0;
    	}
      #endif
    }
    fclose(fp);
}



static int jis2sjis(int	c)
{
    /* ネットのどっかで控えたメモ（初出失念)
    // コードから 0x2121 を引く.
    // (コード & 0x100)	が 0 でなければ	コードに 0x9E, 0 ならばコードに	0x40 を足す.
    // 下位バイトが 0x7F 以上ならば コードに 1 を足す.
    // [上位バイトだけの処理] 上位バイトを右に１ビット シフトし、それに	0x81 を足す.
    // (0xA000 <= コード) ならばコードに 0x4000	を足す.
    */
    c -= 0x2121;
    if (c & 0x100)
    	c += 0x9e;
    else
    	c += 0x40;
    if ((unsigned char)c >= 0x7f)
    	c++;
    c =	(((c >>	(8+1)) + 0x81)<<8) | ((unsigned	char)c);
    if (c >= 0xA000)
    	c += 0x4000;
    return c;
}


static int sjis2jis(int	c)
{
    /* ネットのどっかで控えたメモ（初出失念)
    //コード >=	0xE000 ならばコードから	0x4000 を引く
    //[上位バイトのみ] 上位バイトから 0x81 を引き １ビット左にシフトする
    //下位バイトが 0x80	以上ならコードから１を引く
    //下位バイトが 0x9E	以上ならコード全体に 0x62(0x100-0x9E) を足す。
    //そうでなければコードから0x40 を引く .
    //コードに 0x2121 を足す。
    */
    if (c >= 0xE000)
    	c -= 0x4000;
    c =	(((c>>8) - 0x81)<<9) | (unsigned char)c;
    if ((unsigned char)c >= 0x80)
    	c -= 1;
    if ((unsigned char)c >= 0x9e)
    	c += 0x62;
    else
    	c -= 0x40;
    c += 0x2121;
    return c;
}



/* fontx2

○ASCII/全角共通
+0x00 6	id:"FONTX2"
+0x06 8	nm:フォントの名前
+0x0E 1	fw:フォントの横幅. ピクセル数 1～255
+0x0F 1	fh:フォントの縦幅. ピクセル数 1～255
+0x10 1	ty:  0:ASCIIフォント   1:全角フォント

○ASCIIフォントの場合。
+0x11 より、文字コード 0x00～0xFFまでの	256文字分の
フォントパターン・データが並んで置かれる。
１文字のデータは 2値で無圧縮。
１文字のサイズの計算は

 ((fw+7)/8) * fh.

横幅はバイト単位になるよう1ラインごとに、ビットに左から詰められ、
8ビット(1バイト)に満たない場合は 0が詰められる。それが行数分ある
（つまり、9x10も 14x10 もデータの格納サイズは同じになる）。


○全角フォントの場合。

サポートしている（ファイルにフォントパターンが入っている）文字の
範囲を現すテーブルが +0x11 より置かれる。文字コードはシフトJIS(MS
全角)で、範囲は、開始と終了、の対を複数持つことになる。
文字コードの値の格納方法はリトルエンディアン（インテル形式）。

+0x11	   1  tn:範囲の数(1～255)
+0x12	   2  1つ目の文字コード範囲の開始(含)
+0x14	   2  1つ目の文字コード範囲の終了(含)
  :
  :
+0x0E+tn*4 2 最後(tn番目)の文字コード範囲の開始
+0x10+tn*4 2 最後(tn番目)の文字コード範囲の終了

フォントパターンの形式はASCIIフォントと同じ。
 */
