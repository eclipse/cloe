#!/usr/bin/env python3

import random
import string
import argparse

SIMPLE_TYPES = [
    "bool",
    "uint8_t",
    "uint16_t",
    "uint32_t",
    "uint64_t",
    "int8_t",
    "int16_t",
    "int32_t",
    "int64_t",
    "std::string",
]

COMPLEX_TYPES = [
    "std::vector",
    "std::map",
    "boost::optional",
    # "std::array",  # not supported yet
]

ALL_TYPES = SIMPLE_TYPES + COMPLEX_TYPES

def generate_random_type():
    def _tvector():
        inner = random.choice(SIMPLE_TYPES)
        return f"std::vector<{inner}>"

    def _tmap():
        inner = random.choice(SIMPLE_TYPES)
        return f"std::map<std::string, {inner}>"

    def _toptional():
        inner = random.choice(SIMPLE_TYPES)
        return f"boost::optional<{inner}>"

    def _tarray():
        inner = random.choice(SIMPLE_TYPES)
        length = random.randint(2, 24)
        return f"std::array<{inner}, {length}>"

    mapper = {
        "std::vector": _tvector,
        "std::map": _tmap,
        "boost::optional": _toptional,
    }

    result = random.choice(ALL_TYPES)
    if result in COMPLEX_TYPES:
        return mapper[result]()
    return result

def generate_random_word():
    letters = string.ascii_letters
    return ''.join(random.choice(letters) for _ in range(20))

TEMPLATE = string.Template("""
#pragma once

struct Large : public fable::Confable {
    $variable_lines

    CONFABLE_SCHEMA(Large) {
        using namespace fable;
        return Schema{
            $schema_lines
        };
    }
};
""")

def generate(count) -> str:
    variable_lines = []
    schema_lines = []
    for i in range(count):
        random_word = generate_random_word()
        variable_lines.append(f"{generate_random_type()} v{i};")
        schema_lines.append('{{"{0}", make_schema(&v{1}, "")}},'.format(random_word, i))
    result = TEMPLATE.substitute({
        "variable_lines": "\n    ".join(variable_lines),
        "schema_lines": "\n            ".join(schema_lines),
    })
    return result

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("count", default=10, type=int)
    parser.add_argument("output", default="-", type=str)
    args = parser.parse_args()

    result = generate(args.count)
    if args.output == "-":
        print(result)
    else:
        with open(args.output, "w") as file:
            file.write(result)

if __name__ == "__main__":
    main()
