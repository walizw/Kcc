/*
 * compiler.c - Compiles a file, passes through all its phases and produces the
 * output.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

#include <stdarg.h>
#include <stdlib.h>

struct lex_process_functions compiler_lex_functions
    = { .next_char = compile_process_next_char,
        .peek_char = compile_process_peek_char,
        .push_char = compile_process_push_char };

void
compiler_error (struct compile_process *compiler, const char *msg, ...)
{
  va_list args;
  va_start (args, msg);
  vfprintf (stderr, msg, args);
  va_end (args);

  fprintf (stderr, " on line %d, col %d in file %s\n", compiler->pos.line,
           compiler->pos.col, compiler->pos.fname);
  exit (-1);
}

void
compiler_warning (struct compile_process *compiler, const char *msg, ...)
{
  va_list args;
  va_start (args, msg);
  vfprintf (stderr, msg, args);
  va_end (args);

  fprintf (stderr, " on line %d, col %d in file %s\n", compiler->pos.line,
           compiler->pos.col, compiler->pos.fname);
}

int
compile_file (const char *fname, const char *out_fname, int flags)
{
  struct compile_process *process
      = compile_process_create (fname, out_fname, flags);
  if (!process)
    return COMPILER_FAILED_WITH_ERRORS;

  // Lexical analysis
  struct lex_process *lex_process
      = lex_process_create (process, &compiler_lex_functions, NULL);
  if (!lex_process)
    {
      return COMPILER_FAILED_WITH_ERRORS;
    }

  if (lex (lex_process) != LEXICAL_ANALYSIS_ALL_OK)
    {
      return COMPILER_FAILED_WITH_ERRORS;
    }

  process->token_vec = lex_process->token_vec;

  // TODO: Parsing

  // TODO: Code generation

  return COMPILER_FILE_COMPILED_OK;
}
