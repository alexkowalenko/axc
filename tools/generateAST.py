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
            "FunctionDef": [("std::string", "name", False), ("Compound", "block", False)],
            "Declaration": [("std::string", "name", False), ("std::optional<Expr>", "init", False)],
            "Null": [], # Null statement
            "Return": [("Expr", "expr", False)],
            "If": [("Expr", "condition", False), ("Statement", "then", False), ("std::optional<Statement>", "else_stat", False)],
            "Goto": [("std::string", "label", False)],
            "Label": [("std::string", "label", False)],
            "Break": [],  # Break statement
            "Continue": [],  # Continue statement
            "While": [("Expr", "condition", False), ("Statement", "body", False)],
            "DoWhile": [("Statement", "body", False), ("Expr", "condition", False)],
            "For": [("std::optional<ForInit>", "init", False), ("std::optional<Expr>", "condition", False), ("std::optional<Expr>", "increment", False), ("Statement", "body", False)],
            "Switch": [("Expr", "condition", False), ("Statement", "body", False)],
            "Case": [("Expr", "value", False), ("std::vector<BlockItem>", "block_items", False), ("bool", "is_default", False)],
            "Compound": [("std::vector<BlockItem>", "block_items", False)],
            "UnaryOp": [("TokenType", "op", False), ("Expr", "operand", False)],
            "BinaryOp": [("TokenType", "op", False), ("Expr", "left", False), ("Expr", "right", False)],
            "PostOp": [("TokenType", "op", False), ("Expr", "operand", False)],
            "Conditional": [("Expr", "condition", False), ("Expr", "then_expr", False), ("Expr", "else_expr", False)],
            "Assign": [("TokenType", "op", False), ("Expr", "left", False), ("Expr", "right", False)],
            "Var": [("std::string", "name", False)],
            "Constant": [("std::int32_t", "value", False)],
         })
