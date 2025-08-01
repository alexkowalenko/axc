#!/usr/bin/env python3
#
#  AXC - C Compiler
#
#  Copyright (c) 2025.
#

import argparse
import tempfile
import os
import subprocess
import sys
import platform

clang_path = "clang"
axc_path = "axc_comp"
def main():
    global axc_path
    app = argparse.ArgumentParser(description="AXC C Compiler")
    app.add_argument('-l', '--lex', help='run only the lexer.', action='store_true')
    app.add_argument('-p', '--parse', help='run the lexer and parser.', action='store_true')
    app.add_argument('-v', '--validate', help='run the lexer, parser, and semantic analyser', action='store_true')
    app.add_argument('-t', '--tacky', help='run the lexer, parser, semantic and tac generator', action='store_true')
    app.add_argument('-g', '--codegen', help='run the lexer, parser, semantic, tac and code generator, no output.',action='store_true')
    app.add_argument('-s', '--silent', help='silent operation (no logging)', action='store_true')
    app.add_argument('-m', '--machine', help='machine Architecture.', choices=["x86_64", "amd64", "aarch64", "arm64"] )
    app.add_argument('-c', help='Produce object file only.', action='store_true')
    app.add_argument('filename', help='File to be compile.')
    args = app.parse_args()

    file_name = args.filename
    file_name_base = file_name.split('.')[0]

    script_dir = os.path.dirname(os.path.abspath(__file__))
    axc_path = script_dir + "/" + axc_path

    # Run the preprocessor on the file
    temp_file = tempfile.NamedTemporaryFile(delete=False)
    cmd = f"{clang_path} -E -P {file_name} -o {temp_file.name}.s"
    print(cmd)
    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        print("Error running clang preprocessor")
        sys.exit(result.returncode)

    # Run the AXC compiler
    options = ""
    if args.lex:
        options += " -l"
    if args.parse:
        options += " -p"
    if args.validate:
        options += " -v"
    if args.tacky:
        options += " -t"
    if args.codegen:
        options += " -c"
    if args.silent:
        options += " -s"
    if args.machine:
        options += f" -m {args.machine}"
    cmd = f"{axc_path} {options} {temp_file.name}.s"
    print(cmd)
    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        print("Failed to run AXC compiler")
        sys.exit(result.returncode)

    # Exit not generating output
    if(args.lex or args.parse or args.validate or args.tacky or args.codegen):
        sys.exit(0)

    # Assemble the file
    target = ""
    if platform.system() == "Darwin":
        if args.machine == "x86_64" or args.machine == "amd64":
            target = "--target=x86_64-apple-darwin"
        elif args.machine == "aarch64" or args.machine == "arm64":
            target = "--target=arm64-apple-darwin"
        else:
            target = "--target=x86_64-apple-darwin"

    # Full compile or just binary
    if args.c:
        cmd = f"{clang_path} {target} {temp_file.name}.s -c -o {file_name_base}.o"
    else:
        cmd = f"{clang_path} {target} {temp_file.name}.s -o {file_name_base}"

    # Running the command.
    print(cmd)
    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        print("Failed to assemble the file and link the executable.")
        sys.exit(result.returncode)
    sys.exit(0)

if __name__ == "__main__":
    main()

