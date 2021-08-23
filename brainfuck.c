#include <stdio.h>
#include <stdlib.h>
#define BYTES 30000

char memory[BYTES];

void zeroMemory() {
  int i;
  for(i = 0; i < BYTES; i++) memory[i] = 0;
}

int main() {
  FILE *code;
  int c;
  char *p;

  code = fopen("code.b", "r");
  if(code == NULL) {
    fputs("error: could not open code.b", stderr);
    exit(1);
  }

  p = memory;

  zeroMemory();

  for(c = getc(code); c != EOF; c = getc(code)) {
    switch(c) {
    case '>':
      p++;
      break;
    case '<':
      p--;
      break;
    case '+':
      (*p)++;
      break;
    case '-':
      (*p)--;
      break;
    case '.':
      putchar(*p);
      break;
    case ',':
      *p = getchar();
      break;
    }
  }
  return 0;
}
