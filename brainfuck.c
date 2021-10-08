#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read(char *filename) {
  FILE *fp;
  int len;
  char *code;

  fp = fopen(filename, "r");
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
  return code;

 error:
  fprintf(stderr, "error: could not read %s: ", filename);
  perror(NULL);
  exit(1);
}

/* validates that all '[' have matching ']' and vice versa. */
void validate(char *code) {
  /* the error reporting strategy is to report as early as
     possible. */
  int bracketDepth = 0;

  int currentLine = 1;
  int currentColumn = 1;

  int uppermostLeftBracketLine = -1;
  int uppermostLeftBracketColumn = -1;

  int codeIndex = 0;
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
}

void compile_to_x64(char *code, char *target_filename) {
  FILE *fp;
  /* adhoc stack solution since it only works for 1024 elements, oh well.*/
  int leftBracketIdStack[1024];
  int topLeftBracketIdIndex = -1;
  int nextLeftBracketId = 0;
  int codeIndex = 0;

  fp = fopen(target_filename,"w");
  if(fp == NULL) goto error;

  if(fputs(
"section .bss\n"
  "\tbuffer: resb 30000\n"
"global _start\n"
"section .text\n"
"_start:\n"
  "\tmov r15, buffer\n"
  ,fp) == EOF) goto error;

  for(; code[codeIndex] != '\0'; codeIndex++) {
    switch(code[codeIndex]) {
    case '>':
      if(fputs("\tinc r15\n", fp) == EOF) goto error;
      break;
    case '<':
      if(fputs("\tdec r15\n", fp) == EOF) goto error;
      break;
    case '+':
      if(fputs("\tinc byte [r15]\n", fp) == EOF) goto error;
      break;
    case '-':
      if(fputs("\tdec byte [r15]\n", fp) == EOF) goto error;
      break;
    case '.':
      if(fputs(
  "\tmov rax, 1\n"
  "\tmov rdi, 1\n"
  "\tmov rsi, r15\n"
  "\tmov rdx, 1\n"
  "\tsyscall\n"
      , fp) == EOF) goto error;
      break;
    case ',':
      if(fputs(
  "\tmov rax, 0\n"
  "\tmov rdi, 0\n"
  "\tmov rsi, r15\n"
  "\tmov rdx, 1\n"
  "\tsyscall\n"
      , fp) == EOF) goto error;
      break;
    case '[':
      if(0 > fprintf(fp,
  "\tcmp byte [r15], 0\n"
  "\tje .rightbracket%d\n"
  "\t.leftbracket%d:\n"
      , nextLeftBracketId, nextLeftBracketId)) goto error;
      topLeftBracketIdIndex++;
      leftBracketIdStack[topLeftBracketIdIndex] = nextLeftBracketId;
      nextLeftBracketId++;
      break;
    case ']':
      if(0 > fprintf(fp,
  "\tcmp byte [r15], 0\n"
  "\tjne .leftbracket%d\n"
  "\t.rightbracket%d:\n"
      , leftBracketIdStack[topLeftBracketIdIndex], leftBracketIdStack[topLeftBracketIdIndex])) goto error;
      topLeftBracketIdIndex--;
      break;
    }
  }

  if(fputs(
  "\tmov rax, 60\n"
  "\tmov rdi, 0\n"
  "\tsyscall\n"
  , fp) == EOF) goto error;

  if(fclose(fp) == EOF) goto error;

  return;

 error:
  fprintf(stderr, "error: could not write to %s: ", target_filename);
  perror(NULL);
  exit(1);
}

void compile_to_c89(char *code, char *target_filename) {
  FILE *fp;
  /* adhoc stack solution since it only works for 1024 elements, oh well.*/
  int leftBracketIdStack[1024];
  int topLeftBracketIdIndex = -1;
  int nextLeftBracketId = 0;
  int codeIndex = 0;

  fp = fopen(target_filename,"w");
  if(fp == NULL) goto error;

  if(fputs(
"#include <stdio.h>\n"
"\n"
"char buf[30000];\n" /* since this is a global variable we don't need to manually set all elements to 0 */
"int i = 0;\n"
"\n"
"int main() {\n"
  ,fp) == EOF) goto error;

  for(; code[codeIndex] != '\0'; codeIndex++) {
    switch(code[codeIndex]) {
    case '>':
      if(fputs("\ti++;\n", fp) == EOF) goto error;
      break;
    case '<':
      if(fputs("\ti--;\n", fp) == EOF) goto error;
      break;
    case '+':
      if(fputs("\tbuf[i]++;\n", fp) == EOF) goto error;
      break;
    case '-':
      if(fputs("\tbuf[i]--;\n", fp) == EOF) goto error;
      break;
    case '.':
      if(fputs("\tputchar(buf[i]);\n", fp) == EOF) goto error;
      break;
    case ',':
      if(fputs("\tbuf[i] = getchar();\n", fp) == EOF) goto error;
      break;
    case '[':
      if(0 > fprintf(fp,
  "\tif(buf[i] == 0) goto right%d;\n"
  "\tleft%d:\n"
      , nextLeftBracketId, nextLeftBracketId)) goto error;
      topLeftBracketIdIndex++;
      leftBracketIdStack[topLeftBracketIdIndex] = nextLeftBracketId;
      nextLeftBracketId++;
      break;
    case ']':
      if(0 > fprintf(fp,
  "\tif(buf[i] != 0) goto left%d;\n"
  "\tright%d:\n"
      , leftBracketIdStack[topLeftBracketIdIndex], leftBracketIdStack[topLeftBracketIdIndex])) goto error;
      topLeftBracketIdIndex--;
      break;
    }
  }

  if(fputs(
"\treturn 0;\n" /* return is required as otherwise we risk having a label at the end of main. (which is illegal) */
"}\n"
  , fp) == EOF) goto error;

  if(fclose(fp) == EOF) goto error;

  return;

 error:
  fprintf(stderr, "error: could not write to %s: ", target_filename);
  perror(NULL);
  exit(1);
}

int main(int argc, char** argv) {
  char *code;
  void (*compile)(char *, char *);


  if(argc != 4) {
    fputs("usage: brainfuck target input output\n", stderr);
    exit(1);
  }

  if(strcmp(argv[1], "x64") == 0) {
    compile = compile_to_x64;
  } else if(strcmp(argv[1], "c89") == 0) {
    compile = compile_to_c89;
  } else {
    fputs("error: target must be x64 or c89\n", stderr);
    exit(1);
  }

  code = read(argv[2]);
  validate(code);
  compile(code, argv[3]);

  return 0;
}
