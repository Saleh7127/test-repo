#!/usr/bin/env python3
"""One-liner calculator: evaluate expressions with + - * / ^ and parentheses."""

import re
import sys


def evaluate(expression: str) -> float:
    trimmed = expression.strip()
    if not trimmed:
        raise ValueError("Enter an expression")

    if not re.fullmatch(r"[\d\s+\-*/().^]+", trimmed):
        raise ValueError("Only numbers and + - * / ^ ( ) are allowed")

    tokens = re.findall(r"\d+\.?\d*|[+\-*/()^]", trimmed)
    if not tokens:
        raise ValueError("Invalid expression")

    i = 0

    def peek():
        return tokens[i] if i < len(tokens) else None

    def consume():
        nonlocal i
        token = tokens[i]
        i += 1
        return token

    def parse_number():
        token = consume()
        if token == "(":
            value = parse_add_sub()
            if consume() != ")":
                raise ValueError("Missing closing parenthesis")
            return value
        try:
            return float(token)
        except ValueError as exc:
            raise ValueError("Invalid number") from exc

    def parse_power():
        left = parse_number()
        if peek() == "^":
            consume()
            right = parse_power()
            left = left**right
        return left

    def parse_mul_div():
        left = parse_power()
        while peek() in ("*", "/"):
            op = consume()
            right = parse_power()
            if op == "/" and right == 0:
                raise ValueError("Division by zero")
            left = left * right if op == "*" else left / right
        return left

    def parse_add_sub():
        left = parse_mul_div()
        while peek() in ("+", "-"):
            op = consume()
            right = parse_mul_div()
            left = left + right if op == "+" else left - right
        return left

    value = parse_add_sub()
    if i < len(tokens):
        raise ValueError("Unexpected characters")
    return value


def format_result(value: float) -> str:
    if value == int(value):
        return str(int(value))
    return str(value)


def main() -> None:
    if len(sys.argv) > 1:
        expression = " ".join(sys.argv[1:])
        try:
            print(format_result(evaluate(expression)))
        except ValueError as err:
            print(err, file=sys.stderr)
            sys.exit(1)
        return

    print("One-Liner Calculator")
    print("Supports + - * / ^ (exponent) and parentheses. Type 'quit' to exit.\n")

    while True:
        try:
            expression = input("> ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break

        if expression.lower() in {"quit", "exit", "q"}:
            break
        if not expression:
            continue

        try:
            print(format_result(evaluate(expression)))
        except ValueError as err:
            print(err)


if __name__ == "__main__":
    main()
