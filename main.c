/*
 * main.c - Entry point of Kcc.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include <stdio.h>

#include "compiler.h"

int
main (void)
{
  int res = compile_file ("test.c", "test", 0);
  if (res == COMPILER_FILE_COMPILED_OK)
    {
      printf ("Compilation successful!\n");
    }
  else if (res == COMPILER_FAILED_WITH_ERRORS)
    {
      printf ("There's  been an error compiling\n");
    }
  else
    {
      printf ("Unknown response for compilation\n");
    }

  return 0;
}
