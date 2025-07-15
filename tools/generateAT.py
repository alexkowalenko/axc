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
        "at",
        {
           "Program": [("FunctionDef", "function", False)],
           "FunctionDef": [("std::string", "name", False), ("std::vector<Instruction>", "instructions", False)],
            # Operations for Instructions
           "Mov": [("Operand", "src", False), ("Operand", "dst", False) ],
           "Unary": [("UnaryOpType", "op", False), ("Operand", "operand", False) ],
           "Binary": [("BinaryOpType", "op", False), ("Operand", "operand1", False), ("Operand", "operand2", False) ],
           "Idiv": [("Operand", "src", False) ],
           "Cdq": [],
           "AllocateStack": [("std::int32_t", "size", False)],
           "Ret": [],
            # 4 Operand types for Operand
           "Imm": [("std::int32_t", "value", False)],
           "Register": [("std::string", "reg", False)],
           "Pseudo": [("std::string", "name", False)],
           "Stack": [("std::int32_t", "offset", False)],
         })
