#include <stdio.h>
#include <stdlib.h>

typedef char ElementType;

struct BT_node {
    ElementType element;
    struct BT_node *left, *right;
};

typedef struct BT_node BTnode;


BTnode *GetNewBTCell();
void FreeBTCell(BTnode *p);


/* Implementation of typical operations on binary search tree */

ElementType minElm(BTnode *T)
{
    BTnode *L;

    if (T == NULL) return (ElementType)NULL;  /* error! */
    while ((L = T->left) != NULL) T = L;  // 最も左の値を探索．再帰なしで
    return T->element;
}


BTnode* minNode(BTnode *T)
{
    BTnode *L;

    if (T == NULL) return NULL;  /* error! */
    while ((L = T->left) != NULL) T = L;  // 最も左の値を探索．再帰なしで
    return T;
}

	

#define TRUE 1
#define FALSE 0

int IsLeaf(BTnode *T)
{
	if(T == NULL)	return FALSE;
	if(T->left == NULL && T->right==NULL)	return TRUE;
	else return FALSE;
}


int member(ElementType x, BTnode *T)
{
	// x が2分探索木 T にデータとして存在しているか？
    /* 再帰を使った場合は下記を用い，while 以下をコメントアウト
	  ElementType e;
	  if (T == NULL) return FALSE;
	  if (x == (e = T->element)) return TRUE;
	  return member(x, x<e? T->left: T->right);
    */
    while (T != NULL)
        if (x == T->element) return TRUE;
        else if (x < T->element) T = T->left;
        else T = T->right;
    return FALSE;
}


// INSERT の実装方法2つ．原理は同じだけど呼び出し方が違うので注意(main関数参照)
// 直接探し出して付け加える
BTnode *INSERT(ElementType x, BTnode **p)
{
    ElementType e;
    BTnode *q = *p;

    for (; q != NULL; q = *p)
        if ((e = q->element) == x) return NULL;
		else if (x < e) p = &q->left;
		else p = &q->right;
    *p = q = GetNewBTCell();
    q->element = x,  q->left = q->right = NULL;
    return q;
}


// 再帰を使った実装
BTnode *insert(ElementType x, BTnode *p)
{
	ElementType e;
	if(p == NULL){
		BTnode *q = GetNewBTCell();
		q->element = x;
		q->left = q->right = NULL;
		return q;
	}
	e = p->element;
	if(x < e){ // x が小さい
		p->left = insert(x, p->left);
	}
	else if(x > e){
		p->right = insert(x, p->right);
	}
	return p;
}


ElementType DeleteMin(BTnode **p)
{
    BTnode *q = *p;
    ElementType x;

    if (q == NULL) return (ElementType)NULL;
    /* error case. required to treat totally-empty tree */
    for (; q->left != NULL; q = q->left) p = &q->left;
    x = q->element;
    /*
	  if (q->right == NULL) {
	  *p = NULL;
	  FreeBTCell(q);
	  } else q->element = DeleteMin(&q->right);
    */
    *p = q->right;
    FreeBTCell(q);
    return x;
}


// DELETE の実装方法2つ．原理は同じだけど呼び出し方が違うので注意(main関数参照)
int DELETE(ElementType x, BTnode **p)
{
    ElementType e;
    BTnode *q = *p;

    for (; q != NULL; q = *p)
        if (x == (e = q->element)) {
			if(q->right == NULL) *p = q->left;
			else if(q->left == NULL) *p = q->right;
			else {
				q->element = DeleteMin(&q->right);
				return TRUE;
			}
			FreeBTCell(q);
			return TRUE;
		} else
			p = x < e ? &q->left : &q->right;
    return FALSE;  // なかった
}


BTnode* delete(ElementType x, BTnode *p)
{
	if(p == NULL)	return NULL;
	if(x < p->element)
		p->left = delete(x, p->left);
	else if(x > p->element)
		p->right = delete(x, p->right);
	else{ // 消すノードがいた場合
		BTnode *q;
		ElementType tmp;
		
		if(p->left == NULL){ // 左に子がいなければ右を上げる
			q = p->right;
			FreeBTCell(p);
			return q;
		}
		else if(p->right == NULL){ // 右に子がいなければ左を上げる
			q = p->left;
			FreeBTCell(p);
			return q;
		}
		// 右部分木の最も小さいのを引き上げた上で，そのノードを消去
		tmp = minElm(p->right);
		p->element = tmp;
		p->right = delete(tmp, p->right);
	}
	return p;
}


/**********/
void PrintTree(BTnode *T, int l)
{
    int i, l1 = l+1;

    if (T == NULL) { printf("<>\n"); return; }
    printf(" %c", T->element);
    if (T->right) {
        printf(" -( ");		PrintTree(T->right, l1);
    } else printf("\n");
    if (T->left) {
        for (i = 0; i < l; i++) printf("      ");
		printf("   \\( ");	PrintTree(T->left, l1);
    }
}
/**********/
BTnode *pTree = NULL;

int main(int ac, char **av)
{
    int i, c;

    for (i = 1; i < ac; i++) {
        if ((c = av[i][0]) == '-') {
			PrintTree(pTree, 0);
			printf("*** deleting %c\n", av[i][1]);
			(void) DELETE(av[i][1], &pTree);
			PrintTree(pTree, 0);
		} else INSERT(av[i][0], &pTree);
    }
	printf("final tree\n");
    PrintTree(pTree, 0);

	// insert, delete の別実装
	printf("another insert/delete implementation\n");
    for (i = 1; i < ac; i++) {
        if ((c = av[i][0]) == '-') {
			PrintTree(pTree, 0);
			printf("*** deleting %c\n", av[i][1]);
			pTree = delete(av[i][1], pTree);
			PrintTree(pTree, 0);
		}
		else{
			if(pTree == NULL)	pTree = insert(av[i][0], NULL);
			else{
				insert(av[i][0], pTree);
			}
		}
	}
	printf("final tree\n");
    PrintTree(pTree, 0);

	return 0;
}





/***** セルの割当てとメモリ管理 *****/
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

BTnode *FreeListBTNode = 0;

BTnode *GetNewBTCell()
{
    BTnode *ncell;
    int i, n;

    if ( !FreeListBTNode ) {
        n = PAGESIZE / sizeof(struct BT_node);
		ncell = (BTnode *) malloc(PAGESIZE);
		for (i = 0; i < n; i++, FreeListBTNode = ncell++)
			ncell->right = FreeListBTNode;
    }
    ncell = FreeListBTNode;
    FreeListBTNode = FreeListBTNode->right;
    return ncell;
}

void FreeBTCell(BTnode *p)
{
    p->right = FreeListBTNode;
    FreeListBTNode = p;
}
/*******************************/
