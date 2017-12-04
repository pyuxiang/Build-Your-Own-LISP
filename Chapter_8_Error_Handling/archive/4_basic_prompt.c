#include <stdio.h>
#include <stdlib.h>

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

    puts("LISP version 0.1");
    while (1) {
        char *input = readline(">>> ");
        add_history(input);

        // Newline is captured in fgets
        printf("Echo: %s\n", input);
        free(input);
    }

    return 0;
}
