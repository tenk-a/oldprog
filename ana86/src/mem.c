#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <alloc.h>
#include "anadef.h"

#ifdef DEBUG
#define ET_TBL_SIZ		(1024 * 1 / sizeof(Et_t))
#define ST_TBL_SIZ		(1024 * 1 / sizeof(St_t))
#else
#define ET_TBL_SIZ		(1024 * 32 / sizeof(Et_t))
#define ST_TBL_SIZ		(1024 * 63 / sizeof(St_t))
#endif

/*----------------------------- function ------------------------------------*/
static Et_t_fp Et_enpTop;
static Et_t_fp Et_enpBtm;
static Et_t_fp Et_top;

static	Et_t_fp Et_Init0(void)
{
	int 	i;
	void far *p;
	Et_t_fp s;
	Et_t_fp tbl;

	tbl = calloc(ET_TBL_SIZ, sizeof(Et_t));
	if (tbl == NULL) {
		Msg_Err("メモリが足りません");
#if 0
		Ana_err = ANAERR_MEM;
		return NULL;
#else
		exit(1);
#endif
	}
	MSGF(("Et_Init0(%d個)\n", ET_TBL_SIZ));
	p = NULL;
	i = ET_TBL_SIZ;
	do {
		s = tbl + i;
		s->e.next = p;
		p = s;
	} while (--i >= 0);
	if (Et_enpBtm)
		Et_enpBtm->e.next = s;
	Et_enpBtm = tbl + ET_TBL_SIZ;
	MSGF(("Et_Init  %lx = %lx(next %lx)  botom %lx(next %lx)\n",
		  tbl, s, s->e.next, Et_enpBtm, Et_enpBtm->e.next));
	return s;
}

Et_t_fp Et_New(void)
{
	Et_t_fp ns;

	if (Et_enpTop == NULL) {
		Msg_PrgErr("Etメモリ管理でエラー");
		exit(1);
	}
	if (Et_enpTop->e.next == NULL) {
#if 0
		if ((Et_enpTop->e.next = Et_Init0()) == NULL)
			return NULL;
#else
		Et_enpTop->e.next = Et_Init0();
#endif
#ifndef DEBUG
		Msg_Err("メモリが足りません");
		exit(1);
#endif
	}
	ns = Et_enpTop;
	Et_enpTop = Et_enpTop->e.next;
	memset(ns, 0, sizeof(Et_t));
	Et_top->e.next = ns;
	(Et_top = ns)->e.next = NULL;
	return Et_top;
}

int 	Et_Init(void)
{
	Et_enpBtm = NULL;
	Et_top = Et_Init0();
 /* if (Et_top == NULL) return 1; */
	Et_enpTop = Et_top->e.next;
	Et_top->e.next = NULL;
	return 0;
}

Et_t_fp Et_Sav(void)
{
	return Et_top;
}

#if 0
void	Et_Free(Et_t_fp p)
{								/* 削除するラベル */
	Et_enpBtm->e.next = p;
	Et_enpBtm = p;
	p->e.next = NULL;
 /* memset(St_enpBtm,0,sizeof(Et_t)); */
}
#endif

void	Et_Frees(Et_t_fp ep)
{
	if (ep == NULL)
		return;
	Et_enpBtm->e.next = ep->e.next;
	if (ep->e.next) {
		Et_enpBtm = Et_top;
		Et_enpBtm->e.next = NULL;
	}
	Et_top = ep;
	Et_top->e.next = NULL;
}


/*------------------------------------------------------------------------*/

/****/
static St_t_fp St_enpTop;
static St_t_fp St_enpBtm;
static St_t_fp St_top;
static St_t_fp St_ptr;
static byte_fp St_name;
static int St_nodeCnt;

static	St_t_fp St_Init0(void)
{
	int 	i;
	void far *p;
	St_t_fp s;
	St_t_fp tbl;

	tbl = calloc(ST_TBL_SIZ + 2, sizeof(St_t));
	if (tbl == NULL) {
		Msg_Err("ｼﾝﾎﾞﾙ･ﾃｰﾌﾞﾙのためのメモリがありません");
#if 0
		Ana_err = ANAERR_MEM;
		return NULL;
#else
		exit(1);
#endif
	}
	MSGF(("St_Init0(%d個)\n", ST_TBL_SIZ));
	p = NULL;
	i = ST_TBL_SIZ;
	do {
		s = tbl + i;
		s->c.next = p;
		p = s;
	} while (--i >= 0);
	if (St_enpBtm)
		St_enpBtm->c.next = tbl;
	St_enpBtm = tbl + ST_TBL_SIZ;
	MSGF(("St_Init  %lx = %lx(next %lx)  botom %lx(next %lx)\n",
		  tbl, s, s->c.next, St_enpBtm, St_enpBtm->c.next));
	return s;
}

int 	St_Init(void)
{
	St_enpBtm = NULL;
	St_top = St_Init0();
#if 0
	if (St_top == NULL)
		return 1;
#endif
	St_enpTop = St_top->c.next;
	St_top->c.next = NULL;
	return 0;
}

St_t_fp St_New(void)
{
	St_t_fp ns;

	if (St_enpTop == NULL) {
		Msg_PrgErr("Stメモリ管理でエラー");
		exit(1);
	}
	if (St_enpTop->c.next == NULL) {
#if 0
		if ((St_enpTop->c.next = St_Init0()) == NULL)
			return NULL;
#else
		St_enpTop->c.next = St_Init0();
#endif
#ifndef DEBUG
		Msg_Err("メモリが足りません");
		exit(1);
#endif
	}
	ns = St_enpTop;
	St_enpTop = St_enpTop->c.next;
	memset(ns, 0, sizeof(St_t));
	return ns;
}

#if 1
#define MSGFF(x)
#else
#define MSGFF(x)	(fprintf x)
#endif
#if 0

void	St_Chk1(St_t_fp sp, int lr, int n)
{
	int 	i;

	if (sp == NULL)
		return;
	if (sp->n.lnk[0])
		St_Chk1(sp->n.lnk[0], 0, n + 1);
	for (i = 0; i < n; i++)
		fprintf(Err_File, "    ");
	fprintf(Err_File, "%2d%s %s\t(%s) L:%lx R:%lx\n", n, (lr == 0) ? "L" : "R",
			sp->v.name,
			(sp->v.flg == 0) ? "E" :
			(sp->v.flg == 1) ? "L" :
			(sp->v.flg == 2) ? "R" : "?",
			sp->n.lnk[0], sp->n.lnk[1]);
	if (sp->n.lnk[1])
		St_Chk1(sp->n.lnk[1], 1, n + 1);
}

#endif
#if 1
static byte   *StrPf(int f)
{
	static byte bb[20];

	if (f == 0 || f == 1 || f == 2)
		return (f == 0) ? "E" : (f == 1) ? "L" : "R";
	sprintf(bb, "%d", f);
	return bb;
}

int 	St_Chk(St_t_fp sp, int lr, int n, int nn)
{
	int 	i, j, k;

	if (sp == NULL)
		return 0;
	if (nn) {
		for (i = 0; i < n; i++)
			fprintf(Err_File, "    ");
		fprintf(Err_File, "%2d%s前 %s\t(%s) L:%lx R:%lx\n",
				n,
				(lr == 0) ? "L" : "R",
				sp->v.name,
				(sp->v.flg == 0) ? "E" :
				(sp->v.flg == 1) ? "L" :
				(sp->v.flg == 2) ? "R" : "?",
				sp->n.lnk[0], sp->n.lnk[1]);
	}
	k = j = 0;
	if (sp->n.lnk[0])
		k = St_Chk(sp->n.lnk[0], 0, n + 1, nn);
	if (sp->n.lnk[1])
		j = St_Chk(sp->n.lnk[1], 1, n + 1, nn);
	i = k - j;
	i = (i == 0) ? 0 : (i == 1) ? 1 : (i == -1) ? 2 : -1;
	if (nn || i != sp->v.flg) {
		for (nn = 0; nn < n; nn++)
			fprintf(Err_File, "    ");
		fprintf(Err_File, "%2d%s後 %s\t(%s) L:%d R:%d %s\n", n,
				(lr == 0) ? "L" : "R",
				sp->v.name, StrPf(i), k, j, (i == sp->v.flg) ? "" : "ﾌﾗｸﾞ･ｴﾗｰ");
	}
	if (k > j)
		return k + 1;
	else
		return j + 1;
}

#endif
#if 0
void	StrKai(St_t_fp pp, St_t_fp p, byte far *str, word n, word nl, word f, word a)
{
	MSGFF((Err_File, "\t%s L[%s] R[%s]\n",
		   p->v.name, p->v.lnk[0]->v.name, p->v.lnk[1]->v.name));
	MSGFF((Err_File, "\t%d:[%s] f(%s) a(%s) -> (%s) %s\n", n,
		   StrPf(nl), StrPf(f), StrPf(a), StrPf(p->n.flg), str));
	MSGFF((Err_File, "pp[%lx] p[%lx] lp[%lx] rp[%lx]\n",
		   pp, p, p->n.lnk[0], p->n.lnk[1]));
	MSGFF((Err_File, "llp[%lx] lrp[%lx] rlp[%lx] rrp[%lx]\n",
		   p->n.lnk[0]->n.lnk[0],
		   p->n.lnk[0]->n.lnk[1],
		   p->n.lnk[1]->n.lnk[0],
		   p->n.lnk[1]->n.lnk[1]));
}

#endif

static int St_Ins2(St_t_fp pp, int lr)
{
	byte	l, r, nl, nr;
	int 	a;
	St_t_fp p, lp, lrp, lrlp, lrrp;
#if 0
	int 	n;
#endif

	if (pp == NULL)
		return 0;
#if 0
	n = ++St_nodeCnt;
#endif
	p = pp->n.lnk[lr];
	if (p == NULL) {
		St_ptr = pp->n.lnk[lr] = St_New();
		strcpy(St_ptr->v.name, St_name);
#if 0
		MSGFF((Err_File, "%d: %s\n", n, St_ptr->v.name));
#endif
		return 3;
	}
	if ((a = strcmp(St_name, p->v.name)) == 0) {
		Msg_Err("多重定義しようとした");
		return 0;
	}
	if (a < 0) {
		l = 0;
		r = 1;
		nl = 1;
		nr = 2;
	} else {
		l = 1;
		r = 0;
		nl = 2;
		nr = 1;
	}

	if ((a = St_Ins2(p, l)) == 0) {
#if 0
		MSGFF((Err_File, "\t%d:[%s] f(%s) a(%s) -> (%s)\n", n,
			   StrPf(nl), StrPf(a), StrPf(a), StrPf(p->n.flg)));
#endif
		return 0;
	}
	if (p->n.flg == nr) {
		p->n.flg = 0;
#if 0
		MSGFF((Err_File, "\t%d:[%s] f(%s) a(%s) -> (%s)\n", n,
			   StrPf(nl), StrPf(a), StrPf(a), StrPf(p->n.flg)));
#endif
		return 0;
	} else if (p->n.flg == 0) {
		p->n.flg = nl;
#if 0
		MSGFF((Err_File, "\t%d:[%s] f(%s) a(%s) -> (%s)\n", n,
			   StrPf(nl), StrPf(a), StrPf(a), StrPf(p->n.flg)));
#endif
		return p->n.flg;
	}
	lp = p->n.lnk[l];
	lrp = lp->n.lnk[r];
	if (a == nl) {
		p->n.lnk[l] = lrp;
		p->n.flg = 0;
		lp->n.lnk[r] = p;
		lp->n.flg = 0;
		pp->n.lnk[lr] = lp;
#if 0
		StrKai(pp, lp, "回転1", n, nl, a, a);
		St_Chk(lp, lr, 1, 0);
#endif
	} else if (a == nr) {
		lrlp = lrp->n.lnk[l];
		lrrp = lrp->n.lnk[r];
		pp->n.lnk[lr] = lrp;
		p->n.lnk[l] = lrrp;
		lp->n.lnk[r] = lrlp;
		lrp->n.lnk[l] = lp;
		lrp->n.lnk[r] = p;
		lp->n.flg = p->n.flg = 0;
		if (lrp->n.flg == nl)
			p->n.flg = nr;
		else if (lrp->n.flg == nr)
			lp->n.flg = nl;
		lrp->n.flg = 0;
#if 0
		StrKai(lrp, p, "回転2R", n, nl, a, a);
		St_Chk(p, lr, 1, 0);
		StrKai(lrp, lp, "回転2L", n, nl, a, a);
		St_Chk(lp, lr, 1, 0);
		StrKai(pp, lrp, "回転2", n, nl, a, a);
		St_Chk(lrp, lr, 1, 0);
#endif
	} else {
		Msg_PrgErr("St_Ins2 flg = 3");
	}
	return 0;
}

St_t_fp St_Ins(byte_fp name, St_t_fp far *root)
{
	struct ST_T_NODE dmy;

	St_nodeCnt = 0;
	if (*root == NULL) {
		MSG("St_Ins (root)");
		St_ptr = St_New();
		strcpy(St_ptr->v.name, name);
		*root = St_ptr;
	} else {
		MSG("St_Ins");
		St_name = name;
		St_ptr = NULL;
		dmy.lnk[0] = *root;
		St_Ins2((St_t_fp) (&dmy), 0);
		*root = dmy.lnk[0];
	}
	return St_ptr;
}

St_t_fp St_Search(byte_fp name, St_t_fp root)
{
	int 	a;
	St_t_fp sp;

	sp = root;
	while (sp) {
		if ((a = strcmp(name, sp->v.name)) == 0)
			return sp;
		sp = sp->n.lnk[(a > 0)];
	}
	return NULL;
}

static void St_Free0(St_t_fp sp)
{								/* 削除するroot */
	if (sp == NULL)
		return;
	if (sp->n.lnk[0])
		St_Free0(sp->n.lnk[0]);
	if (sp->n.lnk[1])
		St_Free0(sp->n.lnk[1]);
	if ((sp->v.tok == T_STRUCT /* ||sp->v.tok == T_GROUP */ ) && sp->v.st)
		St_Free0(sp->v.st);
	St_enpBtm->n.lnk[0] = sp;
	St_enpBtm = sp;
	St_enpBtm->n.lnk[0] = NULL;
}

#if 1
void	St_Free(St_t_fp sp)
{
  #if 0
	St_Chk1(sp, 0, 0);
  #endif
	St_Chk(sp, 0, 0, 0);
	St_Free0(sp);
}

#endif

#if 0

St_t_fp St_Sav(void)
{
	return St_top;
}

void	St_Frees(St_t_fp sp)
{
	if (sp == NULL)
		return;
	St_enpBtm->c.next = sp->c.next;
	St_enpBtm = St_top;
	St_enpBtm->c.next = NULL;
	St_top = sp;
	St_top->c.next = NULL;
}

#endif


/*---------------------------------------------------------------------------*/
#ifdef MACR
#define MACBUF_SIZMAX 0x4000
byte_fp Mac_buf, Mac_ptr;
word	Mac_siz;

void	Mac_Init(void)
{
	Mac_buf = calloc(MACBUF_SIZMAX, 1);
	if (Mac_buf == NULL) {
		Msg_Err("メモリが足りません");
	}
	Mac_siz = 0;
	Mac_ptr = Mac_buf;
}

byte_fp Mac_Pos(void)
{
	return Mac_ptr;
}

void	Mac_Putc(word c)
{
	if (Mac_siz > MACBUF_SIZMAX - 2) {
		Msg_Err("マクロ文字列バッファがあふれました");
		exit(1);
	}
	if (c > 0xff) {
		*Mac_ptr++ = c / 0x100;
		*Mac_ptr++ = c % 0x100;
		Mac_siz += 2;
	} else {
		*Mac_ptr++ = c;
		Mac_siz++;
	}
	*Mac_ptr = '\0';
	return;
}

#endif
