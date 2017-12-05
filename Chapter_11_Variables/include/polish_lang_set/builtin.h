#include "lval_lenv.h"

lval *builtin_op(lenv *, lval *, char *);
void lenv_add_builtins(lenv *);

// Evaluate qexpr
#define LASSERT(args, cond, err) \
    if (!(cond)) { lval_free(args); return lval_err(err); }

lval *builtin_add(lenv *, lval *);
lval *builtin_sub(lenv *, lval *);
lval *builtin_mul(lenv *, lval *);
lval *builtin_div(lenv *, lval *);
lval *builtin_mod(lenv *, lval *);
lval *builtin_pow(lenv *, lval *);
lval *builtin_max(lenv *, lval *);
lval *builtin_min(lenv *, lval *);

lval *builtin_def(lenv *, lval *);
lval *builtin_head(lenv *, lval *);
lval *builtin_tail(lenv *, lval *);
lval *builtin_list(lenv *, lval *);
lval *builtin_eval(lenv *, lval *);
lval *builtin_join(lenv *, lval *);
lval *builtin_cons(lenv *, lval *);
lval *builtin_len(lenv *, lval *);
lval *builtin_init(lenv *, lval *);
