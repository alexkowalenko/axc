//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/7/2025.
//

#include "codeGen.h"
#include "exception.h"
#include "machine/x86_64CodeGen.h"

std::unique_ptr<CodeGenerator> make_CodeGen( Option const& option ) {
    switch ( option.machine ) {
    case Machine::X86_64 :
        return std::make_unique<X86_64CodeGen>( option );
    case Machine::AArch64 :
    default :
        throw Exception( "Unsupported machine" );
    }
}

void CodeGenerator::make_output_file_name() {
    output = option.input_file;
    output.replace_extension( ".s" );
}

void CodeGenerator::add_line( std::string line ) {
    file << line << '\n';
    text << line << '\n';
}

void CodeGenerator::add_line( std::string instruct, std::string operands, int line_number ) {
    std::string line;
    if ( line_number > 0 ) {
        line = std::format( "\t{}\t{}\t\t\t\t # line {}", instruct, operands, line_number );
    } else {
        line = std::format( "\t{}\t{}", instruct, operands );
    }
    add_line( line );
}

std::string CodeGenerator::get_output() {
    return text.str();
}