#include "lang_set_polish.h"

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
lval polish_eval(mpc_ast_t *node) {

    // All nodes tagged with number is definitely a primitive number
    if (strstr(node->tag, "number")) {
        errno = 0; // reset value to 0, <errno.h> included in error_handler
        long value = strtol(node->contents, NULL, 10); // robust ver of atoi
        return (errno != ERANGE) ? lval_num(value) : lval_err(LERR_BAD_NUM);
    }

    // op guaranteed to be second child,
    // root node has regex input as first child
    char *op = node->children[1]->contents;
    /* Accessing ptr-ptr type via indexing available since type defined */

    lval result;
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




lval polish_eval_op(char *op, lval arg1, lval arg2) {

    // Rethrow errors
    if (arg1.type == LVAL_ERR) { return arg1; }
    if (arg2.type == LVAL_ERR) { return arg2; }

    // Arithmetic
    if (strcmp(op, "+") == 0) { return lval_num(arg1.num + arg2.num); }
    if (strcmp(op, "-") == 0) { return lval_num(arg1.num - arg2.num); }
    if (strcmp(op, "*") == 0) {
        // Exceed long value bounds
        if (abs(arg1.num) > (LONG_MAX/abs(arg2.num))) {
            return lval_err(LERR_BAD_NUM);
        }
        return lval_num(arg1.num * arg2.num);
    }
    if (strcmp(op, "/") == 0) {
        return arg2.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(arg1.num / arg2.num);
    }
    if (strcmp(op, "%") == 0) {
        return arg2.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(arg1.num % arg2.num);
    }
    if (strcmp(op, "^") == 0) {
        // Negative exponents not supported
        if (arg2.num < 0) { return lval_err(LERR_BAD_NUM); }

        long result = 1; // Note 0^0 is defined as 1
        for (; arg2.num > 0; arg2.num--) {
            // Exceed long value bounds
            if (abs(result) > (LONG_MAX/abs(arg1.num))) {
                return lval_err(LERR_BAD_NUM);
            }
            result *= arg1.num;
        }
        return lval_num(result);
    }
    if (strcmp(op, "min") == 0) {
        return lval_num((arg1.num < arg2.num) ? arg1.num : arg2.num);
    }
    if (strcmp(op, "max") == 0) {
        return lval_num((arg1.num > arg2.num) ? arg1.num : arg2.num);
    }
    return lval_err(LERR_BAD_OP); // Op evaluation fail
}




lval polish_eval_op_single(char *op, lval result) {

    // Rethrow error
    if (result.type == LVAL_ERR) { return result; }

    if (strcmp(op, "+") == 0) { return result; }
    if (strcmp(op, "-") == 0) { return lval_num(-1 * result.num); }
    if (strcmp(op, "max") == 0) { return result; }
    if (strcmp(op, "min") == 0) { return result; }
    return lval_err(LERR_BAD_ARGS); // Single arg to op invalid
}
