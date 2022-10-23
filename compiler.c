/*
 * compiler.c - Compiles a file, passes through all its phases and produces the
 * output.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

int
compile_file (const char *fname, const char *out_fname, int flags)
{
  struct compile_process *process
      = compile_process_create (fname, out_fname, flags);
  if (!process)
    return COMPILER_FAILED_WITH_ERRORS;

  // TODO: Lexical analysis

  // TODO: Parsing

  // TODO: Code generation

  return COMPILER_FILE_COMPILED_OK;
}
