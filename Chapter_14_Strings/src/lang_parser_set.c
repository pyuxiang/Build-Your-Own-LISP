#include "lang_parser_set.h"

parser_set_t *create_parser_set(int argc, ...) {
    va_list valist;
    va_start(valist, argc);

    int i;
    parser_set_t *next = NULL;
    for (i = 0; i < argc; i++) {
        parser_set_t *new_node = malloc(sizeof(parser_set_t));
        if (new_node == NULL) {
            printf("Memory failure during node creation!\n");
            exit(1);
        }
        new_node->parser = va_arg(valist, mpc_parser_t*);
        new_node->next = (next == NULL) ? NULL : next; // initialise!
        next = new_node;
    }
    va_end(valist);
    return next; // Last argument must be the LISP itself
}

void clear_parser_set(parser_set_t *parser_set) {
    parser_set_t *next;
    while (parser_set != NULL) {
        next = parser_set->next;
        free(parser_set);
        parser_set = next;
    }
}
