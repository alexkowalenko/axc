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
            "Program": [("std::vector<TopLevel>", "top_level")],
            "FunctionDef": [("std::string", "name"), ("std::vector<std::string>", "params"), ("std::vector<Instruction>", "instructions"),  ("bool", "global")],
            "StaticVariable": [("std::string", "name"), ("bool", "global"), ("Type", "type"), ("int", "init")],
            "Return": [("Value", "value") ],
            "Unary": [("UnaryOpType", "op"), ("Value", "src"), ("Value", "dst")],
            "Binary": [("BinaryOpType", "op"), ("Value", "src1"), ("Value", "src2"), ("Value", "dst")],
            "Copy" : [("Value", "src"), ("Value", "dst")],
            "Jump": [("std::string", "target")],
            "JumpIfZero": [("Value", "condition"), ("std::string", "target")],
            "JumpIfNotZero": [("Value", "condition"), ("std::string", "target")],
            "Label": [("std::string", "name")],
            "FunCall": [("std::string", "function_name"), ("std::vector<Value>", "arguments"), ("Value", "dst"), ("bool", "external")],
            "SignExtend": [("Value", "src"), ("Value", "dst")],
            "Truncate": [("Value", "src"), ("Value", "dst")],
            "ConstantInt": [("std::int32_t", "value")],
            "ConstantLong": [("std::int64_t", "value")],
            "Variable": [("std::string", "name"), ("Type", "type")],
         },
        {
            "TopLevel": ["FunctionDef", "StaticVariable"],
            "Instruction":  ["Return", "Unary", "Binary", "Copy", "Jump", "JumpIfZero", "JumpIfNotZero", "Label", "FunCall", "SignExtend", "Truncate"],
            "Value": ["ConstantInt", "ConstantLong", "Variable"],
        })
