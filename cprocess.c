/*
 * cprocess.c - Creates and handles the compilation process of an input file.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"

struct compile_process *
compile_process_create (const char *fname, const char *out_fname, int flags)
{
  FILE *f = fopen (fname, "r");
  if (!f)
    {
      return NULL;
    }

  FILE *outf = NULL;

  if (out_fname)
    {
      outf = fopen (out_fname, "w");
      if (!outf)
        {
          return NULL;
        }
    }

  struct compile_process *process
      = calloc (1, sizeof (struct compile_process));

  process->node_vec = vector_create (sizeof (struct node *));
  process->node_tree_vec = vector_create (sizeof (struct node *));

  process->flags = flags;
  process->cfile.fp = f;
  process->out_file = outf;
  return process;
}

char
compile_process_next_char (struct lex_process *lex_process)
{
  struct compile_process *compiler = lex_process->compiler;
  compiler->pos.col += 1;
  char c = getc (compiler->cfile.fp);
  if (c == '\n')
    {
      compiler->pos.line += 1;
      compiler->pos.col = 1;
    }

  return c;
}

char
compile_process_peek_char (struct lex_process *lex_process)
{
  struct compile_process *compiler = lex_process->compiler;
  char c = getc (compiler->cfile.fp);
  ungetc (c, compiler->cfile.fp);
  return c;
}

void
compile_process_push_char (struct lex_process *lex_process, char c)
{
  struct compile_process *compiler = lex_process->compiler;
  ungetc (c, compiler->cfile.fp);
}
