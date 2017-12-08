#include "readline_history.h"

#ifdef _WIN32
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

#endif
