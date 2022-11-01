/*
 * expressionable.c - Manages expressions (such as precedence reordering)
 *
 * Copyright (C) 2022 walizw <yojan.bustamante@udea.edu.co>
 */
#include "compiler.h"

// format: {op1, op2, op3, ..., NULL}
struct expressionable_op_precedence_group op_precedence[TOTAL_OPERATOR_GROUPS]
    = { { .operators = { "++", "--", "()", "[]", "(", "[", ".", "->", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "*", "/", "%", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "+", "-", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "<<", ">>", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "<", "<=", ">", ">=", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "==", "!=", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "&", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "^", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "|", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "&&", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "||", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT },
        { .operators = { "?", ":", NULL },
          .associativity = ASSOCIATIVITY_RIGHT_TO_LEFT },
        { .operators = { "=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=",
                         "^=", "|=", NULL },
          .associativity = ASSOCIATIVITY_RIGHT_TO_LEFT },
        { .operators = { ",", NULL },
          .associativity = ASSOCIATIVITY_LEFT_TO_RIGHT } };
