#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKENS 256
#define MAX_TOKEN_LEN 64
#define MAX_INPUT 1024

typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_POWER,
    TOKEN_END
} TokenType;

typedef struct {
    TokenType type;
    double value;
} Token;

typedef struct {
    Token tokens[MAX_TOKENS];
    int count;
    int index;
} Parser;

typedef enum {
    EVAL_OK = 0,
    EVAL_EMPTY,
    EVAL_INVALID_CHARS,
    EVAL_INVALID,
    EVAL_INVALID_NUMBER,
    EVAL_MISSING_RPAREN,
    EVAL_DIV_ZERO,
    EVAL_UNEXPECTED
} EvalError;

static const char *error_message(EvalError err) {
    switch (err) {
    case EVAL_EMPTY:
        return "Enter an expression";
    case EVAL_INVALID_CHARS:
        return "Only numbers and + - * / ^ ( ) are allowed";
    case EVAL_INVALID:
        return "Invalid expression";
    case EVAL_INVALID_NUMBER:
        return "Invalid number";
    case EVAL_MISSING_RPAREN:
        return "Missing closing parenthesis";
    case EVAL_DIV_ZERO:
        return "Division by zero";
    case EVAL_UNEXPECTED:
        return "Unexpected characters";
    default:
        return "Unknown error";
    }
}

static void trim(char *s) {
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

static int is_allowed_char(char c) {
    return isdigit((unsigned char)c) || strchr("+-*/().^ \t", c) != NULL;
}

static int tokenize(const char *input, Token *tokens, int *count, EvalError *err) {
    int i = 0;
    const char *p = input;

    *count = 0;
    *err = EVAL_OK;

    while (*p) {
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }

        if (isdigit((unsigned char)*p) || *p == '.') {
            char buffer[MAX_TOKEN_LEN];
            int j = 0;
            int saw_dot = 0;

            while (isdigit((unsigned char)*p) || *p == '.') {
                if (*p == '.') {
                    if (saw_dot) {
                        *err = EVAL_INVALID_NUMBER;
                        return -1;
                    }
                    saw_dot = 1;
                }
                if (j >= MAX_TOKEN_LEN - 1) {
                    *err = EVAL_INVALID_NUMBER;
                    return -1;
                }
                buffer[j++] = *p++;
            }
            buffer[j] = '\0';

            char *end = NULL;
            double value = strtod(buffer, &end);
            if (end == buffer || *end != '\0') {
                *err = EVAL_INVALID_NUMBER;
                return -1;
            }

            tokens[i].type = TOKEN_NUMBER;
            tokens[i].value = value;
            i++;
            continue;
        }

        if (strchr("+-*/()^", *p)) {
            switch (*p) {
            case '+':
                tokens[i].type = TOKEN_PLUS;
                break;
            case '-':
                tokens[i].type = TOKEN_MINUS;
                break;
            case '*':
                tokens[i].type = TOKEN_MUL;
                break;
            case '/':
                tokens[i].type = TOKEN_DIV;
                break;
            case '(':
                tokens[i].type = TOKEN_LPAREN;
                break;
            case ')':
                tokens[i].type = TOKEN_RPAREN;
                break;
            case '^':
                tokens[i].type = TOKEN_POWER;
                break;
            default:
                *err = EVAL_INVALID;
                return -1;
            }
            tokens[i].value = 0.0;
            i++;
            p++;
            continue;
        }

        *err = EVAL_INVALID_CHARS;
        return -1;
    }

    if (i == 0) {
        *err = EVAL_INVALID;
        return -1;
    }

    tokens[i].type = TOKEN_END;
    tokens[i].value = 0.0;
    *count = i;
    return 0;
}

static TokenType peek(const Parser *parser) {
    return parser->tokens[parser->index].type;
}

static Token consume(Parser *parser) {
    return parser->tokens[parser->index++];
}

static int parse_number(Parser *parser, double *out, EvalError *err);
static int parse_power(Parser *parser, double *out, EvalError *err);
static int parse_mul_div(Parser *parser, double *out, EvalError *err);
static int parse_add_sub(Parser *parser, double *out, EvalError *err);

static int parse_number(Parser *parser, double *out, EvalError *err) {
    Token token = consume(parser);

    if (token.type == TOKEN_LPAREN) {
        if (parse_add_sub(parser, out, err) != 0) {
            return -1;
        }
        if (consume(parser).type != TOKEN_RPAREN) {
            *err = EVAL_MISSING_RPAREN;
            return -1;
        }
        return 0;
    }

    if (token.type != TOKEN_NUMBER) {
        *err = EVAL_INVALID_NUMBER;
        return -1;
    }

    *out = token.value;
    return 0;
}

static int parse_power(Parser *parser, double *out, EvalError *err) {
    double left;
    double right;

    if (parse_number(parser, &left, err) != 0) {
        return -1;
    }

    if (peek(parser) == TOKEN_POWER) {
        consume(parser);
        if (parse_power(parser, &right, err) != 0) {
            return -1;
        }
        left = pow(left, right);
    }

    *out = left;
    return 0;
}

static int parse_mul_div(Parser *parser, double *out, EvalError *err) {
    double left;
    double right;

    if (parse_power(parser, &left, err) != 0) {
        return -1;
    }

    while (peek(parser) == TOKEN_MUL || peek(parser) == TOKEN_DIV) {
        TokenType op = consume(parser).type;
        if (parse_power(parser, &right, err) != 0) {
            return -1;
        }
        if (op == TOKEN_DIV && right == 0.0) {
            *err = EVAL_DIV_ZERO;
            return -1;
        }
        left = (op == TOKEN_MUL) ? left * right : left / right;
    }

    *out = left;
    return 0;
}

static int parse_add_sub(Parser *parser, double *out, EvalError *err) {
    double left;
    double right;

    if (parse_mul_div(parser, &left, err) != 0) {
        return -1;
    }

    while (peek(parser) == TOKEN_PLUS || peek(parser) == TOKEN_MINUS) {
        TokenType op = consume(parser).type;
        if (parse_mul_div(parser, &right, err) != 0) {
            return -1;
        }
        left = (op == TOKEN_PLUS) ? left + right : left - right;
    }

    *out = left;
    return 0;
}

static int evaluate(const char *expression, double *result, EvalError *err) {
    char input[MAX_INPUT];
    Token tokens[MAX_TOKENS + 1];
    int count = 0;
    Parser parser;
    double value = 0.0;

    strncpy(input, expression, sizeof(input) - 1);
    input[sizeof(input) - 1] = '\0';
    trim(input);

    if (input[0] == '\0') {
        *err = EVAL_EMPTY;
        return -1;
    }

    for (const char *p = input; *p; p++) {
        if (!is_allowed_char(*p)) {
            *err = EVAL_INVALID_CHARS;
            return -1;
        }
    }

    if (tokenize(input, tokens, &count, err) != 0) {
        return -1;
    }

    parser.tokens[0] = tokens[0];
    for (int i = 1; i <= count; i++) {
        parser.tokens[i] = tokens[i];
    }
    parser.count = count;
    parser.index = 0;

    if (parse_add_sub(&parser, &value, err) != 0) {
        return -1;
    }

    if (parser.index < parser.count) {
        *err = EVAL_UNEXPECTED;
        return -1;
    }

    *result = value;
    *err = EVAL_OK;
    return 0;
}

static void print_result(double value) {
    double rounded = round(value);
    if (fabs(value - rounded) < 1e-9) {
        printf("%.0f\n", value);
    } else {
        printf("%g\n", value);
    }
}

static int is_quit_command(const char *input) {
    return strcmp(input, "quit") == 0 ||
           strcmp(input, "exit") == 0 ||
           strcmp(input, "q") == 0 ||
           strcmp(input, "QUIT") == 0 ||
           strcmp(input, "EXIT") == 0 ||
           strcmp(input, "Q") == 0;
}

static void run_expression(const char *expression, int to_stderr) {
    double result = 0.0;
    EvalError err = EVAL_OK;

    if (evaluate(expression, &result, &err) != 0) {
        fprintf(to_stderr ? stderr : stdout, "%s\n", error_message(err));
        if (to_stderr) {
            exit(1);
        }
        return;
    }

    print_result(result);
}

static void run_interactive(void) {
    char line[MAX_INPUT];

    printf("One-Liner Calculator\n");
    printf("Supports + - * / ^ (exponent) and parentheses. Type 'quit' to exit.\n\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        trim(line);
        if (line[0] == '\0') {
            continue;
        }
        if (is_quit_command(line)) {
            break;
        }

        run_expression(line, 0);
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        char expression[MAX_INPUT];
        size_t offset = 0;

        expression[0] = '\0';
        for (int i = 1; i < argc; i++) {
            if (i > 1 && offset < sizeof(expression) - 1) {
                expression[offset++] = ' ';
                expression[offset] = '\0';
            }
            strncat(expression, argv[i], sizeof(expression) - strlen(expression) - 1);
        }

        run_expression(expression, 1);
        return 0;
    }

    run_interactive();
    return 0;
}
