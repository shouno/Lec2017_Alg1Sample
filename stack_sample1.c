#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>

typedef int elmtype;  // スタックで保持するデータ型
typedef struct __StackA__{
	int top, maxn; // スタックトップの位置と最大要素数
	elmtype *elements; // スタック本体
} StackA;

typedef StackA* pStackA;  // スタック型のポインタを方として作っておく

// 真偽値を定義しておく
const int True = 1;
const int False = 0;

pStackA StACreate(int maxsize)
{
	pStackA p;

	p = (pStackA)malloc(sizeof(StackA));
	p->elements = (elmtype*)malloc(maxsize * sizeof(elmtype));
	p->top = -1;
	p->maxn = maxsize;
	return p;
}


int StAIsEmpty(pStackA p)
{
	if(p->top < 0)	return True; // スタックが空の場合
	else			return False;
}


elmtype StATop(pStackA p)
{
	if(StAIsEmpty(p))
		err(EXIT_FAILURE, "Stack is empty");
	else
		return p->elements[p->top];
}

elmtype StAPop(pStackA p)
{
	if(StAIsEmpty(p))
		err(EXIT_FAILURE, "Stack is empty");
	else
		return p->elements[p->top--];
}

void StAPush(elmtype x, pStackA p)
{
	if(p->top+1 >= p->maxn)
		err(EXIT_FAILURE, "Stack overflow");
	else{
		p->top++;
		p->elements[p->top] = x;
		// p->elements[++p->top] == x; としても良いけどね
	}
}


int main(void)
{
	int i;
	pStackA st;

	st = StACreate(10);
	if(StAIsEmpty(st)){
		printf("Stack is empty\n");
	}
	for(i = 0; i < 10; i++){
		StAPush(i, st);
	}
	printf("StackTop = %d\n", StATop(st));
	for(i = 0; i < 3; i++){
		printf("Pop: %d\n", StAPop(st));
	}
	printf("StackTop = %d\n", StATop(st));

	return 0;
}

