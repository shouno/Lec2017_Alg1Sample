#include <stdio.h>
#include <ctype.h>

/*
 *	2-3木(2-3 tree)による集合の実現
 */

typedef int ElementType;	/*  集合の要素の型  */

#define ElementEQ(a,b) ((a) == (b))
#define ElementLT(a,b) ((a) < (b))

struct T23_node {
  enum { node, leaf } child_kind;	/*  ノードか葉かを示す  */
  unsigned short nch;			/*  子の数  */
  union {	/*  ひとつの領域を２種類の用途（木構造，フリーリスト)で共用  */
    struct {
      union {	/*  ひとつの領域を共用：  */
	struct T23_node *SubT[3];	/*  子へのポインタ  */
	ElementType Element[3];		/*  葉の要素  */
      } n_or_l;
      ElementType minST[2];		/*  ノードの場合：２番目と３番目の子の
					    部分木の中の最小の要素  */
    } n;
    struct T23_node *nextfree;	/*  未使用セルをリストとして保持するためのリンク  */
  } l;
};

typedef struct T23_node T23node;

/*  子の番号 i は １〜３  */
#define SubTreeOf(p,i) ((p)->l.n.n_or_l.SubT[i-1])	/*  pの i番目の子の部分木  */
#define minSubTreeOf(p,i) ((p)->l.n.minST[i-2])		/*  その最小の要素  */

#define LeafOf(p,i) ((p)->l.n.n_or_l.Element[i-1])	/*  pの i番目の子の葉の要素  */

/*******************************/
/***** セルの割当てとメモリ管理 *****/
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

T23node *FreeListT23Node = 0;

T23node *GetNewT23Cell()
{
  T23node *ncell;
  int i, n;

  if ( !FreeListT23Node ) {
    n = PAGESIZE / sizeof(struct T23_node);
    ncell = (T23node *) malloc(PAGESIZE);
    for (i = 0; i < n; i++, FreeListT23Node = ncell++)
      ncell->l.nextfree = FreeListT23Node;
  }
  ncell = FreeListT23Node;
  FreeListT23Node = FreeListT23Node->l.nextfree;
  return ncell;
}

void FreeT23Cell(T23node *p)
{
  p->l.nextfree = FreeListT23Node;
  FreeListT23Node = p;
}
/*******************************/
#define HasLeaves(p) ((p)->child_kind == leaf)
#define ElementNULL ((ElementType)0)
/*******************************/
/* Implementation of typical operations on binary search tree */
ElementType min(T23node *T)		/*  木 T の中の最小の要素  */
{
  if (T == NULL) return (ElementType)NULL;  /* error! */
  while ( !HasLeaves(T) ) T = SubTreeOf(T,1);	/*  １番めの子の部分木  */
  return LeafOf(T,1);
}

#define TRUE 1
#define FALSE 0

int member(ElementType x, T23node *T)	/*  要素 x が木 T の中に含まれるか？  */
{
  if (T == NULL) return FALSE;
  if (T->nch == 1) /* Special case of a set consisting of a single element  */
    return ElementEQ(x,LeafOf(T,1));
  while ( !HasLeaves(T) )	/*  葉に辿りつかない間，子の部分木におりていく  */
    if (ElementLT(minSubTreeOf(T,2),x))
      if (T->nch <= 2 || ElementLT(x,minSubTreeOf(T,3))) T = SubTreeOf(T,2);
      else if (ElementEQ(x,minSubTreeOf(T,3))) return TRUE;
      else T = SubTreeOf(T,3);
    else if(ElementEQ(x,minSubTreeOf(T,2))) return TRUE;
    else T = SubTreeOf(T,1);
  /*  子は葉： 要素が x に等しい葉はあるか？ */
  if (ElementEQ(x,LeafOf(T,2))) return TRUE;
  else if (ElementLT(x,LeafOf(T,2))) return ElementEQ(LeafOf(T,1),x);
  else return(T->nch>2 && ElementEQ(x,LeafOf(T,3)));
}

void Insert_sub_newST();
ElementType Insert_sub(), Insert_sub_newnode();

void INSERT(ElementType x, T23node **p)		/*  pが指す木に要素 xを追加  */
{
  ElementType e;
  T23node *q = *p, *n;

  if (q == NULL) {
    *p = q = GetNewT23Cell();
    q->child_kind = leaf;  q->nch = 1;  LeafOf(q,1) = x;
    return;
  } else if (q->nch == 1) {	/*  要素が１個だけの特別な場合  */
    if (ElementEQ(x,LeafOf(q,1))) return;
    else if (ElementLT(x,LeafOf(q,1))) {
      LeafOf(q,2) = minSubTreeOf(q,2) = LeafOf(q,1);	/*  既存の要素を２番目に移動  */
      LeafOf(q,1) = x;
    } else LeafOf(q,2) = minSubTreeOf(q,2) = x;		/* xを２番目の要素として追加  */
    q->nch = 2;
    return;
  }
  e = Insert_sub(x, q, &n);
  if (n != NULL) Insert_sub_newST(q, e, n, p);
}

ElementType Insert_sub(ElementType x, T23node *p, T23node **pn)
{   /* 2-3木 pに要素 xを追加する．p中の最小の要素が変化した場合は新たな最小要素，
       不変の場合は NULL を返す．新たなノードが作られた場合は，*pn に入れて返す  */
  ElementType e1, e2;
  T23node *st;

  if ( HasLeaves(p) ) {		/*  子が葉  */
    if (ElementLT(x,LeafOf(p,2))) {	/*  x < ２番目の子  */
      if (ElementEQ(x,LeafOf(p,1))) ;	    /*  x == １番目の子  */
      else if (p->nch <= 2) {		    /*  子が２つ以下で空きがある  */
	p->nch = 3,  LeafOf(p,3) = minSubTreeOf(p,3) = LeafOf(p,2);
	if (ElementLT(x,LeafOf(p,1))) {
	  LeafOf(p,2) = minSubTreeOf(p,2) = LeafOf(p,1);
	  LeafOf(p,1) = x;
	} else LeafOf(p,2) = x;
      } else {				    /*  子が３つ  */
	e1 = LeafOf(p,2),  e2 = LeafOf(p,3);
	p->nch = 2;				/*  xと１番目の子だけを子とする  */
	if (ElementLT(x,LeafOf(p,1))) {
	  LeafOf(p,2) = minSubTreeOf(p,2) = LeafOf(p,1);
	  LeafOf(p,1) = x;
	} else LeafOf(p,2) = x;
	return Insert_sub_newnode(e1, e2, pn);	/* ２番目と３番目の子で新たなノードを作成 */
      }					/*  x < ２番目の子の場合終了  */
    } else if (ElementEQ(x,LeafOf(p,2))) ;	/*  x == ２番目の子と等しい  */
    else if (p->nch <= 2)		/*  xを3番目の子として追加  */
      p->nch = 3,  LeafOf(p,3) = minSubTreeOf(p,3) = x;
    else if (!ElementEQ(x,LeafOf(p,3))) {
      if (ElementLT(x,LeafOf(p,3))) e1 = x,  e2 = LeafOf(p,3);
      else e1 = LeafOf(p,3),  e2 = x;
      p->nch = 2;
      return Insert_sub_newnode(e1, e2, pn);	/*  xと３番目の子で新たなノードを作成  */
    } else ;					/*  x == ３番目の子と等しい  */
    return ElementNULL;		/*  子が葉の場合終了  */
  }
  if (ElementLT(x,minSubTreeOf(p,2))) {		/*  x < ２番目の子  */
    if (ElementEQ(x,minSubTreeOf(p,1))) ;    /*  x == １番目の子  */
    else {
      e2 = Insert_sub(x,SubTreeOf(p,1),pn);	/*  １番目の子の部分木に追加  */
      if (*pn == NULL) ;	/*  不変  */
      else if (p->nch <= 2) {	/*  子が２つ以下．新しく作られたノードを適当な子として追加  */
	p->nch = 3;
	minSubTreeOf(p,3) = minSubTreeOf(p,2),  SubTreeOf(p,3) = SubTreeOf(p,2);
	minSubTreeOf(p,2) = e2,  SubTreeOf(p,2) = *pn;
	*pn = NULL;
      } else {			/*  ３つの子と新しく作られたノードを２つずつに  */
	e1 = minSubTreeOf(p,2),  st = SubTreeOf(p,2);
	p->nch = 2,  minSubTreeOf(p,2) = e2,  SubTreeOf(p,2) = *pn;
	/*  新しいノードを作成  */
	Insert_sub_newST(st, minSubTreeOf(p,3), SubTreeOf(p,3), pn);
	return e1;
      }
    }
  } else if (ElementEQ(x,minSubTreeOf(p,2))) ;	/*  x = ２番目の子  */
  else if (p->nch <= 2) {		    /*  子が２つ以下で空きがある  */
    e2 = Insert_sub(x,SubTreeOf(p,2),pn);	/*  ２番目の子の部分木に追加  */
    if (*pn != NULL) {		/*  新しく作られたノードを３番目の子として追加  */
      p->nch = 3,  minSubTreeOf(p,3) = e2,  SubTreeOf(p,3) = *pn;
      *pn = NULL;
    }
  } else if (ElementEQ(x,minSubTreeOf(p,3))) ;	/*  x = ３番目の子  */
  else if (ElementLT(x,minSubTreeOf(p,3))) {
    e2 = Insert_sub(x,SubTreeOf(p,2),pn);	/*  ２番目の子の部分木に追加  */
    if (*pn != NULL) {	/*  新しく作られたノードと３番目の子で新しいノードを作成  */
      Insert_sub_newST(*pn, minSubTreeOf(p,3), SubTreeOf(p,3), pn);
      p->nch = 2;	/*  現在のノードの子の数は２つに  */
      return e2;
    }
  } else {
    e2 = Insert_sub(x,SubTreeOf(p,3),pn);	/*  ３番目の子の部分木に追加  */
    if (*pn != NULL) {	/*  ３番目の子と新しく作られたノードで新しいノードを作成  */
      e1 = minSubTreeOf(p,3);
      Insert_sub_newST(SubTreeOf(p,3), e2, *pn, pn);
      p->nch = 2;	/*  現在のノードの子の数は２つに  */
      return e1;
    }
  }
  return ElementNULL;
}

ElementType Insert_sub_newnode(ElementType e1, ElementType e2, T23node **pn)
{   /*  １番目の子 = e1, ２番目の子 = e2  */
  T23node *n;

  *pn = n = GetNewT23Cell();
  n->nch = 2,  n->child_kind = leaf;
  LeafOf(n,1) = e1,  LeafOf(n,2) = e2;
  return e1;
} 

void Insert_sub_newST(T23node *st1, ElementType m2, T23node *st2, T23node **pn)
{   /*  １番目の子 = st1, ２番目の子 = st2, ２番目の部分木の最小の要素 = m2  */
  T23node *n;

  *pn = n = GetNewT23Cell();
  n->nch = 2,  n->child_kind = node;
  SubTreeOf(n,1) = st1,  minSubTreeOf(n,2) = m2,  SubTreeOf(n,2) = st2;
}

/******************/
void DELETE(ElementType x, T23node **p)		/*  pが指す木から要素 xを削除  */
{
  ElementType e;
  T23node *q = *p;
  T23node *Delete_sub();

  if (q == NULL) return;
  else if (q->nch == 1) {
    if (ElementEQ(x,LeafOf(q,1))) {
      FreeT23Cell(q);
      *p = NULL;
    }
    return;
  }
  e = ElementNULL;
  q = Delete_sub(x, q, &e);
  if (q != NULL && !HasLeaves(q)) {
    *p = SubTreeOf(q,1);
    FreeT23Cell(q);
  }
}

#define UpdateMin(n,o) (((n)!=ElementNULL)?(n):(o))

T23node *Delete_sub(ElementType x, T23node *p, ElementType *pmin)
{   /*  2-3木 pから要素 xを削除する．pの最小の要素が変化した場合，新たな最小の
	要素を *pmin に返す（不変の場合は NULL）．子が１個になってしまった場合，
	その子を含む現在のノードを値として返す（そうでない場合 NULL）．  */
  T23node *st1, *st2, *st3;
  ElementType nmin;
  int i;

  if (HasLeaves(p)) {
    if (ElementEQ(x,LeafOf(p,2))) {
      if (--(p->nch) <= 1) return p;
      else LeafOf(p,2) = minSubTreeOf(p,2) = LeafOf(p,3);
    } else if (ElementEQ(x,LeafOf(p,1))) {
      *pmin = LeafOf(p,1) = LeafOf(p,2);
      if (--(p->nch) <= 1) return p;
      else LeafOf(p,2) = minSubTreeOf(p,2) = LeafOf(p,3);
    } else if (p->nch >= 3 && ElementEQ(x,LeafOf(p,3))) p->nch = 2;
    return NULL;
  }
  /*  孫の数の次のようになった場合，=>の右辺のように対処：
   *  1-2-0  => (3)^;	1-2-? => (3,?,0);	1-3-? => (2,2,?)
   *  2-1-0  => (3)^;	2-1-? => (3,?,0);	3-1-? => (2,2,?)
   *  ?-2-1  => (?,3,0);	?-3-1 => (?,2,2)
   */
  if (ElementLT(x,minSubTreeOf(p,2))) {		/*  １番目の部分木から削除  */
    if ((st1 = Delete_sub(x, SubTreeOf(p,1), pmin)) == NULL) ;  /*  削除後も正常  */
    else if ((st2 = SubTreeOf(p,2))->nch >= 3) {  /* 1-3-? => (2,2,?) */
      st1->nch = st2->nch = 2;
      minSubTreeOf(st1,2) = minSubTreeOf(p,2), SubTreeOf(st1,2) = SubTreeOf(st2,1);
      minSubTreeOf(p,2) = minSubTreeOf(st2,2), SubTreeOf(st2,1) = SubTreeOf(st2,2);
      minSubTreeOf(st2,2) = minSubTreeOf(st2,3), SubTreeOf(st2,2) =SubTreeOf(st2,3);
    } else {
      st1->nch = 3;
      minSubTreeOf(st1,2) = minSubTreeOf(p,2), SubTreeOf(st1,2) = SubTreeOf(st2,1);
      minSubTreeOf(st1,3) = minSubTreeOf(st2,2), SubTreeOf(st1,3) =SubTreeOf(st2,2);
      FreeT23Cell(st2);
      if (p->nch >= 3) {  /* 1-2-? => (3,?,0) */
	p->nch = 2;
	minSubTreeOf(p,2) = minSubTreeOf(p,3), SubTreeOf(p,2) = SubTreeOf(p,3);
      } else {  /* 1-2-0 => (3) and ^ */
	p->nch = 1;
	return p;
      }
    }
  } else if (p->nch <= 2 || ElementLT(x,minSubTreeOf(p,3))) {
    /*  ２番目の部分木から削除  */
    /*  2-1-0  => (3) and ^;	2-1-? => (3,?,0);	3-1-? => (2,2,?) */
    st2 = Delete_sub(x, SubTreeOf(p,2), pmin);
    nmin = *pmin;
    *pmin = ElementNULL;
    if (st2 == NULL) {	  /*  削除後も正常  */
      if (nmin != ElementNULL) minSubTreeOf(p,2) = nmin;
      return NULL;
    }
    nmin = UpdateMin(nmin, minSubTreeOf(p,2));
    if ((st1 = SubTreeOf(p,1))->nch >= 3) {  /* 3-1-* => (2,2,*) */
      st1->nch = st2->nch = 2;
      minSubTreeOf(st2,2) = nmin, SubTreeOf(st2,2) = SubTreeOf(st2,1);
      minSubTreeOf(p,2) = minSubTreeOf(st1,3), SubTreeOf(st2,1) = SubTreeOf(st1,3);
    } else {
      st1->nch = 3;
      minSubTreeOf(st1,3) = nmin, SubTreeOf(st1,3) = SubTreeOf(st2,1);
      FreeT23Cell(st2);
      if (p->nch >= 3) {  /* 2-1-? => (3,?,0) */
	p->nch = 2;
	minSubTreeOf(p,2) = minSubTreeOf(p,3), SubTreeOf(p,2) = SubTreeOf(p,3);
      } else {  /* 2-1-0 => (3) and ^ */
	p->nch = 1;
	return p;
      }
    }
  } else {		/*  ３番目の部分木から削除  */
    /*  ?-2-1  => (?,3,0);	?-3-1 => (?,2,2) */
    st3 = Delete_sub(x, SubTreeOf(p,3), pmin);
    nmin = *pmin;
    *pmin = ElementNULL;
    if (st3 == NULL) {  /*  削除後も正常  */
      if (nmin != ElementNULL) minSubTreeOf(p,3) = nmin;
      return NULL;
    }
    nmin = UpdateMin(nmin, minSubTreeOf(p,3));
    if ((st2 = SubTreeOf(p,2))->nch <= 2) {  /* ?-2-1 => (?,3,0) */
      p->nch = 2,  st2->nch = 3;
      minSubTreeOf(st2,3) = nmin, SubTreeOf(st2,3) = SubTreeOf(st3,1);
      FreeT23Cell(st3);
    } else {  /* ?-3-1 => (?,2,2) */
      st2->nch = st3->nch = 3;
      minSubTreeOf(st3,2) = nmin, SubTreeOf(st3,2) = SubTreeOf(st3,1);
      minSubTreeOf(p,3) = minSubTreeOf(st2,3), SubTreeOf(st3,1) = SubTreeOf(st2,3);
    }
  }
  return NULL;
}

/**************************************/
void print_tree_sub(T23node *p)
{
  if (HasLeaves(p)) {
    printf("[%d,%d", LeafOf(p,1), LeafOf(p,2));
    if (p->nch > 2) printf(",%d", LeafOf(p,3));
    printf("]");
  } else {
    printf("(");
    print_tree_sub(SubTreeOf(p,1));
    printf(", <%d>",minSubTreeOf(p,2));
    print_tree_sub(SubTreeOf(p,2));
    if (p->nch > 2) {
      printf(", <%d>",minSubTreeOf(p,3));
      print_tree_sub(SubTreeOf(p,3));
    }
    printf(")");
  }
}

void print_tree(T23node *p)
{
  if (p == NULL) return;
  if (p->nch == 1) printf("[%d]", LeafOf(p,1));
  else print_tree_sub(p);
}
/**************************************/

T23node *pTree = NULL;

int trace = 0;

main(int argc, char **argv)
{
  int i, n;

  for (i = 1; i < argc; i++) {
    char *s = argv[i];

    if (isdigit(s[0])) {
      n = atoi(s);
      INSERT(n, &pTree);
    } else if (s[0] != '-') continue;
    else if (s[1] == 't' || s[1] == 'T') {
      trace = 1 - trace;
      continue;
    } else if ( !isdigit(s[1]) ) continue;
    else {
      n = atoi(&s[1]);
      DELETE(n, &pTree);
    }
    if ( trace ) { print_tree(pTree);  printf("\n"); }
  }
  print_tree(pTree);
  printf("\n");
}
