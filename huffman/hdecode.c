#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;
typedef unsigned char uchar;

#define MAXCHKINDS 257
/* 0x00 ... 0xff と EOF */
#define EOFCODE -1

#define LEFTCODECH '0'
#define RIGHTCODECH '1'

#define LEFTCODE 0

typedef int elmtype;

struct Htree {
	struct Htree *left, *right;
	elmtype element;
} Htrees[2*MAXCHKINDS];	/* 未使用の木の領域 */
uint nUtrees;	/* 使用中の木の数 */

struct Htree *HTroot;

/******** ********* ********* ********* ********* ********* *********/
/*  0/1の文字列を読み込み、木の対応する辺を必要に応じて節点を追加しながら
    辿って行き、最後に(改行した時点で)、葉を追加する  */
void fillTree(struct Htree *p, elmtype leafch, FILE *f)
{
	char c;
	struct Htree *q;

	while ((c = getc(f)) != '\n')
		if (c == LEFTCODECH) {
			if (p->left == NULL) {
				/*  節点を左の子として追加  */
				/**********************************/
				/*******  ここを埋めなさい  *******/
				/**********************************/
			}
			p = p->left;		/*  左の子の節点へ移る  */
		} else {
			/*  節点を右の子として追加  */
			/**********************************/
			/*******  ここを埋めなさい  *******/
			/**********************************/
		}
	/**********************************/
	/*******  ここを埋めなさい  *******/
	/**********************************/
}


/*  文字(コード)と直後の ':'までを読み込む  */
int readchar(FILE *f)
{
	int ch, c;

	if ((ch = getc(f)) != '\\') (void) getc(f);  /*  ':'を読み込む  */
	else if ((ch = getc(f)) == '\\') (void) getc(f);  /*  ':'を読み込む  */
	else if (ch == 'E' || ch == 'e') { /*  EOF  */
		ch = EOFCODE;
		while ((c = getc(f)) != ':') ;
	} else  /*  8進数  */
		for (ch = ch - '0'; (c = getc(f)) != ':'; ) ch = (ch << 3) + (c - '0');
	return ch;
}


/*  ファイルから各文字に対する符号の表を読み込み、対応するハフマン木を作成  */
void gethcode_makeHtree(char *fn)
{
	FILE *f;
	uint ch, c, i, nchars, bitl;

	if ((f = fopen(fn, "r")) == NULL) {
		fprintf(stderr, "Cannot open file %s of the code table.  Bye.\n", fn);
		exit(1);
	}
	/*  最初の行から、文字数と符号の最大ビット長を読み込む  */
	fscanf(f, "%d chars, max %d bits\n", &nchars, &bitl);
	/*  ハフマン木の根を準備  */
	nUtrees = 1;
	HTroot = &Htrees[0];
	HTroot->left = HTroot->right = NULL;

	for (i = 0; i < nchars; i++) {
		ch = readchar(f);		/*  文字  */
		fscanf(f, "%d:", &bitl);	/*  対応するハフマン符号のビット数  */
		fillTree(HTroot, ch, f);	/*  符号の0/1列を読み込み、ハフマン木に登録  */
	}

	fclose(f);
}

#define IsLeaf(p) (p->left==NULL)

/******** ********* ********* ********* ********* ********* *********/

/*  標準入力からの入力の次のビットを返す  */
int getbit(void)
{
	static int ch = 0;
	static int nbits = 0;

	if (nbits == 0)
		if ((ch = getchar()) == EOF) return EOF;
		else nbits = 7;
	else nbits--;
	return((ch >> nbits)&1);
}

void decode_stdinout(void)
{
	struct Htree *p = HTroot;
	int b, c;

	while ((b = getbit()) != EOF) {
		/*  入力のビットに従って木を葉に向かって降りて行く  */
		/**********************************/
		/*******  ここを埋めなさい  *******/
		/**********************************/
		/*  葉に到達した時点で、対応する文字を出力  */
		/**********************************/
		/*******  ここを埋めなさい  *******/
		/**********************************/
		p = HTroot;		/*  根にリセット  */
	}
}

int main(int ac, char **av)
{
	if (ac <= 1) {
		fprintf(stderr, "*** No filename of code table is given.  Bye.\n");
		exit(1);
	}
	gethcode_makeHtree(av[1]);
	decode_stdinout();

	return 0;
}
