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
        "ast",
        {
            "Program": [("FunctionDef", "function", False)],
            "FunctionDef": [("std::string", "name", False), ("std::vector<BlockItem>", "block_items", False)],
            "Declaration": [("std::string", "name", False), ("std::optional<Expr>", "init", False)],
            "Null": [], # Null statement
            "Return": [("Expr", "expr", False)],
            "UnaryOp": [("TokenType", "op", False), ("Expr", "operand", False)],
            "BinaryOp": [("TokenType", "op", False), ("Expr", "left", False), ("Expr", "right", False)],
            "Assign": [("Expr", "left", False), ("Expr", "right", False)],
            "Var": [("std::string", "name", False)],
            "Constant": [("std::int32_t", "value", False)],
         })
