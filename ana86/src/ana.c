/*---------------------------------------------------------------------------*/
/*
 *	ANA86	8086用ｱｾﾝﾌﾞﾗ･ﾌﾟﾘﾌﾟﾛｾｯｻ
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#define EXT
#include "anadef.h"

#define TITLE	"ana86 開発途上版(L1) Ver. 0.20 (" __DATE__ ")  by M.Kitamura"

/*#define BUF_SIZE 1030 */
#define IMP_FNAM_MAX 72

static byte_fp oSegSrc[2][4] = {
	{"_TEXT", "_DATA", "_BSS", "_STACK"},
	{"TEXT", "DATA", "BSS", "XSTACK"}
};
static byte_fp oSegS[4];
static St_t_fp oSegC_sp;
static St_t_fp oStart_sp;
static byte oSegC_flg;
static byte oImpFlg;
static byte *oAsmExt[2] = {"ASM", "A86"};
static byte Opt_lsic;
static byte Ana_comModel;
static int Ana_model0;
static byte Opt_bssClassFlg = 1;

/*----------------------------- sub.c ---------------------------------------*/
#if 0
void	rename_e(char *old_nam, char *new_nam)
{
	if (rename(old_nam, new_nam)) {
		eputs(old_nam);
		eputs(" を ");
		eputs(new_nam);
		eputs(" に変名できません\n");
		exit(errno);
	}
}
#endif

#if 1

void	fopen_errmsg(char *name,char *mode)
{
 /* fprintf(stderr,"%s %d :", Ch_srcName, Ch_line); */
	if (*mode == 'w')
		fprintf(stderr, "'%s'をクリエートできません\n", name);
	else
		fprintf(stderr, "'%s'をオープンできません\n", name);
	exit(errno);
}

FILE   *fopen_e(char *name, char *mode)
{
	FILE   *fp;

	if (mode == NULL)
		fp = NULL;
	else
		fp = fopen(name, mode);
	if (fp == NULL) {
		fopen_errmsg(name,mode);
	}
	return fp;
}

FILE   *freopen_e(char *name, char *mode, FILE *fp)
{

	if (mode == NULL)
		fp = NULL;
	else
		fp = freopen(name, mode, fp);
	if (fp == NULL) {
		fopen_errmsg(name,mode);
	}
	return fp;
}

#endif

#if 1
void	fputs_e(char *buf, FILE *fp)
{
	int 	k;

	k = fputs(buf, fp);
	if (k < 0)
		ep_exit("出力でエラーが発生した\n");
}

#endif


void	ChgDirExt(char onam[], char *inam, char *dir, char *ext)
{
	char   *np;
	char   *s;
	char   *p;

	np = inam;
	if (*np == '.')
		np++;
	if (*np == '.')
		np++;
	for (s = np; *s != '\0'; ++s) {
		if (iskanji(*s)) {
			if (*(++s) == '\0')
				break;
		} else if (*s == ':' || *s == '/' || *s == '\\') {
			np = s + 1;
		}
	}
	p = onam;
	if (dir && inam != onam) {
		if (*dir)
			p = stpcpy(p, dir);
		if (*(p - 1) == '\\' || *(p - 1) == '/')
			p--;
		if (*dir)
			*p++ = '\\';
		s = np;
		while (*s != '\0' && *s != '.')
			*p++ = *s++;
	} else {
		s = inam;
		if (*s == '.')
			*p++ = *s++;
		if (*s == '.')
			*p++ = *s++;
		while (*s != '\0' && *s != '.')
			*p++ = *s++;
	}
	if (ext) {
		if (*ext) {
			*p++ = '.';
			p = stpcpy(p, ext);
		}
	} else {
		if (*s == '.') {
			while (*s != '\0')
				*p++ = *s++;
		}
	}
	*p = '\0';
	for (p = onam; *p; p++) {
		if (*p == '/')
			*p = '\\';
	}
}


/*---------------------------------------------------------------------------*/
static void OutExtrnName(St_t_fp sp)
{
	byte   *s;

	if (sp == NULL)
		return;
	if (sp->n.lnk[0])
		OutExtrnName(sp->n.lnk[0]);
	if (sp->n.lnk[1])
		OutExtrnName(sp->n.lnk[1]);
	if (sp->v.tok == T_MODULE) {
		if (sp->v.grp == Mod_sp || sp->v.grp->v.st == St_root || sp->v.grp->v.st == St_expoRoot)
			/*OutExtrnName(St_expoRoot)*/;
		else
			OutExtrnName(sp->v.grp->v.st);
		return;
	}
	/*printf("%s\n",Str_StName(sp));*/
	if (sp->v.flg2 & FL_USE) {
		sp->v.flg2 &= (~FL_USE);
	  #if 1
		if ((sp->v.flg2 & FL_EXTERN) == 0)	///
			return;							///
	  #endif
		fprintf(Out_file, (Opt_partFlg ? "global %s" : "extrn %s"), Str_StName(sp));
		if (Opt_r86 == 0) {
			if (sp->v.tok == T_PROC_DECL || (Opt_partFlg && sp->v.tok == T_PROC)) {
				s = (sp->v.flg2 & FL_FAR) ? "far" : "near";
				goto J1;
			} else if (sp->v.tok == T_VAR) {
				switch (sp->v.siz) {
				case 1:
					s = "byte";
					break;
				case 2:
					s = "word";
					break;
				case 4:
					s = "dword";
					break;
				default:
					s = "BYTE";
					break;
				}
			  J1:
				fprintf(Out_file, ":%s", s);
			}
		}
		fputc('\n', Out_file);
	}
}

void	Out_DAssume(void)
{
	if (Opt_r86 == 0) {
		fprintf(Out_file, "\tassume\tds:%s,es:NOTHING", Mod_grps[1]);
		if (Ana_model != 2 && Ana_model != 4)
			fprintf(Out_file, ",ss:%s", Mod_grps[1]);
		fputc('\n', Out_file);
	}
}

static void OutSegBgn(St_t_fp sp)
{
	static byte *align[] =
	{"byte", "word", "para", "page"};
	static byte *combi[] =
	{"public", "private", "common", "stack", "at"};
	static byte *lsiseg[2] =
	{"%s\tcseg %s %s ", "%s\tdseg %s %s "};
	static byte *lsicombi[] =
	{"", "", "", "stack", "at"};

	fprintf(Out_file, "\n");
	if (Opt_r86) {
#if 1
		fprintf(Out_file, lsiseg[sp != gSeg_sp[GS_CODE]],
				Str_StName(sp), align[sp->c.align - 1], lsicombi[sp->c.combi]);
		if (sp->c.combi == 4)
			fprintf(Out_file, " 0%xh ", (word) sp->c.val);
		if ((strcmp(sp->p.et->b.buf, "BSS") == 0) && Opt_lsic) {
			fprintf(Out_file, "\n");
		} else
			fprintf(Out_file, "'%s'\n", sp->p.et->b.buf);
#else
		if (strcmp(sp->p.et->b.buf, "STACK") == 0) {
			fprintf(Out_file, "%s\tdseg stack 'STACK'\n", Str_StName(sp));
		} else
			fprintf(Out_file, "%s\tdseg\n", Str_StName(sp));
#endif
	} else {
		byte_fp s;
		s = Str_StName(sp);
		if (sp == gSeg_sp[GS_CODE])
			s = oSegS[GS_CODE];
		fprintf(Out_file, "%s\tsegment %s %s ",
				s, align[sp->c.align - 1], combi[sp->c.combi]);
		if (sp->c.combi == 4)
			fprintf(Out_file, " 0%xh ", (word) sp->c.val);
		fprintf(Out_file, "'%s'\n", sp->p.et->b.buf);
	}
}

static void OutSegEnd(St_t_fp sp)
{
	byte_fp s;

	if (Opt_r86 == 0 && sp) {
		s = Str_StName(sp);
		if (sp == gSeg_sp[GS_CODE])
			s = oSegS[GS_CODE];
		fprintf(Out_file, "%s\tends\n", s);
	}
}

void	Out_Seg(St_t_fp sp)
{
	if (oSegC_sp != sp || oSegC_flg) {
		oSegC_flg = 0;
		OutSegEnd(oSegC_sp);
		oSegC_sp = sp;
		OutSegBgn(sp);
	}
}

static void DeclSeg(void)
{
	St_t_fp sp;
	Et_t_fp ep;
	word	t;
	long	val;
	word	seg;
	byte	f;

	f = 0;
	seg = I_FAR;
	if (!(Ana_mode == MD_EXPO || Ana_mode == MD_MODULE))
		goto ERR;
	if ((t = Sym_Get()) == T_IDENT) {
		if ((sp = Sym_NameNew(Sym_name, T_SEG, Ana_mode)) == NULL)
			goto ERR;
		Et_NEWp(ep);
		sp->p.et = ep;
		sp->v.grp = Mod_sp;
		f = 1;
	} else if (Sym_tok == T_SEG) {
		sp = Sym_sp;
	} else if (Sym_tok == I_VAR) {
		sp = gSeg_sp[GS_DATA];
		seg = 0;
	} else if (Sym_tok == I_CVAR) {
		sp = gSeg_sp[GS_CODE];
		seg = I_CS;
	} else
		goto ERR;
	if (Sym_Get() == I_CLN) {
		if (Sym_Get() != T_SEG2)
			goto ERR;
		seg = Sym_reg;
		Sym_Get();
	}
	sp->v.seg = seg;
 /* if (Sym_tok != I_COMMA) goto ERR; */

	if (Sym_tok == I_PAGE)
		sp->c.align = 4;
	else if (Sym_tok == I_PARA)
		sp->c.align = 3;
	else if (Sym_tok == I_BYTE || Sym_tok == I_WORD)
		sp->c.align = Sym_tok;
	else if (Sym_tok == I_COMMA && f == 0)
		goto J0;
	else
		goto ERR;
	Sym_Get();
	if ((Sym_tok == I_CR || Sym_tok == I_SMCLN) && f == 0)
		goto RTS;
	else if (Sym_tok != I_COMMA)
		goto ERR;
  J0:
	switch (Sym_Get()) {
	case I_PUBLIC:
		sp->c.combi = 0;
		goto J1;
	case I_PRIVATE:
		sp->c.combi = 1;
		goto J1;
	case I_COMMON:
		sp->c.combi = 2;
		goto J1;
	case I_STACK:
		sp->c.combi = 3;
	  J1:
		Sym_Get();
		break;
	case I_COMMA:
		if (f)
			goto ERR;
		goto J2;
	default:
		val = Expr_Cnstnt();
		if (val == ERR_VAL || val < 0 || val > 0xffffU)
			goto ERR;
		sp->c.val = val;
		sp->c.combi = 4;
	}
	if ((Sym_tok == I_CR || Sym_tok == I_SMCLN) && f == 0)
		goto RTS;
	else if (Sym_tok != I_COMMA)
		goto ERR;
  J2:
	Sym_Get();
	if (Sym_tok != T_STRING || Sym_strLen > 11)
		goto ERR;
	strcpy(ep->b.buf, Sym_str);
	Sym_GetChkEol();
  RTS:
	if (t == I_VAR) {
		St_t_fp p;
		p = gSeg_sp[GS_BSS];
		p->c.align = sp->c.align;
		p->c.combi = sp->c.combi;
		p->c.val = sp->c.val;
		p->c.seg = sp->c.seg;
	}
	oSegC_flg = 1;
 /* oSegC_sp = NULL; */
	return;
  ERR:
	Msg_Err("segment 宣言の指定がおかしい");
	Sym_SkipCR();
}


static void DeclOrg(void)
{
	long	val;

	Sym_Get();
	val = Expr_Cnstnt();
	Sym_ChkEol();
	Out_Seg(gSeg_sp[GS_CODE]);
	fprintf(Out_file, "\torg\t0%lxh\n", val);
}


static void DeclModel(int i)
{
	static byte cstr[40 + LBL_NAME_SIZ];

	oSegS[GS_CODE] =			/* "_TEXT"; */
		Mod_grps[0] = oSegSrc[Opt_lsic][GS_CODE];
	Mod_grps[1] = "DGROUP";
	Mod_grps[2] = "";
	if (i > 2) {
		if (Mod_sp) {
		/* sprintf(cstr,"%s%s_TEXT",Mod_sp->g.modname,Mod_sp->g.sep); */
			sprintf(cstr, "%s_TEXT", Mod_sp->g.modname);
			Mod_grps[0] =
				oSegS[GS_CODE] = cstr;
		}						/* else Mod_grps[0] = oSegS[GS_CODE]; */
	} else if (i == 0) {
		Mod_grps[0] = Mod_grps[1];
	}
}

static void OutGroup(int i)
{
 /* if (i) { fprintf(Out_file,"%s\tgroup\t%s\n",Mod_grps[0],oSegS[GS_CODE]); } */
	fprintf(Out_file, "%s\tgroup\t%s,%s", Mod_grps[1],
			oSegS[GS_DATA], oSegS[GS_BSS]);
	if (oStart_sp && (i == 1 || i == 3))
		fprintf(Out_file, ",%s", oSegS[GS_STACK]);
	if (i == 0)
		fprintf(Out_file, ",%s", oSegS[GS_CODE]);
	fprintf(Out_file, "\n");
}

static void RsvLblSet2(void)
{
	St_t_fp sp;
	Et_t_fp ep;

	if ((sp = Sym_NameNew(oSegS[GS_DATA], T_SEG, MD_EXPO)) == NULL)
		goto ENDF;

	sp->c.align = 2;
	Et_NEWp(ep);
	sp->p.et = ep;
	strcpy(ep->b.buf, "DATA");
	gSeg_sp[GS_DATA] = sp;
	if ((sp = Sym_NameNew(oSegS[GS_CODE], T_SEG, MD_EXPO)) == NULL)
		goto ENDF;
	sp->c.align = 1;
	Et_NEWp(ep);
	sp->p.et = ep;
	sp->v.seg = I_CS;
	strcpy(ep->b.buf, "CODE");
	gSeg_sp[GS_CODE] = sp;
	if ((sp = Sym_NameNew(oSegS[GS_BSS], T_SEG, MD_EXPO)) == NULL)
		goto ENDF;
	sp->c.align = 2;
	Et_NEWp(ep);
	sp->p.et = ep;
	if (Opt_bssClassFlg)
		strcpy(ep->b.buf, "BSS");
	else
		strcpy(ep->b.buf, "DATA");
	gSeg_sp[GS_BSS] = sp;
	if ((sp = Sym_NameNew(oSegS[GS_STACK], T_SEG, MD_EXPO)) == NULL)
		goto ENDF;
	sp->c.align = 3;			/* para */
	sp->c.combi = 3;
	Et_NEWp(ep);
	sp->p.et = ep;
	sp->v.seg = I_SS;
	strcpy(ep->b.buf, "STACK");
	gSeg_sp[GS_STACK] = sp;
  ENDF:;
}

void	DeclReOpen(void)
{
	byte_fp p;
	word	c;
	byte	nam[NAM_SIZE];
	byte	nam2[NAM_SIZE];

	c = Ch_SkipSpc();
	if (Sym_NameGet(c)) {
		Msg_Err("part の指定がおかしい\n");
		return;
	}
	if (Decl_defFlg == 0 || Opt_partFlg == 0)
		return;
	ChgDirExt(nam, Sym_name, Ana_dirName, oAsmExt[Opt_r86]);
	p = nam;
	Sym_GetChkEol();
	OutSegEnd(oSegC_sp);
 /* OutExtrnName(St_root); */
 /* OutExtrnName(St_expoRoot); */
	Out_Line("\tend");
	Out_file = freopen_e(p, "a", Out_file);
	if (Ana_partLstFile) {
		ChgDirExt(nam2, p, "", "");
		fprintf(Ana_partLstFile, "%s\t", nam2);
	}
	if (Rsp_file) {
		ChgDirExt(nam2, p, "", "");
		fprintf(Rsp_file, " &\n+%s", nam2);
	}
	if (Opt_cpu && Opt_r86 == 0)
		fprintf(Out_file, "\t.%c86\n", '0' + Opt_cpu);
	OutGroup(Ana_model);
	fprintf(Out_file, "\tinclude %s\n", Ana_partIncName);
	OutSegBgn(gSeg_sp[GS_CODE]);
	oSegC_sp = gSeg_sp[GS_CODE];
	if (Opt_r86 == 0)
		fprintf(Out_file, "\tassume\tcs:%s\n", oSegS[GS_CODE]);
	Out_DAssume();
	Out_Seg(gSeg_sp[GS_DATA]);
	Out_Seg(gSeg_sp[GS_BSS]);
}

static int ImportFile(byte * fnam)
{
	int 	i, dirflg, ef;
	word	k, c;

	ef = dirflg = i = 0;
	k = Ch_SkipSpc();
	switch (k) {
	case '<':
		k = '>';
		dirflg = 1;
	case '"':
		if ((c = Ch_GetK()) == '.') {
			i = strlen(fnam);
		} else if (c == k)
			goto ERR;

		do {
			if (c == '/')
				c = '\\';
			if (c > 0xff) {
				fnam[i++] = c / 0x100;
				fnam[i++] = (byte) c;
			} else if (c >= 0x20 && c <= 0x7e || c >= 0xa0 && c <= 0xef) {
				fnam[i++] = c;
				if (c == '.')
					ef = 1;
			} else {
				goto ERR;
			}
		} while ((c = Ch_GetK()) != k && i < IMP_FNAM_MAX - 4);
		fnam[i] = '\0';
		if (ef == 0) {
			fnam[i++] = '.';
			strcpy(&(fnam[i]), Ana_hdrext);
			i += strlen(Ana_hdrext);
		}
		if (dirflg)
			i += strlen(Ana_incDir);
		if (i > IMP_FNAM_MAX)
			Msg_Err("import でのファイル名が長すぎる");
		Sym_Get();
		return dirflg;
	}
  ERR:
	return -1;
}


static int DeclImportSub(byte * fnam, St_t_fp sp, int dirflg)
{
	byte	fnam2[IMP_FNAM_MAX + 2];
	int 	f;
	St_t_fp sav_St_root;
	St_t_fp sav_St_expoRoot;
	St_t_fp sav_Mod_sp;
	FILE   *sav_Ch_file;
	byte_fp sav_Ch_srcName;
	word	sav_Ch_line;
	word	sav_Opt_comment;
	word	sav_Ana_mode;
	word	sav_Ana_model;
	word	sav_Decl_defFlg;
	word	sav_Opt_cpu;
#ifdef MVAR
	word	sav_gMvarOfs;
#endif

	Ch_Top();
#ifdef MVAR
	sav_gMvarOfs	= gMvarOfs;
#endif
	sav_Ana_mode	= Ana_mode;
	sav_Ana_model	= Ana_model;
	Ana_model		= Ana_model0;
	sav_Opt_cpu		= Opt_cpu;
	sav_Opt_comment = Opt_comment;
	sav_St_expoRoot = St_expoRoot;
	sav_St_root 	= St_root;
	sav_Mod_sp		= Mod_sp;
	sav_Ch_srcName	= Ch_srcName;
	sav_Ch_line		= Ch_line;
	sav_Ch_file		= Ch_file;
	sav_Decl_defFlg	= Decl_defFlg;

	Decl_defFlg	= 0;

	Opt_comment	= 0;
	Ch_srcName	= fnam;
	Ch_line		= 0;
	Ch_file		= fopen(fnam, "r");
	if (Ch_file == NULL) {
		ChgDirExt(fnam2, fnam, NULL, Ana_ext);
		Ch_file = fopen(fnam2, "r");
		if (Ch_file == NULL) {
			if (dirflg & 0x01) {
				ChgDirExt(fnam2, fnam, Ana_incDir, NULL);
				Ch_file = fopen(fnam2, "r");
				if (Ch_file == NULL) {
					ChgDirExt(fnam2, fnam, Ana_incDir, Ana_ext);
					Ch_file = fopen_e(fnam2, "r");
				}
				setvbuf(Ch_file, NULL, _IOFBF, 0x4000);
			} else
				fopen_e(fnam2, NULL);
		}
		Ch_srcName = fnam2;
	}
	f			= Decl_Module(0);
	Mod_sp->v.st= St_expoRoot;
	sp->v.st	= St_expoRoot;
	sp->v.grp	= Mod_sp;

	while (f == ANAERR_MMOD) {
		f = Decl_Module(1);
	}

	fclose(Ch_file);

	if (dirflg & 0x10)	{/* サブモジュールのとき */
		;
	} else {
		Opt_cpu		= sav_Opt_cpu;
		Ana_model	= sav_Ana_model;
		St_expoRoot = sav_St_expoRoot;
		Mod_sp		= sav_Mod_sp;
	}
	Opt_comment = sav_Opt_comment;
	St_root		= sav_St_root;
	Ana_mode	= sav_Ana_mode;
	Decl_defFlg = sav_Decl_defFlg;
	Ch_file		= sav_Ch_file;
	Ch_line		= sav_Ch_line;
	Ch_srcName	= sav_Ch_srcName;
  #ifdef MVAR
	gMvarOfs = sav_gMvarOfs;
  #endif
	Ch_Top();
	if (strcmp(sp->v.name, sp->v.grp->v.name) != 0) {
		Msg_Err("importで指定された名前が実際のモジュール名と一致しません");
	}
	return f;
}


static int DeclImport(int defFlg)
{
	byte	fnam[IMP_FNAM_MAX + 2];
	int 	f, dirflg;
	St_t_fp sp, ssp;

	f = 0;
	dirflg = 0;
	sp = NULL;
	if (Sym_Get() != T_IDENT)
		goto ERR;
	strcpy(fnam, Sym_name);
	if ((sp = Sym_NameNew(Sym_name, T_MODULE, Ana_mode)) == NULL)
		goto ENDF;
	if (defFlg == 0) {
		Sym_SkipCR();
		return 0;
	}
	if ((ssp = St_Search(Sym_name, St_inpRoot)) != NULL) {
		Sym_SkipCR();
		sp->v.st = ssp->v.st;
		sp->v.grp = ssp->v.grp;
	} else {
		if (Sym_Get() != I_CLN) {
			sprintf(fnam, "%s.%s", sp->v.name, Ana_hdrext);
			dirflg = 1;
		} else {
			if ((dirflg = ImportFile(fnam)) < 0)
				goto ERR;
		}
		Sym_ChkEol();
		f = DeclImportSub(fnam, sp, dirflg);
	}
  ENDF:
	return f;
  ERR:
	Msg_Err("importの指定がおかしい");
	Sym_SkipCR();
	return 0;
}


static int DeclSubmodule(char *name)
{
	byte	fnam[IMP_FNAM_MAX + 2];
	int 	f;
	St_t_fp sp;

	f = 0;
	if ((St_Search(name, St_inpRoot)) != NULL)
		goto ERR;
	if ((sp = Sym_NameNew(name, T_MODULE, Ana_mode)) == NULL)
		goto ENDF;

	sprintf(fnam, "%s.%s", sp->v.name, Ana_hdrext);
	f = DeclImportSub(fnam, sp, 0x11);
  #if 0
	Mod_sp->v.st	= NULL;
	St_root			= NULL;
	if ((sp = Sym_NameNew(name, T_MODULE, Ana_mode)) == NULL)
		goto ENDF;
	sp->v.grp		= Mod_sp;
  #else
	sp->v.grp		= Mod_sp;
	Mod_sp->v.st	= St_root;
  #endif
	/*printf("%s %s %c %d %d\n",fnam,mp->g.modname, mp->g.sep[0], mp->g.ccflg, mp->g.flg2);*/
  ENDF:
	return f;
  ERR:
	Msg_Err("submoduleの指定がおかしい");
	Sym_SkipCR();
	return 0;
}



#if 10
int 	gCcc = 0;				/********/
 /* printf("*%d\t",gCcc++); */
#endif


static long ModuleHdr(void)
{
	byte	modelFlg, stkFlg, sttFlg, cnamFlg;
	long	stksiz;

	modelFlg = stkFlg = sttFlg = cnamFlg = 0;
	stksiz = -1;

	Ana_mode = MD_MODULE;
	goto J1;

	while (Ana_err == 0) {
		Ana_mode = MD_MODULE;
	/* Sym_crMode = 1; Sym_Get(); Sym_crMode = 0; */
		Sym_Get();
  J1:
		if (Sym_tok == I_CR || Sym_tok == I_SMCLN) {
			;
		} else if (sttFlg == 0 && Sym_tok == I_START) {
			sttFlg = 1;
			if (Sym_Get() != T_IDENT) {
				Msg_Err("start で指定された名前がすでに登録されている");
			} else {
				if (Decl_defFlg) {
					if ((oStart_sp = Sym_NameNew(Sym_name, T_STARTLBL, Ana_mode)) == NULL)
						goto ENDF;
				}
				Sym_Get();
			}

		} else if (stkFlg == 0 && Sym_tok == I_STACK) {
			stkFlg = 1;
			Sym_Get();
			stksiz = Expr_Cnstnt();

		} else if (modelFlg == 0 && Sym_tok == I_MODEL) {
			int 	am;

			modelFlg = 1;
			Sym_Get();
			am = (int) Expr_Cnstnt();
			if (am < 0 || am > 4) {
				Msg_Err("指定された model の値が 0 ～ 4 以外");
			} else {
				Ana_model = am;
			}
		} else if (cnamFlg == 0 && Sym_tok == I_CNAME) {
			cnamFlg = 1;
			if (Sym_Get() == T_STRING) {
				if (Sym_strLen > 32) {
					Msg_Err("生成するモジュール名文字列が長すぎる");
					Sym_str[32] = '\0';
				}
				Mod_sp->g.modname = strdup(Sym_str);
				Sym_Get();
			}
			if (Sym_tok == I_COMMA) {
				if (Sym_Get() == T_STRING) {
					if (Sym_strLen > 2)
						Msg_Err("c_nameの第2パラメータの指定がおかしい");
					Mod_sp->g.sep[0] = Sym_str[0];
					Mod_sp->g.sep[1] = Sym_str[1];
					Sym_Get();
				}
				if (Sym_tok == I_COMMA) {
					Mod_sp->g.ccflg = 0;
					Sym_Get();
					Mod_sp->g.ccflg = (byte) Expr_Cnstnt();
					if (Mod_sp->g.ccflg < 0 || Mod_sp->g.ccflg > 2)
						Msg_Err("c_nameの第３パラメータの指定がおかしい");
				}
			}
		} else {
			break;
		}
		Sym_ChkEol();
	}
  ENDF:
	if (Ana_model < 0)
		Ana_model = Ana_model0;
	return stksiz;
}

int 	Decl_Module(int mmodFlg)
{
	St_t_fp sp;
	long	stksiz;
	word	t, nomret;

	Ana_mmodFlg	= 0;
	nomret		= 0;
	MSG("Decl_Module");
	Ana_mode	= MD_MODULE;
  #ifdef MVAR
	gMvarOfs	= 0;
  #endif
	St_root		= NULL;
	St_expoRoot = NULL;
	Mod_sp		= NULL;

	/* 'module'命令の処理 */
	if (mmodFlg == 0) {
		Sym_rsvflg = 0;
		Sym_crMode = 1;
		Sym_Get();
		Sym_crMode = 0;
		if (Sym_tok == I_SUBMODULE && Decl_defFlg) {
			char *p;
			if (Sym_Get() != T_IDENT)
				goto ERR_MOD;
			p = strdup(Sym_name);
			Sym_GetChkEol();
			if (DeclSubmodule(p))
				goto ENDF;
			free (p);
			goto SSS1;

		} else if (Sym_tok != I_MODULE) {
			if (Ana_moduleMode == 0 && Decl_defFlg) {	/* module なしのプログラムのとき */
				if ((Mod_sp = St_Search("_@｢db｣@_", St_inpRoot)) != NULL) {
					Msg_PrgErr("module 名がすでに登録されている");
					St_expoRoot = Mod_sp->v.st;
					goto ENDF;
				}
				Mod_sp			= St_Ins("_@｢db｣@_", &St_inpRoot);
				Mod_sp->v.tok	= T_GROUP;
				Mod_sp->v.grp	= Mod_sp;
				Mod_sp->g.modname = "";
				Mod_sp->g.sep[0] = 0;
				goto SSS2;
			}
			Msg_Err("ソースの最初は 'module' でないといけない");
			return 1;
		}
	}
	if (Sym_Get() != T_IDENT)
		goto ERR_MOD;
	if ((Mod_sp = St_Search(Sym_name, St_inpRoot)) != NULL) {
		if (Decl_defFlg) {
			Msg_PrgErr("module 名がすでに登録されている");
			goto ENDF;
		}
		St_expoRoot = Mod_sp->v.st;
		goto ENDF;
	}
	Mod_sp			= St_Ins(Sym_name, &St_inpRoot);
	Mod_sp->v.tok	= T_GROUP;
	Mod_sp->v.grp	= Mod_sp;
	Mod_sp->g.modname = Mod_sp->v.name;
	Mod_sp->g.sep[0] = '@';
 /* Mod_sp->g.seg[2] = '\0'; */
	if ((sp = Sym_NameNew(Sym_name, T_MODULE, MD_MODULE)) == NULL)
		goto ENDF;
	sp->v.grp		= Mod_sp;
	Sym_GetChkEol();

 /* モジュール宣言部 */
 SSS1:
	Sym_Get();
 SSS2:
	stksiz = ModuleHdr();

 /* import 命令 */
	t = 0;
	while (Ana_err == 0) {
		Ana_mode = MD_MODULE;
		t = 0;
		if ( /* Sym_tok == I_GLOBAL || */ Sym_tok == I_EXPORT) {
			t = Sym_tok;
			Sym_Get();
			Ana_mode = MD_EXPO;
		}
		if (Sym_tok == I_IMPORT) {
			if (DeclImport(t || Decl_defFlg || oImpFlg))
				goto ENDF;
		} else {
			break;
		}
		Sym_crMode = 1;
		Sym_Get();
		Sym_crMode = 0;
	}

	/* CODE,DATA,BSS,STACKセグメントを生成 */
	oSegC_sp = gSeg_sp[GS_CODE];
	gStatBrkCont_p = NULL;
	if (Decl_defFlg) {
		/* if (oStart_sp) fprintf(Out_file,"\t;dosseg\n"); */
		if (Opt_cpu && Opt_r86 == 0)
			fprintf(Out_file, "\t.%c86\n", '0' + Opt_cpu);
		RsvLblSet2();
		DeclModel(Ana_model);
		OutGroup(Ana_model);
		OutSegBgn(gSeg_sp[GS_CODE]);
		oSegC_sp = gSeg_sp[GS_CODE];
		if (Opt_r86 == 0)
			fprintf(Out_file, "\tassume\tcs:%s\n", oSegS[GS_CODE]);
		Out_DAssume();
		Out_Seg(gSeg_sp[GS_DATA]);
		Out_Seg(gSeg_sp[GS_BSS]);
		/* if (oStart_sp) */
		/* fprintf(Out_file,"bsstop\tlabel\tbyte\n"); */
		if ((stksiz >= 0 && Ana_comModel == 0)
			|| (oStart_sp && (Ana_model == 1 || Ana_model == 3))) {
			Out_Seg(gSeg_sp[GS_STACK]);
			if (stksiz > 0)
				Out_Dup(-1, stksiz);
		}
	}
	/* 変数宣言、定数宣言、手続きetc */
	if (t) {
		Ana_mode = MD_EXPO;
		goto LL;
	}
	Sym_crMode = 0; 			/* */
	while (Ana_err == 0) {
		int	externFlg;
		Ana_mode = MD_MODULE;
	  LL:
		externFlg = 0;
		if (Sym_tok == I_ENDMODULE) {
			Sym_GetChkEol();
			break;
		}
		if ( /* Sym_tok == I_GLOBAL || */ Sym_tok == I_EXPORT) {
			if (Sym_tok == I_EXPORT || Decl_defFlg)
				Ana_mode = MD_EXPO;
			Sym_Get();
		} else if (Sym_tok == I_EXTERN) {
			Sym_Get();
			externFlg = 1;
			if (Sym_tok == I_PROC) {
				externFlg = 0;
				Ana_mode = MD_EXPO;
			}
		}
		if (Sym_tok == I_CONST)
			Decl_Const();
		else if (Sym_tok == I_VAR)
			Decl_Var(gSeg_sp[GS_DATA], externFlg);
		else if (Sym_tok == I_CVAR)
			Decl_Var(gSeg_sp[GS_CODE], externFlg);
		else if (Sym_tok == T_SEG)
			Decl_Var(Sym_sp, externFlg);
		else if (Sym_tok == I_PROC /* ||Sym_tok == I_CPROC */ )
			Decl_Proc(Sym_tok);
		else if (Sym_tok == I_TYPE)
			Decl_Type();
		else if (Sym_tok == I_STRUCT)
			Decl_Struct();
		else if (Sym_tok == I_SEGMENT)
			DeclSeg();
		else if (Sym_tok == I_ORG)
			DeclOrg();
		else if (Sym_tok == I_PART)
			DeclReOpen();
#ifdef MACR
		else if (Sym_tok == M_DEFINE)
			Sym_MacDef();
#endif
		else if (Sym_tok == I_EVEN) {
			Sym_GetChkEol();
			Ana_align = 2;
		} else if (Sym_tok == M_INCLUDE)
			Sym_MacInclude();
#ifdef MVAR
		else if (Sym_tok == I_MVAR)
			Decl_Mvar();
#endif
#if 0
		else if (Decl_defFlg && Ana_mode == MD_MODULE) {
			Out_Seg(gSeg_sp[GS_CODE]);
			if (Stat())
				return Ana_err;
		}
#endif
		else {
			Msg_Err("module内に余計なものがある");
		}
		if (Ana_err == ANAERR_ENDP)
			Ana_err = 0;
		Sym_crMode = 1;
		Sym_Get();
		Sym_crMode = 0;
	}

	{
		while (Ana_err == 0) {
			Ana_mmodFlg = 1;
			Sym_crMode = 1;
			Sym_Get();
			Sym_crMode = 0;
			Ana_mmodFlg = 0;
			if (Sym_tok == I_MODULE) {
				nomret = ANAERR_MMOD;
				break;
			}
		}
		Ana_err = 0;
	}

	/* end の処理 */
	if (Decl_defFlg) {
		OutSegEnd(oSegC_sp);
		if (Opt_partFlg == 0) {
			OutExtrnName(St_expoRoot);	///
			OutExtrnName(St_root);
			if (oStart_sp) {
				fprintf(Out_file, "\tend\t%s\n", Str_StName(oStart_sp));
				if (oStart_sp->v.tok != T_PROC)
					Msg_Err("start で指定された名前が手続きでない");
			} else {
				Out_Line("\tend");
			}
		} else {
			Out_Line("\tend");
			ChgDirExt(Ana_outName, Ana_partIncName, Ana_dirName, "INC");
			Out_file = freopen_e(Ana_outName, "w", Out_file);
			OutExtrnName(St_expoRoot);
			OutExtrnName(St_root);
		}
	}
	St_Free(St_root);
  ENDF:
	return nomret;

  ERR_MOD:
	Msg_Err("module名の指定がおかしい");
	/* Sym_SkipCR(); */
	return 1;
}

void	Rsv_LblSet(void)
{
	St_t_fp sp;

 /* cpu */
	if ((sp = Sym_NameNew("P86", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.l_flg = 2;
	sp->c.val = Opt_cpu;

	if ((sp = Sym_NameNew("MODEL", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 1;
	Ana_modelsp = sp;

#if 0
	if ((sp = Sym_NameNew("NULL", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 0;

	if ((sp = Sym_NameNew("NIL", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 0;

	if ((sp = Sym_NameNew("bsstop", T_JMPLBL, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 0;
	if ((sp = Sym_NameNew("stacktop", T_JMPLBL, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 0;
#endif

	if ((sp = Sym_NameNew("TINY", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 0;
	if ((sp = Sym_NameNew("SMALL", T_CONST, MD_RSV)) == NULL)
		goto ENDF;

	sp->c.val = 1;
	if ((sp = Sym_NameNew("COMPACT", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 2;
	if ((sp = Sym_NameNew("MEDIUM", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 3;
	if ((sp = Sym_NameNew("LARGE", T_CONST, MD_RSV)) == NULL)
		goto ENDF;
	sp->c.val = 4;

  ENDF:;
}

static void RsvSegNam(void)
{
	int 	i;

	for (i = 0; i < 4; i++) {
		oSegS[i] = oSegSrc[Opt_lsic][i];
	}
}

/*------------------------------- ana.c -------------------------------------*/

void	Usage(void)
{
	eputs(TITLE "\n");
	eputs("usage: ANA [-opts] <ﾌｧｲﾙ名> [-opts]\n");
 /* eputs (" -?         ヘルプ\t\t-v\tana の予約語を表示\n"); */
	eputs(" -c[1|2]    ソースをコメントにして出力(ファイル名・行番号付き)\n");
	eputs("            -c1:ファイル名なし  -c2:ファイル名・行番号なし\n");
	eputs(" -1         186(V30) の命令を有効\n");
	eputs(" -b[0]      var変数で 0 初期化するものを BSS に配置する/-b0 しない\n");
 /* eputs (" -p         @part命令を有効にする(-o指定時のみ)\n"); */
 /* eputs (" -ti        import を再帰的に行う\n"); */
 /* eputs (" -tk        丸括弧'(...)'内の改行を空白扱いにする\n"); */
 /* eputs (" -tp        出力の手続きに proc ～ endp を用いる\n"); */
	eputs(" -tg        生成されるローカルな変数名・手続き名もpublic指定に\n");
 /* eputs (" -te<ext>   ヘッダ・ファイルの拡張子を.<ext> に変更\n"); */
	eputs(" -q[0|1]    手続き引数の暗黙の代入を 0:禁止する *1:しない\n");
	eputs(" -al        LSI-C用のセグメント名にする(ｻﾎﾟｰﾄ不完全)\n");
	eputs(" -ar        出力先アセンブラとして LSI r86 を用いる(ｻﾎﾟｰﾄ不完全)\n");
	eputs(" -aa        手続きの入口で assume を生成しない\n");
 /* eputs (" -ab-       _BSS のクラスを'DATA'にする\"); */
	eputs(" -r         -alarGLSI_C=2\n");
	eputs(" -y-        v0.20以前の命令を使えなくする\n");
	eputs(" -s[<path>] 出力を<path>にする\n");
	eputs(" -o[<dir>]  拡張子を.asm にしたファイルをディレクトリ<dir>に出力\n");
	eputs(" -e[<path>] エラー出力を<path>にする\n");
	eputs(" -i<dir>    <..> でimportするディレクトリを<dir>にする\n");
	eputs(" -m(t|s|c|m|l)    メモリ・モデル\n");
	eputs(" -d<lbl>[=<str>]  @define <lbl> <str> で予約ラベル領域に登録\n");
	eputs(" -g<lbl>[=<val>]  @set <lbl> = <val> で予約領域に登録(10進数)\n");
	eputs("                  -d,-g で初期値無しは -d=0 -g=0 となる\n");
	exit(0);
}


void	init(void)
{
	Et_Init();
	St_Init();
	St_inpRoot = NULL;
	St_rsvRoot = NULL;
#ifdef MACR
	Mac_Init();
#endif
}

void	DefLbl(byte_fp str, word f)
{
	byte_fp p;
	St_t_fp sp;

	p = str;
	while (*p != '=' && *p != '\0')
		p++;
	if (*p == '\0')
		p = NULL;
	else
		*p++ = '\0';
	if (str == NULL || *str == '\0' || St_Search(str, St_rsvRoot)) {
		eputs("-D|G の指定がおかしい\n");
		exit(1);
	}
	if ((sp = Sym_NameNew(str, (f == 0) ? T_CONST : T_MACROSTR, MD_RSV)) == NULL)
		exit(1);
	sp->c.l_flg = 1;
	if (f == 0) {
		if (p)
			sp->c.val = strtol(p, NULL, 10);
		else
			sp->c.val = 0;
	} else {
		if (p)
			sp->m.macbufp = p;
		else
			sp->m.macbufp = "0";
	}
}

static byte *oOutname, *oErrFname;
static byte oOutFlg;

void	Option(byte * p)
{
	word	c;
	static byte incdir[82];

	while ((c = toupper(*p)) != '\0') {
		++p;
		switch (c) {
		case '1':
			Opt_cpu = 1;
			break;

		case '0':
			Opt_cpu = 0;
			break;

		case 'A':
			c = toupper(*p);
			p++;
			if (c == 'M') {
				Opt_r86 = 0;
			} else if (c == 'R') {
				Opt_r86 = 1;
			} else if (c == 'L') {
				Opt_lsic = 1;
			} else if (c == 'A') {
				Opt_procAssumeFlg = 0;
			} else if (c == 'B') {
				Opt_bssClassFlg = (*p == '-') ? p++,0 : 1;
			} else {
				goto OPTS_ERR;
			}
			break;

		case 'X':
			Ana_moduleMode = (*p == '-') ? p++,0 : 1;
			break;

		case 'Y':
			Ana_oldver = (*p == '-') ? p++,0 : 1;
			break;

		case 'R':
			Opt_r86 = 1;
			Opt_lsic = 1;
			DefLbl("LSI_C=2", 0);
			break;

		case 'Q':
			c = *p++;
			if (c == '-' || c == '0') {
				Opt_auto = 0;
			} else if (c == '1') {
				Opt_auto = 1;
			} else
				goto OPTS_ERR;
			break;

		case 'B':
			Opt_zeroBSS = 1;
			if (*p == '-' || *p == '0')
				++p, Opt_zeroBSS = 0;
			break;

		case 'P':
			Opt_partFlg = 1;
			break;

		case 'O':
			oOutFlg = 1;
			if (*p != '\0')
				Ana_dirName = p;
			goto EXIT_WHILE;

		case 'S':
			if (*p != '\0') {
				oOutname = p;
				oOutFlg = 2;
			} else
				oOutFlg = 0;
			goto EXIT_WHILE;

		case 'T':
			c = *p++;
			c = toupper(c);
			if (c == 'I') {
				oImpFlg = 1;
			} else if (c == 'K') {
				Opt_parCr = 1;
			} else if (c == 'P') {
				Opt_namePub = 1;
			} else if (c == 'E') {
				Ana_hdrext = p;
				goto EXIT_WHILE;
			} else if (c == 'G') {
				Opt_procFlg = 1;
				break;
			} else if (c == 'C') {
				Gen_EquFlg(0);
			} else {
				goto OPTS_ERR;
			}
			break;

		case 'M':
			c = *p++;
			c = toupper(c);
			if (c == 'T') {
				c = 0;
				if (*p == 'C' || *p == 'c')
					++p, Ana_comModel = 1;
			} else if (c == 'S')
				c = 1;
			else if (c == 'C')
				c = 2;
			else if (c == 'M')
				c = 3;
			else if (c == 'L')
				c = 4;
			else
				goto OPTS_ERR;
			Ana_modelsp->c.val = c;
			Ana_model0 = Ana_model = c;
			break;

		case 'I':
		/* if (*p == '=') ++p; */
			if (*p != '\0') {
				strncpy(incdir, p, 80);
				Ana_incDir = incdir;
			}
			goto EXIT_WHILE;

		case 'E':
		/* if (*p == '=') ++p; */
			if (*p == '\0') {
				oErrFname = "errors";
			} else if (*p == '-') {
				oErrFname = NULL;
			} else {
				oErrFname = p;
			}
			goto EXIT_WHILE;

		case 'C':
			Opt_comment = 3;
			if (*p == '1') {
				Opt_comment = 1;
				p++;
			} else if (*p == '2') {
				Opt_comment = 2;
				p++;
			}
			break;

		case 'V':
			Rsv_List();
			exit(0);

		case 'G':
		/* if (*p == '=') ++p; */
			DefLbl(p, 0);
			goto EXIT_WHILE;

		case 'D':
			DefLbl(p, 1);
			goto EXIT_WHILE;

		case 'Z':
			if (*p == '1') {
				p++;
				Deb_macFlg = 1;
			}
#ifdef DEBUG
			else
				DEBUG_MODE(1);
#endif
			break;

		case '?':
			Usage();
		default:
		  OPTS_ERR:
			ep_exit("ｵﾌﾟｼｮﾝ指定がおかしい\n");
		}
	}
  EXIT_WHILE:;
}

void	GetCfg(byte * av0)
{
	byte   *np, *p;
	byte	nm[128 * 3 + 2];
	int 	i;
	FILE   *fp;

	Gen_EquFlg(1);
	for (i = 0, np = nm; i < 250 && *av0 && *av0 != '.'; i++, av0++, np++) {
		*np = *av0;
	}
	np = stpcpy(np, ".CFG");
	fp = fopen_e(nm, "r");
	np++;
	i = 128 * 3 - strlen(nm);
	for (; ;) {
		p = fgets(np, i, fp);
		if (p == NULL)
			break;
		for (; ;) {
			p = strtok(p, " \t\n");
			if (p == NULL || *p == '\0')
				break;
			if (*p == '-') {
				p++;
				Option(p);
			} else if (*p == '#') {
				goto J1;
			} else {
				printf("'%s'での指定がおかしい\n", nm);
			}
			p = NULL;
		}
	  J1:;
	}
}

int 	main(int argc, byte * argv[])
{
	static byte in_name[NAM_SIZE];
	FILE   *fp, *ofp, *efp;
	byte   *p;
	byte	c;
	int 	i;

	if (argc < 2)
		Usage();
	Ana_oldver = 1;
	Ana_model0 = 1;
	Ana_model = -1;
	oErrFname = NULL;
	Opt_auto = 1;
	Opt_procAssumeFlg = 1;
	Ana_incDir = NULL;
	Opt_namePub = Opt_procFlg = Opt_comment = oOutFlg = 0;
	init();
	Rsv_LblSet();
	for (i = 1, c = 0; i < argc; i++) {
		if (argv[i][0] == '+' && argv[i][1] == '\0') {
			c = 1;
			break;
		}
	}
	if (c == 0)
		GetCfg(argv[0]);
	for (i = 1; i < argc; ++i) {
		p = argv[i];
		if (*p == '-') {
			p++;
			if (*p == '\0')
				Usage();
			Option(p);
		} else if (*p == '+') {
			p++;
			if (*p)
				GetCfg(p);
		}
	}
	RsvSegNam();
	if (oOutFlg == 2) {
		ofp = fopen_e(oOutname, "w");
		setvbuf(ofp, NULL, _IOFBF, 0x4000);
	} else {
		ofp = stdout;
	}
	if (oErrFname)
		efp = fopen_e(oErrFname, "w");
	else
		efp = ofp;

	for (c = 1, i = 1; i < argc; i++) {
		p = argv[i];
		if (*p == '-' || *p == '@')
			continue;
		c = 0;
		if (strrchr(p, '.') != NULL && *p != '.')
			strcpy(in_name, p);
		else
			ChgDirExt(in_name, p, NULL, Ana_ext);
		p = in_name;
		fp = fopen_e(p, "r");
		setvbuf(fp, NULL, _IOFBF, 0x4000);
		Ana_partLstFile = Rsp_file = NULL;
		if (oOutFlg == 1) {
			if (Ana_dirName) {
				mkdir(Ana_dirName);
			}
			ChgDirExt(Ana_outName, p, Ana_dirName, oAsmExt[Opt_r86]);
			ofp = fopen_e(Ana_outName, "w");
			setvbuf(ofp, NULL, _IOFBF, 0x4000);
			if (Opt_partFlg) {
				ChgDirExt(Ana_partIncName, Ana_outName, NULL, "RSP");
				Rsp_file = fopen_e(Ana_partIncName, "w");
				ChgDirExt(Ana_partIncName, Ana_outName, "", "");
				fprintf(Rsp_file, "+%s", Ana_partIncName);

				ChgDirExt(Ana_partIncName, Ana_outName, NULL, "L");
				Ana_partLstFile = fopen_e(Ana_partIncName, "w");
				ChgDirExt(Ana_partIncName, Ana_outName, "", "");
				fprintf(Ana_partLstFile, "%s\t", Ana_partIncName);

				ChgDirExt(Ana_partIncName, Ana_outName, "", "INC");
			}
		}
		{
			Ch_file = fp;
			Out_file = ofp;
			Err_File = efp;
			Ch_srcName = p;
			GoLbl_no = 0;
			if (Ch_Init())
				Msg_Err("初期化エラー");
			oStart_sp = NULL;
			Decl_defFlg = 1;
			Decl_Module(0);
		}
		fclose(fp);
		if (oOutFlg == 1) {
			fclose(Out_file);
			if (Opt_partFlg) {
				fprintf(Rsp_file, "\n");
				fclose(Rsp_file);
			}
		}
		break;
	}
	if (c)
		Usage();

	if (oOutFlg == 2)
		fclose(ofp);
	if (oErrFname)
		fclose(efp);
	if (Err_cnt)
		return 1;
	return 0;
}
