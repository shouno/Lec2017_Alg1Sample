#include <stdio.h>
#include <stdlib.h>

typedef int elmtype;  // リスト要素で保持するデータ型
typedef struct __Lcell__ { // リスト要素の構造体
    struct __Lcell__ *next;
    elmtype element;
} Lcell;

typedef Lcell* List;   // リスト要素の構造体ポインタを List型とする

List GetNewLCell(void);  // セル構造を作って，そのポインタが返ってくる
void FreeLCell(List p);  // セル構造を破棄する．
List LCreate(void);  // （ダミーヘッダ付きの）リストを作ってそのポインタを返す


List LCreate(void)
{
	List head;
	head = GetNewLCell(); // とりあえず head が指しているところにセルをつくって
	head->element = -1;   // とりあえず -1 をいれておく
	head->next = NULL;    // head の次の要素がない状態が空のリスト
	return head;
}	

int LisEnd(List p)  // 最後のセルか？とりあえず，型チェックできないので関数マクロは使わないで書く
{
	if(p->next == NULL)	return 1; // p の次が NULL なら最後
	else 				return 0;
}


int LisValid(List p) // 有効なセルか？ 上の LisEnd と統合したいなぁ...
{
    if(p != NULL)	return 1;
    else			return 0;
}


List next(List p)
{
	return p->next;
}



elmtype Lretrieve(List p) // p の要素を返す
{
	return p->element;
}


List LFirst(List head) // リストの最初，ダミーヘッダはスキップする
{
	return head->next;
}


List LLast(List head) // リストを最後まで辿って返す
{
	List p;
	for(p = LFirst(head); LisValid(next(p)); p = next(p));
	return p;
}


List prev(List p, List head) // 一個前のセルを指定，最初からたどるしかないので不格好．．．
{
    List q;
    int isFound = 0;
    for(q = LFirst(head); LisValid(q); q = next(q)){
        if(p == next(q)){ // 一個前が p であるセルを探す
            isFound = 1;
            break;
        }
    }
    if(isFound == 1)	return q;
    else				return NULL;
}




List Llocate(elmtype x, List head) // x を持つ要素を返すことにしてある
{
	List p;
	for(p = LFirst(head); LisValid(p); p = next(p)){
		if(p->element == x)	break;
	}
	return p;
}


void LInsertAfter(elmtype x, List p) // p の後ろに x を持つセルを挿入
{
	List n;
	n = GetNewLCell();
	n->element = x;
	n->next = p->next;
	p->next = n;
}

void LDeleteNext(List p) // p の直後のセルを削除
{
	List n;
	n = p->next;
	if(n != NULL){
		p->next = n->next;
		FreeLCell(n); // 確保したメモリ領域の解放
	}
}


// 作成した関数でテストを行ってみる．
int main(void)
{
	int i;
	List L, p;

	// リスト生成
	L = LCreate();
	p = L;
	for(i = 0; i < 10; i++){
		LInsertAfter(i, p);
		p = next(p);
	}

	// とりあえず全部なめてみる
	for(p = LFirst(L); LisValid(p); p = next(p)){
		printf( "item: %d\n", Lretrieve(p));
	}

	printf( "\n\n" );
	p = Llocate(5, L);
	printf( "item: %d\n", Lretrieve(p));
	LDeleteNext(p); // 6 が消される

	p = Llocate(1, L);
	LDeleteNext(p); // 2 が消される
	
	// 全部なめてみる
    printf("---Forward Traverse---\n");
	for(p = LFirst(L); LisValid(p); p = next(p)){
		printf("item: %d\n", Lretrieve(p));
	}

    printf("---Backward Traverse---\n");
    for(p = LLast(L); LisValid(p); p = prev(p, L)){
		printf("item: %d\n", Lretrieve(p));
    }

	printf( "\n\n" );
	p = LLast(L);
	printf( "item: %d\n", Lretrieve(p));
}




// 以下の部分は，GetNewLCell() と，FreeLCell() の機能を理解していればよい．
// とりあえず，そういうものがあるんだ，と一旦受け入れてしまうのが吉かもしれない

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

static List FListLcell = NULL;

List GetNewLCell(void)
{
	// 新規のリスト型の要素へのポインタを返す関数
	//（領域が足りなければ確保する，多分）
	List ncell;
	int i, n;

	if (!FListLcell) { // PAGESIZEに収まる分量の塊を確保
		n = PAGESIZE / sizeof(Lcell);
		ncell = (List) malloc(PAGESIZE);
		// リンク繋ぐ作業
		for (i = 0; i < n; i++, FListLcell = ncell++)
			ncell->next = FListLcell;
	}
	ncell = FListLcell;
	FListLcell = FListLcell->next;
	return ncell;
}


void FreeLCell(List p)
{
    // 使い終わった p は，次に使えるように FListLcell に登録
    p->next = FListLcell; 
    FListLcell = p;
}
