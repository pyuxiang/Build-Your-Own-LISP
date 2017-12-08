#ifndef builtin_h
#define builtin_h

#include "lang_set.h" // For Parser to load
#include "lval_lenv.h"

lval *builtin_op(lenv *, lval *, char *);
void lenv_add_builtins(lenv *);

// Numeric constants
// Block statement to avoid redefinition error during simultaneous defn
#define LENV_DEF_CONST(env, name, input, constructor) { \
    lval *key = lval_sym(name); \
    lval *value = constructor(input); \
    lenv_put(env, key, value); \
    lval_free(key); \
    lval_free(value); \
}

// ## removes leading comma when no args passed
#define LASSERT(args, cond, format, ...) \
    if (!(cond)) { \
        lval *err = lval_err(format, ##__VA_ARGS__); \
        lval_free(args); \
        return err; \
    }

#define LASSERT_TYPE(args, func, index, expected) \
    LASSERT(args, args->cell[index]->type == expected, \
        "Function '%s' passed incorrect type at argument %d. Expected %s instead of %s.", \
        func, index, lval_type_name(expected), lval_type_name(args->cell[index]->type));

#define LASSERT_NUM(args, func, num) \
    LASSERT(args, args->count == num, \
        "Function '%s' took incorrect number of arguments. Expected %d instead of %d.", \
        func, num, args->count);

#define LASSERT_NOT_EMPTY(args, func, index) \
    LASSERT(args, args->cell[index]->count != 0, \
        "Function '%s' passed empty {} at argument %d.", func, index);

lval *builtin_load(lenv *, lval *);
lval *builtin_print(lenv *, lval *);
lval *builtin_error(lenv *, lval *);

lval *builtin_compare_bool(lenv *, lval *, char *);
lval *builtin_or(lenv *, lval *);
lval *builtin_and(lenv *, lval *);
lval *builtin_eq(lenv *, lval *);
lval *builtin_neq(lenv *, lval *);
lval *builtin_compare_num(lenv *, lval *, char *);
lval *builtin_greater(lenv *, lval *);
lval *builtin_greater_eq(lenv *, lval *);
lval *builtin_lesser(lenv *, lval *);
lval *builtin_lesser_eq(lenv *, lval *);
lval *builtin_bool(lenv *, lval *);
lval *builtin_negate(lenv *, lval *);
lval *builtin_if(lenv *, lval *);

lval *builtin_add(lenv *, lval *);
lval *builtin_sub(lenv *, lval *);
lval *builtin_mul(lenv *, lval *);
lval *builtin_div(lenv *, lval *);
lval *builtin_mod(lenv *, lval *);
lval *builtin_pow(lenv *, lval *);
lval *builtin_max(lenv *, lval *);
lval *builtin_min(lenv *, lval *);

lval *builtin_lambda(lenv *, lval *);
lval *builtin_def(lenv *, lval *);
lval *builtin_put(lenv *, lval *);
lval *builtin_var(lenv *, lval *, char *);

lval *builtin_head(lenv *, lval *);
lval *builtin_tail(lenv *, lval *);
lval *builtin_list(lenv *, lval *);
lval *builtin_eval(lenv *, lval *);
lval *builtin_join(lenv *, lval *);
lval *builtin_cons(lenv *, lval *);
lval *builtin_len(lenv *, lval *);
lval *builtin_init(lenv *, lval *);

#endif
