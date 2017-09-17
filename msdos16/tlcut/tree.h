typedef struct TREENODE {
	struct TREENODE *Link[2];
	int   AvltFlg;
	void *Element;
} TREENODE;

typedef void *(*TREENEW)(void *);
typedef void (*TREEDEL)(void *);
typedef int  (*TREECMP)(void *,void *);

typedef struct TREE {
	TREENODE *Root;
	TREENODE *Node;
	int      Flag;
	TREENEW  NewElement;
	TREEDEL  DelElement;
	TREECMP  CmpElement;
} TREE;

TREE *treeMake(TREENEW NewElement,TREEDEL DelElement,TREECMP CmpElement);
void *treeInsert(TREE *Tree, void *e);
void *treeSearch(TREE *Tree, void *p);
void treeClear(TREE *Tree);
void treeDoAll(TREE *tree, void (*func)(void *));
void tree2dlist(TREE *tp);
void *dlistFirst(TREE *tp);
void *dlistNext(TREE *tp);
void dlistClear(TREE *tp);
