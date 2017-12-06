#include <stdio.h>
typedef unsigned int uint;

struct trie_node {
    char ch;
    uint counter;
    struct trie_node *next;
    struct trie_node *child;
};
typedef struct trie_node *p_trie;

static struct trie_node TrieRoot = { 0, 0, NULL, NULL};
static p_trie FreeListTrieNode = 0;	/* フリーリスト */

/***** セルの割当てとメモリ管理 *****/
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

p_trie GetNewTCell()
{
    p_trie ncell;
    int i;

    if ( !FreeListTrieNode ) {
        ncell = (p_trie) malloc(PAGESIZE);
        for (i=0; i < PAGESIZE/sizeof(struct trie_node);
		  i++, FreeListTrieNode = ncell++)
	    ncell->next = FreeListTrieNode;
    }
    ncell = FreeListTrieNode;
    FreeListTrieNode = FreeListTrieNode->next;
    return ncell;
}
/****  トライの子の節点を追加  ****/
static p_trie find_triechildnode(int c, p_trie p)
{
    p_trie q, n, *r;

    if ((q = p->child) == NULL) r = &p->child;	/* 単独の子として追加 */
    else if (c == q->ch) return q;	/* 発見 */
    else if (c > q->ch) {
        for (p = q; (q = p->next) != NULL; p = q) /* 子のリスト中で挿入すべき場所を探す */
            if (c == q->ch) return q;		/* 発見 */
	    else if (c < q->ch) break;	/* p の next として挿入する */
	r = &p->next;
    } else r = &p->child;	/* 子のリストの先頭に追加 */
    *r = n = GetNewTCell();
    n->ch = c,  n->counter = 0,  n->next = q,  n->child = NULL;
    return n;
}

static uint Nwords = 0, Nwkinds = 0;
static uint maxlength = 0;

static void countup_words_in_file(FILE *f)
{
    int c, l;
    p_trie p;

    while ((c = getc(f)) != EOF) {
        p = &TrieRoot;
        if ( isalpha(c) ) {
            l = 0;
            do {		/* トライで下へなぞっていく */
        	l++;
		if ( isupper(c) ) c += ('a' - 'A');
		p = find_triechildnode(c, p);	/* 節点 p から c の指す節点へ */
		if ((c = getc(f)) == EOF) break;
	    } while ( isalpha(c) );
	    p->counter++,  Nwords++;
	    if (p->counter == 1) Nwkinds++;
	    if (l > maxlength) maxlength = l;
	}
    }
}
/******************/
#define MAXNTOPS 5
static char *tops[MAXNTOPS], *buff, *longestword;
static uint ntops, counts[MAXNTOPS], MaxL = 0;

void traverse_allwords(p_trie p, uint lev)	/*  <-------  */
{
    uint i, k, lev1 = lev+1;			/*  <-------  */
    char *w;

    for ( ; p != NULL; p = p->next) {		/*  <-------  */
        buff[lev] = p->ch;				/*  <-------  */
        if (p->counter > 0) {
            if (ntops == 0) {
	        ntops = 1;
		strncpy(tops[0], buff, lev1);
		tops[0][lev1] = 0;	counts[0] = p->counter;
	    } else if (ntops >= MAXNTOPS && p->counter <= counts[ntops-1]) ;
	    else {
	        if (ntops < MAXNTOPS) k = ntops++;
		for (k = ntops -1; k > 0; k--)
		    if (p->counter > counts[k-1]) {
		        w = tops[k];		i = counts[k];
			tops[k] = tops[k-1];	counts[k] = counts[k-1];
			tops[k-1] = w;		counts[k-1] = i;
		    } else break;
		strncpy(tops[k], buff, lev1);
		tops[k][lev1] = 0;	counts[k] = p->counter;
	    }
	    if (p->child == NULL && MaxL < lev1) {
	        strncpy(longestword, buff, lev1);
		longestword[lev1] = 0;
		MaxL = lev1;
	    }
	}
	if (p->child != NULL) traverse_allwords(p->child, lev1);  /* <------ */
    }
}

void print_ntops()
{
    uint i, l = maxlength+1;
    char *p;

    p = (char *) malloc(l*(MAXNTOPS +2));
    for (i = 0; i < MAXNTOPS; i++, p += l) tops[i] = p;
    buff = p;
    longestword = p + l;
    /**/
    traverse_allwords(TrieRoot.child,0);
    /**/
    for (i = 0; i < ntops; i++) printf("%s(%d), ", tops[i], counts[i]);
    printf("\nlongest word=%s(%d)\n", longestword, MaxL);
}
/******************/

main(int ac, char **av)
{
    countup_words_in_file(stdin);
    printf("%d words of %d kinds ::: maxlen=%d\n", Nwords, Nwkinds, maxlength);
    print_ntops();
}
