#include <stdio.h>
#include <stdlib.h>
#include "lang_parser_set.h"
#include "lang_set_polish_error_handler.h"

parser_set_t *polish_notation_set();
lval polish_eval(mpc_ast_t *);
lval polish_eval_op(char *, lval, lval);
lval polish_eval_op_single(char *, lval);
