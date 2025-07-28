//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/7/2025.
//

#pragma once

#include "option.h"
#include "tac/program.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

class CodeGenBase_ {
  public:
    CodeGenBase_() = default;
    virtual ~CodeGenBase_() = default;
};

using CodeGenBase = std::shared_ptr<CodeGenBase_>;

class CodeGenerator {
  public:
    CodeGenerator( Option const& option ) : option( option ) {};
    virtual ~CodeGenerator() = default;

    virtual CodeGenBase run_codegen( tac::Program tac ) = 0;
    virtual void        generate_output_file( CodeGenBase assembly ) = 0;

    std::string get_output();

  protected:
    virtual void generate( CodeGenBase program ) = 0;

    void make_output_file_name();

    void add_line( std::string line );
    void add_line( std::string instruct, std::string operands, int line_number = 0 );
    void add_line( std::string instruct, std::string operand1, std::string operand2, int line_number = 0 );

    Option const&         option;
    std::filesystem::path output;
    std::fstream          file;
    std::stringstream     text;

    std::string comment_prefix = "# ";
};

std::unique_ptr<CodeGenerator> make_CodeGen( Option const& option );