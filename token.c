/*
 * token.c - Includes generic token information.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

_Bool token_is_keyword (struct token *token, const char *value)
{
  return token->type == TOKEN_TYPE_KEYWORD && S_EQ (token->sval, value);
}
