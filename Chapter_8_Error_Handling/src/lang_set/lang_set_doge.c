#include "lang_set_doge.h"

// DOGE LANGUAGE
/* noun      : "LISP" | "language" | "book" | "build" | "c" ;
 * adjective : "wow" | "many" | "so" | "such";
 * phrase    : <adjective> <noun>;
 * doge      : <phrase>*;
 */

parser_set_t *doge_set(void) {

    mpc_parser_t *Noun = mpc_new("noun");
    mpc_parser_t *Adjective = mpc_new("adjective");
    mpc_parser_t *Phrase = mpc_new("phrase");
    mpc_parser_t *Doge = mpc_new("doge");

    // Put more specific rules in front (left-right precedence)
    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                                   \
            noun : \"LISP\" | \"language\" | \"book\" | \"build\" | \"c\";  \
            adjective : \"wow\" | \"many\" | \"so\" | \"such\" ;            \
            phrase: <adjective> <noun> ;                                    \
            doge : /^/ <phrase>* /$/ ;                                      \
        ",
        Noun, Adjective, Phrase, Doge);

    return create_parser_set(4, Noun, Adjective, Phrase, Doge);
}
