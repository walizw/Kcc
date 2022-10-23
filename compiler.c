/*
 * compiler.c - Compiles a file, passes through all its phases and produces the
 * output.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

struct lex_process_functions compiler_lex_functions
    = { .next_char = compile_process_next_char,
        .peek_char = compile_process_peek_char,
        .push_char = compile_process_push_char };

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
  // TODO: Parsing

  // TODO: Code generation

  return COMPILER_FILE_COMPILED_OK;
}
