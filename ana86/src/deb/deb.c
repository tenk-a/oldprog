#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <alloc.h>
#include "anadef.h"

/*---------------------------------------------------------------------------*/

#ifdef DEBUG
typedef struct {
	word t;
	byte *name;
} chktbl_t;

chktbl_t Deb_ChkTbl[] = {
	{I_EOF,		"EOF"},
	{I_BYTE, 	"byte"},
	{I_WORD, 	"word"},
	{I_PTR,  	"pointer"},
	{I_DWORD,	"dword"},
	{I_FPTR, 	"fptr"},
	{I_SEGPTR,   "segptr"},
	{I_NEAR, 	"near"},
	{I_QWORD,	"qword"},
	{I_FAR,  	"far"},
	{I_TBYTE,	"tbyte"},

	/* reg*/
	{I_AH,   	"ah"},
	{I_CH,   	"ch"},
	{I_DH,   	"dh"},
	{I_BH,   	"bh"},

	{I_AL,   	"al"},
	{I_CL,   	"cl"},
	{I_DL,   	"dl"},
	{I_BL,   	"bl"},

	{I_AX,   	"ax"},
	{I_CX,   	"cx"},
	{I_DX,   	"dx"},
	{I_BX,   	"bx"},

	{I_DI,   	"di"},
	{I_SI,   	"si"},
	{I_BP,   	"bp"},

	{I_SP,   	"sp"},
	{I_IP,   	"ip"},

	{I_CS,   	"cs"},
	{I_DS,   	"ds"},
	{I_ES,   	"es"},
	{I_SS,   	"ss"},

	{I_FH,   	"fh"},
	{I_FL,   	"fl"},
	{I_FX,   	"fx"},

	/* flag */
	{I_CF,   	"cf"},
	{I_AF,   	"af"},
	{I_PF,   	"pf"},
	{I_SF,   	"sf"},
	{I_ZF,   	"zf"},
	{I_IIF,  	"iif"},
	{I_TF,   	"tf"},
	{I_DF,   	"df"},
	{I_OVF,  	"ovf"},

	{I_EXG,  	"<=>"},
	{I_LEA,  	"(lea)"},
	{I_SIEQ, 	".=."},
	{I_LDS,  	"(lds)"},
	{I_LES,  	"(les)"},
	{I_EQU,  	"="},

	{I_EQEQ, 	"=="},
	{I_NEQ,  	"!="},
	{I_ADCEQ,	"adc"},
	{I_SBCEQ,	"sbc"},
	{I_ADD,  	"+"},
	{I_ADDEQ,	"+="},
	{I_SUB,  	"-"},
	{I_SUBEQ,	"-="},
	{I_AND,  	"&"},
	{I_ANDEQ,	"&="},
	{I_LAND, 	"&&"},
	{I_OR,   	"|"},
	{I_OREQ, 	"|="},
	{I_LOR,  	"||"},
	{I_XOR,  	"^"},
	{I_EOREQ,	"^="},
	{I_LT,   	"<"},
	{I_LE,   	"<="},
	{I_SHL,  	"<<"},
	{I_GT,   	">"},
	{I_GE,   	">="},
	{I_SHR,  	">>"},
	{I_ILT,  	".<."},
	{I_ILE,  	".<=."},
	{I_IGT,  	".>."},
	{I_IGE,  	".>=."},
	{I_SAR,  	".>>."},

	{I_SHLEQ,	"<<="},
	{I_SHREQ,	">>="},
	{I_RCLEQ,	"rcl"},
	{I_RCREQ,	"rcr"},
	{I_ROLEQ,	"rol"},
	{I_ROREQ,	"ror"},
	{I_SAREQ,	".>>="},

	{I_SARS, 	"sar"},
	{I_SHLS, 	"shl"},
	{I_SHRS, 	"shr"},

	{I_MUL,  	"*"},
	{I_MULEQ,	"*="},
	{I_IMUL, 	".*."},
	{I_DIV,  	"/"},
	{I_DIVEQ,	"/="},
	{I_IDIV, 	"./."},
	{I_MOD,  	"%"},
	{I_MODEQ,	"%="},
	{I_GLOBLE,   "%%"},
	{I_IMOD, 	".%."},
	{I_DIVS, 	"div"},
	{I_IDIVS,	"idiv"},

	{I_CLN,  	":"},
	{I_CLNEQ,	":="},
	{I_CR,   	"\n"},
	{I_SHARP,	"#"},
	{I_DOLL, 	"$"},
	{I_ATMARK,   "@"},
	{I_YEN,  	"\\"},
	{I_DQUOT,	"\""},
	{I_APO,  	"'"},
	{I_PERIOD,   "."},
	{I_PP,   	".."},
	{I_PPP,  	"..."},
	{I_QUESTION, "?"},
	{I_COMMA,	"},"},
	{I_SMCLN,	";"},
	{I_LP,   	"("},
	{I_RP,   	")"},
	{I_LB,   	"["},
	{I_RB,   	"]"},
	{I_LC,   	"{"},
	{I_RC,   	"}"},

	{I_COM,  	"~"},
	{I_NOT,  	"!"},
	{I_INC,  	"++"},
	{I_NEG,  	"íPçÄ-"},
	{I_DEC,  	"--"},
	{I_NEGS, 	"neg"},
	{I_COMS, 	"com"},

	{I_AAADD,	"aaadd"},
	{I_AADIV,	"aadiv"},
	{I_AAMUL,	"aamul"},
	{I_AASUB,	"aasub"},
	{I_DAADD,	"daadd"},
	{I_DASUB,	"dasub"},
	{I_LEAVE,	"leave"},
	{I_LOCK, 	"lock"},
	{I_HLT,  	"hlt"},
	{I_NOP,  	"nop"},
	{I_INTO,	"into"},
	{I_IRET, 	"iret"},
	{I_POPA, 	"popa"},
	{I_PUSHA,	"pusha"},
	{I_WAIT, 	"wait"},

	{I_CLRC, 	"clrc"},
	{I_SETC, 	"setc"},
	{I_CLRI, 	"clri"},
	{I_SETI, 	"seti"},
	{I_CLRD, 	"clrd"},
	{I_SETD, 	"setd"},
	{I_COMC, 	"comc"},
	{I_XLAT, 	"xlat"},
	{I_SEXT1,	"cbw"},
	{I_SEXT2,	"cwd"},
	{I_LAHF, 	"lahf"},
	{I_SAHF, 	"sahf"},
	{I_PUSHF,	"pushf"},
	{I_POPF, 	"popf"},
	{I_ESC,  	"esc"},

	{I_REP,  	"rep"},
	{I_REPE, 	"repe"},
	{I_REPN, 	"repn"},
	{I_POP,  	"pop"},
	{I_PUSH, 	"push"},

	{I_INTR, 	"intr"},
	{I_ENTER,	"enter"},
	{I_BOUND,	"bound"},
	{I_BEGIN,	"begin"},

	{I_GO,   	"go"},
	{I_GOTO, 	"goto"},
	{I_GOSUB,	"gosub"},
	{I_RETURN,   "return"},
	{I_RETN, 	"retn"},
	{I_RET,  	"ret"},
	{I_RETF, 	"retf"},
	{I_JMPS, 	"jmp short"},
	{I_JMP,  	"jmp"},
	{I_JMPF, 	"jmpf"},
	{I_CALLN,	"calln"},
	{I_CALL, 	"call"},
	{I_CALLF,	"callf"},

	{I_IF,   	"if"},
	{I_ELSIF,	"elsif"},
	{I_ELSE, 	"else"},
	{I_FI,   	"fi"},
	{I_EXIT,		"exit"},
	{I_NEXT,		"next"},
	{I_LOOP, 	"loop"},
	{I_ENDLOOP,  "pool"},
	{I_DO,   	"do"},
	{I_ENDDO,	"od"},

	{I_VAR,  	"var"},
	{I_CVAR, 	"code"},
	{I_MVAR, 	"mvar"},
	{I_LOCAL,	"auto"},
	{I_CONST,	"const"},
	{I_TYPE, 	"type"},
	{I_DATA, 	"data"},
	{I_ATAD, 	"enddata"},
	{I_STRUCT,   "struct"},
	{I_ENDSTRUCT,"endstruct"},

	{I_PROC, 	"proc"},
	{I_CPROC,	"c_proc"},
	{I_ENDP, 	"corp"},
	{I_EXTERN,   "extern"},
	{I_FORWORD,  "forword"},
	{I_MACRO,	"macro"},
	{I_BEFORE,   "macro"},
	{I_CDECL,   "c_decl"},
	{I_SAVE, 	"save"},
	{I_LOAD, 	"load"},
	{I_IN,   	"in"},
	{I_OUT,  	"out"},
	{I_BREAK,	"exit"},

	{I_MODULE,   "module"},
	{I_ENDMODULE,"endmodule"},
	{I_PART,		"part"},
	{I_MODEL,	"model"},
	{I_IMPORT,   "import"},
	{I_CNAME,	"c_name"},
	{I_STACK,	"stack"},
	{I_START,	"start"},
	{I_ASSUME,   "assume"},
	{I_SEG,  	"seg"},
	{I_PARA, 	"para"},
	{I_PAGE, 	"page"},
	{I_PUBLIC,   "public"},
	{I_PRIVATE,  "private"},
	{I_COMMON,   "common"},
	{I_NOTHING,  "nothing"},
	{I_ORG,  	"org"},

	{I_ORIG, 	"orig"},
	{I_DEFINED,  "defined"},
	{I_SIZEOF, 	"size"},
	{I_W2D,  	"ww"},
	{I_PORT, 	"port"},
	{I_BXAL, 	"BYTE[bx+al]"},
	{I_ERR,  	"(err)"},

	{T_CNST, 	"(íËêî)"},
	{T_EXTCNST,  "(◊ÕﬁŸÇÃíl)"},
	{T_CNSTEXPR, "(◊ÕﬁŸñºÇÃåÇ¥Ç¡ÇΩíËêîéÆ)"},
	{T_OFS,  	"(%◊ÕﬁŸ)"},
	{T_EXTCONST, "(external const)"},
	{T_R1,   	"(reg1)"},
	{T_R2,   	"(reg2)"},
	{T_R4,   	"(reg4)"},
	{T_SEG2, 	"(seg)"},
	{T_SEG4, 	"(seg)"},
	{T_M1,   	"(mem1)"},
	{T_M2,   	"(mem2)"},
	{T_M4,   	"(mem4)"},
	{T_ADDR, 	"(addr)"},
	{T_JMPLBL,   "(jmplbl)"},
	{T_SEG,  	"(seg)"},
	{T_ONE,  	"(one)"},

	{T_COND, 	"(î‰är)"},
	{T_LBL,  	"(LBL)"},
	{T_BLKLBL,   "(BLKLBL)"},
	{T_IDENT,	"(ident)"},
	{T_STRING,   "(ï∂éöóÒ)"},
	{T_CONST,	"(const)"},
	{T_TYPE, 	"(type)"},
	{T_STRUCT,   "(strict)"},
	{T_ARRAY,	"(array)"},
	{T_MEMB, 	"(member)"},
	{T_CLASS,	"(class)"},
	{T_VAR,  	"(var)"},
	{T_LOCAL,	"(€∞∂Ÿïœêî)"},
	{T_ARGLOCAL, "(à¯êîïœêî)"},
	{T_STARTLBL, "(export)"},
	{T_PROC, 	"(proc)"},
	{T_INTRPROC, "(proc)"},
	{T_PROC_DECL,"(proc_decl)"},
	{T_MACPROC,  "(macro)"},
	{T_GROUP,	"(group)"},
	{T_MODULE,   "module"},
	{T_MODULEVAR,"(modulevar)"},
	{T_MACROSTR, "(macrostr)"},

	{I_TO,   	"to"},
	{I_DB,   	"db"},
	{I_DD,   	"dd"},
	{I_DW,   	"dw"},
	{I_SWITCH,   "switch"},
	{I_DEFAULT,  "default"},
	{I_CASE, 	"case"},
	{I_ESAC, 	"esac"},
	{I_CMP,  	"comp"},
	{I_STATIC,   "static"},
	{I_CLASS,	"class"},
	{I_ENDCLASS, "endclass"},
	{I_ARG,  	"arg"},
	{I_ARRAY,	"array"},
	{I_OF,   	"of"},
	{I_GROUP,	"group"},
	{I_INP,  	"inp"},
	{I_OUTP, 	"outp"},
	{I_CODE, 	"code"},
	{I_DECL, 	"decl"},

	{M_IF,   	"@if"},
	{M_ELSIF,	"@elsif"},
	{M_ELSE, 	"@else"},
	{M_ENDIF,	"@fi"},
	{M_SET,  	"@set"},
	{M_MSG,  	"@msg"},
	{M_DEFINE,   "@define"},
	{T_SYMEND,   "(static)"},

};

void
Deb_SymNam(word t)
{
	word i;

	for (i = 0;i < T_SYMEND;i++) {
		if (t == Deb_ChkTbl[i].t) {
			MSGF(("[%s]",Deb_ChkTbl[i].name));
			return;
		}
	}
}

byte_fp
Deb_ChkOp(word t)
{
	word i;

	for (i = 0;i < T_SYMEND;i++) {
		if (t == Deb_ChkTbl[i].t) {
			return Deb_ChkTbl[i].name;
		}
	}
	return "<!bad!>";
}

void
Deb_EtMsg(byte *msg,Et_t_fp xp)
{
	MSGF(("\n<%s [%lx  op=%s]>\n",msg,xp,Deb_ChkOp(xp->e.op)));
}
#endif
