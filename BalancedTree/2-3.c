#include <stdio.h>
#include <ctype.h>

/*
 *	2-3��(2-3 tree)�ˤ�뽸��μ¸�
 */

typedef int ElementType;	/*  ��������Ǥη�  */

#define ElementEQ(a,b) ((a) == (b))
#define ElementLT(a,b) ((a) < (b))

struct T23_node {
  enum { node, leaf } child_kind;	/*  �Ρ��ɤ��դ��򼨤�  */
  unsigned short nch;			/*  �Ҥο�  */
  union {	/*  �ҤȤĤ��ΰ�򣲼�������ӡ��ڹ�¤���ե꡼�ꥹ��)�Ƕ���  */
    struct {
      union {	/*  �ҤȤĤ��ΰ���ѡ�  */
	struct T23_node *SubT[3];	/*  �ҤؤΥݥ���  */
	ElementType Element[3];		/*  �դ�����  */
      } n_or_l;
      ElementType minST[2];		/*  �Ρ��ɤξ�硧�����ܤȣ����ܤλҤ�
					    ��ʬ�ڤ���κǾ�������  */
    } n;
    struct T23_node *nextfree;	/*  ̤���ѥ����ꥹ�ȤȤ����ݻ����뤿��Υ��  */
  } l;
};

typedef struct T23_node T23node;

/*  �Ҥ��ֹ� i �� ������  */
#define SubTreeOf(p,i) ((p)->l.n.n_or_l.SubT[i-1])	/*  p�� i���ܤλҤ���ʬ��  */
#define minSubTreeOf(p,i) ((p)->l.n.minST[i-2])		/*  ���κǾ�������  */

#define LeafOf(p,i) ((p)->l.n.n_or_l.Element[i-1])	/*  p�� i���ܤλҤ��դ�����  */

/*******************************/
/***** ����γ����Ƥȥ������ *****/
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
ElementType min(T23node *T)		/*  �� T ����κǾ�������  */
{
  if (T == NULL) return (ElementType)NULL;  /* error! */
  while ( !HasLeaves(T) ) T = SubTreeOf(T,1);	/*  ���֤�λҤ���ʬ��  */
  return LeafOf(T,1);
}

#define TRUE 1
#define FALSE 0

int member(ElementType x, T23node *T)	/*  ���� x ���� T ����˴ޤޤ�뤫��  */
{
  if (T == NULL) return FALSE;
  if (T->nch == 1) /* Special case of a set consisting of a single element  */
    return ElementEQ(x,LeafOf(T,1));
  while ( !HasLeaves(T) )	/*  �դ�é��Ĥ��ʤ��֡��Ҥ���ʬ�ڤˤ���Ƥ���  */
    if (ElementLT(minSubTreeOf(T,2),x))
      if (T->nch <= 2 || ElementLT(x,minSubTreeOf(T,3))) T = SubTreeOf(T,2);
      else if (ElementEQ(x,minSubTreeOf(T,3))) return TRUE;
      else T = SubTreeOf(T,3);
    else if(ElementEQ(x,minSubTreeOf(T,2))) return TRUE;
    else T = SubTreeOf(T,1);
  /*  �Ҥ��ա� ���Ǥ� x ���������դϤ��뤫�� */
  if (ElementEQ(x,LeafOf(T,2))) return TRUE;
  else if (ElementLT(x,LeafOf(T,2))) return ElementEQ(LeafOf(T,1),x);
  else return(T->nch>2 && ElementEQ(x,LeafOf(T,3)));
}

void Insert_sub_newST();
ElementType Insert_sub(), Insert_sub_newnode();

void INSERT(ElementType x, T23node **p)		/*  p���ؤ��ڤ����� x���ɲ�  */
{
  ElementType e;
  T23node *q = *p, *n;

  if (q == NULL) {
    *p = q = GetNewT23Cell();
    q->child_kind = leaf;  q->nch = 1;  LeafOf(q,1) = x;
    return;
  } else if (q->nch == 1) {	/*  ���Ǥ����Ĥ��������̤ʾ��  */
    if (ElementEQ(x,LeafOf(q,1))) return;
    else if (ElementLT(x,LeafOf(q,1))) {
      LeafOf(q,2) = minSubTreeOf(q,2) = LeafOf(q,1);	/*  ��¸�����Ǥ����ܤ˰�ư  */
      LeafOf(q,1) = x;
    } else LeafOf(q,2) = minSubTreeOf(q,2) = x;		/* x�����ܤ����ǤȤ����ɲ�  */
    q->nch = 2;
    return;
  }
  e = Insert_sub(x, q, &n);
  if (n != NULL) Insert_sub_newST(q, e, n, p);
}

ElementType Insert_sub(ElementType x, T23node *p, T23node **pn)
{   /* 2-3�� p������ x���ɲä��롥p��κǾ������Ǥ��Ѳ��������Ͽ����ʺǾ����ǡ�
       ���Ѥξ��� NULL ���֤��������ʥΡ��ɤ����줿���ϡ�*pn ��������֤�  */
  ElementType e1, e2;
  T23node *st;

  if ( HasLeaves(p) ) {		/*  �Ҥ���  */
    if (ElementLT(x,LeafOf(p,2))) {	/*  x < �����ܤλ�  */
      if (ElementEQ(x,LeafOf(p,1))) ;	    /*  x == �����ܤλ�  */
      else if (p->nch <= 2) {		    /*  �Ҥ����İʲ��Ƕ���������  */
	p->nch = 3,  LeafOf(p,3) = minSubTreeOf(p,3) = LeafOf(p,2);
	if (ElementLT(x,LeafOf(p,1))) {
	  LeafOf(p,2) = minSubTreeOf(p,2) = LeafOf(p,1);
	  LeafOf(p,1) = x;
	} else LeafOf(p,2) = x;
      } else {				    /*  �Ҥ�����  */
	e1 = LeafOf(p,2),  e2 = LeafOf(p,3);
	p->nch = 2;				/*  x�ȣ����ܤλҤ�����ҤȤ���  */
	if (ElementLT(x,LeafOf(p,1))) {
	  LeafOf(p,2) = minSubTreeOf(p,2) = LeafOf(p,1);
	  LeafOf(p,1) = x;
	} else LeafOf(p,2) = x;
	return Insert_sub_newnode(e1, e2, pn);	/* �����ܤȣ����ܤλҤǿ����ʥΡ��ɤ���� */
      }					/*  x < �����ܤλҤξ�罪λ  */
    } else if (ElementEQ(x,LeafOf(p,2))) ;	/*  x == �����ܤλҤ�������  */
    else if (p->nch <= 2)		/*  x��3���ܤλҤȤ����ɲ�  */
      p->nch = 3,  LeafOf(p,3) = minSubTreeOf(p,3) = x;
    else if (!ElementEQ(x,LeafOf(p,3))) {
      if (ElementLT(x,LeafOf(p,3))) e1 = x,  e2 = LeafOf(p,3);
      else e1 = LeafOf(p,3),  e2 = x;
      p->nch = 2;
      return Insert_sub_newnode(e1, e2, pn);	/*  x�ȣ����ܤλҤǿ����ʥΡ��ɤ����  */
    } else ;					/*  x == �����ܤλҤ�������  */
    return ElementNULL;		/*  �Ҥ��դξ�罪λ  */
  }
  if (ElementLT(x,minSubTreeOf(p,2))) {		/*  x < �����ܤλ�  */
    if (ElementEQ(x,minSubTreeOf(p,1))) ;    /*  x == �����ܤλ�  */
    else {
      e2 = Insert_sub(x,SubTreeOf(p,1),pn);	/*  �����ܤλҤ���ʬ�ڤ��ɲ�  */
      if (*pn == NULL) ;	/*  ����  */
      else if (p->nch <= 2) {	/*  �Ҥ����İʲ������������줿�Ρ��ɤ�Ŭ���ʻҤȤ����ɲ�  */
	p->nch = 3;
	minSubTreeOf(p,3) = minSubTreeOf(p,2),  SubTreeOf(p,3) = SubTreeOf(p,2);
	minSubTreeOf(p,2) = e2,  SubTreeOf(p,2) = *pn;
	*pn = NULL;
      } else {			/*  ���ĤλҤȿ��������줿�Ρ��ɤ򣲤Ĥ��Ĥ�  */
	e1 = minSubTreeOf(p,2),  st = SubTreeOf(p,2);
	p->nch = 2,  minSubTreeOf(p,2) = e2,  SubTreeOf(p,2) = *pn;
	/*  �������Ρ��ɤ����  */
	Insert_sub_newST(st, minSubTreeOf(p,3), SubTreeOf(p,3), pn);
	return e1;
      }
    }
  } else if (ElementEQ(x,minSubTreeOf(p,2))) ;	/*  x = �����ܤλ�  */
  else if (p->nch <= 2) {		    /*  �Ҥ����İʲ��Ƕ���������  */
    e2 = Insert_sub(x,SubTreeOf(p,2),pn);	/*  �����ܤλҤ���ʬ�ڤ��ɲ�  */
    if (*pn != NULL) {		/*  ���������줿�Ρ��ɤ����ܤλҤȤ����ɲ�  */
      p->nch = 3,  minSubTreeOf(p,3) = e2,  SubTreeOf(p,3) = *pn;
      *pn = NULL;
    }
  } else if (ElementEQ(x,minSubTreeOf(p,3))) ;	/*  x = �����ܤλ�  */
  else if (ElementLT(x,minSubTreeOf(p,3))) {
    e2 = Insert_sub(x,SubTreeOf(p,2),pn);	/*  �����ܤλҤ���ʬ�ڤ��ɲ�  */
    if (*pn != NULL) {	/*  ���������줿�Ρ��ɤȣ����ܤλҤǿ������Ρ��ɤ����  */
      Insert_sub_newST(*pn, minSubTreeOf(p,3), SubTreeOf(p,3), pn);
      p->nch = 2;	/*  ���ߤΥΡ��ɤλҤο��ϣ��Ĥ�  */
      return e2;
    }
  } else {
    e2 = Insert_sub(x,SubTreeOf(p,3),pn);	/*  �����ܤλҤ���ʬ�ڤ��ɲ�  */
    if (*pn != NULL) {	/*  �����ܤλҤȿ��������줿�Ρ��ɤǿ������Ρ��ɤ����  */
      e1 = minSubTreeOf(p,3);
      Insert_sub_newST(SubTreeOf(p,3), e2, *pn, pn);
      p->nch = 2;	/*  ���ߤΥΡ��ɤλҤο��ϣ��Ĥ�  */
      return e1;
    }
  }
  return ElementNULL;
}

ElementType Insert_sub_newnode(ElementType e1, ElementType e2, T23node **pn)
{   /*  �����ܤλ� = e1, �����ܤλ� = e2  */
  T23node *n;

  *pn = n = GetNewT23Cell();
  n->nch = 2,  n->child_kind = leaf;
  LeafOf(n,1) = e1,  LeafOf(n,2) = e2;
  return e1;
} 

void Insert_sub_newST(T23node *st1, ElementType m2, T23node *st2, T23node **pn)
{   /*  �����ܤλ� = st1, �����ܤλ� = st2, �����ܤ���ʬ�ڤκǾ������� = m2  */
  T23node *n;

  *pn = n = GetNewT23Cell();
  n->nch = 2,  n->child_kind = node;
  SubTreeOf(n,1) = st1,  minSubTreeOf(n,2) = m2,  SubTreeOf(n,2) = st2;
}

/******************/
void DELETE(ElementType x, T23node **p)		/*  p���ؤ��ڤ������� x����  */
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
{   /*  2-3�� p�������� x�������롥p�κǾ������Ǥ��Ѳ�������硤�����ʺǾ���
	���Ǥ� *pmin ���֤������Ѥξ��� NULL�ˡ��Ҥ����ĤˤʤäƤ��ޤä���硤
	���λҤ�ޤ฽�ߤΥΡ��ɤ��ͤȤ����֤��ʤ����Ǥʤ���� NULL�ˡ�  */
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
  /*  ¹�ο��μ��Τ褦�ˤʤä���硤=>�α��դΤ褦���н衧
   *  1-2-0  => (3)^;	1-2-? => (3,?,0);	1-3-? => (2,2,?)
   *  2-1-0  => (3)^;	2-1-? => (3,?,0);	3-1-? => (2,2,?)
   *  ?-2-1  => (?,3,0);	?-3-1 => (?,2,2)
   */
  if (ElementLT(x,minSubTreeOf(p,2))) {		/*  �����ܤ���ʬ�ڤ�����  */
    if ((st1 = Delete_sub(x, SubTreeOf(p,1), pmin)) == NULL) ;  /*  ����������  */
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
    /*  �����ܤ���ʬ�ڤ�����  */
    /*  2-1-0  => (3) and ^;	2-1-? => (3,?,0);	3-1-? => (2,2,?) */
    st2 = Delete_sub(x, SubTreeOf(p,2), pmin);
    nmin = *pmin;
    *pmin = ElementNULL;
    if (st2 == NULL) {	  /*  ����������  */
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
  } else {		/*  �����ܤ���ʬ�ڤ�����  */
    /*  ?-2-1  => (?,3,0);	?-3-1 => (?,2,2) */
    st3 = Delete_sub(x, SubTreeOf(p,3), pmin);
    nmin = *pmin;
    *pmin = ElementNULL;
    if (st3 == NULL) {  /*  ����������  */
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
