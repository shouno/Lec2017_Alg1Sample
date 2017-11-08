//
// 指定されたファイルを 8 bit 文字列として読み込み，
// 各文字の出現頻度を数え上げ，
// Huffman Tree を作成し，
// 各文字に割り当てられた符号(Huffman符号) を出力
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define MAXCHKINDS 257
typedef unsigned int uint;
typedef struct elm {
	// Huffmann Tree に格納するデータ構造
	uint freq;  // 出現頻度
	int ch;     //
} elmtype;

typedef struct _HTree_ {
	struct _HTree_ *l;  // 左ノード
	struct _HTree_ *r;  // 右ノード
	elmtype element;
} HTree;

// ブール値の定義
const int True = 1;
const int False = 0;

HTree* Hforest[MAXCHKINDS]; // 木の集合(森)．
uint nFTrees = 0; // 森の中の木の数

HTree Nodes[2*MAXCHKINDS]; // 木のノード実体
uint nNodes = 0; // 使用中の木の数

static int freq[MAXCHKINDS]; // 出現頻度記録用配列


int IsLeaf(HTree *p)
{
	// HTree が葉か？
	// 左の子がいなければ葉ノード
	if( p->l == NULL )  return True;
	else				return False;
}


HTree* makeNode(int ch, uint freq, HTree *lft, HTree *rgt)
{
	// lft, rgt が共に　NULL であれば Leaf 扱いにする
	// 逆に Leaf でないときは ch = 0 であることを仮定しておく
	HTree *p;
	p = &Nodes[nNodes++]; // 木の領域を確保 malloc で取ってきても良いんだけどね
	p->element.freq = freq;
	p->element.ch = ch;
	p->l = lft;
	p->r = rgt;

	return p;
}


void init_chcount(void)
{
	int c;
	for(c = 0; c < MAXCHKINDS; c++){
		freq[c] = 0;
	}
}


void chcountf(FILE *in)
{
	int c;
	while((c = getc(in)) != EOF){
		freq[c]++;
	}
}


void printchar(int c)
{
	if(c == EOF){
		printf("\\EOF");
	}
	else if(c == '\\'){
		printf("\\\\");
	}
	else if(isprint(c)){
		printf("%c", c);
	}
	else{
		printf("\\%03o", c);
	}
}


void makeInitialForest(void)
{
	int c;

	for(c = 0; c < MAXCHKINDS; c++){
		// if(freq[c] > 0 && isalpha(c)){ // アルファベットのみを対象にする場合コメントアウト
		if(freq[c] > 0){
			Hforest[nFTrees++] = makeNode(c, freq[c], NULL, NULL);
		}
	}
	// アルファベットのみを対象にする場合は以下をコメントアウト
	Hforest[nFTrees++] = makeNode(EOF, 1, NULL, NULL); // EOF は一回だけ出現
}


// 現在の森の中から最小の頻度となる木を選び出す．二つの木の持つ添字は *id1, *id2 へ
// 格納し，呼び出し元で使う
void get2LeastIndices(uint *id1, uint *id2)
{
	uint i1, i2, i, f1, f2, f;

	// とりあえず最初の二つの要素を二つの添字候補とする
	i1 = 0;
	f1 = Hforest[i1]->element.freq;
	i = 1;
	f = Hforest[i]->element.freq;
	if(f1 <= f){
		f2 = f;	i2 = i;
	}
	else{
		f2 = f1; i2 = i1;
		f1 = f;  i1 = i;
	}
	// この段階で f1 < f2 な状況
	// 最小頻度探索
	for(i = 2; i < nFTrees; i++){
		f = Hforest[i]->element.freq;
		if(f < f1){ // 現在の f1 よりも小さい
			f2 = f1;  i2 = i1;
			f1 = f;   i1 = i;
		}
		else if(f < f2 || (f == f2 && !IsLeaf(Hforest[i2]))){
			f2 = f, i2 = i;
		}
	}
	*id1 = i1;
	*id2 = i2;
}


void composeHuffmanTree(void)
{
	uint id1, id2;
	HTree *p, *p1, *p2;

	while(nFTrees >= 2){
		// 森のなかから最小頻度の木を二つ選ぶ
		get2LeastIndices(&id1, &id2);
		p1 = Hforest[id1];
		p2 = Hforest[id2];
		p = makeNode(0, p1->element.freq+p2->element.freq, p1, p2);

		// 子とした木(p1, p2)を森から取り除き，新たな木 p を追加
		nFTrees--;
		if(id1 == nFTrees){
			Hforest[id2] = p;
		}
		else if(id2 == nFTrees){
			Hforest[id1] = p;
		}
		else{
			Hforest[id1] = p;
			Hforest[id2] = Hforest[nFTrees];
		}
	}
}


uint TreeHeight(HTree *p)
{
	uint hl, hr;
	if(IsLeaf(p))	return 0;
	hl = TreeHeight(p->l);
	hr = TreeHeight(p->r);
	if(hl > hr)		return hl+1;
	else			return hr+1;
}


const int LeftCode = '0';
const int RightCode = '1';

void printHTree(uint lv, HTree *p)
{
	const int NodePerLine = 7;
	uint l = lv;
	uint lvd = lv / NodePerLine;
	uint i;
	
	if(lvd > 0 && (l %= NodePerLine) == 0)
		printf( "*%02x ", lvd);
	printf("%8d", p->element.freq); // 当該ノードの頻度値を出力
	if(IsLeaf(p)){
		i = NodePerLine-1 - l;
		if(i > 0)		i *= 10;
		if(lvd == 0)	i += 4;
		for(; i > 0; i--)
			putchar(':');
		putchar('<');
		printchar(p->element.ch);
		putchar('>');
		putchar('\n');
	}
	else{
		lv++;
		// 左の辺と子を表示
		printf("-(");
		if(l == NodePerLine-1)
			printf("\n");
		printHTree(lv, p->l);

		// 右の辺と子を表示
		if(lvd > 0)
			printf("*%02x ", lvd);
		for(i = 0; i < l; i++)
			printf("          ");
		printf("        \\(");
		if(l == NodePerLine-1)
			printf("\n");
		printHTree(lv, p->r);
	}
}




// 木のなぞりに用いる

char codestack[MAXCHKINDS]; 





extern char *optarg;

int main(int ac, char* av[])
{
	int h, nfiles = 0, dumptree = 0;
	int Nchars;
	int opt;
	FILE *fp;

	init_chcount();
	while((opt = getopt(ac, av, "dDr:")) != -1){
		switch(opt){
		case 'd':
		case 'D':
			printf("Dump Tree Mode is set\n");
			dumptree = 1;
			break;
		case 'r':
			fp = fopen(optarg, "r");
			if(fp == NULL){
				fprintf(stderr, "Cannot open %s\n", optarg);
				exit(1);
			}
			nfiles = 1;
			chcountf(fp);
			fclose(fp);
			break;
		default:
			fprintf(stderr, "Usage: \n");
		}
	}
	if(nfiles == 0){
		// 入力ファイルが指定されない場合，標準入力 stdin を指定
		chcountf(stdin);
		nfiles = 1;
	}

	makeInitialForest();
	Nchars = nFTrees;
	composeHuffmanTree();
	h = TreeHeight(Hforest[0]);
	printf("#leaves: %d, tree height=%d\n", Nchars, h);
	printHTree(0, Hforest[0]);
	return 0;
}
