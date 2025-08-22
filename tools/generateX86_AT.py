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
        "x86_at",
        {
            "Program": [("std::vector<FunctionDef>", "functions", False)],
            "FunctionDef": [("std::string", "name", False), ("std::vector<Instruction>", "instructions", False), ("std::int32_t", "stack_size", False), ("bool", "global", False)],
            # Operations for Instructions
            "Mov": [("Operand", "src", False), ("Operand", "dst", False) ],
            "Unary": [("UnaryOpType", "op", False), ("Operand", "operand", False) ],
            "Binary": [("BinaryOpType", "op", False), ("Operand", "operand1", False), ("Operand", "operand2", False) ],
            "Cmp": [("Operand", "operand1", False), ("Operand", "operand2", False)],
            "Idiv": [("Operand", "src", False) ],
            "Cdq": [],
            "Jump": [("std::string", "target", False)],
            "JumpCC": [("CondCode", "cond", False), ("std::string", "target", False)],
            "SetCC": [("CondCode", "cond", False), ("Operand", "operand", False)],
            "Label": [("std::string", "name", False)],
            "AllocateStack": [("std::int32_t", "size", False)],
            "DeallocateStack": [("std::int32_t", "size", False)],
            "Push": [("Operand", "operand", False)],
            "Call": [("std::string", "function_name", False)],
            "Ret": [("std::optional<Operand>", "value", True)],
            # Operand types
            "Ret": [],
            # 4 Operand types for Operand
            "Imm": [("std::int32_t", "value", False)],
            "Register": [("RegisterName", "reg", False), ("RegisterSize", "size", False)],
            "Pseudo": [("std::string", "name", False)],
            "Stack": [("std::int32_t", "offset", False)],
         })
