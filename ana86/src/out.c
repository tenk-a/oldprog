#include <string.h>
#include "anadef.h"

/*---------------------------------------------------------------------------*/

global void
Msg_Err_(
	byte_fp msg
  #ifdef DEBUG
	, byte *dbgsrcfile
	, word dbgerrline
  #endif
)
{
	/*fprintf(Err_File,"%s %d : %s\n", Ch_srcName, Ch_line, msg);*/
	fprintf(Err_File,"%s %d :", Ch_srcName, Ch_line);
	fprintf(Err_File,"%s\n", msg);
  #ifdef DEBUG
	fprintf(Err_File,"\t%s %d : ana src debug\n", dbgsrcfile, dbgerrline);
  #endif
	Err_cnt++;
}


global void
Msg_PrgErr_(
	byte_fp msg
  #ifdef DEBUG
	, byte *dbgsrcfile
	, word dbgerrline
  #endif
)
{
	/*fprintf(Err_File,"%s %d : %s\n", Ch_srcName, Ch_line, msg);*/
	fprintf(Err_File,"%s %d :", Ch_srcName, Ch_line);
	fprintf(Err_File,"anaŽ©g‚Ì’Ž/%s\n", msg);
  #ifdef DEBUG
	fprintf(Err_File,"\t%s %d : ana src debug\n", dbgsrcfile, dbgerrline);
  #endif
	Err_cnt++;
}


/*---------------------------------------------------------------------------*/


int
IsEp_R1op(Et_t_fp	ep, word n)
{
	return (IsEp_Op(ep,T_R1) && ep->c.reg == n);
}

int
IsEp_R2op(Et_t_fp	ep, word n)
{
	return (IsEp_Op(ep,T_R2) && ep->c.reg == n);
}

int
IsEp_R4op(Et_t_fp	ep, word n)
{
	return (IsEp_Op(ep,T_R4) && ep->c.reg == n);
}

int
IsEp_Reg12(Et_t_fp ep)
{
	return (IsEp_Op(ep,T_R2) || IsEp_Op(ep,T_R1));
}

int
IsEp_CnstVal(Et_t_fp ep, long	n)
{
	return  (IsEp_Op(ep,T_CNST) && ep->c.val == n);
}

int
IsEp_W2DCnst(Et_t_fp ep)
{
	if (IsEp_Op(ep,I_W2D)) {
		switch (ep->e.lep->e.op) {
		case T_CNST:
		case T_CNSTEXPR:
		case T_OFS:
			switch (ep->e.rep->e.op) {
			case T_CNST:
			case T_CNSTEXPR:
			case T_OFS:
				return 1;
			}
		}
	}
	return 0;
}

int
IsEp_Cnsts(Et_t_fp ep)
{
	return (IsEp_Op(ep,T_CNST) || IsEp_Op(ep,T_CNSTEXPR) || IsEp_Op(ep,T_OFS));
}

int
IsEp_Lft1(Et_t_fp	ep)
{
	return (IsEp_Op(ep,T_R1) || IsEp_Op(ep,T_M1));
}

int
IsEp_Lft2(Et_t_fp	ep)
{
	return (IsEp_Op(ep,T_R2) || IsEp_Op(ep,T_M2));
}

int
IsEp_Lft4(Et_t_fp	ep)
{

	return (IsEp_Op(ep,T_R4) || IsEp_Op(ep,T_M4));
}

int
IsEp_Lft4S(Et_t_fp ep)
{
	return (IsEp_Op(ep,T_R4) || IsEp_Op(ep,T_M4) || IsEp_Op(ep,T_SEG4));
}

int
IsEp_Lft12(Et_t_fp ep)
{
	return (IsEp_Op(ep,T_R2)||IsEp_Op(ep,T_M2)
		||  IsEp_Op(ep,T_R1)||IsEp_Op(ep,T_M1));
}

int
IsEp_Lft124(Et_t_fp ep)
{
	if (ep == NULL)
		return 0;
	return (IsEp_Op(ep,T_R2)||IsEp_Op(ep,T_M2)
		||  IsEp_Op(ep,T_R1)||IsEp_Op(ep,T_M1)
		||  IsEp_Op(ep,T_R4)||IsEp_Op(ep,T_M4));
}

int
IsEp_Rht1(Et_t_fp ep)
{
	if (IsEp_Op(ep,T_CNST)) {
		if (ep->c.val < -128 || ep->c.val > 0xff)
			return 0;
		return 1;
	}
	return (IsEp_Op(ep,T_CNSTEXPR) || IsEp_Lft1(ep));
}

int
IsEp_Rht12(Et_t_fp ep)
{
	if (IsEp_Op(ep,T_CNST)) {
		if (ep->c.val < -0x10000L || ep->c.val > 0xffffL)
			return 0;
		return 1;
	}
	return (IsEp_Op(ep,T_CNSTEXPR) || IsEp_Lft12(ep)
		|| IsEp_Op(ep,T_OFS) || IsEp_Op(ep,T_ADDR)  );
}

int
IsEp_Rht4(Et_t_fp ep)
{
	return (IsEp_Cnsts(ep) || IsEp_Reg4(ep)/* || IsEp_Seg4(ep)*/
		|| IsEp_Mem4(ep)|| IsEp_Op(ep,I_W2D));
}

int
IsEp_Mem124(Et_t_fp ep)
{
	return (IsEp_Op(ep,T_M2) || IsEp_Op(ep,T_M1) || IsEp_Op(ep,T_M4));
}

int
IsEp_MemMem(Et_t_fp lp, Et_t_fp rp)
{
	switch (lp->e.op) {
	case T_M1:
	case T_M2:
	case T_M4:
		switch (rp->e.op) {
		case T_M1:
		case T_M2:
		case T_M4:
			Msg_Err("ƒƒ‚ƒŠ“¯Žm‚Ì‰‰ŽZ‚Í‚Å‚«‚È‚¢");
			return 1;
		}
	}
	return 0;
}

#if 0
int
IsEp_Dual(Et_t_fp	ep)
{
	switch (ep->e.op) {
	case I_INC: 	case I_DEC: 	case I_COMS:	case I_NEGS:
		return 1;
	case I_ADCEQ:	case I_SBCEQ:	case I_ADDEQ:	case I_SUBEQ:
	case I_ANDEQ:	case I_OREQ:	case I_EOREQ:
	case I_SUB: 	case I_AND: 	case I_EXG:
		return 2;
	case I_SHLEQ:	case I_SHREQ:	case I_SAREQ:	case I_ROREQ:
	case I_ROLEQ:	case I_RCREQ:	case I_RCLEQ:
		return 3;
	case I_EQU: 	case I_SIEQ:	case I_DIVS:	case I_IDIVS:
		return 4;
	}
	return 0;
}
#endif

int
IsEp_CnstTyp(word t, Et_t_fp ep)
{
	if (IsEp_Cnst(ep)) {
		if (t == I_BYTE) {
			if (ep->c.val <-128 || ep->c.val > 255) {
				Msg_Err("’è”’l‚ª1ÊÞ²Ä‚Ì”ÍˆÍ‚ð‰z‚¦‚Ä‚¢‚é");
				return -1;
			}
		} else if (t == I_WORD) {
			if (ep->c.val < -(long)(0x8000) || ep->c.val > 0xffff) {
				Msg_Err("’è”’l‚ª2ÊÞ²Ä‚Ì”ÍˆÍ‚ð‰z‚¦‚Ä‚¢‚é");
				return 1;
			}
		}
	} else if (IsEp_Cnsts(ep)) {
		;
	} else {
 #if 1
		if (t == I_BYTE) {
			if (!IsEp_Lft1(ep))
				Msg_Err("Œ^‚ª‰ï‚í‚È‚¢");
		} else if (t == I_WORD) {
			if (IsEp_Lft4(ep)||IsEp_Seg4(ep))
				Msg_Err("Œ^‚ª‚ ‚í‚È‚¢");
		}
 #endif
		/* Msg_Err("’è”‚Å‚È‚¢‚à‚Ì‚ªŽw’è‚³‚ê‚½"); */
		return 1;
	}
	return 0;
}

/*------------------------------------------------------------------*/

byte_fp
Str_StName(St_t_fp sp)
{
	byte far *str;
	static byte buf[LBL_NAME_SIZ*2+4];

	if ((sp->v.tok == T_VAR) && sp->v.ofs > 0) {
		str = GoLbl_Strs(sp->v.ofs);
	} else if (sp->v.tok == T_LBL && sp->v.ofs > 0) {
		str = GoLbl_Strs(sp->v.ofs);
	} else if (sp->v.grp && sp->v.tok != T_MODULE) {
		sprintf(buf,"%s%s%s%s%s",
			(sp->v.grp->g.ccflg == 1) ? "_" : "",
			sp->v.grp->g.modname,
			sp->v.grp->g.sep,
			sp->v.name,
			(sp->v.grp->g.ccflg == 2) ? "_" : "");
		if (sp->v.grp != Mod_sp || Opt_partFlg) {
			switch (sp->v.tok) {
			case T_PROC:
				/*if (Opt_partFlg == 0)
					break;*/
			case T_PROC_DECL:
			case T_VAR:
				sp->v.flg2 |= FL_USE;
			}
		}
		str = buf;
	} else {
		str = sp->v.name;
	}
	return str;
}

static byte *
StrTyp(word op)
{
	switch (op) {
	case I_BYTE:	return "byte";
	case I_WORD:	return "word";
	case I_DWORD:	return "dword";
 /*
	case I_QWORD:	return "qword";
	case I_TBYTE:	return "tbyte";
 */
	}
	return "BYTE";
}

void
Out_LblTyp(St_t_fp sp, int  typ)
{
	byte far *str;

	str = Str_StName(sp);
	if (Opt_r86) {
		if (typ == -2/*endp*/)
			return;
		fprintf(Out_file,"%s:",str);
		if (sp->v.flg2 & 0x01)
			fputc(':',Out_file);
	} else {
		if (((sp->v.flg2 & FL_EXPO) || Opt_namePub) && typ != -2)
			fprintf(Out_file,"\tpublic\t%s\n",str);
		if (typ) {
			if (typ == 1 || typ == 2 || typ == 4) {
				fprintf(Out_file,"%s\tlabel\t%s\n",str,StrTyp(typ));
			} else if (typ == -1) {
				if (Opt_procFlg) {
					fprintf(Out_file,"%s\tproc\t%s\n",
						str,(sp->v.flg2 & FL_FAR) ? "far" : "near");
				} else {
					fprintf(Out_file,"%s\tlabel\t%s\n",
						str,(sp->v.flg2 & FL_FAR) ? "far" : "near");
				}
			} else if (typ == -2) {
				if (Opt_procFlg) {
					fprintf(Out_file,"%s\tendp\n",str);
				}
			} else {
				fprintf(Out_file,"%s\tlabel\tbyte\n",str);
			}
		} else {
			fprintf(Out_file,"%s:",str);
		}
	}
}

global void
Out_golbl(word n) /*$$*/
{
	fprintf(Out_file,"L$%d:\n",n);
	return;
}

global void
Out_LblStr(byte_fp p)
{
	fprintf(Out_file,"%s:\n",p);
	return;
}

global void
Out_Nm(byte_fp p)
{
	fprintf(Out_file,"\t%s\n",p);
}

global void
Out_NmSt(byte_fp p,byte_fp q)
{
	fprintf(Out_file,"\t%s\t%s\n",p,q);
}

global void
Out_NmStSt(byte_fp p,byte_fp q,byte_fp r)
{
	fprintf(Out_file,"\t%s\t%s,%s\n",p,q,r);
}

global void
Out_NmStStSt(byte_fp p,byte_fp q,byte_fp r,byte_fp s)
{
	fprintf(Out_file,"\t%s\t%s,%s,%s\n",p,q,r,s);
}

global void
Out_Line(byte_fp l)
{
	fprintf(Out_file,"%s\n",l);
}


void
Out_Dup(long t,word n) /* t ‚Í 0 or -1 */
{
	if (Decl_defFlg == 0 || n == 0)
		return;
	if (Opt_r86) {
		fprintf(Out_file,"\trs\t%u\n", n);
	} else if (t < 0) {
		fprintf(Out_file,"\tdb\t%u dup(?)\n", n);
	} else {
		fprintf(Out_file,"\tdb\t%u dup(%d)\n", n,t);
	}
}

/*------------------------------------------------------------------------*/
static byte oOffsetFlg;

static byte oStrReg[][3] = {
	"ah","ch","dh","bh","al","cl","dl","bl",
	"ax","cx","dx","bx","di","si","bp",
	"sp","ip","cs","ds","es","ss"
};
#define StrReg(n)	oStrReg[(n) - I_AH]

static byte *
StrSegGrp(byte *s,word seg)
{
	if (seg == I_CS) {
		s = stpcpy(s,Mod_grps[0]);
	} else if (seg == 0) {
		s = stpcpy(s,Mod_grps[1]);
	}
	return s;
}

static byte *
StrAddVal(byte *s, int  val)
{
	int l;

	l = 0;
	if (val < 0)
		l = sprintf(s,"-%d",-val);
	else if (val > 0)
		l = sprintf(s,"+%d",val);
	return s + l;
}

static byte *
StrAddOfs(byte *s, int  ofs)
{
	//if (ofs < 0)
	//	Msg_PrgErr("µÌ¾¯Ä‚ª•‰‚É‚È‚Á‚Ä‚¢‚é");
	return StrAddVal (s, ofs);
}

static byte *
StrVal(byte *s, long val)
{
	int  l;

	if (val < 0)
		l = sprintf(s, "(%ld)", val);
	else
		l = sprintf(s, "0%lXh", val);
	return s + l;
}

static byte *
StrExpr(byte *s, Et_t_fp xp)
{
	static byte r86typ[][3] = {".b",".w","",".d"};
	byte f;
	Et_t_fp ep;
	St_t_fp sp;

	EXPMSG("StrExpr",xp);
	*s = '\0';
	if (xp == NULL)
		return NULL;
	if (xp->e.op == T_CNSTEXPR) {
		xp = xp->e.lep;
	}
	switch (xp->e.op) {
	case T_R1:
	case T_R2:
	case T_SEG2:
		return stpcpy(s, StrReg(xp->c.reg));

	case T_CNST:
		return StrVal(s, xp->c.val);
 /*
	case T_EXTCNST:
		return stpcpy(s, Str_StName(xp->v.st));
 */
	case T_OFS:
		if (xp->v.st == NULL) {
			if (xp->m.seg == 0x7f)	// $
				return stpcpy(s,"$");
			if (xp->m.seg != I_SI)
				return StrSegGrp(s,xp->m.seg);
		}
		if (Opt_r86 == 0 && oOffsetFlg == 0 && xp->m.seg != 0xff) {
			s = stpcpy(s,"offset ");
			oOffsetFlg = 1;

			if (xp->m.seg == I_CS || xp->m.seg == 0) {
				s = StrSegGrp(s,xp->m.seg);
				*(s++) = ':';
			} else if (xp->m.seg == I_SI) {
				s = StrSegGrp(s,0);
				*(s++) = ':';
			}
		}
		if (xp->v.st == NULL && xp->m.seg == I_SI) {//•¶Žš—ñ
			s = stpcpy(s,GoLbl_Strs(xp->m.siz));
			//printf("Str (%Fp) %d\n",xp,xp->m.siz);
		} else  {
			s = stpcpy(s, Str_StName(xp->v.st));
			s = StrAddOfs(s, xp->m.ofs);
		}
		return s;

	case I_NEG:
		*(s++) = '-';
		*s = '\0';
		goto J1;

	case I_COM:
		s = stpcpy(s,"NOT ");
 J1:
		*(s++) = '(';
		s = StrExpr(s,xp->e.lep);
		if (s == NULL)
			return NULL;
		*(s++) = ')';
		*(s) = '\0';
		return s;

	case T_M1:
	case T_M2:
	case T_M4:
		if (!Opt_r86 && xp->m.typ) {
			s = stpcpy(s,StrTyp(xp->m.typ));
			s = stpcpy(s," ptr ");
			*s = '\0';
		}
		if (xp->m.seg) {
			s = stpcpy(s,StrReg(xp->m.seg));
			*(s++) = ':';
		}
		ep = xp->e.lep;
		*s++ = '[';
		oOffsetFlg = 1;
		switch (ep->e.op) {
		case T_LOCAL:
			s = stpcpy(s,"bp");
			s = StrAddVal(s,xp->m.ofs);
			break;
		case T_VAR:
			sp = ep->v.st;
			s = stpcpy(s, Str_StName(sp));
			s = StrAddOfs(s, xp->m.ofs);
			break;
		case T_CNST:
			s = StrVal(s, xp->m.ofs + ep->c.val);
			break;
		default:
			if ((s = StrExpr(s,ep)) == NULL)
				return NULL;
			s = StrAddOfs(s,xp->m.ofs);
		}
		*s++ = ']';
		if (Opt_r86) {
			int a;
			a = xp->m.typ;
			if (a <= 0 || a > 4)
				a = 3;
			s = stpcpy(s, r86typ[a - 1]);
		}
		*s = '\0';
		return s;
	}
	if ((xp->e.op == I_ADD || xp->e.op == I_SUB)
		&& (IsEp_Reg2(xp->e.lep) || IsEp_Cnsts(xp->e.lep)
			|| IsEp_Reg2(xp->e.rep) || IsEp_Cnsts(xp->e.rep)) )
		f = 0;
	else
		f = 1;
	if (f)
		*s++ = '(';
	if ((s = StrExpr(s,xp->e.lep))  == NULL)
		return NULL;
	switch (xp->e.op) {
	case I_ADD: *(s++) = '+';	break;
	case I_SUB: *(s++) = '-';	break;
	case I_MUL: *(s++) = '*';	break;
	case I_DIV: *(s++) = '/';	break;
	case I_MOD: s = stpcpy(s," MOD ");  break;
	case I_AND: s = stpcpy(s," AND ");  break;
	case I_OR:  s = stpcpy(s," OR ");	break;
	case I_XOR: s = stpcpy(s," XOR ");  break;
	case I_SHL: s = stpcpy(s," SHL ");  break;
	case I_SHR: s = stpcpy(s," SHR ");  break;
	case I_EQEQ:s = stpcpy(s," EQ ");	break;
	case I_NEQ: s = stpcpy(s," NE ");	break;
	case I_GT:  s = stpcpy(s," GT ");	break;
	case I_GE:  s = stpcpy(s," GE ");	break;
	case I_LT:  s = stpcpy(s," LT ");	break;
	case I_LE:  s = stpcpy(s," LE ");	break;
	default:
		Msg_Err("StrExpr()‚Ìˆø‚«”‚ª‚¨‚©‚µ‚¢");
		return NULL;
	}
	if ((s = StrExpr(s,xp->e.rep)) ==	NULL)
		return NULL;
	if (f)
		*(s++) = ')';
	*(s) = '\0';
	return s;
}

global void
Out_Nem0(word t)
{
	byte *nem[] = {
		"aaa","aad","aam","aas","daa","das","db\t0C9h\t;leave","lock",
		"hlt","nop","into","iret","db\t61h\t;popa","db\t60h\t;pusha",
		"db\t26h\t;seg es","db\t2Eh\t;seg ds",
		"db\t36h\t;seg ss","db\t3Eh\t;seg cs",
		"wait",
		"clc","stc","cli","sti","cld","std","cmc",
		"xlat","cbw","cwd","lahf","sahf","pushf","popf"
	};
	if (t < I_AAADD || t > I_POPF) {
		Msg_Err ("Out_Nem0()‚Ìˆø”‚ª‚¨‚©‚µ‚¢");
	} else
		Out_Nm(nem[t - I_AAADD]);
	return;
}

static byte oBuf[1000],oBuf2[1000];

static void
OutRetf(Et_t_fp lp)
{
	oOffsetFlg = 0;
	if (lp) {
		if (StrExpr(oBuf,lp)) {
			fprintf(Out_file,"\tdb\t0CAh\t;retf Imm\n");
			fprintf(Out_file,"\tdw\t%s\t\n",oBuf);
		}
	} else {
		fprintf(Out_file,"\tdb\t0CBh\t;retf\n");
	}
}

global void
Out_Nem1(word xo, Et_t_fp lp)
{
	byte_fp s;

	switch (xo) {
	case I_JMPS:	s = gJmps[Opt_r86]; break;
	case I_JMP: 	s = "jmp";  break;
	case I_JMPF:	s = gJmpf[Opt_r86]; break;
	case I_CALL:	s = "call"; break;
	case I_CALLF:	s = gCallf[Opt_r86];break;
	case I_INC: 	s = "inc";  break;
	case I_DEC: 	s = "dec";  break;
	case I_NEGS:	s = "neg";  break;
	case I_COMS:	s = "not";  break;
	case I_PUSH:	s = "push"; break;
	case I_POP: 	s = "pop";  break;
	case I_MUL: 	s = "mul";  break;
	case I_IMUL:	s = "imul"; break;
	case I_DIVS:	s = "div";  break;
	case I_IDIVS:	s = "idiv"; break;
	case I_INTR:	s = "int";  break;
	case I_BYTE:	s = "db";	break;
	case I_WORD:	s = "dw";	break;
	case I_DWORD:	s = "dd";	break;
	case I_RET: 	s = "ret";  break;
	case I_RETF:	OutRetf(lp);return;
	default:
		Msg_Err("Out_Nem1() ‚Ìˆø‚«”‚ª‚¨‚©‚µ‚¢");
		return;
	}
	oOffsetFlg = 0;
	if (lp) {
		if (StrExpr(oBuf,lp))
			Out_NmSt(s,oBuf);
	} else {
		Out_Nm(s);
	}
	return;
}


global void
Out_Nem2(word xo, Et_t_fp lp,Et_t_fp rp)
{
	byte *s;

	switch (xo) {
	case I_EQU: 	s = "mov";  break;
	case I_ADDEQ:	s = "add";  break;
	case I_SUBEQ:	s = "sub";  break;
	case I_ANDEQ:	s = "and";  break;
	case I_EOREQ:	s = "xor";  break;
	case I_OREQ:	s = "or";	break;
	case I_SUB: 	s = "cmp";  break;
	case I_SHREQ:	s = "shr";  break;
	case I_SAREQ:	s = "sar";  break;
	case I_SHLEQ:	s = "shl";  break;
	case I_ROLEQ:	s = "rol";  break;
	case I_ROREQ:	s = "ror";  break;
	case I_RCLEQ:	s = "rcl";  break;
	case I_RCREQ:	s = "rcr";  break;
	case I_LEA: 	s = "lea";  break;
	case I_AND: 	s = "test"; break;
	case I_EXG: 	s = "xchg"; break;
	case I_ADCEQ:	s = "adc";  break;
	case I_SBCEQ:	s = "sbb";  break;
	case I_OUTP:	s = "out";  break;
	case I_INP: 	s = "in";	break;
	case I_LDS: 	s = "lds";  break;
	case I_LES: 	s = "les";  break;
	case I_ENTER:	s = "enter";break;
	case I_BOUND:	s = "bound";break;
	case I_ESC: 	s = "esc";  break;
	default:
		Msg_Err("Out_Nem2() ‚Ìˆø‚«”‚ª‚¨‚©‚µ‚¢");
		return;
	}
	oOffsetFlg = 0;
	if (StrExpr(oBuf,lp)) {
		oOffsetFlg = 0;
		if (StrExpr(oBuf2,rp))
			Out_NmStSt(s,oBuf,oBuf2);
	}
	return;
}

global void
Out_Nem3(word xo, Et_t_fp p1,Et_t_fp p2,Et_t_fp p3)
{
	byte *s;
	byte buf3[1000];

	if (xo == I_IMUL) {
		s = "imul";
	} else {
		Msg_Err("Out_Nem2() ‚Ìˆø‚«”‚ª‚¨‚©‚µ‚¢");
		return;
	}
	oOffsetFlg = 0;
	if (StrExpr(oBuf,p1)) {
		oOffsetFlg = 0;
		if (StrExpr(oBuf2,p2)) {
			oOffsetFlg = 0;
			if (StrExpr(buf3,p3)) {
				Out_NmStStSt(s,oBuf,oBuf2,buf3);
			}
		}
	}
	return;
}


void
Out_Enter(word siz,word level)
{
	if (Opt_cpu == 0 /**/|| siz == 0/**/) {
		if (level) {
			Msg_Err("enter‚Í8086Ó°ÄÞ‚Å‚Í‘æ“ñÊß×Ò°À‚Íí‚É0‚Å‚È‚¢‚ÆŽg‚¦‚Ü‚¹‚ñ");
		}
		Out_Nem1(I_PUSH,Ev_Reg(I_BP));
		Gen_Equ(Ev_Reg2(I_BP),Ev_Reg(I_SP));
		if (siz != 0)
			Gen_SubEq(Ev_Reg(I_SP),Ev_Cnst(siz));
	} else {
		if (Opt_r86 == 0) {
			Out_Nem2(I_ENTER,Ev_Cnst(siz),Ev_Cnst2(level));
		} else {
			if (siz >= 0 && siz <= 0xff)
				fprintf(Out_file,"\tdb\t0C8h,%d,%d,%d",siz&255,siz>>8,level);
			else
				fprintf(Out_file,"\tdb\t0C8h,0%02xh,0%02xh,%d",
					siz&0xff,siz>>8,level);
			fprintf(Out_file,"\t;enter %d,%d\n",siz,level);
		}
	}
}

void
Out_Leave(void)
{
	if (Opt_cpu)
		Out_Nem0(I_LEAVE);
	else {
		Gen_Equ(Ev_Reg(I_SP),Ev_Reg2(I_BP));
		Out_Nem1(I_POP,Ev_Reg2(I_BP));
	}
}

void
Out_ShiftReg12Cnst(word t,word reg, int n)
{
	byte regs[] = {4,5,6,7, 0,1,2,3, 0,1,2,3, 7,6,5,4};
	byte *p;
	int o0,o1;

	o0 = (Op_Reg1(reg)) ? 0xc0 : 0xc1;
	switch (t) {
	case I_ROLEQ:	o1 = 0xC0;  p = "rol";  break;
	case I_ROREQ:	o1 = 0xC8;  p = "ror";  break;
	case I_RCLEQ:	o1 = 0xD0;  p = "rcl";  break;
	case I_RCREQ:	o1 = 0xD8;  p = "rcr";  break;
	case I_SHLEQ:	o1 = 0xE0;  p = "shl";  break;
	case I_SHREQ:	o1 = 0xE8;  p = "shr";  break;
	case I_SAREQ:	o1 = 0xF8;  p = "sar";  break;
	}
	fprintf(Out_file,"\tdb\t0%02xh,0%02xh,%d\t;%s %s,%d\n",
		o0,o1+regs[reg-I_AH],n,p,StrReg(reg),n);
}
