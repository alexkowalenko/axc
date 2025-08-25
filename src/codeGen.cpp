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
#include "machine/arm64/arm64CodeGen.h"
#include "machine/x86_64/x86_64CodeGen.h"

std::unique_ptr<CodeGenerator> make_CodeGen( Option const& option, SymbolTable& symbol_table ) {
    switch ( option.machine ) {
    case Machine::X86_64 :
        return std::make_unique<X86_64CodeGen>( option, symbol_table );
    case Machine::AArch64 :
        return std::make_unique<Arm64CodeGen>( option, symbol_table );
    default :
        throw CodeException( "Unsupported machine" );
    }
}

void CodeGenerator::make_output_file_name() {
    output = option.input_file;
    output.replace_extension( ".s" );
}

void CodeGenerator::add_line( const std::string& line ) {
    file << line << '\n';
    text << line << '\n';
}

void CodeGenerator::add_line( std::string const& instruct, std::string const& operands, int line_number ) {
    std::string line;
    if ( line_number > 0 ) {
        line = std::format( "\t{}\t{}\t\t\t\t {} line {}", instruct, operands, comment_prefix, line_number );
    } else {
        line = std::format( "\t{}\t{}", instruct, operands );
    }
    add_line( line );
}

void CodeGenerator::add_line( std::string const& instruct, std::string const& operand1, std::string const& operand2,
                              int line_number ) {
    std::string line;
    if ( line_number > 0 ) {
        line =
            std::format( "\t{}\t{}, {}\t\t\t\t {} line {}", instruct, operand1, operand2, comment_prefix, line_number );
    } else {
        line = std::format( "\t{}\t{}, {}", instruct, operand1, operand2 );
    }
    add_line( line );
}

void CodeGenerator::add_line( std::string const& instruct, std::string const& operand1, std::string const& operand2,
                              std::string const& operand3, int line_number ) {
    std::string line;
    if ( line_number > 0 ) {
        line = std::format( "\t{}\t{}, {}, {}\t\t\t\t {} line {}", instruct, operand1, operand2, operand3,
                            comment_prefix, line_number );
    } else {
        line = std::format( "\t{}\t{}, {}, {}", instruct, operand1, operand2, operand3 );
    }
    add_line( line );
}

std::string CodeGenerator::get_output() const {
    return text.str();
}