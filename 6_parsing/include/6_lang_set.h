#include "mpc.h"

typedef struct parser_set_t {
    mpc_parser_t *parser;
    struct parser_set_t *next;
} parser_set_t;

parser_set_t *create_parser_set(int, ...);
void clear_parser_set(parser_set_t *);

parser_set_t *polish_notation_set();
parser_set_t *decimal_set();
parser_set_t *doge_set();
