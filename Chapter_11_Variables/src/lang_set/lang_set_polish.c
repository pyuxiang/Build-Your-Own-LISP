#include "lang_set_polish.h"

// Approach to add new features to language:
// 1. Syntax: Add new rule to grammar
// 2. Representation: Add new data type variation
// 3. Parsing: Add new functions to read feature
// 4. Semantics: Add new functions to evaluate and manipulate feature

// Q-expressions similar to Lisp macros to stop evaluation.
// Arguments are evaluated via different set of rules
parser_set_t *polish_notation_set(void) {

    // Parsers
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    // Why does "%%" nor '%%' work here?
    // Why does '%' work even though it is a flag? Direct str reading?
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                      \
            number : /-?[0-9]+/ ;                              \
            symbol : \"list\" | \"head\" | \"tail\" | \"join\" \
                   | \"eval\" | \"cons\" | \"len\" | \"init\"  \
                   | '+' | '-' | '*' | '/' | '^'               \
                   | '%' | \"min\" | \"max\" ;                 \
            sexpr  : '(' <expr>* ')' ;                         \
            qexpr  : '{' <expr>* '}' ;                         \
            expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
            lispy  : /^/ <expr>* /$/ ;                         \
        ",
        Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return create_parser_set(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
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

// Qexpr lval constructor
lval *lval_qexpr(void) {
    lval *value = malloc(sizeof(lval));
    value->type = LVAL_QEXPR;
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
        case LVAL_QEXPR:
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

lval *lval_pop(lval *list, int index) {
    lval *result = list->cell[index];
    // Protects agianst segfault
    if (list->count != index + 1) {
        memmove(&(list->cell[index]), &(list->cell[index+1]),
            sizeof(lval *)*(list->count-index-1)); // pop value->cell[index]
    }
    list->count--;
    list->cell = realloc(list->cell, sizeof(lval *)*(list->count));
    return result;
}

// General form of lval_add
// Will this cause segfault if list->count == index?
lval *lval_insert(lval *list, lval *value, int index) {
    list->cell = realloc(list->cell, sizeof(lval *)*(list->count + 1));
    // Protects against segfault when appending
    if (list->count != index){
        memmove(&(list->cell[index+1]), &(list->cell[index]),
            sizeof(lval *)*(list->count-index));
    }
    list->count++;
    list->cell[index] = value;
    lval_free(value);
    return list;
}

lval *lval_extract(lval *list, int index) {
    lval *result = lval_pop(list, index);
    lval_free(list);
    return result;
}

void lval_expr_print(lval *expr, char open, char close) {
    putchar(open);
    int i;
    for (i = 0; i < expr->count; i++) {
        lval_print(expr->cell[i]);
        if (i != (expr->count -1)) putchar(' ');
    }
    putchar(close);
}

void lval_print(lval *value) {
    switch (value->type) {
        case LVAL_NUM: printf("%li", value->num); break;
        case LVAL_ERR: printf("Error: %s", value->err); break;
        case LVAL_SYM: printf("%s", value->sym); break;
        case LVAL_SEXPR: lval_expr_print(value, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(value, '{', '}'); break;
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

    // Create empty sexpr list if root (>) or sexpr
    lval *list = NULL;
    if (strstr(node->tag, "sexpr")||(strcmp(node->tag, ">") == 0)) {
        list = lval_sexpr();
    }
    // Create empty qexpr list if qexpr
    if (strstr(node->tag, "qexpr")) {
        list = lval_qexpr();
    }

    // Ignore invalid expressions (parentheses, regex start, end)
    int i;
    for (i = 0; i < node->children_num; i++) {
        if (strcmp(node->children[i]->contents, "(") == 0) continue;
        if (strcmp(node->children[i]->contents, ")") == 0) continue;
        if (strcmp(node->children[i]->contents, "{") == 0) continue;
        if (strcmp(node->children[i]->contents, "}") == 0) continue;
        if (strcmp(node->children[i]->tag, "regex") == 0) continue;
        list = lval_add(list, lval_read(node->children[i]));
    }
    return list;
}







// Evaluation
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
    lval *result = builtin(value, first->sym); // value freed in builtin
    lval_free(first);
    return result;
}

lval *builtin(lval *args, char *sym) {
    if (strcmp("list", sym) == 0) { return builtin_list(args); }
    if (strcmp("head", sym) == 0) { return builtin_head(args); }
    if (strcmp("tail", sym) == 0) { return builtin_tail(args); }
    if (strcmp("eval", sym) == 0) { return builtin_eval(args); }
    if (strcmp("join", sym) == 0) { return builtin_join(args); }
    if (strcmp("cons", sym) == 0) { return builtin_cons(args); }
    if (strcmp("len", sym) == 0) { return builtin_len(args); }
    if (strcmp("init", sym) == 0) { return builtin_init(args); }
    if (strstr("+-/*%^ min max", sym)) { return builtin_op(args, sym); }
    lval_free(args);
    return lval_err("Unknown function");
}











lval *builtin_head(lval *args) {
    /* Gets only the first element */
    // Only the qexpr itself should be passed, with nonzero elements
    LASSERT(args, args->count == 1,
        "Function 'head' passed too many arguments");
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'head' passed incorrect type");
    LASSERT(args, args->cell[0]->count != 0,
        "Function 'head' passed empty qexpr");

    lval *value = lval_extract(args, 0);
    // Free all elements except head
    while (value->count > 1) { lval_free(lval_pop(value, 1)); }
    return value;
}

lval *builtin_tail(lval *args) {
    /* Gets all elements other than the first */
    // Only the qexpr itself should be passed, with nonzero elements
    LASSERT(args, args->count == 1,
        "Function 'tail' passed too many arguments");
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'tail' passed incorrect type");
    LASSERT(args, args->cell[0]->count != 0,
        "Function 'tail' passed empty qexpr");

    lval *list = lval_extract(args, 0);
    // Free only first element
    lval_free(lval_pop(list, 0));
    return list;
}

lval *builtin_list(lval *args) {
    /* Converts sexpr to qexpr */
    args->type = LVAL_QEXPR;
    return args;
}

lval *builtin_eval(lval *args) {
    /* Converts qexpr to sexpr and evaluates it */
    LASSERT(args, args->count == 1,
        "Function 'eval' passed too many arguments");
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'eval' passed incorrect type");

    lval *list = lval_extract(args, 0);
    list->type = LVAL_SEXPR;
    return lval_eval(list);
}

lval *builtin_join(lval *args) {
    /* Concatenates multiple qexpr */
    int i;
    for (i = 0; i < args->count; i++) {
        LASSERT(args, args->cell[i]->type == LVAL_QEXPR,
            "Function 'join' passed incorrect type");
    }

    // Individual qexpr concatenation
    lval *list = lval_pop(args, 0);
    while (args->count) {
        list = lval_join(list, lval_pop(args, 0));
    }
    lval_free(args);
    return list;
}

lval *lval_join(lval *list, lval *next) {
    /* Concatenates two qexpr */
    // Note qexpr and sexpr share same list attribute, i.e. lval_add
    while (next->count) {
        list = lval_add(list, lval_pop(next, 0));
    }
    lval_free(next);
    return list;
}

lval *builtin_cons(lval *args) {
    /* Takes lval and qexpr in args, and appends them */
    LASSERT(args, args->count == 2,
        "Function 'cons' passed wrong number of arguments");
    LASSERT(args, args->cell[1]->type == LVAL_QEXPR,
        "Function 'cons' passed incorrect type");

    lval *value = lval_pop(args, 0);
    lval *list = lval_extract(args, 0);
    return lval_insert(list, value, 0);
}

lval *builtin_len(lval *args) {
    /* Take single qexpr in args and return length */
    LASSERT(args, args->count == 1,
        "Function 'len' passed too many arguments");
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'len' passed incorrect type");

    long count = args->cell[0]->count;
    lval_free(args);
    return lval_num(count);
}

lval *builtin_init(lval *args) {
    /* Take single qexpr in args and remove last element */
    LASSERT(args, args->count == 1,
        "Function 'init' passed too many arguments");
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'init' passed incorrect type");

    lval *list = lval_extract(args, 0);
    lval_free(lval_pop(list, list->count - 1));
    return list;
}

lval *builtin_op(lval *list, char *op) {

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
            if (next->num != 0) {
                if (abs(result->num) > (LONG_MAX/abs(next->num))) {
                    lval_free(result);
                    lval_free(next);
                    result = lval_err("Integer overflow");
                    break;
                }
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
            if (result->num == 0) {
                if (next->num != 0) {
                    exp_result = 0; // ^ 0 1 evaluates to 0, not 1
                }
            } else {
                for (; next->num > 0; next->num--) {
                    if (abs(exp_result) > (LONG_MAX/abs(result->num))) {
                        lval_free(result);
                        lval_free(next);
                        result = lval_err("Integer overflow");
                        break;
                    }
                    exp_result *= result->num;
                }
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
