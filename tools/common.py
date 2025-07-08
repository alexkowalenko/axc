#
#  AXC - C Compiler
#
#  Copyright (c) 2025.
#

import os

import jinja2

template_file = "class_template.h"
template_visit_file = "visitor_template.h"
includes_file = "includes.h"

def define_ast(output_dir, namespace_name, types):
    """Generate AST class definitions using specification and jinja2"""

    # type_items = sorted(types.items(), key=lambda i: i[0])

    # open template base file
    with open(template_file) as f:
        template = jinja2.Template(f.read())

    for class_name, members in types.items():
        print(class_name)
        print(members)
        file_name = class_name.lower()

        with open(os.path.join(output_dir, "{}.h".format(file_name)), 'w') as f:
            f.write(template.render(base_name=class_name, members=members, namespace=namespace_name))

    # open visitor template
    with open(template_visit_file) as f:
        visitor = jinja2.Template(f.read())

    with open(os.path.join(output_dir, "visitor.h"), 'w') as fv:
        fv.write(visitor.render(type_items=types.items(), namespace=namespace_name))

    # include file
    with open(os.path.join(output_dir, includes_file), 'w') as f:
        for class_name, members in types.items():
            file_name = class_name.lower()
            f.write(f'#include "{file_name}.h"\n')
