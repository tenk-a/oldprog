/*
	Q0 を読み込んで、色数をカウントし、256色以下ならTIFFを生成
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"

/*#undef	FDATEGET*/

typedef long		PIXEL;		/* 1ピクセルを収める型 */

/*---------------------------------------------------------------------------*/

void Usage(void)
{
	printf("dcol v0.50    RGB,Q0 -> 256色 TIFF コンバータ\n");
	printf("usage: dcol [-opts] file(s)[.rgb] [-opts]\n");
	printf(" -o<FILE>  出力ファイルを FILE にする（１ファイルのみ）\n");
	printf(" -r        RGB入力\n");
	printf(" -q0       Q0 入力\n");
	printf(" -e        予め r:g:b=0:0:0 の色を使用済みにする(透明色等を想定)\n");
	printf(" -pr<N>    赤のビット数. N=1～8.  省略時 8\n");
	printf(" -pg<N>    緑のビット数. N=1～8.  省略時 8\n");
	printf(" -pb<N>    青のビット数. N=1～8.  省略時 8\n");
	printf(" -a<M,N>   アスペクト比(横:縦)を M:N にする. M,N <= 255\n");
	printf(" -6        アスペクト比をx68kな値にする.\n");
	/*printf(" -c<N>     24ﾋﾞｯﾄ色画像を強制的にNﾋﾞｯﾄ色画像にする. N=15,16のみ\n");*/
  #if 0 //def FDATEGET
	printf(" -d[-]     入力ファイルの日付も複写. -d- で日付複写しない\n");
  #endif
	printf("\n");
	printf("rgb,q0を入力し色数を数え256色以内なら256色TIFを作成します\n");
	printf("rgb,q0 入力では、まず.falファイルを探し、なければ .iprファイルを探し、それで\n");
	printf("なけば、640*400の画像として扱います\n");
	exit(0);
}


/*--------------------------------------------------------------------------*/
/*- 日付保持のために使用 ----------------------------------------------------*/
#ifdef FDATEGET	/*MS-C/QC系の手続き名^^;*/
#if 0 /*def __TURBOC__*/
	#include <dos.h>

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
  /*printf("\nhno=%x fdate=%4x ftime=%4x\n",hno,dat,tim);*/
	return 0;
}

unsigned _dos_getftime(int hno, unsigned *dat, unsigned *tim)
{
	union REGS reg;

	reg.x.ax = 0x5700;
	reg.x.bx = hno;
	intdos(&reg, &reg);
	if (reg.x.flags & 1) {	/* reg.x.flags はTCに依存 */
		return (unsigned)(errno = reg.x.ax);
	}
	*tim = reg.x.cx;
	*dat = reg.x.dx;
  /*printf("\nhno=%x fdate=%4x ftime=%4x\n",hno,*dat,*tim);*/
	return 0;
}
#endif
#endif

/*---------------------------------------------------------------------------*/

unsigned char *FileBaseName(unsigned char *adr)
{
	unsigned char *p;

	p = adr;
	while (*p != '\0') {
		if (*p == ':' || *p == '/' || *p == '\\') {
			adr = p + 1;
		}
		if (iskanji(*p) && *(p+1) ) {
			p++;
		}
		p++;
	}
	return adr;
}

BYTE *FIL_ChgExt(BYTE filename[], BYTE *ext)
{
	BYTE *p;

	p = strrchr(filename, '/');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr(p, '\\');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr( p, '.');
	if (p == NULL) {
		strcat(filename,".");
		strcat( filename, ext);
	} else {
		strcpy(p+1, ext);
	}
	return filename;
}

BYTE *FIL_AddExt(BYTE filename[], BYTE *ext)
{
	BYTE *p;

	p = strrchr(filename, '/');
	if ( p == NULL) {
		p = filename;
	}
	p = strrchr(p, '\\');
	if ( p == NULL) {
		p = filename;
	}
	if ( strrchr( p, '.') == NULL) {
		strcat(filename,".");
		strcat(filename, ext);
	}
	return filename;
}

void *calloc_m(size_t nobj, size_t sz)
{
	void *p;
	p = calloc(nobj,sz);
	if(p == NULL) {
		printf("必要なメモリ(%d*%d)を確保できないよお(T^T)\n",nobj,sz);
	}
	return p;
}

FILE *TmpCreate(BYTE *name, BYTE **tmpname)
	/*name にﾃﾞｨﾚｸﾄﾘ名付ﾌｧｲﾙ名を入れて呼び出すと、そのﾃﾞｨﾚｸﾄﾘにﾃﾝﾎﾟﾗﾘな		*/
	/*名前でﾌｧｲﾙを書き込みopenし、成功すればそのﾌｧｲﾙﾎﾟｲﾝﾀを返す。			*/
	/*失敗すれば名前を少し変名して再度openをためし、数回試しみて駄目ならNULL*/
	/*を返す. 復帰時にはtmpnameに実際に生成したﾌｧｲﾙ名を入れて返す.	*/
	/*tmpnameはmallocで確保される*/
{
	BYTE *p;
	FILE *fp = NULL;
	int n;

	*tmpname = calloc_m(strlen(name)+13,1);
		if (*tmpname == NULL) {
			return NULL;
		}
		strcpy(*tmpname, name);
	p = FileBaseName(*tmpname);
	for (n = 0; n < 10; n+=1) {
		sprintf(p,"p2t$$%03d.bak",n);
		fp = fopen(*tmpname,"wb");
		if (fp) {
			break;
		}
	}
	return fp;
}

void FIL_ReanemeBak(BYTE *oldname, BYTE *newname)
	/* ﾌｧｲﾙ名をrenameする。newnameがすでに存在したときはそれを".bak"しておく*/
{
	/*if (oldname && newname) {*/
		BYTE nbuf[FNAMESIZE+20];
		 /*出力名と同じ名前があればrename*/
		 strcpy(nbuf,newname);
		 FIL_ChgExt(nbuf,"bak");
		 remove(nbuf);
		 rename(newname,nbuf);
		 /*仮名をちゃんとした名前に変名*/
		 rename(oldname, newname);
	/*}*/
}

FILE *fopen_m(char *name, char *mode)
{
	FILE *fp;

	fp = fopen(name,mode);
	if (fp == NULL) {
		printf("%s をオープンできないよお(T^T)\n",name);
	}
	return fp;
}

size_t fwriteE(const void *p, size_t siz, size_t n, FILE *fp)
{
	size_t nn;
	nn = fwrite(p,siz,n,fp);
	if (nn < n) {
		printf("ファイル書き込みでエラー\n");
		exit(1);
	}
	return nn;
}


void Asp2Resol(int asp1, int asp2, long *resolX, long *resolY)
	/* PIC のアスペクト比を元にTIFFの解像度を決める */
	/* TIFF(BMP)で、PICのアスペクト比を保存するのを目的にしているので */
	/* 解像度としては不当な値になるかも^^; */
{
  #if 0
	int f;
  #endif

	*resolX = *resolY = 100;
	while (((asp1 & 0x01) == 0) && ((asp2 & 0x01) == 0)) {
		asp1 >>= 1;
		asp2 >>= 1;
	}
	if (asp1 == 0 || asp2 == 0) {
		printf("アスペクト比がおかしい\n");
		return;
	}
	if (asp1 == asp2) {
		return;
	}
  #if 0
	f = 0;
	if (asp1 < asp2) {
		f = asp1;
		asp1 = asp2;
		asp2 = f;
		f = 1;
	}
  #endif

	if (asp1 <= 5 && asp2 <= 5) {
		asp1 *= 50;
		asp2 *= 50;
	} else if (asp1 <= 10 && asp2 <= 10) {
		asp1 *= 25;
		asp2 *= 25;
	} else if (asp1 <= 50 && asp2 <= 50) {
		asp1 *= 10;
		asp2 *= 10;
	}

  #if 1
	*resolX = asp2;
	*resolY = asp1;
  #else
	if (f == 0) {
		*resolX = asp2;
		*resolY = asp1;
	} else {
		*resolX = asp1;
		*resolY = asp2;
	}
  #endif
}
/*--------------------------------------------------------------------------*/
int pbR, pbG, pbB;
int pb[8] = {0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0xFE,0xFF};

#define Get1(p) ((p[0]&pbR)*0x100L+(p[1]&pbG)*0x10000L+(p[2]&pbB)*0x1L)


int DCOL_Tbl2Rgb(PIXEL *pall, BYTE *rgb)
{
	int i;
	for (i = 0; i < 256; i++) {
		rgb[i*3+0] = (pall[i] >> 8) & 0xff;
		rgb[i*3+1] = (pall[i] >> 16) & 0xff;
		rgb[i*3+2] = (pall[i] >> 0) & 0xff;
	}
	return 0;
}

static int DCOL_Cmp(const void *p1, const void *p2)
{
	long l;
	l =  ( *(PIXEL *)p1 - *(PIXEL *)p2 );
	if (l > 0L)
		return 1;
	else if (l < 0L)
		return -1;
	return 0;
}

static int DCOL_Search(PIXEL *pall, int pall_n, PIXEL c)
	/*
	 *  c:さがす色
	 *  pall:文字列へのポインタをおさめた配列
	 *  pall_n:配列のサイズ
	 *  復帰値:見つかった文字列の番号(0より)  みつからなかったとき-1
	 */
{
	int  low,mid,nn;
	long f;

	nn = pall_n;
	low = 0;
	while (low < nn) {
		mid = (low + nn - 1) / 2;
		f = c - pall[mid];
		if ( f < 0)
			nn = mid;
		else if (f > 0)
			low = mid + 1;
		else
			return mid;
   }
   return -1;
}

int DCOL_Pass1(FILE *fp, int x,int y, PIXEL *pall, BYTE *rbuf, int b255flg)
{
	int n;
	PIXEL c;
	BYTE *p;
	int pall_n;

	pall_n = 0;
	if (b255flg) {		/* 透明色を強制追加 */
		pall[0] = 0;
		pall_n++;
	}
	while (--y >= 0) {
		n = fread(rbuf,3,x,fp);
		if (n != x) {
			pall_n = -1;
			goto RET;
		}
		p = rbuf;
		while(--n >= 0) {
			c = Get1(p); p+= 3;
			if (pall_n == 0)
				goto J1;
			if (DCOL_Search(pall,pall_n, c) < 0) {
				if (pall_n > 255) {
					goto RET;
				}
		  J1:
				pall[pall_n++] = c;
				qsort(pall, pall_n, sizeof(PIXEL), DCOL_Cmp);
			}
		}
	}
  RET:;
	return pall_n;
}

int DCOL_Pass2(FILE *fp, FILE *ofp, int x, int y, PIXEL *pall, int pall_n,
				BYTE *rbuf, BYTE *wbuf)
{
	int n;
	PIXEL c;
	BYTE *p,*s;

	while (--y >= 0) {
		n = fread(rbuf,3,x,fp);
		if (n != x) {
			return 2;
		}
		s = rbuf;
		p = wbuf;
		while (--n >= 0) {
			c = Get1(s); s+= 3;
			*p++ = (BYTE)DCOL_Search(pall,pall_n,c);
		}
		n = fwrite(wbuf,1,x,ofp);
		if (n != x) {
			return 3;
		}
	}
	return 0;
}
#if 0
DCOL_Cnv(FILE *fp, FILE *ofp, int x, int y)
{
	static PIXEL pall[256];
	#define RBUFSZ	(3 * 1024 * 4)
	#define WBUFSZ	(1 * 1024 * 4)
	static BYTE rbuf[RBUFSZ];
	static BYTE wbuf[WBUFSZ];
	int pall_n;
	long pos;

	/*読み込み開始位置を保存*/
	pos = ftell(fp);
	if (pos < 0) {
		return 2;/*read error*/
	}
	/*取り敢えず読み来んで色数かうんと*/
	pall_n = DCOL_Pass1(fp,x,y, pall, rbuf,b255flg);
	if (pall_n > 255) {
		return 1;/*over 255 color*/
	} else if (pall_n < 0) {
		return 2;/*read error*/
	}
	/*読み込み位置を元に戻す*/
	fseek(fp,pos,SEEK_SET);
	/*変換*/
	if (DCOL_Pass2(fp,ofp,x,y,pall,pall_n, rbuf, wbuf)) {
		return 3;/*write error*/
	}
}
#endif

/*---------------------------------------------------------------------------*/
/*							R G B(Q0)入 力									 */
/*---------------------------------------------------------------------------*/
/*module Q0*/
/* export Q0, Q0_Open, Q0_CloseR, Q0_GetLine */

#define	Q0_RDBUFSIZ	0x4000

typedef struct Q0_T {
	FILE *fp;
	BYTE *name;
	int  colbit;
	int  sizeX,sizeY;
	int  startX,startY;
	int  asp1, asp2;
	BYTE *rgb;
/* private: */
  #if 0
	BYTE *linebuf;
  #endif
} Q0;

static void Q0_Free(Q0 *q0p)
{
	if (q0p->fp) {
		fclose (q0p->fp);
	}
	if (q0p->name) {
		free(q0p->name);
	}
  #if 0
	if (q0p->linebuf) {
		free(q0p->linebuf);
	}
  #endif
	if (q0p->rgb) {
		free(q0p->rgb);
	}
	free(q0p);
}

void Q0_CloseR(Q0 *q0p)
{
	Q0_Free(q0p);
}

Q0 *Q0_Open(BYTE *name)
{
	FILE *fp;
	BYTE buf[FNAMESIZE+2];
	Q0 *q0p;

	q0p = calloc_m(1,sizeof(Q0));
	if (q0p == NULL) {
		return NULL;
	}
	q0p->colbit= 24;
	q0p->sizeX = 640;
	q0p->sizeY = 400;
	q0p->startX = 0;
	q0p->startY = 0;
	q0p->asp1  = 1;
	q0p->asp2  = 1;
	q0p->rgb   = NULL;
	q0p->name  = strdup(name);

	/*falファイル出力 */
	strcpy(buf,name);
	FIL_ChgExt(buf,"FAL");
	fp = fopen(buf,"r");
	if (fp) {
		printf("RGB(Q0)情報ファイル:%s\n",buf);
		fgets(buf,FNAMESIZE,fp);
	} else {
		strcpy(buf,name);
		FIL_ChgExt(buf,"IPR");
		fp = fopen(buf,"r");
		if (fp) {
			printf("RGB(Q0)情報ファイル:%s\n",buf);
		}
	}
	if (fp) {
		fgets(buf,FNAMESIZE,fp);
		sscanf(buf,"%d %d %d %d",
			&q0p->sizeX,&q0p->sizeY,&q0p->startX,&q0p->startY);
	}
	if (q0p->sizeX <= 0 || q0p->sizeY <= 0
		|| q0p->startX < 0 || q0p->startY < 0) {
		printf("情報ファイルの内容がおかしいです(%d,%d,%d,%d)\n",
			q0p->sizeX,q0p->sizeY,q0p->startX,q0p->startY);
		exit(1);
	}
	fclose(fp);

	/*q0ファイル オープン*/
	q0p->name = strdup(name);
	q0p->fp = fopen_m(name,"rb");
	if (q0p->fp == NULL) {
		goto ERR;
	}
	setvbuf(q0p->fp,NULL, _IOFBF, Q0_RDBUFSIZ);
  #if 0
	q0p->linebuf = calloc_m(q0p->sizeX,3);
	if (q0p->linebuf == NULL) {
		goto ERR;
	}
  #endif
	return q0p;

  ERR:
	Q0_Free(q0p);
	return NULL;
}

/*----------------------------------*/
#if 0
void Q0_GetLine(Q0 *q0p, BYTE *buf)
	/* PIC_PutLines から呼ばれることになる関数 */
{
	int i;

	fread(buf, 3, q0p->sizeX, q0p->fp);
  #if 0
	for (i = 0; i < q0p->sizeX; i++, buf+=3) {	/* R G B -> G R B */
		BYTE r,g,b;
		r = buf[0];
		g = buf[1];
		b = buf[2];
		buf[0] = g;
		buf[1] = r;
		buf[2] = b;
	}
  #endif
}
#endif

/*end Q0*/
/*---------------------------------------------------------------------------*/
/*							T I F F 出 力									 */
/*---------------------------------------------------------------------------*/
/*module TIF*/
	/*export TIF, TIF_Create, TIF_CloseW, TIF_PutLine */

#define TIF__BYTE	1
#define TIF__ASCIIZ 2
#define TIF__WORD	3
#define TIF__DWORD	4
#define TIF__RAT	5

typedef struct TIF_T {
	FILE *fp;
	BYTE *name;
	BYTE *tmpname;
	int  colbit;
	int  sizeX, sizeY;
	int  startX, startY;
	long resolX, resolY;
	BYTE *rgb;
/* private: */
	BYTE *linebuf;
} TIF;

static void TIF_Free(TIF *tf)
{
	if (tf->fp) {
		fclose(tf->fp);
	}
	if (tf->name) {
		free(tf->name);
	}
	if (tf->tmpname) {
		free(tf->tmpname);
	}
  #if 0
	if (tf->rgb) {
		free(tf->rgb);
	}
  #endif
	if (tf->linebuf) {
		free(tf->linebuf);
	}
	free(tf);
}

void TIF_CloseW(TIF *tf)
{
	if (tf->tmpname) {
		fclose(tf->fp); tf->fp = NULL;
		FIL_ReanemeBak(tf->tmpname,tf->name);
	}
	TIF_Free(tf);
}

static int TIF_ocnt = 0, TIF_id = 0;

static void TIF_InitPut(int tifID)
{
	TIF_ocnt = 0;
	TIF_id = tifID;
}

static void TIF_PutB(TIF* tf, int ch)
{
	fputc(ch & 0xff, tf->fp);
	++TIF_ocnt;
}

static void TIF_PutW(TIF* tf, WORD w)
{
	if (TIF_id == 0) {
		TIF_PutB(tf, w & 0xff);
		TIF_PutB(tf, w >> 8);
	} else {
		TIF_PutB(tf, w >> 8);
		TIF_PutB(tf, w & 0xff);
	}
}

static void TIF_PutD(TIF* tf, DWORD d)
{
	if (TIF_id == 0) {
		TIF_PutW(tf, (WORD)(d & 0xffff));
		TIF_PutW(tf, (WORD)(d >> 16));
	} else {
		TIF_PutW(tf, (WORD)(d >> 16));
		TIF_PutW(tf, (WORD)(d & 0xffff));
	}
}

static void TIF_PutTag(TIF* tf, int tag, int typ, long cnt, DWORD dat)
{
	TIF_PutW(tf, tag);
	TIF_PutW(tf, typ);
	TIF_PutD(tf, cnt);
	switch (typ) {
	case TIF__BYTE:
	case TIF__ASCIIZ:
		if (cnt == 1) {
			TIF_PutB(tf, (BYTE)dat&0xff);TIF_PutB(tf, 0);TIF_PutW(tf,0);
		} else if (cnt == 2) {
			TIF_PutB(tf,(BYTE)(dat>>8)&0xff);TIF_PutB(tf,(BYTE)(dat)&0xff);
			TIF_PutW(tf,0);
		} else if (cnt == 3) {
			TIF_PutB(tf,(BYTE)(dat>>16)&0xff);TIF_PutB(tf,(BYTE)(dat>>8)&0xff);
			TIF_PutB(tf,(BYTE)(dat)&0xff);TIF_PutB(tf,0);
		} else if (cnt == 4 && typ != TIF__ASCIIZ) {
			TIF_PutB(tf,(BYTE)(dat>>24)&0xff);TIF_PutB(tf,(BYTE)(dat>>16)&255);
			TIF_PutB(tf,(BYTE)(dat>>8)&0xff);TIF_PutB(tf,(BYTE)dat&0xff);
		} else {
			TIF_PutD(tf,dat);
		}
		break;
	case TIF__WORD:
		if (cnt == 1) {
			TIF_PutW(tf,(WORD)dat&0xffff); TIF_PutW(tf, 0);
		} else if (cnt == 2) {
			TIF_PutW(tf,(WORD)(dat>>16)&0xffff); TIF_PutW(tf,(WORD)dat&0xffff);
		} else {
			TIF_PutD(tf,dat);
		}
		break;
	default:
		TIF_PutD(tf, dat);
	}
}

TIF *TIF_Create(BYTE *name, int colbit, int szX, int szY,
				 long resolX, long resolY, BYTE *rgb, int tifID)
	/* tifID 0=II(ｲﾝﾃﾙ)  1=MM(ﾓﾄﾛｰﾗ) */
{
	TIF *tf;
	int i,townsFlg;

	tf = calloc_m(1,sizeof(TIF));
	if (tf == NULL) {
		return NULL;
	}
	
	tf->name = strdup(name);
	tf->colbit= colbit;
	if (colbit == 4) {
	  #if 1
		if (szX & 0x01) {
			printf("16色TIFFで横幅が奇数なので誤変換します(横幅を強制的に偶数化)\n");
			szX++;
		}
	  #endif
	}
	tf->sizeX = szX;
	tf->sizeY = szY;
	tf->resolX = resolX;
	tf->resolY = resolY;
	tf->rgb   = rgb;

	tf->fp = TmpCreate(name, &tf->tmpname);
	if (tf->fp == NULL) {
		goto ERR;
	}
	setvbuf(tf->fp,NULL, _IOFBF, WRTBUFSIZE);
	TIF_InitPut(tifID);

	if (tifID == 0) {
		TIF_PutW(tf, 'I'*0x100+'I');
	} else {
		TIF_PutW(tf, 'M'*0x100+'M');
	}
	TIF_PutW(tf, 42);
	TIF_PutD(tf, 8);

	townsFlg = 0;
	if (tifID == 0 && (colbit == 8 || colbit == 24)){
		townsFlg = 1;
	}
	TIF_PutW( tf, townsFlg + 8 + 6 + ((colbit<24 && rgb) ? 1 : 0) );
	if (townsFlg) {
		TIF_PutTag(tf, 0x0FE, TIF__DWORD, 1, 0);
	}

	if (colbit < 24) {
		TIF_PutTag(tf, 0x100, TIF__WORD, 1, szX);	/* Image Width TAG*/
		TIF_PutTag(tf, 0x101, TIF__WORD, 1, szY);	/* Image Length TAG*/
		TIF_PutTag(tf, 0x102, TIF__WORD, 1, colbit);/*+ Bits Per Sample TAG*/
		TIF_PutTag(tf, 0x103, TIF__WORD, 1, 1); 	/* compression TAG*/
		if (rgb == NULL) {
			TIF_PutTag(tf, 0x106, TIF__WORD, 1, 1); /*+ color */
		} else {
			TIF_PutTag(tf, 0x106, TIF__WORD, 1, 3);	/*+ palette */
		}
		TIF_PutTag(tf, 0x10A, TIF__WORD, 1, 1); 	/* Fill Order. TAG*/
		if (rgb == NULL) {							/* Photo.Interp. TAG*/
			TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x100); /*+ Strip Offset */
		} else {
			if (colbit == 4) {
				TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x200); /*+ Strip Offset*/
			} else {
				TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x700); /*+ Strip Offset*/
			}
		}
		TIF_PutTag(tf, 0x115, TIF__WORD, 1, 1);		/*+ Sample Per Pixel TAG*/
		TIF_PutTag(tf, 0x118, TIF__WORD,1,0);		/*+MinSampleValue*/
		TIF_PutTag(tf, 0x119, TIF__WORD,1,(0x01<<colbit)-1);/*+MaxSampleValue*/
		TIF_PutTag(tf, 0x11A, TIF__RAT, 1, 0xF0);	/* X Resolution TAG*/
		TIF_PutTag(tf, 0x11B, TIF__RAT, 1, 0xF8);	/* Y Resolution TAG*/
		TIF_PutTag(tf, 0x11C, TIF__WORD, 1, 1); 	/* Planer Config. TAG*/
		TIF_PutTag(tf, 0x128, TIF__WORD, 1, 2); 	/* Resolution Unit TAG*/
		if (rgb && colbit <= 256) {
			int col;
			col = 0x01 << colbit;
			TIF_PutTag(tf, 0x140, TIF__WORD, col*3, 0x100);	/*+ pal. ofs. */
		}
		TIF_PutD(tf, 0L);

		if (TIF_ocnt > 0xf0) {
			printf("PRGERR:TIFF-HEADER\n");
		}
		for (i = TIF_ocnt; i < 0xF0; i++) {
			TIF_PutB(tf,0);
		}
		TIF_PutD(tf,75);TIF_PutD(tf,1);
		TIF_PutD(tf,75);TIF_PutD(tf,1);
		if (rgb) {
			if (colbit == 4) {
				for (i = 0; i < 16*3; i+=3) {
					TIF_PutW(tf, rgb[i]*0x100+rgb[i]);
				}
				for (i = 0; i < 16*3; i+=3) {
					TIF_PutW(tf, rgb[i+1]*0x100+rgb[i+1]);
				}
				for (i = 0; i < 16*3; i+=3) {
					TIF_PutW(tf, rgb[i+2]*0x100+rgb[i+2]);
				}
				for (i = 16*2*3;i < 0x100;i++) {
					TIF_PutB(tf, 0x00);
				}
			} else if (colbit == 8) {
				for (i = 0; i < 256*3; i+=3) {
					TIF_PutW(tf, rgb[i]*0x100+rgb[i]);
				}
				for (i = 0; i < 256*3; i+=3) {
					TIF_PutW(tf, rgb[i+1]*0x100+rgb[i+1]);
				}
				for (i = 0; i < 256*3; i+=3) {
					TIF_PutW(tf, rgb[i+2]*0x100+rgb[i+2]);
				}
			}
		}
		/* 変換用バッファ確保 */
		tf->linebuf = calloc_m(tf->sizeX, 1);
		if (tf->linebuf == NULL) {
			goto ERR;
		}
	} else {	/* full color */
		TIF_PutTag(tf, 0x100, TIF__WORD, 1, szX);	/* Image Width TAG*/
		TIF_PutTag(tf, 0x101, TIF__WORD, 1, szY);	/* Image Length TAG*/
		TIF_PutTag(tf, 0x102, TIF__WORD, 3, 0xDE);	/*+ Bits Per Sample TAG*/
		TIF_PutTag(tf, 0x103, TIF__WORD, 1, 1); 	/* compression TAG*/
		TIF_PutTag(tf, 0x106, TIF__WORD, 1, 2); 	/*+ full color */
		TIF_PutTag(tf, 0x10A, TIF__WORD, 1, 1); 	/* Fill Order. TAG*/
		TIF_PutTag(tf, 0x111, TIF__DWORD, 1, 0x100);/* Strip Offset */
		TIF_PutTag(tf, 0x115, TIF__WORD, 1, 3);	    /*+ Sample Per Pixel TAG*/
		TIF_PutTag(tf, 0x118, TIF__WORD, 3, 0xE4);	/*+MinSampleValue*/
		TIF_PutTag(tf, 0x119, TIF__WORD, 3, 0xEA);	/*+ Max Sample Value */
		TIF_PutTag(tf, 0x11A, TIF__RAT, 1, 0xF0);	/* X Resolution TAG*/
		TIF_PutTag(tf, 0x11B, TIF__RAT, 1, 0xF8);	/* Y Resolution TAG*/
		TIF_PutTag(tf, 0x11C, TIF__WORD, 1, 1); 	/* Planer Config. TAG*/
		TIF_PutTag(tf, 0x128, TIF__WORD, 1, 2); 	/* Resolution Unit TAG*/
		TIF_PutD(tf, 0L);

		if (TIF_ocnt > 0xDE) {
			printf("PRGERR:TIFF-HEADER\n");
		}
		for (i = TIF_ocnt; i < 0xDE; i++) {
			TIF_PutB(tf,0);
		}
		TIF_PutW(tf,8);TIF_PutW(tf,8);TIF_PutW(tf,8);	/* Bits Per Sample */
		TIF_PutW(tf,0x00);TIF_PutW(tf,0x00);TIF_PutW(tf,0x00);/* Min.Smpl.Val*/
		TIF_PutW(tf,0xFF);TIF_PutW(tf,0xFF);TIF_PutW(tf,0xFF);/* Max.Smpl.Val*/
		TIF_PutD(tf,resolX);TIF_PutD(tf,1); 			/* X Resolution */
		TIF_PutD(tf,resolY);TIF_PutD(tf,1); 			/* Y Resolution */
		/* 変換用バッファ確保 */
		tf->linebuf = calloc_m(tf->sizeX, 3);
		if (tf->linebuf == NULL) {
			goto ERR;
		}
	}
	return tf;

  ERR:
	TIF_Free(tf);
	return NULL;
}

#if 0
/*------------------------*/

void TIF_PutLine(TIF *tf, PIXEL *buf)
	/* PIC_PutLines から呼ばれることになる関数 */
{
	int i;
	PIXEL col;
	BYTE *p;

	p = tf->linebuf;
	if (tf->colbit == 4) {
		for (i = 0; i < tf->sizeX; i+=2) {
			*p++ = (BYTE)( ((buf[i] >> 20)&0xF0) | ((buf[i+1] >> 24)&0x0f) );
		}
		fwriteE(tf->linebuf, 1, tf->sizeX / 2, tf->fp);
	} else if (tf->colbit <= 8) {
		for (i = 0; i < tf->sizeX; i++) {
			*p++ = (BYTE)(buf[i] >> 24);
		}
		fwriteE(tf->linebuf, 1, tf->sizeX, tf->fp);
	} else {
		for (i = 0; i < tf->sizeX; i++) {
			col = buf[i];
			*p++ = (col 	 ) & 0xFF;	/* R */
			*p++ = (col >>	8) & 0xFF;	/* G */
			*p++ = (col >> 16) & 0xFF;	/* B */
		}
		fwriteE(tf->linebuf, 3, tf->sizeX, tf->fp);
	}
}
#endif
/*end TIF*/


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#ifdef FDATEGET
  	static int  fdateFlg = 1;	/* 元ﾌｧｲﾙの日付もコピー */
#endif


static void PriInfo(TIF *tif, int rdFmtNo,BYTE *srcName, BYTE *dstName)
	/* 変換メッセージ */
{
	static BYTE *srcExt[] = {"TIF","BMP","RGB","Q0",""};
	int startX,startY;

	startX = tif->startX; if (startX < 0) startX = 0;
	startY = tif->startY; if (startY < 0) startY = 0;
	printf("[%3s->TIF] %s -> %s\n",srcExt[rdFmtNo], srcName, dstName);
	printf("           ｻｲｽﾞ%4d*%-4d\n",tif->sizeX,tif->sizeY);
}


int ConvPdata(BYTE *srcName, BYTE *dstName,int rdFmtNo, int x68kflg,
				 int colbit, int asp1, int asp2, int b255flg)
{
  #ifdef FDATEGET
	static unsigned fdat, ftim;
  #endif
	TIF *tif;
	Q0	*q0p;
	static PIXEL pall[256];
	static BYTE rgb[3*256];
	#define RBUFSZ	(3 * 1024 * 4)
	#define WBUFSZ	(1 * 1024 * 4)
	static BYTE rbuf[RBUFSZ];
	static BYTE wbuf[WBUFSZ];
	int pall_n,n;
	long pos;

	if (x68kflg) {
		asp1 = 130;
		asp2 = 90;
	} else {
		asp1 = 100;
		asp2 = 100;
	}

		/* Q0ファイル(ヘッダ)作成 */
		q0p = Q0_Open(srcName);
		if (q0p == NULL) {
			return 2;
		}
  #ifdef FDATEGET
		/* ファイル日付取得 */
		if (fdateFlg) {
			_dos_getftime(fileno(q0p->fp), &fdat, &ftim);
  		}
  #endif
  		/*
		if (q0p->colbit <= 8 || colbit == 0) {
			colbit = q0p->colbit;
		}
		*/
		colbit = 8;

		pos = ftell(q0p->fp);
		if (pos < 0) {
			Q0_CloseR(q0p);
			return 2;/*read error*/
		}
		/*取り敢えず読み来んで色数かうんと*/
		printf("色数カウント開始\n");
		pall_n = DCOL_Pass1(q0p->fp,q0p->sizeX,q0p->sizeY, pall, rbuf,b255flg);
		if (pall_n > 255) {
			return 1;/*over 255 color*/
		} else if (pall_n < 0) {
			return 2;/*read error*/
		}
		printf("  色数=%d\n",pall_n);
		DCOL_Tbl2Rgb(pall,rgb);
		/*読み込み位置を元に戻す*/
		fseek(q0p->fp,pos,SEEK_SET);
		printf("tiff作成\n");
		/*TIFF Create*/
		tif = TIF_Create(dstName,colbit,q0p->sizeX,q0p->sizeY,
							asp1,asp2,rgb,0);
		if (tif == NULL) {
			Q0_CloseR(q0p);
			return 3;
		}
		PriInfo(tif, rdFmtNo,srcName,dstName);		/* 変換メッセージ */
		/*変換*/
		n = DCOL_Pass2(q0p->fp,tif->fp,q0p->sizeX,q0p->sizeY,pall,pall_n,
				rbuf, wbuf);
		if (n) {
			TIF_CloseW(tif);
			Q0_CloseR(q0p);
			return n;
		}

	  #ifdef FDATEGET
		if (fdateFlg) {
			_dos_setftime(fileno(tif->fp), fdat, ftim);
		}
	  #endif
		TIF_CloseW(tif);
		Q0_CloseR(q0p);
	return 0;
}

/*---------------------------------------------------------------------------*/


#ifdef KEYBRK
void KeyBrk(int sig)
	/* Stop-Key , ctrl-c 割り込み用*/
{
	printf("Abort.\n");
	sig = 1;
	exit(sig);
}
#endif

int main(int argc, char *argv[])
{
  #ifdef DIRENTRY
	int DirEntryGet(BYTE far *fname, BYTE far *wildname, int fmode);
	static BYTE nambuf[FNAMESIZE+2];
  #endif
	static BYTE srcName[FNAMESIZE+2];
	static BYTE dstName[FNAMESIZE+2];
	static BYTE *srcExt = "RGB";
	int  i,c;
	int  sw_oneFile;
	int  rdFmtNo;				/* 0:tiff 1:bmp 2:rgb 3:q0 */
	int  colbit;
	int  x68kflg;
	int  asp1,asp2,b255flg;
	char *p;

  #ifdef KEYBRK
	signal(SIGINT,KeyBrk);
  #endif
	srcName[0] = dstName[0] = 0;
	x68kflg = asp1 = asp2 = colbit = sw_oneFile = b255flg = 0;
	pbR = pbG = pbB = 8;
	rdFmtNo = 2;

	if (argc < 2) {
		Usage();
	}
	/* オプション読み取り */
	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			p++;
			c = toupper(*p);
			p++;
			switch (c) {
			case 'O':
				strncpy(dstName,p,FNAMESIZE);
				sw_oneFile = 1;
				break;
			case 'E':
				b255flg = 1;
				break;
			case 'A':
				if (*p == 0) {
					asp1 = asp2 = 100;
				} else {
					asp1 = (int)strtol(p,&p, 0);
					if (*p == ',') {
						asp2 = (int)strtol(p+1,NULL,0);
					}
				}
				break;
			case 'P':
				if (*p == 'B' || *p == 'b') {
					pbB = (int)strtol(p+1,NULL,10);
					if (pbB < 1 || pbB > 8)
						printf("-pbN の N は 1～8\n"),exit(1);
				} else if (*p == 'R' || *p == 'r') {
					pbR = (int)strtol(p+1,NULL,10);
					if (pbR < 1 || pbR > 8)
						printf("-prN の N は 1～8\n"),exit(1);
				} else if (*p == 'G' || *p == 'g') {
					pbG = (int)strtol(p+1,NULL,10);
					if (pbG < 1 || pbG > 8)
						printf("-pgN の N は 1～8\n"),exit(1);
				}
				break;
			case '6':
				x68kflg = 1;
				break;
			/*case 'T':
				rdFmtNo = 0;
				srcExt = "TIF";
				break;
			case 'B':
				rdFmtNo = 1;
				srcExt = "BMP";
				break;*/
			case 'R':
				rdFmtNo = 2;
				srcExt = "RGB";
				break;
			case 'Q':
				if (*p == '0') {
					rdFmtNo = 3;
					srcExt = "Q0";
				} else {
					goto OPTERR;
				}
				break;
  #ifdef FDATEGET
  			case 'D':
  				fdateFlg = 1;
  				if (*p == '-') {
  					fdateFlg = 0;
  				}
  				break;
  #endif
			case '?':
			case '\0':
				Usage();
				break;
			default:
	  OPTERR:
				printf("unkown option -%s",p-1);
				exit(1);
			}
		}
	}
	pbB = pb[(pbB-1)&0x7];
	pbR = pb[(pbR-1)&0x7];
	pbG = pb[(pbG-1)&0x7];
	
	/* ファイルごとの処理 */
	for (i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-') {
			continue;
		}
		strcpy(srcName,p);
		FIL_AddExt(srcName,srcExt);
  	  #ifdef DIRENTRY
		if (DirEntryGet(nambuf,srcName,0) == 0) {
			strcpy(srcName,nambuf);
			do {
	  #endif
				if (sw_oneFile == 0) {
					FIL_ChgExt(strcpy(dstName, srcName), "TIF");
					c = ConvPdata(srcName,dstName,rdFmtNo,x68kflg,colbit,
						asp1,asp2,b255flg);
					if (c == 1) {
						printf("256色以上使っています\n");
					} else if (c == 2) {
						printf("読み込みでおかしい\n");
						return c;
					} else if (c == 3) {
						printf("書き込みでおかしい\n");
						return c;
					} else if (c < 0) {
						printf ("何かエラー発生\n");
						return c;
					}
				} else {	/* -o */
					FIL_AddExt(strcpy(dstName, dstName), "TIF");
					c = ConvPdata(srcName,dstName,rdFmtNo,x68kflg,colbit,
						asp1,asp2,b255flg);
					if (c == 1) {
						printf("256色以上使っています\n");
					} else if (c == 2) {
						printf("読み込みがおかしい\n");
					} else if (c == 3) {
						printf("書き込みがおかしい\n");
					} else if (c < 0) {
						printf ("何かおかしい\n");
					}
					return c;
				}
	  #ifdef DIRENTRY
			} while(DirEntryGet(srcName,NULL,0) == 0);
		}
	  #endif
	}
	return c;
}
