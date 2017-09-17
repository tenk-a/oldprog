/*
    AVL–Ø
    1991-1993	M.Kitamura
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

#define _S  static
#if 0
  #define MSGF(x)   (printf x)
#else
  #define MSGF(x)
#endif

/*---------------------------------------------------------------------------*/

_S TREENODE *CurNode;
_S void *Elem;

_S TREECMP CmpElement;
_S TREEDEL DelElement;
_S TREENEW NewElement;
_S int	NewElementFlg;

TREE *treeMake(TREENEW NewElement,TREEDEL DelElement,TREECMP CmpElement)
    /* “ñ•ª–Ø‚ðì¬‚µ‚Ü‚·Bˆø”‚ÍA—v‘f‚Ìì¬,íœ,”äŠr‚Ì‚½‚ß‚ÌŠÖ”‚Ö‚ÌÎß²ÝÀ*/
{
    TREE *p;

    p = malloc(sizeof(TREE));
    if (p) {
    	p->Root = NULL;
    	p->Node = NULL;
    	p->Flag = 0;
    	p->NewElement  = NewElement;
    	p->DelElement  = DelElement;
    	p->CmpElement  = CmpElement;
    }
    return p;
}


_S TREENODE *newNode(void)
{
    TREENODE *sp;

    sp = malloc(sizeof(TREENODE));
    if (sp) {
    	memset(sp,0x00,sizeof(TREENODE));
      /*
    	sp->Link[0] = sp->Link[1] = NULL;
    	sp->AvltFlg = 0;
    	sp->Element = NULL;
      */
    }
    return sp;
}

_S int insertNode(TREENODE *pp, int  lrFlg)
{
    int l,r,nl,nr;
    int  a;
    TREENODE *p,*lp,*lrp,*lrlp,*lrrp;

 MSGF(("pp=%p  ",pp));
    if (pp == NULL)
    	return 0;
    p = pp->Link[lrFlg];
 MSGF(("p=%p  ",p));
    if (p == NULL) {
    	CurNode = p = pp->Link[lrFlg] = newNode();
    	NewElementFlg = 1;
    	p->Element = NewElement(Elem);
 MSGF(("Elem=%p\n",p->Element));
    	return 3;
    }
    a = CmpElement(Elem,p->Element);
 MSGF(("a=%d\n",a));
    if (a == 0) {
    	CurNode = p;
    	return 0;
    }
    if (a < 0) {
    	l = 0;	r = 1;	nl = 1; nr = 2;
    } else {
    	l = 1;	r = 0;	nl = 2; nr = 1;
    }

    a = insertNode(p,l);
    if (a == 0) {
    	return 0;
    }
    if (p->AvltFlg == nr) {
    	p->AvltFlg = 0;
    	return 0;
    } else if (p->AvltFlg == 0) {
    	p->AvltFlg = nl;
    	return p->AvltFlg;
    }
    if (a == nl) {
    	lp = p->Link[l];
    	lrp = lp->Link[r];
    	p->Link[l] = lrp;
    	p->AvltFlg = 0;
    	lp->Link[r] = p;
    	lp->AvltFlg = 0;
    	pp->Link[lrFlg] = lp;
    } else if (a == nr) {
    	lp = p->Link[l];
    	lrp = lp->Link[r];
    	lrlp = lrp->Link[l];
    	lrrp = lrp->Link[r];
    	pp->Link[lrFlg] = lrp;
    	p->Link[l]   = lrrp;
    	lp->Link[r]  = lrlp;
    	lrp->Link[l] = lp;
    	lrp->Link[r] = p;
    	lp->AvltFlg = p->AvltFlg = 0;
    	if (lrp->AvltFlg == nl)
    	    p->AvltFlg = nr;
    	else if (lrp->AvltFlg == nr)
    	    lp->AvltFlg = nl;
    	lrp->AvltFlg = 0;
    } else {
    	printf("PRGERR:insertNode AvltFlg = 3\n");
    	exit(1);
    }
    return 0;
}


void *treeInsert(TREE *Tree, void *e)
    /* —v‘f‚ð–Ø‚É‘}“ü */
{
    static TREENODE Tmp;

    CmpElement	= Tree->CmpElement;
    NewElement	= Tree->NewElement;
    Tmp.Link[0] = Tree->Root;
    NewElementFlg = 0;
    CurNode = NULL;
    Elem = e;
    insertNode(&Tmp, 0);
    Tree->Root = Tmp.Link[0];
    Tree->Node = CurNode;
    Tree->Flag = NewElementFlg;
    if (CurNode)
    	return CurNode->Element;
    return NULL;
}


/*----------------------------------*/
_S TREENODE *searchElement(TREENODE *np, void *p)
{
    int n;

    if (np == NULL)
    	return NULL;
    if ((n = CmpElement(p,np->Element)) == 0)
    	return np;
    if (n < 0 && np->Link[0])
    	return searchElement(np->Link[0],p);
    else if (np->Link[1])
    	return searchElement(np->Link[1],p);
    return NULL;
}

void *treeSearch(TREE *Tree, void *p)
    /* –Ø‚©‚ç—v‘f‚ð’T‚· */
{
    TREENODE *np;

    CmpElement	= Tree->CmpElement;
    Tree->Node = np = searchElement(Tree->Root, p);
    if (np == NULL)
    	return NULL;
    return np->Element;
}


/*----------------------------------*/
_S void delAllNode(TREENODE *np)
{
    if (np == NULL)
    	return;
    if (np->Link[0])
    	delAllNode(np->Link[0]);
    if (np->Link[1])
    	delAllNode(np->Link[1]);
    if (DelElement)
    	DelElement(np->Element);
    free(np);
    return;
}

void treeClear(TREE *Tree)
    /* –Ø‚ðÁ‹Ž‚·‚é */
{
    DelElement	= Tree->DelElement;
    delAllNode(Tree->Root);
    free(Tree);
    return;
}

/*---------------------------------------------------------------------------*/
#if 1
_S void (*DoElem)(void *);

_S void doElement(TREENODE *np)
{
    if (np == NULL)
    	return;
    if (np->Link[0])
    	doElement(np->Link[0]);
    DoElem(np->Element);
    if (np->Link[1])
    	doElement(np->Link[1]);
    return;
}

void treeDoAll(TREE *tree, void (*func)(void *))
    /* –Ø‚Ì‚·‚×‚Ä‚Ì—v‘f‚É‚Â‚¢‚Ä func(void *) ‚ðŽÀs.
    	func‚É‚Í—v‘f‚Ö‚Ìƒ|ƒCƒ“ƒ^‚ª“n‚³‚ê‚é */
{
    DoElem = func;
    doElement(tree->Root);
}
#endif

/*---------------------------------------------------------------------------*/
_S TREENODE *ListCur;
_S TREENODE *ListTop;

_S TREENODE *tree2dlist_sub(TREENODE *dp)
{
    if (dp == NULL)
    	return NULL;
    if (dp->Link[0] == NULL && ListTop == NULL) {
    	ListCur = ListTop = dp;
    } else {
    	if (dp->Link[0])
    	    tree2dlist_sub(dp->Link[0]);
    	if (ListTop == NULL) {
    	    printf("PRGERR:tree2list ‚ª‚¨‚©‚µ‚¢‚¼!");
    	    exit(1);
    	}
    	ListCur->Link[1] = dp;
    	dp->Link[0] = ListCur;
    	ListCur = dp;
    }
    if (dp->Link[1])
    	tree2dlist_sub(dp->Link[1]);
    return dp;
}

void tree2dlist(TREE *tp)
    /* “ñ•ª–Ø‚ð‘o•ûŒüƒŠƒXƒg‚É•ÏŠ· */
{
    ListTop = NULL;
    ListCur = NULL;
    tree2dlist_sub(tp->Root);
    tp->Node = tp->Root = ListTop;
}

void *dlistFirst(TREE *tp)
    /* ‘o•ûŒüƒŠƒXƒg‚Ìæ“ª‚ÉƒJ[ƒ\ƒ‹‚ðˆÚ‚· */
{
    tp->Node = tp->Root;
    if (tp->Node)
    	return tp->Node->Element;
    return NULL;
}

void *dlistNext(TREE *tp)
    /* ŽŸ‚Ì—v‘f‚ÖˆÚ“® */
{
    if (tp->Node) {
    	tp->Node = tp->Node->Link[1];
    	if (tp->Node)
    	    return tp->Node->Element;
    }
    return NULL;
}

void dlistClear(TREE *tp)
    /* ‘o•ûŒüƒŠƒXƒg(‚à‚Æ‚à‚Æ‚Í–Øj‚ðÁ‹Ž */
{
    TREENODE *p;
    TREENODE *q;

    p = tp->Root;
    while (p) {
    	q = p->Link[1];
    	if (DelElement)
    	    DelElement(p->Element);
    	free(p);
    	p = q;
    }
    free(tp);
}


/*---------------------------------------------------------------------------*/

