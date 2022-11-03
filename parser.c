/*
 * parser.c - Parses a vector of tokens.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#include "compiler.h"

static struct compile_process *current_process;
static struct token *parser_last_token;

extern struct expressionable_op_precedence_group
    op_precedence[TOTAL_OPERATOR_GROUPS];

struct history
{
  int flags;
};

struct history *
history_begin (int flags)
{
  struct history *history = calloc (1, sizeof (struct history));
  history->flags = flags;
  return history;
}

struct history *
history_down (struct history *history, int flags)
{
  struct history *new_history = calloc (1, sizeof (struct history));
  memcpy (new_history, history, sizeof (struct history));
  new_history->flags = flags; // overwrite flags
  return new_history;
}

int parse_expressionable_single (struct history *history);
void parse_expressionable (struct history *history);

// this will ignore a newline or a comment
static void
parser_ignore_nl_or_comment (struct token *token)
{
  while (token && token_is_nl_or_comment_or_nl_separator (token))
    {
      // skip the token
      vector_peek (current_process->token_vec);
      token = vector_peek_no_increment (current_process->token_vec);
    }
}

static struct token *
token_next ()
{
  struct token *next_token
      = vector_peek_no_increment (current_process->token_vec);
  parser_ignore_nl_or_comment (next_token);
  current_process->pos = next_token->pos;
  parser_last_token = next_token;
  return vector_peek (current_process->token_vec);
}

static struct token *
token_peek_next ()
{
  struct token *next_token
      = vector_peek_no_increment (current_process->token_vec);
  parser_ignore_nl_or_comment (next_token);
  return vector_peek_no_increment (current_process->token_vec);
}

static _Bool
token_next_is_operator (const char *op)
{
  struct token *tok = token_peek_next ();
  return token_is_operator (tok, op);
}

void
parse_single_token_to_node ()
{
  struct token *token = token_next ();
  struct node *node = NULL;

  switch (token->type)
    {
    case TOKEN_TYPE_NUMBER:
      node = node_create (
          &(struct node){ .type = NODE_TYPE_NUMBER, .llnum = token->llnum });
      break;

    case TOKEN_TYPE_IDENTIFIER:
      node = node_create (
          &(struct node){ .type = NODE_TYPE_IDENTIFIER, .sval = token->sval });
      break;

    case TOKEN_TYPE_STRING:
      node = node_create (
          &(struct node){ .type = NODE_TYPE_STRING, .sval = token->sval });
      break;

    default:
      compiler_error (
          current_process,
          "This is not a single token that can be converted to a node");
      break;
    }
}

void
parse_expressionable_for_op (struct history *history, const char *op)
{
  parse_expressionable (history);
}

static int
parser_get_precedence_for_op (
    const char *op, struct expressionable_op_precedence_group **group_out)
{
  *group_out = NULL;
  for (int i = 0; i < TOTAL_OPERATOR_GROUPS; i++)
    {
      for (int j = 0; op_precedence[i].operators[j]; j++)
        {
          const char *_op = op_precedence[i].operators[j];
          if (S_EQ (op, _op))
            {
              *group_out = &op_precedence[i];
              return i;
            }
        }
    }

  return -1;
}

static _Bool
parser_left_op_has_priority (const char *op_left, const char *op_right)
{
  struct expressionable_op_precedence_group *group_left = NULL;
  struct expressionable_op_precedence_group *group_right = NULL;

  if (S_EQ (op_left, op_right))
    {
      // there's no priority, they are equal
      return 0;
    }

  int precedence_left = parser_get_precedence_for_op (op_left, &group_left);
  int precedence_right = parser_get_precedence_for_op (op_right, &group_right);

  if (group_left->associativity == ASSOCIATIVITY_RIGHT_TO_LEFT)
    {
      // we can't handle this here
      return 0;
    }

  return precedence_left <= precedence_right;
}

void
parser_node_shift_children_left (struct node *node)
{
  assert (node->type == NODE_TYPE_EXPRESSION);
  assert (node->exp.right->type == NODE_TYPE_EXPRESSION);

  const char *right_op = node->exp.right->exp.op;
  struct node *new_exp_left_node = node->exp.left;
  struct node *new_exp_right_node = node->exp.right->exp.left;
  make_exp_node (new_exp_left_node, new_exp_right_node, node->exp.op);

  struct node *new_left_operand = node_pop ();
  struct node *new_right_operand = node->exp.right->exp.right;
  node->exp.left = new_left_operand;
  node->exp.right = new_right_operand;
  node->exp.op = right_op;
}

void
parser_reorder_exp (struct node **node_out)
{
  struct node *node = *node_out;
  if (node->type != NODE_TYPE_EXPRESSION)
    return;

  // no expressions, nothing to do
  if (node->exp.left->type != NODE_TYPE_EXPRESSION && node->exp.right
      && node->exp.right->type != NODE_TYPE_EXPRESSION)
    return;

  // 50 + e(50 * 20) for example
  if (node->exp.left->type != NODE_TYPE_EXPRESSION
      && node->exp.right->type == NODE_TYPE_EXPRESSION)
    {
      const char *right_op = node->exp.right->exp.op;
      if (parser_left_op_has_priority (node->exp.op, right_op))
        {
          parser_node_shift_children_left (node);

          // reorder the ast
          parser_reorder_exp (&node->exp.left);
          parser_reorder_exp (&node->exp.right);
        }
    }
}

void
parse_exp_normal (struct history *history)
{
  struct token *op_token = token_peek_next ();
  const char *op = op_token->sval;
  struct node *node_left = node_peek_expressionable_or_null ();
  if (!node_left)
    return;

  // pop off the operator token
  token_next ();

  // pop off the left node
  node_pop ();
  node_left->flags |= NODE_FLAG_INSIDE_EXPRESSION;
  parse_expressionable_for_op (history_down (history, history->flags), op);

  struct node *node_right = node_pop ();
  node_right->flags |= NODE_FLAG_INSIDE_EXPRESSION;

  make_exp_node (node_left, node_right, op);
  struct node *exp_node = node_pop ();

  // reorder the expression
  parser_reorder_exp (&exp_node);

  node_push (exp_node);
}

int
parse_exp (struct history *history)
{
  parse_exp_normal (history); // normal expressions like 50+20

  return 0;
}

void
parse_identifier (struct history *history)
{
  assert (token_peek_next ()->type == NODE_TYPE_IDENTIFIER);
  parse_single_token_to_node ();
}

static _Bool
is_keyword_variable_modifier (const char *val)
{
  return S_EQ (val, "unsigned") || S_EQ (val, "signed") || S_EQ (val, "static")
         || S_EQ (val, "const") || S_EQ (val, "extern")
         || S_EQ (val, "__ignore_typecheck");
}

void
parse_datatype_modifiers (struct datatype *dtype)
{
  struct token *tok = token_peek_next ();
  while (tok && tok->type == TOKEN_TYPE_KEYWORD)
    {
      if (!is_keyword_variable_modifier (tok->sval))
        break;

      if (S_EQ (tok->sval, "signed"))
        {
          dtype->flags |= DATATYPE_FLAG_IS_SIGNED;
        }
      else if (S_EQ (tok->sval, "unsigned"))
        {
          dtype->flags &= ~DATATYPE_FLAG_IS_SIGNED;
        }
      else if (S_EQ (tok->sval, "static"))
        {
          dtype->flags |= DATATYPE_FLAG_IS_STATIC;
        }
      else if (S_EQ (tok->sval, "const"))
        {
          dtype->flags |= DATATYPE_FLAG_IS_CONST;
        }
      else if (S_EQ (tok->sval, "extern"))
        {
          dtype->flags |= DATATYPE_FLAG_IS_EXTERN;
        }
      else if (S_EQ (tok->sval, "__ignore_typecheck"))
        {
          dtype->flags |= DATATYPE_FLAG_IGNORE_TYPE_CHECKING;
        }

      token_next ();
      tok = token_peek_next ();
    }
}

void
parser_get_datatype_tokens (struct token **dtype_tok,
                            struct token **dtype_sec_tok)
{
  *dtype_tok = token_next ();
  struct token *next = token_peek_next ();

  if (token_is_primitive_keyword (next))
    {
      *dtype_sec_tok = next;
      token_next ();
    }
}

int
parser_datatype_expected_for_type_string (const char *str)
{
  int type = DATA_TYPE_EXPECT_PRIMITIVE;

  if (S_EQ (str, "union"))
    {
      type = DATA_TYPE_EXPECT_UNION;
    }
  else if (S_EQ (str, "struct"))
    {
      type = DATA_TYPE_EXPECT_STRUCT;
    }

  return type;
}

int
parser_get_random_type_index ()
{
  // not actually random hehehe
  static int x = 0;
  x++;
  return x;
}

struct token *
parser_build_random_type_name ()
{
  char tmp_name[23] = { 0 };
  sprintf (tmp_name, "customtypename_%d", parser_get_random_type_index ());
  char *sval = malloc (sizeof (tmp_name));
  strncpy (sval, tmp_name, sizeof (tmp_name));

  struct token *tok = calloc (1, sizeof (struct token));
  tok->type = TOKEN_TYPE_IDENTIFIER;
  tok->sval = sval;

  return tok;
}

int
parser_get_pointer_depth ()
{
  int depth = 0;
  while (token_next_is_operator ("*"))
    {
      depth++;
      token_next ();
    }
  return depth;
}

_Bool
parser_datatype_is_secondary_allowed (int expected_type)
{
  return expected_type == DATA_TYPE_EXPECT_PRIMITIVE;
}

_Bool
parser_datatype_is_secondary_allowed_for_type (const char *type)
{
  return S_EQ (type, "long") || S_EQ (type, "short") || S_EQ (type, "double")
         || S_EQ (type, "float");
}

void parser_datatype_init_type_and_size_for_primitive (
    struct token *dtype_token, struct token *dtype_sec_token,
    struct datatype *dtype_out);

void
parser_datatype_adjust_size_for_secondary (struct datatype *dtype,
                                           struct token *dtype_sec_token)
{
  if (!dtype_sec_token)
    return;

  struct datatype *sec_datatype = calloc (1, sizeof (struct datatype));
  parser_datatype_init_type_and_size_for_primitive (dtype_sec_token, NULL,
                                                    sec_datatype);
  dtype->size += sec_datatype->size;
  dtype->secondary = sec_datatype;
  dtype->flags |= DATATYPE_FLAG_IS_SECONDARY;
}

void
parser_datatype_init_type_and_size_for_primitive (
    struct token *dtype_token, struct token *dtype_sec_token,
    struct datatype *dtype_out)
{
  if (!parser_datatype_is_secondary_allowed_for_type (dtype_token->sval)
      && dtype_sec_token)
    {
      compiler_error (current_process,
                      "You are not allowed a secondary "
                      "datatype here for the given datatype `%s'",
                      dtype_token->sval);
    }

  if (S_EQ (dtype_token->sval, "void"))
    {
      dtype_out->type = DATA_TYPE_VOID;
      dtype_out->size = DATA_SIZE_ZERO;
    }
  else if (S_EQ (dtype_token->sval, "char"))
    {
      dtype_out->type = DATA_TYPE_CHAR;
      dtype_out->size = DATA_SIZE_BYTE;
    }
  else if (S_EQ (dtype_token->sval, "short"))
    {
      dtype_out->type = DATA_TYPE_SHORT;
      dtype_out->size = DATA_SIZE_WORD;
    }
  else if (S_EQ (dtype_token->sval, "int"))
    {
      dtype_out->type = DATA_TYPE_INTEGER;
      dtype_out->size = DATA_SIZE_DWORD;
    }
  else if (S_EQ (dtype_token->sval, "long"))
    {
      dtype_out->type = DATA_TYPE_LONG;
      dtype_out->size = DATA_SIZE_DWORD;
    }
  else if (S_EQ (dtype_token->sval, "float"))
    {
      dtype_out->type = DATA_TYPE_FLOAT;
      dtype_out->size = DATA_SIZE_DWORD;
    }
  else if (S_EQ (dtype_token->sval, "double"))
    {
      dtype_out->type = DATA_TYPE_DOUBLE;
      dtype_out->size = DATA_SIZE_DWORD;
    }
  else
    {
      compiler_error (current_process, "Invalid primitive datatype");
    }

  parser_datatype_adjust_size_for_secondary (dtype_out, dtype_sec_token);
}

void
parser_datatype_init_type_and_size (struct token *dtype_token,
                                    struct token *dtype_sec_token,
                                    struct datatype *dtype_out,
                                    int pointer_depth, int expected_type)
{
  if (!parser_datatype_is_secondary_allowed (expected_type) && dtype_sec_token)
    {
      compiler_error (current_process,
                      "You provided an invalid secondary datatype");
    }

  switch (expected_type)
    {
    case DATA_TYPE_EXPECT_PRIMITIVE:
      parser_datatype_init_type_and_size_for_primitive (
          dtype_token, dtype_sec_token, dtype_out);
      break;

    case DATA_TYPE_EXPECT_UNION:
    case DATA_TYPE_EXPECT_STRUCT:
      compiler_error (current_process,
                      "Structures and unions are not yet implemented :'c");
      break;

    default:
      compiler_error (current_process, "Unknown datatype expectation");
      break;
    }
}

void
parser_datatype_init (struct token *dtype_token, struct token *dtype_sec_token,
                      struct datatype *dtype_out, int pointer_depth,
                      int expected_type)
{
  parser_datatype_init_type_and_size (dtype_token, dtype_sec_token, dtype_out,
                                      pointer_depth, expected_type);

  dtype_out->type_str = dtype_token->sval;
  if (S_EQ (dtype_token->sval, "long") && dtype_sec_token
      && S_EQ (dtype_sec_token->sval, "long"))
    {
      compiler_warning (current_process, "Kcc does not support 64 bit longs, "
                                         "your long long will be 32 bits :D");
      dtype_out->size = DATA_SIZE_DWORD;
    }
}

void
parse_datatype_type (struct datatype *dtype)
{
  struct token *dtype_tok = NULL;
  struct token *dtype_sec_tok = NULL; // secondary
  parser_get_datatype_tokens (&dtype_tok, &dtype_sec_tok);

  int expected_type
      = parser_datatype_expected_for_type_string (dtype_tok->sval);

  if (datatype_is_struct_or_union_for_name (dtype_tok->sval))
    {
      if (token_peek_next ()->type == TOKEN_TYPE_IDENTIFIER)
        {
          // change datatype token, i.e. from struct to struct_name
          dtype_tok = token_next ();
        }
      else
        {
          // structure without name
          // i.e.
          // struct { } abc;
          dtype_tok = parser_build_random_type_name ();
          dtype->flags |= DATATYPE_FLAG_STRUCT_UNION_NO_NAME;
        }
    }

  int pointer_depth = parser_get_pointer_depth ();
  parser_datatype_init (dtype_tok, dtype_sec_tok, dtype, pointer_depth,
                        expected_type);
}

void
parse_datatype (struct datatype *dtype)
{
  memset (dtype, 0, sizeof (struct datatype));
  dtype->flags |= DATATYPE_FLAG_IS_SIGNED; // signed by default

  parse_datatype_modifiers (dtype);
  parse_datatype_type (dtype);

  // in case there are modifiers after the type
  // i.e. const char * const
  parse_datatype_modifiers (dtype);
}

void
parse_variable_function_or_struct_union (struct history *history)
{
  struct datatype dtype;
  parse_datatype (&dtype);
}

void
parse_keyword (struct history *history)
{
  struct token *tok = token_peek_next ();

  if (is_keyword_variable_modifier (tok->sval)
      || keyword_is_datatype (tok->sval))
    {
      // parsing a variable, a structure, a function or union
      parse_variable_function_or_struct_union (history);
      return;
    }
}

int
parse_expressionable_single (struct history *history)
{
  struct token *token = token_peek_next ();
  if (!token)
    return -1;

  history->flags |= NODE_FLAG_INSIDE_EXPRESSION;
  int res = -1;

  switch (token->type)
    {
    case TOKEN_TYPE_NUMBER:
      parse_single_token_to_node ();
      res = 0;
      break;

    case TOKEN_TYPE_IDENTIFIER:
      parse_identifier (history);
      res = 0;
      break;

    case TOKEN_TYPE_OPERATOR:
      parse_exp (history);
      res = 0;
      break;

    case TOKEN_TYPE_KEYWORD:
      parse_keyword (history);
      res = 0;
      break;
    }

  return res;
}

void
parse_expressionable (struct history *history)
{
  while (parse_expressionable_single (history) == 0)
    {
    }
}

void
parse_keyword_for_global ()
{
  parse_keyword (history_begin (0));
  struct node *node = node_pop ();
}

int
parse_next ()
{
  struct token *token = token_peek_next ();
  if (!token)
    return -1;

  int res = 0;

  switch (token->type)
    {
    case TOKEN_TYPE_NUMBER:
    case TOKEN_TYPE_IDENTIFIER:
    case TOKEN_TYPE_STRING:
      parse_expressionable (history_begin (0));
      break;

    case TOKEN_TYPE_KEYWORD:
      parse_keyword_for_global ();
      break;
    }

  return res;
}

int
parse (struct compile_process *process)
{
  current_process = process;
  parser_last_token = NULL;

  node_set_vector (process->node_vec, process->node_tree_vec);
  struct node *node = NULL;
  vector_set_peek_pointer (process->token_vec, 0);
  while (parse_next () == 0)
    {
      node = node_peek ();
      vector_push (process->node_tree_vec, &node);
    }

  return PARSE_ALL_OK;
}
