#include "language_set.h"
#include <string.h>
#include <stdlib.h>

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

parser_set_t *polish_notation_set(void) {

    // Parsers
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    // Why does "%%" nor '%%' work here?
    // Why does '%' work even though it is a flag? Direct str reading?
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                  \
            number : /-?[0-9]+/ ;                          \
            operator : '+' | '-' | '*' | '/' | '^'         \
                     | '%' | \"min\" | \"max\" ;           \
            expr : <number> | '(' <operator> <expr>+ ')' ; \
            lispy : /^/ <operator> <expr>+ /$/ ;           \
        ",
        Number, Operator, Expr, Lispy);

    return create_parser_set(4, Number, Operator, Expr, Lispy);
}


// ast == Abstract Syntax Tree
// r.output is a mpc_val_t*,
// so why is passing argument as a mpc_ast_t* allowed?
// If it is type promotion, how is it implemented?

// Refer to mpc.h for mpc_ast_t defintion
// Treating all nodes as expressions
long polish_eval(mpc_ast_t *node) {

    // All nodes tagged with number is definitely a primitive number
    if (strstr(node->tag, "number")) {
        return atoi(node->contents);
    }

    // op guaranteed to be second child,
    // root node has regex input as first child
    char *op = node->children[1]->contents;
    /* Accessing ptr-ptr type via indexing available since type defined */

    long result;
    int i;
    // Alternatively, while(strstr(t->children[i]->tag, "expr"))
    // Left-right precedence of operations
    for (i = 2; i < node->children_num - 1; i++) {
        if (i == 2) {
            result = polish_eval(node->children[i]);
        } else {
            result = polish_eval_op(op, result, polish_eval(node->children[i]));
        }
    }

    // Single argument to operator
    if (i == 3) {
        result = polish_eval_op_single(op, result);
    }
    return result;
}

long polish_eval_op(char *op, long arg1, long arg2) {
    if (strcmp(op, "+") == 0) { return arg1 + arg2; }
    if (strcmp(op, "-") == 0) { return arg1 - arg2; }
    if (strcmp(op, "*") == 0) { return arg1 * arg2; }
    if (strcmp(op, "/") == 0) { return arg1 / arg2; }
    if (strcmp(op, "%") == 0) { return arg1 % arg2; }
    if (strcmp(op, "^") == 0) {
        long result = 1;
        for (; arg2 > 0; arg2--) {
            result *= arg1;
        }
        return result;
    }
    if (strcmp(op, "min") == 0) { return (arg1 < arg2) ? arg1 : arg2; }
    if (strcmp(op, "max") == 0) { return (arg1 > arg2) ? arg1 : arg2; }
    return 0; // Evaluation fail, should be captured during error handling
}

long polish_eval_op_single(char *op, long result) {
    if (strcmp(op, "-") == 0) { return -1 * result; }
    return result; // Single arg to op does not exist
}

/* Decimals
 * digit   : <nonzero> | <zero>;
 * integer : <zero> | (<nonzero> <digit>*);
 * decimal : <integer> '.' <digit>* <nonzero>;
 */

parser_set_t *decimal_set(void) {

    mpc_parser_t *Integer = mpc_new("integer");
    mpc_parser_t *Decimal = mpc_new("decimal");
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Lisp = mpc_new("lisp");

    // Put more specific rules in front (left-right precedence)
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                      \
            integer : /-?[1-9][0-9]*/ | /0/ ;  \
            decimal : <integer> '.' /[0-9]+/ ; \
            number: <decimal> | <integer> ;    \
            lisp : /^/ <number> /$/ ;          \
        ",
        Integer, Decimal, Number, Lisp);

    return create_parser_set(4, Integer, Decimal, Number, Lisp);
}


// DOGE LANGUAGE
/* noun      : "LISP" | "language" | "book" | "build" | "c" ;
 * adjective : "wow" | "many" | "so" | "such";
 * phrase    : <adjective> <noun>;
 * doge      : <phrase>*;
 */

parser_set_t *doge_set(void) {

    mpc_parser_t *Noun = mpc_new("noun");
    mpc_parser_t *Adjective = mpc_new("adjective");
    mpc_parser_t *Phrase = mpc_new("phrase");
    mpc_parser_t *Doge = mpc_new("doge");

    // Put more specific rules in front (left-right precedence)
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                                   \
            noun : \"LISP\" | \"language\" | \"book\" | \"build\" | \"c\";  \
            adjective : \"wow\" | \"many\" | \"so\" | \"such\" ;            \
            phrase: <adjective> <noun> ;                                    \
            doge : /^/ <phrase>* /$/ ;                                      \
        ",
        Noun, Adjective, Phrase, Doge);

    return create_parser_set(4, Noun, Adjective, Phrase, Doge);
}
