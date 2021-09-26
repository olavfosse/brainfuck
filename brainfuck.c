#include <stdio.h>
#include <stdlib.h>

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

  int currentLine = 1;
  int currentColumn = 1;

  int uppermostLeftBracketLine = -1;
  int uppermostLeftBracketColumn = -1;

  for(; code[codeIndex] != '\0';) {
    switch(code[codeIndex]) {
    case '[':
      if(bracketDepth == 0) {
	uppermostLeftBracketLine = currentLine;
	uppermostLeftBracketColumn = currentColumn;
      }
      bracketDepth++;
      break;
    case ']':
      if(bracketDepth == 0) {
	fprintf(stderr, "error: %d. char in %d. line is an unmatched ']'\n", currentColumn, currentLine);
	exit(1);
      }
      bracketDepth--;
      break;
    case '\n':
      currentLine++;
      currentColumn = 0;
      break;
    }
    codeIndex++;
    currentColumn++;
  }
  if(bracketDepth != 0) {
    fprintf(stderr, "error: %d. char in %d. line is an unmatched '['\n", uppermostLeftBracketColumn, uppermostLeftBracketLine);
    exit(1);
  }
  codeIndex = 0;
}

int main() {
  FILE *fp;
  int bracketDepth;

  readCode();
  validateCode();

  fp = fopen("code.asm", "w");
  if(fp == NULL) {
    fputs("error: could not open code.asm\n", stderr);
    exit(1);
  }

  fputs("section .bss\n\tbuffer: resb 30000\nglobal _start\nsection .text\n_start:\n\tmov r15, buffer\n", fp);

  /* bracketDepth is equal to n - m where n is the number of '[' read
     and m is the number of ']' read. */
  bracketDepth = 0;

  for(; code[codeIndex] != '\0'; codeIndex++) {
    switch(code[codeIndex]) {
    case '>':
      fputs("\tinc r15\n", fp);
      break;
    case '<':
      fputs("\tdec r15\n", fp);
      break;
    case '+':
      fputs("\tinc byte [r15]\n", fp);
      break;
    case '-':
      fputs("\tdec byte [r15]\n", fp);
      break;
    case '.':
      fputs("\tmov rax, 1\n\tmov rdi, 1\n\tmov rsi, r15\n\tmov rdx, 1\n\tsyscall\n", fp);
      break;
    case ',':
      fputs("\tmov rax, 0\n\tmov rdi, 0\n\tmov rsi, r15\n\tmov rdx, 1\n\tsyscall\n", fp);
      break;
    case '[':
      bracketDepth++;
    case ']':
      bracketDepth--;
    }
  }

  /* exit with 0 status code */
  fputs("\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n", fp);

  fclose(fp);

  system("nasm -f elf64 -o code.o code.asm && ld code.o -o code");
  return 0;
}

/* leftBracketToLabel writes the label representing the leftBracket at depth bracketDepth to code.asm. */
void leftBracketToLabel(int bracketDepth);

/* rightBracketToLabel writes the label representing the rightBracket at depth bracketDepth to code.asm. */
void rightBracketToLabel(int bracketDepth);
