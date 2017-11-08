#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

typedef int elmtype;  // リスト要素で保持するデータ型
typedef struct __Lcell__ { // リスト要素の構造体
  struct __Lcell__ *next;
  elmtype element;
} Lcell;

typedef Lcell* LcellP; // リスト要素の構造体ポインタ
typedef LcellP List;   // リスト要素の構造体ポインタを List型とする

typedef struct __Queue__{
	LcellP front;  // キューの取り出し口
	LcellP rear;   // キューの入れ口
} Queue;

typedef Queue* QueueP;

// 真偽値を定義しておく
const int True = 1;
const int False = 0;

List GetNewLCell(void);  // セル構造を作って，そのポインタが返ってくる
void FreeLCell(List p);  // セル構造を破棄する．


QueueP QCreate() // 空のキューを作成
{
	QueueP q;
	q = (QueueP)malloc(sizeof(Queue));
	q->front = NULL;
	q->rear = NULL;
	return q;
}

int QIsEmpty(QueueP q)
{
	if(q == NULL || q->front == NULL)
		return True;
	else
		return False;
}


elmtype QFront(QueueP q)
{
	if(QIsEmpty(q))
		err(EXIT_FAILURE, "Queue is emtpy");
	else
		return q->front->element;
}


void QEnqueue(elmtype x, QueueP q)
{
	LcellP n;
	// 新しいセルの作成
	n = GetNewLCell();
	n->element = x;
	n->next = NULL;

	// キューへの挿入
	if(QIsEmpty(q))
		q->front = n;
	else
		q->rear->next = n;
	q->rear = n;
}


elmtype QDequeue(QueueP q)
{
	// キューからの取り出し
	LcellP h = q->front;
	elmtype x = h->element;
	q->front = h->next;

	// なんだが，キューが空になった時の処理がいる
	if(q->front == NULL)
		q->rear = NULL;

	FreeLCell(h);
	return x;
}


int main(void)
{
	int i;
	QueueP q;

	q = QCreate();
	
	for(i = 0 ; i < 10; i++){
		QEnqueue(i, q);
	}

	for(i = 0 ; i < 5; i++){
		printf("Deq: %d\n", QDequeue(q));
	}
	printf("QFront: %d\n", QFront(q));
	for(i = 0 ; i < 5; i++){
		printf("Deq: %d\n", QDequeue(q));
	}

	return 0;
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


