#ifdef LINEARHASH
#define HASHING
#endif

#include <stdio.h>

typedef unsigned int uint;

static uint Nwords, Nwkinds;

static char *strbuff = NULL, *strbuffend = NULL;
static char tmpbuf[BUFSIZ];

/***** 単語（'a'-'z','A'-'Z'の列）を小文字に変換して tmpbuff[] に読み込む *****/
static int GetWord(FILE *f)
{
  int l, c;
  char *p;

  while ((c = getc(f)) != EOF)
    if ( isalpha(c) ) {
      p = tmpbuf;
      do {
	*p++ = isupper(c) ? c + 'a'-'A': c;
	if ((c = getc(f)) == EOF) break;
      } while (isalpha(c));
      *p = 0;
      Nwords++;
      return(p - tmpbuf);
    }
  return(-1);
}

/***** 単語の情報を保存する構造体 *****/
struct bucket {
  uint counter;		/* 出現回数 */
  char *str;		/* 文字列   */
  uint len;		/* 文字数   */
#ifdef LINEARHASH
  /*  linear search の場合，ハッシュ値も保存する．
      この領域は FreeList のリンクと共用 */
  union { uint hashval; struct bucket *next; } u;
#else /* LINEARHASH */
  /*  連結リストとするためのリンク  */
  struct bucket *next;
#endif /* LINEARHASH */
};

#ifdef LINEARHASH
#define B_NEXT(p) (p)->u.next
#define B_HASHV(p) (p)->u.hashval
#else /* LINEARHASH */
#define B_NEXT(p) (p)->next
#endif /* LINEARHASH */

/*******************************/
/***** セルの割当てとメモリ管理 *****/
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

static struct bucket *FListBcell = 0;

struct bucket *GetNewBCell()
{
  struct bucket *ncell;
  int i, n;

  if ( !FListBcell ) {
    n = PAGESIZE / sizeof(struct bucket);
    ncell = (struct bucket *) malloc(PAGESIZE);
    for (i = 0; i < n; i++, FListBcell = ncell++) B_NEXT(ncell) = FListBcell;
  }
  ncell = FListBcell;
  FListBcell = B_NEXT(FListBcell);
  return ncell;
}
/*******************************/

#ifdef HASHING
static uint tablesize = 8209;
#else /* HASHING */
static uint tablesize = 26;
#endif /* HASHING */

/***** 表：要素は構造体へのポインタ *****/
static struct bucket **hashtable;

#ifdef LINEARHASH
static uint *lnrsrchlen;
#else /* LINEARHASH */
static uint *collisions;
#endif /* LINEARHASH */

#ifdef HASHING
#define updHASHVAL(c,h)  ((((h)<<3)|((uint)(h)>>(8*sizeof(uint)-3)))+(c))
/**  /
#define updHASHVAL(c,h)  ((((h)<<5)|((uint)(h)>>(8*sizeof(uint)-5)))+(c))
/**   /
#define updHASHVAL(c,h)  ((((h)<<3)^((uint)(h)>>(8*sizeof(uint)-3)))+(c))
/**/
#endif /* HASHING */

static void countup_word(char *s, uint l)
{
  uint h, i, c;
  char *p;
  struct bucket *q, *n, **r;

#ifdef HASHING
  for (h = 0, p = s; (c = *p++); ) h = updHASHVAL(c,h);
  h %= tablesize;
#else /* HASHING */
  h = s[0] - 'a';
#endif /* HASHING */
#ifdef LINEARHASH
  /***  線形探索（linear search）の場合  ***/
  for (r =&hashtable[i = h], c = 0; (q = *r) != NULL && c < tablesize; c++)
    /* 同じ文字列かの確認に，ハッシュ値が等しいかが有効 */
    if (h == B_HASHV(q) && l == q->len && strcmp(s, q->str) == 0) {  /* 見つかった */
      q->counter++;
      return;
    } else if (++i < tablesize) r++;	/*  次の要素へ  */
    else i = 0, r = hashtable;		/*  表の最後までいったので折り返す  */
  if (c >= tablesize) {
    /* 表を一周してしまったけれど，同じ語も空きも見つからなかった */
    fprintf(stderr, "*** Too small size %d of hashtable\n", tablesize);
    exit(1);
  }
  lnrsrchlen[i] = c+1;
#else /* LINEARHASH */
  /*** 連結リスト内を探す．但し，要素は辞書順に整列している ***/
  for (r = &hashtable[h]; (q = *r) != NULL; r = &(B_NEXT(q)))
    if ((c = strcmp(s, q->str)) < 0) break;	/* 辞書順では，文字列 s の方が先 */
    else if (c == 0) {				/* 同じ語が見つかった */
      q->counter++;
      return;
    }
  collisions[h]++;
#endif /* LINEARHASH */
  /* 見つからなかったので，空き場所 r = &hashtabl[i] に登録 */
  Nwkinds++;
  *r = n = GetNewBCell();
#ifdef LINEARHASH
  B_HASHV(n) = h;
#else /* LINEARHASH */
  B_NEXT(n) = q;
#endif /* LINEARHASH */
  n->counter = 1;
  /*****  文字列の保存 *****/
  if (strbuffend-strbuff < l+1) {
    strbuff = (char *)malloc(PAGESIZE);
    strbuffend = strbuff + PAGESIZE;
  }
  n->len = l;
  strcpy(n->str = strbuff, tmpbuf);
  strbuff += (l+1);
}

/***** 単語を読み込んでは表に登録・数え上げ *****/
void countup_words_in_file(FILE *f)
{
  int l;

  while ((l = GetWord(f)) != EOF) countup_word(tmpbuf, l);
}

/***** 表の情報を出力 *****/

/*** 出現頻度の高いものから５個だけ出力する ***/
#define MAXNTOPS 5
static struct bucket *tops[MAXNTOPS];
static uint ntops;
static uint empties;

void dump_hashtbl()
{
  uint i, k, longestwl, avl;
  struct bucket *p, *longestw;
#ifdef LINEARHASH
  uint worstl = 0;  struct bucket *worst;
#endif /* LINEARHASH */

  ntops = 0;  empties = 0;  longestwl = 0;
  for (i = 0; i < tablesize; i++)
    if ((p = hashtable[i]) != NULL) {
#ifdef HASHING
#ifdef LINEARHASH
      avl += lnrsrchlen[i];
      if (lnrsrchlen[i] > 1) printf("search length %d at %d\n", lnrsrchlen[i], i);
      if (worstl < lnrsrchlen[i]) worstl = lnrsrchlen[i],  worst = p;
#else /* LINEARHASH */
      avl += collisions[i];
      if (collisions[i] > 1) printf("%d collisions at %d\n", collisions[i], i);
#endif /* LINEARHASH */
#else /* HASHING */
      printf("%d words with head char '%c'\n", collisions[i], i+'a');
#endif /* HASHING */
#ifndef LINEARHASH
      do {
#endif /* !LINEARHASH */
	if (ntops == 0) ntops = 1, tops[0] = p;
	else if (ntops >= MAXNTOPS && p->counter <= tops[ntops-1]->counter) ;
	else {
	  if (ntops < MAXNTOPS) k = ntops++;
	  for (k = ntops -1; k > 0; k--)
	    if (p->counter > tops[k-1]->counter) tops[k] = tops[k-1];
	    else break;
	  tops[k] = p;
	}
	if (longestwl < p->len) longestwl = p->len,  longestw = p;
#ifndef LINEARHASH
      } while (p = B_NEXT(p));
#endif /* !LINEARHASH */
    } else { empties++; continue; }
  /**/
  printf("%d words of %d kinds::: %d empty\n", Nwords, Nwkinds, empties);
  printf("*** Top %d ***: ", ntops);
  for (i = 0; i < ntops; i++) printf("%s(%d), ", tops[i]->str, tops[i]->counter);
  printf("\nlongest word=%s(%d)\n", longestw->str, longestw->len);
#ifdef HASHING
#ifdef LINEARHASH
  printf("  search length: average=%f, worst=%d\n", avl/(Nwkinds +0.0), worstl);
#else /* LINEARHASH */
  printf("  average # collisions=%f\n", avl/(Nwkinds +0.0));
#endif /* LINEARHASH */
#endif /* HASHING */
}

#ifdef HASHING
static uint primes[] = {
  4093, 8209, 16411, 32771, 65537, 131101, 199999, 0
};
#endif /* HASHING */

/***** 主プログラム *****/
main(int argc, char **argv)
{
  int k, n;
  char *p;

#ifdef HASHING
  /*** ハッシングを用いる場合，表の大きさを指定 ***/
  /*** +n : 表の大きさを n とする ***/
  /*** n または -n : 表の大きさを n 以上の素数 (上の表中の)とする ***/
  for (argc--, argv++ ; argc > 0; argc--, argv++)
    if ((p = *argv)[0] == '+') {
      n = atoi(&p[1]);
      tablesize = n;
    } else {
      if (p[0] == '-') p++;
      n = atoi(p);
      for (k = 0; primes[k] != 0; k++)
	if (primes[k] >= n) {
	  tablesize = primes[k];
	  break;
	}
    }
#endif /* HASHING */
  /**/
  hashtable = (struct bucket **) malloc(tablesize*sizeof(struct bucket *));
#ifdef LINEARHASH
  lnrsrchlen = (uint *) malloc(tablesize*sizeof(uint));
#else /* LINEARHASH */
  collisions = (uint *) malloc(tablesize*sizeof(uint));
  for (k = 0; k < tablesize; k++) hashtable[k] = NULL,  collisions[k] = 0;
#endif /* LINEARHASH */
  Nwords = Nwkinds = 0;
  /**/
  countup_words_in_file(stdin);
  /**/
  dump_hashtbl();
}
