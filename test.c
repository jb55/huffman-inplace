
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
  int code;
};

int compare_entries(const void *v1, const void *v2) {
  struct entry *a = (struct entry *)v1;
  struct entry *b = (struct entry *)v2;

  return a->weight > b->weight;
}

void disp_array(const char *msg, struct entry *A, int n) {
  printf("%s\n", msg);
  for (int i = 0; i < n; ++i) {
    printf ("%s\t%ld\n", A[i].symbol, A[i].weight);
  }
  printf("\n\n");
}

void disp_code(struct entry *A, int n) {
  for (int i = 0; i < n; ++i) {
    printf ("%s\t", A[i].symbol);
    for (int j = A[i].weight-1; j >= 0; j--) {
      putchar(A[i].code & (1 << j) ? '1' : '0');
    }
    putchar('\n');
  }
  /* printf("\n\n"); */
}

void calc_codes(struct entry *A, int n) {
    int i;
    /* disp_array("begin", A, n); */

    // Phase 1
    int s,r,t;
    for(s=0, r=0, t=0; t < n-1; t++) {
        /* if (t == 3) */
        /*   disp_array("t=3", A, n); */

        if(s>=n || (r<t && A[r].weight < A[s].weight)) {
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

        if(s>=n || (r<t && A[r].weight < A[s].weight)) {
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

    /* printf("s %d t %d\n", n-s+1, n-t); */

    /* disp_array("Phase 1", A, n); */

    int level_top = n - 2; //root
    int depth = 1;
    i = n;
    int j, k;

    int total_nodes_at_level = 2;
    while(i > 0) {
      for(k=level_top; k>0 && A[k-1].weight >=level_top; k--) {}

      int internal_nodes_at_level = level_top - k;
      int leaves_at_level = total_nodes_at_level - internal_nodes_at_level;
      for(j=0; j<leaves_at_level; j++) {
        A[--i].weight = depth;
      }

      total_nodes_at_level = internal_nodes_at_level * 2;
      level_top = k;
      depth++;
    }

    /* disp_array("Code lengths", A, n); */

    int max_len = depth-1;

    // kraft check
    double kraft = 0;
    for (i = 0; i < n; ++i) {
      kraft += pow(2.0, -A[i].weight);
    }

    /* fprintf(stderr, "kraft check: %f\n", kraft); */

    /* fprintf(stderr, "max_len: %d\n", max_len); *\/ */

    for (i = 0; i < n; ++i) {
      int len = A[i].weight;

      int maxsum = 0;
      for (int j = 0; j < i; ++j) {
        maxsum += 2 << (max_len - A[j].weight);
      }

      int nbits = (1 << (len+1))-1;
      A[i].code = (maxsum / (2 << (max_len - len))) & nbits;
    }
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
  size_t alloc = sizeof(struct entry) * count;

  entries = malloc(alloc);
  memset(entries, 0, alloc);

  int c = 0;

  hash_each(hash, {
      struct entry entry;
      entry.symbol = key;
      entry.weight = (long)val;
      entries[c++] = entry;
  });

  qsort(entries, count, sizeof(*entries), compare_entries);

  calc_codes(entries, count);
  disp_code(entries, count);

  //free(entries);

  fclose(input);

  return 0;
}
