#!/usr/bin/env python3


def calculate(a: float, b: float, op: str) -> float:
    if op == "+":
        return a + b
    if op == "-":
        return a - b
    if op == "*":
        return a * b
    if op == "/":
        if b == 0:
            raise ValueError("Division by zero")
        return a / b
    raise ValueError("Invalid operator")


def main() -> None:
    try:
        a = float(input("Enter first number: "))
        b = float(input("Enter second number: "))
        op = input("Enter operator (+, -, *, /): ").strip()
        print(f"Result: {calculate(a, b, op)}")
    except ValueError as err:
        print(err)


if __name__ == "__main__":
    main()
