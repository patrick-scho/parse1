#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <log.h>

typedef enum TokenType {
  TOKEN_WORD, TOKEN_STRING, TOKEN_NUMBER, TOKEN_PAR_OPEN, TOKEN_PAR_CLOSE
} TokenType;

typedef enum LexState {
  LEX_NONE, LEX_WHITESPACE, LEX_WORD, LEX_STRING, LEX_NUMBER, LEX_PAR_OPEN, LEX_PAR_CLOSE
} LexState;

const char *tokentype_to_s(TokenType type) {
  switch (type) {
  case TOKEN_WORD:      return "TOKEN_WORD";
  case TOKEN_STRING:    return "TOKEN_STRING";
  case TOKEN_NUMBER:    return "TOKEN_NUMBER";
  case TOKEN_PAR_OPEN:  return "TOKEN_PAR_OPEN";
  case TOKEN_PAR_CLOSE: return "TOKEN_PAR_CLOSE";
  default:              return "UNDEFINED";
  }
}

const char *lexstate_to_s(LexState state) {
  switch (state) {
    case LEX_NONE: return "LEX_NONE";
    case LEX_WHITESPACE: return "LEX_WHITESPACE";
    case LEX_WORD: return "LEX_WORD";
    case LEX_STRING: return "LEX_STRING";
    case LEX_NUMBER: return "LEX_NUMBER";
    case LEX_PAR_OPEN: return "LEX_PAR_OPEN";
    case LEX_PAR_CLOSE: return "LEX_PAR_CLOSE";
    default: return "UNDEFINED";
  }
}

bool is_whitespace(char c) {
  return
    c == ' '  ||
    c == '\t' ||
    c == '\r' ||
    c == '\n';
}

bool is_digit(char c) {
  return (c >= '0' && c <= '9');
}

typedef struct Token {
  TokenType type;
  char *content;
  struct Token *next, *prev;
} Token;

void free_token(Token *token) {
  Token *ptr = token;
  if (ptr == NULL)       log_error("Token is NULL");
  if (ptr->prev != NULL) log_error("Didn't provide first Token");

  while (ptr != NULL) {
    free(ptr->content);
    Token *next = ptr->next;
    free(ptr);
    ptr = next;
  }
}

void print(Token *token) {
  Token *ptr = token;
  if (ptr == NULL)       log_error("Token is NULL");
  if (ptr->prev != NULL) log_error("Didn't provide first Token");

  while (ptr != NULL) {
    printf("%s(%s)\n", tokentype_to_s(ptr->type), ptr->content);
    ptr = ptr->next;
  }
}

long read_file(const char *filename, char **buffer) {
  FILE *file = fopen(filename, "rb+");

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

Token *new_token(TokenType type, char *content) {
  Token *result = malloc(sizeof(Token));
  result->type = type;
  result->content = malloc(strlen(content));
  strcpy(result->content, content);
  result->prev = result->next = NULL;
}

Token *add_token(Token *t1, Token *t2) {
  if (t2 == NULL) log_error("t2 is NULL");
  if (t1 == NULL) return t2;
  t1->next = t2;
  t2->prev = t1;
  return t2;
}

Token *lex(const char *file, size_t size) {
  LexState state = LEX_NONE;

  Token *result = NULL, *current = NULL;

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
      } else
      if (c == ')') {
        state = LEX_PAR_CLOSE;
      } else
      if (is_whitespace(c)) {
        state = LEX_WHITESPACE;
      } else
      if (is_digit(c) || ((c == '+' || c == '-') && is_digit(file[i + 1]))) {
        state = LEX_NUMBER;
        begin = i;
      } else
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
        char *str = malloc(length + 1);
        strncpy(str, file + begin, length);
        str[length] = 0;
        Token *token;
        if (state == LEX_WORD)   token = new_token(TOKEN_WORD, str);
        if (state == LEX_NUMBER) token = new_token(TOKEN_NUMBER, str);
        current = add_token(current, token);
        free(str);

        if (is_whitespace(c)) state = LEX_WHITESPACE;
        if (c == '(')         state = LEX_PAR_OPEN;
        if (c == ')')         state = LEX_PAR_CLOSE;
      }
      break;
    }

    if (result == NULL && current != NULL)
      result = current;

    log_trace("%c %s", c, lexstate_to_s(state), current == NULL ? 0 : 1);
  }

  return result;
}

int main(int argc, char **argv) {
  log_set_level(LOG_TRACE);

  char *file;
  size_t size = read_file("test1", &file);

  Token *t = lex(file, size);
  print(t);

  free(file);

  //Program program = parse(t);

  free_token(t);

  return 0;
}