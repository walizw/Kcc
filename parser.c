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
