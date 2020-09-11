#include <stdio.h>
#include <string.h>
#include <time.h>

#include <log.h>

#include "memory.h"
#include "util.h"

#define WORD_SIZE_MAX 64

#define SOME_ENUM(X) \
    X(TOKEN_WORD,) \
    X(TOKEN_STRING_LIT,) \
    X(TOKEN_NUMBER_LIT,) \
    X(TOKEN_BOOLEAN_LIT,) \
    X(TOKEN_PAR_OPEN,) \
    X(TOKEN_PAR_CLOSE,) \
    X(TOKEN_SYMBOL,) \

DEF_ENUM(TokenType, SOME_ENUM)

#define SOME_ENUM(X) \
    X(LEX_NONE,) \
    X(LEX_WHITESPACE,) \
    X(LEX_WORD,) \
    X(LEX_STRING_LIT,) \
    X(LEX_NUMBER_LIT,) \
    X(LEX_BOOLEAN_LIT,) \
    X(LEX_PAR_OPEN,) \
    X(LEX_PAR_CLOSE,) \
    X(LEX_SYMBOL,) \

DEF_ENUM(LexState, SOME_ENUM)

#define SOME_ENUM(X) \
    X(EXPR_LIST,) \
    X(EXPR_SYM,) \
    X(EXPR_QUOTE,) \
    X(EXPR_ID,) \
    X(EXPR_STRING_LIT,) \
    X(EXPR_NUMBER_LIT,) \
    X(EXPR_BOOLEAN_LIT,) \
    X(EXPR_OP_CALL,) \
    X(EXPR_FN_CALL,) \

DEF_ENUM(ExprType, SOME_ENUM)

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
  char content[WORD_SIZE_MAX + 1];
} Token;

//typedef struct Op {
//} Op;
//
//typedef struct Fn {
//} Fn;
//
//typedef struct Var {
//} Var;
//
//typedef struct Expr {
//  ExprType type;
//  Expr children;
//  size_t index, next;
//} Expr;
//
//typedef struct Context {
//  Op ops;
//  Fn fns;
//  Var vars;
//  Expr exprs;
//} Context;
//
//typedef struct Program {
//  Context* main;
//} Program;

void new_token(Allocator* allocator, TokenType type, char* content) {
  size_t index = allocator_get(allocator, 1);
  Token* result = allocator_at(allocator, index);
  result->type = type;
  size_t length = strlen(content);
  strncpy(result->content, content, length);
  result->content[length] = 0;
}

void print(Allocator* allocator) {
  for (size_t i = 0; i < allocator->count; i++) {
    Token* val = allocator_at(allocator, i);
    printf("%s(%s)\n", TokenType_to_s(val->type), val->content);
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

void lex(const char* file, size_t size, Allocator* result) {
  LexState state = LEX_NONE;

  size_t begin;

  for (size_t i = 0; i < size; i++) {
    char c = file[i];
    switch (state) {
    case LEX_NONE:
    case LEX_WHITESPACE:
    case LEX_PAR_OPEN:
    case LEX_PAR_CLOSE:
      if (state == LEX_PAR_OPEN)
        new_token(result, TOKEN_PAR_OPEN, "");
      if (state == LEX_PAR_CLOSE)
        new_token(result, TOKEN_PAR_CLOSE, "");
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
              state = LEX_NUMBER_LIT;
              begin = i;
            }
            else
              if (c == '"') {
                state = LEX_STRING_LIT;
                begin = i + 1;
              }
              else
                if (c == '\'') {
                  state = LEX_SYMBOL;
                  begin = i + 1;
                }
                else {
                  state = LEX_WORD;
                  begin = i;
                }
      break;
    case LEX_WORD:
    case LEX_NUMBER_LIT:
    case LEX_SYMBOL:
      if (is_whitespace(c) || c == '(' || c == ')') {
        size_t length = i - begin;
        if (length > WORD_SIZE_MAX) length = WORD_SIZE_MAX;
        static char str[WORD_SIZE_MAX + 1];
        strncpy(str, file + begin, length);
        str[length] = 0;
        if (state == LEX_WORD)   new_token(result, TOKEN_WORD, str);
        if (state == LEX_NUMBER_LIT) new_token(result, TOKEN_NUMBER_LIT, str);
        if (state == LEX_SYMBOL) new_token(result, TOKEN_SYMBOL, str);

        if (is_whitespace(c)) state = LEX_WHITESPACE;
        if (c == '(')         state = LEX_PAR_OPEN;
        if (c == ')')         state = LEX_PAR_CLOSE;
      }
      break;
    case LEX_STRING_LIT:
      if (c == '"') {
        size_t length = i - begin;
        if (length > WORD_SIZE_MAX) length = WORD_SIZE_MAX;
        static char str[WORD_SIZE_MAX + 1];
        strncpy(str, file + begin, length);
        str[length] = 0;
        new_token(result, TOKEN_STRING_LIT, str);

        state = LEX_NONE;

      }
      break;
    }
  }
}

//Program* parse(Allocator* t, Allocator* p) {
//  return NULL;
//}

int main(int argc, char** argv) {
  log_set_level(LOG_DEBUG);

  char* file;
  size_t size = read_file("parse1/test4", &file);

  Allocator* t = allocator_new(sizeof(Token), 1024);
  lex(file, size, t);

  free(file);

  print(t);

  Allocator* p = allocator_new(1, 1024);
  //Program* prg = parse(t, p);

  allocator_free(p);
  allocator_free(t);

  return 0;
}