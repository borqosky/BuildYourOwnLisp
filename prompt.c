#include <stdio.h>

/* Declare a buffer for user input of size 2048 */
static char input[2048];

int main(int argc, char* argv[]) {
    /* Print Version and Exit Information */
    puts("Lispy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    /* In a newer ending loop */
    while (1) {
        /* Outputs our prompt */
        fputs("lispy> ", stdout);

        /* Read a line of user input of maximum size 2048 */
        fgets(input, sizeof(input), stdin);

        /* Echo input back to user */
        printf("Now you're a %s", input);
    }
    return 0;
}
