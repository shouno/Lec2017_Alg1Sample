#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint;
typedef unsigned char uchar;

#define MAXCHCODE 256

/*  ASCII 1文字に対するハフマン符号を表す構造体  */
struct hcode {
	uint bitl;	/*  ビット数  */
	uchar *bits;	/*  ビット列へのポインタ  */
};

struct hcode hcodes[MAXCHCODE+1];
/*  8ビットで表される ASCII文字 256字分と EOFの分の、ハフマン符号の表  */

char *CodeBits;		/*  実際にビット列を入れておく場所  */

/*  文字(コード)と直後の ':'までを読み込む  */
int readchar(FILE *f)
{
	int ch, c;

	if ((ch = getc(f)) != '\\') (void) getc(f);  /*  ':'を読み込む  */
	else if ((ch = getc(f)) == '\\') (void) getc(f);  /*  ':'を読み込む  */
	else if (ch == 'E' || ch == 'e') { /*  EOF  */
		ch = MAXCHCODE;
		while ((c = getc(f)) != ':') ;
	} else  /*  8進数  */
		for (ch = ch - '0'; (c = getc(f)) != ':'; ) ch = (ch << 3) + (c - '0');
	return ch;
}


/*  ファイル中のコード表を読み込む。但し、表は 1文字につき次のような行:

	文字:ビット数:ビット列('0'/'1'の並び)
*/
void gethcodes(char *fn)
{
	FILE *f;
	uchar c, *p;
	uint ch, i, k, nchars, bitl;

	if ((f = fopen(fn, "r")) == NULL) {
		fprintf(stderr, "Cannot open file %s of the code table.  Bye.\n", fn);
		exit(1);
	}
	/*  すべての文字に対するビット数を 0に初期化  */
	for (i = 0; i < MAXCHCODE; i++) hcodes[i].bitl = 0;
	/*  ファイルの先頭行に書かれた、表の文字数と最大のビット数を読み込む  */
	fscanf(f, "%d chars, max %d bits\n", &nchars, &bitl);
	/*  最大ビット長のコードを記憶するのに必要なバイト数  */
	i = bitl >> 3;
	if ((bitl&7) != 0) i++;
	/*  ビット列を記憶するための領域を確保。
		(最大ビット長の場合のバイト数)*(文字数)のバイト数だけ確保  */
	p = CodeBits = (char *)malloc(nchars*i);
	/**/
	for (i = 0; i < nchars; i++) {
		ch = readchar(f);		/*  文字を(':'まで)読み込む  */
		fscanf(f, "%d:", &bitl);	/*  ビット長  */
		/*  0/1の列を読み込み、pが指す領域に左詰めでビット列を作成する  */
		hcodes[ch].bitl = bitl,  hcodes[ch].bits = p;
		for (ch = 0, k = 0; k < bitl;) {
			/*  読み込んだ文字の 0/1 に対応するビットを ch の最下位ビットに  */
			ch <<= 1;
			if ((c = getc(f)) == '1') ch |= 1;
			if (((++k)&7) == 0) {
				/*  8ビット溜ったらビット列の領域に記憶し、ch はリセット  */
				*p++ = ch;
				ch = 0;
			}
		}
		/*  chにためたビットがあれば、左に詰めて領域に記憶  */
		if ((k&=7) != 0) *p++ = ch << (8-k);
		(void)getc(f);	/* read NL */
	}
	/**/
	fclose(f);
}


/*  ある文字に対応するビット列を出力。
    但し、実際の出力は 8ビット溜る毎に行う  */
void putbits(struct hcode *p)
{
	static char ch = 0;		/*  8ビットになるまで貯める領域  */
	static int nbits = 0;		/*  溜っているビット数  */
	/**/
	uint bitl, i, nbytes;
	uchar *q, bits;

	if (p == NULL)	/*  EOF  */
		bitl = hcodes[MAXCHCODE].bitl,  q = hcodes[MAXCHCODE].bits;
	else bitl = p->bitl,  q = p->bits;
	nbytes = bitl >> 3;	/*  ビット長/8 = バイト列の長さ  */
	if (nbits == 0) {	/*  今までに溜っている分がない  */
		for (i = 0; i < nbytes; i++) putchar(*q++);	/*  バイト列をそのまま出力  */
		if ((nbits = bitl&7) != 0) ch = *q;		/*  残りのビット列は貯めておく  */

	} else {
		for (i = 0; i < nbytes; i++) {
			/*  バイト列の分は、ch に溜っている分を補いながら出力  */
			bits = *q++;
			putchar(ch | (bits >> nbits));
			ch = bits << (8 - nbits);
		}
		/*  chの上位 nbits ビットと、残りのビット列の処理  */
		if ((bitl &= 7) != 0) {
			bits = *q;
			ch |= (bits >> nbits);	/*  残りのビット列を chの下位ビットに補う  */
			if ((nbits += bitl) >= 8) {
				/*  補った結果 8ビットに達したならば  */
				putchar(ch);	/*  8ビットの分は出力し、  */
				/*  残りは貯めておく  */
				nbits -= 8;
				ch = bits << (bitl - nbits);
			}
		}
	}
	/*  EOFの場合、貯めてある分があれば出力  */
	if (p == NULL && nbits != 0) putchar(ch);
}

/*  標準入力から 1文字ずつ読み込み、表をひいて得られた符号を標準出力に出力  */
void encode_stdinout()
{
	int c;

	for (; (c = getchar()) != EOF;)
		if (hcodes[c].bitl == 0)
			fprintf(stderr, "*** no code for %c(0x%02x)\n", c, c);
		else putbits(&hcodes[c]);
	putbits(NULL);
}

/******** ********* ********* ********* ********* ********* *********/

int main(int ac, char **av)
{
	if (ac <= 1) {
		fprintf(stderr, "*** No filename of code table is given.  Bye.\n");
		exit(1);
	}
	/**/
	gethcodes(av[1]);
	/**/
	encode_stdinout();
}
