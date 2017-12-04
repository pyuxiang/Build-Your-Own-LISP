#include "lang_parser_set.h"

parser_set_t *polish_notation_set();
long polish_eval(mpc_ast_t *);
long polish_eval_op(char *, long, long);
long polish_eval_op_single(char *, long);
