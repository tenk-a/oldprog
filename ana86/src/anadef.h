#include <stdio.h>
#include "tenkdefs.h"

/*#define MVAR*/
#define MACR

#define ERR_VAL  (0x80000000L)
#define strend(p)	((p)+strlen(p))
/*--------------- symbol ---------------*/
enum {
	MD_EXPO,MD_RSV,MD_MODULE,MD_PROC,MD_STRUCT
};
enum {
	GS_CODE, GS_DATA, GS_BSS, GS_STACK
};
enum {
	FL_EXPO = 0x01, FL_FAR = 0x02, FL_CPROC = 0x04, FL_EXTERN = 0x08,
	FL_INPO = 0x10, FL_USE = 0x20
};
enum {
	I_EOF=0,	/* "EOF" */
	I_BYTE, 	/*ˇ"byte" */
	I_WORD, 	/* "word"ˇ*/
	I_PTR,  	/* "pointer" */
	I_DWORD,	/* "dword" */
	I_FPTR, 	/* "fptr" */
	I_SEGPTR,	/* "segptr" */
	I_NEAR, 	/*ˇ"near" */
	I_QWORD,	/* "qword" */
	I_FAR,  	/* "far" */
	I_TBYTE,	/* "tbyte" */

	/* reg */
	I_AH,		/* "ah" */
	I_CH,		/* "ch" */
	I_DH,		/* "dh" */
	I_BH,		/* "bh" */

	I_AL,		/* "al" */
	I_CL,		/* "cl" */
	I_DL,		/* "dl" */
	I_BL,		/* "bl" */

	I_AX,		/* "ax" */
	I_CX,		/* "cx" */
	I_DX,		/* "dx" */
	I_BX,		/* "bx" */

	I_DI,		/* "di" */
	I_SI,		/* "si" */
	I_BP,		/* "bp" */

	I_SP,		/* "sp" */
	I_IP,		/* "ip" */

	I_CS,		/* "cs" */
	I_DS,		/* "ds" */
	I_ES,		/* "es" */
	I_SS,		/* "ss" */

	I_FH,		/* "fh" */
	I_FL,		/* "fl" */
	I_FX,		/* "fx" */

	/* flag */
	I_CF,		/* "cf" */
	I_AF,		/* "af" */
	I_PF,		/* "pf" */
	I_SF,		/* "sf" */
	I_ZF,		/* "zf" */
	I_IIF,  	/* "iif" */
	I_TF,		/* "tf" */
	I_DF,		/* "df" */
	I_OVF,  	/* "ovf" */

	/* ìÒçÄââéZ */
	I_EXG,  	/* "<=>" */
	I_LEA,  	/* "(lea)" */
	I_SIEQ, 	/* ".=." */
	I_LDS,  	/* "(lds)" */
	I_LES,  	/* "(les)" */
	I_EQU,  	/* "=" */

	I_EQEQ, 	/* "==" */
	I_NEQ,  	/* "!=" */
	I_ADCEQ,	/* "+=." */
	I_SBCEQ,	/* "-=." */
	I_ADD,  	/* "+" */
	I_ADDEQ,	/* "+=" */
	I_SUB,  	/* "-" */
	I_SUBEQ,	/* "-=" */
	I_AND,  	/* "&" */
	I_ANDEQ,	/* "&=" */
	I_LAND, 	/* "&&" */
	I_OR,		/* "|" */
	I_OREQ, 	/* "|=" */
	I_LOR,  	/* "||" */
	I_XOR,  	/* "^" */
	I_EOREQ,	/* "^=" */
	I_LT,		/* "<" */
	I_LE,		/* "<=" */
	I_SHL,  	/* "<<" */
	I_GT,		/* ">" */
	I_GE,		/* ">=" */
	I_SHR,  	/* ">>" */
	I_ILT,  	/* ".<." */
	I_ILE,  	/* ".<=." */
	I_IGT,  	/* ".>." */
	I_IGE,  	/* ".>=." */
	I_SAR,  	/* ".>>." */

	I_SHLEQ,	/* "<<=" */
	I_SHREQ,	/* ">>=" */
	I_RCLEQ,	/* "<<=." */
	I_RCREQ,	/* ">>=." */
	I_ROLEQ,	/* "<<<=" */
	I_ROREQ,	/* ">>>=" */
	I_SAREQ,	/* ".>>=" */

	I_ROL,		/* "<<<" */
	I_ROR,		/* ">>>" */

	I_SHLS, 	/* "shl" */
	I_SHRS, 	/* "shr" */
	I_RCLS,		/* "rcl" */
	I_RCRS,		/* "rcr" */
	I_ROLS,		/* "rol" */
	I_RORS,		/* "ror" */
	I_SARS, 	/* "sar" */

	I_ADCS,		/* "adc" */
	I_SBCS,		/* "sbc" */

	I_MUL,  	/* "*" */
	I_MULEQ,	/* "*=" */
	I_IMUL, 	/* ".*." */
	I_DIV,  	/* "/" */
	I_DIVEQ,	/* "/=" */
	I_IDIV, 	/* "./." */
	I_MOD,  	/* "%" */
	I_MODEQ,	/* "%=" */
	I_GLOBAL,	/* "%%" */
	I_IMOD, 	/* ".%." */
	I_DIVS, 	/* "div" */
	I_IDIVS,	/* "idiv" */

	/* ì¡ï ÇÃº›ŒﬁŸ */
	I_CLN,  	/* ":" */
	I_CLNEQ,	/* ":=" */
	I_CR,		/* "\n" */
	I_SHARP,	/* "#" */
	I_DOLL, 	/* "$" */
	I_ATMARK,	/* "@" */
	I_YEN,  	/* "\\" */
	I_DQUOT,	/* "\"" */
	I_APO,  	/* "'" */
	I_PERIOD,	/* "." */
	I_PP,		/* ".." */
	I_PPP,  	/* "..." */
	I_QUESTION, /* "?" */
	I_COMMA,	/* "," */
	I_SMCLN,	/* ";" */
	I_LP,		/* "(" */
	I_RP,		/* ")" */
	I_LB,		/* "[" */
	I_RB,		/* "]" */
	I_LC,		/* "{" */
	I_RC,		/* "}" */

	/* íPçÄââéZ */
	I_COM,  	/* "~" */
	I_NOT,  	/* "!" */
	I_INC,  	/* "++" */
	I_NEG,  	/* "íPçÄ-" */
	I_DEC,  	/* "--" */
	I_NEGS, 	/* "neg" */
	I_COMS, 	/* "com" */

	/* statement */
	I_AAADD,	/* "aaadd" */
	I_AADIV,	/* "aadiv" */
	I_AAMUL,	/* "aamul" */
	I_AASUB,	/* "aasub" */
	I_DAADD,	/* "daadd" */
	I_DASUB,	/* "dasub" */
	I_LEAVE,	/* "leave" */
	I_LOCK, 	/* "lock" */
	I_HLT,  	/* "hlt" */
	I_NOP,  	/* "nop" */
	I_INTO, 	/* "into" */
	I_IRET, 	/* "iret" */
	I_POPA, 	/* "popa" */
	I_PUSHA,	/* "pusha" */
	I_SEG_ES,	/* "seg_es" */
	I_SEG_CS,	/* "seg_cs" */
	I_SEG_SS,	/* "seg_ss" */
	I_SEG_DS,	/* "seg_ds" */
	I_WAIT, 	/* "wait" */

	I_CLRC, 	/* "clrc" */
	I_SETC, 	/* "setc" */
	I_CLRI, 	/* "clri" */
	I_SETI, 	/* "seti" */
	I_CLRD, 	/* "clrd" */
	I_SETD, 	/* "setd" */
	I_COMC, 	/* "comc" */
	I_XLAT, 	/* "xlat" */
	I_SEXT1,	/* "cbw" */
	I_SEXT2,	/* "cwd" */
	I_LAHF, 	/* "lahf" */
	I_SAHF, 	/* "sahf" */
	I_PUSHF,	/* "pushf" */
	I_POPF, 	/* "popf" */

	I_ESC,  	/* "esc" */

	I_REP,  	/* "rep" */
	I_REPE, 	/* "repe" */
	I_REPN, 	/* "repn" */
	I_POP,  	/* "pop" */
	I_PUSH, 	/* "push" */

	I_INTR, 	/* "intr" */
	I_ENTER,	/* "enter" */
	I_BOUND,	/* "bound" */
	I_BEGIN,	/* "begin" */

	I_GO,		/* "go" */
	I_GOTO, 	/* "goto" */
	I_GOSUB,	/* "gosub" */
	I_RETURN,	/* "return" */
	I_RETN, 	/* "retn" */
	I_RET,  	/* "ret" */
	I_RETF, 	/* "retf" */
	I_JMPS, 	/* "jmp short" */
	I_JMP,  	/* "jmp" */
	I_JMPF, 	/* "jmpf" */
	I_CALLN,	/* "calln" */
	I_CALL, 	/* "call" */
	I_CALLF,	/* "callf" */

	I_IF,		/* "if" */
	I_ELSIF,	/* "elsif" */
	I_ELSE, 	/* "else" */
	I_FI,		/* "fi" */
	I_EXIT, 	/* "exit" */
	I_NEXT, 	/* "next" */
	I_LOOP, 	/* "loop" */
	I_ENDLOOP,  /* "pool" */
	I_DO,		/* "do" */
	I_ENDDO,	/* "od" */

	I_VAR,  	/* "var" */
	I_CVAR, 	/* "code" */
	I_MVAR, 	/* "mvar" */
	I_LOCAL,	/* "auto" */
	I_CONST,	/* "const" */
	I_TYPE, 	/* "type" */
	I_DATA, 	/* "data" */
	I_ATAD, 	/* "enddata" */
	I_STRUCT,	/* "struct" */
	I_ENDSTRUCT,/* "endstruct" */

	I_PROC, 	/* "proc" */
	/*I_CPROC,*/	/* "c_proc" */
	I_ENDP, 	/* "corp" */
	I_EXTERN,	/* "extern" */
	I_FORWORD,  /* "forword" */
	I_MACRO,	/* "macro" */
	I_BEFORE,	/* "macro" */
	I_CDECL,   /* "c_decl" */
	I_SAVE, 	/* "save" */
	I_LOAD, 	/* "load" */
	I_IN,		/* "in" */
	I_OUT,  	/* "out" */
	I_BREAK,	/* "break" */

	I_MODULE,	/* "module" */
	I_ENDMODULE,/* "endmodule" */
	I_PART, 	/* "part" */
	I_MODEL,	/* "model" */
	I_IMPORT,	/* "import" */
	I_CNAME,	/* "c_name" */
	I_STACK,	/* "stack" */
	I_START,	/* "start" */
	I_ASSUME,	/* "assume" */
	I_SEG,  	/* "seg" */
	I_SEGMENT,  /* "segment" */
	I_EVEN, 	/* "even" */
	I_PARA, 	/* "para" */
	I_PAGE, 	/* "page" */
	I_PUBLIC,	/* "public" */
	I_PRIVATE,  /* "private */
	I_COMMON,	/* "common" */
	I_NOTHING,  /* "nothing" */
	I_ORG,  	/* "org" */

	I_CFCM, 	/* ".cf." */
	/*I_AFCM,*/ /* ".af." */
	I_PFCM, 	/* ".pf." */
	I_SFCM, 	/* ".sf." */
	I_ZFCM, 	/* ".zf." */
	I_OVFCM,	/* ".ovf." */
	
	I_NCFCM, 	/* ".cf." */
	I_NPFCM, 	/* ".pf." */
	I_NSFCM, 	/* ".sf." */
	I_NZFCM, 	/* ".zf." */
	I_NOVFCM,	/* ".ovf." */

	I_ORIG, 	/* "orig" */
	I_DEFINED,  /* "defined" */
	I_SIZEOF,	/* "size" */
	I_W2D,  	/* "ww" */
	I_PORT, 	/* "port" */
	I_BXAL, 	/* "BYTE[bx+al]" */
	I_ERR,  	/* "(err)" */
	I_TYPEOF,	/* "(typeof)" */

	/* term ÇÃà íuÇ…Ç≠ÇÈº›ŒﬁŸ */
	T_CNST, 	/* "(íËêî)" */
	T_EXTCNST,  /* "(◊ÕﬁŸÇÃíl)" */
	T_CNSTEXPR, /* "(◊ÕﬁŸñºÇÃåÇ¥Ç¡ÇΩíËêîéÆ)" */
	T_OFS,  	/* "(%◊ÕﬁŸ)" */
	T_EXTCONST, /* "(external const)" */
	T_R1,		/* "(reg1)" */
	T_R2,		/* "(reg2)" */
	T_R4,		/* "(reg4)" */
	T_SEG2, 	/* "(seg)" */
	T_SEG4, 	/* "(seg)" */
	T_M1,		/* "(mem1)" */
	T_M2,		/* "(mem2)" */
	T_M4,		/* "(mem4)" */
	T_ADDR, 	/* "(addr)" */
	T_JMPLBL,	/* "(jmplbl)" */
	T_SEG,  	/* "(seg)" */
	T_ONE,  	/* "(one)" */

	/**/
	T_COND, 	/* "(î‰är)" */
	T_LBL,  	/* "(LBL)" */
	T_BLKLBL,	/* "(BLKLBL)" */
	T_IDENT,	/* "(ident)" */
	T_STRING,	/* "(ï∂éöóÒ)" */
	T_CONST,	/* "(const)" */
	T_TYPE, 	/* "(type)" */
	T_STRUCT,	/* "(strict)" */
	T_ARRAY,	/* "(array)" */
	T_MEMB, 	/* "(member)" */
	#define T_MVAR  T_MEMB
	T_CLASS,	/* "(class)" */
	T_VAR,  	/* "(var)" */
	/*T_CVAR,*/ /* "(code)" */
	T_LOCAL,	/* "(€∞∂Ÿïœêî)" */
	T_ARGLOCAL, /* "(à¯êîïœêî)" */
	T_STARTLBL, /* "(export)" */
	T_PROC, 	/* "(proc)" */
	T_INTRPROC, /* "(proc)" */
	T_PROC_DECL,/* "(proc_decl)" */
	T_MACPROC,  /* "(macro) */
	T_GROUP,	/* "(group)" */
	T_MODULE,	/* "module" */
	T_MODULEVAR,/* "(modulevar) */
	T_MACROSTR, /* "(macrostr) */

	I_TO,		/* "to" */
	I_DB,		/* "db" */
	I_DD,		/* "dd" */
	I_DW,		/* "dw" */
	I_LABEL,	/* "label" */
	I_SWITCH,	/* "switch" */
	I_DEFAULT,  /* "default" */
	I_CASE, 	/* "case" */
	I_ESAC, 	/* "esac" */
	I_CMP,  	/* "comp" */
	I_STATIC,	/* "static" */
	I_CLASS,	/* "class" */
	I_ENDCLASS, /* "endclass" */
	I_ARG,  	/* "arg" */
	I_ARRAY,	/* "array" */
	I_OF,		/* "of" */
	I_GROUP,	/* "group" */
	I_INP,  	/* "inp" */
	I_OUTP, 	/* "outp" */
	I_CODE, 	/* "code" */
	I_DECL, 	/* "decl"	*/
	I_SUBMODULE,/* "submodule" */
	/*I_EXPORT,*/	/* "export" */
	#define I_EXPORT  I_MUL

	I_EQUS,
	I_SET,
	I_ENDMACRO,
	I_REPT,
	I_ENDREPT,

	M_IF,		/* "@if" */
	M_ELSIF,	/* "@elsif" */
	M_ELSE, 	/* "@else" */
	M_ENDIF,	/* "@fi" */
	M_SET,  	/* "@set" */
	M_MSG,  	/* "@msg" */
	M_DEFINE,	/* "@define" */
	M_INCLUDE,  /* "@include" */
	M_MACRO,	/* "@macro" */
	M_ENDMACRO,	/* "@endmacro" */
	M_REPT,		/* "@rept" */
	M_ENDREPT,	/* "@endrept" */
	M_FOR,		/* "@for" */
	M_ENDFOR,	/* "@endfor" */
	M_LOCAL,	/* "@local" */
	T_SYMEND,	/* "(static)" */

};

#define I_DXAX  (I_DX*0x100+I_AX)	/* "dx.ax" */

typedef struct {
	word lbl[2];
	byte flg[2];
} BrkCont_t;


/*---------------- St ---------------- "tbl.c" */
#define LBL_NAME_SIZ (25+16)
struct ST_T_VAR {
	/* union ST_T far *next; */
	union ST_T far *lnk[2];
	byte	flg;
	byte	flg2;				/* bit0: 0=moduleì‡ 1=export */
								/* bit1: 0=near 	1=far	 */
								/* bit2: 0=proc 	1=c_proc */
								/* bit3: 0=moduleì‡ 1=external */
	word	tok;				/* T_TYPE,T_VAR,T_LOCAL */
	byte	name[LBL_NAME_SIZ]; /* ñºëO */
	union ST_T far *grp;		/* module,class Ç÷ÇÃŒﬂ≤›¿ */
	union ST_T far *st; 		/* NULLÇÃÇ∆Ç´ÅAäÓñ{å^(1,2,4) */
	int 	ofs;				/* T_LOCAL,T_TYPE:µÃæØƒ */
								/* T_VAR,T_STATIC:ì‡ïî◊ÕﬁŸî‘çÜ */
	word	siz;				/* ª≤Ωﬁ(1:BYTE,2:WORD,4:DWORD,3:CWORD etc) */
								/* NULLà»äO, T_STRUCT,T_ARRAY */
	word	seg;				/* segment */
};
struct ST_T_PROC {
	/* union ST_T far *next; */
	union ST_T far *lnk[2];
	byte	flg;
	byte	flg2;				/* 0:moduleì‡ïœêî 1:export */
	word	tok;				/* T_PROC */
	byte	name[LBL_NAME_SIZ];
	union ST_T far *grp;
	union ET_T far *et; 		/* à¯Ç´êîÿΩƒ */
	int 	ofs;				/* ì‡ïî◊ÕﬁŸî‘çÜ */
	word	argc;				/* à¯Ç´êîÇÃêî */
	word	argsiz; 			/* à¯êîΩ¿Ø∏ÇÃÇ®Ç®Ç´Ç≥ */
	union ET_T far *et2;		/* regèÓïÒ */
};

struct ST_T_GRP {
	/* union ST_T far *next; */
	union ST_T far *lnk[2];
	byte	flg;
	byte	flg2;				/* 0:¿≤∆∞ 1:Ω”∞Ÿ 2:∫› ﬂ∏ƒ 3:–√ﬁ®±— 4:◊∞ºﬁ */
	word	tok;				/* T_GROUP  */
	byte	name[LBL_NAME_SIZ];
	union ST_T far *grp;
	union ST_T far *st; 		/* ÉÅÉìÉoñºÇÃñÿ */
	byte_fp modname;			/* ê∂ê¨Ç∑ÇÈÉÇÉWÉÖÅ[Éãñº */
	byte	sep[3]; 			/* ê∂ê¨Ç∑ÇÈÉÇÉWÉÖÅ[ÉãñºÇÃÉZÉpÉåÅ[É^ */
	byte	ccflg;  			/* 1:'_'ÇëOÇ…ïtâ¡ 2:'_'Çå„Ç…ïtâ¡ */
	word	mvarSize;			/* ÉNÉâÉXå^ÇÃïœêîïî */
};

struct ST_T_MACFUNC {
	union ST_T far *lnk[2];
	byte	flg;
	byte	flg2;
	word	tok;
	byte	name[LBL_NAME_SIZ];
	union ST_T far *grp;
	byte_fp macbufp;
	word	macArgCnt;
};

struct ST_T_CONST {
	union ST_T far *next, far *back;
	/* union ST_T far *lnk[2];*/
	byte	flg;
	byte	flg2;
	word	tok;				/* T_CONST , T_SEG */
	byte	name[LBL_NAME_SIZ];
	union ST_T far *grp;
	union ET_T far *et;
	long	val;				/* íËêîíl */
	word	seg;
	byte	align;
	byte	combi;
	byte	l_flg;  			/* setÇ≈íËã`Ç≥ÇÍÇΩíËêîÇÃÇ∆Ç´0à»äO */
};
struct ST_T_LBL {
	/* union ST_T far *next; */
	union ST_T far *lnk[2];
	byte	flg;
	byte	flg2;
	word	tok;				/* T_LBL */
	byte	name[LBL_NAME_SIZ];
	union ST_T far *grp;
	union ST_T far *st;
	int 	ofs;				/* ì‡ïî◊ÕﬁŸî‘çÜ */
};
struct ST_T_BLKLBL {
	/* union ST_T far *next; */
	union ST_T far *lnk[2];
	byte	flg;
	byte	flg2;
	word	tok;				/* T_BLKLBL */
	byte	name[LBL_NAME_SIZ];
	union ST_T far *grp;
	BrkCont_t far *blklblp;

};
struct ST_T_NODE {
	/* union ST_T far *next; */
	union ST_T far *lnk[2];
	byte	flg;
};
typedef union ST_T {
	struct ST_T_PROC  p;
	struct ST_T_VAR   v;
	struct ST_T_CONST c;
	struct ST_T_LBL   l;
	struct ST_T_GRP   g;
	struct ST_T_NODE  n;
	struct ST_T_BLKLBL b;
	struct ST_T_MACFUNC m;
} St_t;

typedef St_t far* St_t_fp;

int 	St_Init(void);
St_t_fp St_New(void);
St_t_fp St_Ins(byte_fp name, St_t_fp far*);
St_t_fp St_Search(byte far*, St_t_fp);
void	St_Free(St_t_fp);
void	St_Frees(St_t_fp);
St_t_fp St_Sav(void);

/*---------------- Et ---------------- "tbl.c" */
struct ET_T_EP {
	union ET_T far *next;
	word op;
	union ET_T far *lep;
	union ET_T far *rep;

};
struct ET_T_ST {
	union ET_T far *next;
	word op;
	union ST_T far *st;
	union ST_T far *st2;
};
struct ET_T_MEM {
	union ET_T far *next;
	word op;
	union ET_T far *lep;
	int  ofs;
	byte typ;
	byte seg;
	word siz;
};
struct ET_T_CNSTNT {
	union ET_T far *next;
	word op;
	long val;
	//union ET_T far *rep;
	word reg;
};
struct ET_T_JMP {
	union ET_T far *next;
	word op;		/* T_COND */
	union ET_T far *lep;
	word cond;  	/* 0:et 1Å`  GO_CF,GO_NEQ... */
	byte mode;  	/* 0:ê¨óßÇ∑ÇÍÇŒJMP 1:ê¨óßÇµÇ»ÇØÇπÇŒJMP */
	byte nf;		/* 0:ÇªÇÃÇ‹Ç‹ 1:èåèîΩì] */
};
struct ET_T_FUNC_EX {
	union ET_T far *next;
	word	intr_no;	/* intr proc Ç≈ÇÃäÑçûî‘çÜ */
	union ET_T far *lep;/* Ç≥ÇÁÇ…ägí£ */
	word	in; 	/* ah,al,bh,bl,ch,cl,dh,dl,si,di,es,ds,fx,cf,df,arg*/
	word	out;
	word	brk;
};

struct ET_T_CLSTR {
	union ET_T far *next;
	byte  buf[12];
};

#if 0
struct ET_T_STR {
	union ET_T far *next;
	word op;
	byte seg;
	byte typ;
	byte_fp str;
	word len;
};
#endif

typedef union ET_T {
	struct ET_T_EP  	e;

	struct ET_T_CNSTNT  c;
	struct ET_T_ST  	v;
	struct ET_T_MEM 	m;
	struct ET_T_JMP 	j;
	struct ET_T_FUNC_EX f;
	struct ET_T_CLSTR	b;
 /* struct ET_T_STR 	s; */
} Et_t;
typedef Et_t far* Et_t_fp;

Et_t_fp Et_New(void);
void	Et_Frees(Et_t far*);
Et_t_fp Et_Sav(void);
int 	Et_Init(void);
#define Et_NEWp(p)  (p = Et_New())

/*---------------- Expr & Gen ----------------*/
static enum {
	GO_EQU=2,GO_NEQ,
	GO_GT,  GO_LE,
	GO_LT,  GO_GE,
	GO_IGT, GO_ILE,
	GO_ILT, GO_IGE,
	GO_C,	GO_NC,
	GO_Z,	GO_NZ,
	GO_S,	GO_NS,
	GO_O,	GO_NO,
	GO_P,	GO_NP,
	GO_CX,  GO_NCX,
	GO_DECX,GO_NDECX,
	GO_DECX_Z,  GO_NDECX_Z,
	GO_DECX_NZ, GO_NDECX_NZ,
	GO_JMP, GO_NJMP
};
#define Op_ShiftEq(c)	((c) >= I_SHLEQ && (c) <= I_SAREQ))
#define Op_Bin(c)		((c) >= I_EXG	&& (c) <= I_IDIVS))
#define Op_Bin2(c)  	((c) >= I_EQEQ  && (c) <= I_SAR))
#define Op_One(c)		((c) >= I_AAADD && (c) <= I_WAIT)
#define Op_Reg(c)		((c) >= I_AH	&& (c) <= I_SS)
#define Op_Reg1(c)  	((c) >= I_AH	&& (c) <= I_BL)
#define Op_Reg1h(c) 	((c) >= I_AH	&& (c) <= I_BH)
#define Op_Reg1l(c) 	((c) >= I_AL	&& (c) <= I_BL)
/*#define	Op_Regs(c)  ((c) >= I_AX	&& (c) <= I_SS)*/
#define Op_Seg2(c)  	((c) >= I_CS	&& (c) <= I_SS)
#define Op_RegI(c)  	((c) >= I_BX	&& (c) <= I_BP)
#define Op_Reg2(c)  	((c) >= I_AX	&& (c) <= I_SP)
#define Op_RegD(c)  	((c) >= I_AX	&& (c) <= I_BX)
#define RegXtoL(c)  	((c)-4)
#define RegXtoH(c)  	((c)-8)
#define RegLtoX(c)  	((c)+4)

/* out.c */
#define IsEp_Op(p,t)	(p->e.op == t)
#define IsEp_Reg1(p)	(p->e.op == T_R1)
#define IsEp_Reg2(p)	(p->e.op == T_R2)
#define IsEp_Reg4(p)	(p->e.op == T_R4)
#define IsEp_Mem1(p)	(p->e.op == T_M1)
#define IsEp_Mem2(p)	(p->e.op == T_M2)
#define IsEp_Mem4(p)	(p->e.op == T_M4)
#define IsEp_Seg2(p)	(p->e.op == T_SEG2)
#define IsEp_Seg4(p)	(p->e.op == T_SEG4)
#define IsEp_Cnst(p)	(p->e.op == T_CNST)
int 	IsEp_Mem124(Et_t_fp ep);
int 	IsEp_Reg12(Et_t_fp ep);
int 	IsEp_Dual(Et_t_fp ep);
int 	IsEp_Rht4(Et_t_fp ep);
int 	IsEp_Rht1(Et_t_fp ep);
int 	IsEp_Rht12(Et_t_fp ep);
int 	IsEp_Lft124(Et_t_fp ep);
int 	IsEp_Lft12(Et_t_fp ep);
int 	IsEp_Lft4S(Et_t_fp ep);
int 	IsEp_Lft4(Et_t_fp ep);
int 	IsEp_Lft2(Et_t_fp ep);
int 	IsEp_Lft1(Et_t_fp ep);
int 	IsEp_W2DCnst(Et_t_fp ep);
int 	IsEp_Cnsts(Et_t_fp ep);
int 	IsEp_CnstTyp(word t,Et_t_fp ep);
int 	IsEp_CnstVal(Et_t_fp ep, long n);
int 	IsEp_R1op(Et_t_fp ep, word n);
int 	IsEp_R2op(Et_t_fp ep, word n);
int 	IsEp_R4op(Et_t_fp ep, word n);
int 	IsEp_MemMem(Et_t_fp,Et_t_fp);

#define RdEp_Val(ep)	(ep->c.val)
#define RdEp_Lef(ep)	(ep->e.lep)
#define RdEp_Rig(ep)	(ep->e.rep)

/* "expr.c" */
Et_t_fp Expr(byte);
Et_t_fp Expr_Stat(byte mode);
long	Expr_Cnstnt(void);
Et_t_fp Expr_Cond(void);
Et_t_fp Expr_CnvRVal(byte, Et_t_fp);
byte	Expr_RVal(Et_t_fp);
word	Expr_CondNeg(word);

/* "gen.c" */
#ifdef EXT
  struct ET_T_CNSTNT gEtZero = {NULL,T_CNST,0L,0};
#else
  extern struct ET_T_CNSTNT gEtZero;
#endif
#define EV_ZERO  ((Et_t *)&gEtZero)
void	Gen_Expr(Et_t_fp ,word);
void	Gen_CondExpr(Et_t_fp ,word,word,word,word);
void	Gen_EquFlg(int);
void	Gen_ChkEquFlg(void);
void	Gen_Equ(Et_t_fp lp, Et_t_fp rp);
void	Gen_Equ4(Et_t_fp lp, Et_t_fp rp);
void	Gen_Rep(word t);
void	Gen_SubEq(Et_t_fp lp, Et_t_fp rp);
void	Gen_Pusha(void);
void	Gen_Popa(void);
Et_t_fp Ev_Reg(word);
Et_t_fp Ev_Reg2(word);
Et_t_fp Ev_Reg4(word);
Et_t_fp Ev_Cnst(long);
Et_t_fp Ev_Cnst2(long);

/*--------------- Ch --------------- "sym.c" */
EXTERN  byte	*Ch_srcName;		/* åªç›ì¸óÕÇµÇƒÇ¢ÇÈÉtÉ@ÉCÉã */
EXTERN  FILE	*Ch_file;			/* ì¸óÕÉtÉ@ÉCÉãÇÃÉtÉ@ÉCÉãÅEÉ|ÉCÉìÉ^ */
EXTERN  word	Ch_line;			/* ì¸óÕÇµÇƒÇ¢ÇÈçsî‘çÜ */
word	Ch_GetLine(byte *,word);
word	Ch_Get(void);
word	Ch_GetK(void);
void	Ch_Unget(int  c);
word	Ch_SkipSpc(void);
int 	Ch_ChkPeriod(void);
byte	Ch_ChkBW(void);
void	Ch_Top(void);
void	Sym_MacInclude(void);

/*---------------- Sym ---------------- "sym.c" */
#define PAR_CR_INC()	do {if (Opt_parCr) Sym_crMode++;} while(0)
#define PAR_CR_DEC()	do {if (Opt_parCr) Sym_crMode--;} while(0)
EXTERN  int 	Sym_crMode;
EXTERN  byte	Sym_name[LBL_NAME_SIZ+256];
EXTERN  word	Sym_tok;
EXTERN  word	Sym_typ;
EXTERN  word	Sym_reg;
EXTERN  word	Sym_flg;
EXTERN  long	Sym_val;
EXTERN  int 	Sym_rsvflg;
EXTERN  int 	Sym_strLen;
EXTERN  byte_fp Sym_str;
EXTERN  St_t_fp Sym_sp;
word	Ch_Init(void);
word	Sym_Get(void);
word	Sym_GetRsvOff(void);
word	Sym_GetRsvOff2(void);
int 	Sym_NameGet(word);
#ifdef DEBUG
EXTERN  byte_fp Deba_fname;
EXTERN  int 	Deba_line;
#define Sym_ChkEol() do\
	{Deba_fname = __FILE__;Deba_line = __LINE__;_Sym_ChkEol();} while(0)
void	_Sym_ChkEol(void);
#define Sym_GetChkEol() do\
	{Deba_fname=__FILE__;Deba_line=__LINE__;_Sym_GetChkEol();} while(0)
void	_Sym_GetChkEol(void);
#else
void	Sym_ChkEol(void);
void	Sym_GetChkEol(void);
#endif
void	Rsv_List(void);
void	Sym_SkipCR(void);
int 	Sym_GetChkLP(void);
int 	Sym_ChkLP(void);
int 	Sym_ChkRP(void);
int 	Sym_ChkLC(void);
int 	Sym_ChkRC(void);
int 	Sym_ChkLB(void);
int 	Sym_ChkRB(void);
void	Sym_MacDef(void);

/*---------------- Out ---------------- "out.c" */
EXTERN  FILE	*Out_file;  		/* èoóÕÉtÉ@ÉCÉãÇÃÉtÉ@ÉCÉãÅEÉ|ÉCÉìÉ^ */
void	Out_PubLblStr(byte_fp);
void	Out_LblStr(byte_fp);
void	Out_Line(byte_fp);
void	Out_Nm(byte_fp);
void	Out_NmSt(byte_fp ,byte_fp);
void	Out_LblNmSt(byte_fp l,byte_fp p,byte_fp q);
void	Out_NmStSt(byte_fp p,byte_fp q,byte_fp r);
void	Out_Nem0(word);
void	Out_Nem1(word, Et_t_fp);
void	Out_Nem2(word,Et_t_fp ,Et_t_fp);
void	Out_Nem3(word,Et_t_fp ,Et_t_fp, Et_t_fp);
void	Out_golbl(word n);
void	Out_Leave(void);
void	Out_Enter(word siz,word level);
void	Out_LblTyp(St_t_fp sp, int  typ);
void	Out_Dup(long t,word n);
void	Out_ShiftReg12Cnst(word t,word reg,int n);
byte_fp Str_StName(St_t_fp);

/*--------------- Err --------------- "out.c" */
EXTERN  FILE *Err_File;
EXTERN  int  Err_cnt;
#ifdef DEBUG
#define Msg_Err(s)  Msg_Err_(s,__FILE__,__LINE__)
void	Msg_Err_(byte_fp ,byte *, word);
#define Msg_PrgErr(s)	Msg_Err_(s,__FILE__,__LINE__)
void	Msg_PrgErr_(byte_fp ,byte *, word);
#else
#define Msg_Err(s)  Msg_Err_(s)
void	Msg_Err_(byte_fp);
#define Msg_PrgErr(s)	Msg_Err_(s)
void	Msg_PrgErr_(byte_fp);
#endif

/*---------------- ETC ---------------*/
enum {ANAERR_EOF = 1,ANAERR_MEM,ANAERR_MIF,ANAERR_ENDP,ANAERR_MMOD};
EXTERN  St_t_fp Mod_sp;
EXTERN  byte_fp Mod_grps[3];
EXTERN  FILE*	Rsp_file;
EXTERN  FILE*	Ana_partLstFile;
EXTERN  St_t_fp Ana_modelsp;
EXTERN  byte_fp Ana_incDir;
EXTERN  inte	Ana_model;  		/* TINY,SMALL,MEDIUM */
EXTERN  byte	Ana_mode;			/* MD_??? */
EXTERN  byte	Ana_err;			/* 1Ç»ÇÁEOF 2Ç»ÇÁ“”ÿ•ÃŸ */
EXTERN  byte	Ana_saveOfs;		/* saveÇ…ÇÊÇÈ⁄ºﬁΩ¿ï€ë∂Ç≈ÇÃµÃæØƒ */
EXTERN  byte	Opt_comment;		/* ø∞ΩÇ∫“›ƒÇ…ÇµÇƒèoóÕ */
EXTERN  byte	Opt_cpu;			/* cpu¿≤Ãﬂ 0:8086 1:186 2:286 3:396 */
EXTERN  byte	Opt_r86;			/* LSI R86 */
EXTERN  byte	Opt_partFlg;		/* @part ÇçsÇ§Ç©Ç«Ç§Ç© */
EXTERN  byte	Opt_auto;			/* à¯êîÇÃà√ñŸÇÃ⁄ºﬁΩ¿ë„ì¸ */
EXTERN  byte	Opt_namePub;		/* ê∂ê¨Ç∑ÇÈÉçÅ[ÉJÉãÇ»ñºëOÇ publicÇ… */
EXTERN  byte	Opt_procFlg;		/* èoóÕÇ…procÅ`endp ÇégÇ§1,égÇÌÇ»Ç¢ 0 */
EXTERN  byte	Opt_zeroBSS;		/* var ïœêîÇ≈ 0 èâä˙âªÇ∑ÇÈÇ‡ÇÃÇ BSSÇ… */
EXTERN  byte	Opt_parCr;  		/* (..) ì‡ÇÃâ¸çsÇãÛîíàµÇ¢Ç…Ç∑ÇÈ */
EXTERN  byte	Opt_procAssumeFlg;  /* */
EXTERN  byte_fp Ana_dirName;		/* èoóÕêÊÉfÉBÉåÉNÉgÉä */
#define NAM_SIZE 85
EXTERN  byte	Ana_outName[NAM_SIZE];
EXTERN  byte	Ana_partIncName[NAM_SIZE];
EXTERN  int 	Ana_align;
EXTERN  int 	Ana_mmodFlg;		/* import º¿ file √ﬁ module ∂ﬁ Ã∏Ω≥ ±Ÿ ∂?*/
EXTERN	int		Ana_moduleMode;		/* module ï∂Ç™ïKóvÇ©Ç«Ç§Ç©ÇÃÉtÉâÉO */
EXTERN	int		Ana_oldver;			/* v0.17-v0.20ÇÃï∂ñ@ÇãñÇ∑ */
#ifdef EXT
	byte_fp Ana_ext = "ANA";
	byte_fp Ana_hdrext = "HAN";
#else
	extern byte_fp Ana_ext;
	extern byte_fp Ana_hdrext;
#endif
FILE*	fopen_e(char *,char *);

/* stat.c */
int  Stat(void);
word Sym_Get(void);
void	SkipSym(word t);
St_t_fp GoLbl_New(byte_fp name);
St_t_fp Sym_NameNew(byte far *name,word t,word flg);
extern  BrkCont_t *gStatBrkCont_p;

/* decl.c */
#ifdef EXT
	byte_fp gJmps[2] = {"jmp short","jmp"};
	byte_fp gJmpf[2] = {"jmp far ptr","jmpf"};
	byte_fp gCallf[2] = {"call far ptr","callf"};
#else
	extern byte_fp gJmps[2];
	extern byte_fp gJmpf[2];
	extern byte_fp gCallf[2];
#endif
EXTERN  St_t_fp St_inpRoot; 		/* moduleñºÇÃä«óùópŒﬂ≤›¿ */
EXTERN  St_t_fp St_expoRoot;		/* export ◊ÕﬁŸÇÃä«óùópŒﬂ≤›¿ */
EXTERN  St_t_fp St_rsvRoot; 		/* -DÇ≈ê›íËÇ≥ÇÍÇΩ◊ÕﬁŸÇÃä«óùópŒﬂ≤›¿ */
EXTERN  St_t_fp St_root;			/* moduleì‡∏ﬁ€∞ ﬁŸ•◊ÕﬁŸÇÃä«óùópŒﬂ≤›¿ */
EXTERN  St_t_fp St_localRoot;		/* €∞∂Ÿ•◊ÕﬁŸÇÃä«óùópŒﬂ≤›¿ */
EXTERN  byte Decl_defFlg;
int 	Decl_Module(int);
void	Decl_Return(void);
void	Decl_Load(void);
int 	Decl_Data(word cnt);
int 	Decl_Proc(word op);
void	Decl_Var(St_t_fp cp,int a);
void	Decl_Type(void);
void	Decl_Struct(void);
void	Decl_Const(void);
void	Decl_Align(void);
void	Out_DAssume(void);
void	Out_Seg(St_t_fp sp);
EXTERN  St_t_fp gSeg_sp[4];
#ifdef MVAR
EXTERN  word gMvarOfs;
void	Decl_Mvar(void);
#endif

/* "gen.c" */
#define GoLbl_SIZ 12
EXTERN  word GoLbl_no;  		/* ì‡ïîÉâÉxÉãÇÃî‘çÜ */
EXTERN  byte Gen_condMode;  	/* 0:ïÑçÜïœâªñ≥éã 1:ç›ÇË 2:ç›ÇË(äÓñ{ñΩóﬂÇÃÇ›)*/
byte_fp GoLbl_Strs(word n);
word	GoLbl_NewNo(void);
void	Gen_PushPop(word t,Et_t_fp lp);

/* "mem.c" */
void Mac_Init(void);
void Mac_Putc(word);
byte_fp Mac_Pos(void);

/* "deb.c" */
#ifdef DEBUG
 #define EXPMSG(n,p) Deb_EtMsg(n,p)
 void	Deb_EtMsg(byte *,Et_t_fp);
 byte_fp	Deb_ChkOp(word);
 void Deb_SymNam(word t);
#else
 #define EXPMSG(n,p)
#endif
EXTERN byte Deb_macFlg;

//
typedef struct Sss_t {
	struct Sss_t *next;
	byte far *str;
	int  no;
} Sss_t;
#ifdef EXT
	Sss_t *Expr_strl = NULL;
	Sss_t *Expr_strlTop = NULL;
#else
	extern Sss_t *Expr_strl;
	extern Sss_t *Expr_strlTop;
#endif
