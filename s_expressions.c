#include <stdio.h>#include <stdlib.h>#include <signal.h>#include <math.h>#include "mpc.h"/* If we are compiling on Windows compile these functions */#ifdef _WIN32#include <string.h>static char buffer[2048];/* Fake readline function */char* readline(char* prompt) {    fputs(prompt, stdout);    fgets(buffer, sizeof(buffer), stdin);    char* cpy = malloc(strlen(buffer)+1);    strcpy(cpy, buffer);    return cpy;}/* Fake add_history function */void add_history(const char* unused) {}/* Otherwise include the editline headers */#else#include <editline/readline.h>#endifstatic mpc_parser_t* Number;static mpc_parser_t* Symbol;static mpc_parser_t* Expr;static mpc_parser_t* Sexpr;static mpc_parser_t* Lispy;static char* input;#define MAX(a, b) (a.num > b.num ? a.num : b.num)#define MIN(a, b) (a.num < b.num ? a.num : b.num)/* Create Enumeration of Possible lval Types */enum {LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR};/* Declare New lval Struct */typedef struct {    int type;    double num;    /* Error and Symbol types have some string data */    char* err;    char* sym;    /* Count and Pointer to a list of "lval*" */    int count;    struct lval** cell;} lval;/* Undefine and Delete our Parsers */void exit_handler(int signum) {    if (input)        free(input);    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);}/* Create a new number type lval */lval lval_num(double x) {    lval v;    v.type = LVAL_NUM;    v.num = x;    return v;}/* Create a new error type lval */lval lval_err(char* x) {    lval v;    v.type = LVAL_ERR;    v.err = x;    return v;}static inline int isInteger(double x) {    return x - floor(x) == 0;}/* Print an "lval" */static void lval_print(lval v) {    switch (v.type) {        /* In the case the type is a number print it */        /* Then 'break' out of the switch. */        case LVAL_NUM:            if (isInteger(v.num))                printf("%li", (long) v.num);            else                printf("%g", v.num);            break;        /* In the case the type is an error */        case LVAL_ERR:            /* Check what type of error it is and print it */            fprintf(stderr, "Error: %s", v.err);            break;        default:            fprintf(stderr, "Catastrofical error!");    }}/* Print an "lval" followed by a newline */void lval_println(lval v) {    lval_print(v);    putchar('\n');}/* Use operator string to see which operation to perform */lval eval_op(lval x, char* op, lval y) {    if (x.type == LVAL_ERR)        return x;    if (y.type == LVAL_ERR)        return y;    if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { return lval_num(x.num + y.num); }    else if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { return lval_num(x.num - y.num); }    else if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) { return lval_num(x.num * y.num); }    else if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) {        /* If second operand is zero return error */        return y.num == 0 ? lval_err("Division by zero") : lval_num(x.num / y.num); }    else if (strcmp(op, "%") == 0) { return lval_num((long) x.num % (long) y.num); }    else if (strcmp(op, "^") == 0) { return lval_num(pow(x.num, y.num)); }    else if (strcmp(op, "min") == 0) { return  lval_num(MIN(x, y)); }    else if (strcmp(op, "max") == 0) { return  lval_num(MAX(x, y)); }    else return lval_err("Bad number given");}lval eval(mpc_ast_t* t) {    /* If tagged as number return it directly. */    if (strstr(t->tag, "number")) {        /* Check if there is some error in conversion */        errno = 0;        double x = strtod(t->contents, NULL);        return errno != ERANGE ? lval_num(x) : lval_err("Bad number given.");    }    /* The operator is always second child. */    char* op = t->children[1]->contents;    /* We store the third child in `x` */    lval x = eval(t->children[2]);    /* Iterate the remaining children and combining. */    int i = 3;    while (strstr(t->children[i]->tag, "expr")) {        x = eval_op(x, op, eval(t->children[i]));        i++;    }    /* One number with sign case. */    return (i == 3 && strcmp(op, "-") == 0) ? lval_num(-x.num) : x;}int main(int argc, char* argv[]) {    /* Print Version and Exit Information */    puts("Lispy Version 0.0.0.0.7");    puts("Press Ctrl+c to Exit\n");    signal(SIGINT, exit_handler);    /* Create Some Parsers */    Number = mpc_new("number");    Symbol = mpc_new("symbol");    Expr = mpc_new("expr");    Sexpr = mpc_new("sexpr");    Lispy = mpc_new("lispy");    /* Define them with the following Language */    mpca_lang(MPCA_LANG_DEFAULT,              "                                                     \                number   : /-?[0-9]+\\.?[0-9]+/ ;                   \                symbol : '+' | '-' | '*' | '/' | '%' | '^'          \                         | \"add\" | \"sub\" | \"mul\" | \"div\"    \                         | \"min\" | \"max\" ;                      \                sexpr    : '(' <expr>* ')' ;                        \                expr     : <number> | <symbol> | <sexpr> ;          \                lispy    : /^/ <expr>* /$/ ;                        \              ",              Number, Symbol, Sexpr, Expr, Lispy);    /* In a newer ending loop */    while (1) {        /* Outputs our prompt and get input */        input = readline("lispy> ");        /* Add input to history */        add_history(input);        /* Attempt to Parse the user Input */        mpc_result_t r;        if (mpc_parse("<stdin>", input, Lispy, &r)) {            lval result = eval(r.output);            lval_println(result);            mpc_ast_delete(r.output);        } else {            if (strcmp(input, "\n") == 0) {                free(input);                continue;            }            /* Otherwise Print the Error */            mpc_err_print(r.error);            mpc_err_delete(r.error);        }        /* Free retrieved input */        free(input);    }    return 0;}