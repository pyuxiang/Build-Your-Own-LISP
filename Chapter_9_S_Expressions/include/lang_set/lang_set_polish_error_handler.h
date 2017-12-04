#include <stdio.h>
#include <errno.h>
#include <limits.h>

// lval - Lisp Value
typedef struct {
    int type; // specifies which field to access
    long num; // successful eval result
    int err; // runtime error code
} lval;
// union data types should not be used, since dispatch-on-type


// lval types
enum { LVAL_NUM, LVAL_ERR };

// err types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_BAD_ARGS };

lval lval_num(long);
lval lval_err(int);
void lval_print(lval);
void lval_println(lval);
