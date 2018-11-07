//
// 指定されたファイルを 8 bit 文字列として読み込み，
// 各文字の出現頻度を数え上げる，
//


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define MAXCHKINDS 257

static int freq[MAXCHKINDS]; // 出現頻度記録用配列
extern char *optarg;


void init_chcount(void)
{
    // 記録用配列 freq の初期化
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


void printcount(void)
{
	int c;

	for(c = 0; c < MAXCHKINDS; c++){
		if(freq[c] != 0){ // 出現した文字頻度を出力
			if(isprint(c)){
				printf("%c: %d\n", c, freq[c]);
			}
			else if(c == '\\'){
				printf("\\\\: %d\n", freq[c]);
			} 
			else{ // 表示できない文字
				printf( "\\%03o: %d\n", c, freq[c]);
			}
		}
	}
}


int main(int ac, char* av[])
{
	int nfiles = 0;
	int opt;
	FILE *fp;

	init_chcount();
	while((opt = getopt(ac, av, "r:")) != -1){
		switch(opt){
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
			fprintf(stderr, "Usage: %s [-r filename]\n", av[0]);
		}
	}
	if(nfiles == 0){
		// 入力ファイルが指定されない場合，標準入力 stdin を指定
		chcountf(stdin);
		nfiles = 1;
	}

	printcount();

	return 0;
}
