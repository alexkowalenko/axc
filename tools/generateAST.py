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
            "Program": [("std::vector<Declaration>", "declarations")],
            "FunctionDef": [("std::string", "name"), ("std::vector<std::string>", "params"), ("std::optional<Compound>", "block"), ("StorageClass", "storage")],
            "VariableDef": [("std::string", "name"), ("std::optional<Expr>", "init"), ("StorageClass", "storage")],
            "Statement": [("std::optional<Label>", "label"), ("std::optional<StatementItem>", "statement")],
            "Null": [], # Null statement
            "Return": [("Expr", "expr")],
            "If": [("Expr", "condition"), ("Statement", "then"), ("std::optional<Statement>", "else_stat")],
            "Goto": [("std::string", "label")],
            "Label": [("std::string", "label")],
            "Break": [],  # Break statement
            "Continue": [],  # Continue statement
            "While": [("Expr", "condition"), ("Statement", "body")],
            "DoWhile": [("Statement", "body"), ("Expr", "condition")],
            "For": [("std::optional<ForInit>", "init"), ("std::optional<Expr>", "condition"), ("std::optional<Expr>", "increment"), ("Statement", "body")],
            "Switch": [("Expr", "condition"), ("Statement", "body"), ("std::vector<Case>", "cases")],
            "Case": [("Expr", "value"), ("std::vector<BlockItem>", "block_items"), ("bool", "is_default")],
            "Compound": [("std::vector<BlockItem>", "block_items")],
            "UnaryOp": [("TokenType", "op"), ("Expr", "operand")],
            "BinaryOp": [("TokenType", "op"), ("Expr", "left"), ("Expr", "right")],
            "PostOp": [("TokenType", "op"), ("Expr", "operand")],
            "Conditional": [("Expr", "condition"), ("Expr", "then_expr"), ("Expr", "else_expr")],
            "Assign": [("TokenType", "op"), ("Expr", "left"), ("Expr", "right")],
            "Call": [("std::string", "function_name"), ("std::vector<Expr>", "arguments")],
            "Var": [("std::string", "name")],
            "Constant": [("std::int32_t", "value")],
         },
        {
            "BlockItem": ["Statement", "VariableDef", "FunctionDef"],
            "Declaration": ["VariableDef", "FunctionDef"],
        })
