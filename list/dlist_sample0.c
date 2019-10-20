#include <stdio.h>
#include <stdlib.h>

typedef int elmtype;  // リスト要素で保持するデータ型
typedef struct __DLcell__ { // doulbey linked リスト要素の構造体
    struct __DLcell__ *prev;
    struct __DLcell__ *next;
    elmtype element;
} DLcell;

typedef DLcell* DList;   // リスト要素の構造体ポインタを List型とする

DList GetNewDLCell(void);  // セル構造を作って，そのポインタが返ってくる
void FreeDLCell(DList p);  // セル構造を破棄する．
DList DLCreate(void);  // （ダミーヘッダ付きの）リストを作ってそのポインタを返す


DList DLCreate(void)
{
	DList head;
	head = GetNewDLCell(); // とりあえず head が指しているところにセルをつくって
	head->element = -1;   // とりあえず -1 をいれておく
	head->next = head;    // head の次の要素は自分自身（head) を指しておく
    head->prev = head;    // head の前の要素も head を指しておく
	return head;
}	

int DLisEnd(DList p, DList head)  // 最後のセルか？とりあえず，型チェックできないので関数マクロは使わないで書く
{
	if(p->next == head)	return 1; // p の次が head なら最後
	else 				return 0;
}

int DLisBegin(DList p, DList head) // 最初のセルか？
{
    if(p->prev == head)	return 1;
    else				return 0;
}

int DLisValid(DList p, DList head)	// 有効なセルか？ head を指している場合は有効なものが存在しない
{
    if(p == NULL || p == head)	return 0;
    else						return 1;
}

DList next(DList p)
{
	return p->next;
}

DList prev(DList p)
{
    return p->prev;
}

elmtype DLretrieve(DList p) // p の要素を返す
{
	return p->element;
}


DList DLFirst(DList head) // リストの最初，ダミーヘッダはスキップする
{
	return head->next;
}

DList DLLast(DList head) // リストの最後を返す，ダミーヘッダはスキップ
{
	return head->prev;
}


DList DLlocate(elmtype x, DList head) // x を持つ最初の要素を返すことにしてある
{
	DList p;
	for(p = DLFirst(head); DLisValid(p, head); p = next(p)){
		if(DLretrieve(p) == x)	break;
	}
	return p;
}


void DLInsertAfter(elmtype x, DList p) // p の後ろに x を持つセルを挿入
{
	DList n;
	n = GetNewDLCell();
	n->element = x;
	n->next = p->next;
    n->prev = p;

    // リンク繋ぎ変え
    p->next->prev = n;
	p->next = n;
}


void DLInsertBefore(elmtype x, DList p) // p の前に x を持つセルを挿入
{
    DList n;
    n = GetNewDLCell();
    n->element = x;
    n->prev = p->prev;
    n->next = p;

    // リンク繋ぎ変え
    p->prev->next = n;
    p->prev = n;
}

void DLDeleteNext(DList p) // p の直後のセルを削除
{
	DList n;
	n = p->next;
	if(n != NULL){
		p->next = n->next;
		FreeDLCell(n); // 確保したメモリ領域の解放
	}
}


// 作成した関数でテストを行ってみる．
int main(void)
{
	int i;
	DList DL, p;

	// リスト生成
	DL = DLCreate();
	p = DL;
	for(i = 0; i < 10; i++){
		DLInsertAfter(i, p);
		p = next(p);
	}

	// とりあえず全部なめてみる
    printf("head: %p\n", DL);
	for(p = DLFirst(DL); DLisValid(p, DL); p = next(p)){
        printf("%p: pre:%p, pos%p\n", p, prev(p), next(p));
		printf("    item: %d\n", DLretrieve(p));
	}

	printf( "\n\n" );
	p = DLlocate(5, DL);
	printf( "item: %d\n", DLretrieve(p));
	DLDeleteNext(p); // 6 が消される

	p = DLlocate(1, DL);
	DLDeleteNext(p); // 2 が消される
	
	// 全部なめてみる
    printf("---Forward Traverse---\n");
	for(p = DLFirst(DL); DLisValid(p, DL); p = next(p)){
		printf( "item: %d\n", DLretrieve(p));
	}


    printf("---Backward Traverse---\n");    
    for(p = DLLast(DL); DLisValid(p, DL); p = prev(p)){
		printf( "item: %d\n", DLretrieve(p));
    }

	printf( "\n\n" );
	p = DLLast(DL);
	printf( "item: %d\n", DLretrieve(p));
}






// 以下の部分は，GetNewDLCell() と，FreeDLCell() の機能を理解していればよい．
// とりあえず，そういうものがあるんだ，と一旦受け入れてしまうのが吉かもしれない

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

static DList FListLcell = NULL;

DList GetNewDLCell(void)
{
	// 新規のリスト型の要素へのポインタを返す関数
	//（領域が足りなければ確保する，多分）
	DList ncell;
	int i, n;

	if (!FListLcell) { // PAGESIZEに収まる分量の塊を確保
		n = PAGESIZE / sizeof(DLcell);
		ncell = (DList) malloc(PAGESIZE);
		// リンク繋ぐ作業
		for (i = 0; i < n; i++, FListLcell = ncell++)
			ncell->next = FListLcell;
	}
	ncell = FListLcell;
	FListLcell = FListLcell->next;
	return ncell;
}


void FreeDLCell(DList p)
{
    // 使い終わった p は，次に使えるように FListLcell に登録
    p->next = FListLcell; 
    FListLcell = p;
}
