typedef struct FIL_DIRS {
	char	 name[13];
	char	 attr;
	unsigned time;
	unsigned date;
	long	 size;
	unsigned data;	/* user data*/
} FIL_DIRS;

#if 1 /*def WILDC*/
extern int FIL_GetDirEnt(char far *,char far *,unsigned);
#endif

int FIL_GetDrive(void);
int FIL_GetCurDir(int dno, char far *curdir);
int FIL_FullPath(char *src, char dst[]);
int  FIL_GetDirs(char far *path, int atb, FIL_DIRS far *tbl, int tblsiz);
int  FIL_DirName(char far *path, char far *dir);
char *FIL_ChgExt(char onam[], char *ext);
char *FIL_AddExt(char onam[], char *ext);
char *FIL_BaseName_N(char *path);
char far *FIL_BaseNameFar(char far *path);
char *FIL_GetLt(int hdl);

/* ファイル検索のバッファ(FIL_FindFirst/next()用) */
typedef struct FIL_FIND_T {
	char reserved[21];
	char attrib;
	unsigned wr_time;
	unsigned wr_date;
	long size;
	char name[13];
} FIL_FIND_T;
unsigned FIL_FindFirst(char *PathName, unsigned Attrib, FIL_FIND_T *pfind);
unsigned FIL_FindNext(FIL_FIND_T *pfind);
/* DOSのファイルアトリビュート値 */
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_VOLID  0x08
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
