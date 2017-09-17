/*--------------------------------------------------------------------------*/
/* �T�u���[�`�����낢��														*/
/*--------------------------------------------------------------------------*/

#ifndef SUBR_H
#define SUBR_H

#include "def.h"

/*--------------------------------------------------------------------------*/
/* �����񑀍쓙 */

char *strncpyZ(char *dst, char *src, size_t size);
long  strtolKM(char *s, char **d, int r);
char *StrSkipSpc(char *s);
char *StrDelLf(char *s);

void *MemSearch(void *area0, int asiz, void *ptn0, int ptnlen);

typedef struct slist_t {
	struct slist_t	*link;
	char			*s;
} slist_t;

slist_t *slist_add(slist_t **root, char *s);
void     slist_free(slist_t **root);


#if 0
int Vsprintf(char *dst, const char *fmt, void *args);	/* �Ȃ����A�^�[�Q�b�g�̃��C�u������ vsprintf�����݂��Ȃ������ꍇ�̌��Ƃ��� */
#endif
uint32_t MemCrc32(void *dat, int sz);		/* CCITT 32�r�b�g CRC �̌v�Z */


/*--------------------------------------------------------------------------*/

void *mallocE(size_t a);
void *reallocE(void *a, size_t b);
void *callocE(size_t a, size_t b);
char *strdupE(char *p);
int freeE(void *p);
void *mallocMa(int sz, int minSz, int algn);
void *mallocMaE(int sz, int minSz, int algn);



/*--------------------------------------------------------------------------*/
/* �G���[�����A�f�o�b�O�p�֐�												*/

#define DBG_LOG_NAME	"dbg.txt"

/*volatile*/void err_exit(char *fmt, ...);
void dbg_printf(char *fmt, ...);

#ifndef NDEBUG		// �f�o�b�O����Ƃ�
 #ifdef __GNUC__
  #define dbg_assert(x)	do {if ((x) == 0) {err_exit("%-14s %5d %s(): Assertion failed.\n", __FILE__, __LINE__, __FUNCTION__);}} while(0)
  #define DBG_M()		dbg_printf("%-14s %5d %s():\n", __FILE__, __LINE__, __FUNCTION__)
  #define DBG_S(s)		dbg_printf("%-14s %5d %s(): %s", __FILE__, __LINE__, __FUNCTION__, (s))
  #define DBG_F(x)		dbg_printf x
 #else
  #define dbg_assert(x)	do {if ((x) == 0) {err_exit("%-14s %5d : Assertion failed.\n", __FILE__, __LINE__);}} while(0)
  #define DBG_M()		dbg_printf("%-14s %5d :\n", __FILE__, __LINE__)
  #define DBG_S(s)		dbg_printf("%-14s %5d : %s", __FILE__, __LINE__, (s))
  #define DBG_F(x)		dbg_printf x
 #endif
#else			// �����[�X��
 #define dbg_assert(x)
 #define DBG_M()
 #define DBG_S(s)
 #define DBG_F(x)
#endif

/*--------------------------------------------------------------------------*/
/* �t�@�C���֌W */

extern int  FIL_sjisFlag;
#define FIL_ISKANJI(c)	(FIL_sjisFlag && ISKANJI(c))

int	  FIL_SetSjisMode(int sw);												// �t�@�C������MS�S�p(SJIS)��Ή�����(1)����(0)��
char *FIL_BaseName(char *adr);
char *FIL_ExtPtr(char *name);
char *FIL_ChgExt(char filename[], char *ext);
char *FIL_AddExt(char filename[], char *ext);
char *FIL_DelLastDirSep(char *dir);
int   FIL_Rename(char *oldname, char *newname);								// rename��. newname.bak���쐬���Ă���A�ϖ�
char *FIL_DirNameDup(char *dir, char *name);
char *FIL_DirNameDupE(char *dir, char *name);
char *FIL_DirNameChgExt(char *nam, char *dir, char *name, char *chgext);
char *FIL_DirNameAddExt(char *nam, char *dir, char *name, char *addext);

int   FIL_GetTmpDir(char *t);
int   FIL_GetTmpDirE(char *t);
void *FIL_Load(char *name, void *buf, int bufsz, int *rdszp);
void *FIL_LoadE(char *name, void *buf, int bufsz, int *rdszp);
void *FIL_Save(char *name, void *buf, int size);
void *FIL_SaveE(char *name, void *buf, int size);
int   FIL_Copy(char *dstfile, char *srcfile, void *buf, int bufsz);
int   FIL_MakeDmyFile(char *oname, size_t fsiz, int fh);
void  FIL_MakeDmyFileE(char *oname, size_t fsiz, int fh);

#ifdef __BORLANDC__
typedef struct ftime FIL_TIME;
FIL_TIME FIL_TimeGet(char *srcname);
int 	 FIL_TimeCmp(FIL_TIME t1, FIL_TIME t2);
int 	 FIL_TimeSet(char *dstname, FIL_TIME fdt);
#endif /* __BORLANDC__ */



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
/* ���v�Z */

int StrExpr(char *s_old, char **s_new, long *val);		/* �߂�l0:no error  !0:error */
void StrExpr_SetNameChkFunc(int (*name2valFnc)(char *name, long *valp));
	/* name2valFnc �́A���O���n����A����Ȃ�0��Ԃ����̖��O�̒l�� *valp�ɂ����. �ُ�Ȃ�-1��Ԃ��֐���ݒ肷�邱�� */



/*--------------------------------------------------------------------------*/
/* �e�L�X�g���o�� */

int  TXT1_Open(char *name);
void TXT1_OpenE(char *name);
void TXT1_Close(void);
char *TXT1_GetsE(char *buf, int sz);
void TXT1_Error(char *fmt, ...);
void TXT1_ErrorE(char *fmt, ...);
extern unsigned long	TXT1_line;
extern char		TXT1_name[FIL_NMSZ];
extern FILE		*TXT1_fp;




#endif	/* SUBR_H */
