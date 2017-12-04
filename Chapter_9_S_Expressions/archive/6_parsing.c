#include <stdio.h>
#include "mpc.h"

#ifdef _WIN32
    #include <string.h>
    static char buffer[2048];

    // Simulate readline
    char *readline(char *prompt) {
        fputs(prompt, stdout);
        fgets(buffer, 2048, stdin);

        // Ensure buffer never exceeeded
        char *buffer_cpy = malloc(strlen(buffer) + 1);
        strcpy(buffer_cpy, buffer);
        buffer_cpy[strlen(buffer) - 1] = '\0'; // Removes \n
        return buffer_cpy;
    }

    // Ignore function (functionality already in _WIN32)
    void add_history(char *unused) {}

#else
    #include <editline/readline.h> // Remember to link library
    #include <editline/history.h>
#endif





int main(int argc, char **argv) {

    // Parsers
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                  \
            number : /-?[0-9]+/ ;                          \
            operator : '+' | '-' | '*' | '/' ;             \
            expr : <number> | '(' <operator> <expr>+ ')' ; \
            lispy : /^/ <operator> <expr>+ /$/ ;           \
        ",
        Number, Operator, Expr, Lispy);

    puts("LISP version 0.2");
    while (1) {
        char *input = readline(">>> ");
        add_history(input);

        mpc_result_t r;
        if (mpc_parse("<stdin", input, Lispy, &r)) {
            // Interpretation successful
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    // Clean up: Undefine and delete Parsers
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}
