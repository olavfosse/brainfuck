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
  /* adhoc stack solution since it only works for 1024 elements, oh well.*/
  int leftBracketIdStack[1024];
  int topLeftBracketIdIndex = -1;
  int nextLeftBracketId = 0;

  readCode();
  validateCode();

  fp = fopen("code.asm", "w");
  if(fp == NULL) {
    fputs("error: could not open code.asm\n", stderr);
    exit(1);
  }

  fputs("section .bss\n\tbuffer: resb 30000\nglobal _start\nsection .text\n_start:\n\tmov r15, buffer\n", fp);

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
      fprintf(fp, "\tcmp byte [r15], 0\n\tje .rightbracket%d\n\t", nextLeftBracketId);
      fprintf(fp, ".leftbracket%d:\n", nextLeftBracketId);
      topLeftBracketIdIndex++;
      leftBracketIdStack[topLeftBracketIdIndex] = nextLeftBracketId;
      nextLeftBracketId++;
      break;
    case ']':
      fprintf(fp, "\tcmp byte [r15], 0\n\tjne .leftbracket%d\n\t", leftBracketIdStack[topLeftBracketIdIndex]);
      fprintf(fp, ".rightbracket%d:\n", leftBracketIdStack[topLeftBracketIdIndex]);
      topLeftBracketIdIndex--;
      break;
    }
  }

  /* exit with 0 status code */
  fputs("\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n", fp);

  fclose(fp);

  system("nasm -f elf64 -o code.o code.asm && ld code.o -o code");
  return 0;
}
