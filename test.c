
#define __STDC_WANT_LIB_EXT2__ 1
#include <hash/hash.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
    printf ("%s\t%ld\n", A[i].symbol, A[i].weight);
  }
  printf("\n\n");
}

int calc_code_lengths(struct entry *A, int n) {
    int i;

    // Phase 1
    int s,r,t;
    for(s=0, r=0, t=0; t<n-1; t++) {
        int sum = 0;
        for(i=0; i<2; i++) {
            if(s>=n || (r<t && A[r].weight < A[s].weight)) {
                sum += A[r].weight;
                A[r].weight = t;
                r++;
            }
            else {
                sum += A[s].weight;
                if(s > t) {
                    A[s].weight = 0;
                }
                s++;
            }
        }
        A[t].weight = sum;
    }

    int level_top = n - 2; //root
    int depth = 1;
    int j, k;
    i = n;
    int total_nodes_at_level = 2;

    while(i > 0) {
      for(k=level_top; k>0 && A[k-1].weight >= level_top; k--) {}

      int internal_nodes_at_level = level_top - k;
      int leaves_at_level = total_nodes_at_level - internal_nodes_at_level;
      for(j=0; j<leaves_at_level; j++) {
        A[--i].weight = depth;
      }

      total_nodes_at_level = internal_nodes_at_level * 2;
      level_top = k;
      depth++;
    }

    return depth-1;
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
  }

  int count = hash_size(hash);
  size_t alloc = sizeof(*entries) * count;

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

  int max_code_len = calc_code_lengths(entries, count);
  disp_array("code lengths", entries, count);
  printf("max_code_len %d\n", max_code_len);

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
