#include <string.h>
#include <stdlib.h>
#include "anadef.h"

BrkCont_t *gStatBrkCont_p;
static word oJmpFlg = 0;

/*-----------------------------  手続き  ------------------------------------*/
static void
OutJmpNo(word n,byte long_f)
{
	byte_fp p;

	p = GoLbl_Strs(n);
	if (long_f)
		Out_NmSt("jmp",p);
	else
		Out_NmSt(gJmps[Opt_r86],p);
}

global St_t_fp
GoLbl_New(byte_fp name)
{
	St_t_fp sp;

	if ((sp = Sym_NameNew(name,T_LBL,Ana_mode)) != NULL) {
		sp->l.ofs = GoLbl_NewNo();
	}
	return sp;
}

static void
StatLbl(word t)
{
	St_t_fp st;

	if (t == T_IDENT)
		st = GoLbl_New(Sym_name);
	else
		st = Sym_sp;
	if (Sym_Get() != I_CLN) {
		if (t == T_IDENT)
			Msg_Err("未定義ラベルがある");
		else
			Msg_Err("gotoラベルの指定がおかしい");
		Sym_SkipCR();
	} else if (st)
		Out_golbl(st->l.ofs);
	return;
}

static void
StatGoto(word t)
{
	byte long_f;

	MSG("StatGoto\n");
	long_f = Ch_ChkBW();
	if (Sym_Get() == T_IDENT) {
		if ((Sym_sp = GoLbl_New(Sym_name)) == NULL)
			return;
	} else if (Sym_tok == T_LBL) {
		;
	} else {
		Msg_Err("goto の行き先ラベルがおかしい");
		return;
	}
	Sym_GetChkEol();
	if (t == I_GOTO) {
		oJmpFlg = 1;
		OutJmpNo(Sym_sp->l.ofs,long_f);
	} else
		Out_NmSt("call",GoLbl_Strs(Sym_sp->l.ofs));
	return;
}

static void
StatGo(word mode)
{
	Et_t_fp ep;
	byte long_f;

	MSG("StatGo\n");
	long_f = Ch_ChkBW();
	if (Sym_Get() == I_LP || Sym_tok == I_NOT) {
		if ((ep = Expr_Cond()) == NULL)
			return;
		Sym_Get();
	} else {
		oJmpFlg = 1;
		ep = NULL;
	}
	if (Sym_tok == T_IDENT) {
		if ((Sym_sp = GoLbl_New(Sym_name)) == NULL)
			return;
	} else if (Sym_tok == T_LBL) {
		;
	} else {
		Msg_Err("goto の行き先ラベルがおかしい");
		return;
	}
	Sym_GetChkEol();
	Gen_CondExpr(ep,mode,long_f, 0, Sym_sp->l.ofs);
	return;
}

static void
StatIf(word mode)
{
	Et_t_fp ep, sav_ep;
	word t;
	word l1,l2;
	byte long_f,f;

	MSG("StatIf");
	ep = NULL;
	l2 = f = 0;
	sav_ep = Et_Sav();
	long_f = Ch_ChkBW();
	do {
		oJmpFlg = 0;
		Sym_Get();
		if ((ep = Expr_Cond()) == NULL)
			goto ENDF;
		Sym_GetChkEol();
		l1 = GoLbl_NewNo();
		Gen_CondExpr(ep,mode,long_f, 1, l1);
		while ((t = Sym_Get()) != I_ELSIF && t != I_ELSE && t != I_FI) {
			if (Stat())
				goto ENDF;
		}
		if (t != I_FI) {
			long_f = Ch_ChkBW();
			if (f == 0)
				l2 = GoLbl_NewNo();
			f = 1;
			if (oJmpFlg == 0)
				OutJmpNo(l2,1);
			Out_golbl(l1);
		}
		oJmpFlg = 0;
	} while (t == I_ELSIF);
	if (t == I_ELSE) {
		while ((t = Sym_Get()) != I_FI) {
			if (Stat())
				goto ENDF;
		}
	} else if (f)
		Out_golbl(l1);
	Out_golbl((f) ? l2 : l1);
	Sym_GetChkEol();
 ENDF:
	oJmpFlg = 0;
	Et_Frees(sav_ep);
	return;
}

static BrkCont_t *
SetExitNext(BrkCont_t *bc, word brkno,word contno)
{
	BrkCont_t *p;

	p = gStatBrkCont_p;

	gStatBrkCont_p = bc;

	if (brkno) {
		bc->lbl[0] = brkno;
		bc->lbl[1] = contno;
		bc->flg[0] = bc->flg[1] = 0;
	}
	return p;
}

static void
StatExitNext(word t, word mode)
{
	Et_t_fp ep, sav_ep;
	byte a,long_f;
	int  lblno;

	ep = NULL;
	sav_ep = Et_Sav();
	a = (t == I_EXIT) ? 0 : 1;
	MSG("StatExitNext\n");
	if (gStatBrkCont_p == NULL) {
		Msg_Err("loop,block ブロック外では exit, next は使えません");
		goto ENDF;
	}
	long_f = Ch_ChkBW();
	if (Sym_Get() == I_LP || Sym_tok == I_NOT) {
		if ((ep = Expr_Cond()) == NULL)
			goto ENDF;
		Sym_Get();
	} else {
		oJmpFlg = 1;
	}
	if (Sym_tok == T_BLKLBL) {
		Sym_sp->b.blklblp->flg[a] = 1;
		lblno = Sym_sp->b.blklblp->lbl[a];
		Sym_Get();
	} else {
		gStatBrkCont_p->flg[a] = 1;
		lblno = gStatBrkCont_p->lbl[a];
	}
	Sym_ChkEol();
	Gen_CondExpr(ep,mode,long_f, 0, lblno);
 ENDF:
	Et_Frees(sav_ep);
	return;
}

static void
StatLoop(word t, word mode)
{
	Et_t_fp ep, sav_ep;
	St_t_fp sp;
	word l1,l2,l3;
	BrkCont_t *bb;
	BrkCont_t bc;
	byte long_f;
	word tend;

	MSG("StatLoop");
	tend = (t == I_LOOP) ? I_ENDLOOP : I_ENDDO;
	l1 = GoLbl_NewNo();
	l2 = GoLbl_NewNo();
	l3 = GoLbl_NewNo();
	long_f = Ch_ChkBW();
	ep = NULL;
	sp = NULL;
	sav_ep = Et_Sav();
	bb = SetExitNext(&bc,l3,(t == I_LOOP) ? l2 : l1);
	Sym_Get();
	if (t == I_LOOP && (Sym_tok == I_LP || Sym_tok == I_NOT)) {
		if ((ep = Expr_Cond()) == NULL)
			goto ENDF;
		Sym_Get();
	}
	if (Sym_tok == T_IDENT) {
		if ((sp = Sym_NameNew(Sym_name,T_BLKLBL,Ana_mode)) == NULL)
			goto ENDF;
		sp->b.blklblp = &bc;
		Sym_Get();
	}
	Sym_ChkEol();
	if (ep) {
		bc.flg[1] = 1;
		OutJmpNo(l2,long_f);
	}
	Out_golbl(l1);
	while (Sym_Get() != tend) {
		if (Stat())
			goto ENDF;
	}
	Sym_Get();
	if (t == I_LOOP && ep == NULL && (Sym_tok == I_LP || Sym_tok == I_NOT)) {
		if ((ep = Expr_Cond()) == NULL)
			goto ENDF;
		Sym_Get();
	}
	Sym_ChkEol();
	if (bc.flg[1])
		Out_golbl(l2);
	if (t == I_LOOP)
		Gen_CondExpr(ep,mode,long_f, 0, l1);
	if (bc.flg[0])
		Out_golbl(l3);
 ENDF:
	oJmpFlg = 0;
	if (sp)
		sp->v.tok = I_ERR;
	Et_Frees(sav_ep);
	SetExitNext(bb,0,0);
	return;
}

static void
StatCallSt(St_t_fp sp,int farflg)
{
	if (farflg == 2 || (farflg == 1 && (sp->v.flg2 & FL_FAR)))
		Out_NmSt(gCallf[Opt_r86], Str_StName(sp));
	else
		Out_NmSt("call", Str_StName(sp));
}

static byte
ChChkBWD(void)
{
	word c;
	byte f;

	f = 1;
	if (Ch_ChkPeriod()) {
		if ((c = Ch_Get()) == 'w')
			f = 1;
		else if (c == 'd')
			f = 2;
		else if (c != 'b')
			Msg_Err(".b .w .d の指定がおかしい");
	}
	return f;
}

static void
StatJmp(word t,word mode)
{
	Et_t_fp ep;
	int  farflg;

	MSG("StatJmp\n");
	farflg = ChChkBWD();
	if (t == I_JMP) {
		oJmpFlg = 1;
	} else if (farflg == 0 && t == I_CALL)
		goto ERR;
	Sym_Get();
	if (Sym_tok == T_PROC || Sym_tok == T_PROC_DECL || Sym_tok == T_JMPLBL) {
		if (t == I_CALL)
			StatCallSt(Sym_sp,farflg);
		else {
			if (farflg == 0)
				Out_NmSt(gJmps[Opt_r86], Str_StName(Sym_sp));
			else if (farflg == 2 || (farflg == 1 && (Sym_sp->v.flg2 & 0x02/*FL_FAR*/)))
				Out_NmSt(gJmpf[Opt_r86], Str_StName(Sym_sp));
			else
				Out_NmSt("jmp", Str_StName(Sym_sp));
		}
		Sym_Get();
	} else {
		if ((ep = Expr(mode)) == NULL)
			goto ENDF;
		if (IsEp_Cnst(ep)) {
			if (farflg != 2)
				Msg_Err("jmp の定数指定は jmp.d としたときのみ可");
			Out_NmSt("db","0EAh");
			Out_Nem1(I_WORD,Ev_Cnst(RdEp_Val(ep) & 0xffff));
			Out_Nem1(I_WORD,Ev_Cnst((unsigned long)RdEp_Val(ep) / 0x10000L));
		} else if (IsEp_Op(ep,I_W2D)) {
			if (farflg == 0)
				goto ERR;
			if (!IsEp_W2DCnst(ep))
				goto ERR;
			Out_NmSt("db","0EAh");
			Out_Nem1(I_WORD,RdEp_Rig(ep));
			Out_Nem1(I_WORD,RdEp_Lef(ep));
		} else {
			Out_Nem1(t - 1 + farflg, ep);
		}
	}
 ENDF:
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("jmp または call の指定がおかしい");
}

/*------------------------------------------------------------------------*/
#if 0
static Et_t_fp oPush_ep[20];
static int  oPush_n;

static void
StatPush(word mode)
{
	/*Et_t_fp sav_ep;*/
	int  n;

	/*sav_ep = Et_Sav();*/
	n = oPush_n = 0;
	do {
		if (n == 20) {
			Msg_Err("push のパラメータが多すぎる");
			break;
		}
		Sym_Get();
		oPush_ep[n++] = Expr(mode);
	} while (Sym_tok == I_COMMA);
	Sym_ChkEol();
	if (n == 0)
		Msg_Err("push のパラメータがない");
	oPush_n = n;
	Gen_condMode = 0;
	while (--n >= 0)
		Gen_PushPop(I_PUSH,oPush_ep[n]);
	/*Et_Frees(sav_ep);*/
}

static void
StatPop(word mode)
{
	Et_t_fp sav_ep;
	int 	n;

	Gen_condMode = 0;
	if (Sym_Get() != I_CR && Sym_tok != I_SMCLN) {
		for (;;) {
			sav_ep = Et_Sav();
			Gen_PushPop(I_POP,Expr(mode));
			Et_Frees(sav_ep);
			if (Sym_tok != I_COMMA)
				break;
			Sym_Get();
		}
	} else if (oPush_n) {
		for (n = 0; n < oPush_n; n++ )
			Gen_PushPop(I_POP,oPush_ep[n]);
	} else {
		goto ERR;
	}
	Sym_ChkEol();
	oPush_n = 0;
	return;
 ERR:
	oPush_n = 0;
	Msg_Err("pop の指定がおかしい");
	Sym_SkipCR();
}

#endif

/*---------------*/
void ArgTypeErr()
{
	Msg_Err("手続呼出で引数のエラー");
}

void
StatProc(word t)
{
	int  n, ofs, ofs2;
	St_t_fp sp;
	Et_t_fp ep, sav_ep,lep,cxp;
	#define EPL_MAX 26
	Et_t_fp epl[EPL_MAX];
	byte parflg;

	MSG("関数呼び出し");
	n = 0;
	sp = Sym_sp;
	lep = sp->p.et;
	sav_ep = Et_Sav();
	parflg = Sym_GetChkLP();
	if (!(parflg && Sym_tok==I_RP) && Sym_tok != I_CR && Sym_tok != I_SMCLN){
		for (;;) {
			ep = Expr(0);
			if (ep == NULL || lep == NULL)
				goto ERR;
			if (!(IsEp_Rht12(ep)||IsEp_Rht4(ep)||IsEp_Seg2(ep)||IsEp_Seg4(ep)))
				ArgTypeErr();
			if (n == EPL_MAX) {
				Msg_Err("手続きの引き数がおおすぎる");
				goto ERR;
			}
			epl[n++] = ep;
			if (Sym_tok != I_COMMA)
				break;
			Sym_Get();
		}
	}
	if (parflg) {
		Sym_ChkRP();
		Sym_Get();
	}
	Sym_ChkEol();
	lep = sp->p.et;
	cxp = NULL;
	if (lep && IsEp_Op(lep,I_PPP)) {
		cxp = lep->e.rep;	/* あやしいぞ!! */
		lep = lep->e.lep;
		if (n < sp->p.argc)
			goto JJ2;
	} else if (n != sp->p.argc) {
 JJ2:
		Msg_Err("引数の数があわない");
	}
	ofs2 = ofs = 0;
  //printf("argc %d\n",sp->p.argc);
	while (--n >= 0) {
		Gen_condMode = 0;
		ep = epl[n];
  	//  printf("引数 %d / lep = %Fp  / ep = %Fp\n",n,lep,ep);
		if (n >= sp->p.argc) {
  	//	  printf("vvv\n");
	
			if (IsEp_Lft2(ep) || IsEp_Seg2(ep)||IsEp_Cnsts(ep))
				ofs += 2;
			else if (IsEp_Rht4(ep)||IsEp_Seg4(ep))
				ofs += 4;
			else
				goto ERR;
			Gen_PushPop(I_PUSH,ep);
			ofs2 = ofs;
			continue;
		}
		if (lep == NULL) {
			goto ERR;
		} else if (IsEp_Mem2(lep)) {
			if (!IsEp_Lft2(ep) && !IsEp_Seg2(ep) && !IsEp_Cnsts(ep))
				ArgTypeErr();
			ofs += 2;
			Gen_PushPop(I_PUSH,ep);

		} else if (IsEp_Mem4(lep)) {
			if (IsEp_Lft2(ep) || IsEp_Seg2(ep)) {
				Gen_ChkEquFlg();
				Gen_PushPop(I_PUSH, EV_ZERO);
			} else if (!IsEp_Rht4(ep)&&!IsEp_Seg4(ep))
				ArgTypeErr();
			ofs += 4;
			Gen_PushPop(I_PUSH,ep);

		} else if (IsEp_Reg2(lep)||IsEp_Seg2(lep)) {
			if (!(IsEp_Rht12(ep)||IsEp_Seg2(ep)||IsEp_Cnsts(ep)))
				ArgTypeErr();
			Gen_Equ(lep,ep);
			if (Opt_auto == 0) {
				if ((IsEp_Reg2(ep)||IsEp_Seg2(ep)) && ep->c.reg == lep->c.reg)
					break;
				goto EE;
			}

		} else if (IsEp_Reg1(lep)) {
			if (!(IsEp_Rht1(ep)||IsEp_Cnsts(ep)))
				ArgTypeErr();
			Gen_Equ(lep,ep);
			if (Opt_auto == 0) {
				if (IsEp_Reg1(ep) && ep->c.reg == lep->c.reg)
					break;
				goto EE;
			}
		} else if (IsEp_Reg4(lep)||IsEp_Seg4(lep)) {
			if (!(IsEp_Rht4(ep)||IsEp_Rht12(ep)||IsEp_Seg2(ep)||IsEp_Seg4(ep)))
				ArgTypeErr();
			Gen_Equ4(lep,ep);
			if (Opt_auto == 0) {
				if ((IsEp_Reg4(ep)||IsEp_Seg4(ep)) && ep->c.reg == lep->c.reg)
					break;
  EE:
				Msg_Err("手続きまたはrep命令の引数で暗黙の代入が行われた");
			}
		} else {
			goto ERR;
		}
		if ((lep = lep->e.lep) == NULL && n) {
			Msg_Err("PRGERR:Stat_Proc");
			goto ERR;
		}
	}
	if (cxp)
		Gen_Equ(Ev_Reg(cxp->c.reg), Ev_Cnst(ofs2 >> (cxp->m.typ == 2)));
	cxp = sp->p.et2->e.lep;
	while (cxp && cxp->e.rep) {
		Gen_Expr(cxp->e.rep,0);
		cxp = cxp->e.lep;
	}
	if (t != T_MACPROC)
		StatCallSt(sp,1);
	if (ofs && ((sp->p.flg2 & 0x04/*FL_CPROC*/) || ofs2)) {
		Out_Nem2(I_ADDEQ,Ev_Reg(I_SP),Ev_Cnst(
			(sp->p.flg2 & 0x04/*FL_CPROC*/) ? ofs : ofs2) );
	}
 ENDF:
	Et_Frees(sav_ep);
	return;
 ERR:
	Msg_Err("手続呼出でエラー");
	Sym_SkipCR();
	goto ENDF;
}


void
StatAssume(void)
{
	byte segs[4][3] = {"cs","ds","es","ss"};
	word seg;
	byte_fp s;

	if (Opt_r86)
		goto END2;
	fprintf(Out_file,"\tassume\t");
	for (;;) {
		if (Sym_Get() == T_SEG2) {
			seg = Sym_reg;
			if (Sym_Get() == I_CLN) {
				if (Sym_Get() == I_NOTHING)
					s = "NOTHING";
				else if (Sym_tok == I_VAR)
					s = Mod_grps[1];
				else if (Sym_tok == I_CVAR)
					s = Mod_grps[0];
				else if (Sym_tok == T_SEG)
					s = Str_StName(Sym_sp);
				else
					goto ERR;
			} else
				goto ERR;
			fprintf(Out_file,"%s:%s",segs[seg - I_CS],s);
		} else if (Sym_tok == I_NOTHING) {
			fprintf(Out_file,"NOTHING");
		} else
			goto ERR;
		if (Sym_Get() != I_COMMA)
			break;
		fputc(',',Out_file);
	}
	fputc('\n',Out_file);
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("assume の指定がおかしい");
 END2:
	Sym_SkipCR();
	return;
}

/*------------------------------------------------------------------------*/
int
Stat(void)
{
	word t,mode;
	Et_t_fp sav_ep;
	Et_t_fp p;

	oJmpFlg = 0;
	if (Sym_tok == I_OR||Sym_tok == I_ORIG) {
		mode = 2;
		Sym_Get();
	} else
		mode = 0;
	if (Sym_tok == I_LOCK) {
		Out_Nem0(Sym_tok);
		mode = 2;
		/*if (mode != 2) Msg_Err("必要な'|'がない");*/
		Sym_Get();
	}
	if (Sym_tok >= I_SEG_ES && Sym_tok <= I_SEG_DS) {
		Out_Nem0(Sym_tok);
		mode = 2;
		Sym_Get();
	}
	if (Ana_err)
		goto ERR;
	switch (t = Sym_tok) {
	case I_ERR:
		Sym_SkipCR();
		break;
	case I_EOF:
		Ana_err = ANAERR_EOF;
		break;
	case I_CR:
	case I_SMCLN:
		break;
	case I_IF:
		StatIf(mode);
		break;
	case I_LOOP:
	case I_DO:
		StatLoop(Sym_tok,mode);
		break;
	case I_EXIT:
	case I_NEXT:
		StatExitNext(Sym_tok,mode);
		break;
	case I_RETURN:
		oJmpFlg = 1;
		Decl_Return();
		break;
	case T_LBL:
	case T_IDENT:
		StatLbl(t);
		break;
	case I_GO:
		StatGo(mode);
		break;
	case I_GOTO:
	case I_GOSUB:
		StatGoto(t);
		break;
  #if 0
	case I_POP:
		StatPop(mode);
		break;
	case I_PUSH:
		StatPush(mode);
		break;
  #endif
	case I_REP:
	case I_REPE:
	case I_REPN:
		Gen_Rep(t);
		Sym_ChkEol();
		break;
	case I_JMP:
	case I_CALL:
		StatJmp(t,mode);
		break;
	case T_INTRPROC:
	case T_PROC:
	case T_MACPROC:
	case T_PROC_DECL:
		StatProc(t);
		break;
	case I_DATA:
		Decl_Data(0);
		break;
	case I_ASSUME:
		StatAssume();
		break;
	case I_ENDMODULE:
		Ana_err = ANAERR_EOF;
		goto JJJJ;
	case I_EVEN:
		Sym_GetChkEol();
		Ana_align = 2;
		Decl_Align();
		break;
	case I_ENDP:
		Ana_err = ANAERR_ENDP;
	case I_PROC:
	/*case I_GLOBAL:*/
	case I_MODULE:
	case I_FI:
	case I_ENDLOOP:
	case I_ENDDO:
	case I_ENDSTRUCT:
	case I_ATAD:
	/*case I_CPROC:*/
	JJJJ:
		{
			byte buf[40];
			sprintf(buf,"%s の現れる位置がおかしい",Sym_name);
			Msg_Err(buf);
		}
		if (Sym_tok == I_PROC) {
			while (Ana_err == 0 && Sym_Get() != I_ENDP)
				;
		}
		break;
	case I_DIV:/* '/' */
		{
			int c;
			do {
				c = Ch_Get();
				fputc(c,Out_file);
			} while (c != '\n' && Ana_err == 0);
		}
		break;
	default:
		if (t == I_RET || t == I_RETF)
			oJmpFlg = 1;
		sav_ep = Et_Sav();
		p = Expr_Stat(mode);
		Gen_Expr(p,mode);
		MSGF(("(p=%lx, sav_ep=%lx, %lx)\n",p,sav_ep,Et_Sav()));
		Et_Frees(sav_ep);
		Sym_ChkEol();
	}
 ERR:
	return Ana_err;
}
