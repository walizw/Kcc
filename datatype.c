/*
 * datatype.c - Functions for datatype management.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

_Bool
datatype_is_struct_or_union_for_name (const char *name)
{
  return S_EQ (name, "union") || S_EQ (name, "struct");
}
