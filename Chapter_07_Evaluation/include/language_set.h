#include "mpc.h"

typedef struct parser_set_t {
    mpc_parser_t *parser;
    struct parser_set_t *next;
} parser_set_t;

parser_set_t *create_parser_set(int, ...);
void clear_parser_set(parser_set_t *);

parser_set_t *polish_notation_set();
long polish_eval(mpc_ast_t *);
long polish_eval_op(char *, long, long);
long polish_eval_op_single(char *, long);

parser_set_t *decimal_set();
parser_set_t *doge_set();
