#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "anadef.h"

static byte oExprSymFlg,oAddrFlg;
static byte ExprcondFlg;
static byte oExprIndexRegFlg;

/*-----------------------------  手続き  ------------------------------------*/
static Et_t_fp ExprUnary(void);
static Et_t_fp ExprLOr(void);
static Et_t_fp ExprEqu(void);

static Et_t_fp
ExprLVal(Et_t_fp xp)
{
	/* MSGF((" lval %lx (%s)\n",xp,Deb_ChkOp(xp->e.op))); */
	if (xp == NULL)
		return NULL;
	if (!IsEp_Lft124(xp))
		Msg_Err("左辺値でない");
	return xp;
}

static byte
ExprRVal2(Et_t_fp xp)
{
	word op;
	byte rv,lv;

	switch (op = xp->e.op) {
	case T_OFS:
	case T_CNST:
	case T_EXTCNST:
		return 2;
	case T_R2:
		switch (xp->c.reg) {
		case I_BX: oExprIndexRegFlg |= 1;	goto J1;
		case I_BP: oExprIndexRegFlg |= 2;	goto J1;
		case I_SI: oExprIndexRegFlg |= 4;	goto J1;
		case I_DI: oExprIndexRegFlg |= 8;	goto J1;
 J1:
			return 3;
		}
		return 4;
	case I_ADD:
	case I_SUB:
		lv = ExprRVal2(xp->e.lep);
		rv = ExprRVal2(xp->e.rep);
		if (lv == 2 && rv == 2)
			return 2;
		if (op == I_ADD) {
			if (lv < 4 && rv < 4)
				return 3;
		} else if (op == I_SUB) {
			if (lv < 4 && rv < 3)
				return 3;
		}
		return 4;
	case I_MUL: case I_IMUL: case I_DIV: case I_IDIV: case I_MOD: case I_IMOD:
	case I_SHL: case I_SHR: case I_SAR:
	case I_AND: case I_XOR: case I_OR:
	case I_GT: case I_GE: case I_LT: case I_LE:
	case I_IGT: case I_IGE: case I_ILT: case I_ILE:
	case I_EQEQ: case I_NEQ: case I_LAND: case I_LOR:
		lv = ExprRVal2(xp->e.lep);
		rv = ExprRVal2(xp->e.rep);
		if (lv == 2 && rv == 2)
			return 2;
	default:
		return 4;
	}
}

byte
Expr_RVal(Et_t_fp xp)
{
	oExprIndexRegFlg = 0;
	switch (xp->e.op) {
	case T_R2:
	case T_R1:
	case T_M2:
	case T_M1:
		return 0;
	case T_CNST:
		return 1;
	case T_OFS:
	case T_EXTCNST:
		return 2;
	case T_SEG2:
	case T_SEG4:
	case T_R4:
	case T_M4:
		return 5;
	default:
		return ExprRVal2(xp);
	}
}

static byte

ExprChkLea(void)
{
	if ((oExprIndexRegFlg & 3) == 3 || (oExprIndexRegFlg & 0x0c) == 0x0c) {
		Msg_Err("間接ｱﾄﾞﾚｽ指定がおかしい");
		return 1;
	}
	return 0;
}

Et_t_fp
Expr_CnvRVal(byte a, Et_t_fp xp)
{
	Et_t_fp ep,yp;

	if (a == 2) {
		Et_NEWp(ep);
		ep->e.op = T_CNSTEXPR;
		ep->e.lep = xp;
		xp = ep;
	} else if (a == 3) {
		MSG("<ADDR CHK>");
		if (ExprChkLea())
			return NULL;
		Et_NEWp(yp);
		yp->e.op = T_M1;
		yp->e.lep = xp;
		Et_NEWp(ep);
		ep->e.op = T_ADDR;
		ep->e.lep = yp;
		xp = ep;
	}
	return xp;
}


static Et_t_fp
ExprMemb(Et_t_fp xp,word typ, Et_t_fp tp)
{
	St_t_fp sp;
	long val;
	word c;

	while (tp) {
		if (tp->e.op == T_STRUCT) {
			/*typ = tp->m.siz;*/
			if (!Ch_ChkPeriod())
				break;
			Sym_GetRsvOff();
			if (Sym_tok != T_IDENT)
				goto ERR;
			if ((sp = St_Search(Sym_name,tp->v.st)) == NULL) {
				Msg_Err("構造体のﾒﾝﾊﾞ名がおかしい");
				goto ERR0;
			}
			typ = sp->v.siz;
			xp->m.ofs += sp->v.ofs;
			tp = sp->p.et;
			continue;
		} else if (tp->e.op == T_ARRAY) {
			if ((c = Ch_SkipSpc()) != '(') {
				Ch_Unget(c);
				break;
			}
			Sym_tok = I_LP;
			Sym_ChkLP();
			do {
				if (tp->e.op != T_ARRAY)
					goto ARYERR;
				Sym_Get();
				if ((val = Expr_Cnstnt()) == ERR_VAL)
					goto ERR;
				if (tp->m.ofs != 0
					&& (val < 0 || val >= (word)tp->m.ofs))
					Msg_Err("配列の添字が範囲外である");
				typ = tp->m.siz;
				xp->m.ofs += (int)(val * typ);
				tp = tp->e.lep;
			} while (Sym_tok == I_COMMA);
			Sym_ChkRP();
		} else {
 ARYERR:
			Msg_PrgErr("配列・構造体nodeが異常");
			goto ERR;
		}
	}
	if (tp) {
		if (xp->e.lep && oAddrFlg == 0)
			goto ERR;
	} else {
		if (typ == I_DWORD) {
			if (Ch_ChkPeriod()) {
				if ((c = Ch_Get()) == 'h')
					xp->m.ofs += 2;
				else if (c != 'l')
					goto ERR;
				typ = I_WORD;
			}
		}
		if (typ == I_WORD) {
			if (Ch_ChkPeriod()) {
				if ((c = Ch_Get()) == 'h')
					++xp->m.ofs;
				else if (c != 'l')
					goto ERR;
				typ = I_BYTE;
			}
		}
	}
	xp->m.siz = typ;
	return xp;
 ERR:
	Msg_Err("型指定がおかしい");
 ERR0:
	return NULL;
}

static void
ExprMemc(Et_t_fp xp,word seg)
{
	word typ;

	if (xp == NULL)
		return;
	typ = xp->m.siz;
	xp->e.op =  (typ == I_BYTE) ? T_M1 :
				(typ == I_WORD) ? T_M2 :
				(typ == I_DWORD) ? T_M4 : 0;
	if (xp->e.op == 0) {
		if (oAddrFlg == 0)
			Msg_Err("ｻｲｽﾞが1,2,4ﾊﾞｲﾄでない変数にｱｸｾｽしようとした");
		else
			xp->e.op = T_M1;
		oAddrFlg = 0;
	}
	xp->m.typ = typ;
	xp->m.seg = seg;
}


static Et_t_fp
ExprAddr(void)
{
	Et_t_fp xp,lp;

	if (oAddrFlg)
		goto ERR;
	Et_NEWp(xp);
	switch (Sym_Get()) {
	case T_IDENT:
		if ((Sym_sp = GoLbl_New(Sym_name)) == NULL)
			goto ERR;
	case T_PROC:
	case T_PROC_DECL:
	case T_JMPLBL:
	case T_LBL:
		xp->e.op = T_OFS;
		xp->v.st = Sym_sp;
		xp->m.seg = I_CS;	/* code ｾｸﾞﾒﾝﾄ･ｸﾞﾙｰﾌﾟ */
		Sym_Get();
		break;
	default:
		oExprSymFlg = 1;
		oAddrFlg = 1;
		if ((lp = ExprUnary()) == NULL)
			return NULL;
		oAddrFlg = 0;
		if (!IsEp_Mem124(lp))
			goto ERR;

		if (lp->e.lep->e.op == T_VAR) {
			xp->e.op = T_OFS;
			xp->v.st = lp->e.lep->v.st;
			xp->m.ofs = lp->m.ofs;
			xp->m.seg = lp/*->e.lep*/->m.seg;
		} else {
			xp->e.op = T_ADDR;
			xp->e.lep = lp;
		}
	}
	return xp;
 ERR:
	Msg_Err("&指定がおかしい");
	Sym_SkipCR();
	return NULL;
}

static Et_t_fp
ExprOfs(word t,St_t_fp sp)
{
	Et_t_fp xp,yp;

	MSG("<ofset>");
	Et_NEWp(xp);
	switch (t) {
	case T_IDENT:
		if ((sp = Sym_sp = GoLbl_New(Sym_name)) == NULL)
			goto ERR;
	case T_PROC:
	case T_JMPLBL:
	case T_LBL:
		xp->e.op = T_OFS;
		xp->v.st = sp;
		xp->m.seg = I_CS;	/* code ｾｸﾞﾒﾝﾄ･ｸﾞﾙｰﾌﾟ */
		break;
	case T_VAR:
		ExprMemb(xp, sp->v.siz, sp->p.et);
		xp->e.op = T_OFS;
		xp->v.st = sp;
		xp->m.typ = 0;
		xp->m.seg = sp->v.seg;
		break;
	case T_ARGLOCAL:
		xp->m.ofs = sp->v.ofs + Ana_saveOfs;
		goto JJ1;
	case T_LOCAL:
  #ifdef MVAR
	case T_MVAR:
  #endif
		xp->m.ofs = sp->v.ofs;
 JJ1:
		ExprMemb(xp, sp->v.siz, sp->p.et);
		xp->c.val = xp->m.ofs;
		xp->e.op = T_CNST;
		break;
	case I_BYTE:
	case I_WORD:
	case I_DWORD:
		t = Sym_tok;
		yp = NULL;
		goto JJ2;

	case T_STRUCT:
		Et_NEWp(yp);
		yp->e.op = T_STRUCT;
		yp->m.siz = sp->v.siz;
		yp->v.st = sp->v.st;
		goto JJ3;

	case T_TYPE:
		yp = sp->p.et;
 JJ3:
		t = 0;
 JJ2:
		ExprMemb(xp,t,yp);
		xp->c.val = xp->m.ofs;
		xp->e.op = T_CNST;
		break;
	case I_VAR:
	case I_CVAR:
		xp->e.op = T_OFS;
		xp->v.st = NULL;
		xp->m.seg = (t == I_VAR) ? 0 : I_CS;
		break;
	case T_SEG:
		xp->e.op = T_OFS;
		xp->v.st = sp;
		xp->m.seg = 0xff;
		break;
	case I_MODEL:
		xp->e.op = T_CNST;
		xp->c.val = Ana_model;
		break;
	default:
 ERR:
		Msg_Err("'%'ｵﾌｾｯﾄ指定がおかしい");
		xp = NULL;
	}
	return xp;
}

static int
ExprSize(void)
{
	word siz,t;
	Et_t_fp xp,yp;

	switch (Sym_Get()) {
	case I_FL:
	case T_R1:
		siz = 1;
		break;
	case I_FX:
	case T_SEG2:
	case T_R2:
		siz = 2;
		break;
	case I_W2D:
	case T_SEG4:
	case T_R4:
		siz = 4;
		break;

	case T_VAR:
  #ifdef MVAR
	case T_MVAR:
  #endif
	case T_LOCAL:
	case T_ARGLOCAL:
		Et_NEWp(xp);
		ExprMemb(xp, Sym_sp->v.siz, Sym_sp->p.et);
		siz = xp->m.siz;
		break;
	case I_BYTE:
	case I_WORD:
	case I_DWORD:
		t = Sym_tok;
		yp = NULL;
		goto JJ2;

	case T_STRUCT:
		Et_NEWp(yp);
		yp->e.op = T_STRUCT;
		yp->m.siz = Sym_sp->v.siz;
		yp->v.st = Sym_sp->v.st;
		goto JJ;

	case T_TYPE:
		yp = Sym_sp->p.et;
 JJ:
		t = yp->m.siz;/* 0; */
 JJ2:
		Et_NEWp(xp);
		ExprMemb(xp,t,yp);
		siz = xp->m.siz;
		break;
  #if 0
	case T_SEG:
		Et_NEWp(xp);
		xp->e.op = T_SIZE;
		xp->e.lep = Sym_sp;
		break;
  #endif
	default:
		siz = -1;
		Msg_Err("size()の指定がおかしい");
	}
	return siz;
}

static Et_t_fp
ExprMem(word typ, Et_t_fp tp)
{
	Et_t_fp xp,ep;
	word seg,t;

	MSG("<ExprMem>");
	seg = 0;
	Sym_Get();
	if (Sym_ChkLB())
		return NULL;
	t = Sym_Get();
	if (t == T_SEG2) {
		Sym_Get();
		if (Sym_tok != I_CLN) {
			goto ERR;
		}
		seg = Sym_reg;
		Sym_Get();
	}
	oExprSymFlg = 1;
	if ((ep = ExprLOr()) == NULL)
		return NULL;
	if (Sym_ChkRB())
		return NULL;
	oExprIndexRegFlg = 0;
	if (IsEp_Op(ep,I_ADD) && typ == I_BYTE && seg == 0
		&& IsEp_R2op(ep->e.lep,I_BX) && IsEp_R1op(ep->e.rep,I_AL)) {
		ep->e.op = I_BXAL;
		return ep;
	} else if (ExprRVal2(ep) > 3) {
 ERR:
		Msg_Err("間接ｱﾄﾞﾚｽ指定がおかしい");
		return NULL;
	} else if (ExprChkLea())
		return NULL;
	Et_NEWp(xp);
	xp->e.lep = ep;
	ExprMemb(xp, typ, tp);
	ExprMemc(xp, seg);
	return xp;
}

static Et_t_fp
ExprVar(word t)
{
	Et_t_fp xp,yp,lp;
	St_t_fp sp;
	word seg,c;

	MSG("<ExprVar>");
	switch (Sym_sp->v.seg) {
	case I_CS:
	case I_DS:
	case I_SS:
	case I_ES:
	case I_FAR:
		seg = Sym_sp->v.seg;
		break;
	default:
		seg = 0;
	}
	Et_NEWp(xp);
	Et_NEWp(yp);
	xp->e.lep = yp;
	sp = Sym_sp;
  #ifdef MVAR
	if (sp->p.et->e.op == T_MVAR) {
		if ((c = Ch_SkipSpc()) != '[')
			goto ERR;
		goto J1;
	}
  #endif
	if (sp->p.et->e.op == T_ARRAY) {
		if ((c = Ch_SkipSpc()) == '[') {
  #ifdef MVAR
  J1:
  #endif
			Sym_tok = I_LB;
			if (Sym_ChkLB())
				return NULL;
			Sym_Get();
			oExprSymFlg = 1;
			if ((lp = ExprLOr()) == NULL)
				return NULL;
			if (Sym_ChkRB())
				return NULL;
			oExprIndexRegFlg = 0;
			if (ExprRVal2(lp) > 3) {
				Msg_Err("間接ｱﾄﾞﾚｽ指定がおかしい");
				return NULL;
			} else if (ExprChkLea())
				return NULL;
			if (t == T_LOCAL || t == T_ARGLOCAL) {
				Et_t_fp zp;
				xp->m.ofs = sp->v.ofs;
				if (t == T_ARGLOCAL) {
					xp->m.ofs += Ana_saveOfs;
				}
				Et_NEWp(zp);
				zp->e.op  = T_R2;
				zp->c.reg = I_BP;
				zp->m.typ = I_WORD;
				yp->e.rep = zp;
			} else {
				if ((yp->e.rep = ExprOfs(t,sp)) == NULL)
					return NULL;
			}
			yp->e.op = I_ADD;
			yp->e.lep = lp;
			for (c = sp->v.siz,lp = sp->p.et;lp && lp->e.op == T_ARRAY;
				c = lp->m.siz, lp = lp->e.lep)
				;
			ExprMemb(xp, c, lp);
		} else {
			Ch_Unget(c);
			goto J1;
		}
	} else {
  J1:
		yp->e.op = t;
		yp->v.st = sp;
		if (t == T_LOCAL) {
			xp->m.ofs = Sym_sp->v.ofs;
		} else if (t == T_ARGLOCAL) {
			xp->m.ofs = Sym_sp->v.ofs + Ana_saveOfs;
			yp->e.op = T_LOCAL;
		}
		ExprMemb(xp, sp->v.siz, sp->p.et);
	}
	ExprMemc(xp, seg);
	return xp;
}

static Et_t_fp
ExprSegOvr(word seg)
{
	Et_t_fp xp;

	MSG("<ExprSegOvr>");
	Sym_tok = I_LP;
	Sym_ChkLP();
	if (Sym_Get() != T_VAR && Sym_tok != T_LOCAL && Sym_tok != T_ARGLOCAL)
		goto ERR;
	xp = ExprVar(Sym_tok);
	if (xp == NULL || !IsEp_Mem124(xp))
		goto ERR;
	Sym_Get();
	Sym_ChkRP();
	xp->m.seg = seg;
	return xp;
 ERR:
	Msg_Err("ｾｸﾞﾒﾝﾄ･ｵｰﾊﾞｰﾗｲﾄﾞ指定がおかしい");
	return NULL;
}

static Et_t_fp
ExprPort(void)
{
	word a;
	Et_t_fp xp,yp;

	MSG("<ExprPort>");
	Sym_Get();
	if (Sym_ChkLP())
		return NULL;
	Et_NEWp(xp);
	xp->e.op = I_PORT;
	xp->m.typ = 0;
	if ((yp = ExprLOr()) == NULL)
		return NULL;
	if ((a = Expr_RVal(yp)) > 2
		|| (a == 1 && yp->c.val >= 0x100)
		|| (a == 0 && yp->c.reg != I_DX)) {
		Msg_Err("port()の指定がおかしい");
		return NULL;
	}
	xp->e.lep = Expr_CnvRVal(a,yp);
	if (Sym_ChkRP())
		return NULL;
	return xp;
}

#if 0
static Et_t_fp
ExprSeg(void)
{
	Et_t_fp xp;

	Sym_Get();
	if (Sym_ChkLP())
		goto SEG1;
	Et_NEWp(xp);
	xp->e.op = I_OFS;
	xp->v.st = NULL;
	if (Sym_Get() == T_VAR||Sym_tok == I_VAR)
		xp->m.seg = 0;
	else if (Sym_tok==T_ARGLOCAL || Sym_tok==T_LOCAL || Sym_tok==I_LOCAL)
		xp->m.seg = 0;
	else if (Sym_tok == T_PROC||Sym_tok == I_CVAR)
		xp->m.seg = I_CS;
	else
		goto SEG1;
	if (Sym_ChkRP()) {
  SEG1:
		Msg_Err("seg()の指定がおかしい");
		return NULL;
	}
	return xp;
}
#endif

static Et_t_fp
ExprW2D(void)
{
	Et_t_fp xp,yp;
	byte a;

	Sym_Get();
	if (Sym_ChkLP())
		goto DW1;
	Et_NEWp(xp);
	xp->e.op = I_W2D;
	if ((yp = ExprUnary()) == NULL)
		return NULL;
	if ((a = Expr_RVal(yp)) > 2 && !IsEp_Seg2(yp))
		goto DW1;
	xp->e.lep = Expr_CnvRVal(a,yp);
	if (Sym_tok != I_COMMA)
		goto DW1;
	if ((yp = ExprUnary()) == NULL)
		return NULL;
	a = Expr_RVal(yp);
	/*if (a > 2 && !IsEp_Seg2(yp))
		goto DW1;*/
	xp->e.rep = Expr_CnvRVal(a,yp);
	if (Sym_ChkRP()) {
  DW1:
		Msg_Err("ww()の指定がおかしい");
		return NULL;
	}
	if (IsEp_Cnst(xp->e.lep) && IsEp_Cnst(xp->e.rep)) {
		xp->c.val = xp->e.lep->c.val * 0x10000L + xp->e.rep->c.val;
		xp->e.op = T_CNST;
	}
	return xp;
}

Et_t_fp
ExprNegCom(word t,word f)
{
	Et_t_fp xp,yp;
	word parflg;

	MSG("<neg()>");

	parflg = Sym_GetChkLP();
	if (parflg == 0 && f == 0)
		Msg_Err("必要な'('がない");
	Et_NEWp(xp);
	xp->e.op = t;
	xp->e.rep = NULL;
	oExprSymFlg = 1;
	if ((yp = ExprUnary()) == NULL)
		return NULL;
	if (t == I_COMS && yp->e.op == I_CF)
		xp->e.op = I_COMC;
	else if ((xp->e.lep = ExprLVal(yp)) ==  NULL)
		return NULL;
	if (parflg) {
		Sym_ChkRP();
		Sym_Get();
	}
	return xp;
}

Et_t_fp
ExprShls(word t,word f)
{
	Et_t_fp xp,yp;
	int  a;
	word parflg;

	MSG("<rol>");
	parflg = Sym_GetChkLP();
	if (parflg == 0 && f == 0)
		Sym_ChkLP();
	Et_NEWp(xp);
	xp->e.op = t;
	oExprSymFlg = 1;
	if ((xp->e.lep  = ExprLVal(ExprUnary())) == NULL)
		return NULL;
	if (Sym_tok == I_COMMA) {
		if ((yp = ExprLOr()) == NULL)
			return NULL;
		if ((a = Expr_RVal(yp)) > 2) {
			Msg_Err("ｼﾌﾄ命令の第二引数がおかしい");
			return NULL;
		}
		xp->e.rep = Expr_CnvRVal(a,yp);
	} else {
		if (t == I_DIVS || t == I_IDIVS||t == I_ADCEQ || t == I_SBCEQ)
			Msg_Err("div,idiv,adc,sbc がおかしい");
		Et_NEWp(yp);
		yp->e.op = T_CNST;
		yp->c.val = 1;
		xp->e.rep	= yp;
	}
	if (parflg) {
		Sym_ChkRP();
		Sym_Get();
	}
	return xp;
}

static Et_t_fp
ExprUnary(void)
{
	Et_t_fp xp;
	Et_t_fp yp;
	word t;

	if (oExprSymFlg) {
		t = Sym_tok;
		oExprSymFlg = 0;
	} else
		t = Sym_Get();

	switch (t) {
	case I_LP:
		MSG("<(括弧)>");
		Sym_ChkLP();
		xp = ExprEqu();
		if (Sym_ChkRP())
			return NULL;
		break;

	case T_CNST:
		Et_NEWp(xp);
		MSG("<定数>");
		xp->c.val = Sym_val;
		xp->e.op = T_CNST;
		break;

	case T_CONST:
		Et_NEWp(xp);
		MSG("<定数>");
		xp->c.val = Sym_sp->c.val;
		xp->e.op = T_CNST;
		break;

	case T_EXTCONST:
		Et_NEWp(xp);
		MSG("<外部定数>");
		xp->v.st = Sym_sp;
		xp->e.op = T_EXTCNST;
		break;

	case T_VAR:
	case T_LOCAL:
	case T_ARGLOCAL:
		xp = ExprVar(t);
		break;

	case T_STRUCT:
		Et_NEWp(xp);
		xp->e.op = T_STRUCT;
		xp->m.siz = Sym_sp->v.siz;
		xp->v.st = Sym_sp->v.st;
		xp = ExprMem(xp->m.siz,xp);
		break;

	case T_TYPE:
		xp = ExprMem(Sym_sp->v.siz,Sym_sp->p.et);
		break;

	case I_BYTE:
	case I_WORD:
	case I_DWORD:
		xp = ExprMem(t,NULL);
		break;

	case T_SEG2:
		if ((t = Ch_SkipSpc()) == '(') {
			xp = ExprSegOvr(Sym_reg);
			break;
		}
		Ch_Unget(t);
	case T_R1:
	case T_R2:
	case T_R4:
	case T_SEG4:
		Et_NEWp(xp);
		MSG("<ﾚｼﾞｽﾀ>");
		xp->e.op  = Sym_tok;
		xp->c.reg = Sym_reg;
		xp->m.typ = Sym_typ;
		break;

	case I_FL:
	case I_FX:
		Et_NEWp(xp);
		MSG("<FL,FX>");
		xp->e.op = xp->c.reg = t;
		xp->m.typ = (t == I_FX) ? I_WORD : I_BYTE;
		break;

	case I_MOD:
		Sym_Get();
		xp = ExprOfs(Sym_tok,Sym_sp);
		break;

	case I_AND:
		xp = ExprAddr();
		goto ENDF;

	case I_MUL:
		Et_NEWp(xp);
		xp->e.op = T_OFS;
		xp->m.seg = 0x7f;
		break;
 #if 0
	case I_DOLL:
 #endif
	case I_TYPEOF:
		Sym_Get();
		if (Sym_ChkLP())
			goto ERR;
		t = Sym_Get();
		xp = (void far *)Sym_sp;
		Sym_Get();
		if (Sym_ChkRP())
			goto ERR;
		Sym_sp = (void far *)xp;
		switch (t) {
		case T_VAR:
		case T_LOCAL:
		case T_ARGLOCAL:
			xp = ExprMem(Sym_sp->v.siz,Sym_sp->p.et);
			break;
		default:
			goto ERR;
		}
		break;

	case I_ADD:
	case I_SUB:
	case I_COM:
	case I_NOT:
		MSG("<~,!,+,->");
		if ((yp = ExprUnary()) == NULL)
			return NULL;
		if (t == I_ADD)
			xp = yp;
		else if (IsEp_Cnst(yp)) {
			yp->c.val = (t == I_SUB) ? -yp->c.val :
						(t == I_COM) ? ~yp->c.val :
						(t == I_NOT) ? !yp->c.val : 0;
			xp = yp;
		} else {
			Et_NEWp(xp);
			xp->e.op = (t == I_SUB) ? I_NEG : t;
			xp->e.lep = yp;
		}
		goto ENDF;

	case I_PORT:
		xp = ExprPort();
		break;

	case I_OVF:
	case I_CF:
	case I_AF:
	case I_TF:
	case I_PF:
	case I_SF:
	case I_IIF:
	case I_ZF:
	case I_DF:
		MSG("<ﾌﾗｸﾞ>");
		Et_NEWp(xp);
		xp->e.op = t;
		break;

	case I_SIZEOF:
		MSG("<size()>");
		Sym_Get();
		if (Sym_ChkLP())
			return NULL;
		Et_NEWp(xp);
		xp->e.op = T_CNST;
		xp->c.val = ExprSize();
		Sym_Get();
		if (Sym_ChkRP())
			return NULL;
		break;

	case I_DEFINED:
		MSG("<defined()>");
		Sym_Get();
		if (Sym_ChkLP())
			return NULL;
		Et_NEWp(xp);
		xp->e.op = T_CNST;
		if (Sym_Get() == T_IDENT)
			xp->c.val = 0;
		else
			xp->c.val = 1;
		Sym_Get();
		if (Sym_ChkRP())
			return NULL;
		break;

	case I_W2D:
		xp = ExprW2D();
		break;
/*
	case I_SEG:
		xp = ExprSeg();
		break;
*/
	case T_STRING:
		MSG("<文字列>");
		{
			Sss_t *s = malloc(sizeof(Sss_t));
			if (s == NULL)
				Msg_Err("文字列用のバッファが確保できなかった");
			Et_NEWp(xp);
			xp->e.op = T_OFS;
			xp->v.st = NULL;
			xp->m.seg = I_SI;
			xp->m.siz = GoLbl_NewNo();
			if (s) {
				s->no = xp->m.siz;
				s->str = strdup(Sym_str);
				s->next = NULL;
				if (Expr_strlTop)
					Expr_strl->next = s;
				else
					Expr_strlTop = s;
				Expr_strl = s;
			}
		}
		break;
	case I_INC:
	case I_DEC:
		MSG("<inc>");
		Et_NEWp(xp);
		xp->e.op = t;
		xp->e.rep = NULL;
		if ((xp->e.lep = ExprLVal(ExprUnary())) == NULL)
			return NULL;
		goto ENDF;

	case I_NEGS:
	case I_COMS:
		xp = ExprNegCom(t,0);
		goto ENDF;

	case I_SARS:t = I_SAREQ;goto JJ;
	case I_SHRS:t = I_SHREQ;goto JJ;
	case I_SHLS:t = I_SHLEQ;goto JJ;
	case I_DIVS:
	case I_IDIVS:
	case I_ADCEQ:
	case I_SBCEQ:
	case I_ROLEQ:
	case I_ROREQ:
	case I_RCLEQ:
	case I_RCREQ:
 JJ:
		xp = ExprShls(t,0);
		goto ENDF;

	default:
		goto ERR;
	}
	if (xp != NULL)
		Sym_Get();
 ENDF:
	EXPMSG("unary",xp);
	return xp;
 ERR:
	Msg_Err("読み込んだｼﾝﾎﾞﾙがおかしい");
	return NULL;
}

static Et_t_fp
ExprBin(word t, word mf, Et_t_fp lp, Et_t_fp rp)
{
	word lo,ro;
	long lv,rv;
	Et_t_fp xp,tmp;

	if (!lp || !rp)
		return NULL;
	MSG("ExprBin");
	lo = lp->e.op;
	ro = rp->e.op;
	if (lo == T_CNST && ro == T_CNST) {
		lv = lp->c.val;
		rv = rp->c.val;
		switch (t) {
		case I_MUL: lv *= rv;	break;
		/*case I_IMUL:lv *= rv; break;*/
		case I_DIV: lv /= rv;	break;
		/*case I_IDIV:lv /= rv; break;*/
		case I_MOD: lv %= rv;	break;
		/*case I_IMOD:lv %= rv; break;*/
		case I_ADD: lv += rv;	break;
		case I_SUB: lv -= rv;	break;
		case I_SHL: lv <<= rv;  break;
		case I_SHR: (dword)lv >>= rv;  break;
		case I_SAR: (long)lv >>= rv;   break;
		case I_AND: lv &= rv;	break;
		case I_XOR: lv ^= rv;	break;
		case I_OR:  lv |= rv;	break;
		case I_LE:  lv = (lv <= rv);	break;
		case I_GT:  lv = (lv >  rv);	break;
		case I_GE:  lv = (lv >= rv);	break;
		case I_LT:  lv = (lv <  rv);	break;
		/*case I_ILE:	lv = (lv <= rv);	break;*/
		/*case I_IGT:	lv = (lv >  rv);	break;*/
		/*case I_IGE:	lv = (lv >= rv);	break;*/
		/*case I_ILT:	lv = (lv <  rv);	break;*/
		case I_EQEQ:lv = (lv == rv);	break;
		case I_NEQ: lv = (lv != rv);	break;
		case I_LAND:lv = (lv && rv);	break;
		case I_LOR: lv = (lv || rv);	break;
		default:
			Msg_Err("定数演算できない演算子が使われた");
		}

		lp->c.val = lv;
		return lp;
	} else if (mf && ro != T_CNST) {
		if (lo == T_CNST
			||(ro == T_R4 &&(lo==T_R2||lo==T_R1||lo==T_M2||lo==T_M1))
			||((ro==T_R2||ro==T_M2)&&(lo==T_R1||ro==T_M1))
			||(lo!=T_R2&&lo!=T_R1&&lo!=T_R4&&lo!=T_M2&&lo!=T_M1)
		   ){
			tmp = lp;
			lp = rp;
			rp = tmp;
			lo = lp->e.op;
			ro = rp->e.op;
		}
	}
	if (ro == T_CNST) {
		if (t == I_MUL || t == I_IMUL || t == I_DIV || t == I_IDIV) {
			if ((lo == I_MUL || lo == I_IMUL) && IsEp_Cnst(lp->e.rep)) {
				lp->e.rep->c.val = (t	== I_MUL || t == I_IMUL) ?
					lp->e.rep->c.val * rp->c.val:
					lp->e.rep->c.val / rp->c.val;
				return lp;
			} else if (lo == I_DIV || lo == I_IDIV) {
				if (IsEp_Cnst(lp->e.lep)) {
					lp->e.lep->c.val =  (t == I_MUL || t == I_IMUL) ?
						lp->e.lep->c.val *  rp->c.val:
						lp->e.lep->c.val /  rp->c.val;
					return lp;
				} else  if (IsEp_Cnst(lp->e.rep)) {
					lp->e.rep->c.val = (t	== I_MUL || t == I_IMUL) ?
						rp->c.val / lp->e.rep->c.val:
						rp->c.val * lp->e.rep->c.val;
					lp->e.op = t;
					return lp;
				}
			}
		} else if (t == I_ADD || t == I_SUB) {
			if (lo == I_ADD && IsEp_Cnst(lp->e.rep)) {
				lp->e.rep->c.val = (t	== I_ADD) ?
					lp->e.rep->c.val + rp->c.val:
					lp->e.rep->c.val - rp->c.val;
				return lp;
			} else if (lo == I_SUB && IsEp_Cnst(lp->e.rep)) {
				lp->e.rep->c.val = (t	== I_ADD) ?
					rp->c.val - lp->e.rep->c.val:
					rp->c.val + lp->e.rep->c.val;
				lp->e.op = t;
				return lp;
			}
		}
	}
	Et_NEWp(xp);
	xp->e.op = t;
	xp->e.lep = lp;
	xp->e.rep	= rp;
	return xp;
}

static Et_t_fp
ExprMul(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprUnary();
	while (lp) {
		/* MSGF((" mul %lx\n",lp)); */
		switch (t = Sym_tok) {
		case I_MUL: case I_IMUL:
			MSG("<*>");
			lp = ExprBin(t,1,lp,ExprUnary());
			break;
		case I_DIV: case I_IDIV:
		case I_MOD: case I_IMOD:
			MSG("</>");
			lp = ExprBin(t,0,lp,ExprUnary());
			break;
		default:
			return lp;
		}
	}
	return lp;
}

static Et_t_fp
ExprAdd(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprMul();
	while (lp) {
		/* MSGF((" add %lx\n",lp)); */
		switch (t = Sym_tok) {
		case I_ADD:
			MSG("<+>");
			lp = ExprBin(t,1,lp,ExprMul());
			break;
		case I_SUB:
			MSG("<->");
			lp = ExprBin(t,0,lp,ExprMul());
			break;
		default:
			return lp;
		}
	}
	return lp;
}

static Et_t_fp
ExprShl(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprAdd();
	while (lp) {
		/* MSGF((" shl %lx\n",lp)); */
		switch (t = Sym_tok) {
		case I_SAR:
		case I_SHR:
		case I_SHL:
			MSG("<shift>");
			lp = ExprBin(t,0,lp,ExprAdd());
			break;
		default:
			return lp;
		}
	}
	return lp;
}

static Et_t_fp
ExprGtLt(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprShl();
	while (lp) {
		/* MSGF((" gtlt %lx\n",lp)); */
		switch (t = Sym_tok) {
		case I_GT:case I_GE:case I_LT:case I_LE:
		case I_IGT:case I_IGE:case I_ILT:case I_ILE:
		case I_CFCM:case I_SFCM:case I_ZFCM:
		case I_OVFCM:case I_PFCM:/*case I_AFCM:*/
			MSG("<gtlt>");
			lp = ExprBin(t,0,lp,ExprShl());
			break;
		default:
			return lp;
		}
	}
	return lp;
}

static Et_t_fp
ExprEqEq(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprGtLt();
	while (lp) {
		/* MSGF((" eqeq %lx\n",lp)); */
		switch (t = Sym_tok) {
		case I_EQEQ:
		case I_NEQ:
			MSG("< == >");
			lp = ExprBin(t,1,lp,ExprGtLt());
			break;
		default:
			return lp;
		}
	}
	return lp;
}

static Et_t_fp
ExprAnd(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprEqEq();
	/* MSGF((" mul %lx\n",lp)); */
	while (lp && (t = Sym_tok) == I_AND) {
		MSG("<&>");
		lp = ExprBin(t,1,lp,ExprEqEq());
	}
	return lp;
}

static Et_t_fp
ExprEor(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprAnd();
	/* MSGF((" eor %lx\n",lp)); */
	while (lp && (t = Sym_tok) == I_XOR) {
		MSG("<^>");
		lp = ExprBin(t,1,lp,ExprAnd());
	}
	return lp;
}

static Et_t_fp
ExprOr(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprEor();
	/* MSGF((" or %lx\n",lp)); */
	while (lp && (t = Sym_tok) == I_OR) {
		MSG("<|>");
		lp = ExprBin(t,1,lp,ExprEor());
	}
	return lp;
}

static Et_t_fp
ExprLAnd(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprOr();
	/* MSGF((" land %lx\n",lp)); */
	while (lp && (t = Sym_tok) == I_LAND) {
		MSG("<&&>");
		lp = ExprBin(t,0,lp,ExprOr());
	}
	return lp;
}

static Et_t_fp
ExprLOr(void)
{
	Et_t_fp lp;
	word	t;

	lp = ExprLAnd();
	/* MSGF((" lor %lx\n",lp)); */
	while (lp && (t = Sym_tok) == I_LOR) {
		MSG("<||>");
		lp = ExprBin(t,0,lp,ExprLAnd());
	}
	return lp;
}


static Et_t_fp
ExprEqu2(word a,Et_t_fp xp,Et_t_fp lp,Et_t_fp rp)
{
	word lo,ro;

	lo = lp->e.op;
	ro = rp->e.op;
	switch (a) {
	case 4:
		if (ro == I_MUL || ro == I_IMUL) {
			word a1,a2;
			if ((a1 = Expr_RVal(rp->e.lep)) > 2
				|| (a2 = Expr_RVal(rp->e.rep)) > 2)
				Msg_Err("右辺がおかしい(L*R)");
			rp->e.lep = Expr_CnvRVal(a1,rp->e.lep);
			rp->e.rep = Expr_CnvRVal(a2,rp->e.rep);
		}
		break;
	case 3:
		if (lo != T_R2 && lo != T_R4 && lo != I_EQU)
			Msg_Err("左辺は16/32ﾋﾞｯﾄ･ﾚｼﾞｽﾀでないといけない");
		break;
	case 2:
	case 1:
		if (lo==I_CF || lo==I_IIF ||lo==I_DF) {
			if (ro == T_CNST) {
				if (rp->c.val == 0) {
					xp->e.op =  (lo == I_CF) ? I_CLRC :
								(lo == I_IIF) ? I_CLRI : I_DF;
				} else if (rp->c.val == 1) {
					xp->e.op =  (lo == I_CF) ? I_SETC :
								(lo == I_IIF) ? I_SETI : I_DF;
				}
			}
		}
	}
	return xp;
}

static Et_t_fp
ExprEqu(void)
{
	Et_t_fp xp;
	Et_t_fp lp, rp;
	word t,a;

	lp = ExprLOr();
	while (lp) {
		switch(t = Sym_tok) {
		case I_SIEQ:
			if (IsEp_R2op(lp,I_AX)||IsEp_R4op(lp,I_DXAX))
				goto J2;
			Msg_Err(".=. の左辺は ax か dx.ax でないといけない");
			return NULL;
		case I_EQU:
			switch (lp->e.op) {
			case I_FX:case I_FL:case T_SEG2:case T_SEG4:case I_PORT:
			case I_CF:case I_DF:case I_IIF: case I_EQU:
				goto J2;
			}
		case I_ADDEQ:
		case I_SUBEQ:
		case I_ANDEQ:
		case I_OREQ:
		case I_EOREQ:
		case I_SHLEQ:
		case I_SHREQ:
		case I_SAREQ:
		case I_EXG:
			if (!IsEp_Lft124(lp))
				return NULL;
 J2:
			MSG("< = >");
			Et_NEWp(xp);
			xp->e.op = t;
			xp->e.lep = lp;
			if ((rp = ExprLOr()) == NULL)
				return NULL;
 J3:
			a = Expr_RVal(rp);
			xp->e.rep = rp = Expr_CnvRVal(a,rp);
			if (a > 2/*1*/ && t != I_EQU && !IsEp_Rht4(rp)) {
				Msg_Err("右辺がおかしい");
				return NULL;
			}
			if (IsEp_MemMem(lp,rp))
				return NULL;
			switch (t) {
			case I_EXG:
				if (a > 0 && a < 5) {
					Msg_Err("<=>の右辺がおかしい");
					return NULL;
				}
				if (lp->m.typ != rp->m.typ) {
					Msg_Err("両辺の型が会わない");
					MSGF(("型 %d,%d\n",lp->m.typ,lp->m.typ));
				}
		/*  case I_ADDEQ:
			case I_SUBEQ:
			case I_ANDEQ:
			case I_OREQ:
			case I_EOREQ:
				break;*/
			case I_SUB:
			case I_AND:
				lp = xp;
				goto LOOPOUT;
			case I_EQU:
				xp = ExprEqu2(a,xp,lp,rp);
			}
			lp = xp;
			break;
		default:
			a = Expr_RVal(lp);
			t = lp->e.op;
			if (ExprcondFlg
				&& ((a == 3 || a == 4) && (t == I_SUB || t == I_AND))) {
				xp = lp;
				lp = xp->e.lep;
				rp = xp->e.rep;
				goto J3;
			} else {
				lp = Expr_CnvRVal(a,lp);
			}
			goto LOOPOUT;
		}
	}
 LOOPOUT:
	return lp;
}

Et_t_fp
Expr(byte mode)
{
	Et_t_fp xp;

	MSG("\n<<<Expr>>>");
	oAddrFlg = 0;
	oExprSymFlg = 1;
	ExprcondFlg = mode;
	xp = ExprEqu();
	ExprcondFlg = 0;
	MSGF(("<<EXP [%s  xp=%lx]>>\n",Deb_ChkOp(xp->e.op),xp));
	return xp;
}

static Et_t_fp
ExprIntr(void)
{
	Et_t_fp ep;

	Et_NEWp(ep);
	if (Ch_ChkPeriod()) {
		Sym_NameGet(Ch_Get());
		if (strcmp(Sym_name,"ovf") == 0)
			ep->e.op = I_INTO;
		else if (strcmp(Sym_name,"on") == 0)
			ep->e.op = I_SETI;
		else if (strcmp(Sym_name,"off") == 0)
			ep->e.op = I_CLRI;
		else
			Msg_Err("intr の指定がおかしい");
		Sym_Get();
	} else {
		Sym_Get();
		ep->e.op = I_INTR;
		ep->e.lep = Expr(0);
		IsEp_CnstTyp(I_BYTE,ep->e.lep);
	}
	return ep;
}

static Et_t_fp
ExprBound(void)
{
	Et_t_fp xp, ep;

	if (Opt_cpu == 0)
		Msg_Err("8086では使えない命令");
	Et_NEWp(xp);
	xp->e.op = I_BOUND;
	Sym_Get();
	xp->e.lep = ep = Expr(0);
	if (Sym_tok != I_COMMA)
		goto ERR;
	Sym_Get();
	xp->e.rep = Expr(0);
	if (IsEp_Reg2(ep) && Op_RegD(ep->e.op) && IsEp_Mem4(xp->e.rep)) {
		;
	} else {
 ERR:
		Msg_Err("bound の指定がおかしい");
	}
	return xp;
}

static Et_t_fp
ExprEnter(void)
{
	Et_t_fp xp, ep, ep2;

	Et_NEWp(xp);
	xp->e.op = I_ENTER;
	Sym_Get();
	xp->e.lep = ep = Expr(0);
	if (Sym_tok != I_COMMA)
		goto ERR;
	Sym_Get();
	xp->e.rep = ep2 = Expr(0);
	if (ep && ep2 && IsEp_Cnst(ep) && IsEp_Cnst(ep2)
		&& !IsEp_CnstTyp(I_WORD,ep) && !IsEp_CnstTyp(I_BYTE,ep2)) {
		;
	} else {
 ERR:
		Msg_Err("enter のパラメータがおかしい");
	}
	return xp;
}

static Et_t_fp
ExprRet(word t)
{
	Et_t_fp xp,ep;

	Et_NEWp(xp);
	if (t == I_RET && Ch_ChkPeriod()) {
		Sym_NameGet(Ch_Get());
		if (strcmp(Sym_name,"d") == 0) {
			t = I_RETF;
		} else {
			Msg_Err("retの指定がおかしい");
		}
	}
	xp->e.op = t;
	if (Sym_Get() == I_CR || Sym_tok == I_SMCLN) {
		;
	} else {
		xp->e.lep = ep = Expr(0);
		if (IsEp_Cnst(ep)	> 0)
			IsEp_CnstTyp(I_WORD,ep);
		else if (!IsEp_Cnsts(ep))
			Msg_Err("ret で定数以外が指定された");
	}
	return xp;
}

static Et_t_fp
ExprEsc(void)
{
	Et_t_fp xp;

	Et_NEWp(xp);
	xp->e.op = I_ESC;
	Sym_Get();
	if ((xp->e.lep = Expr(0)) == NULL)
		goto ENDF;
	if (IsEp_CnstTyp(I_BYTE,xp->e.lep))
		goto ERR;
	if (Sym_tok != I_COMMA)
		goto ERR;
	Sym_Get();
	if ((xp->e.rep = Expr(0)) == NULL)
		goto ENDF;
	if (!IsEp_Lft124(xp->e.rep))
		goto ERR;
 ENDF:
	return xp;
 ERR:
	Msg_Err("esc の指定がおかしい");
	return NULL;

}

static Et_t_fp
ExprPushPop(word t)
{
	Et_t_fp cdp,xp,cdp0;

	Sym_Get();
	if (Sym_tok == I_CR || Sym_tok == I_SMCLN) {
		Msg_Err("push,pop のパラメータがない");
		return NULL;
	}
	Et_NEWp(xp);
	cdp0 = xp;
	cdp = NULL;
	for (;;) {
		xp->e.op = t;
		if ((xp->e.lep = Expr(0)) == NULL)
			return NULL;
		if (cdp)
			cdp->e.rep = xp;
		cdp = xp;
		if (Sym_tok != I_COMMA)
			break;
		Sym_Get();
		Et_NEWp(xp);
	}
	Sym_ChkEol();
	return cdp0;
}

Et_t_fp
Expr_Stat(byte mode)
{
	word t;
	Et_t_fp xp;

	switch (t = Sym_tok) {
	case I_SARS:Sym_tok = I_SAREQ;goto JJ;
	case I_SHRS:Sym_tok = I_SHREQ;goto JJ;
	case I_SHLS:Sym_tok = I_SHLEQ;goto JJ;
	case I_DIVS:
	case I_IDIVS:
	case I_ADCEQ:
	case I_SBCEQ:
	case I_ROLEQ:
	case I_ROREQ:
	case I_RCLEQ:
	case I_RCREQ:
 JJ:
		xp = ExprShls(Sym_tok,1);
		break;
	case I_NEGS:
	case I_COMS:
		xp = ExprNegCom(Sym_tok,1);
		break;
	case I_INTR:
		xp = ExprIntr();
		break;
	case I_RET:
	case I_RETF:
		xp = ExprRet(Sym_tok);
		break;
	case I_PUSH:
	case I_POP:
		xp = ExprPushPop(Sym_tok);
		break;
	case I_ENTER:
		if (mode && Opt_cpu == 0)
			goto ERR2;
		xp = ExprEnter();
		break;
	case I_LEAVE:
		if (mode && Opt_cpu == 0)
			goto ERR2;
		Et_NEWp(xp);
		xp->e.op = I_LEAVE;
		Sym_Get();
		break;
	case I_LOAD:
		Et_NEWp(xp);
		xp->e.op = I_LOAD;
		Sym_Get();
		break;
	case I_BOUND:
		xp = ExprBound();
		break;
	case I_ESC:
		xp = ExprEsc();
		break;
	case I_PUSHA:
	case I_POPA:
		if (Opt_cpu == 0) {

			Et_NEWp(xp);
			xp->e.op = t;
			Sym_Get();
			break;
		}
	default:
		if (Op_One(t)) {
			Et_NEWp(xp);
			xp->e.op = T_ONE;
			xp->m.ofs = t;
			Sym_Get();
		} else {
			xp = Expr(mode);
		}
	}
	return xp;
 ERR2:
	Msg_Err("'|'指定で使えない合成命令が指定された");
	return NULL;
}

long
Expr_Cnstnt(void)
{
	Et_t_fp xp;
	Et_t_fp sav_ep;
	long val;

	sav_ep = Et_Sav();
	xp = Expr(0);
	if (!IsEp_Cnst(xp)) {
		Msg_Err("定数値でない");
		val = ERR_VAL;
	} else
		val = xp->c.val;
	Et_Frees(sav_ep);
	return val;
}

static int
ExprOpGo(Et_t_fp ep)
{
	word t;

	switch (ep->e.op) {
	case I_CF:  t = GO_C;	break;
	case I_ZF:  t = GO_Z;	break;
	case I_SF:  t = GO_S;	break;
	case I_OVF: t = GO_O;	break;
	case I_PF:  t = GO_P;	break;
	case T_R2:
		if (ep->c.reg == I_CX) {
			t = GO_CX;
			break;
		}
	default:	t = 0;
	}
	return t;
}

word
Expr_CondNeg(word jt)
{
	if (jt & 0x0001)
		jt &= ~(0x0001);
	else
		jt |= 0x0001;
	return jt;
}

Et_t_fp
Expr_Cond(void)
{
	Et_t_fp yp;
	Et_t_fp lp;
	Et_t_fp rp;
	word t = 0;
	word nf = 0;

	if (Sym_tok == I_NOT) {
		nf = (nf == 0);
		Sym_Get();
	}
	if (Sym_ChkLP())
		return NULL;
	yp = NULL;
	switch (Sym_Get()) {
	case I_GT:  t = GO_GT;  goto J1;
	case I_LE:  t = GO_LE;  goto J1;
	case I_LT:  t = GO_LT;  goto J1;
	case I_GE:  t = GO_GE;  goto J1;
	case I_IGT: t = GO_IGT; goto J1;
	case I_ILE: t = GO_ILE; goto J1;
	case I_ILT: t = GO_ILT; goto J1;
	case I_IGE: t = GO_IGE; goto J1;
	case I_NEQ: t = GO_NEQ; goto J1;
	case I_EQEQ:t = GO_EQU;
 J1:
		Sym_Get();
		break;
	default:
		if ((yp = Expr(1)) == NULL)
			return NULL;
		t = 0;
		lp = yp->e.lep;
		rp = yp->e.rep;
		switch (yp->e.op) {
		case I_NEQ:
		case I_EQEQ:
			if (IsEp_Cnst(rp)) {
				if (rp->c.val == 0 || rp->c.val == 1) {
					t = ExprOpGo(lp);
					if (t == GO_CX && rp->c.val == 1)
						t = 0;
					if (t && rp->c.val == (yp->e.op == I_NEQ))
						t = Expr_CondNeg(t);
				}
			}
			break;
		case I_NOT:
			if ((t = ExprOpGo(lp)) != 0)
				t = Expr_CondNeg(t);
			break;
		case I_DEC:
			if (IsEp_R2op(lp,I_CX))
				t = GO_DECX;
			break;
		case I_LAND:
			if (lp->e.op == I_DEC && IsEp_R2op(lp->e.lep,I_CX)) {
				lp = rp->e.lep;
				switch (rp->e.op) {
				case I_ZF:
					t = GO_DECX_Z;
					break;
				case I_NOT:
					if (lp->e.op == I_ZF)
						t = GO_DECX_NZ;
					break;
				case I_EQEQ:
					rp = rp->e.rep;
					if (lp->e.op == I_ZF && IsEp_Cnst(rp)) {
						if (rp->c.val == 0)
							t = GO_DECX_NZ;
						else if (rp->c.val == 1)

							t = GO_DECX_Z;
					}
					break;
				}
			}
			break;
		default:
			t = ExprOpGo(yp);
		}
	}
	if (Sym_ChkRP())
		return NULL;
	Et_NEWp(lp);
	lp->e.op = T_COND;
	lp->j.nf = 0;
	if (t) {
		if (nf)
			t = Expr_CondNeg(t);
		lp->j.cond = t;
		lp->j.nf = 0;
		lp->e.lep = NULL;
	} else {
		lp->j.cond = 0;
		lp->j.nf = nf;
		lp->e.lep = yp;
	}
	return lp;
}
