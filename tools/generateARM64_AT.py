#!/usr/bin/env python3
#
#  AXC - C Compiler
#
#  Copyright (c) 2025.
#

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
        "arm64_at",
        {
            "Program": [("FunctionDef", "function")],
            "FunctionDef": [("std::string", "name"), ("std::vector<Instruction>", "instructions"), ("std::int32_t", "stack_size")],
            # Operations for Instructions
            "Mov": [("Operand", "src"), ("Operand", "dst") ],
            "Load": [("Operand", "src"), ("Operand", "dst") ],
            "Store": [("Operand", "src"), ("Operand", "dst") ],
            "Unary": [("UnaryOpType", "op"), ("Operand", "dst"),  ("Operand", "src") ],
            "Binary": [("BinaryOpType", "op"), ("Operand", "dst"), ("Operand", "src1"), ("Operand", "src2")],
            "AllocateStack": [("std::int32_t", "size")],
            "DeallocateStack": [("std::int32_t", "size")],
            "Branch": [("std::string", "target")],
            "BranchCC": [("CondCode", "condition"), ("std::string", "target")],
            "Label": [("std::string", "name")],
            "Cmp": [("Operand", "operand1"), ("Operand", "operand2")],
            "Cset": [("Operand", "operand"), ("CondCode", "cond")],
            "Ret": [],
            # Operand types for Operand
            "Imm": [("std::int32_t", "value")],
            "Register": [("RegisterName", "reg")],
            "Pseudo": [("std::string", "name")],
            "Stack": [("std::int32_t", "offset")],
        },
        {
           "Instruction": ["Mov", "Load", "Store", "Ret", "Unary", "Binary", "AllocateStack", "DeallocateStack", "Branch", "BranchCC", "Label", "Cmp", "Cset"],
           "Operand":  ["Imm", "Register", "Pseudo", "Stack"],
        })