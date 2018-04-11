/*--------------------------------------------------------------------------*/
/* サブルーチンいろいろ														*/
/*--------------------------------------------------------------------------*/

#ifndef SUBR_H
#define SUBR_H

#include "def.h"

/*--------------------------------------------------------------------------*/
/* 文字列操作等 */

char *strncpyZ(char *dst, const char *src, size_t size);
long  strtolKM(char *s, char **d, int r);
char *StrSkipSpc(char *s);
char *StrDelLf(char *s);

void *memSearch(void *area0, int asiz, const void *ptn0, int ptnlen);
void *memStr(void *area0, const char *ptn, int asiz);

typedef struct slist_t {
	struct slist_t	*link;
	char			*s;
} slist_t;

slist_t *slist_add(slist_t **root, char *s);
void     slist_free(slist_t **root);


#if 0
int Vsprintf(char *dst, const char *fmt, void *args);	/* なぜか、ターゲットのライブラリに vsprintfが存在しなかった場合の交代として */
#endif
uint32_t MemCrc32(void *dat, int sz);		/* CCITT 32ビット CRC の計算 */


/*--------------------------------------------------------------------------*/

void *mallocE(size_t a);
void *reallocE(void *a, size_t b);
void *callocE(size_t a, size_t b);
char *strdupE(char *p);
int freeE(void *p);
void *mallocMa(int sz, int minSz, int algn);
void *mallocMaE(int sz, int minSz, int algn);



/*--------------------------------------------------------------------------*/
/* エラー処理、デバッグ用関数												*/

#define DBG_LOG_NAME	"dbg.txt"

/*volatile*/void err_exit(char *fmt, ...);
void dbg_printf(char *fmt, ...);

#ifndef NDEBUG		// デバッグするとき
 #ifdef __GNUC__
//  #define dbg_assert(x)	do {if ((x) == 0) {err_exit("%-14s %5d %s(): Assertion failed.\n", __FILE__, __LINE__, __FUNCTION__);}} while(0)
  #define DBG_ASSERT(x)	do {if ((x) == 0) {err_exit("%-14s %5d %s(): Assertion failed.\n", __FILE__, __LINE__, __FUNCTION__);}} while(0)
  #define DBG_M()		dbg_printf("%-14s %5d %s():\n", __FILE__, __LINE__, __FUNCTION__)
  #define DBG_S(s)		dbg_printf("%-14s %5d %s(): %s", __FILE__, __LINE__, __FUNCTION__, (s))
  #define DBG_F(x)		dbg_printf x
 #else
//  #define dbg_assert(x)	do {if ((x) == 0) {err_exit("%-14s %5d : Assertion failed.\n", __FILE__, __LINE__);}} while(0)
  #define DBG_ASSERT(x)	do {if ((x) == 0) {err_exit("%-14s %5d : Assertion failed.\n", __FILE__, __LINE__);}} while(0)
  #define DBG_M()		dbg_printf("%-14s %5d :\n", __FILE__, __LINE__)
  #define DBG_S(s)		dbg_printf("%-14s %5d : %s", __FILE__, __LINE__, (s))
  #define DBG_F(x)		dbg_printf x
 #endif
#else			// リリース時
 #define DBG_ASSERT(x)
 #define DBG_M()
 #define DBG_S(s)
 #define DBG_F(x)
#endif

/*--------------------------------------------------------------------------*/
/* ファイル関係 */

extern int  file_sjisFlag;

#define FILE_ISKANJI(c)	(file_sjisFlag && ISKANJI(c))

int	  file_setSjisMode(int sw);												// ファイル名はMS全角(SJIS)を対応する(1)か否(0)か
char *file_baseName(char *adr);
char *file_extPtr(char *name);
char *file_chgExt(char filename[], const char *ext);
char *file_addExt(char filename[], const char *ext);
char *file_delLastDirSep(char *dir);
size_t	file_size(const char *name);
int   file_rename(char *oldname, char *newname);								// rename改. newname.bakを作成してから、変名
char *file_dirNameDup(char *dir, char *name);
char *file_dirNameDupE(char *dir, char *name);
char *file_dirNameAddExt(char *nam, char *dir, char *name, char *addext);
char *file_dirNameChgExt(char *nam, char *dir, char *name, char *chgext);
char *file_dirDirNameChgExt(char *onam, const char *dir, const char *mdir, const char *name, const char *chgext);
char *file_getMidDir(char mdir[], const char *name);

int   file_getTmpDir(char *t);
int   file_getTmpDirE(char *t);
void *file_load(const char *name, void *buf, int bufsz, int *rdszp);
void *file_loadE(const char *name, void *buf, int bufsz, int *rdszp);
void *file_save(const char *name, void *buf, int size);
void *file_saveE(const char *name, void *buf, int size);
int   file_copy(char *dstfile, char *srcfile, void *buf, int bufsz);
int   file_makeDmyFile(char *oname, size_t fsiz, int fh);
void  file_makeDmyFileE(char *oname, size_t fsiz, int fh);

#ifdef __BORLANDC__
typedef struct ftime FILE_TIME;
FILE_TIME file_timeGet(char *srcname);
int 	 file_timeCmp(FILE_TIME t1, FILE_TIME t2);
int 	 file_timeSet(char *dstname, FILE_TIME fdt);
#endif /* __BORLANDC__ */


FILE *fopenMD(const char *name, char *mode);

int file_isWildC(const char *onam);
char *file_findFirstName(char dst[], const char *src);
char *file_findNextName(char dst[]);

int file_fdateCmp(const char *tgt, const char *src);


typedef struct flist_t {
	struct flist_t *link;
	char *s;
	char *d;
} flist_t;

void     flist_setMode(int mode);
void     flist_setSrcDir(const char *src);
void     flist_setSrcExt(const char *src);
void     flist_setDstDir(const char *dst);
void     flist_setDstExt(const char *dst);
flist_t *flist_add(flist_t **p0, const char *nm);



/*--------------------------------------------------------------------------*/

FILE *fopenE(char *name, char *mod);
void fcloseE(FILE *fp);
size_t  fwriteE(void *buf, size_t sz, size_t num, FILE *fp);
size_t  freadE(void *buf, size_t sz, size_t num, FILE *fp);
size_t	flength(FILE *fp);


int fgetcE(FILE *fp);
int fgetc2iE(FILE *fp);
int fgetc4iE(FILE *fp);
int fgetc2mE(FILE *fp);
int fgetc4mE(FILE *fp);
void fputcE(int c, FILE *fp);
void fputc2mE(int c, FILE *fp);
void fputc4mE(int c, FILE *fp);
void *fputsE(char *s, FILE *fp);
void fputc2iE(int c, FILE *fp);
void fputc4iE(int c, FILE *fp);



/*--------------------------------------------------------------------------*/

typedef int (*STBL_CMP)(void *s0, void *s1);
STBL_CMP STBL_SetFncCmp(STBL_CMP fncCmp);
int STBL_Add(void *t[], int *tblcnt, void *key);
int STBL_Search(void *tbl[], int nn, void *key);



/*--------------------------------------------------------------------------*/
/* 式計算 */

int StrExpr(char *s_old, char **s_new, long *val);		/* 戻り値0:no error  !0:error */
void StrExpr_SetNameChkFunc(int (*name2valFnc)(char *name, long *valp));
	/* name2valFnc は、名前が渡され、正常なら0を返しその名前の値を *valpにいれる. 異常なら-1を返す関数を設定すること */



/*--------------------------------------------------------------------------*/
/* テキスト入出力 */

int  TXT1_Open(const char *name);
void TXT1_OpenE(const char *name);
void TXT1_Close(void);
char *TXT1_GetsE(char *buf, int sz);
void TXT1_Error(char *fmt, ...);
void TXT1_ErrorE(char *fmt, ...);
extern unsigned long	TXT1_line;
extern char		TXT1_name[FILE_NMSZ];
extern FILE		*TXT1_fp;




#endif	/* SUBR_H */
