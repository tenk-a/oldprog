#include <string.h>
#include <stdlib.h>
#include "anadef.h"

static byte oFarProcFlg,oProcEnter,oCProcFlg;
static int  oLocalOfs,oProcArgCnt,oProcArgSiz;
static word oInR,oOutR,oBreakR,oSaveReg,oAutoReg_n;
#define AUTOREG_MAX  20
static St_t_fp oAutoReg[AUTOREG_MAX+2];
enum {SV_PUSHA,SV_AX,SV_CX,SV_DX,SV_BX,SV_SI,SV_DI,SV_DS,SV_ES,SV_FX,SV_MAX};
enum {SS_PUSHA=0x01,SS_AX=0x02,SS_CX=0x04,SS_DX=0x08,SS_BX=0x10,
	  SS_SI=0x20,SS_DI=0x40,SS_DS=0x80,SS_ES=0x100,SS_FX=0x200};
static St_t_fp oSaveRegLbl[SV_MAX];
static word oSaveRegTbl[] = {
	I_PUSHA,I_AX,I_CX,I_DX,I_BX,I_SI,I_DI,I_DS,I_ES,I_FX
};
/*---------------------------------------------------------------------------*/
#define Sym_CrModeClr() 		(Sym_crMode = 0)
#if 0
#define Sym_GetRsvOff_CrOff()	(Sym_crMode = 1,Sym_GetRsvOff(),Sym_crMode = 0)
#else
#define Sym_GetRsvOff_CrOff()	(Sym_GetRsvOff())
#endif

void
Decl_Const(void)
{
	long val;
	St_t_fp sp;

	MSG("Decl_Const");
	val = 0;
	do {
		Sym_GetRsvOff_CrOff();
		if (Sym_tok != T_IDENT)
			goto ERR;
		if ((sp = Sym_NameNew(Sym_name,T_CONST,Ana_mode)) == NULL)
			goto ERR2;
		if (Ana_mode != MD_PROC)
			sp->v.grp = Mod_sp;
		if (Sym_Get() == I_EQU) {
			Sym_Get();
			sp->c.val = Expr_Cnstnt();
			val = sp->c.val + 1;
		} else {
			sp->c.val = val++;
		}
		sp->c.l_flg = 0;
	} while (Sym_tok == I_COMMA);
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("const定義がおかしい");
 ERR2:
	Sym_SkipCR();
}

static word
GetType(St_t_fp sp)
{
	Et_t_fp ep,sav_ep,xp;
	word t;
	int  i;
	int  nn[10];

	MSG("GetType");
	t = Sym_tok;
 #if 1
	if (t == T_STRUCT) {
		Et_NEWp(xp);
		xp->e.op = T_STRUCT;
		xp->v.st = Sym_sp->v.st;
		sp->v.siz = xp->m.siz = Sym_sp->v.siz;
		xp->m.typ = (byte)xp->m.siz;
		sp->p.et = xp;
	} else
 #endif
 #ifdef MVAR
	if (t == T_MODULE) {
		sp->v.siz = Sym_sp->g.mvarSize;
		if (sp->v.siz == 0)
			goto ERR;
		sp->v.st = Sym_sp;
	} else
 #endif
 #if 0
	if (t == I_DOLL)
		if ((t = Sym_Get()) == T_VAR || t == T_LOCAL)
			goto J1;
	else
 #endif
	if (t == I_TYPEOF) {
		Sym_Get();
		if (Sym_ChkLP())
			goto ERR;
		if ((t = Sym_Get()) != T_VAR && t != T_LOCAL) {
			goto ERR;
		}
		xp = (void far *)Sym_sp;
		Sym_Get();
		if (Sym_ChkRP())
			goto ERR;
		Sym_sp = (void far *)xp;
		goto J1;
	} else if (t == T_TYPE) {
		if (Sym_sp->p.et) {
 J1:
			sp->v.siz = Sym_sp->v.siz;
			sp->p.et = Sym_sp->p.et;
		} else {
			Msg_PrgErr("GetType()");
			goto ERR;
		}
	} else if ((t == I_WORD || t == I_BYTE || t == I_DWORD)) {
		sp->v.siz = t;
		sp->v.st = NULL;
	} else {
		goto ERR;
	}

	if (Sym_GetChkLP()) {
		i = 0;
		nn[0] = 0;
		if (Sym_tok == I_RP) {
			i = 1;
			goto J2;
		}
		for (;;) {
			sav_ep = Et_Sav();
			ep = Expr(0);
			if (i >= 10 || ep == NULL || !IsEp_Op(ep,T_CNST)
				|| ep->c.val < 0 || ep->c.val > 0xffff) {
				Et_Frees(sav_ep);
				Msg_Err("配列宣言の添字がおかしい");
				goto ERR;
			}
			nn[i++] = (int)ep->c.val;
			Et_Frees(sav_ep);
			if (Sym_tok != I_COMMA)
				break;
			Sym_Get();
		}
  J2:
		Sym_ChkRP();
		Sym_Get();
		while (--i >= 0) {
			Et_NEWp(xp);
			xp->e.op = T_ARRAY;
			xp->m.ofs = nn[i];
			xp->m.siz = sp->v.siz;
			xp->m.typ = (byte)xp->m.siz;
			xp->e.lep = sp->p.et;
			sp->p.et = xp;
			sp->v.siz = xp->m.siz * nn[i];
			MSGF(("(ary %d = %d * %d)\n",sp->v.siz,xp->m.siz,xp->m.ofs));
		}
	}
 ENDF:
	/* if (sp->v.siz == 0)  Msg_Err("PRG:型ｻｲｽﾞの処理がおかしい");*/
	return sp->v.siz;

 ERR:
	Msg_Err("型指定がおかしい");
	Sym_SkipCR();
	return 0;
}

void
Decl_Struct(void)
{
	word t,ofs,ofsB,ofsT;
	/* Et_t_fp xp; */
	St_t_fp sp,mp;
	St_t_fp root;
	word sav_mode;

	MSG("Decl_Struct");
	Sym_CrModeClr();
	if (Sym_GetRsvOff() == T_IDENT) {
		if ((sp = Sym_NameNew(Sym_name,T_STRUCT,Ana_mode)) == NULL)
			goto ENDF;
		if (Ana_mode != MD_PROC)
			sp->v.grp = Mod_sp;
	} else
		goto ERR;
	ofs = ofsT = ofsB = 0;
	sav_mode = Ana_mode;
	root = NULL;
	Ana_mode = MD_STRUCT;
	Sym_GetChkEol();
	for (;;) {
		Sym_crMode = 1; Sym_GetRsvOff(); Sym_crMode = 0;
		if (Sym_tok == I_DEC) {
			if (ofs > ofsB)
				ofsB = ofs;
			ofs = ofsT;
			Sym_crMode = 1; Sym_GetRsvOff(); Sym_crMode = 0;
		} else if (Sym_tok == I_INC) {
			if (ofs < ofsB)
				ofs = ofsB;
			ofsT = ofsB = ofs;
			Sym_crMode = 1; Sym_GetRsvOff(); Sym_crMode = 0;
		}
		if (Sym_tok == T_IDENT) {
			if (strcmp(Sym_name, "endstruct") == 0) {
				Sym_GetChkEol();
				break;
			}
			if ((mp = St_Ins(Sym_name, &root)) == NULL)
				goto ENDF;
			if (Sym_Get() != I_CLN)
				goto ERR;
			mp->v.tok = T_MEMB;
			mp->v.seg = 0;
			Sym_Get();
			t = GetType(mp);
			/*if (t == 0)
				goto ERR;*/
		  #if 0
			if (t > I_BYTE && (ofs & 0x01))
				ofs++;
		  #endif
			mp->v.ofs = ofs;
			ofs += t;
		} else
			goto ERR;
		if (Sym_tok != I_COMMA)
			Sym_ChkEol();
	}
	if (root == NULL)
		goto ERR;
	if (ofs > ofsB)
		ofsB = ofs;
	sp->v.siz = ofsB;
	sp->v.st = root;
	Ana_mode = sav_mode;
 ENDF:
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("構造体定義がおかしい");
	Sym_SkipCR();
	return;
}

void
Decl_Type(void)
{
	St_t_fp sp;

	MSG("Decl_Type");
	do {
		Sym_GetRsvOff_CrOff();
		if (Sym_tok == T_IDENT) {
			if ((sp = Sym_NameNew(Sym_name,T_TYPE,Ana_mode)) == NULL)
				goto ENDF2;
		} else
			goto ERR;
		if (Sym_Get() != I_CLN)
			goto ERR;
		Sym_Get();
		if (Sym_tok == T_MODULE) {
			sp->v.tok = T_MODULE;
			sp->v.grp = Sym_sp->v.grp;
			sp->v.st = Sym_sp->v.st;
			Sym_Get();
		} else {
			if (GetType(sp) == 0)
				goto ERR;
			if (sp->p.et == NULL) {
				if (sp->v.siz == 1 || sp->v.siz == 2 || sp->v.siz == 4)
					sp->v.tok = sp->v.siz;
			}
			if (Ana_mode != MD_PROC)
				sp->v.grp = Mod_sp;
		}
	} while (Sym_tok == I_COMMA);
 ENDF:
	Sym_ChkEol();
 ENDF2:
	return;
 ERR:
	Msg_Err("型名がおかしい");
	Sym_SkipCR();
	return;
}

/*------------*/

static int
Out_WordStr(byte far *str, word l)
{
	word cnt,c;
	int i;

	MSG("Out_WordStr");
	cnt = i = 0;
	if (l) {
		while (l-- > 0) {
			c = *str++;
			if (l > 0 && iskanji(c)) {
				c = c * 0x100 + *str++;
				--l;
			}
			if (i == 0) {
				fprintf(Out_file,"\tdw\t%d",c);
			} else
				fprintf(Out_file,",%d",c);
			if (++i == 16) {
				fprintf(Out_file,"\n");
				i = 0;
			}
			++cnt;
		}
		if (i)
			fprintf(Out_file,"\n");
	}
	return cnt;
}

static int
Out_ByteStr(byte far *str, word cnt)
{
	word l;
	int i;

	MSG("Out_ByteStr");
	if (cnt) {
		l = cnt;
		i = 0;
		while (l-- > 0) {
			if (i == 0) {
				fprintf(Out_file,"\tdb\t%d",*str++);
			} else
				fprintf(Out_file,",%d",*str++);
			if (++i == 16) {
				fprintf(Out_file,"\n");
				i = 0;
			}
		}
		if (i)
			fprintf(Out_file,"\n");
	}
	return cnt;
}

static int
InitOne(word t, long len, int  f)
	/* 要素サイズt=1,2,4 で len 個の配列を初期化. len が０のときは0x7fffffff
	 * f = 1:{}外での初期化. ','で続けない. 1つ'\0'を付加
	 * f = 0:','があれば続ける。要素数の残りを'\0'で埋める。
	 * f =-1:','があれば続ける。要素数の残りを'\0'で埋めない。
	 */
{
	long n,mx;
	Et_t_fp p;

	MSG("InitOne");
	if (!(t == I_BYTE || t == I_WORD || t == I_DWORD)) {
		Msg_PrgErr("変数初期化");
		goto ERR;
	}
	if (len == 0) {
		mx = 0x7fffffffL;
	} else
		mx = len;
	MSGF(("typ=%d  len=%ld  mx=%ld\n",t,len,mx));
	n = 0;
	for (;;) {
		if (Sym_tok == T_STRING) {
			if (Decl_defFlg) {
				if (t == I_WORD)
					n += Out_WordStr(Sym_str,Sym_strLen);
				else {
					n += Out_ByteStr(Sym_str,Sym_strLen);
					if (t != I_BYTE)
						Msg_Err("文字列の型が一致しない");
				}
			}
			Sym_Get();
		} else {
			if ((p = Expr(0)) == NULL)
				goto ERR;
			if (!IsEp_Cnsts(p))
				goto ERR;
			if (Decl_defFlg)
				Out_Nem1(t,p);
			++n;
		}
		if (n > mx) {
			Msg_Err("初期値が多すぎる");
			goto ENDF;
		}
		if (Sym_tok != I_COMMA || f > 0)
			break;
		Sym_Get();
	}
	if (f >= 0 && len > n) {
		/*if (Decl_defFlg)*/
			Out_Dup(0,t * (len - n));
		n = len;
	} else if (f > 0 && len == 0) {
		Out_Dup(0,t);
		++n;
	}
 ENDF:
	MSGF(("typ=%d  len=%ld  n=%ld\n",t,len,n));
	return (int)n;
 ERR:
	Msg_Err("変数の初期化がおかしい");
	return (int)n;
}

static void
SkipSym(word t)
{
	while (Ana_err == 0 && Sym_Get() != t)
		;
	/*  Msg_Err("括弧の数が合っていないかもしれない"); */
}

int
Decl_Data(word cnt)
{
	int  f;
	long n,l,val;
	word sav_crMode,t,cnt2;
	word c;

	MSG("Decl_Data");
	MSGF(("cnt=%d\n",cnt));
	cnt2 = (cnt) ? cnt : 0x7fff;
	sav_crMode = Sym_crMode;
	Sym_crMode = 0;
	l = 0;
	Sym_GetChkEol();
	t = Sym_Get();
	while (t != I_ATAD) {
		if (Sym_tok == I_CR || Sym_tok == I_SMCLN) {
			t = Sym_Get();
			continue;
		} else if (t == I_BYTE || t == I_WORD || t == I_DWORD) {
			if ((c = Ch_GetK()) == '(') {
				Sym_Get();
				if ((val = Expr_Cnstnt()) == ERR_VAL||val <= 0||val >= cnt2)
					goto ENDF;
				if (Sym_ChkRP())
					goto ERR;
				f = 0;
			} else {
				Ch_Unget(c);
				val = cnt / t;
				f = -1;
			}
			Sym_Get();
			if (t == I_BYTE) {
				n = InitOne(I_BYTE,val,f);
				l += n;
			} else if (t == I_WORD) {
				n = InitOne(I_WORD,val,f);
				l += n * 2;
			} else if (t == I_DWORD) {
				n = InitOne(I_DWORD,val,f);
				l += n * 4;
			}
			t = Sym_tok;
		} else {
 ERR:
			Msg_Err("data～enddataの間に余分なものがある");
			SkipSym(I_ATAD);
			goto ENDF;
		}
	}
	MSGF(("l=%d\n",l));
	if (cnt) {
		if (l < cnt) {
			/*if (Decl_defFlg)*/
			Out_Dup(0,cnt-l);
		} else if (l > cnt) {
			Msg_Err("data～enddata中の初期値が多すぎる");
		}
		l = cnt;
	}
 ENDF:
	Sym_crMode = sav_crMode;
	Sym_Get();
	return (int)l;
}

static void
VarInitVal(Et_t_fp tp,word typ)
{
	long n;
	word cnt;

	MSG("VarInitVal");
	if (tp == NULL) {
		InitOne(typ,1,1);
	} else if (Sym_tok == T_CNST && Sym_val == 0) {

	} else if (Sym_tok == T_STRING) {
		MSGF(("strlen=%d,ofs=%d\n",Sym_strLen,tp->m.ofs));
		if (tp->e.op==T_ARRAY && (tp->m.typ == I_BYTE || tp->m.typ == I_WORD)
			&& (tp->m.ofs == 0 || Sym_strLen < tp->m.ofs) ) {
			n = InitOne(tp->m.typ, tp->m.ofs, 1);
			if (tp->m.ofs == 0)
				tp->m.ofs = (int)n;
		} else
			goto ERR;
	} else if (Sym_tok == I_LC) {
		if (tp->e.op != T_ARRAY)
			goto ERR;
		Sym_crMode++; /* PAR_CR_INC();*/
		Sym_Get();
		if (tp->e.lep == NULL) {
			n = InitOne(tp->m.typ, tp->m.ofs, 0);
			if (tp->m.ofs == 0)
				tp->m.ofs = (int)n;
		} else {
			cnt = (word)tp->m.ofs;
			if (cnt == 0)
				cnt = 0x7fffffffL;
			n = 0;
			do {
				Sym_Get();
				VarInitVal(tp->e.lep,0);
				if (++n == cnt)
					break;
			} while (Sym_Get() == I_COMMA);
			if (tp->m.ofs == 0)
				tp->m.ofs = (int)n;
		}
		Sym_crMode--; /* PAR_CR_DEC();*/
		if (Sym_tok != I_RC)
			SkipSym(I_RC);
		Sym_Get();
	} else if (Sym_tok == I_DATA) {
		if (tp->e.op == T_STRUCT)
			n = Decl_Data(tp->m.siz);
		else if (tp->e.op == T_ARRAY)
			n = Decl_Data(tp->m.siz * tp->m.ofs);
		else {
			goto ERR2;
		}
		if (tp->m.ofs == 0) {
			if (tp->m.siz && ((n % tp->m.siz) == 0))
				tp->m.ofs = (int)(n / tp->m.siz);
			else
				Msg_PrgErr("data命令\n");
		}
	} else {
		goto ERR;
	}
	return;
 ERR:
	Msg_Err("初期値がおかしい");
	return;
 ERR2:
	Msg_PrgErr("構造体・配列のﾘｽﾄがおかしい");
}

void
Decl_Align(void)
{
	if (Ana_align) {
		if (Ana_align == 2)
			Out_Nm("even");
		else
			fprintf(Out_file,"\talign\t%d\n",Ana_align);
		Ana_align = 0;
	}
}

void
OutSegLbl(St_t_fp cp, St_t_fp sp, word t)
{
	if (Decl_defFlg) {
		Out_Seg(cp);
		Decl_Align();
		if (sp->p.et && sp->p.et->e.op == T_ARRAY)
			Out_LblTyp(sp,sp->p.et->m.siz);
		else
			Out_LblTyp(sp,t);
	}
}

void
Decl_Var(St_t_fp cp)
{
	word t;
	int  lblno;
	St_t_fp sp;

	MSG("Decl_Var");
	do {
		Sym_GetRsvOff_CrOff();
		if (Sym_tok == T_IDENT) {
			if ((sp = Sym_NameNew(Sym_name,T_VAR,Ana_mode)) == NULL)
				goto ENDF;
		} else
			goto ERR;
		if (Sym_Get() != I_CLN)
			goto ERR;
		Sym_Get();
	#ifdef MVAR
		if (Sym_tok == T_MODULE) {
			sp->v.tok = T_MODULEVAR;
			sp->v.grp = Sym_sp->v.grp;
			sp->v.st = Sym_sp->v.st;
		}
	#endif
		t = GetType(sp);
		if (Ana_mode == MD_MODULE) {
			lblno = 0;
			sp->v.grp = Mod_sp;
		} else if (Ana_mode == MD_EXPO) {
			sp->v.flg2 |= FL_EXPO;
			sp->v.grp = Mod_sp;
			lblno = -1;
		} else {
			lblno = GoLbl_NewNo();
		}
		if (Decl_defFlg == 0)
			sp->v.flg2 |= FL_INPO;
		sp->v.ofs = lblno;
		sp->v.seg = cp->c.seg;
		if (Sym_tok == I_EQU) {
			if (Sym_Get() == T_CNST && Sym_val == 0 && sp->v.siz > 0) {
				Sym_Get();
				OutSegLbl(cp,sp,t);
				if (Opt_zeroBSS && cp == gSeg_sp[GS_DATA])
					goto ZZ;
				Out_Dup(0,sp->v.siz);
			} else {
				OutSegLbl(cp,sp,t);
				VarInitVal(sp->p.et,t);
				if (sp->v.siz == 0)
					sp->v.siz = sp->p.et->m.siz * sp->p.et->m.ofs;
			}
		} else if (Decl_defFlg) {
			if (cp == gSeg_sp[GS_DATA])
				OutSegLbl(gSeg_sp[GS_BSS],sp,t);
			else
				OutSegLbl(cp,sp,t);
  ZZ:
			Out_Dup(-1,sp->v.siz);
		}
	} while (Sym_tok == I_COMMA);
 ENDF:
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("変数定義がおかしい");
	Sym_SkipCR();
}

#ifdef MVAR
void
Decl_Mvar(void)
{
	word t;
	St_t_fp sp;

	MSG("Decl_Mvar");
	if (Ana_mode > MD_MODULE)
		goto ERR;
	do {
		Sym_GetRsvOff_CrOff();
		if (Sym_tok == T_IDENT) {
			if ((sp = Sym_NameNew(Sym_name,T_MVAR,Ana_mode)) == NULL)
				goto ENDF;
		} else
			goto ERR;
		if (Sym_Get() != I_CLN)
			goto ERR;
		Sym_Get();
		t = GetType(sp);
		sp->v.grp = Mod_sp;
		sp->v.ofs = gMvarOfs;
		sp->v.seg = 0;
		gMvarOfs += t;
		if (Ana_mode == MD_EXPO) {
			sp->v.flg2 |= FL_EXPO;
		}
		if (Decl_defFlg == 0)
			sp->v.flg2 |= FL_INPO;
	} while (Sym_tok == I_COMMA);
 ENDF:
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("mvar 定義がおかしい");
	Sym_SkipCR();
}
#endif

/*---------------------------------------------------------------------------*/
void
OutStrl(void)
{
	Sss_t *s,*bs;

	s = Expr_strlTop;
	if (s == NULL)
		return;
	Out_Seg(gSeg_sp[GS_DATA]);
	do {
		if (Opt_r86) {
			fprintf(Out_file,"%s:\n",GoLbl_Strs(s->no));
		} else {
			fprintf(Out_file,"%s label byte\n",GoLbl_Strs(s->no));
		}
		Out_ByteStr(s->str,strlen(s->str)+1);
		bs = s;
		s = s->next;
		if (bs->str)
			free(bs->str);
		free(bs);
	} while (s);
	Expr_strl = NULL;
	Expr_strlTop = NULL;
}

/*---------------------------------------------------------------------------*/
static void
SaveRegPush(void)
{
	int  i,j;
	word n;

	MSGF(("SaveRegPush %x\n",oSaveReg));
	for (n = 1, i = 0; i < SV_MAX; n <<= 1, i++ ) {
		if (oSaveReg & n) {
			if (i == SV_PUSHA) {
				Gen_Pusha();
				for (j = SV_AX; j <= SV_DI; j++) {
					if (j == SV_SI)
						Ana_saveOfs += 2*3;
					else
						Ana_saveOfs += 2;
					if (oSaveRegLbl[j])
						oSaveRegLbl[j]->v.ofs = -Ana_saveOfs;
				}
			} else {
				if (i == SV_FX)
					Out_Nem0(I_PUSHF);
				else
					Out_Nem1(I_PUSH,Ev_Reg(oSaveRegTbl[i]));
				Ana_saveOfs += 2;
				if (oSaveRegLbl[i])
					oSaveRegLbl[i]->v.ofs = -Ana_saveOfs;
			}
		}
	}
}

static void
SaveRegPop(void)
{
	word n;
	int  i;

	MSGF(("SaveRegPop %x\n",oSaveReg));
	if (oSaveReg & SS_FX)
		Out_Nem0(I_POPF);
	for (n = SS_ES, i = SV_ES; i >= 0; n >>= 1, i-- ) {
		if (oSaveReg & n) {
			if (i == SV_PUSHA)
				Gen_Popa();
			else
				Out_Nem1(I_POP,Ev_Reg(oSaveRegTbl[i]));
		}
	}
}

void
Decl_Load(void)
{
	SaveRegPop();
}

void
Decl_Return(void)
{
	Et_t_fp ep;

	if (Ana_mode != MD_PROC) {
		Msg_Err("returnは proc～endproc 手続き内でしか使えない");
		return;
	}
	Sym_GetChkEol();
	if (oProcEnter)
		Out_Leave();
	SaveRegPop();
	if (oCProcFlg /* oOutR & 0x8000*/) {
		ep = NULL;
	} else {
		ep = Ev_Cnst(oProcArgSiz);
		IsEp_CnstTyp(I_WORD,ep);
		if (ep->c.val == 0)
			ep = NULL;
	}
	Out_Nem1((oFarProcFlg) ? I_RETF : I_RET, ep);
}

static void

DeclSave(void)
{
	word n;
	St_t_fp sp;

	do {
		sp = NULL;
		if (Sym_Get() == T_IDENT) {
			if (oProcEnter == 0)
				Msg_Err("proc～enter～endprocでないのに save に名札がある");
			if ((sp = Sym_NameNew(Sym_name,T_ARGLOCAL,MD_PROC)) == NULL)
				goto ENDF;
			if (Sym_Get() != I_CLN)
				goto ERR;
			sp->v.siz = I_WORD;
			sp->v.st = NULL;
			sp->v.ofs = 0;
			Sym_Get();
		}
		if (Sym_tok == T_R2 || Sym_tok == T_SEG2) {
			Sym_tok = Sym_reg;
			goto JJ1;
		} else if (Sym_tok == I_FX || (Sym_tok == I_PUSHA && sp == NULL)) {
 JJ1:
			for (n = 0; n < SV_MAX; n++ ) {
				if (Sym_tok == oSaveRegTbl[n])
					break;
			}
			if (n == SV_MAX) {
				goto JJ2;
			}
		} else {
 JJ2:
			Msg_Err("pushできないものが指定された");
			goto ERR;
		}
		if (sp)
			oSaveRegLbl[n] = sp;
		n = 1 << n;
		if (oSaveReg & n)
			Msg_Err("saveの指定でﾚｼﾞｽﾀがだぶっている");
		oSaveReg |= n;
	} while (Sym_Get() == I_COMMA);

	if (oSaveReg & SS_PUSHA) {
		if (Opt_cpu)
			oSaveReg &= SS_PUSHA|SS_FX|SS_ES|SS_DS;
		else {
			oSaveReg |= SS_AX|SS_BX|SS_CX|SS_DX|SS_DI|SS_SI;
			oSaveReg &= ~SS_PUSHA;
		}
	}
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("saveの指定がおかしい");
 ENDF:
	Sym_SkipCR();
}

static void
AutoRegPush(void)
{
	int  i;
	St_t_fp sp;

	for (i = 0; i < oAutoReg_n; i++) {
		sp = oAutoReg[i];
		Gen_PushPop(I_PUSH,Ev_Reg4(sp->v.tok));
		oLocalOfs += sp->v.siz;
		sp->v.ofs = -oLocalOfs;
		sp->v.tok = T_LOCAL;
	}
	oAutoReg_n = 0;
}

static void
DeclLocal(void)
{
	word t;
	St_t_fp sp;

	do {
		Sym_GetRsvOff_CrOff();
		if (Sym_tok != T_IDENT)
			goto ERR;
		if ((sp = Sym_NameNew(Sym_name,T_LOCAL,MD_PROC)) == NULL)
			goto ENDF;
		if (Sym_Get() != I_CLN)
			goto ERR;
		sp->v.st = NULL;
		sp->v.ofs = 0;
		sp->v.seg = (Ana_model == 2 || Ana_model == 4) ? I_SS : 0;
		Sym_Get();
		if ((t = GetType(sp)) == 0)
			goto ERR;
		if (Sym_tok != I_EQU) {
			if (t > I_BYTE && (oLocalOfs&0x01))
				oLocalOfs++;
			oLocalOfs += t;
			sp->v.ofs = -oLocalOfs;
		} else {
			Sym_Get();
			if (Sym_tok == T_R4 || Sym_tok == T_SEG4) {
				if (sp->v.siz != 4)
					goto ERR;
				goto JJ;
			}else if (Sym_tok == T_R2 || Sym_tok == T_SEG2) {
				if (sp->v.siz != 2)
					goto ERR;
	 JJ:
				if (oAutoReg_n >= AUTOREG_MAX-1) {
					Msg_Err("localでﾚｼﾞｽﾀの指定が多すぎる");
					goto ENDF;
				}
				sp->v.tok = Sym_reg;
				oAutoReg[oAutoReg_n++] = sp;
				oAutoReg[oAutoReg_n] = NULL;
				Sym_Get();
			} else
				goto ERR;
		}
	} while (Sym_tok == I_COMMA);
 ENDF:
	Sym_ChkEol();
	return;
 ERR:
	Msg_Err("ﾛｰｶﾙ変数定義がおかしい");
	Sym_SkipCR();
}

/*---------------*/
#if 0
static word
ProcRegNo(word t)
{
	word i;
	static word tbl[] = {
		I_AH,I_CH,I_DH,I_BH,I_AL,I_CL,I_DL,I_BL,I_AX,I_CX,I_DX,I_BX,
		I_DI,I_SI,I_DS,I_ES,I_FX,I_CF,I_DF,I_IIF
	};
	static word tbl2[] = {
		0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x11,0x22,0x44,0x88,
		0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000
	};
	for (i = 0;i < 16+4;i++) {
		if (t == tbl[i])
			return tbl2[i];
	}
	Msg_Err("手続きのﾍｯﾀﾞ部で指定できないﾚｼﾞｽﾀ名が指定された");
	return 0;
}
#endif
#if 0
static void
ProcRegChk(word *regflgs)
{
	word nn,t;

	switch (Sym_tok) {
	case I_FX:
	case I_CF:
	case I_DF:
	case I_IIF:
		t = Sym_tok;
		break;
	case T_R2:
	case T_R1:
	case T_SEG2:
		t = Sym_reg;
		break;
	case T_R4:
	case T_SEG4:
		nn = ProcRegNo(Sym_reg / 0x100);
		if (nn & *regflgs)
			goto ERR;
		*regflgs |= nn;
		t = Sym_reg & 0xff;
		break;
	default:
		Msg_Err("in,out,breakの指定がおかしい");
		return;
	}
	nn = ProcRegNo(t);
	if (nn & *regflgs)
		goto ERR;
	*regflgs |= nn;
	return;
 ERR:
	Msg_Err("手続きﾍｯﾀﾞ部でﾚｼﾞｽﾀ名がだぶっている");
}
#endif

static int
ChkTyp(Et_t_fp ap, word ao, Et_t_fp bp, word bo)
{
	while (ao == bo && ap != NULL && bp != NULL) {
		if (ap->e.op != T_ARRAY && ap->e.op != T_STRUCT)
			goto ERR;
		if (ap->e.op != bp->e.op || ap->m.ofs != bp->m.ofs)
			goto ERR;
		ap = ap->e.lep;
		bp = bp->e.lep;
		ao = ap->m.siz;
		bo = bp->m.siz;
	}
	if (ap == NULL && bp == NULL)
		return 0;
 ERR:
	return 1;
}

static int
ChkArg(Et_t_fp ap, Et_t_fp bp)
{
	for (;;) {
		if (ap == NULL && bp == NULL)
			return 0;
		if (ap->e.op != bp->e.op)
			goto ERR;
		switch (ap->e.op) {
		case T_R2:
		case T_R1:
		case T_R4:
		case T_SEG2:
		case T_SEG4:
			if (ap->m.typ != bp->m.typ
				|| ap->c.reg != bp->c.reg)
				goto ERR;
			break;
		case T_IDENT:
			if (ChkTyp(ap->e.rep,ap->m.siz,bp->e.rep,bp->m.siz) )
				goto ERR;
			break;
		default:
			Msg_PrgErr("引き数ﾘｽﾄ");
			goto ERR;
		}
		ap = ap->e.lep;
		bp = bp->e.lep;
	}
 ERR:
	Msg_Err("引数が最初の宣言と矛盾している");
	return 1;
}

static byte oPPPflg = 0;

static Et_t_fp
ProcArg(void)
{
	word t;
	St_t_fp sp;
	Et_t_fp ep, bp, xp, arglst;

	oPPPflg = 0;
	arglst = bp = NULL;
	oProcArgCnt = oProcArgSiz = 0;
	if (Sym_Get() == I_RP || Sym_tok == I_CR)
		goto ENDF;
	for ( ; ; ) {
		if ((t = Sym_tok) == T_R2 || t == T_R1 || t == T_R4
			|| t == T_SEG2 || t == T_SEG4){
			Et_NEWp(ep);
		  #if 0
			if (bp)
				bp->e.lep = ep;
		  #else
			ep->e.lep = bp;
		  #endif
			ep->e.op = Sym_tok;
			ep->m.typ = (t == T_R2 || t == T_SEG2) ? I_WORD :
						(t == T_R1) ? I_BYTE : I_DWORD;
			ep->c.reg = Sym_reg;
			//ProcRegChk(&oInR);
			if (Sym_Get() == I_CLN) {
				Sym_Get();
				if (t == T_R1)
					goto E1;
				if (Sym_tok == I_WORD || Sym_tok == I_DWORD)
					ep->m.typ = Sym_tok;
				else {
 E1:
					Msg_Err("ﾚｼﾞｽﾀに対する型指定がおかしい");
				}
				Sym_Get();
			}
		} else if (t == T_IDENT) {
			if ((sp = Sym_NameNew(Sym_name,T_ARGLOCAL,MD_PROC)) == NULL)
				goto ERR2;
			if (Sym_Get() != I_CLN)
				goto ERR;
			sp->v.st = NULL;
			sp->v.seg = (Ana_model == 2 || Ana_model == 4) ? I_SS : 0;
			sp->v.ofs = oProcArgSiz;
			Sym_Get();
			if ((t = GetType(sp)) == 0||(t != I_WORD && t != I_DWORD))
				goto ERR2;
			oProcArgSiz += t;
			Et_NEWp(ep);
		  #if 0
			if (bp)
				bp->e.lep = ep;
		  #else
			ep->e.lep = bp;
		  #endif
			ep->e.op = (t == I_WORD) ? T_M2: T_M4;
			ep->e.rep = sp->p.et;
			ep->m.siz = t;
 #if 1
		} else if (t == I_PPP) {
			oPPPflg = 1;
			Et_NEWp(ep);
		  #if 0
			if (bp)
				bp->e.lep = ep;
		  #else
			ep->e.lep = bp;
		  #endif
			ep->e.op = I_PPP;
			/*ep->m.typ = ep->c.reg = 0;*/
			if (Sym_Get() == T_IDENT) {
				if ((sp = Sym_NameNew(Sym_name,T_ARGLOCAL,MD_PROC)) == NULL)
					goto ERR2;
				/*sp->v.st = NULL;*/
				/*sp->v.cnt = 0;*/
				sp->v.seg = (Ana_model == 2 || Ana_model == 4) ? I_SS : 0;
				sp->v.ofs = oProcArgSiz;
				Et_NEWp(xp);
				sp->p.et = xp;
				xp->e.op = T_ARRAY;
				/*xp->e.cnt = 0;*/
				if (Sym_Get() != I_CLN)
					goto ERR;
				if (Sym_Get() == I_WORD||Sym_tok == I_BYTE)
					xp->m.siz = sp->v.siz = Sym_tok;
				else
					goto ERR;
				if (Sym_Get() != I_LP)
					goto ERR;
				if (Sym_Get() == T_R2 || Sym_tok == T_R1) {
					Et_NEWp(xp);
					ep->e.rep = xp;
					xp->e.op = Sym_tok;
					xp->c.reg = Sym_reg;
					xp->m.typ = sp->v.siz;
					//ProcRegChk(&oInR);
					Sym_Get();
				}
				if (Sym_tok != I_RP)
					goto ERR;
				Sym_Get();
			} else
				goto ERR;
 #endif
		} else {
			goto ERR;
		}
	  #if 0
		if (bp == NULL)
			arglst = ep;
	  #else
		arglst =
	  #endif
		bp = ep;
		++oProcArgCnt;
		if (Sym_tok != I_COMMA)
			break;
		if (bp->e.op == I_PPP)
			goto ERR;
		Sym_Get();
	}
	if (bp->e.op == I_PPP)
		--oProcArgCnt;
 ENDF:
	return arglst;

 ERR:
	Msg_Err("引数処理でエラー");
 ERR2:
	Sym_CrModeClr();
	Sym_SkipCR();
	return NULL;
}

static int
ProcBody(St_t_fp sp,int alain)
{
	Et_t_fp sav_ep;

	sav_ep = Et_Sav();
	while (Ana_err == 0) {
		Sym_CrModeClr();
		if (Sym_Get() == I_ENDP)
			break;//goto ENDF2;
		else if (Sym_tok == I_CONST)
			Decl_Const();
		else if (Sym_tok == I_VAR)
			Decl_Var(gSeg_sp[GS_DATA]);
		else if (Sym_tok == I_CVAR)
			Decl_Var(gSeg_sp[GS_CODE]);
		else if (Sym_tok == T_SEG)
			Decl_Var(Sym_sp);
		else if (Sym_tok == I_TYPE)
			Decl_Type();
		else if (Sym_tok == I_STRUCT)
			Decl_Struct();
  #ifdef MACR	/* Abunai zo!!!!!!!!!!!!!!!! */
		else if (Sym_tok == M_DEFINE)
			Sym_MacDef();
  #endif
		else if (Sym_tok == I_SAVE) {
			/*if (!oProcEnter)
				goto JJJ;*/
			DeclSave();
		} else if (Sym_tok == I_LOCAL) {
			if (!oProcEnter) {
 /*JJJ:*/
				Msg_Err("proc～enter～endprocでないと local は使えません");
				Sym_SkipCR();
			} else
				DeclLocal();
		} else if (Sym_tok == I_EVEN) {
			Sym_GetChkEol();
			Ana_align = 2;
		} else if (Sym_tok == I_SMCLN || Sym_tok == I_CR) {
			;
		} else
			break;
	}

	Out_Seg(gSeg_sp[GS_CODE]);
	if (alain) {
		Ana_align = alain;
		Decl_Align();
	}
	Out_LblTyp(sp,-1);  /* proc */
	if (Opt_procAssumeFlg)
		Out_DAssume();
	SaveRegPush();
	if (oProcEnter) {
		/*SaveRegPush();*/
		Ana_saveOfs += 2;
		if (oLocalOfs & 0x01)
			++oLocalOfs;
		Out_Enter(oLocalOfs,0);
		AutoRegPush();
	}
	while (Ana_err == 0) {
		Sym_CrModeClr();
		if (Sym_tok == I_ENDP) {
			Sym_GetChkEol();
			goto ENDF;
		} else if (Stat()) {
			return Ana_err;
		}
		Sym_Get();
	}
 ENDF:
	Out_LblTyp(sp,-2);  /* endproc */
	Out_Line("");
 ENDF2:
	Et_Frees(sav_ep);
	return 0;
}

int
Decl_Proc(word op)
{
	St_t_fp sp;
	Et_t_fp xp,cdp;
	Et_t_fp sav_ep;
	word t;
	byte declFlg;
	byte alain;
  #if 0
	long val;
  #endif

	MSG("Decl_Proc");
	St_localRoot = NULL;
	gStatBrkCont_p = NULL;
	cdp = NULL;
	alain = Ana_align; Ana_align = 0;
	declFlg = oLocalOfs = oProcEnter =
	oSaveReg = oProcArgSiz = oInR = oOutR = oBreakR = 0;
	oFarProcFlg = ((Ana_mode == MD_EXPO) && (Ana_model > 2));
	oCProcFlg = 0/*(op == I_CPROC)*/;
	for (t = 0;t < SV_MAX; t++)
		oSaveRegLbl[t] = NULL;

	switch (Sym_GetRsvOff2()) {
	case T_IDENT:
		if ((sp = Sym_NameNew(Sym_name,T_PROC,Ana_mode)) == NULL)
			goto ENDF;
		goto J1;
	case T_PROC_DECL:
		sp = Sym_sp;
		declFlg = 1;
		if (oCProcFlg && !(sp->v.flg2 & FL_CPROC)
			|| (Ana_mode == MD_EXPO) && !(sp->v.flg2 & FL_EXPO))
			Msg_Err("手続きの定義が最初の宣言と矛盾している");
		oCProcFlg = (sp->v.flg2 & FL_CPROC) ? 1 : 0;
		break;
	case T_STARTLBL:
		sp = Sym_sp;
		if (Ana_mode == MD_EXPO)
			sp->v.flg2 |= FL_EXPO;
		goto J1;
	default:
		Msg_Err("定義しようとした手続き名はすでに登録されている");
		if ((sp = St_New()) == NULL)
			goto ENDF;
 J1:
		Et_NEWp(xp);
		sp->p.et2 = xp;
	}
	sp->v.tok = T_PROC;
	Ana_mode = MD_PROC;
	if (Sym_Get() == I_CR) {
		while (Sym_Get() == I_CR)
			;
	}
	if (declFlg)
		sav_ep = Et_Sav();
	Sym_ChkLP();
	xp = ProcArg();
	if (declFlg == 0 || ChkArg(sp->p.et,xp)) {
		sp->p.et = xp;
		sp->p.argc = oProcArgCnt;
		sp->p.argsiz = oProcArgSiz;
	} else {
		Et_Frees(sav_ep);
	}
	Sym_ChkRP();
	Sym_GetChkEol();
	if (oFarProcFlg) {
		Ana_saveOfs = 4;
		sp->v.flg2 |= FL_FAR;
	} else {
		Ana_saveOfs = 2;
	}
	if (oCProcFlg)
		sp->v.flg2 |= FL_CPROC;
	sp->v.grp = Mod_sp;
	while (Ana_err == 0) {
		/*Sym_crMode = 1;*/
		Sym_Get();
		Sym_crMode = 0;
		if (Sym_tok == I_MACRO) {
			if (declFlg)
				goto E1;
			sp->v.tok = T_MACPROC;
			goto JJ;
		} else if (Sym_tok == I_BEFORE) {
 JJ:
			Sym_GetChkEol();
			while (Ana_err == 0) {
				Sym_Get();
				if (Sym_tok==I_BEGIN || Sym_tok==I_ENTER || Sym_tok==I_ENDP)
					break;
				Et_NEWp(xp);
				if ((xp->e.rep = Expr_Stat(0)) == NULL)
					goto E1;
				Sym_ChkEol();
				if (declFlg == 0) {
					if (sp->p.et2->e.lep == NULL)
						sp->p.et2->e.lep = xp;
					else
						cdp->e.lep = xp;
					cdp = xp;
				}
			}
		}
		if (Sym_tok == I_ENDP) {
			if (sp->v.tok != T_MACPROC)
				sp->v.tok = T_PROC_DECL;
			Sym_GetChkEol();
			goto ENDF;
		} else if (Sym_tok == I_ENTER) {
			oProcEnter = 1;
			break;
		} else if (Sym_tok == I_BEGIN) {
			if (oPPPflg || oProcArgSiz)
				Msg_Err("スタックに積まれた引数があるのにbeginが指定された");
			break;
		} else if (Sym_tok == I_IN || Sym_tok == I_OUT || Sym_tok == I_BREAK) {
			t = Sym_tok;
			do {
				Sym_Get();
				//ProcRegChk( (t == I_IN)  ? &oInR:
				//			(t == I_OUT) ? &oOutR: &oBreakR);
			} while (Sym_Get() == I_COMMA);
			Sym_ChkEol();
		} else if (Sym_tok == I_CDECL) {
			oCProcFlg = 1;
			sp->v.flg2 |= FL_CPROC;
			Sym_GetChkEol();
		} else if (Sym_tok == I_FORWORD||Sym_tok == I_EXTERN) {
			if (Sym_tok == I_EXTERN)
				sp->v.flg2 |= FL_EXTERN;
			sp->v.tok = T_PROC_DECL;
			Sym_GetChkEol();
		} else if (Sym_tok == I_FAR) {
			oFarProcFlg = 1;
			Ana_saveOfs = 4;
			sp->v.flg2 |= FL_FAR;
			Sym_GetChkEol();
		} else if (Sym_tok == I_NEAR) {
			oFarProcFlg = 0;
			Ana_saveOfs = 2;
			sp->v.flg2 &= ~FL_FAR;
			Sym_GetChkEol();
		} else if (Sym_tok == I_CR || Sym_tok == I_SMCLN) {
			;
		} else {
 E1:
			Msg_Err("手続きの宣言部がおかしい");
 E2:
			Sym_SkipCR();
		}
	}
	if (sp->v.tok != T_PROC) {
		Msg_Err("手続宣言なのに本体がある");
		sp->v.tok = T_PROC;
	}
	Sym_GetChkEol();
	if (declFlg && sp->p.et2->f.in == oInR && sp->p.et2->f.out == oOutR
		&&sp->p.et2->f.brk == oBreakR) {
		;
	} else {
		if (declFlg)
			Msg_Err("in,out,breakが最初の宣言と矛盾している");
		sp->p.et2->f.in = oInR;
		sp->p.et2->f.out = oOutR;
		sp->p.et2->f.brk = oBreakR;
	}
	MSGF(("in:%x  out:%x  break:%x\n",oInR,oOutR,oBreakR));
	if (oOutR & oBreakR)
		Msg_Err("break ﾊﾟﾗﾒｰﾀがoutのﾚｼﾞｽﾀとﾀﾞﾌﾞっている");
	if (Decl_defFlg) {
		if (ProcBody(sp,alain))
			return Ana_err;
	} else {
		sp->v.tok = T_PROC_DECL;
		sp->v.flg2 |= FL_INPO;
		while (Sym_Get()!= I_ENDP && Sym_tok != I_ENDMODULE && Ana_err == 0)
			;
		if (Sym_tok != I_ENDP)
			Msg_Err("import中の手続きが閉じていない");
	}
 ENDF:
	oFarProcFlg = oProcEnter = 0;
	OutStrl();
	St_Free(St_localRoot);
	return 0;
}

