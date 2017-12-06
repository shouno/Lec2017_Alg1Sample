#include <stdio.h>

typedef int ElementType;

enum balance { unknown, left, equal, right };

struct AVL_node {
  ElementType element;
  struct AVL_node *left, *right;
  enum balance balance;
};

typedef struct AVL_node AVLnode;

#define ElementNULL ((ElementType)0)
#define ElementEQ(a,b) ((a)==(b))
#define ElementLT(a,b) ((a)<(b))
#define ElementGT(a,b) ((a)>(b))

/*******************************/
/***** セルの割当てとメモリ管理 *****/
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

AVLnode *FreeListAVLNode = 0;

AVLnode *GetNewAVLCell()
{
  AVLnode *ncell;
  int i, n;

  if ( !FreeListAVLNode ) {
    n = PAGESIZE / sizeof(struct AVL_node);
    ncell = (AVLnode *) malloc(PAGESIZE);
    for (i = 0; i < n; i++, FreeListAVLNode = ncell++)
      ncell->right = FreeListAVLNode;
  }
  ncell = FreeListAVLNode;
  FreeListAVLNode = FreeListAVLNode->right;
  return ncell;
}

void FreeAVLCell(AVLnode *p)
{
  p->right = FreeListAVLNode;
  FreeListAVLNode = p;
}
/*******************************/
/* Implementation of typical operations on binary search tree */

ElementType AVLmin(AVLnode *T)
{
  AVLnode *L;

  if (T == NULL) return ElementNULL;  /* error! */
  while ((L = T->left) != NULL) T = L;
  return T->element;
}

#define TRUE 1
#define FALSE 0

int AVLmember(ElementType x, AVLnode *T)
{
  ElementType e;

  if (T == NULL) return FALSE;
  if (x == (e = T->element)) return TRUE;
  return AVLmember(x, x<e? T->left: T->right);
}
/*******************************/
AVLnode *AVLINSERT(ElementType x, AVLnode **p)
{
  AVLnode *u = *p, *v, *w, *beta1, *beta2;
  ElementType e;

  if (u == NULL) {
    *p = u = GetNewAVLCell();
    u->element = x,  u->left = u->right = NULL, u->balance = equal;
    return u;
  } else if (ElementLT(x, e = u->element)) {
    if ((v = AVLINSERT(x, &u->left)) == NULL) ;
    else if (u->balance == right) u->balance = equal;
    else if (u->balance == equal) {
      u->balance = left;
      return u;
    } else {
      w = v->right;
      if (v->balance == left) {
	v->right = u;  u->left = w;
	u->balance = v->balance = equal;
	*p = v;
      } else {  /* v->balance == right */
	v->right = w->left,  u->left = w->right;
	w->left = v,  w->right = u;
	v->balance = w->balance == right? left: equal;
	u->balance = w->balance == left? right: equal;
	w->balance = equal;
	*p = w;
      }
    }
  } else if (ElementGT(x, e)) {
    if ((v = AVLINSERT(x, &u->right)) == NULL) ;
    else if (u->balance == left) u->balance = equal;
    else if (u->balance == equal) {
      u->balance = right;
      return u;
    } else {
      w = v->left;
      if (v->balance == right) {
	v->left = u;  u->right = w;
	u->balance = v->balance = equal;
	*p = v;
      } else {  /* v->balance == left */
	v->left = w->right,  u->right = w->left;
	w->right = v,  w->left = u;
	v->balance = w->balance == left? right: equal;
	u->balance = w->balance == right? left: equal;
	w->balance = equal;
	*p = w;
      }
    }
  }
  return NULL;
}

int AVLDeleteMin(AVLnode **p, ElementType *pmin)
{
  AVLnode *u = *p, *w, *z;
  int AVLDeleteAdjfromL();

  if (u->left == NULL) {
    *pmin = u->element;  *p = u->right;
    FreeAVLCell(u);
    return 1;
  }
  if (!AVLDeleteMin(&(u->left), pmin)) return 0;
  else return AVLDeleteAdjfromL(p, u);
}

int AVLDeleteAdjfromL(AVLnode **p, AVLnode *u)
{
  AVLnode *w, *z;

  if (u->balance == left) u->balance = equal;
  else if (u->balance == equal) {
    u->balance = right;
    return 0;
  } else if ((w = u->right)->balance == left) {
    *p = z = w->left;
    u->right = z->left;  w->left = z->right;
    u->balance = z->balance == right? left: equal;
    w->balance = z->balance == left? right: equal;
    z->balance = equal;
  } else {
    *p = w;
    u->right = w->left;  w->left = u;
    if (w->balance == equal) {
      u->balance = right;  w->balance = left;
      return 0;
    } else u->balance = w->balance = equal;
  }
  return 1;
}

int AVLDeleteMax(AVLnode **p, ElementType *pmax)
{
  AVLnode *u = *p, *w, *z;
  int AVLDeleteAdjfromR();

  if (u->right == NULL) {
    *pmax = u->element;  *p = u->left;
    FreeAVLCell(u);
    return 1;
  }
  if (!AVLDeleteMax(&(u->right), pmax)) return 0;
  else return AVLDeleteAdjfromR(p, u);
}

int AVLDeleteAdjfromR(AVLnode **p, AVLnode *u)
{
  AVLnode *w, *z;

  if (u->balance == right) u->balance = equal;
  else if (u->balance == equal) {
    u->balance = left;
    return 0;
  } else if ((w = u->left)->balance == right) {
    *p = z = w->right;
    u->left = z->right;  w->right = z->left;
    u->balance = z->balance == left? right: equal;
    w->balance = z->balance == right? left: equal;
    z->balance = equal;
  } else {
    *p = w;
    u->left = w->right;  w->right = u;
    if (w->balance == equal) {
      u->balance = left;  w->balance = right;
      return 0;
    } else u->balance = w->balance = equal;
  }
  return 1;
}

int AVLDELETE(ElementType x, AVLnode **p)
{
  AVLnode *u = *p, *v, *w, *beta1, *beta2;
  ElementType e;

  if (u == NULL) return 0;
  else if (ElementEQ(x, e = u->element)) {
    if (u->right == NULL) *p = u->left;
    else if (u->left == NULL) *p = u->right;
    else if (u->balance == left) {
      if (!AVLDeleteMax(&(u->left), &(u->element))) return 0;
      else return AVLDeleteAdjfromL(p,u);
    } else
      if (!AVLDeleteMin(&(u->right), &(u->element))) return 0;
      else return AVLDeleteAdjfromR(p, u);
    FreeAVLCell(u);
    return 1;
  } else if (ElementLT(x, e)) {
    if (!AVLDELETE(x, &(u->left))) return 0;
    else return AVLDeleteAdjfromL(p, u);
  } else {
    if (!AVLDELETE(x, &(u->right))) return 0;
    else return AVLDeleteAdjfromR(p, u);
  }
  return 1;
}
/**********/
void AVLPrintTree(AVLnode *T, int l)
{
  int i, l1 = l+1;

  if (T == NULL) { printf("<>\n"); return; }
  printf(" %2d", T->element);
  if (T->right) {
    printf(" -( ");
    AVLPrintTree(T->right, l1);
  } else printf("\n");
  if (T->left) {
    for (i = 0; i < l; i++) printf("       ");
    printf("    \\( ");
    AVLPrintTree(T->left, l1);
  }
}
/**********/

AVLnode *pTree = NULL;

static int trace = 0;

main(int argc, char **argv)
{
  int i;
  ElementType n;

  for (i = 1; i < argc; i++) {
    char *s = argv[i];

    if (isdigit(s[0])) {
      n = atoi(s);
      AVLINSERT(n, &pTree);
    } else if (s[0] != '-') continue;
    else if (s[1] == 't' || s[1] == 'T') {
      trace = 1 - trace;
      continue;
    } else if (s[1] == '-') (void) AVLDeleteMin(&pTree, &n);
    else if (s[1] == '+') (void) AVLDeleteMax(&pTree, &n);
    else if ( !isdigit(s[1]) ) continue;
    else {
      n = atoi(&s[1]);
      AVLDELETE(n, &pTree);
    }
    if ( trace ) { AVLPrintTree(pTree, 0); }
  }
  if (!trace) AVLPrintTree(pTree, 0);
}
