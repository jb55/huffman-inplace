
#define __STDC_WANT_LIB_EXT2__ 1
#include <hash/hash.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define _BITS "%c%c%c%c%c%c%c%c"
#define TO_BITS(byte)                    \
  (byte & 0x80 ? '1' : '0'),                    \
    (byte & 0x40 ? '1' : '0'),                  \
    (byte & 0x20 ? '1' : '0'),                  \
    (byte & 0x10 ? '1' : '0'),                  \
    (byte & 0x08 ? '1' : '0'),                  \
    (byte & 0x04 ? '1' : '0'),                  \
    (byte & 0x02 ? '1' : '0'),                  \
    (byte & 0x01 ? '1' : '0')


struct entry {
  const char *symbol;
  long weight;
};

int compare_entries(const void *v1, const void *v2) {
  struct entry *a = (struct entry *)v1;
  struct entry *b = (struct entry *)v2;

  return a->weight > b->weight;
}

void disp_array(const char *msg, struct entry *A, int n) {
  printf("%s\n", msg);
  for (int i = 0; i < n; ++i) {
    printf ("%s\t%ld\t"_BITS"\n", A[i].symbol,
            A[i].weight, TO_BITS(A[i].weight));
  }
  printf("\n\n");
}

void calc_codes(struct entry *A, int n) {
    int i;
    disp_array("begin", A, n);

    // Phase 1
    int s,r,t;
    for(s=0, r=0, t=0; t < n-1; t++) {
        if (t == 3)
          disp_array("t=3", A, n);

        if(s>n || (r<t && A[r].weight < A[s].weight)) {
            A[t].weight = A[r].weight;
            A[r].weight = t;
            r++;
        }
        else {
            A[t].weight = A[s].weight;
            if (s > t)
              A[s].weight = 0;
            s++;
        }

        if(s>n || (r<t && A[r].weight < A[s].weight)) {
          A[t].weight += A[r].weight;
          A[r].weight = t;
          r++;
        }
        else {
          A[t].weight += A[s].weight;
          if (s > t)
            A[s].weight = 0;
          s++;
        }
    }

    disp_array("Phase 1", A, n);

    int depth = 1;
    int a = 1, u = 0, d = 0, x = n;

    A[n-1].weight = 0;
    for (t = n-2; t >= 0; t--) {
      A[t].weight = A[A[t].weight].weight + 1;
    }

    while (a > 0) {
      while (t >= 0 && A[t].weight == d) {
        u++;
        t--;
      }
      while (a > u) {
        A[x--].weight = d;
        a--;
      }
      a = 2*u;
      d++;
      u = 0;
    }

    disp_array("Code lengths", A, n);

    int max_len = depth-1;

    int *base = malloc(sizeof(*base) * n);
    int *offset = malloc(sizeof(*offset) * n);

    // kraft check
    double kraft = 0;
    for (i = 0; i < n; ++i) {
      kraft += pow(2.0, -A[i].weight);
    }

    fprintf(stderr, "kraft check: %f\n", kraft);
    assert(kraft == 1);

    for (i = 0; i < n; ++i) {
      int len = A[i].weight;

      int maxsum = 0;
      for (int j = 0; j <= i-1; ++j) {
        maxsum += 2 << (max_len - A[j].weight);
      }

      int nbits = (1 << (len+1))-1;
      A[i].weight = (maxsum/(2 << (max_len - len))) & nbits;
    }

    free(base);
}


int main(int argc, char *argv[]) {
  char line[512];
  struct entry *entries;
  FILE *input;
  hash_t *hash = hash_new();
  long total = 0;

  input = argc <= 1 ? stdin : fopen(argv[1], "r");

  while (fgets(line, sizeof(line), input)) {
    line[strlen(line)-1] = '\0';

    khiter_t k = kh_get(ptr, hash, line);
    int nohit = k == kh_end(hash);
    long val = nohit ? 0 : (long)kh_value(hash, k);
    const char *key = nohit ? strdup(line) : kh_key(hash, k);
    hash_set(hash, (char*)key, (void*)(val+1));
    total++;
    line[0] = '\0';
  }

  int count = hash_size(hash);
  size_t alloc = sizeof(*entries) * count;

  entries = malloc(alloc);
  memset(entries, 0, alloc);

  int c = 0;

  hash_each(hash, {
      struct entry entry;

      printf("key '%s'\n", key);
      entry.symbol = key;
      entry.weight = (long)val;
      entries[c++] = entry;
  });

  /* qsort(entries, count, sizeof(*entries), compare_entries); */

  calc_codes(entries, count);
  disp_array("codes", entries, count);

  /* for (int i = 0; i < count; ++i) { */
  /*   struct entry *entry = &entries[i]; */

  /*   int code_len = ceil(1.4404 * log(entry->weight / total) / log(2)); */
  /*   if (code_len > max_code_len) */
  /*     max_code_len = code_len; */
  /* } */

  free(entries);

  fclose(input);

  return 0;
}
