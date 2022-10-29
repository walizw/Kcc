/*
 * token.c - Includes generic token information.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

_Bool
token_is_keyword (struct token *token, const char *value)
{
  return token->type == TOKEN_TYPE_KEYWORD && S_EQ (token->sval, value);
}

_Bool
token_is_symbol (struct token *token, char c)
{
  return token->type == TOKEN_TYPE_SYMBOL && token->cval == c;
}

_Bool
token_is_nl_or_comment_or_nl_separator (struct token *token)
{
  return token->type == TOKEN_TYPE_NEWLINE || token->type == TOKEN_TYPE_COMMENT
         || token_is_symbol (token, '\\');
}
