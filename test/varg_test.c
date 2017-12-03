#include <stdio.h>
#include <stdarg.h>

double multiply(int argc, ...) {
    va_list valist;
    va_start(valist, argc);

    double result = 1.0;
    int i;
    for (i = 0; i < argc; i++) {
        result *= va_arg(valist, double);
    }

    va_end(valist);
    return result;
}

int main() {
    printf("%f\n", multiply(4, 1.2, 2.3, 3.4, 4.5));
    return 0;
}
