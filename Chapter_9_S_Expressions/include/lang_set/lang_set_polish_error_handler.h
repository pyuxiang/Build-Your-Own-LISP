#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

// lval - Lisp Value
typedef struct lval {
    int type; // specifies type of lval and field to access
    long num; // successful num result
    char *err; // runtime error code
    char *sym; // symbol string data

    int count; // lval* count
    struct lval** cell;
} lval;

// lval types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };

// Constructors and deconstructors
lval *lval_num(long);
lval *lval_err(char *);
lval *lval_sym(char *);
lval *lval_sexpr(void);
lval *lval_add(lval *, lval *);
void lval_free(lval *);

// Display
void lval_print(lval *);
void lval_println(lval *);
