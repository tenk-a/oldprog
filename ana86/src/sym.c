#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <alloc.h>
#include "anadef.h"

/*---------------------- ctbl.h ---------------------------------------------*/
#define CHR  0x00
#define CT2  0x00
#define CTL  0x01
#define CR   0x01
#define SPC  0x01
#define PUN  0x02
#define PEQ  0x04
#define PU2  0x08
#define PUX  0x10
#define DDG  0x20
#define XDG  0x40
#define DGT  (DDG|XDG)
#define ALP  0x80
#define ALD  (XDG|ALP)
#define ASP  ALP
#define ASD  ALD
#define KJ1  0x00

#define isCsymF(c)  	(gChrTbl[(byte)c]&(ALP|ASP))
#define isCsym(c)		(gChrTbl[(byte)c]&(ALP|ASP|DGT))
#define isDigit(c)  	(gChrTbl[(byte)c]&DDG)
#define isXdigit(c) 	(gChrTbl[(byte)c]&XDG)
#define isSpace(c)  	(gChrTbl[(byte)c]&SPC)
#define isPunCt(c)  	(gChrTbl[(byte)c]&(PUN|PU2|PUX|PEQ))
#define isPun(c)		(gChrTbl[(byte)c]&PUN)
#define isPunEq(c)  	(gChrTbl[(byte)c]&PEQ)
#define isPun2(c)		(gChrTbl[(byte)c]&PU2)
#define isPunX(c)		(gChrTbl[(byte)c]&PUX)
#define isPunEx(c)  	(gChrTbl[(byte)c]&(PEQ|PU2|PUX))

#define toXdigit(c) 	(isDigit(c) ? (c)-'0'\
						:(((c) >= 'A' && (c) <= 'F') ? (c) - 'A'\
						:(((c) >= 'a' && (c) <= 'f') ? (c) - 'a' : 0) ) )

extern unsigned char gChrTbl[256];
extern unsigned char gSymTbl[128];
/*---------------------- ctbl.c ---------------------------------------------*/
byte gChrTbl[256] = {
		0,CTL,CTL,CTL,CTL,CTL,CTL,SPC,
		CTL,SPC,CR,SPC,SPC,CR,CTL,CTL,
		CTL,CTL,CTL,CTL,CTL,CTL,CTL,CTL,
		CTL,CTL,CTL,CTL,CTL,CTL,CTL,CTL,

		SPC,PEQ,PUN,PUN,CHR,PEQ|PU2,PEQ|PU2,PUN,
		PUN,PUN,PEQ,PEQ|PU2,PUN,PEQ|PU2|PUX,PUX,PEQ|PU2|PUX,
		DGT,DGT,DGT,DGT,DGT,DGT,DGT,DGT,
		DGT,DGT,PUN,PUN,PEQ|PU2,PEQ,PEQ|PU2,PUN,

		PUX,ALD,ALD,ALD,ALD,ALD,ALD,ALP,
		ALP,ALP,ALP,ALP,ALP,ALP,ALP,ALP,
		ALP,ALP,ALP,ALP,ALP,ALP,ALP,ALP,
		ALP,ALP,ALP,PUN,PUN,PUN,PEQ,ALP|ASP,

		CHR,ASD,ASD,ASD,ASD,ASD,ASD,ASP,
		ASP,ASP,ASP,ASP,ASP,ASP,ASP,ASP,
		ASP,ASP,ASP,ASP,ASP,ASP,ASP,ASP,
		ASP,ASP,ASP,PUN,PEQ|PU2,PUN,PUN,CT2,

		0,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,
		KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,
		KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,
		KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,

		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,
		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,
		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,
		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,

		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,
		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,
		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,
		CHR,CHR,CHR,CHR,CHR,CHR,CHR,CHR,

		KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,
		KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,
		KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,KJ1,
		KJ1,KJ1,KJ1,KJ1,KJ1,CT2,CT2,CT2
};

byte gSymTbl[128] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,I_NOT,I_DQUOT,I_SHARP,I_DOLL,I_MOD,I_AND,I_APO,
				I_LP,I_RP,I_MUL,I_ADD,I_COMMA,I_SUB,I_PERIOD,I_DIV,
		0,0,0,0,0,0,0,0,0,0,I_CLN,I_SMCLN,I_LT,I_EQU,I_GT,I_QUESTION,
		I_ATMARK,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,I_LB,I_YEN,I_RB,I_XOR,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,I_LC,I_OR,I_RC,I_COM,0
};

/*-------------------------------- rsvtbl.c --------------------------------*/
typedef struct {
	byte *name;
	word tok;
} Rsv_t;

#define RSVTBLSIZ	(sizeof oRsvTbl / sizeof oRsvTbl[0])

static Rsv_t oRsvTbl[] = {
	{"",		0		},
	{"aaadd",	I_AAADD },
	{"aadiv",	I_AADIV },
	{"aamul",	I_AAMUL },
	{"aasub",	I_AASUB },
	{"adc", 	I_ADCEQ },
	{"af",  	I_AF	},
	{"ah",  	I_AH	},
	{"al",  	I_AL	},
	//{"arg",	I_ARG	},
	//{"array",   I_ARRAY },
	{"assume",  I_ASSUME},
	/*{"atad",  I_ATAD  },*/
	/*{"auto",  I_LOCAL },*/
	{"ax",  	I_AX	},
	{"b",		I_BYTE  },
	{"before",  I_BEFORE},
	{"begin",	I_BEGIN },
	{"bh",  	I_BH	},
	{"bl",  	I_BL	},
	{"block",	I_DO	},
	{"bound",	I_BOUND },
	{"bp",  	I_BP	},
	{"break",	I_BREAK },
	{"bx",  	I_BX	},
	{"byte",	I_BYTE  },
	//{"c_decl",  I_CDECL },
	{"c_name",  I_CNAME },
	/*{"c_proc",  I_CPROC },*/
	{"call",	I_CALL  },
	//{"case",  I_CASE  },
	{"cdecl",	I_CDECL },
	{"cf",  	I_CF	},
	{"ch",  	I_CH	},
	{"cl",  	I_CL	},
	//{"class",   I_CLASS },
	//{"code",  I_CODE  },
	{"com", 	I_COMS  },
	{"common",  I_COMMON},
	{"const",	I_CONST },
	/*{"continue",I_NEXT},*/
	/*{"corp",  I_ENDP  },*/
	{"cs",  	I_CS	},
	{"cvar",	I_CVAR  },
	{"cx",  	I_CX	},
	{"d",		I_DWORD },
	{"daadd",	I_DAADD },
	{"dasub",	I_DASUB },
	{"data",	I_DATA  },
	//{"db",	I_DB	},
	//{"dd",	I_DD	},
	{"decl",	I_FORWORD},
	//{"default", I_DEFAULT},
	{"defined", I_DEFINED},
	{"df",  	I_DF	},
	{"dh",  	I_DH	},
	{"di",  	I_DI	},
	{"div", 	I_DIVS  },
	{"dl",  	I_DL	},
	//{"do",	I_DO	},
	{"ds",  	I_DS	},
	{"dw",  	I_DW	},
	{"dword",	I_DWORD },
	{"dx",  	I_DX	},
	{"else",	I_ELSE  },
	{"elsif",	I_ELSIF },
	/*{"eludom",I_ENDMODULE },*/
	//{"end", 	I_ENDP  },
	{"endblock",I_ENDDO },
	//{"endclass",I_ENDCLASS},
	{"enddata", I_ATAD  },
	//{"enddo",   I_ENDDO },
	{"endif",	I_FI	},
	{"endloop", I_ENDLOOP	},
	{"endmodule",I_ENDMODULE },
	/*{"endp",  I_ENDP  },*/
	{"endproc", I_ENDP  },
	{"endstruct",I_ENDSTRUCT},
	{"enter",	I_ENTER },
	{"es",  	I_ES	},
	{"esc", 	I_ESC	},
	{"even",	I_EVEN  },
	{"exit",	I_EXIT  },
	{"extern",	I_EXTERN},
	{"external",I_EXTERN},
	/*{"export",	I_EXPORT},*/
	{"far", 	I_FAR	},
	{"fh",  	I_FH	},
	{"fi",  	I_FI	},
	{"fl",  	I_FL	},
	{"forword", I_FORWORD},
	/*{"fptr",  I_FPTR  },*/
	{"fx",  	I_FX	},
	{"go",  	I_GO	},
	{"gosub",	I_GOSUB },
	{"goto",	I_GOTO  },
	//{"group",   I_GROUP },
	{"hlt", 	I_HLT	},
	{"idiv",	I_IDIVS },
	{"if",  	I_IF	},
	{"iif", 	I_IIF	},
	{"import",  I_IMPORT},
	{"in",  	I_IN	},
	/*{"inherit",	I_IMPORT},*/
	/*{"into",  I_INTO  },*/
	{"intr",	I_INTR  },
	{"ip",  	I_IP	},
	{"iret",	I_IRET  },
	{"jmp", 	I_JMP	},
	//{"label",	I_LABEL },
	{"leave",	I_LEAVE },
	{"load",	I_LOAD  },
	{"local",	I_LOCAL },
	{"lock",	I_LOCK  },
	{"loop",	I_LOOP  },
	{"macro",	I_MACRO },
	{"model",	I_MODEL },
	{"module",  I_MODULE},
	{"near",	I_NEAR  },
	{"neg", 	I_NEGS  },
	{"next",	I_NEXT  },
	{"nop", 	I_NOP	},
	{"not", 	I_NOT	},
	{"nothing", I_NOTHING},
	/*{"od",	I_ENDDO },*/
	{"of",  	I_OF	},
	/*{"off",	I_OFF	},*/
	/*{"on",	I_ON	},*/
	{"org", 	I_ORG	},
	{"orig",	I_ORIG  },
	{"out", 	I_OUT	},
	{"ovf", 	I_OVF	},
	{"page",	I_PAGE  },
	{"para",	I_PARA  },
	//{"part",  I_PART  },
	{"pf",  	I_PF	},
	/*{"pool",  I_ENDLOOP	},*/
	{"pop", 	I_POP	},
	{"popa",	I_POPA  },
	{"port",	I_PORT  },
	{"private", I_PRIVATE},
	{"proc",	I_PROC  },
	//{"ptr",	I_PTR	},
	{"public",  I_PUBLIC},
	{"push",	I_PUSH  },
	{"pusha",	I_PUSHA },
	//{"qword",	I_QWORD },
	{"rcl", 	I_RCLEQ },
	{"rcr", 	I_RCREQ },
	{"rep", 	I_REP	},
	{"repe",	I_REPE  },
	{"repn",	I_REPN  },
	{"ret", 	I_RET	},
	{"retf",	I_RETF  },
	{"return",  I_RETURN},
	{"rol", 	I_ROLEQ },
	{"ror", 	I_ROREQ },
	{"sar", 	I_SARS  },
	{"save",	I_SAVE  },
	{"sbc", 	I_SBCEQ },
	{"seg", 	I_SEG	},
	{"seg_cs",  I_SEG_CS},
	{"seg_ds",  I_SEG_DS},
	{"seg_es",  I_SEG_ES},
	{"seg_ss",  I_SEG_SS},
	{"segment", I_SEGMENT},
	/*{"segptr",	I_SEGPTR},*/
	/*{"select",  I_SWITCH},*/
	{"sf",  	I_SF	},
	{"shl", 	I_SHLS  },
	{"shr", 	I_SHRS  },
	{"si",  	I_SI	},
	/*{"size",  I_SIZEOF  },*/
	{"sizeof",  I_SIZEOF  },
	{"sp",  	I_SP	},
	{"ss",  	I_SS	},
	{"stack",	I_STACK },
	{"start",	I_START },
	//{"static",  I_STATIC},
	{"struct",  I_STRUCT},
	//{"switch",  I_SWITCH},
	//{"tbyte",   I_TBYTE },
	{"tf",  	I_TF	},
	{"to",  	I_TO	},
	{"type",	I_TYPE  },
	{"typeof",  I_TYPEOF},
	{"var", 	I_VAR	},
	{"w",		I_WORD  },
	{"wait",	I_WAIT  },
	{"word",	I_WORD  },
	{"ww",  	I_W2D	},
	{"zf",  	I_ZF	},
	{"",		0		}
};

void
Rsv_List(void)
{
	int i;

	for (i = 1;i < RSVTBLSIZ;i++) {
		printf("%s\n",oRsvTbl[i].name);
	}
}

static int
RsvSrch0(byte *key)
	/*
	 *  key:さがす文字列へのポインタ
	 *  tbl:文字列へのポインタをおさめた配列
	 *  nn:配列のサイズ
	 *  復帰値:見つかった文字列の番号(0より)  みつからなかったとき-1
	 */
{
	int  low,mid,f,nn;

	nn = RSVTBLSIZ;
	low = 0;
	while (low < nn) {
		mid = (low + nn - 1) / 2;
		if ( (f = strcmp(key,oRsvTbl[mid].name)) < 0)
			nn = mid;
		else if (f > 0)
			low = mid + 1;
		else
			return mid;
   }
   return -1;
}

#if 0
void main(void)
{
	int  i;
	int  s;
	static byte *str[] = {
		"test",
		"young",
		"aaa",
		"staticc",
		"zzz",
	};

	printf("RSVTBLSIZ = %d\n",RSVTBLSIZ);
	for (i = 1;i < RSVTBLSIZ;i++) {
		s = RsvSrch0(oRsvTbl[i].name);
		if (s == -1)
			printf("Error! : %s(%d) is not found.\n",oRsvTbl[i].name,i);
		else
			printf("+");
	}
	for (i = 0;i < 5;i++) {
		s = RsvSrch0(str[i]);
		if (s != -1)
			printf("Error! :未登録名%s が登録扱いされた(%d)!\n",str[i],s);
		else
			printf("-");
	}

}

#else

static word
RsvSearch(byte *name)
{
	int  s;

	s = RsvSrch0(name);
	if (s == -1)
		return T_IDENT;
	return Sym_tok = oRsvTbl[s].tok;
}

/*----------------------------------------------------------------*/
#define CH_LINBUF_SIZ 0x2000
static int  oChBak;
static byte *oChLinPtr;
static byte oChLinBuf[CH_LINBUF_SIZ+2];
#define SYM_STR_SIZ 0x2000
static byte oSymStrBuf[SYM_STR_SIZ+20];
#define NOCHR	(-1)

#ifdef MACR
static word oMacN;
#define MACPTR_MAX 9
static byte *oMacPtr[MACPTR_MAX+1];
/*#define MACCHBAK 0*/
#ifdef MACCHBAK
  static int  oMacChBak[MACPTR_MAX+1];
#endif
#endif

#define INCLMAX 20
static word oInclNo;
static word oInclLine[INCLMAX];
static byte *oInclName[INCLMAX];
static FILE *oInclFile[INCLMAX];

void
Ch_Top(void)
{
  #ifdef MACR
	oMacN = 0;
   #ifdef MACCHBAK
	oMacChBak[0] =
   #endif
  #endif
	oChBak = NOCHR;
	oChLinPtr = oChLinBuf;
	oChLinBuf[0] = '\0';
}

static word
ChGetLine(void)
{
 JJJ:
  #ifdef MACR
	oMacN = 0;
   #ifdef MACCHBAK
	oMacChBak[0] = NOCHR;
   #endif
  #endif
	oChLinBuf[0] = '\0';
	oChLinPtr = fgets(oChLinBuf,CH_LINBUF_SIZ,Ch_file);
	if (oChLinPtr == NULL) {
		if (feof(Ch_file)) {
			if (oInclNo == 0) {
				if (Ana_mmodFlg == 0)
					Msg_Err("へんなところでEOFが現れた");
			} else {
				oInclNo--;
				fclose(Ch_file);
				free(Ch_srcName);
				Ch_file = oInclFile[oInclNo];
				Ch_srcName = oInclName[oInclNo];
				Ch_line = oInclLine[oInclNo];
				Ch_Top();
				goto JJJ;
			}
		} else
			Msg_Err("ﾌｧｲﾙ入力ｴﾗｰ");
		Ana_err = ANAERR_EOF;
		return 1;
	}
	++Ch_line;  /* */
	if (Opt_comment) {
		if (Opt_comment == 1)
			fprintf(Out_file,";%4u : ", Ch_line);
		else if (Opt_comment == 2)
			fprintf(Out_file,";\t");
		else
			fprintf(Out_file,"; %s %4u :\t",Ch_srcName, Ch_line);
		fputs(oChLinPtr,Out_file);
	}
	return 0;
}

word
Ch_Init(void)
{
	oChBak = NOCHR;/*'\n'; */
	Ana_err = 0;
	Ch_line = 0;
	if (ChGetLine())
		return 1;
	return 0;
}

#ifdef MACR
static word
ChMacSet (byte_fp mp, byte far* far* argp, word k)
{
	byte_fp p;
	word c;
	if (oMacN >= MACPTR_MAX) {
		Msg_Err("マクロがネストしすぎです");
		goto ERREND;
	}
  #ifdef MACCHBAK
	oMacChBak[oMacN] = oChBak;
	oChBak = NOCHR;
  #endif
	oMacPtr[oMacN++] = oChLinPtr;
	while (*(oChLinPtr++))
		;
	p = oChLinPtr;
	do {
		*p++ = c = *mp++;
		if (p >= oChLinBuf + CH_LINBUF_SIZ)
			goto ERR;
		if (c > 0 && c < 8) {
			byte_fp q;
			q = argp[--c];
			p--;
			do {
				*p++ = c = *q++;
				if (p >= oChLinBuf + CH_LINBUF_SIZ)
					goto ERR;
			} while (c != '\0');
			p--;
			c = 1;
		}
	} while (c != '\0');
	if (k) {
		--p;
		if (k > 0xff) {
			*p++ = k / 0x100;
		}
		*p++ = k % 0x100;
		*p = '\0';
	}
	if (Deb_macFlg)
		printf(";;;マクロ文字列展開\t%s\n",oChLinPtr);
 ENDF:
	return 0;
 ERR:
	oChLinPtr = oMacPtr[0];
   #ifdef MACCHBAK
	oChBak = oMacChBak[0];
   #endif
	Msg_Err("マクロ文字列展開に失敗");
 ERREND:
	return 1;
}
#endif

word
Ch_GetK(void)
{
	word c;
	byte c2;

	if (oChBak != NOCHR) {
		c = oChBak;
		oChBak = NOCHR;
		return c;
	}
	while ((c = *oChLinPtr) == '\0') {
  L1:
	  #ifdef MACR
		if (oMacN == 0) {
			if (ChGetLine())
				return '\0';
		} else {
			oChLinPtr = oMacPtr[--oMacN];
		  #ifdef MACCHBAK
			oChBak = oMacChBak[oMacN];
		  #endif
		}
	  #else
		if (ChGetLine())
			return '\0';
	  #endif
	}
	if (iskanji(c)) {
		if ((c2 = *++oChLinPtr) == '\0')
			goto L1;
		if (iskanji2(c2)) {
			c = c * 0x100 + c2;
		} else {
			Msg_Err("全角の２バイト目がおかしい");
			c = ' ';
		}
	} else if (c == C_LF || c == C_CR) {
		c = *oChLinPtr = '\n';
		/*MSG("｢CR｣");*/
  #if 1
	} else if (c == '\\') {
		if (*(oChLinPtr + 1) == C_LF || *(oChLinPtr + 1) == C_CR) {
			c = ' ';
			*oChLinPtr++ = c;
			*oChLinPtr = c;
		}
  #endif
	} else {
		/*MSGF(("｢%c｣",c));*/
	}
	++oChLinPtr;
	return c;
}

word
Ch_Get(void)
{
	word c;

	c = Ch_GetK();
	if (c > 0xff) {
		Msg_Err("全角文字のおけないところに表せた");
		return c >> 8;
	}
	return c;
}

global void
Ch_Unget(int  c)
{
	oChBak = c;
}

static word
ChGet2(void)
{
	word c,c2;

	if ((c = Ch_GetK()) == '#')
		goto J1;
  #if 0
	if (c == '\\') {
		c2 = Ch_GetK();
		if (c2 == '\n')
			return ' ';
		Ch_Unget(c2);
		return c;
	}
  #endif
	if (c != '/')
		return c;
	c2 = Ch_GetK();
	if (c2 == '*') {
		c = Ch_GetK();
		c2 = Ch_GetK();
		while (c != '*' || c2 != '/') {
			c = c2;
			c2 = Ch_GetK();
			if (c2 == '\0')
				return 0;
		}
		return ' ';

	} else if (c2 == '/') {
 J1:
		do {
			c = Ch_GetK();
		} while (c != '\n' && c!= 0);
		if (c == 0)
			return 0;
		return '\n';

	} else {
		Ch_Unget(c2);
		return '/';
	}
}

word
Ch_SkipSpc(void)
{
	word c;

	do {
		do {
			c = ChGet2();
		} while (c <= 0xff && isSpace(c) && c != '\n');
	} while (c == '\n' && Sym_crMode > 0);

	return c;
}

int
Ch_ChkPeriod(void)
{
	word c;

	if ((c = Ch_GetK()) == '.')
		return 1;
	Ch_Unget(c);
	return 0;
}

byte
Ch_ChkBW(void)
{
	word c;
	byte f;

	f = 0;
	if (Ch_ChkPeriod()) {
		if ((c = Ch_Get()) == 'w')
			f = 1;
		else if (c != 'b')
			Msg_Err(".b .wの指定がおかしい");
	}
	return f;
}

/*---------------------------------------------------------------------------*/

static word
SymYenEsc()
{
	word c,b;

 YEN_ESC1:
	c = Ch_GetK();
	switch(c) {
	case 'e':
	case '[':c = 0x1b;break;
	case 'n':c = '\n';break;
	case 'r':c = '\r';break;
	case 't':c = '\t';break;
	case 'v':c = '\v';break;
	case 'f':c = '\f';break;
	case 'a':c = '\a';break;
	case 'b':c = '\b';break;
	case '0':c = '\0';break;
	case '\\':c = '\\';break;
	case '\'':c = '\'';break;
	case '"':c = '"';break;
	case 'N':c = 0x0d0a;break;
	case 'x':
		c = 0;
		while ((b = Ch_Get(),isXdigit(b)))
			c = c * 16 + toXdigit(b);
		Ch_Unget(b);
		break;
	/*case '\n':
		c = Ch_Get();
		if (c == '\\')
			goto YEN_ESC1;
		break;*/
	default:
  #if 0
		if (c >= '1' && c <= '9') {
			c -= '0';
			while ((b = Ch_Get(), b >= '0' && b <= '9'))
				c = c * 10 + b - '0';
			Ch_Unget(b);
			break;
		}
  #endif
		Msg_Err("'\\'エスケープの指定がおかしい");
	}
	return c;
}

static void
SymGetValue(word c)
{
	dword val;

	val = 0;
	if (c == '0') {
		c = Ch_Get();
		if (c == 'x') {
 XX:
			for (;;) {
				c = Ch_Get();
				if (isDigit(c)) {
					val = val * 0x10 + c - '0';
				} else if ('A' <= c && c <= 'F') {
					val = val * 0x10 + c - 'A' + 10;
				} else if ('a' <= c && c <= 'f') {
					val = val * 0x10 + c - 'a' + 10;
				} else
					break;
			}
		} else if (c == 'b' || c == 'p') {
 BB:
			while ((c = Ch_Get()) == '0' || c == '1')
				val = val * 2 + c - '0';
		} else if (c == 'q') {
 QQ:
			while ((c = Ch_Get()) >= '0' && c < '4')
				val = val * 4 + c - '0';
		} else if (c == 'o') {
 OO:
			while ((c = Ch_Get()) >= '0' && c < '8')
				val = val * 8 + c - '0';
		} else if ('0' <= c && c <= '9') {
			Msg_Err("'0'から始まる10進数がある");
			goto J1;
		}
	} else {
 J1:
		while (isDigit(c)) {
			val = val * 10 + c - '0';
			c = Ch_Get();
		}
	}
 #if 0
	switch (c) {
	case 'p':
	case 'b':
	case 'B':
		goto BB;
	case 'q':
		goto QQ;
	case 'o':
		goto OO;
	case 'x':
		goto XX;
	}
 #endif
	if (c > 0x80 || isCsymF(c))
		Msg_Err("数字に余計な文字がくっついている");
	Ch_Unget(c);
	Sym_val = val;
}


static int
SymGetString(void)
{
	word c;
	byte_fp p;

	p = oSymStrBuf;
	Sym_strLen = 0;
	do {
		while ((c = Ch_GetK()) != '"') {
			if (c == '\\')
				c = SymYenEsc();
			else if (c == '\n') {
				Msg_Err("文字列の途中に改行がある");
			} else if (c == 0) {
				Msg_Err("文字列の途中でEOFになった");
				return -1;
			}
			if (Sym_strLen < SYM_STR_SIZ) {
				if (c > 0xff) {
					Sym_strLen += 2;
					if (Sym_strLen > SYM_STR_SIZ)
						goto E1;
					*(p++) = c >> 8;
					*(p++) = (byte)c;
				} else {
					++Sym_strLen;
					*(p++) = c;
				}
			} else if (Sym_strLen == SYM_STR_SIZ)
  E1:
				Msg_Err("文字列が長すぎる");
		}
		*p = '\0';
		c = Ch_SkipSpc();
	} while (c == '"');
	Ch_Unget(c);
	Sym_str = oSymStrBuf;
	return 0;
}


int
Sym_NameGet(word c)
{
	byte *p;
	int  i;

	if (c == '\0') {
		c = Ch_SkipSpc();
	}
	if (!(c > 0xff || isCsymF(c))) {
		Msg_Err("名前が指定されていない");
		return 1;
	}
	p = Sym_name;
	i = 0;
	for (;;) {
		if (c > 0xff) {
			byte c0;
			if (c < 0x824f)
				Msg_Err("名前に使えない全角文字が使われた");
			c0 = c >> 8;
			c &= 0xff;
			if ((i += 2) < LBL_NAME_SIZ) {
				*(p++) = c0;
				*(p++) = c;
			}
		} else {
			if (isCsym(c) == 0)
				break;
			if (++i < LBL_NAME_SIZ)
				*(p++) = c;
		}
		c = ChGet2();
	}
	*p = '\0';
	Ch_Unget(c);
	if (i >= LBL_NAME_SIZ) {
		Msg_Err("名前が長すぎる");
		return 1;
	}
	return 0;
}


static word oAtMarkFlg;

static word
SymGet0()
{
	word c;
	word t;

	Sym_val = 0;
	oAtMarkFlg = 0;
	*Sym_name = '\0';
	c = Ch_SkipSpc();
	if (c == 0) {
		t = I_EOF;
		goto ENDF;
	}
	if (c > 0xff || isCsymF(c)) {  /* 名前 */
		Sym_NameGet(c);
		t = T_IDENT;
		if (Sym_rsvflg == 0) {
			t = RsvSearch(Sym_name);
			/*printf(":::%s %s Rsv=%d,Mode=%d\n",
				Sym_name,Deb_ChkOp(Sym_tok),Sym_rsvflg,Ana_mode);*/
		}
		Sym_rsvflg = 0;
		goto ENDF;

	} else if (isDigit(c)) {  /* 数字 */
		SymGetValue(c);
		t = T_CNST;
		goto ENDF;

	} else if (c == '\'') {  /* '　*/
		byte c2;

		c = Ch_GetK();
		if (c == '\\')
			c = SymYenEsc();
		c2 = Ch_Get();
		if (c2 != '\'') {
			if (c <= 0xff && c2 <= 0xff && c2 > 0) {
				c = c * 0x100 + c2;
				if (Ch_Get() == '\'')
					goto J_APO;
			}
			Msg_Err("アポストロヒィ(')の指定がおかしい");
		}
 J_APO:
		Sym_val = c;
		t = T_CNST;
		goto ENDF;

	} else if (c == '"') {
		if (SymGetString())
			goto ERR;
		t = T_STRING;
		goto ENDF;

	} else if (c == '\n') {
		t = I_CR;
		goto ENDF;
  #if 1
	} else if (c == ',') {
		Sym_crMode++;
		c = Ch_SkipSpc();
		Sym_crMode--;
		Ch_Unget(c);
		t = I_COMMA;
		goto ENDF;
  #endif
	} else if (c == '.') {
		byte c2;

		c2 = Ch_Get();
		switch (c2) {
		case '.':
			c2 = Ch_Get();
			if (c2 != '.') {
				Ch_Unget(c2);
				t = I_PP;
			} else
				t = I_PPP;
			goto ENDF;

		case '*':
		case '/':
		case '%':
		case '=':
			c = Ch_Get();
			if (c != '.') {
				Ch_Unget(c);
				goto ERR;
			}
			switch (c2) {
			case '*': t = I_IMUL; goto ENDF;
			case '/': t = I_IDIV; goto ENDF;
			case '%': t = I_IMOD; goto ENDF;

			case '=': t = I_SIEQ; goto ENDF;
			}
			goto ERR;
		case '>':
		case '<':
			c = Ch_Get();
			if (c2 == '>' && c == '>') {
				c2 = 0x01;
				c = Ch_Get();
			}
			if (c == '=') {
				if (c2 == 0x01) {
					t = I_SAREQ;
					goto ENDF;
				}
				c2 |= 0x80;
				c = Ch_Get();
			}
			if (c != '.') {
				Ch_Unget(c);
				goto ERR;
			}
			switch(c2) {
			case '>':		t = I_IGT;  goto ENDF;
			case '<':		t = I_ILT;  goto ENDF;
			case '>'|0x80:  t = I_IGE;  goto ENDF;
			case '<'|0x80:  t = I_ILE;  goto ENDF;
			case 0x01:  	t = I_SAR;  goto ENDF;
			}
			Msg_Err("シンボルがおかしい");
			goto ERR;
		case 'c':
		case 's':
		case 'p':
		case 'z':
		case 'o':
			Sym_NameGet(c2);
			if (strcmp(Sym_name,"cf") == 0) {
				t = I_CFCM;
			} else if (strcmp(Sym_name,"sf") == 0) {
				t = I_SFCM;
			} else if (strcmp(Sym_name,"zf") == 0) {
				t = I_ZFCM;
			} else if (strcmp(Sym_name,"pf") == 0) {
				t = I_PFCM;
			/*} else if (strcmp(Sym_name,"af") == 0) {
				t = I_AFCM;*/
			} else if (strcmp(Sym_name,"ovf") == 0) {
				t = I_OVFCM;
			} else {
				goto ERR2;
			}
			c2 = Ch_Get();
			if (c2 != '.')
				goto ERR2;
			goto ENDF;
		}
		Ch_Unget(c2);
		t = I_PERIOD;
		goto ENDF;
	} else if (c == '@') {
			Sym_NameGet(0);
			if (!strcmp(Sym_name,"set"))
				t = M_SET;
			else if (!strcmp(Sym_name,"if"))
				t = M_IF;
			else if (!strcmp(Sym_name,"else"))
				t = M_ELSE;
			else if (!strcmp(Sym_name,"elsif"))
				t = M_ELSIF;
			else if (!strcmp(Sym_name,"fi") || !strcmp(Sym_name,"endif"))
				t = M_ENDIF;
			else if (!strcmp(Sym_name,"part")) {
				t = I_PART;
				goto ENDF;
			} else if (!strcmp(Sym_name,"define")) {
				t = M_DEFINE;
				goto ENDF;
			} else if (!strcmp(Sym_name,"include")) {
				t = M_INCLUDE;
				goto ENDF;
			} else if (!strcmp(Sym_name,"print"))
				t = M_MSG;
		#ifdef DEBUG
			else if (!strcmp(Sym_name,"ChkBgn"))
				(DEBUG_MODE(1),t = I_CR);
			else if (!strcmp(Sym_name,"ChkEnd"))
				(DEBUG_MODE(0),t = I_CR);
		#endif
		#if 1
			else {
				Msg_Err("@命令がおかしい");
				t = I_ERR;
			}
		#endif
			oAtMarkFlg = 1;
			goto ENDF;

	} else if (isPun(c)) {
		t = gSymTbl[c];
		goto ENDF;

	} else if (isPunEx(c)) {
		byte c2;

		c2 = Ch_Get();
		if (c2 == '=' && isPunEq(c)) {
			if (c == '!') {
				t = I_NEQ;
			} else if (c != '<') {
				t = gSymTbl[c]+1;
			} else {
				c2 = Ch_Get();
				if (c2 == '>') {
					t = I_EXG;
				} else {
					Ch_Unget(c2);
					t = I_LE;
				}
			}
			goto ENDF;

		} else if (c2 == c && isPun2(c)) {
			if (c == '+') {
				t = I_INC;
				goto ENDF;
			} else if (c == '-') {
				t = I_DEC;
				goto ENDF;
			} else if (c == '>' || c == '<') {
				c2 = Ch_Get();
				if (c2 == '=') {
					t = (c == '<') ? I_SHLEQ : I_SHREQ;
					goto ENDF;
				} else
					Ch_Unget(c2);
			}
			t = gSymTbl[c]+2;
			goto ENDF;
		}
		Ch_Unget(c2);
		t = gSymTbl[c];
 ENDF:
 #if 0 /* ifdef DEBUG */
		Deb_SymNam(t);
 #endif
		return Sym_tok = t;
	}
 ERR:
	Msg_Err("読み込んだシンボルがおかしい");
 ERR2:
	return Sym_tok = I_ERR;
}

St_t_fp
Sym_NameNew(byte far *name,word t,word f)
{
	St_t_fp sp;
	St_t_fp *p;

	switch (f) {
	case MD_MODULE:
		p = &St_root;
		break;
	case MD_PROC:
		p = &St_localRoot;
		break;
	case MD_EXPO:
		p = &St_expoRoot;
		break;
	case MD_RSV:
		p = &St_rsvRoot;
		break;
	}
	sp = St_Ins(name,p);
	if (sp)
		sp->v.tok = t;
	if (f == MD_EXPO)
		sp->v.flg2 |= FL_EXPO;
	return sp;
}

static word
SymGet1(void)
{
	word u,t;

	t = SymGet0();
	if (Op_Reg1(t)) {
		Sym_reg = t;
		t = T_R1;
		Sym_typ = I_BYTE;
		/*goto ENDF;*/
	} else if (Op_Reg2(t)||Op_Seg2(t)) {
		if (!Ch_ChkPeriod()) {
			Sym_reg = t;
			if (Op_Seg2(t))
				t = T_SEG2;
			else
				t = T_R2;
			Sym_typ = I_WORD;
			goto ENDF;
		}
		u = SymGet0();
		if (!Op_Reg2(u)) {
			if (Op_RegD(t) && Sym_name[1] == '\0') {
				if (*Sym_name == 'h')
					Sym_reg = RegXtoH(t);
				else if (*Sym_name == 'l')
					Sym_reg = RegXtoL(t);

				else
					goto EE;
			} else
				goto EE;
			Sym_typ = I_BYTE;
			t = T_R1;
			/*goto ENDF;*/
		} else if (t == u) {
  EE:
			Msg_Err("３２ビット合成レジスタの指定がおかしい");
			goto ERR;
		} else {
			Sym_reg = t * 0x100 + u;
			Sym_typ = I_DWORD;
			t = (Op_Seg2(t)) ? T_SEG4 : T_R4;
			/*goto ENDF;*/
		}
	} else if (t == T_IDENT) {
		if (Ana_mode == MD_PROC) {
			if ((Sym_sp = St_Search(Sym_name, St_localRoot)) != NULL) {
				t = Sym_sp->v.tok;
				goto ENDF;
			}
		}
		if ((Sym_sp = St_Search(Sym_name, St_root)) != NULL) {
			t = Sym_sp->v.tok;
			if (t == T_MODULE && Ch_ChkPeriod()) {
				St_t_fp sp;
				Sym_NameGet(0);
				if (Sym_sp->v.grp == Mod_sp) {
					if ((sp = St_Search(Sym_name,St_expoRoot)) == NULL)
						if ((sp = St_Search(Sym_name,St_root)) == NULL)
							goto JJJ;
				} else if (Sym_sp->v.grp
					&& (sp = St_Search(Sym_name,Sym_sp->v.grp->v.st)) == NULL){
					/*PRNF(("%s:%lx  %s:%lx  %lx\n",Sym_name,
						Sym_sp->v.grp->v.name,Sym_sp->v.grp,Sym_sp->v.st));*/
  JJJ:
					Msg_Err("モジュール名. に続く名前がおかしい");
					goto ERR;
				}
				Sym_sp = sp;
				t = Sym_tok = sp->v.tok;
	#if 0
				switch (t) {
				case T_PROC:case T_MACPROC:
				case T_PROC_DECL:case T_VAR:case T_CONST:case T_SEG:
				case T_TYPE:case T_STRUCT:case T_INTRPROC:
					break;
				default:
					Msg_PrgErr("importしたラベルがおかしい");
				}
	#endif
			}
		} else if ((Sym_sp = St_Search(Sym_name, St_expoRoot)) != NULL) {
			t = Sym_sp->v.tok;
		} else if ((Sym_sp = St_Search(Sym_name, St_rsvRoot)) != NULL) {
			t = Sym_sp->v.tok;
		}
	}
 ENDF:
 #ifdef DEBUG
		Deb_SymNam(t);
 #endif
	return Sym_tok = t;

 ERR:
	return I_ERR;
}


#if 0
word
Sym_GetSkipCr(void)
{
	Sym_crMode = 1;
	Sym_Get();
	Sym_crMode = 0;
	return Sym_tok;
}
#endif

word
Sym_GetRsvOff(void)
{
	Sym_rsvflg = 1;
	return SymGet0();
}

void
Sym_SkipCR(void)
{
	Sym_crMode = 0;
	while (Ana_err == 0 && Sym_tok != I_CR && Sym_tok != I_SMCLN)
		Sym_Get();
}

static void
Sym_SkipCR0(void)
{
	Sym_crMode = 0;
	while (Ana_err == 0 && Sym_tok != I_CR && Sym_tok != I_SMCLN)
		SymGet1();
}

void
#ifdef DEBUG
_Sym_ChkEol(void)
#else
Sym_ChkEol(void)
#endif
{
	if (Sym_tok != I_CR && Sym_tok != I_SMCLN) {
		Msg_Err("必要な';'か改行がない");
	  #ifdef DEBUG
		PRNF(("%s %d:",Deba_fname,Deba_line));
	  #endif
		Sym_SkipCR();
	}
}

void
#ifdef DEBUG
_Sym_GetChkEol(void)
#else
Sym_GetChkEol(void)
#endif
{
	Sym_Get();
	Sym_ChkEol();
}

static int
Sym_ChkPar(word t,byte c)
{
	static byte msg[] = "必要な' 'がない";

	if (Sym_tok == t)
		return 0;
	msg[7] = c;
	Msg_Err(msg);
	return 1;
}

int
Sym_ChkLB(void)
{
	return Sym_ChkPar(I_LB,'[');
}

int
Sym_ChkRB(void)
{
	return Sym_ChkPar(I_RB,']');
}

int
Sym_ChkLP(void)
{
	PAR_CR_INC();
	return Sym_ChkPar(I_LP,'(');
}

int
Sym_ChkRP(void)
{
	PAR_CR_DEC();
	return Sym_ChkPar(I_RP,')');
}

int
Sym_GetChkLP(void)
{
	if (Sym_Get() == I_LP) {
		PAR_CR_INC();
		Sym_Get();
		return 1;
	}
	return 0;
}

#if 0
int
Sym_ChkLC(void)
{
	return (Sym_ChkPar(I_LC,'{') ? 1 : (Sym_crMode++,0));
}

int
Sym_ChkRC(void)
{
	return (Sym_ChkPar(I_RC,'}') ? 1 : (Sym_crMode--,0));
}
#endif


#ifdef MACR
void
Sym_MacDef(void)
{
	int  i,n;
	byte wd[7][LBL_NAME_SIZ];
	St_t_fp sp;
	word c;

	if (SymGet1() != T_IDENT)
		goto ERR;
	if ((sp = Sym_NameNew(Sym_name,T_MACROSTR,Ana_mode)) == NULL)
		goto ERR;
	MSGF((";;;;define %s\n",sp->v.name));
	n = 0;
	c = Ch_SkipSpc();
	if (c == '(') {
		PAR_CR_INC();
		c = Ch_SkipSpc();
		if (c != ')') {
			while (n < 7) {
				if (c > 0xff || isCsymF(c)) {
					Sym_NameGet(c);
					strcpy(wd[n++],Sym_name);
					MSGF((";;;;\tARG(%d) = %s\n",n-1,wd[n-1]));
					c = Ch_SkipSpc();
					if (c != ',')
						break;
					c = Ch_SkipSpc();
				} else
					goto ERR;
			}
		}
		PAR_CR_DEC();
		if (c != ')')
			goto ERR;
		c = Ch_SkipSpc();
	}
	sp->m.macbufp = Mac_Pos();
	sp->m.macArgCnt = n;
	while (c != '\n' && c != '\0') {
		if (c > 0xff || isCsymF(c)) {
			Sym_NameGet(c);
			for (i = 0; i < n; i++) {
				if (strcmp(wd[i],Sym_name) == 0) {
					Mac_Putc(i + 1);
					goto J2;
				}
			}
			{
				byte_fp p;
				p = Sym_name;
				while (*p)
					Mac_Putc(*p++);
			}
		} else if (c == ' '|| c == '\t') {
			Mac_Putc(' ');
			c = Ch_SkipSpc();
			Ch_Unget(c);
		} else if (c == '"' || c == '\'') {
			word k;
			Mac_Putc(c);
			k = c;
			do {
				c = Ch_GetK();
				if (c == '\n')
					goto ERR;
				Mac_Putc(c);
				if (c == '\\') {
					c = Ch_GetK();
					Mac_Putc(c);
				}
			} while (c != k && c);
		} else {
			Mac_Putc(c);
		}
  J2:;
		c = Ch_GetK();
	}
	Mac_Putc('\0');
 ENDF:
	return;
 ERR:
	Msg_Err("マクロ定義がおかしい");
	Sym_SkipCR();
}

#define MACARGMAX	80
static word oMacArgSiz;

static byte_fp
SymMacPutC(byte_fp p, word c)
{
	if (oMacArgSiz <= MACARGMAX) {
		if (c >= 0xff) {
			*p++ = c / 0x100;
			*p++ = c % 0x100;
			oMacArgSiz += 2;
		} else {
			*p++ = c;
			oMacArgSiz++;
		}
		*p = '\0';
	} else if (oMacArgSiz > MACARGMAX)
		Msg_Err("マクロの引数が長する");
	return p;
}

static word
SymMacFuncArg(byte_fp p,word c)
{
	word k;
	word parflg;

	parflg = 0;
	oMacArgSiz = 0;
	*p = '\0';
	for (;;) {
		switch (c) {
		case ' ':
		case '\t':
			c = Ch_SkipSpc();
			Ch_Unget(c);
			c = ' ';
			break;
		case '\n':
			if (parflg == 0)
				goto EX;
		case '\0':
			goto ERR;
		case '(':
		case '[':
		case '{':
			parflg++;
			break;
		case ')':
			if (parflg == 0)
				goto EX;
		case '}':
		case ']':
			if (parflg > 0)
				parflg--;
			else
				goto ERR;
			break;
		case ',':
			if (parflg == 0)
				goto EX;
		case '\'':
		case '"':
			p = SymMacPutC(p,c);
			k = c;
			for (;;) {
				c = Ch_GetK();
				if (c == 0 || c== '\n')
					goto ERR;
				else if (c == '\\') {
					p = SymMacPutC(p,c);
					c = Ch_GetK();
				} else if (c == k)
					break;
				p = SymMacPutC(p,c);
			}
		}
		p = SymMacPutC(p,c);
		c = Ch_GetK();
	}
 EX:
	if (oMacArgSiz > MACARGMAX)
		goto ERR;
	return c;
 ERR:
	return 0xffff;
}


static void
SymMacFunc(void)
{
	word c,n,k;
	St_t_fp sp;
	byte_fp bufp[7];
	byte buf[7][MACARGMAX+2];

	memset(buf,'\0',7*(MACARGMAX+2));
	for (n = 0;n < 7;n++)
		bufp[n] = buf[n];
	sp = Sym_sp;
	c = Ch_SkipSpc();
	k = '\n';
	if (c == '(') {
		PAR_CR_INC();
		c = Ch_SkipSpc();
		k = ')';
	}
	n = 0;
	if (sp->m.macArgCnt != 0) {
		while (n < sp->m.macArgCnt && n < 7) {
			c = SymMacFuncArg(bufp[n],c);
			n++;
			if (c != ',')
				break;
			c = Ch_SkipSpc();
		}
	}
	/*printf("spn %d/%d kc %d/%d\n",sp->m.macArgCnt,n,k,c);*/
	if (k == ')' && c == k) {
		PAR_CR_DEC();
		c = 0;
	}
	if (n != sp->m.macArgCnt)
		goto ERR;
	ChMacSet(sp->m.macbufp,bufp,c);
	return;
 ERR:
	Msg_Err("引数の数が定義と一致しない");
	Sym_SkipCR0();
	return;
}

word
SymGetM(void)
{
	while (SymGet1() == T_MACROSTR)
		SymMacFunc();
	return Sym_tok;
}
#else

#define SymGetM SymGet1

#endif


static void
SymAtSet(void)
{
	St_t_fp sp;
	word bcr;

	MSG("@set decl");
	bcr = Sym_crMode;
	Sym_crMode = 0;
	SymGet0();
	if (Sym_tok == T_CONST && Sym_sp->c.l_flg) {
		sp = Sym_sp;
	} else {
		if (Sym_tok != T_IDENT)
			goto ERR;
		if ((sp = Sym_NameNew(Sym_name,T_CONST,Ana_mode)) == NULL)
			goto ENDF;
		sp->c.l_flg = 1;
	}
	if (SymGetM() == I_EQU) {
		SymGetM();
		sp->c.val = Expr_Cnstnt();
		Sym_ChkEol();
		if (sp->c.l_flg == 2) {
			Opt_cpu = sp->c.val;
			if (Opt_cpu && Opt_r86 == 0)
				fprintf(Out_file,"\t.%c86\n",'0'+Opt_cpu);
		}
	} else {
 ERR:
		Msg_Err("@setがおかしい");
	}
 ENDF:
	Sym_crMode = bcr;
	return;
}

word
Sym_Get(void)
{
	static byte stk[50];
	static int  sn = 0;
	word t,p;
	long val;
	word sav_crMode;

	t = SymGetM();
	if (oAtMarkFlg == 0)
		return t;
	sav_crMode = Sym_crMode;
	while (Ana_err == 0) {
		Sym_crMode = 0;
		switch (t) {
		case M_SET:
			SymAtSet();
			break;
		case M_IF:
		case M_ELSIF:
			SymGetM();
			val = Expr_Cnstnt();
			Sym_ChkEol();
			if (t == M_ELSIF)
				goto J1;
			if (sn >= 50 - 1) {
				Msg_Err("@if のネストが深すぎる");
				/* Ana_err = ANAERR_MIF;*/
				return I_ERR;
			}
			stk[++sn] = (val) ? 1 : 2;
			break;
		case M_ELSE:
			val = 1;
			Sym_GetChkEol();
 J1:
			if (stk[sn] == 2)
				stk[sn] = (val) ? 1 : 2;
			else if (stk[sn] == 0)
				Msg_Err("@else か @elsif があまっている");
			else
				stk[sn] = 3;
			break;
		case M_ENDIF:
			Sym_GetChkEol();
			stk[sn] = 0;
			if (sn == 0)
				Msg_Err("@endif があまってる");
			else
				--sn;
			break;
  #if 0
		case M_DEFINE:
			Sym_MacDef();
			break;
  #endif
		case M_MSG:
			while (SymGetM() != I_CR && Sym_tok != I_SMCLN) {
				if (Sym_tok == T_STRING) {
					if (stk[sn] < 2)
						fprintf(Err_File,"%s",Sym_str);
					SymGetM();
				} else {
					if (stk[sn] < 2)
						fprintf(Err_File,"%ld",Expr_Cnstnt());
					else
						Expr_Cnstnt();
				}
				if (Sym_tok != I_COMMA)
					break;
			}
			Sym_ChkEol();
			if (stk[sn] < 2)
				fprintf(Err_File,"\n");
		case I_CR:
			break;
		default:
			if (stk[sn] < 2)
				goto LOOPOUT;
			p = 0;
			do {
				t = SymGet0();
				switch (t) {
				case M_IF:
					p++;
					break;
				case M_ENDIF:
					if (p > 0) {
						p--;
						break;
					}
				case M_ELSIF:
				case M_ELSE:
					if (p == 0)
						goto JJJ;
					break;
				case M_DEFINE:
					Sym_SkipCR0();
					break;
				}
			} while (Ana_err == 0);
  JJJ:
			continue;
		}
		t = SymGetM();
	}
 LOOPOUT:
	Sym_crMode = sav_crMode;
	return t;
}


word
Sym_GetRsvOff2(void)
{
	Sym_rsvflg = 1;
	return Sym_Get();
}

void
Sym_MacInclude(void)
{
	int  i;
	word k,c;
	static byte fnam[100];

	i = 0;
	if ((k = Ch_SkipSpc()) == '<') {
		k = '>';
		if (Ana_incDir)
			i = sprintf(fnam,"%s",Ana_incDir);
		goto J1;
	} else if (k == '"') {
 J1:
		while ((c = Ch_GetK()) != k && i < 98) {
			if (c == '/')
				c = '\\';
			if (c > 0xff) {
				fnam[i++] = c / 0x100;
				fnam[i++] = (byte)c;
			} else if (c >= 0x20 && c <= 0x7e||c >= 0xa0 && c <= 0xef) {
				fnam[i++] = c;
			} else {
				goto ERR;
			}
		}
		fnam[i] = '\0';
	} else
		 goto ERR;
	Sym_GetChkEol();

	if (oInclNo < INCLMAX) {
		oInclName[oInclNo] = Ch_srcName;
		oInclLine[oInclNo] = Ch_line;
		oInclFile[oInclNo] = Ch_file;
		oInclNo++;
		Ch_srcName = strdup(fnam);
		Ch_line 	= 0;
		Ch_file 	= fopen_e(fnam,"r");
	} else {
		Msg_Err("@includeがネストしすぎる");
	}
	return;
 ERR:
	Msg_Err("@include の指定がおかしい");
	return;
}
#endif
