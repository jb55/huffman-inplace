

#include <stdio.h>

int main(int argc, char *argv[]) {
  char line[512];
  FILE *input = argc == 0 ? stdin : fopen(argv[1], "rb");

  while (fgets(line, sizeof(line), input)) {
  }
}
