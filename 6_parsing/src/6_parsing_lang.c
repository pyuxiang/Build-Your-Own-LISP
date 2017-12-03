#include <stdio.h>
#include <stdarg.h>
#include "mpc.h"
#include "6_lang_set.h"

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

    // Language specification
    parser_set_t *parser_set;
    char *input = readline("Specify language: ");

    if (strcmp(input, "polish") == 0) {
        parser_set = polish_notation_set();
    } else if (strcmp(input, "decimal") == 0) {
        parser_set = decimal_set();
    } else if (strcmp(input, "doge") == 0) {
        parser_set = doge_set();
    } else {
        puts("Unknown language!");
        exit(1);
    }

    // REPL
    printf("LISP %s (v0.2)\n", input);
    while (1) {
        char *input = readline(">>> ");
        add_history(input);

        // Exit program
        if (strcmp(input, ":q") == 0) {
            break;
        }

        // Parsing
        mpc_result_t r;
        if (mpc_parse("<stdin", input, parser_set->parser, &r)) {
            // Interpretation successful
            mpc_ast_print(r.output);
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
