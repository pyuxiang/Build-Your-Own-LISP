#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>
char *readline(char *);
void add_history(char *);

#else
#include <editline/readline.h> // Remember to link library
#include <editline/history.h>

#endif
