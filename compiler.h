/*
 * compiler.h - Here are all the function, structures and enums definitions -
 * yeah, it's all in one file.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#ifndef __COMPILER_H
#define __COMPILE_H

#include <stdio.h>
#include <string.h>

#include "helpers/vector.h"

#define S_EQ(str, str2) \
  (str && str2 && (strcmp (str, str2) == 0))

#define NUMERIC_CASE                                                          \
  case '0':                                                                   \
  case '1':                                                                   \
  case '2':                                                                   \
  case '3':                                                                   \
  case '4':                                                                   \
  case '5':                                                                   \
  case '6':                                                                   \
  case '7':                                                                   \
  case '8':                                                                   \
  case '9'

#define OPERATOR_CASE_EXCLUDING_DIVISION                                      \
  case '+':                                                                   \
  case '-':                                                                   \
  case '*':                                                                   \
  case '>':                                                                   \
  case '<':                                                                   \
  case '^':                                                                   \
  case '%':                                                                   \
  case '!':                                                                   \
  case '=':                                                                   \
  case '~':                                                                   \
  case '|':                                                                   \
  case '&':                                                                   \
  case '(':                                                                   \
  case '[':                                                                   \
  case ',':                                                                   \
  case '.':                                                                   \
  case '?'

struct pos
{
  // store the position we are at
  int line;
  int col;
  const char *fname;
};

enum
{
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_KEYWORD,
  TOKEN_TYPE_OPERATOR,
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_COMMENT,
  TOKEN_TYPE_NEWLINE
};

struct token
{
  int type;
  int flags;
  struct pos pos;

  union
  {
    char cval;
    const char *sval;
    unsigned int inum;
    unsigned long lnum;
    unsigned long long llnum;
    void *any;
  };

  // is there a whitespace between the token and next token
  // i.e. * a for operator * whitespace would be set for token "a"
  _Bool whitespace;

  // if it's between brackets, points to the opening bracket
  // i.e. (5+10+20) tokens 5, 10 and 20, will point to "("
  const char *between_brackets;
};

struct lex_process;
typedef char (*LEX_PROCESS_NEXT_CHAR) (struct lex_process *process);
typedef char (*LEX_PROCESS_PEEK_CHAR) (struct lex_process *process);
typedef void (*LEX_PROCESS_PUSH_CHAR) (struct lex_process *process, char c);

struct lex_process_functions
{
  LEX_PROCESS_NEXT_CHAR next_char;
  LEX_PROCESS_PEEK_CHAR peek_char;
  LEX_PROCESS_PUSH_CHAR push_char;
};

struct lex_process
{

  struct pos pos;
  struct vector *token_vec;
  struct compile_process *compiler;

  /*
   * number of brackets, in ((60)) for example, it'd be 2
   */
  int current_expression_count;
  struct buffer *parentheses_buffer;
  struct lex_process_functions *function;

  // point to private data that the lexer does not understand, but the person
  // using the lexer does.
  void *private;
};

// this will be used as return codes, if there was an error or if compiling
// went ok
enum
{
  COMPILER_FILE_COMPILED_OK,
  COMPILER_FAILED_WITH_ERRORS
};

enum
{
  LEXICAL_ANALYSIS_ALL_OK,
  LEXICAL_ANALYSIS_INPUT_ERROR
};

struct compile_process
{
  // this will determine how code must be compiled
  int flags;

  struct pos pos;
  struct compile_process_input_file
  {
    FILE *fp;
    const char *abs_path;
  } cfile;

  FILE *out_file;
};

void compiler_error (struct compile_process *compiler, const char *msg, ...);
void compiler_warning (struct compile_process *compiler, const char *msg, ...);
int compile_file (const char *fname, const char *out_fname, int flags);

// cprocess
struct compile_process *
compile_process_create (const char *fname, const char *out_fname, int flags);

char compile_process_next_char (struct lex_process *lex_process);
char compile_process_peek_char (struct lex_process *lex_process);
void compile_process_push_char (struct lex_process *lex_process, char c);

// lex_process
struct lex_process *
lex_process_create (struct compile_process *compiler,
                    struct lex_process_functions *functions, void *private);
void lex_process_free (struct lex_process *process);
void *lex_process_private (struct lex_process *process);
struct vector *lex_process_tokens (struct lex_process *process);

// lexer
int lex (struct lex_process *process);

// token
_Bool
token_is_keyword (struct token *token, const char *value);

#endif
