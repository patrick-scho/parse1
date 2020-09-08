#include <stdio.h>
#include <string.h>
#include <time.h>

#include <log.h>

#include "memory.h"
#include "util.h"

#define WORD_SIZE_MAX 64

#define SOME_ENUM(X) \
    X(TOKEN_WORD,) \
    X(TOKEN_STRING,) \
    X(TOKEN_NUMBER,) \
    X(TOKEN_PAR_OPEN,) \
    X(TOKEN_PAR_CLOSE,) \

DEF_ENUM(TokenType, SOME_ENUM)

#define SOME_ENUM(X) \
    X(LEX_NONE,) \
    X(LEX_WHITESPACE,) \
    X(LEX_WORD,) \
    X(LEX_STRING,) \
    X(LEX_NUMBER,) \
    X(LEX_PAR_OPEN,) \
    X(LEX_PAR_CLOSE,) \

DEF_ENUM(LexState, SOME_ENUM)

bool is_whitespace(char c) {
  return
    c == ' ' ||
    c == '\t' ||
    c == '\r' ||
    c == '\n';
}

bool is_digit(char c) {
  return (c >= '0' && c <= '9');
}

typedef struct Token {
  TokenType type;
  //char *content;
  char content[WORD_SIZE_MAX + 1];
  struct Token* next, * prev;
} Token;

Allocator* token_allocator;

Token* new_token(TokenType type, char* content) {
  Token* result = allocator_get(token_allocator, 1);
  result->type = type;
  size_t length = strlen(content);
  //result->content = malloc(length + 1);
  strncpy(result->content, content, length + 1);
  result->prev = result->next = NULL;
  return result;
}

Token* add_token(Token* t1, Token* t2) {
  if (t2 == NULL) log_error("t2 is NULL");
  if (t1 == NULL) return t2;
  t1->next = t2;
  t2->prev = t1;
  return t2;
}

void free_token(Token* token) {
  Token* ptr = token;
  if (ptr == NULL)       log_error("Token is NULL");
  if (ptr->prev != NULL) log_error("Didn't provide first Token");

  while (ptr != NULL) {
    //free(ptr->content);
    Token* next = ptr->next;
    free(ptr);
    ptr = next;
  }
}

void print(Token* token) {
  Token* ptr = token;
  if (ptr == NULL)       log_error("Token is NULL");
  if (ptr->prev != NULL) log_error("Didn't provide first Token");

  while (ptr != NULL) {
    printf("%s(%s)\n", TokenType_to_s(ptr->type), ptr->content);
    ptr = ptr->next;
  }
}

long read_file(const char* filename, char** buffer) {
  FILE* file = fopen(filename, "rb+");

  if (!file) {
    log_error("Unable to open file");
  }

  fseek(file, 0, SEEK_END);
  long filesize = ftell(file);
  fseek(file, 0, SEEK_SET);

  *buffer = malloc(filesize + 1);
  size_t read = fread(*buffer, 1, filesize, file);
  if (read != filesize) {
    log_error("Incomplete file read (%d/%d)", read, filesize);
  }
  (*buffer)[filesize] = 0;

  fclose(file);

  return read;
}

Token* lex(const char* file, size_t size) {
  LexState state = LEX_NONE;

  Token* result = NULL, * current = NULL;

  size_t begin, end;

  for (size_t i = 0; i < size; i++) {
    char c = file[i];
    switch (state) {
    case LEX_NONE:
    case LEX_WHITESPACE:
    case LEX_PAR_OPEN:
    case LEX_PAR_CLOSE:
      if (state == LEX_PAR_OPEN)
        current = add_token(current, new_token(TOKEN_PAR_OPEN, ""));
      if (state == LEX_PAR_CLOSE)
        current = add_token(current, new_token(TOKEN_PAR_CLOSE, ""));
      if (c == '(') {
        state = LEX_PAR_OPEN;
      }
      else
        if (c == ')') {
          state = LEX_PAR_CLOSE;
        }
        else
          if (is_whitespace(c)) {
            state = LEX_WHITESPACE;
          }
          else
            if (is_digit(c) || ((c == '+' || c == '-') && is_digit(file[i + 1]))) {
              state = LEX_NUMBER;
              begin = i;
            }
            else
              if (c == '"') {
                state = LEX_STRING;
                begin = i;
              }
              else {
                state = LEX_WORD;
                begin = i;
              }
      break;
    case LEX_WORD:
    case LEX_NUMBER:
      if (is_whitespace(c) || c == '(' || c == ')') {
        size_t length = i - begin;
        if (length > WORD_SIZE_MAX) length = WORD_SIZE_MAX;
        //char *str = malloc(length + 1);
        static char str[WORD_SIZE_MAX + 1];
        strncpy(str, file + begin, length);
        str[length] = 0;
        Token* token = NULL;
        if (state == LEX_WORD)   token = new_token(TOKEN_WORD, str);
        if (state == LEX_NUMBER) token = new_token(TOKEN_NUMBER, str);
        current = add_token(current, token);
        //free(str);

        if (is_whitespace(c)) state = LEX_WHITESPACE;
        if (c == '(')         state = LEX_PAR_OPEN;
        if (c == ')')         state = LEX_PAR_CLOSE;
      }
      break;
    }

    if (result == NULL && current != NULL)
      result = current;

    log_trace("%c %s", c, LexState_to_s(state), current == NULL ? 0 : 1);
  }

  return result;
}

int main(int argc, char** argv) {
  log_set_level(LOG_DEBUG);

  for (int i = 0; i < 10; i++) {
    token_allocator = allocator_new(sizeof(Token), 1);

    clock_t timer = clock();

    char* file;
    size_t size = read_file("parse1/test2", &file);
    printf("read_file %f seconds\n", (double)(clock() - timer) / CLOCKS_PER_SEC);
    timer = clock();

    Token* t = lex(file, size);
    printf("lex       %f seconds\n", (double)(clock() - timer) / CLOCKS_PER_SEC);
    timer = clock();

    Token* ptr = t;
    int count = 0;
    while (ptr != NULL) {
      ptr = ptr->next;
      count++;
    }
    printf("%d\n", count);
    printf("test      %f seconds\n", (double)(clock() - timer) / CLOCKS_PER_SEC);
    timer = clock();

    free(file);
    //print(t);

    //Program program = parse(t);

    //free_token(t);

    allocator_free(token_allocator);
  }

  return 0;
}