#include "lang_set_polish.h"

parser_set_t *polish_notation_set(void) {

    // Parsers
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    // Why does "%%" nor '%%' work here?
    // Why does '%' work even though it is a flag? Direct str reading?
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                          \
            number : /-?[0-9]+/ ;                  \
            symbol : '+' | '-' | '*' | '/' | '^'   \
                   | '%' | \"min\" | \"max\" ;     \
            sexpr  : '(' <expr>* ')' ;             \
            expr : <number> | <symbol> | <sexpr> ; \
            lispy : /^/ <expr>* /$/ ;              \
        ",
        Number, Symbol, Sexpr, Expr, Lispy);

    return create_parser_set(5, Number, Symbol, Sexpr, Expr, Lispy);
}

// Num lval constructor
lval *lval_num(long result) {
    lval *value = malloc(sizeof(lval));
    value->type = LVAL_NUM;
    value->num = result;
    return value;
}

// Error lval constructor
lval *lval_err(char *error_msg) {
    lval *value = malloc(sizeof(lval));
    value->type = LVAL_ERR;
    value->err = malloc(strlen(error_msg) + 1);
    strcpy(value->err, error_msg);
    return value;
}

// Symbol lval constructor
lval *lval_sym(char *symbol_str) {
    lval *value = malloc(sizeof(lval));
    value->type = LVAL_SYM;
    value->sym = malloc(strlen(symbol_str) + 1);
    strcpy(value->sym, symbol_str);
    return value;
}

// Sexpr lval constructor
lval *lval_sexpr(void) {
    lval *value = malloc(sizeof(lval));
    value->type = LVAL_SEXPR;
    value->count = 0;
    value->cell = NULL;
    return value;
}

// Append to sexpr list
lval *lval_add(lval *list, lval *node) {
    list->count++;
    list->cell = realloc(list->cell, (list->count)*sizeof(lval *));
    list->cell[list->count-1] = node;
    return list;
}

// Free memory allocated to lval value based on type
void lval_free(lval *value) {
    switch (value->type) {
        case LVAL_NUM: break;
        case LVAL_ERR: free(value->err); break;
        case LVAL_SYM: free(value->sym); break;
        case LVAL_SEXPR: {
            int i;
            for (i = 0; i < value->count; i++) {
                lval_free(value->cell[i]);
            }
            free(value->cell);
            break;
        }
    }
    free(value);
}

lval *lval_pop(lval *value, int index) {
    lval *result = value->cell[index];
    memmove(&(value->cell[index]), &(value->cell[index+1]),
        sizeof(lval *)*(value->count-index-1)); // pop value->cell[index]
    value->count--;
    value->cell = realloc(value->cell, sizeof(lval *)*(value->count));
    return result;
}

lval *lval_extract(lval *value, int index) {
    lval *result = lval_pop(value, index);
    lval_free(value);
    return result;
}

void lval_print(lval *value) {
    switch (value->type) {
        case LVAL_NUM: printf("%li", value->num); break;
        case LVAL_ERR: printf("Error: %s", value->err); break;
        case LVAL_SYM: printf("%s", value->sym); break;
        case LVAL_SEXPR: {
            putchar('(');
            int i;
            for (i = 0; i < value->count; i++) {
                lval_print(value->cell[i]);
                if (i != (value->count - 1)) putchar(' ');
            }
            putchar(')');
            break;
        }
    }
}

void lval_println(lval *value) {
    lval_print(value);
    putchar('\n');
}

// ast == Abstract Syntax Tree
// r.output is a mpc_val_t*,
// so why is passing argument as a mpc_ast_t* allowed?
// If it is type promotion, how is it implemented?
lval *lval_read(mpc_ast_t *node) {

    // All nodes tagged with number / symbol is definitely a primitive number
    if (strstr(node->tag, "number")) {
        errno = 0; // reset value to 0, <errno.h> included in error_handler
        long value = strtol(node->contents, NULL, 10); // robust ver of atoi
        return (errno != ERANGE) ? lval_num(value) : lval_err("Invalid number");
    }
    if (strstr(node->tag, "symbol")) {
        return lval_sym(node->contents);
    }

    // Create empty list if root (>) or sexpr
    lval *list = NULL;
    if (strstr(node->tag, "sexpr")||(strcmp(node->tag, ">") == 0)) {
        list = lval_sexpr();
    }

    // Ignore invalid expressions (parentheses, regex start, end)
    int i;
    for (i = 0; i < node->children_num; i++) {
        if (strcmp(node->children[i]->contents, "(") == 0) continue;
        if (strcmp(node->children[i]->contents, ")") == 0) continue;
        if (strcmp(node->children[i]->tag, "regex") == 0) continue;
        list = lval_add(list, lval_read(node->children[i]));
    }
    return list;
}

lval *lval_eval(lval *value) {
    if (value->type != LVAL_SEXPR) return value;

    // Evaluate children, rethrowing errors if any
    int i;
    for (i = 0; i < value->count; i++) {
        if (value->cell[i]->type == LVAL_ERR) { return lval_extract(value, i); }
        value->cell[i] = lval_eval(value->cell[i]);
    }

    // Empty expressions return self, single expr extract value
    if (value->count == 0) { return value; }
    if (value->count == 1) { return lval_extract(value, 0); }

    // Multiple element sexpr
    // Check if first element is symbol
    lval *first = lval_pop(value, 0);
    if (first->type != LVAL_SYM) {
        lval_free(first);
        lval_free(value);
        return lval_err("S-expression does not start with symbol");
    }

    // Call operator on rest of elements
    lval *result = eval_op(first->sym, value); // value freed in op_eval
    lval_free(first);
    return result;
}

lval *eval_op(char *op, lval *list) {

    // Checks all arguments are numbers
    int i;
    for (i = 0; i < list->count; i++) {
        if (list->cell[i]->type != LVAL_NUM) {
            lval_free(list);
            return lval_err("Cannot operate on non-number");
        }
    }

    lval *result = lval_pop(list, 0);
    // Unary negation operator
    if ((strcmp(op, "-") == 0) && list->count == 0) {
        result->num *= -1;
    }

    while (list->count > 0) {
        lval *next = lval_pop(list, 0);

        // num can only be up to LONG_MAX
        if (strcmp(op, "+") == 0) {
            if (((result->num > 0)&&(next->num > 0)) || \
                    ((result->num < 0)&&(next->num < 0))) {
                if ((LONG_MAX - abs(result->num)) < abs(next->num)) {
                    lval_free(result); // result freed before creating err lval
                    lval_free(next); // next must be freed from same scope
                    result = lval_err("Integer overflow");
                    break;
                }
            }
            result->num += next->num;
        }
        if (strcmp(op, "-") == 0) {
            if (((result->num > 0)&&(next->num < 0)) || \
                    ((result->num < 0)&&(next->num > 0))) {
                if ((LONG_MAX - abs(result->num)) < abs(next->num)) {
                    lval_free(result);
                    lval_free(next);
                    result = lval_err("Integer overflow");
                    break;
                }
            }
            result->num -= next->num;
        }
        if (strcmp(op, "*") == 0) {
            if (abs(result->num) > (LONG_MAX/abs(next->num))) {
                lval_free(result);
                lval_free(next);
                result = lval_err("Integer overflow");
                break;
            }
            result->num *= next->num;
        }
        if (strcmp(op, "/") == 0) {
            if (next->num == 0) {
                lval_free(result);
                lval_free(next);
                result = lval_err("Division by zero");
                break;
            }
            result->num /= next->num;
        }

        if (strcmp(op, "%") == 0) {
            if (next->num == 0) {
                lval_free(result);
                lval_free(next);
                result = lval_err("Division by zero");
                break;
            }
            result->num %= next->num;
        }
        if (strcmp(op, "^") == 0) {
            if (next->num < 0) {
                lval_free(result);
                lval_free(next);
                result = lval_err("Negative exponents not supported");
                break;
            }
            long exp_result = 1; // Note 0^0 is defined as 1
            for (; next->num > 0; next->num--) {
                if (abs(exp_result) > (LONG_MAX/abs(result->num))) {
                    lval_free(result);
                    lval_free(next);
                    result = lval_err("Integer overflow");
                    break;
                }
                exp_result *= result->num;
            }
            result->num = exp_result;
        }

        if (strcmp(op, "min") == 0) {
            result->num = (result->num < next->num)
                ? result->num : next->num;
        }
        if (strcmp(op, "max") == 0) {
            result->num = (result->num > next->num)
                ? result->num : next->num;
        }
        lval_free(next);
    }
    lval_free(list);
    return result;
}
