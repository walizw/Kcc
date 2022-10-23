#ifndef __COMPILER_H
#define __COMPILE_H

#include <stdio.h>

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
