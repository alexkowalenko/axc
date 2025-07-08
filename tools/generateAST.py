#!/usr/bin/env python3

import os
import sys

import jinja2

template_file = "ast_template.h"
template_visit_file = "visitor_template.h"
includes_file = "includes.h"

def define_ast(output_dir, types):
    """Generate AST class definitions using specification and jinja2"""

    print(types)
    # type_items = sorted(types.items(), key=lambda i: i[0])

    # open template base file
    with open(template_file) as f:
        template = jinja2.Template(f.read())

    for class_name, members in types.items():
        print(class_name)
        print(members)
        file_name = class_name.lower()

        with open(os.path.join(output_dir, "{}.h".format(file_name)), 'w') as f:
            f.write(template.render(base_name=class_name, members=members))

    # open visitor template
    with open(template_visit_file) as f:
        visitor = jinja2.Template(f.read())

    with open(os.path.join(output_dir, "visitor.h"), 'w') as fv:
            fv.write(visitor.render(type_items=types.items()))

    # include file 
    with open(os.path.join(output_dir, includes_file), 'w') as f:
        for class_name, members in types.items():
             file_name = class_name.lower()
             f.write(f'#include "{file_name}.h"\n')


if __name__ == "__main__":

    try:
        output_dir = sys.argv[1]
    except IndexError:
        print("Usage: python {} <output directory>".format(sys.argv[0]))
        sys.exit(1)

    define_ast(
        output_dir,
        {
           "Program": [("FunctionDef", "function", False)],
           "FunctionDef": [("std::string", "name", False), ("Statement", "statement", False)],
           "Statement": [("Return", "ret", False)],
           "Return": [("Expr", "expr", False)],
           "Expr": [("Constant", "constant", False)],
           "Constant": [("std::int32_t", "value", False)],
         })
