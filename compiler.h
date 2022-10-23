#ifndef __COMPILER_H
#define __COMPILE_H

#include <stdio.h>

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

// This will be used as return codes, if there was an error or if compiling
// went ok
enum
{
  COMPILER_FILE_COMPILED_OK,
  COMPILER_FAILED_WITH_ERRORS
};

struct compile_process
{
  // This will determine how code must be compiled
  int flags;

  struct compile_process_input_file
  {
    FILE *fp;
    const char *abs_path;
  } cfile;

  FILE *out_file;
};

int compile_file (const char *fname, const char *out_fname, int flags);

// create a populated "compile_process"
struct compile_process *
compile_process_create (const char *fname, const char *out_fname, int flags);

#endif
