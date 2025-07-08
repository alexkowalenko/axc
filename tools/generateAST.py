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
           "FunctionDef": [("std::string", "name", False), ("Statement", "statement", False)],
           "Statement": [("Return", "ret", False)],
           "Return": [("Expr", "expr", False)],
           "Expr": [("Constant", "constant", False)],
           "Constant": [("std::int32_t", "value", False)],
         })
