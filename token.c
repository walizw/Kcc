/*
 * token.c - Includes generic token information.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

#define PRIMITIVE_TYPES_TOTAL 7
const char *primitive_types[PRIMITIVE_TYPES_TOTAL]
    = { "void", "char", "short", "int", "long", "float", "double" };

_Bool
token_is_keyword (struct token *token, const char *value)
{
  return token && token->type == TOKEN_TYPE_KEYWORD
         && S_EQ (token->sval, value);
}

_Bool
token_is_symbol (struct token *token, char c)
{
  return token && token->type == TOKEN_TYPE_SYMBOL && token->cval == c;
}

_Bool
token_is_operator (struct token *tok, const char *val)
{
  return tok && tok->type == TOKEN_TYPE_OPERATOR && S_EQ (tok->sval, val);
}

_Bool
token_is_nl_or_comment_or_nl_separator (struct token *token)
{
  if (!token)
    return 0;

  return token->type == TOKEN_TYPE_NEWLINE || token->type == TOKEN_TYPE_COMMENT
         || token_is_symbol (token, '\\');
}

_Bool
token_is_primitive_keyword (struct token *token)
{
  if (!token)
    return 0;

  if (token->type != TOKEN_TYPE_KEYWORD)
    return 0;

  for (int i = 0; i < PRIMITIVE_TYPES_TOTAL; i++)
    {
      if (S_EQ (primitive_types[i], token->sval))
        return 1;
    }

  return 0;
}
