#include <stdio.h>

int main(void) {
    double a, b;
    char op;

    printf("Enter first number: ");
    if (scanf("%lf", &a) != 1) {
        printf("Invalid input\n");
        return 1;
    }

    printf("Enter second number: ");
    if (scanf("%lf", &b) != 1) {
        printf("Invalid input\n");
        return 1;
    }

    printf("Enter operator (+, -, *, /): ");
    if (scanf(" %c", &op) != 1) {
        printf("Invalid input\n");
        return 1;
    }

    switch (op) {
    case '+':
        printf("Result: %g\n", a + b);
        break;
    case '-':
        printf("Result: %g\n", a - b);
        break;
    case '*':
        printf("Result: %g\n", a * b);
        break;
    case '/':
        if (b == 0.0) {
            printf("Error: Division by zero\n");
            return 1;
        }
        printf("Result: %g\n", a / b);
        break;
    default:
        printf("Invalid operator\n");
        return 1;
    }

    return 0;
}
