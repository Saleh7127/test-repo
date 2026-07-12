#!/usr/bin/env python3
"""Simple measurement converter: length, weight, and temperature."""

from __future__ import annotations

import sys
from dataclasses import dataclass


@dataclass(frozen=True)
class Unit:
    name: str
    aliases: tuple[str, ...]
    to_base: float  # multiply by this to get base unit


@dataclass(frozen=True)
class Category:
    name: str
    base_unit: str
    units: tuple[Unit, ...]


LENGTH = Category(
    name="length",
    base_unit="meter",
    units=(
        Unit("millimeter", ("mm", "millimeter", "millimeters"), 0.001),
        Unit("centimeter", ("cm", "centimeter", "centimeters"), 0.01),
        Unit("meter", ("m", "meter", "meters"), 1.0),
        Unit("kilometer", ("km", "kilometer", "kilometers"), 1000.0),
        Unit("inch", ("in", "inch", "inches"), 0.0254),
        Unit("foot", ("ft", "foot", "feet"), 0.3048),
        Unit("yard", ("yd", "yard", "yards"), 0.9144),
        Unit("mile", ("mi", "mile", "miles"), 1609.344),
    ),
)

WEIGHT = Category(
    name="weight",
    base_unit="kilogram",
    units=(
        Unit("milligram", ("mg", "milligram", "milligrams"), 0.000001),
        Unit("gram", ("g", "gram", "grams"), 0.001),
        Unit("kilogram", ("kg", "kilogram", "kilograms"), 1.0),
        Unit("ounce", ("oz", "ounce", "ounces"), 0.0283495),
        Unit("pound", ("lb", "lbs", "pound", "pounds"), 0.453592),
        Unit("ton", ("t", "ton", "tons"), 1000.0),
    ),
)

TEMPERATURE = Category(
    name="temperature",
    base_unit="celsius",
    units=(
        Unit("celsius", ("c", "celsius", "centigrade"), 1.0),
        Unit("fahrenheit", ("f", "fahrenheit"), 1.0),
        Unit("kelvin", ("k", "kelvin"), 1.0),
    ),
)

CATEGORIES = (LENGTH, WEIGHT, TEMPERATURE)


def normalize(text: str) -> str:
    return text.strip().lower()


def find_unit(category: Category, name: str) -> Unit:
    key = normalize(name)
    for unit in category.units:
        if key in unit.aliases or key == unit.name:
            return unit
    raise ValueError(
        f"Unknown {category.name} unit '{name}'. "
        f"Try: {', '.join(unit.aliases[0] for unit in category.units)}"
    )


def convert_linear(value: float, source: Unit, target: Unit) -> float:
    base_value = value * source.to_base
    return base_value / target.to_base


def convert_temperature(value: float, source: Unit, target: Unit) -> float:
    if source.name == target.name:
        return value

    if source.name == "celsius":
        celsius = value
    elif source.name == "fahrenheit":
        celsius = (value - 32) * 5 / 9
    elif source.name == "kelvin":
        celsius = value - 273.15
    else:
        raise ValueError("Unsupported temperature unit")

    if target.name == "celsius":
        return celsius
    if target.name == "fahrenheit":
        return celsius * 9 / 5 + 32
    if target.name == "kelvin":
        return celsius + 273.15
    raise ValueError("Unsupported temperature unit")


def convert(category_name: str, value: float, source_name: str, target_name: str) -> float:
    category = find_category(category_name)
    source = find_unit(category, source_name)
    target = find_unit(category, target_name)

    if category.name == "temperature":
        return convert_temperature(value, source, target)
    return convert_linear(value, source, target)


def find_category(name: str) -> Category:
    key = normalize(name)
    for category in CATEGORIES:
        if key in {category.name, category.name[0]}:
            return category
    raise ValueError(f"Unknown category '{name}'. Use: length, weight, or temperature")


def format_value(value: float) -> str:
    if abs(value - round(value)) < 1e-9:
        return str(int(round(value)))
    return f"{value:.6g}"


def list_units(category_name: str) -> str:
    category = find_category(category_name)
    labels = [f"{unit.name} ({unit.aliases[0]})" for unit in category.units]
    return f"{category.name.title()} units: {', '.join(labels)}"


def parse_args(argv: list[str]) -> tuple[str, float, str, str]:
    if len(argv) != 5:
        raise ValueError(
            "Usage: python measurement.py <category> <value> <from_unit> <to_unit>"
        )

    category_name = argv[1]
    try:
        value = float(argv[2])
    except ValueError as exc:
        raise ValueError("Value must be a number") from exc

    return category_name, value, argv[3], argv[4]


def run_conversion(category_name: str, value: float, source_name: str, target_name: str) -> None:
    result = convert(category_name, value, source_name, target_name)
    print(f"{format_value(value)} {normalize(source_name)} = {format_value(result)} {normalize(target_name)}")


def interactive() -> None:
    print("Simple Measurement Converter")
    print("Categories: length, weight, temperature")
    print("Commands: convert, units, help, quit\n")

    while True:
        try:
            line = input("> ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break

        if not line:
            continue

        parts = line.split()
        command = parts[0].lower()

        if command in {"quit", "exit", "q"}:
            break

        if command == "help":
            print("convert <category> <value> <from> <to>")
            print("units <category>")
            print("quit")
            continue

        if command == "units":
            if len(parts) != 2:
                print("Usage: units <category>")
                continue
            try:
                print(list_units(parts[1]))
            except ValueError as err:
                print(err)
            continue

        if command == "convert":
            if len(parts) != 5:
                print("Usage: convert <category> <value> <from> <to>")
                continue
            try:
                run_conversion(parts[1], float(parts[2]), parts[3], parts[4])
            except ValueError as err:
                print(err)
            continue

        print("Unknown command. Type 'help'.")


def main() -> None:
    if len(sys.argv) > 1:
        try:
            category_name, value, source_name, target_name = parse_args(sys.argv)
            run_conversion(category_name, value, source_name, target_name)
        except ValueError as err:
            print(err, file=sys.stderr)
            sys.exit(1)
        return

    interactive()


if __name__ == "__main__":
    main()
