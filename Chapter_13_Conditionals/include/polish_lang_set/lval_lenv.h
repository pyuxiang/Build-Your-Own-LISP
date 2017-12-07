#ifndef lval_lenv_h
#define lval_lenv_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include "mpc.h"

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

// Function pointer
// lbuiltin is pointer to function that takes in lenv* and lval*.
typedef lval *(*lbuiltin)(lenv *, lval *);

struct lval {
    int type; // specifies type of lval and field to access

    // Basic types
    long num; // successful num result
    char *err; // runtime error code
    char *sym; // symbol string data

    // Function types
    lbuiltin builtin;
    lenv *env;
    lval *formals;
    lval *body;

    // Expression types
    int count; // lval* count
    struct lval** cell;
};

// Environment to store variables
struct lenv {
    // sym-val pair at each index
    lenv *parent;
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
lenv *lenv_copy(lenv *);
void lenv_def(lenv *, lval *, lval *);
void lval_check_get_replace(lenv *, lval *);
void lval_get_replace(lenv *, lval *);
void lenv_print_dir(lenv *);
void lenv_add_builtin(lenv *, char *, lbuiltin);

// lval types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM,
       LVAL_FUNC, LVAL_SEXPR, LVAL_QEXPR };

// lval constructors and deconstructors
lval *lval_num(long);
lval *lval_err(char *, ...);
lval *lval_sym(char *);
lval *lval_func(lbuiltin);
lval *lval_lambda(lval *, lval *);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
void lval_free(lval *);

// lval methods
int lval_bool(lval *);
char *lval_type_name(int);
int lval_eq(lval *, lval *);
lval *lval_add(lval *, lval *);
lval *lval_insert(lval *, lval *, int);
lval *lval_pop(lval *, int);
lval *lval_extract(lval *, int);
lval *lval_copy(lval *);
lval *lval_join(lval *, lval *);

// lval display
void lval_expr_print(lval *, char, char);
void lval_print(lval *);
void lval_println(lval *);

// lval read
lval *lval_read(mpc_ast_t *);

// Evaluate (sexpr)
lval *lval_eval(lenv *, lval *);
lval *lval_eval_sexpr(lenv *, lval *);
lval *lval_call(lenv *, lval *, lval *);

#endif
