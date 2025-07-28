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
            "Program": [],
        })