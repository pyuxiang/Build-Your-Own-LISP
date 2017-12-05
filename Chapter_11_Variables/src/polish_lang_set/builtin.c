#include "builtin.h"

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
