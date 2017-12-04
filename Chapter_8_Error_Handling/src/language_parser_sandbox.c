#include <stdio.h>
#include <stdarg.h>
#include "mpc.h"
#include "language_set.h"
#include "mpc_ast_misc.h"

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



// Evaluation of mathematical results with polish notation
int main(int argc, char **argv) {

    // Language specification
    puts("LISP version 0.7");
    parser_set_t *parser_set = polish_notation_set();
    if (parser_set == NULL) {
        exit(1);
    }

    // REPL
    while (1) {
        char *input = readline(">>> ");
        if (strcmp(input, ":q") == 0) { break; } // exit
        add_history(input);

        // Parsing
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, parser_set->parser, &r)) {
            // Interpretation successful
            printf("Number of nodes = %d\n", count_leaves(r.output));
            long result = polish_eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }
    clear_parser_set(parser_set);
    return 0;
}
