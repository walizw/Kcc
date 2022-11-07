/*
 * compiler.h - Here are all the function, structures and enums definitions -
 * yeah, it's all in one file.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */

#ifndef __COMPILER_H
#define __COMPILE_H

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers/vector.h"

#define S_EQ(str, str2) (str && str2 && (strcmp (str, str2) == 0))

#define NUMERIC_CASE                                                          \
  case '0':                                                                   \
  case '1':                                                                   \
  case '2':                                                                   \
  case '3':                                                                   \
  case '4':                                                                   \
  case '5':                                                                   \
  case '6':                                                                   \
  case '7':                                                                   \
  case '8':                                                                   \
  case '9'

#define OPERATOR_CASE_EXCLUDING_DIVISION                                      \
  case '+':                                                                   \
  case '-':                                                                   \
  case '*':                                                                   \
  case '>':                                                                   \
  case '<':                                                                   \
  case '^':                                                                   \
  case '%':                                                                   \
  case '!':                                                                   \
  case '=':                                                                   \
  case '~':                                                                   \
  case '|':                                                                   \
  case '&':                                                                   \
  case '(':                                                                   \
  case '[':                                                                   \
  case ',':                                                                   \
  case '.':                                                                   \
  case '?'

#define SYMBOL_CASE                                                           \
  case '{':                                                                   \
  case '}':                                                                   \
  case ':':                                                                   \
  case ';':                                                                   \
  case '#':                                                                   \
  case '\\':                                                                  \
  case ')':                                                                   \
  case ']'

struct pos
{
  // store the position we are at
  int line;
  int col;
  const char *fname;
};

enum
{
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_KEYWORD,
  TOKEN_TYPE_OPERATOR,
  TOKEN_TYPE_SYMBOL,
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_COMMENT,
  TOKEN_TYPE_NEWLINE
};

enum
{
  NUMBER_TYPE_NORMAL, // integers
  NUMBER_TYPE_LONG,
  NUMBER_TYPE_FLOAT,
  NUMBER_TYPE_DOUBLE
};

struct token
{
  int type;
  int flags;
  struct pos pos;

  union
  {
    char cval;
    const char *sval;
    unsigned int inum;
    unsigned long lnum;
    unsigned long long llnum;
    void *any;
  };

  struct token_number
  {
    int type;
  } num;

  // is there a whitespace between the token and next token
  // i.e. * a for operator * whitespace would be set for token "a"
  _Bool whitespace;

  // if it's between brackets, points to the opening bracket
  // i.e. (5+10+20) tokens 5, 10 and 20, will point to "("
  const char *between_brackets;
};

struct lex_process;
typedef char (*LEX_PROCESS_NEXT_CHAR) (struct lex_process *process);
typedef char (*LEX_PROCESS_PEEK_CHAR) (struct lex_process *process);
typedef void (*LEX_PROCESS_PUSH_CHAR) (struct lex_process *process, char c);

struct lex_process_functions
{
  LEX_PROCESS_NEXT_CHAR next_char;
  LEX_PROCESS_PEEK_CHAR peek_char;
  LEX_PROCESS_PUSH_CHAR push_char;
};

struct lex_process
{

  struct pos pos;
  struct vector *token_vec;
  struct compile_process *compiler;

  /*
   * number of brackets, in ((60)) for example, it'd be 2
   */
  int current_expression_count;
  struct buffer *parentheses_buffer;
  struct lex_process_functions *function;

  // point to private data that the lexer does not understand, but the person
  // using the lexer does.
  void *private;
};

// this will be used as return codes, if there was an error or if compiling
// went ok
enum
{
  COMPILER_FILE_COMPILED_OK,
  COMPILER_FAILED_WITH_ERRORS
};

enum
{
  LEXICAL_ANALYSIS_ALL_OK,
  LEXICAL_ANALYSIS_INPUT_ERROR
};

struct scope
{
  int flags;

  // a vector of void ptrs
  struct vector *entities;

  // total number of bytes of this scope (aligned to 16 bytes)
  size_t size;

  struct scope *parent;
};

enum
{
  SYMBOL_TYPE_NODE,
  SYMBOL_TYPE_NATIVE_FUNCTION,
  SYMBOL_TYPE_UNKNOWN,
};

struct symbol
{
  const char *name;
  int type;
  void *data;
};

struct compile_process
{
  // this will determine how code must be compiled
  int flags;

  struct pos pos;
  struct compile_process_input_file
  {
    FILE *fp;
    const char *abs_path;
  } cfile;

  // vector of tokens
  struct vector *token_vec;

  struct vector *node_vec;
  struct vector *node_tree_vec; // root of the tree

  FILE *out_file;

  struct
  {
    struct scope *root;
    struct scope *current;
  } scope;

  struct
  {
    // current active symbol table
    struct vector *table;

    // a vector of tables
    struct vector *tables;
  } symbols;
};

enum
{
  DATATYPE_FLAG_IS_SIGNED = 0b00000001,
  DATATYPE_FLAG_IS_STATIC = 0b00000010,
  DATATYPE_FLAG_IS_CONST = 0b00000100,
  DATATYPE_FLAG_IS_POINTER = 0b00001000,
  DATATYPE_FLAG_IS_ARRAY = 0b00010000,
  DATATYPE_FLAG_IS_EXTERN = 0b00100000,
  DATATYPE_FLAG_IS_RESTRICT = 0b01000000,
  DATATYPE_FLAG_IGNORE_TYPE_CHECKING = 0b10000000,
  DATATYPE_FLAG_IS_SECONDARY = 0b100000000,
  DATATYPE_FLAG_STRUCT_UNION_NO_NAME = 0b1000000000,
  DATATYPE_FLAG_IS_LITERAL = 0b10000000000
};

enum
{
  DATA_TYPE_VOID,
  DATA_TYPE_CHAR,
  DATA_TYPE_SHORT,
  DATA_TYPE_INTEGER,
  DATA_TYPE_LONG,
  DATA_TYPE_FLOAT,
  DATA_TYPE_DOUBLE,
  DATA_TYPE_STRUCT,
  DATA_TYPE_UNION,
  DATA_TYPE_UNKNOWN
};

struct datatype
{
  int flags;

  // i.e long, int, etc;
  int type;

  // i.e. long long, struct my_struct
  struct datatype *secondary;

  // i.e "int"
  const char *type_str;

  // the size of the datatype
  size_t size;

  // i.e. int *** would be pointer_depth 3
  int pointer_depth;

  union
  {
    struct node *struct_node;
    struct node *union_node;
  };
};

enum
{
  DATA_TYPE_EXPECT_PRIMITIVE,
  DATA_TYPE_EXPECT_UNION,
  DATA_TYPE_EXPECT_STRUCT
};

enum
{
  DATA_SIZE_ZERO = 0,
  DATA_SIZE_BYTE = 1,
  DATA_SIZE_WORD = 2,
  DATA_SIZE_DWORD = 4,
  DATA_SIZE_DDWORD = 8
};

void compiler_error (struct compile_process *compiler, const char *msg, ...);
void compiler_warning (struct compile_process *compiler, const char *msg, ...);
int compile_file (const char *fname, const char *out_fname, int flags);

// cprocess
struct compile_process *
compile_process_create (const char *fname, const char *out_fname, int flags);

char compile_process_next_char (struct lex_process *lex_process);
char compile_process_peek_char (struct lex_process *lex_process);
void compile_process_push_char (struct lex_process *lex_process, char c);

// parser
enum
{
  PARSE_ALL_OK,
  PARSE_GENERAL_ERROR
};

enum
{
  NODE_TYPE_EXPRESSION,
  NODE_TYPE_EXPRESSION_PARENTHESES,
  NODE_TYPE_NUMBER,
  NODE_TYPE_IDENTIFIER,
  NODE_TYPE_STRING,
  NODE_TYPE_VARIABLE,
  NODE_TYPE_VARIABLE_LIST,
  NODE_TYPE_FUNCTION,
  NODE_TYPE_BODY,
  NODE_TYPE_STATEMENT_RETURN,
  NODE_TYPE_STATEMENT_IF,
  NODE_TYPE_STATEMENT_ELSE,
  NODE_TYPE_STATEMENT_WHILE,
  NODE_TYPE_STATEMENT_DO_WHILE,
  NODE_TYPE_STATEMENT_FOR,
  NODE_TYPE_STATEMENT_BREAK,
  NODE_TYPE_STATEMENT_CONTINUE,
  NODE_TYPE_STATEMENT_SWITCH,
  NODE_TYPE_STATEMENT_CASE,
  NODE_TYPE_STATEMENT_DEFAULT, // in a switch,
  NODE_TYPE_STATEMENT_GOTO,

  NODE_TYPE_UNARY,
  NODE_TYPE_TENARY,
  NODE_TYPE_LABEL,
  NODE_TYPE_STRUCT,
  NODE_TYPE_UNION,
  NODE_TYPE_BRACKET,
  NODE_TYPE_CAST,
  NODE_TYPE_BLANK // ignored
};

enum
{
  NODE_FLAG_INSIDE_EXPRESSION = 0b00000001
};

struct node
{
  int type;
  int flags;

  struct pos pos;

  struct node_binded
  {
    // ptr to body node
    struct node *owner;

    // ptr to parent function
    struct node *function;
  } binded;

  union
  {
    struct exp
    {
      struct node *left;
      struct node *right;
      const char *op;
    } exp;

    struct var
    {
      struct datatype type;
      const char *name;
      struct node *val;
    } var;
  };

  union
  {
    char cval;
    const char *sval;
    unsigned int inum;
    unsigned long lnum;
    unsigned long long llnum;
  };
};

int parse (struct compile_process *process);

// lex_process
struct lex_process *
lex_process_create (struct compile_process *compiler,
                    struct lex_process_functions *functions, void *private);
void lex_process_free (struct lex_process *process);
void *lex_process_private (struct lex_process *process);
struct vector *lex_process_tokens (struct lex_process *process);

// lexer
int lex (struct lex_process *process);

// builds token for `str'
struct lex_process *tokens_build_for_string (struct compile_process *compiler,
                                             const char *str);

// token
_Bool keyword_is_datatype (const char *str);
_Bool token_is_keyword (struct token *token, const char *value);
_Bool token_is_nl_or_comment_or_nl_separator (struct token *token);
_Bool token_is_symbol (struct token *token, char c);
_Bool token_is_operator (struct token *tok, const char *val);
_Bool token_is_primitive_keyword (struct token *token);

// node
void node_set_vector (struct vector *vec, struct vector *root_vec);
void node_push (struct node *node);
struct node *node_peek ();
struct node *node_peek_or_null ();
struct node *node_pop ();
struct node *node_create (struct node *_node);
void make_exp_node (struct node *left_node, struct node *right_node,
                    const char *op);

_Bool node_is_expressionable (struct node *node);
struct node *node_peek_expressionable_or_null ();

// expressionable
#define TOTAL_OPERATOR_GROUPS 14
#define MAX_OPERATORS_IN_GROUP 12

enum
{
  ASSOCIATIVITY_LEFT_TO_RIGHT,
  ASSOCIATIVITY_RIGHT_TO_LEFT
};

struct expressionable_op_precedence_group
{
  char *operators[MAX_OPERATORS_IN_GROUP];
  int associativity; // left to right or right to left
};

// datatype
_Bool datatype_is_struct_or_union_for_name (const char *name);

// scope

#endif
