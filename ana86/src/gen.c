#include <string.h>
#include <dos.h>
#include "anadef.h"

#if 0
static byte Gen_condMode;   	/* 0:符号変化無視  1:在り　2:在り(基本命令のみ) */
#endif
static byte oGenEquFlg;
static Et_t oETmpH, oETmpL, oETmp2H, oETmp2L, oWTmpH, oWTmpL, oWTmp2H, oWTmp2L;
static Et_t oETmpC, oETmpR;

/*---------------------------------------------------------------------------*/

void	Gen_EquFlg(int n)
{
    oGenEquFlg = n;
}

void	Gen_ChkEquFlg(void)
{
    if (Gen_condMode || oGenEquFlg)
    	Msg_Err("両辺のサイズが違う（左辺のほうが大きい）");
}

Et_t_fp Ev_Cnst(long val)
{
    oETmpC.e.op = T_CNST;
    oETmpC.c.val = val;
    return &oETmpC;
}

Et_t_fp Ev_Cnst2(long val)
{
    oETmp2L.e.op = T_CNST;
    oETmp2L.c.val = val;
    return &oETmp2L;
}

Et_t_fp Ev_Reg(word reg)
{
    if (Op_Reg1(reg)) {
    	oETmpR.e.op = T_R1;
    	oETmpR.m.typ = I_BYTE;
    } else {
    	oETmpR.e.op = T_R2;
    	oETmpR.m.typ = I_WORD;
    }
    oETmpR.c.reg = reg;
    return &oETmpR;
}

Et_t_fp Ev_Reg4(word reg)
{
    if (reg <= 0xff)
    	return Ev_Reg(reg);
    oETmpR.c.reg = reg;
    oETmpR.m.typ = I_DWORD;
    reg /= 0x100;
    if (Op_Seg2(reg)) {
    	oETmpR.e.op = T_SEG4;
    } else {
    	oETmpR.e.op = T_R4;
    }
    return &oETmpR;
}

Et_t_fp Ev_Reg2(word reg)
{
    if (Op_Reg1(reg)) {
    	oETmp2H.e.op = T_R1;
    	oETmp2H.m.typ = I_BYTE;
    } else {
    	oETmp2H.e.op = T_R2;
    	oETmp2H.m.typ = I_WORD;
    }
    oETmp2H.c.reg = reg;
    return &oETmp2H;
}

static void EtWTmp(Et_t_fp lp)
{
    oWTmpH = oWTmpL = *lp;
    oWTmpH.m.typ = oWTmpL.m.typ = I_BYTE;
    switch (lp->e.op) {
    case T_M2:
    	oWTmpH.e.op = oWTmpL.e.op = T_M1;
    	oWTmpH.m.ofs = lp->m.ofs + 1;
    	oWTmpL.m.ofs = lp->m.ofs;
    	break;
    case T_R2:
    	oWTmpH.e.op = oWTmpL.e.op = T_R1;
    	oWTmpH.c.reg = RegXtoH(lp->c.reg);
    	oWTmpL.c.reg = RegXtoL(lp->c.reg);
    }
}

static void EtWTmp2(Et_t_fp lp)
{
    oWTmp2H = oWTmp2L = *lp;
    oWTmp2H.m.typ = oWTmp2L.m.typ = I_BYTE;
    switch (lp->e.op) {
    case T_M2:
    	oWTmp2H.e.op  = oWTmp2L.e.op = T_M1;
    	oWTmp2H.m.ofs = lp->m.ofs + 1;
    	oWTmp2L.m.ofs = lp->m.ofs;
    	break;
    case T_R2:
    	oWTmp2H.e.op  = oWTmp2L.e.op = T_R1;
    	oWTmp2H.c.reg = RegXtoH(lp->c.reg);
    	oWTmp2L.c.reg = RegXtoL(lp->c.reg);
    }
}

static void EtDTmp(Et_t_fp lp)
{
    switch (lp->e.op) {
    case T_M4:
    	oETmpH = oETmpL = *lp;
    	oETmpH.m.typ = oETmpL.m.typ = I_WORD;
    	oETmpH.e.op  = oETmpL.e.op = T_M2;
    	oETmpH.m.ofs = lp->m.ofs + 2;
    	oETmpL.m.ofs = lp->m.ofs;
    	break;
    case T_CNST:
    	oETmpH = oETmpL = *lp;
    	oETmpH.m.typ = oETmpL.m.typ = I_WORD;
    	oETmpH.e.op  = oETmpL.e.op = T_CNST;
    	oETmpH.c.val = (unsigned long) lp->c.val / 0x10000L;
    	oETmpL.c.val = lp->c.val & 0xFFFF;
    	break;
    case T_R4:
    case T_SEG4:
    	oETmpH = oETmpL = *lp;
    	oETmpH.m.typ = oETmpL.m.typ = I_WORD;
    	oETmpH.e.op  = oETmpL.e.op = T_R2;
    	oETmpH.c.reg = lp->c.reg / 0x100;
    	oETmpL.c.reg = lp->c.reg % 0x100;
    	if (IsEp_Seg4(lp))
    	    oETmpH.e.op = T_SEG2;
    	break;
    case I_W2D:
    	oETmpL = *(lp->e.rep);
    	oETmpH = *(lp->e.lep);
    	break;
    case T_R2:
    case T_M2:
    case T_R1:
    case T_M1:
    	Gen_ChkEquFlg();
    	oETmpL = *lp;
    	oETmpH = *EV_ZERO;
    }
}

static void EtDTmp2(Et_t_fp lp)
{
    switch (lp->e.op) {
    	case T_M4:
    	oETmp2H = oETmp2L = *lp;
    	oETmp2H.m.typ = oETmp2L.m.typ = I_WORD;
    	oETmp2H.e.op  = oETmp2L.e.op = T_M2;
    	oETmp2H.m.ofs = lp->m.ofs + 2;
    	oETmp2L.m.ofs = lp->m.ofs;
    	break;
    case T_CNST:
    	oETmp2H = oETmp2L = *lp;
    	oETmp2H.m.typ = oETmp2L.m.typ = I_WORD;
    	oETmp2H.e.op  = oETmp2L.e.op = T_CNST;
    	oETmp2H.c.val = (unsigned long) lp->c.val / 0x10000L;
    	oETmp2L.c.val = lp->c.val & 0xFFFF;
    	break;
    case T_R4:
    case T_SEG4:
    	oETmp2H = oETmp2L = *lp;
    	oETmp2H.m.typ = oETmp2L.m.typ = I_WORD;
    	oETmp2H.e.op  = oETmp2L.e.op = T_R2;
    	oETmp2H.c.reg = lp->c.reg / 0x100;
    	oETmp2L.c.reg = lp->c.reg % 0x100;
    	if (IsEp_Seg4(lp))
    	    oETmp2H.e.op = T_SEG2;
    	break;
    case I_W2D:
    	oETmp2L = *(lp->e.rep);
    	oETmp2H = *(lp->e.lep);
    	break;
    case T_R2:
    case T_M2:
    case T_R1:
    case T_M1:
    	Gen_ChkEquFlg();
    	oETmp2L = *lp;
    	oETmp2H = *EV_ZERO;
    }
}

void	Gen_PushPop(word t, Et_t_fp lp)
{
    switch (lp->e.op) {
    	case I_FX:
    	Out_Nem0((t == I_PUSH) ? I_PUSHF : I_POPF);
    	break;
    case T_CNST:
    case T_CNSTEXPR:
    	if (t == I_POP)
    	    goto ERR;
    	if (Opt_cpu == 0)
    	    Msg_Err("8086ﾓｰﾄﾞでは即値をpushできません");
    	Out_Nem1(t, lp);
    	break;
    case I_W2D:
    	if (t == I_PUSH) {
    	    Gen_PushPop(t, lp->e.lep);
    	    Gen_PushPop(t, lp->e.rep);
    	} else {
    	    Gen_PushPop(t, lp->e.rep);
    	    Gen_PushPop(t, lp->e.lep);
    	}
    	break;
    default:
    	if (IsEp_Lft2(lp) || IsEp_Seg2(lp))
    	    Out_Nem1(t, lp);
    	else if (IsEp_Lft4(lp) || IsEp_Seg4(lp)) {
    	    EtDTmp2(lp);
    	    if (t == I_PUSH) {
    	    	Out_Nem1(t, &oETmp2H);
    	    	Out_Nem1(t, &oETmp2L);
    	    } else {
    	    	Out_Nem1(t, &oETmp2L);
    	    	Out_Nem1(t, &oETmp2H);
    	    }
    	} else
    	    goto ERR;
    }
    return;
  ERR:
    Msg_Err("push,popがおかしい");
}


static void GenPush(Et_t_fp ep)
{
    static Et_t_fp kp[24];
    int     i;

    for (i = 0; i < 20 && ep; i++, ep = ep->e.rep) {
    	kp[i] = ep->e.lep;
    }
    if (i >= 20)
    	Msg_Err("push のパラメータが多すぎる");
    else if (i == 0)
    	Msg_Err("push のパラメータがない");
    else {
    	while (--i >= 0)
    	    Gen_PushPop(I_PUSH, kp[i]);
    }
}

static void GenPop(Et_t_fp ep)
{
    while (ep) {
    	Gen_PushPop(I_POP, ep->e.lep);
    	ep = ep->e.rep;
    }
}

void	Gen_Equ(Et_t_fp lp, Et_t_fp rp)
{
    Et_t    et;

    MSG("Gen_Equ");
    switch (lp->e.op) {

    case T_SEG2:
    	if (rp->e.op == T_SEG2) {
    	    if (lp->c.reg == rp->c.reg)
    	    	break;
    	    Out_Nem1(I_PUSH, rp);
    	    Out_Nem1(I_POP, lp);
    	    break;
    	} if (IsEp_Cnst(rp)) {
    	    if (Opt_cpu == 0)
    	    	Msg_Err("セグメント・レジスタに定数は直接代入できない");
    	    Out_Nem1(I_PUSH, rp);
    	    Out_Nem1(I_POP, lp);
    	    break;
    	}
    case T_R2:
    	if (IsEp_Lft1(rp)) {
    	    Gen_ChkEquFlg();
    	    et.e.op = T_R1;
    	    et.c.reg = RegXtoL(lp->c.reg);
    	    if (et.c.reg != rp->c.reg)
    	    	Out_Nem2(I_EQU, &et, rp);
    	    et.c.reg = RegXtoH(lp->c.reg);
    	    Out_Nem2(I_EOREQ, &et, &et);
    	    break;
    	} else if (rp->e.op == I_FX) {
    	    goto J_FX;
    	} else if (rp->e.op == T_ADDR) {
    	    Out_Nem2(I_LEA, lp, rp->e.lep);
    	    break;
    	} else if (rp->e.op == I_PORT && lp->c.reg == I_AX) {
    	    Out_Nem2(I_INP, Ev_Reg(I_AX), rp->e.lep);
    	    break;
    	}
    	goto J_RR;
    case T_R1:
    	if (lp->c.reg == I_AL) {
    	    if (rp->e.op == I_PORT) {
    	    	Out_Nem2(I_INP, Ev_Reg(I_AL), rp->e.lep);
    	    	break;
    	    } else if (rp->e.op == I_BXAL) {
    	    	Out_Nem0(I_XLAT);
    	    	break;
    	    }
    	} else if (rp->e.op == I_FL && lp->c.reg == I_AH) {
    	    Out_Nem0(I_LAHF);
    	    break;
    	} else if (!IsEp_Rht1(rp))
    	    goto ERR2;
      J_RR:
    	if (Gen_condMode == 0) {
    	    if (IsEp_CnstVal(rp, 0)) {
    	    	Out_Nem2(I_EOREQ, lp, lp);
    	    	break;
    	    } else if (IsEp_Reg12(rp) && lp->c.reg == rp->c.reg)
    	    	break;
    	}
    	IsEp_CnstTyp(lp->m.typ, rp);
    	Out_Nem2(I_EQU, lp, rp);
    	break;

    case T_M2:
    	if (IsEp_Reg1(rp)) {
    	    Gen_ChkEquFlg();
    	    et.e.op = T_M1;
    	    et.e.lep = lp->e.lep;
    	    et.m.ofs = lp->m.ofs;
    	    et.m.typ = I_BYTE;
    	    Out_Nem2(I_EQU, &et, rp);
    	    et.m.ofs++;
    	    Out_Nem2(I_EQU, &et, EV_ZERO);
    	    break;
    	} else if (rp->e.op == I_FX) {
    	  J_FX:
    	    Out_Nem0(I_POPF);
    	    Out_Nem1(I_PUSH, lp);
    	    break;
    	}
    	goto J_MM;
    case T_M1:
    	if (!IsEp_Rht1(rp))
    	    goto ERR2;
      J_MM:
    	if (IsEp_Mem124(rp)) {
    	    Msg_Err("メモリ同士の代入はできない");
    	    break;
    	}
    	IsEp_CnstTyp(lp->m.typ, rp);
    	Out_Nem2(I_EQU, lp, rp);
    	break;

    case I_FX:
    	if (IsEp_Cnst(rp) && Opt_cpu == 0) {
    	    Msg_Err("ﾌﾗｸﾞ･ﾚｼﾞｽﾀに定数は代入できない");
    	} else {
    	    Out_Nem1(I_PUSH, rp);
    	    Out_Nem0(I_POPF);
    	}
    	break;

    case I_FL:
    	if (IsEp_R1op(rp, I_AH))
    	    Out_Nem0(I_SAHF);
    	else
    	    Msg_Err("fl=ahの指定がいかしい");
    	break;

    case I_PORT:
    	if (IsEp_R1op(rp, I_AL)) {
    	    Out_Nem2(I_OUTP, lp->e.lep, Ev_Reg(I_AL));
    	} else if (IsEp_R2op(rp, I_AX)) {
    	    Out_Nem2(I_OUTP, lp->e.lep, Ev_Reg(I_AX));
    	} else
    	    Msg_Err("port()はalかaxでないとｱｸｾｽできない");
    	break;

    default:
    	Msg_Err("代入の左辺がおかしい");
    }
 /* oGenEquFlg = 0; */
    return;

  ERR2:
    Msg_Err("小さな型のものへ大きな型のものを代入できない");
 /* oGenEquFlg = 0; */
}


void	GenEquSeg(Et_t_fp lp, Et_t_fp rp)
{
#if 0
    word    seg;

    if (oGenEquFlg) {
    	if (rp->e.op == T_R2 && (rp->c.reg == I_BP || rp->c.reg == I_SP))
    	    seg = I_SS;
    	else
    	    seg = I_DS;
    	Gen_Equ(lp, Ev_Reg(seg));
    } else {
    	Gen_Equ(lp, EV_ZERO);
    }
#else
    rp;
    Gen_Equ(lp, EV_ZERO);
#endif
}

void	Gen_Equ4(Et_t_fp lp, Et_t_fp rp)
{
    if (rp->e.op == T_CNSTEXPR) {
    	Msg_Err("32ﾋﾞｯﾄ･ﾚｼﾞｽﾀにはﾗﾍﾞﾙの交ざった定数式をつかえない");
    	return;
    }
    EtDTmp(lp);
    EtDTmp2(rp);
    switch (lp->e.op) {
    case T_SEG4:
    	if (IsEp_Mem4(rp)) {
    	    Out_Nem2((oETmpH.c.reg == I_DS) ? I_LDS : I_LES, &oETmpL,
    	    	      /* (Opt_r86) ? */ rp /* : &oETmp2L */ );
    	    break;
    	}
    case T_R4:
    	switch (rp->e.op) {
    	case T_R1:
    	case T_M1:
    	    goto J_R1;
    	case T_R2:
    	case T_M2:
    	    goto J_R2;
    	case T_R4:
    	case T_SEG4:
    	case T_M4:
    	    goto J_R4;
    	case T_CNST:
    	    Gen_Equ(&oETmpL, &oETmp2L);
    	    if (oETmp2L.c.val == oETmp2H.c.val) {
    	    	Gen_Equ(&oETmpH, &oETmpL);
    	    } else {
    	    	Gen_Equ(&oETmpH, &oETmp2H);
    	    }
    	    break;
    	case I_W2D:
    	    goto J_W2D;
    	default:
    	    goto ERR;
    	}
    	break;
    case T_M4:
    	switch (rp->e.op) {
    	case T_R1:
    	  J_R1:
    	case T_R2:
    	  J_R2:
    	    Gen_ChkEquFlg();
    	    Gen_Equ(&oETmpL, rp);
    	    GenEquSeg(&oETmpH, rp);
    	    break;
    	case T_R4:
    	case T_SEG4:
    	case T_CNST:
    	  J_R4:
    	    Gen_Equ(&oETmpL, &oETmp2L);
    	    Gen_Equ(&oETmpH, &oETmp2H);
    	    break;
    	case I_W2D:
    	  J_W2D:
    	    Gen_Equ(&oETmpL, rp->e.rep);
    	    Gen_Equ(&oETmpH, rp->e.lep);
    	    break;
    	default:
    	    goto ERR;
    	}
    	break;
    default:
      ERR:
    	Msg_Err("代入の左辺がおかしい");
    }
 /* oGenEquFlg = 0; */
    return;
}

static void GenMul(Et_t_fp xp, word op, Et_t_fp lp, Et_t_fp rp)
{
    word    lr, r, typ;

    r = 0;
    typ = I_WORD;
    if (IsEp_Reg2(xp)) {
    	if (xp->c.reg == I_AX) {
    	    r = I_AL;
    	    typ = I_BYTE;
    	}
    } else if (IsEp_Reg4(xp)) {
    	if (xp->c.reg != I_DXAX)
    	    goto ERR;
    	r = I_AX;
    } else {
    	goto ERR;
    }
    lr = lp->c.reg;
#if 0
    {
    	word	a1, a2;
    	if ((a1 = Expr_RVal(lp)) > 2 || (a2 = Expr_RVal(rp)) > 2)
    	    Msg_Err("右辺がおかしい");
    	lp = Expr_CnvRVal(a1, lp);
    	rp = Expr_CnvRVal(a2, rp);
    }
#endif
    while ((IsEp_Lft12(lp) && typ != lp->m.typ)
    	   || (IsEp_Lft12(rp) && typ != rp->m.typ)) {
    	if (r == I_AL) {
    	    r = 0;
    	    typ = I_WORD;
    	} else
    	    goto ERR;
    }
    if (r) {

    	if (IsEp_Reg12(lp) && lr == r) ;
    	else {
    	    Et_t_fp tmp;
    	    tmp = lp;
    	    lp = rp;
    	    rp = tmp;
    	}
    	Gen_Equ(Ev_Reg(r), lp);
    	if (IsEp_Cnsts(rp)) {
    	    lp = rp;
    	    Gen_Equ(rp = Ev_Reg((r == I_AX) ? I_DX : I_AH), lp);
    	}
    	Out_Nem1(op, rp);
    } else {
    	if (IsEp_Lft2(lp) && IsEp_Cnsts(rp))
    	    Out_Nem3(I_IMUL, xp, lp, rp);
    	else if (IsEp_Cnsts(lp) && IsEp_Lft2(rp))
    	    Out_Nem3(I_IMUL, xp, rp, lp);
    	else
    	    goto ERR;
    }
    return;
  ERR:
    Msg_Err("掛算の指定がおかしい");
}

static void GenEqus(Et_t_fp lp, Et_t_fp rp)
{
    if (rp->e.op == I_MUL || rp->e.op == I_IMUL)
    	GenMul(lp, rp->e.op, rp->e.lep, rp->e.rep);
    else if (IsEp_Lft4S(lp))
    	Gen_Equ4(lp, rp);
    else
    	Gen_Equ(lp, rp);
}

static void GenEq(Et_t_fp lp, Et_t_fp rp, void (*func)(Et_t_fp,Et_t_fp) )
{
    if (lp->e.op != I_EQU) {
    	(*func) (lp, rp);
    	return;
    }
    (*func)(lp->e.rep, rp);
    for (; ;) {
    	rp = lp->e.rep;
    	lp = lp->e.lep;
    	if (lp->e.op != I_EQU)
    	    break;
    	GenEqus(lp->e.rep, rp);
    }
    GenEqus(lp, rp);
    return;
}

static void GenZeroTest(Et_t_fp p)
{
    Out_Nem2(I_OREQ, p, p);
}

static int GenShift4(word xo, Et_t_fp lp, int val)
{
#if 1
    int     i;
#endif

    if (val >= 24) {
    	if (xo == I_SHLEQ) {
    	    EtDTmp(lp);
    	    EtWTmp(&oETmpH);
    	    EtWTmp2(&oETmpL);
    	    val -= 24;
    	    Gen_Equ(&oWTmpH, &oWTmp2L);
    	    Gen_Equ(&oWTmpL, EV_ZERO);
    	    Gen_Equ(&oETmpL, EV_ZERO);
    	} else if (xo == I_SHREQ) {
    	    EtDTmp(lp);
    	    EtWTmp(&oETmpH);
    	    EtWTmp2(&oETmpL);
    	    val -= 24;
    	    Gen_Equ(&oWTmp2L, &oWTmpH);
    	    Gen_Equ(&oWTmp2H, EV_ZERO);
    	    Gen_Equ(&oETmpH, EV_ZERO);
    	} else if (xo == I_SAREQ) {
    	    EtDTmp(lp);
    	    EtWTmp(&oETmpH);
    	    EtWTmp2(&oETmpL);
    	    val -= 24;
    	    Gen_Equ(&oWTmp2L, &oWTmpH);
#if 1
    	    i = 7;
    	    while (--i >= 0)
    	    	Out_Nem2(xo, &oWTmpH, Ev_Cnst(1));
#else
    	    Out_Nem2(I_ANDEQ, &oWTmpH, Ev_Cnst(0x80));
    	    Out_NmSt("je", "$+4");
    	    Gen_Equ(&oWTmpH, Ev_Cnst(0xFF));
#endif
    	    Gen_Equ(&oWTmpL, &oWTmpH);
    	    Gen_Equ(&oWTmp2H, &oWTmpH);
    	}
    }
    if (val >= 16) {
    	EtDTmp(lp);
    	val -= 16;
    	if (xo == I_ROLEQ || xo == I_ROREQ) {
    	    Out_Nem2(I_EXG, &oETmpH, &oETmpL);
    	} else if (xo == I_SHLEQ) {
    	    Gen_Equ(&oETmpH, &oETmpL);
    	    Gen_Equ(&oETmpL, EV_ZERO);
    	} else if (xo == I_SHREQ) {
    	    Gen_Equ(&oETmpL, &oETmpH);
    	    Gen_Equ(&oETmpH, EV_ZERO);
    	} else if (xo == I_SAREQ) {
    	    Gen_Equ(&oETmpL, &oETmpH);
    	    EtWTmp(&oETmpH);
#if 1
    	    i = 7;
    	    while (--i >= 0)
    	    	Out_Nem2(xo, &oWTmpH, Ev_Cnst(1));
#else
    	    Out_Nem2(I_ANDEQ, &oWTmpH, Ev_Cnst(0x80));
    	    Out_NmSt("je", "$+4");
    	    Gen_Equ(&oWTmpH, Ev_Cnst(0xFF));
#endif
    	    Gen_Equ(&oWTmpL, &oWTmpH);
    	} else {
    	    val += 16;
    	}
    }
    if (val >= 8) {
    	EtDTmp(lp);
    	EtWTmp(&oETmpH);
    	EtWTmp2(&oETmpL);
    	val -= 8;
    	if (xo == I_SHLEQ) {
    	    Gen_Equ(&oWTmpH, &oWTmpL);
    	    Gen_Equ(&oWTmpL, &oWTmp2H);
    	    Gen_Equ(&oWTmp2H, &oWTmp2L);
    	    Gen_Equ(&oWTmp2L, EV_ZERO);
    	} else if (xo == I_SHREQ) {
    	    Gen_Equ(&oWTmp2L, &oWTmp2H);
    	    Gen_Equ(&oWTmp2H, &oWTmpL);
    	    Gen_Equ(&oWTmpL, &oWTmpH);
    	    Gen_Equ(&oWTmpH, EV_ZERO);
    	} else if (xo == I_ROLEQ) {
    	    Out_Nem2(I_EXG, &oWTmpH, &oWTmpL);
    	    Out_Nem2(I_EXG, &oWTmpL, &oWTmp2H);
    	    Out_Nem2(I_EXG, &oWTmp2H, &oWTmp2L);
    	} else if (xo == I_ROREQ) {
    	    Out_Nem2(I_EXG, &oWTmp2L, &oWTmp2H);
    	    Out_Nem2(I_EXG, &oWTmp2H, &oWTmpL);
    	    Out_Nem2(I_EXG, &oWTmpL, &oWTmpH);
    	} else if (xo == I_SAREQ) {
    	    Gen_Equ(&oWTmp2L, &oWTmp2H);
    	    Gen_Equ(&oWTmp2H, &oWTmpL);
    	    Gen_Equ(&oWTmpL, &oWTmpH);
#if 1
    	    i = 7;
    	    while (--i >= 0)
    	    	Out_Nem2(xo, &oWTmpH, Ev_Cnst(1));
#else
    	    Out_Nem2(I_ANDEQ, &oWTmpH, Ev_Cnst(0x80));
    	    Out_NmSt("je", "$+4");
    	    Gen_Equ(&oWTmpH, Ev_Cnst(0xFF));
#endif
    	} else
    	    val += 8;
    }
    if (val == 0)
    	return 0;
    if (xo == I_ROLEQ || xo == I_ROREQ)
    	return 1;
    EtDTmp(lp);
    Ev_Cnst(1);
    while (--val >= 0) {
    	switch (xo) {
    	case I_SHLEQ:
    	case I_RCLEQ:
    	    Out_Nem2(xo, &oETmpL, &oETmpC);
    	    Out_Nem2(I_RCLEQ, &oETmpH, &oETmpC);
    	    break;
    	case I_SHREQ:
    	case I_SAREQ:
    	case I_RCREQ:
    	    Out_Nem2(xo, &oETmpH, &oETmpC);
    	    Out_Nem2(I_RCREQ, &oETmpL, &oETmpC);
    	    break;
    	}
    }
    return 0;
}

static void GenShift(word xo, Et_t_fp lp, Et_t_fp rp)
{
    int     val, l, i;

    switch (lp->m.typ) {
    case I_BYTE:
    	l = 8;
    	break;
    case I_WORD:
    	l = 16;
    	break;
    case I_DWORD:
    	l = 32;
    	break;
    default:
    	goto ERL;
    }
    if (IsEp_R1op(rp, I_CL)) {
    	if (!IsEp_Lft12(lp))
    	    goto ERL;
    	Out_Nem2(xo, lp, Ev_Reg(I_CL));
    	goto ENDF;
    } else if (!IsEp_Cnst(rp))
    	goto ERR;

    IsEp_CnstTyp(I_BYTE, rp);
    val = (int16) rp->c.val;
    if (val < 0) {
    	val = -val;
    	switch (xo) {
    	case I_SAREQ:
    	case I_SHREQ:
    	    xo = I_SHLEQ;
    	    break;
    	case I_SHLEQ:
    	    xo = I_SHREQ;
    	    break;
    	case I_ROLEQ:
    	    xo = I_ROREQ;
    	    break;
    	case I_ROREQ:
    	    xo = I_ROLEQ;
    	    break;
    	case I_RCLEQ:
    	    xo = I_RCREQ;
    	    break;
    	case I_RCREQ:
    	    xo = I_RCLEQ;
    	    break;
    	}
    }
    if (Gen_condMode) {
    	if (IsEp_Lft12(lp))
    	    goto J1;
    	else
    	    goto ERR;
    }
    if (xo == I_ROLEQ || xo == I_ROREQ) {
    	val %= l;
    	if ((val > 4 && val < 8) || (val > 11 && val < 16)
    	    || (l == 32 && val > 16)) {
    	    val = l - val;
    	    xo = (xo == I_ROLEQ) ? I_ROREQ : I_ROLEQ;
    	}
    } else if (xo == I_RCLEQ || xo == I_RCREQ) {
    	val %= l + 1;
    	if (val > l / 2) {
    	    val = l + 1 - val;
    	    xo = (xo == I_RCLEQ) ? I_RCREQ : I_RCLEQ;
    	}
    } else if (val > l) {
    	val = l;
    }
    if (val == l) {
    	if (xo == I_SHLEQ || xo == I_SHREQ) {
    	    GenEqus(lp, EV_ZERO);
    	    goto ENDF;
    	}
    }
    if (l == 32) {
    	if (GenShift4(xo, lp, val))
    	    goto ERR;
    	goto ENDF;
    }
    if (xo == I_SAREQ && (val == l || val == l - 1)) {
    	if (IsEp_Reg2(lp)) {
#if 1
    	    EtWTmp(lp);
    	    i = 7;
    	    while (--i >= 0)
    	    	Out_Nem2(xo, &oWTmpH, Ev_Cnst(1));
    	    Out_Nem2(I_EQU, &oWTmpL, &oWTmpH);
#else
    	    Out_Nem2(I_ANDEQ, lp, Ev_Cnst(0x8000));
    	    Out_NmSt("je", "$+5");
    	    Out_Nem2(I_EQU, lp, Ev_Cnst(0xFFFF));
#endif
    	    goto ENDF;
    	} else if (IsEp_Reg1(lp)) {
#if 1
    	    i = 7;
    	    while (--i >= 0)
    	    	Out_Nem2(xo, lp, Ev_Cnst(1));
#else
    	    Out_Nem2(I_ANDEQ, lp, Ev_Cnst(0x80));
    	    Out_NmSt("je", "$+4");
    	    Out_Nem2(I_EQU, lp, Ev_Cnst(0xFF));
#endif
    	    goto ENDF;
    	}
    }
    if (IsEp_Reg2(lp) && val >= 8 && Opt_cpu == 0) {
    	EtWTmp(lp);
    	val -= 8;
    	if (xo == I_ROLEQ || xo == I_ROREQ) {
    	    Out_Nem2(I_EXG, &oWTmpH, &oWTmpL);
    	} else if (xo == I_SHLEQ) {
    	    Gen_Equ(&oWTmpH, &oWTmpL);
    	    Gen_Equ(&oWTmpL, EV_ZERO);
    	} else if (xo == I_SHREQ) {
    	    Gen_Equ(&oWTmpL, &oWTmpH);
    	    Gen_Equ(&oWTmpH, EV_ZERO);
#if 0
    	} else if (xo == I_SAREQ) {
    	    Gen_Equ(&oWTmpL, &oWTmpH);
    	    Out_Nem2(I_ANDEQ, &oWTmpH, Ev_Cnst(0x80));
    	    Out_NmSt("je", "$+4");
    	    Gen_Equ(&oWTmpH, Ev_Cnst(0xFF));
#endif
    	} else {
    	    val += 8;
    	}
    }
  J1:
    if (Opt_cpu) {
    	if (Opt_r86 && IsEp_Reg12(lp) && val > 1)
    	    Out_ShiftReg12Cnst(xo, lp->c.reg, val);
    	else
    	    Out_Nem2(xo, lp, Ev_Cnst(val));
    } else {
    	while (--val >= 0)
    	    Out_Nem2(xo, lp, Ev_Cnst(1));
    }
  ENDF:
    return;
  ERR:
    Msg_Err("ｼﾌﾄ命令の右辺の指定がおかしい");
    return;
  ERL:
    Msg_Err("ｼﾌﾄ命令の左辺の指定がおかしい");
    return;
}

static void GenAndeq(Et_t_fp lp, Et_t_fp rp)
{
    if (Gen_condMode == 0 && IsEp_Cnst(rp)) {
    	if (rp->c.val == 0) {
    	    Gen_Equ(lp, EV_ZERO);
    	    return;
    	} else if (IsEp_Lft1(lp) && rp->c.val == 0xff) {
    	    return;
    	} else if (IsEp_Lft2(lp)) {
    	    if (rp->c.val == 0xff) {
    	    	EtWTmp(lp);
    	    	Gen_Equ(&oWTmpH, EV_ZERO);
    	    	return;
    	    } else if (rp->c.val == 0xff00) {
    	    	EtWTmp(lp);
    	    	Gen_Equ(&oWTmpL, EV_ZERO);
    	    	return;
    	    } else if (rp->c.val == 0xffff) {
    	    	return;
    	    }
    	}
    }
    Out_Nem2(I_ANDEQ, lp, rp);
}

static void GenOreq(Et_t_fp lp, Et_t_fp rp)
{
    if (Gen_condMode == 0 && IsEp_Cnst(rp)) {
    	if (rp->c.val == 0) {
    	    return;
    	} else if (IsEp_Lft1(lp) && rp->c.val == 0xff) {
    	    goto J1;
    	} else if (IsEp_Lft2(lp)) {
    	    if (rp->c.val == 0xffff) {
    	      J1:
    	    	Gen_Equ(lp, rp);
    	    	return;
    	    } else if (rp->c.val == 0xff00 || rp->c.val == 0xff) {
    	    	EtWTmp(lp);
    	    	Gen_Equ((rp->c.val == 0xff) ? &oWTmpL : &oWTmpH, Ev_Cnst(0xff));
    	    	return;
    	    }
    	}
    }
    Out_Nem2(I_OREQ, lp, rp);
    return;
}

void	GenAO(word xo, Et_t_fp lp, Et_t_fp rp)
{
    IsEp_CnstTyp(lp->m.typ, rp);
    switch (xo) {
    case I_ANDEQ:
    	GenAndeq(lp, rp);
    	break;
    case I_OREQ:
    	GenOreq(lp, rp);
    	break;
    case I_EXG:
    	if (IsEp_Reg12(lp) && IsEp_Reg12(rp) && lp->c.reg == rp->c.reg
    	    && Gen_condMode == 0) {
    	    break;
    	}
    case I_EOREQ:
    case I_ADCEQ:
    case I_SBCEQ:
    	Out_Nem2(xo, lp, rp);
    }
    return;
}

static void GenInc(Et_t_fp lp)
{
#if 0
    if (IsEp_Reg4(lp)) {
    	EtDTmp(lp);
    	Out_Nem1(I_INC, &oETmpL);
    	Out_NmSt("jnz", "$+3");
    	Out_Nem1(I_INC, &oETmpH);
    } else if (IsEp_Mem4(lp))
#else
    if (IsEp_Lft4(lp))
#endif
    {
    	EtDTmp(lp);
    	Out_Nem2(I_ADDEQ, &oETmpL, Ev_Cnst(1));
    	Out_Nem2(I_ADCEQ, &oETmpH, EV_ZERO);
    } else if (IsEp_Lft12(lp))
    	Out_Nem1(I_INC, lp);
    else
    	Msg_Err("'++'でエラー");
    return;
}

static void GenDec(Et_t_fp lp)
{
    if (IsEp_Lft4(lp)) {
    	EtDTmp(lp);
    	Out_Nem2(I_SUBEQ, &oETmpL, Ev_Cnst(1));
    	Out_Nem2(I_SBCEQ, &oETmpH, EV_ZERO);
    } else if (IsEp_Lft12(lp)) {
    	Out_Nem1(I_DEC, lp);
    } else {
    	Msg_Err("'--'でエラー");
    }
    return;
}

static void GenAddeq(Et_t_fp lp, Et_t_fp rp)
{
    IsEp_CnstTyp(lp->m.typ, rp);
    if (Gen_condMode == 1 && IsEp_CnstVal(rp, 0) && IsEp_Reg12(lp)) {
    	GenZeroTest(lp);
    	return;
    } else if (Gen_condMode == 0) {
    	if (IsEp_Cnst(rp)) {
    	    switch (rp->c.val) {
    	    case 0:
    	    	return;
    	    case -1:
    	    	GenDec(lp);
    	    	return;
    	    case 1:
    	    	GenInc(lp);
    	    	return;
    	    case -2:
    	    	if (!IsEp_Reg2(lp))
    	    	    break;
    	    	GenDec(lp);
    	    	GenDec(lp);
    	    	return;
    	    case 2:
    	    	if (!IsEp_Reg2(lp))
    	    	    break;
    	    	GenInc(lp);
    	    	GenInc(lp);
    	    	return;
    	    }
    	}
#if 0
    	else if (IsEp_Reg12(lp) && IsEp_Reg12(rp) && lp->c.reg == rp->c.reg) {
    	    GenShift(I_SHLEQ, lp, Ev_Cnst(1));
    	    return;
    	}
#endif
    }
    Out_Nem2(I_ADDEQ, lp, rp);
}

static void GenAddeq4(Et_t_fp lp, Et_t_fp rp)
{
    if (IsEp_CnstVal(rp, 0)) {
    	return;
    } else if (IsEp_Reg4(rp) && lp->c.reg == rp->c.reg) {
    	GenShift(I_SHLEQ, lp, Ev_Cnst(1));
    	return;
    }
    EtDTmp(lp);
    EtDTmp2(rp);
#if 1
    if (oETmp2L.e.op == T_CNST && oETmp2L.c.val == 0) {
    	GenAddeq(&oETmpH, &oETmp2H);
    	return;
    }
#endif
    Out_Nem2(I_ADDEQ, &oETmpL, &oETmp2L);
    GenAO(I_ADCEQ, &oETmpH, &oETmp2H);
}

void	Gen_SubEq(Et_t_fp lp, Et_t_fp rp)
{
    IsEp_CnstTyp(lp->m.typ, rp);
    if (Gen_condMode == 1 && IsEp_CnstVal(rp, 0) && IsEp_Reg12(lp)) {
    	GenZeroTest(lp);
    	return;
    } else if (Gen_condMode == 0) {
    	if (IsEp_Cnst(rp)) {
    	    switch (rp->c.val) {
    	    case 0:
    	    	return;
    	    case -1:
    	    	GenInc(lp);
    	    	return;
    	    case 1:
    	    	GenDec(lp);
    	    	return;
    	    case -2:
    	    	if (lp->e.op != T_R2)
    	    	    break;
    	    	GenInc(lp);
    	    	GenInc(lp);
    	    	return;
    	    case 2:
    	    	if (lp->e.op != T_R2)
    	    	    break;
    	    	GenDec(lp);
    	    	GenDec(lp);
    	    	return;
    	    }
    	} else if (IsEp_Reg12(lp) && IsEp_Reg12(rp) && lp->c.reg == rp->c.reg) {
    	    Gen_Equ(lp, EV_ZERO);
    	    return;
    	}
    }
    Out_Nem2(I_SUBEQ, lp, rp);
}

void	GenSubeq4(Et_t_fp lp, Et_t_fp rp)
{
    if (IsEp_CnstVal(rp, 0)) {
    	return;
    } else if (IsEp_Reg4(rp) && lp->c.reg == rp->c.reg) {
    	Gen_Equ4(lp, EV_ZERO);
    	return;
    }
    EtDTmp(lp);
    EtDTmp2(rp);
#if 1
    if (oETmp2L.e.op == T_CNST && oETmp2L.c.val == 0) {

    	Gen_SubEq(&oETmpH, &oETmp2H);
    	return;
    }
#endif
    Out_Nem2(I_SUBEQ, &oETmpL, &oETmp2L);
    GenAO(I_SBCEQ, &oETmpH, &oETmp2H);
}

static void GenSieq(Et_t_fp lp, Et_t_fp rp)
{
    if (lp->m.typ == I_WORD) {
    	Gen_Equ(Ev_Reg(I_AL), rp);
    	Out_Nem0(I_SEXT1);
    	return;
    } else {
    	if (IsEp_Lft1(rp)) {
    	    Gen_Equ(Ev_Reg(I_AL), rp);
    	    Out_Nem0(I_SEXT1);
    	} else
    	    Gen_Equ(Ev_Reg(I_AX), rp);
    	Out_Nem0(I_SEXT2);
    	return;
    }
}

global void Gen_Pusha(void)
{
    if (Opt_cpu) {
    	Out_Nem0(I_PUSHA);
    } else {
    	Gen_PushPop(I_PUSH, Ev_Reg(I_AX));
    	Gen_PushPop(I_PUSH, Ev_Reg(I_CX));
    	Gen_PushPop(I_PUSH, Ev_Reg(I_DX));
    	Gen_PushPop(I_PUSH, Ev_Reg(I_BX));
    	Gen_PushPop(I_PUSH, Ev_Reg(I_SP));
    	Gen_PushPop(I_PUSH, Ev_Reg(I_BP));
    	Gen_PushPop(I_PUSH, Ev_Reg(I_SI));
    	Gen_PushPop(I_PUSH, Ev_Reg(I_DI));
    }
}

global void Gen_Popa(void)
{
    if (Opt_cpu) {
    	Out_Nem0(I_POPA);
    } else {
    	Gen_PushPop(I_POP, Ev_Reg(I_DI));
    	Gen_PushPop(I_POP, Ev_Reg(I_SI));
    	Gen_PushPop(I_POP, Ev_Reg(I_BP));
    	Gen_PushPop(I_POP, Ev_Reg(I_BX));
    	Gen_PushPop(I_POP, Ev_Reg(I_BX));
    	Gen_PushPop(I_POP, Ev_Reg(I_DX));
    	Gen_PushPop(I_POP, Ev_Reg(I_CX));
    	Gen_PushPop(I_POP, Ev_Reg(I_AX));
    }
}


global void Gen_Expr(Et_t_fp xp, word mode)
{
    Et_t_fp rp;
    Et_t_fp lp;
    word    xo;

    if (xp == NULL)
    	return;
    MSG("<<<Gen_Expr>>>");
    Gen_condMode = mode;
    lp = xp->e.lep;
    rp = xp->e.rep;
    xo = xp->e.op;
    switch (xo) {
    case I_EQU:
    	GenEq(lp, rp, GenEqus);
    	return;

    case I_SIEQ:
    	GenEq(lp, rp, GenSieq);
    	return;

    case I_ANDEQ:
    case I_OREQ:
    case I_EOREQ:
    case I_ADCEQ:
    case I_SBCEQ:
    case I_EXG:
    	if (IsEp_Reg4(lp) && !Gen_condMode) {
    	    EtDTmp(lp);
    	    EtDTmp2(rp);
    	    GenAO(xo, &oETmpL, &oETmp2L);
    	    GenAO(xo, &oETmpH, &oETmp2H);
    	} else if (IsEp_Lft4S(lp))
    	    goto ERR32;
    	else
    	    GenAO(xo, lp, rp);
    	return;

    case I_ADDEQ:
    	if (IsEp_Reg4(lp) && !Gen_condMode)
    	    GenAddeq4(lp, rp);
    	else if (IsEp_Lft4S(lp))
    	    goto ERR32;
    	else
    	    GenAddeq(lp, rp);
    	return;

    case I_SUBEQ:
    	if (IsEp_Reg4(lp) && !Gen_condMode)
    	    GenSubeq4(lp, rp);
    	else if (IsEp_Lft4S(lp))
    	    goto ERR32;
    	else
    	    Gen_SubEq(lp, rp);
    	return;

    case I_SHLEQ:
    case I_SHREQ:
    case I_SAREQ:
    case I_ROREQ:
    case I_ROLEQ:
    case I_RCREQ:
    case I_RCLEQ:
    	if (IsEp_Rht12(lp) || IsEp_Reg4(lp))
    	    GenShift(xo, lp, rp);
    	else
    	    goto ERR32;
    	return;

    case I_INC:
    	GenInc(lp);
    	return;

    case I_DEC:
    	GenDec(lp);
    	return;

    case I_COMS:
    case I_NEGS:
    	if (IsEp_Reg4(lp)) {
    	    EtDTmp(lp);
    	    Out_Nem1(xo, &oETmpH);
    	    Out_Nem1(xo, &oETmpL);
    	    if (xo == I_NEGS)
    	    	Out_Nem2(I_SBCEQ, &oETmpH, EV_ZERO);
    	} else if (IsEp_Lft4S(lp))
    	    goto ERR32;
    	else
    	    Out_Nem1(xo, lp);
    	return;
    /* case T_ADDR: if (lp->e.op != I_SUB) goto ERR; rp = lp->e.rep; lp = lp->e.lep; */
    case I_SUB:
    	if (IsEp_Lft4S(lp))
    	    goto ERR32;
    	if (Gen_condMode == 0)
    	    Msg_Err("ﾌﾗｸﾞ変化命令の使えないところで使われた");
    	if (Gen_condMode < 2 && IsEp_Reg12(lp) && IsEp_CnstVal(rp, 0)) {
    	    GenZeroTest(lp);
    	    return;
    	}
    	Out_Nem2(xo, lp, rp);
    	return;

    case I_AND:
    	if (IsEp_Lft4S(lp))
    	    goto ERR32;
    	if (Gen_condMode == 0)
    	    Msg_Err("ﾌﾗｸﾞ変化命令の使えないところで使われた");
    case I_BOUND:
    case I_ESC:
    	Out_Nem2(xo, lp, rp);
    	return;

    case I_DIVS:
    case I_IDIVS:
    	if (IsEp_R4op(lp, I_DXAX) && IsEp_Lft2(rp)) {
    	    Out_Nem1(xo, rp);
    	} else if (IsEp_R2op(lp, I_AX) && IsEp_Lft1(rp)) {
    	    Out_Nem1(xo, rp);
    	} else
    	    Msg_Err("div,divsの指定がおかしい");
    	return;
    case I_INTR:
    case I_RET:
    case I_RETF:
    	Out_Nem1(xo, lp);
    	return;
    case I_LEAVE:
    	Out_Leave();
    	return;
    case I_ENTER:
    	Out_Enter(lp->c.val, rp->c.val);
    	return;
    case I_PUSH:
    	GenPush(xp);
    	return;
    case I_POP:
    	GenPop(xp);
    	return;
    case I_CLRC:
    case I_SETC:
    case I_COMC:
    /* if (Gen_condMode == 0) Msg_Err("ﾌﾗｸﾞ変化命令が'|'なしに使われた"); */
    case I_INTO:
    case I_CLRI:
    case I_SETI:
    	Out_Nem0(xo);
    	return;
    case T_ONE:
    	Out_Nem0(xp->m.ofs);
    	return;
    case I_PUSHA:
    	Gen_Pusha();
    	return;
    case I_POPA:
    	Gen_Popa();
    	return;
    }
  //ERR:
    Msg_Err("Gen_Expr()がおかしい");
    return;
  ERR32:
    Msg_Err("使えない32ﾋﾞｯﾄ演算が使われた");
    return;
}


void	Gen_Rep(word t)
{
    enum {R_SET, R_SCAN, R_LOAD, R_CPY, R_CMP, R_INP, R_OUTP, R_INC, R_DEC};
    typedef byte *byte_p;
    static byte_p segv[4] = {"2Eh", "3Eh", "26h", "36h"};
    static byte_p reps[3] = {"rep", "repe", "repne"};
    static byte_p reps2[7][2] = {
    	{"stosb", "stosw"},
    	{"scasb", "scasw"},
    	{"lodsb", "lodsw"},
    	{"movsb", "movsw"},
    	{"cmpsb", "cmpsw"},
    	{"insb", "insw"},
    	{"outsb", "outsw"},
    };
    static byte_p memb[] = {"set", "scan", "load", "cpy", "cmp", "inp", "outp", "inc", "dec"};
    Et_t_fp p1, p2, p3, sav_p;
    word    reg1, reg2, seg2;
    int     f, r, parflg;

    seg2 = f = r = 0;
    if (!Ch_ChkPeriod()) {
    	Msg_Err("必要な'.'がない");
    	goto ENDF2;
    }
    Sym_NameGet(Ch_Get());
    for (f = 0; f < 9; f++) {
    	if (strcmp(Sym_name, memb[f]) == 0)
    	    break;
    }
    if (f < 7) {
    	if (f == 4 && t == 0) { // cmp
    	    t = 1;  	    	// rep を repe に変換.
    	}
    } else if (f == 7) {
    	Out_Nem0(I_CLRD);
    	Sym_Get();
    	goto ENDF2;
    } else if (f == 8) {
    	Out_Nem0(I_SETD);
    	Sym_Get();
    	goto ENDF2;
    } else {
    	Msg_Err("repの指定がおかしい");
    	goto ENDF2;
    }
    r = Ch_ChkBW();
    parflg = Sym_GetChkLP();
    sav_p = Et_Sav();
    if ((p1 = Expr(0)) == NULL || !(IsEp_Rht12(p1) || p1->e.op == T_ADDR))
    	goto ERR;
    if (Sym_tok != I_COMMA)
    	goto ERR;
    Sym_Get();
    if ((p2 = Expr(0)) == NULL)
    	goto ERR;
    if (!(IsEp_Rht12(p2) || p2->e.op == T_ADDR)) {
    	if (IsEp_Seg4(p2)) {
    	    seg2 = p2->c.reg / 0x100;
    	    p2->e.op = T_R2;
    	    p2->c.reg = p2->c.reg % 0x100;
    	    p2->m.typ = I_WORD;
    	} else if (p2->e.op == I_W2D) {
    	    if (!IsEp_Seg2(p2->e.lep) || !IsEp_Lft2(p2->e.rep))
    	    	goto ERR;
    	    seg2 = p2->c.reg / 0x100;
    	    p2 = p2->e.rep;
    	} else {
    	    goto ERR;
    	}
    }
    if (Sym_tok == I_COMMA) {
    	Sym_Get();
    	if ((p3 = Expr(0)) == NULL || !IsEp_Rht12(p3))
    	    goto ERR;
    } else {
    	p3 = NULL;
    }
    if (parflg) {
    	if (!Sym_ChkRP())
    	    Sym_Get();
    }
    reg1 = I_DI;
    reg2 = I_SI;
    switch (f) {
    case R_INP:
    	reg2 = I_DX;
    	break;
    case R_OUTP:
    	reg1 = I_DX;
    	break;
    case R_SET:
    case R_SCAN:
    	reg2 = (r != 0) ? I_AX : I_AL;
    	/* if (reg2 == I_AL && p2->m.typ != I_BYTE) goto ERR; */
    	break;
    case R_LOAD:
    	reg1 = (r != 0) ? I_AX : I_AL;
    	if (p1->c.reg != reg1)
    	    goto ERR;
    	break;
    }
    /*	if (reg2 == I_SI && (p2->m.typ != I_WORD && p2->e.op != T_ADDR)) goto ERR;
    	if (reg1 == I_DI && (p1->m.typ != I_WORD && p1->e.op != T_ADDR)) goto ERR; */
    if (IsEp_Reg12(p1) && p1->c.reg == reg1
    	&& IsEp_Reg12(p2) && p2->c.reg == reg2
    	&& (p3 == NULL || IsEp_R2op(p3, I_CX))) {
    	;
    } else {
    	if (p3)
    	    Gen_Equ(Ev_Reg(I_CX), p3);
    	Gen_Equ(Ev_Reg(reg2), p2);
    	Gen_Equ(Ev_Reg(reg1), p1);
    	if (Opt_auto == 0)
    	    Msg_Err("手続きまたはrep命令の引数で暗黙の代入が行われた");
    }
    if (seg2 && seg2 != I_DS)
    	Out_NmSt("db", segv[seg2 - I_CS]);
    if (p3)
    	Out_NmSt(reps[t - I_REP], reps2[f][r]);
    else
    	Out_Nm(reps2[f][r]);
  ENDF:
    Et_Frees(sav_p);
  ENDF2:
    return;
  ERR:
    Msg_Err("rep命令のﾊﾟﾗﾒｰﾀがおかしい");
    goto ENDF;
}

/*------------------------------------------------------------------*/

global	word GoLbl_NewNo(void)
{
    return ++GoLbl_no;
}

static	byte_fp GoLblStr(byte_fp lbl, word n)
{   	    	    	    	/* $$ */
    if (lbl)
    	sprintf(lbl, "L$%d", n);
    return lbl;
}

byte_fp GoLbl_Strs(word n)
{
    static byte lbl[GoLbl_SIZ + 2];

    return GoLblStr(lbl, n);
}

static	byte_fp GoLblNewStr(byte_fp lbl)
{
    return GoLblStr(lbl, GoLbl_NewNo());
}

/*------------------------- 条件ジャンプ ---------------------------------*/

static void GenCondJmp(word t, word ls, word neg_f, byte * lbl)
{
    static byte *jtbl[] = {
    	"je", "jne",
    	"ja", "jbe",
    	"jb", "jae",
    	"jg", "jle",
    	"jl", "jge",
    	"jc", "jnc",
    	"jz", "jnz",
    	"js", "jns",
    	"jo", "jno",
    	"jp", "jnp",
    	"jcxz", "jcxz",
    	"loop", "loop",
    	"loopz", "loopz",
    	"loopnz", "loopnz",
    	"jmp", NULL
    };
    byte    *s;
    byte    lbl2[GoLbl_SIZ + 2];
    byte    lbl3[GoLbl_SIZ + 2];

    if (neg_f)
    	t = Expr_CondNeg(t);
    if (ls && t < GO_CX)
    	t = Expr_CondNeg(t);
    s = jtbl[t - 2];
    if (t == GO_NJMP) {
    	;
    } else if (t == GO_JMP) {
    	if (ls)
    	    Out_NmSt("jmp", lbl);
    	else
    	    Out_NmSt(gJmps[Opt_r86], lbl);
    } else if (t == GO_CX && ls == 0 && Gen_condMode == 1) {
    	GenZeroTest(Ev_Reg(I_CX));
    	Out_NmSt("jne", lbl);

    } else if (ls || t == GO_CX || t == GO_NDECX
    	       || t == GO_NDECX_Z || t == GO_NDECX_NZ) {
    	if (GoLblNewStr(lbl2) == NULL)
    	    return;
    	if (t == GO_NCX || t == GO_DECX || t == GO_DECX_Z || t == GO_DECX_NZ) {
    	    if (GoLblNewStr(lbl3) == NULL)
    	    	return;
    	    Out_NmSt(s, lbl2);
    	    Out_NmSt(gJmps[Opt_r86], lbl3);
    	    Out_LblStr(lbl2);
    	    Out_NmSt("jmp", lbl);
    	    Out_LblStr(lbl3);
    	} else {
    	    Out_NmSt(s, lbl2);
    	    Out_NmSt("jmp", lbl);
    	    Out_LblStr(lbl2);
    	}
    } else {
    	Out_NmSt(s, lbl);
    }
}

static int GenCondOp(Et_t_fp xp, word long_f, word neg_f, byte * lbl)
{
    word    t, f;

    f = 0;
    switch (t = xp->e.op) {
    case T_R1:
    case T_R2:
    	if (neg_f != 0 && long_f == 0 && xp->c.reg == I_CX) {
    	    GenCondJmp(GO_NCX, 0, 0, lbl);
    	    break;
    	}
    	GenZeroTest(xp);
    	GenCondJmp(GO_NEQ, long_f, neg_f, lbl);
    	break;
    case T_M1:
    case T_M2:
    	Out_Nem2(I_SUB, xp, EV_ZERO);
    	GenCondJmp(GO_NEQ, long_f, neg_f, lbl);
    /* GenCondJmp(GO_NEQ,neg_f,long_f,lbl); */
    	break;
#if 0
    case T_CNST:
    	neg_f = (negf != 0) ^ (xp->c.val == 0);
    	GenCondJmp(GO_JMP, long_f, neg_f, lbl);
    	if (neg_f)
    	    return -1;
    	return 1;
#endif
    case I_DEC:
    	if (neg_f == 0 && long_f == 0 && IsEp_R2op(xp->e.lep, I_CX)) {
    	    GenCondJmp(GO_DECX, 0, 0, lbl);
    	    break;
    	}
    case I_INC:
    case I_SUB:
    case I_AND:
    case I_SUBEQ:
    case I_ADDEQ:
    case I_ANDEQ:
    case I_EOREQ:
    case I_OREQ:
    case I_SHREQ:
    case I_SHLEQ:
    case I_SAREQ:
    	Gen_Expr(xp, 1);
    	GenCondJmp(GO_NEQ, long_f, neg_f, lbl);
    	break;
    case I_NOT:
    	GenCondOp(xp->e.lep, long_f, (neg_f == 0), lbl);
    	break;
    case I_GT:
    	t = GO_GT;
    	goto J1;
    case I_GE:
    	t = GO_GE;
    	goto J1;
    case I_LT:
    	t = GO_LT;
    	goto J1;
    case I_LE:
    	t = GO_LE;
    	goto J1;
    case I_ILT:
    	t = GO_ILT;
    	goto J1;
    case I_ILE:
    	t = GO_ILE;
    	goto J1;
    case I_IGT:
    	t = GO_IGT;
    	goto J1;
    case I_IGE:
    	t = GO_IGE;
    	goto J1;
    case I_NEQ:
    	t = GO_NEQ;
    	goto J1;
    case I_EQEQ:
    	t = GO_EQU;
    	goto J1;
    case I_CFCM:
    	t = GO_NC;
    	f = 1;
    	goto J1;
    case I_SFCM:
    	t = GO_NS;
    	f = 1;
    	goto J1;
    case I_ZFCM:
    	t = GO_NZ;
    	f = 1;
    	goto J1;
    case I_PFCM:
    	t = GO_NP;
    	f = 1;
    	goto J1;
    case I_OVFCM:
    	t = GO_NO;
    	f = 1;
    	goto J1;
    case I_NCFCM:
    	t = GO_C;
    	f = 1;
    	goto J1;
    case I_NSFCM:
    	t = GO_S;
    	f = 1;
    	goto J1;
    case I_NZFCM:
    	t = GO_Z;
    	f = 1;
    	goto J1;
    case I_NPFCM:
    	t = GO_P;
    	f = 1;
    	goto J1;
    case I_NOVFCM:
    	t = GO_O;
    	f = 1;
    	goto J1;
      J1:
    	if (IsEp_Lft12(xp->e.lep)) {
    	    word    tt;
    	    if (f)
    	    	goto ERR;
    	    tt = xp->e.op;
    	    xp->e.op = I_SUB;
    	    Gen_Expr(xp, 1);
    	    xp->e.op = tt;
    	    GenCondJmp(t, long_f, neg_f, lbl);
    	} else {
    	    switch (xp->e.lep->e.op) {
    	    case I_DEC:
    	    	if (IsEp_R2op(xp->e.lep, I_CX) && long_f == 0
    	    	    && ((neg_f == 0 && t == GO_EQU) || (neg_f && t == GO_NEQ))
    	    	    && IsEp_CnstVal(xp->e.rep, 0)) {
    	    	    GenCondJmp(GO_DECX, 0, 0, lbl);
    	    	    break;
    	    	}
    	    case I_INC:
    	    	if (t == GO_LT || t == GO_LE || t == GO_GT || t == GO_GE)
    	    	    Msg_Err("比較での++,--はcﾌﾗｸﾞを変化させない");
    	    case I_SUB:
    	    case I_AND:
    	    case I_SUBEQ:
    	    case I_ADDEQ:
    	    case I_ANDEQ:
    	    case I_EOREQ:
    	    case I_OREQ:
    	    case I_SHREQ:
    	    case I_SHLEQ:
    	    case I_SAREQ:
    	    case I_RCREQ:
    	    case I_RCLEQ:
    	    	if (!IsEp_CnstVal(xp->e.rep, 0)) {
    	    	    if (f == 0 || !IsEp_CnstVal(xp->e.rep, 1))
    	    	    	goto ERR;
    	    	    neg_f = (neg_f) ? 0 : 1;
    	    	}
    	    	Gen_Expr(xp->e.lep, 1);
    	    	GenCondJmp(t, long_f, neg_f, lbl);
    	    	break;
    	    default:
    	    	goto ERR;
    	    }
    	}
    	break;
    default:
      ERR:
    	Msg_Err("条件として指定できないものが指定された");
    }
    return 0;
}

static void GenCondLAnd(Et_t_fp xp, word long_f, word neg_f, byte * lbl)
{
    word    t, nf;
    byte    lbl1[GoLbl_SIZ + 2];
    byte    lbl2[GoLbl_SIZ + 2];

    t = xp->e.op;
    if (t != I_LAND && t != I_LOR) {
    	GenCondOp(xp, long_f, neg_f, lbl);
    	return;
    }
    nf = (t == I_LAND);
    if (long_f == 0) {
    	if (neg_f == nf) {  	/* !&&, || */
    	    GenCondLAnd(xp->e.lep, 0, nf, lbl);
    	    GenCondLAnd(xp->e.rep, 0, nf, lbl);
    	} else {    	    	/* &&, !|| */
    	    GoLblNewStr(lbl1);
    	    GenCondLAnd(xp->e.lep, 0, nf, lbl1);
    	    GenCondLAnd(xp->e.rep, 0, !nf, lbl);
    	    Out_LblStr(lbl1);
    	}
    } else {
    	if (neg_f == nf) {  	/* !&&, || */
    	    GoLblNewStr(lbl1);
    	    GoLblNewStr(lbl2);
    	    GenCondLAnd(xp->e.lep, 0, nf, lbl1);
    	    GenCondLAnd(xp->e.rep, 0, !nf, lbl2);
    	    Out_LblStr(lbl1);
    	    Out_NmSt("jmp", lbl);
    	    Out_LblStr(lbl2);
    	} else {    	    	/* &&, !|| */
    	    GoLblNewStr(lbl1);
    	    GenCondLAnd(xp->e.lep, 0, nf, lbl1);
    	    GenCondLAnd(xp->e.rep, 0, nf, lbl1);
    	    Out_NmSt("jmp", lbl);
    	    Out_LblStr(lbl1);
    	}
    }
}

void	Gen_CondExpr(Et_t_fp xp, word mode, word ls, word neg_f, word lblno)
    /* ls   	neg_f
    	0   	0   	条件が成立すればショートジャンプ
    	0   	1   	条件が成立しなければショートジャンプ
    	1   	0   	条件が成立すればﾛﾝｸﾞｼﾞｬﾝﾌﾟ
    	1   	1   	条件が成立しなければﾛﾝｸﾞｼﾞｬﾝﾌﾟ
    */
{
    byte    jt;
    byte    lbl[GoLbl_SIZ + 2];

    GoLblStr(lbl, lblno);
    if (xp == NULL) {
    	GenCondJmp(GO_JMP, ls, neg_f, lbl);
    	return;
    }
    if (xp->e.op != T_COND) {
    	Msg_Err("比較がおかしい");
    	return;
    }
    if (mode == 0)
    	Gen_condMode = 1;
    else
    	Gen_condMode = mode;
    neg_f ^= xp->j.nf;
    if ((jt = xp->j.cond) == 0) {
    	if (mode < 2)
    	    GenCondLAnd(xp->e.lep, ls, neg_f, lbl);
    	else
    	    Msg_Err("orig('|')指定状態で使えない条件が指定された");
    } else {
    	GenCondJmp(jt, ls, neg_f, lbl);
    }
    Gen_condMode = 0;
    return;
}
