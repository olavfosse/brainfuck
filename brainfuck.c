#include <stdio.h>
#include <stdlib.h>

int codeIndex = 0;
char *code;

void readCode() {
  FILE *fp;
  int len;

  fp = fopen("code.b", "r");
  if(fp == NULL) goto error;

  if(fseek(fp, 0, SEEK_END) == -1) goto error;

  len = ftell(fp);
  if(len == -1) goto error;

  if(fseek(fp, 0, SEEK_SET) == -1) goto error;

  code = malloc(len * sizeof(char) + 1);
  if(code == NULL) goto error;

  code[len] = '\0';

  if(fread(code, len, 1, fp) == 0 && len != 0) goto error;

  if(fclose(fp) == EOF) goto error;
  return;

 error:
  perror("error: could not read code.b");
  exit(1);
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

  fp = fopen("code.asm","w");
  if(fp == NULL) goto write_error;

  if(fputs(
"section .bss\n"
  "\tbuffer: resb 30000\n"
"global _start\n"
"section .text\n"
"_start:\n"
  "\tmov r15, buffer\n"
  ,fp) == EOF) goto write_error;

  for(; code[codeIndex] != '\0'; codeIndex++) {
    switch(code[codeIndex]) {
    case '>':
      if(fputs("\tinc r15\n", fp) == EOF) goto write_error;
      break;
    case '<':
      if(fputs("\tdec r15\n", fp) == EOF) goto write_error;
      break;
    case '+':
      if(fputs("\tinc byte [r15]\n", fp) == EOF) goto write_error;
      break;
    case '-':
      if(fputs("\tdec byte [r15]\n", fp) == EOF) goto write_error;
      break;
    case '.':
      if(fputs(
  "\tmov rax, 1\n"
  "\tmov rdi, 1\n"
  "\tmov rsi, r15\n"
  "\tmov rdx, 1\n"
  "\tsyscall\n"
      , fp) == EOF) goto write_error;
      break;
    case ',':
      if(fputs(
  "\tmov rax, 0\n"
  "\tmov rdi, 0\n"
  "\tmov rsi, r15\n"
  "\tmov rdx, 1\n"
  "\tsyscall\n"
      , fp) == EOF) goto write_error;
      break;
    case '[':
      if(0 > fprintf(fp,
  "\tcmp byte [r15], 0\n"
  "\tje .rightbracket%d\n"
  "\t.leftbracket%d:\n"
      , nextLeftBracketId, nextLeftBracketId)) goto write_error;
      topLeftBracketIdIndex++;
      leftBracketIdStack[topLeftBracketIdIndex] = nextLeftBracketId;
      nextLeftBracketId++;
      break;
    case ']':
      if(0 > fprintf(fp,
  "\tcmp byte [r15], 0\n"
  "\tjne .leftbracket%d\n"
  "\t.rightbracket%d:\n"
      , leftBracketIdStack[topLeftBracketIdIndex], leftBracketIdStack[topLeftBracketIdIndex])) goto write_error;
      topLeftBracketIdIndex--;
      break;
    }
  }

  /* exit with 0 status code */
  if(fputs(
  "\tmov rax, 60\n"
  "\tmov rdi, 0\n"
  "\tsyscall\n"
  , fp) == EOF) goto write_error;

  if(fclose(fp) == EOF) goto write_error;

  system("nasm -f elf64 -o code.o code.asm && ld code.o -o code");
  return 0;

 write_error:
  perror("error: could not write to code.asm");
  exit(1);
}
