#include <stdio.h>
#include <stdlib.h>
#define BYTES 30000

char memory[BYTES];

void zeroMemory() {
  int i;
  for(i = 0; i < BYTES; i++) memory[i] = 0;
}

int codeIndex = 0;
char *code;

void readCode() {
  /* TODO: handle errors and understand wtf this is doing. */
  FILE *fp;
  int len;

  fp = fopen("code.b", "r");
  if(fp == NULL) {
    fputs("error: could not open code.b\n", stderr);
    exit(1);
  }

  fseek(fp, 0, SEEK_END);
  len = ftell(fp);
  rewind(fp);

  code = malloc(len * sizeof(char) + 1);
  code[len] = '\0';
  fread(code, len, 1, fp);
  fclose(fp);
}

/* validates that all '[' have matching ']' and vice versa. */
void validateCode() {
  /* the error reporting strategy is to report as early as
     possible. */
  int bracketDepth = 0;
  int uppermostLeftBracketIndex = -1;
  for(; code[codeIndex] != '\0'; codeIndex++) {
    if(code[codeIndex] == '[') {
      if(bracketDepth == 0) {
	uppermostLeftBracketIndex = codeIndex;
      }
      bracketDepth++;
    }
    else if(code[codeIndex] == ']') {
      if(bracketDepth == 0) {
	/* TODO: specify line and column */
	fprintf(stderr, "error: %d. char is an unmatched ']'\n", codeIndex + 1);
	exit(1);
      }
      bracketDepth--;
    }
  }
  if(bracketDepth != 0) {
    fprintf(stderr, "error: %d. char is an unmatched '['\n", uppermostLeftBracketIndex + 1);
    exit(1);
  }
  codeIndex = 0;
}

int main() {
  char *p;
  int bracketDepth;

  readCode();
  validateCode();

  p = memory;

  zeroMemory();

  /* bracketDepth is equal to n - m where n is the number of '[' read
     and m is the number of ']' read. */
  bracketDepth = 0;

  for(codeIndex++; code[codeIndex] != '\0'; codeIndex++) {
    switch(code[codeIndex]) {
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
      /* BUG: this line segfaults when ran with
	 http://brainfuck.org/400quine.b. The segfault only occurs
	 when the program is ran outside of gdb or inside of gdb with
	 `set disable-randomization off`. If it is ran inside of gdb
	 without setting disable-randomization it works fine. */
      (*p)--;
      break;
    case '.':
      putchar(*p);
      break;
    case ',':
      *p = getchar();
      break;
    case '[': {
      int originalBracketDepth = bracketDepth;
      bracketDepth++;
      if(*p == 0) {
	codeIndex++;
	for(;;) {
	  if(code[codeIndex] == '[') bracketDepth++;
	  else if(code[codeIndex] == ']') bracketDepth--;

	  if(bracketDepth == originalBracketDepth) break;
	  codeIndex++;
	}
      }
      break;
    }
    case ']': {
      int originalBracketDepth = bracketDepth;
      bracketDepth--;
      if(*p != 0) {
	codeIndex--;
	for(;;) {
	  if(code[codeIndex] == '[') bracketDepth++;
	  else if(code[codeIndex] == ']') bracketDepth--;

	  if(bracketDepth == originalBracketDepth) break;
	  codeIndex--;
	}
      }

    }
    }
  }

  return 0;
}
