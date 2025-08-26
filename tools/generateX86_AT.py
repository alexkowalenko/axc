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
            "Program": [("std::vector<TopLevel>", "top_level")],
            "FunctionDef": [("std::string", "name"), ("std::vector<Instruction>", "instructions"), ("std::int32_t", "stack_size"), ("bool", "global")],
            "StaticVariable": [("std::string", "name"), ("bool", "global"), ("int", "init")],
            # Operations for Instructions
            "Mov": [("Operand", "src"), ("Operand", "dst") ],
            "Unary": [("UnaryOpType", "op"), ("Operand", "operand") ],
            "Binary": [("BinaryOpType", "op"), ("Operand", "operand1"), ("Operand", "operand2") ],
            "Cmp": [("Operand", "operand1"), ("Operand", "operand2")],
            "Idiv": [("Operand", "src") ],
            "Cdq": [],
            "Jump": [("std::string", "target")],
            "JumpCC": [("CondCode", "cond"), ("std::string", "target")],
            "SetCC": [("CondCode", "cond"), ("Operand", "operand")],
            "Label": [("std::string", "name")],
            "AllocateStack": [("std::int32_t", "size")],
            "DeallocateStack": [("std::int32_t", "size")],
            "Push": [("Operand", "operand")],
            "Call": [("std::string", "function_name")],
            "Ret": [("std::optional<Operand>", "value")],
            # Operand types
            "Ret": [],
            # Operand types for Operand
            "Imm": [("std::int32_t", "value")],
            "Register": [("RegisterName", "reg"), ("RegisterSize", "size")],
            "Pseudo": [("std::string", "name")],
            "Stack": [("std::int32_t", "offset")],
            "Data": [("std::string", "name")],
         },
        {
            "TopLevel": ["FunctionDef", "StaticVariable"],
            "Instruction": ["Mov", "Unary", "Binary", "Cmp", "Idiv", "Cdq", "Jump", "JumpCC", "SetCC", "Label", "AllocateStack","DeallocateStack", "Push", "Call", "Ret"],
            "Operand": ["Imm", "Register", "Pseudo", "Stack", "Data"],
        })
