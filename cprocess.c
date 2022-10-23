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
  process->flags = flags;
  process->cfile.fp = f;
  process->out_file = outf;
  return process;
}
