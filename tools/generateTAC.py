#!/usr/bin/env python3

import sys
from common import define_ast

if __name__ == "__main__":
    try:
        output_dir = sys.argv[1]
    except IndexError:
        print("Usage: python {} <output directory>".format(sys.argv[0]))
        sys.exit(1)

    define_ast(
        output_dir,
        "tac",
        {
           "Program": [("FunctionDef", "function", False)],
           "FunctionDef": [("std::string", "name", False), ("std::vector<Instruction>", "instructions", False)],
           "Return": [("Value", "value", False) ],
           "Unary": [("TokenType", "op", False), ("Value", "src", False), ("Value", "dst", False)],
           "Constant": [("std::int32_t", "value", False)],
           "Variable": [("std::string", "name", False)],
         })
