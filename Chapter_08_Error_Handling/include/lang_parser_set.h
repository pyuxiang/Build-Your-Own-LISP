#include "mpc.h"
#include <string.h>
#include <stdlib.h>

typedef struct parser_set_t {
    mpc_parser_t *parser;
    struct parser_set_t *next;
} parser_set_t;

parser_set_t *create_parser_set(int, ...);
void clear_parser_set(parser_set_t *);
