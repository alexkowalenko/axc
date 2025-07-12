//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/7/2025.
//

#pragma once

#include "at/visitor.h"
#include "option.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

class CodeGenerator {
  public:
    CodeGenerator( Option const& option ) : option( option ) {};
    virtual ~CodeGenerator() = default;

    virtual void generate( at::Program program ) = 0;

    std::string get_output();

  protected:
    void make_output_file_name();

    void add_line( std::string line);
    void add_line( std::string instruct, std::string operands, int line_number = 0);

    Option const&         option;
    std::filesystem::path output;
    std::fstream          file;
    std::stringstream     text;
};

std::unique_ptr<CodeGenerator> make_CodeGen( Option const& option );