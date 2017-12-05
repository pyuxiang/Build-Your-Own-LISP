#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "lang_parser_set.h"

parser_set_t *polish_notation_set(void);

typedef struct lval {
    int type; // specifies type of lval and field to access
    long num; // successful num result
    char *err; // runtime error code
    char *sym; // symbol string data

    int count; // lval* count
    struct lval** cell;
} lval;

// lval types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

// Constructors and deconstructors
lval *lval_num(long);
lval *lval_err(char *);
lval *lval_sym(char *);
lval *lval_sexpr(void);
lval *lval_qexpr(void);
void lval_free(lval *);

// lval methods
lval *lval_add(lval *, lval *);
lval *lval_insert(lval *, lval *, int);
lval *lval_pop(lval *, int);
lval *lval_extract(lval *, int);

// Display
void lval_expr_print(lval *, char, char);
void lval_print(lval *);
void lval_println(lval *);

// Read
lval *lval_read(mpc_ast_t *);

// Evaluate (sexpr)
lval *lval_eval(lval *);
lval *builtin(lval *, char *);
lval *builtin_op(lval *, char *);

// Evaluate qexpr
#define LASSERT(args, cond, err) \
    if (!(cond)) { lval_free(args); return lval_err(err); }
lval *builtin_head(lval *);
lval *builtin_tail(lval *);
lval *builtin_list(lval *);
lval *builtin_eval(lval *);
lval *builtin_join(lval *);
lval *builtin_cons(lval *);
lval *builtin_len(lval *);
lval *builtin_init(lval *);
lval *lval_join(lval *, lval *);
