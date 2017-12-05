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
            symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        \
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

lval *lval_func(lbuiltin func) {
    lval *value = malloc(sizeof(lval));
    value->type = LVAL_FUNC;
    value->func = func;
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
        case LVAL_FUNC:
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

lval *lval_copy(lval *value) {
    lval *copy = malloc(sizeof(lval));
    copy->type = value->type;

    switch (value->type) {
        case LVAL_FUNC: copy->func = value->func; break;
        case LVAL_NUM: copy->num = value->num; break;
        case LVAL_ERR:
            copy->err = malloc(strlen(value->err) + 1);
            strcpy(copy->err, value->err);
            break;
        case LVAL_SYM:
            copy->sym = malloc(strlen(value->sym) + 1);
            strcpy(copy->sym, value->sym);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            copy->count = value->count;
            copy->cell = malloc(sizeof(lval *) * copy->count);
            int i;
            for (i = 0; i < copy->count; i++) {
                copy->cell[i] = lval_copy(value->cell[i]);
            }
            break;
    }
    return copy;
}

// This is not required during evaluation... why???
void lval_check_get_replace(lenv *env, lval *src) {
    if (src->type == LVAL_SYM) {
        lval_get_replace(env, src);
    }
}

void lval_get_replace(lenv *env, lval *src) {

    /* Attempt to get env var and replace dest */
    lval *value = lenv_get(env, src);
    lval_free(src);
    switch (value->type) {
        case LVAL_FUNC: src = lval_func(value->func); break;
        case LVAL_NUM: src = lval_num(value->num); break;
        case LVAL_ERR: src = lval_err(value->err); break;
        case LVAL_SYM: src = lval_sym(value->sym); break; // or check again?
        case LVAL_SEXPR:
            src = lval_sexpr();
            src = lval_join(src, value);
            break;
        case LVAL_QEXPR:
            src = lval_qexpr();
            src = lval_join(src, value);
            break;
    }
    lval_free(value);
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
        case LVAL_FUNC: printf("<function>"); break;
        case LVAL_SEXPR: lval_expr_print(value, '(', ')'); break;
        case LVAL_QEXPR: lval_expr_print(value, '{', '}'); break;
    }
}

void lval_println(lval *value) {
    lval_print(value);
    putchar('\n');
}




// lenv constructors and methods
lenv *lenv_new(void) {
    lenv *env = malloc(sizeof(lenv));
    env->count = 0;
    env->syms = NULL;
    env->vals = NULL;
    return env;
}

void lenv_free(lenv *env) {
    int i;
    for (i = 0; i < env->count; i++) {
        free(env->syms[i]);
        lval_free(env->vals[i]);
    }
    free(env->syms);
    free(env->vals);
    free(env);
}

// Get variable from environment
lval *lenv_get(lenv *env, lval *key) {
    int i;
    for (i = 0; i < env->count; i++) {
        if (strcmp(env->syms[i], key->sym) == 0) {
            return lval_copy(env->vals[i]);
        }
    }
    return lval_err("Unbound symbol");
}

void lenv_put(lenv *env, lval *key, lval *value) {
    int i;
    for (i = 0; i < env->count; i++) {
        if (strcmp(env->syms[i], key->sym) == 0) {
            lval_free(env->vals[i]); // Replaces variable name
            env->vals[i] = lval_copy(value);
            return;
        }
    }

    // No existing entry found
    env->count++;
    env->vals = realloc(env->vals, sizeof(lval *) * env->count);
    env->vals[env->count-1] = lval_copy(value);

    env->syms = realloc(env->syms, sizeof(char *) * env->count);
    env->syms[env->count-1] = malloc(strlen(key->sym) + 1);
    strcpy(env->syms[env->count-1], key->sym);
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
lval *lval_eval(lenv *env, lval *value) {
    if (value->type == LVAL_SYM) {
        lval *result = lenv_get(env, value);
        lval_free(value);
        return result;
    }
    if (value->type == LVAL_SEXPR) {
        return lval_eval_sexpr(env, value);
    }
    return value;
}

lval *lval_eval_sexpr(lenv *env, lval *value) {

    // Evaluate children, rethrowing errors if any
    int i;
    for (i = 0; i < value->count; i++) {
        value->cell[i] = lval_eval(env, value->cell[i]);
        // Check if evaluation error
        if (value->cell[i]->type == LVAL_ERR) { return lval_extract(value, i); }
    }

    // Empty expressions return self, single expr extract value
    if (value->count == 0) { return value; }
    if (value->count == 1) { return lval_extract(value, 0); }

    // Multiple element sexpr
    // Check if first element is symbol
    lval *first = lval_pop(value, 0);
    if (first->type != LVAL_FUNC) {
        lval_free(first);
        lval_free(value);
        return lval_err("First element is not a function");
    }

    // Call operator on rest of elements
    lval *result = first->func(env, value);
    lval_free(first);
    return result;
}

void lenv_add_builtin(lenv *env, char *name, lbuiltin func) {
    lval *key = lval_sym(name);
    lval *value = lval_func(func);
    lenv_put(env, key, value);
    lval_free(key);
    lval_free(value);
}

void lenv_add_builtins(lenv *env) {
    lenv_add_builtin(env, "def", builtin_def);

    lenv_add_builtin(env, "list", builtin_list);
    lenv_add_builtin(env, "head", builtin_head);
    lenv_add_builtin(env, "tail", builtin_tail);
    lenv_add_builtin(env, "eval", builtin_eval);
    lenv_add_builtin(env, "join", builtin_join);
    lenv_add_builtin(env, "cons", builtin_cons);
    lenv_add_builtin(env, "len", builtin_len);
    lenv_add_builtin(env, "init", builtin_init);

    lenv_add_builtin(env, "+", builtin_add);
    lenv_add_builtin(env, "-", builtin_sub);
    lenv_add_builtin(env, "*", builtin_mul);
    lenv_add_builtin(env, "/", builtin_div);
    lenv_add_builtin(env, "%", builtin_mod);
    lenv_add_builtin(env, "^", builtin_pow);
    lenv_add_builtin(env, "max", builtin_max);
    lenv_add_builtin(env, "min", builtin_min);
}

// Symbol definition should be done inside qexpr
// Otherwise an attempt to evaluate sexpr will yield
// an unbound symbol error.
lval *builtin_def(lenv *env, lval *args) {
    /* Defines multiple symbols to values */
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'def' passed incorrect type");

    // Check symbol list contains valid symbols
    lval *syms = args->cell[0];
    int i;
    for (i = 0; i < syms->count; i++) {
        LASSERT(args, syms->cell[i]->type == LVAL_SYM,
            "Function 'def' cannot define non-symbol");
    }

    // Check number of symbols matches number of values
    LASSERT(args, syms->count == args->count - 1,
        "Function 'def' takes incorrect number of values");

    // Assignment
    for (i = 0; i < syms->count; i++) {
        lenv_put(env, syms->cell[i], args->cell[i+1]);
    }
    lval_free(args);
    return lval_sexpr(); // success returns ()
}

lval *builtin_add(lenv *env, lval *args) {
    return builtin_op(env, args, "+");
}

lval *builtin_sub(lenv *env, lval *args) {
    return builtin_op(env, args, "-");
}

lval *builtin_mul(lenv *env, lval *args) {
    return builtin_op(env, args, "*");
}

lval *builtin_div(lenv *env, lval *args) {
    return builtin_op(env, args, "/");
}

lval *builtin_mod(lenv *env, lval *args) {
    return builtin_op(env, args, "%");
}

lval *builtin_pow(lenv *env, lval *args) {
    return builtin_op(env, args, "^");
}

lval *builtin_max(lenv *env, lval *args) {
    return builtin_op(env, args, "max");
}

lval *builtin_min(lenv *env, lval *args) {
    return builtin_op(env, args, "min");
}

lval *builtin_head(lenv *env, lval *args) {
    /* Gets only the first element */
    // Only the qexpr itself should be passed, with nonzero elements
    LASSERT(args, args->count == 1,
        "Function 'head' passed too many arguments");
    lval_check_get_replace(env, args->cell[0]);
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'head' passed incorrect type");
    LASSERT(args, args->cell[0]->count != 0,
        "Function 'head' passed empty qexpr");

    lval *value = lval_extract(args, 0);
    // Free all elements except head
    while (value->count > 1) { lval_free(lval_pop(value, 1)); }
    return value;
}

lval *builtin_tail(lenv *env, lval *args) {
    /* Gets all elements other than the first */
    // Only the qexpr itself should be passed, with nonzero elements
    LASSERT(args, args->count == 1,
        "Function 'tail' passed too many arguments");
    lval_check_get_replace(env, args->cell[0]);
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'tail' passed incorrect type");
    LASSERT(args, args->cell[0]->count != 0,
        "Function 'tail' passed empty qexpr");

    lval *list = lval_extract(args, 0);
    // Free only first element
    lval_free(lval_pop(list, 0));
    return list;
}

lval *builtin_list(lenv *env, lval *args) {
    /* Converts sexpr to qexpr */
    lval_check_get_replace(env, args);
    args->type = LVAL_QEXPR;
    return args;
}

lval *builtin_eval(lenv *env, lval *args) {
    /* Converts qexpr to sexpr and evaluates it */
    LASSERT(args, args->count == 1,
        "Function 'eval' passed too many arguments");
    lval_check_get_replace(env, args->cell[0]);
    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'eval' passed incorrect type");

    lval *list = lval_extract(args, 0);
    list->type = LVAL_SEXPR;
    return lval_eval_sexpr(env, list);
}

lval *builtin_join(lenv *env, lval *args) {
    /* Concatenates multiple qexpr */
    int i;
    for (i = 0; i < args->count; i++) {
        lval_check_get_replace(env, args->cell[i]);
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

lval *builtin_cons(lenv *env, lval *args) {
    /* Takes lval and qexpr in args, and appends them */
    LASSERT(args, args->count == 2,
        "Function 'cons' passed wrong number of arguments");

    lval_check_get_replace(env, args->cell[0]);
    lval_check_get_replace(env, args->cell[1]);

    LASSERT(args, args->cell[1]->type == LVAL_QEXPR,
        "Function 'cons' passed incorrect type");

    lval *value = lval_pop(args, 0);
    lval *list = lval_extract(args, 0);
    return lval_insert(list, value, 0);
}

lval *builtin_len(lenv *env, lval *args) {
    /* Take single qexpr in args and return length */
    LASSERT(args, args->count == 1,
        "Function 'len' passed too many arguments");

    lval_check_get_replace(env, args->cell[0]);

    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'len' passed incorrect type");

    long count = args->cell[0]->count;
    lval_free(args);
    return lval_num(count);
}

lval *builtin_init(lenv *env, lval *args) {
    /* Take single qexpr in args and remove last element */

    // Check if args is sym
    LASSERT(args, args->count == 1,
        "Function 'init' passed too many arguments");

    lval_check_get_replace(env, args->cell[0]);

    LASSERT(args, args->cell[0]->type == LVAL_QEXPR,
        "Function 'init' passed incorrect type");

    lval *list = lval_extract(args, 0);
    lval_free(lval_pop(list, list->count - 1));
    return list;
}

lval *builtin_op(lenv *env, lval *args, char *op) {

    // Checks all arguments are numbers
    int i;
    for (i = 0; i < args->count; i++) {
        lval_check_get_replace(env, args->cell[i]);
        if (args->cell[i]->type != LVAL_NUM) {
            lval_free(args);
            return lval_err("Cannot operate on non-number");
        }
    }

    lval *result = lval_pop(args, 0);
    // Unary negation operator
    if ((strcmp(op, "-") == 0) && args->count == 0) {
        result->num *= -1;
    }

    while (args->count > 0) {
        lval *next = lval_pop(args, 0);

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
    lval_free(args);
    return result;
}
