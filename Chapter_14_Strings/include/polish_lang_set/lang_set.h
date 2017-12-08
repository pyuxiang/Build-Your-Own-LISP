#ifndef lang_set_h
#define lang_set_h

#include "lang_parser_set.h"

// For loading function
mpc_parser_t *Number;
mpc_parser_t *Symbol;
mpc_parser_t *String;
mpc_parser_t *Comment;
mpc_parser_t *Sexpr;
mpc_parser_t *Qexpr;
mpc_parser_t *Expr;
mpc_parser_t *Lispy;

parser_set_t *polish_notation_set(void);

#endif
