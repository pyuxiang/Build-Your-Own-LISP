#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "lang_parser_set.h"

parser_set_t *polish_notation_set(void);

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

// Function pointer
// lbuiltin is pointer to function that takes in lenv* and lval*.
typedef lval *(*lbuiltin)(lenv *, lval *);

struct lval {
    int type; // specifies type of lval and field to access
    long num; // successful num result
    char *err; // runtime error code
    char *sym; // symbol string data
    lbuiltin func;

    int count; // lval* count
    struct lval** cell;
};

// Environment to store variables
struct lenv {
    // sym-val pair at each index
    int count;
    char **syms;
    lval **vals;
};

// lenv constructors and deconstructors
lenv *lenv_new(void);
void lenv_free(lenv *);

// Accessors
lval *lenv_get(lenv *, lval *);
void lenv_put(lenv *, lval *, lval *);
void lval_check_get_replace(lenv *, lval *);
void lval_get_replace(lenv *, lval *);
void lenv_add_builtin(lenv *, char *, lbuiltin);
void lenv_add_builtins(lenv *);

// lval types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM,
       LVAL_FUNC, LVAL_SEXPR, LVAL_QEXPR };

// lval constructors and deconstructors
lval *lval_num(long);
lval *lval_err(char *);
lval *lval_sym(char *);
lval *lval_func(lbuiltin func);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
void lval_free(lval *);

// lval methods
lval *lval_add(lval *, lval *);
lval *lval_insert(lval *, lval *, int);
lval *lval_pop(lval *, int);
lval *lval_extract(lval *, int);
lval *lval_copy(lval *);

// lval display
void lval_expr_print(lval *, char, char);
void lval_print(lval *);
void lval_println(lval *);

// lval read
lval *lval_read(mpc_ast_t *);

// Evaluate (sexpr)
lval *lval_eval(lenv *, lval *);
lval *lval_eval_sexpr(lenv *, lval *);
lval *builtin_op(lenv *, lval *, char *);

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

lval *builtin_head(lenv *, lval *);
lval *builtin_tail(lenv *, lval *);
lval *builtin_list(lenv *, lval *);
lval *builtin_eval(lenv *, lval *);
lval *builtin_join(lenv *, lval *);
lval *builtin_cons(lenv *, lval *);
lval *builtin_len(lenv *, lval *);
lval *builtin_init(lenv *, lval *);
lval *lval_join(lval *, lval *);
