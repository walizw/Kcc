/*
 * symres.c - Symbol table.
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */
#include "compiler.h"

static void
symres_push_symbol (struct compile_process *process, struct symbol *sym)
{
  vector_push (process->symbols.table, &sym);
}

void
symres_init (struct compile_process *process)
{
  process->symbols.tables = vector_create (sizeof (struct vector *));
}

void
symres_new_table (struct compile_process *process)
{
  // save the current table
  vector_push (process->symbols.tables, &process->symbols.table);

  // overwrite the active table
  process->symbols.table = vector_create (sizeof (struct symbol *));
}

void
symres_end_table (struct compile_process *process)
{
  struct vector *last_table = vector_back_ptr (process->symbols.tables);
  process->symbols.table = last_table;
  vector_pop (process->symbols.tables);
}

struct symbol *
symres_get_symbol (struct compile_process *process, const char *name)
{
  vector_set_peek_pointer (process->symbols.table, 0);
  struct symbol *sym = vector_peek_ptr (process->symbols.table);
  while (sym)
    {
      if (S_EQ (sym->name, name))
        {
          break;
        }

      sym = vector_peek_ptr (process->symbols.table);
    }

  return sym;
}

struct symbol *
symres_get_symbol_fo_native_function (struct compile_process *process,
                                      const char *name)
{
  struct symbol *sym = symres_get_symbol (process, name);
  if (!sym)
    {
      return NULL;
    }

  if (sym->type != SYMBOL_TYPE_NATIVE_FUNCTION)
    return NULL;

  return sym;
}

struct symbol *
symres_register_symbol (struct compile_process *process, const char *sym_name,
                        int type, void *data)
{
  if (symres_get_symbol (process, sym_name))
    return NULL;

  struct symbol *sym = calloc (1, sizeof (struct symbol));
  sym->name = sym_name;
  sym->type = type;
  sym->data = data;
  symres_push_symbol (process, sym);

  return sym;
}

struct node *
sumres_node (struct symbol *sym)
{
  if (sym->type != SYMBOL_TYPE_NODE)
    return NULL;

  return sym->data;
}

void
symres_build_for_variable_node (struct compile_process *process,
                                struct node *node)
{
  compiler_error (process, "Variables are not supported yet :'c");
}

void
symres_build_for_function_node (struct compile_process *process,
                                struct node *node)
{
  compiler_error (process, "Functions are not supported yet :'c");
}

void
symres_build_for_structure_node (struct compile_process *process,
                                 struct node *node)
{
  compiler_error (process, "Structures are not supported yet :'c");
}

void
symres_build_for_union_node (struct compile_process *process,
                             struct node *node)
{
  compiler_error (process, "Unions are not supported yet :'c");
}

void
symres_build_for_node (struct compile_process *process, struct node *node)
{
  switch (node->type)
    {
    case NODE_TYPE_VARIABLE:
      symres_build_for_variable_node (process, node);
      break;

    case NODE_TYPE_FUNCTION:
      symres_build_for_function_node (process, node);
      break;

    case NODE_TYPE_STRUCT:
      symres_build_for_structure_node (process, node);
      break;

    case NODE_TYPE_UNION:
      symres_build_for_union_node (process, node);
      break;

      // ignore other node types
    }
}
