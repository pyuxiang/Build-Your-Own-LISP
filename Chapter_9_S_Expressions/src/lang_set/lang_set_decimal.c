#include "lang_set_decimal.h"

/* Decimals
 * digit   : <nonzero> | <zero>;
 * integer : <zero> | (<nonzero> <digit>*);
 * decimal : <integer> '.' <digit>* <nonzero>;
 */

parser_set_t *decimal_set(void) {

    mpc_parser_t *Integer = mpc_new("integer");
    mpc_parser_t *Decimal = mpc_new("decimal");
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Lisp = mpc_new("lisp");

    // Put more specific rules in front (left-right precedence)
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                      \
            integer : /-?[1-9][0-9]*/ | /0/ ;  \
            decimal : <integer> '.' /[0-9]+/ ; \
            number: <decimal> | <integer> ;    \
            lisp : /^/ <number> /$/ ;          \
        ",
        Integer, Decimal, Number, Lisp);

    return create_parser_set(4, Integer, Decimal, Number, Lisp);
}
