#include "readline_history.h"
#include "mpc.h"
#include "lang_set.h"
#include "builtin.h"

// Evaluation of mathematical results with polish notation
int main(int argc, char **argv) {

    // Language specification
    puts("LISP version 0.14");
    parser_set_t *parser_set = polish_notation_set();
    if (parser_set == NULL) { exit(1); }

    lenv *env = lenv_new();
    // Command line arguments are provided, i.e. filenames
    // Load environment with library
    if (argc > 1) {
        int i;
        for (i = 1; i < argc; i++) {
            lval *args = lval_add(lval_sexpr(), lval_str(argv[i]));
            lval *result = builtin_load(env, args);
            if (result->type == LVAL_ERR) { lval_println(result); }
            lval_free(result); // args freed in builtin_load
        }
    }


    // REPL Interpreter
    lenv_add_builtins(env);
    puts("Type 'dir' for available functions.");
    while (1) {
        char *input = readline(">>> ");
        if (strcmp(input, ":q") == 0) { break; } // exit
        add_history(input);

        // Parsing
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, parser_set->parser, &r)) {
            // Interpretation successful
            lval *value = lval_eval(env, lval_read(r.output));
            lval_println(value);
            lval_free(value);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    lenv_free(env);
    clear_parser_set(parser_set);
    return 0;
}
