#include "lang_set_polish_error_handler.h"

// Create lval instances of num type
lval lval_num(long result) {
    lval value;
    value.type = LVAL_NUM;
    value.num = result;
    return value;
}

lval lval_err(int error_code) {
    lval value;
    value.type = LVAL_ERR;
    value.err = error_code;
    return value;
}

// A bit apprehensive with the use of switch
// since it is a goto label under the hood
// But it sure is convenient with enum values!
void lval_print(lval value) {
    switch (value.type) {
        case LVAL_NUM: {
            printf("%li", value.num);
            break;
        }
        case LVAL_ERR: {
            switch (value.err) {
                case LERR_DIV_ZERO:
                    printf("Error: Division by zero!");
                    break;
                case LERR_BAD_OP:
                    printf("Error: Invalid operator!");
                    break;
                case LERR_BAD_NUM:
                    printf("Error: Invalid number!");
                    break;
                case LERR_BAD_ARGS:
                    printf("Error: Invalid number of arguments!");
                    break;
            }
            break;
        }
    }
}

// lval_print with newline
void lval_println(lval value) {
    lval_print(value);
    putchar('\n');
}
