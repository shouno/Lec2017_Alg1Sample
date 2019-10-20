#include <stdio.h>
#include <stdlib.h>

typedef int elmtype;  // リスト要素で保持するデータ型
typedef struct __Lcell__ { // リスト要素の構造体
    struct __Lcell__ *next;
    elmtype element;
} Lcell;

typedef Lcell* List;   // リスト要素の構造体ポインタを List型とする


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




// テストしてみる．とりあえず領域確保と解放が動くかを確認
int main(void)
{
	List p[10];
	int i;

    p[0] = GetNewLCell();
    p[1] = GetNewLCell();
	FreeLCell(p[0]);
	p[2] = GetNewLCell();
	p[3] = GetNewLCell();
	p[4] = GetNewLCell();
	p[5] = GetNewLCell();
	FreeLCell(p[2]);
	FreeLCell(p[4]);
	p[6] = GetNewLCell();
	p[7] = GetNewLCell();

	for (i = 0; i < 8; i++)
		printf("%2d: %p\n", i, p[i]);  // 保存されているポインタを表示
}
