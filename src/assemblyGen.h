//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "ast/includes.h"
#include "at/includes.h"

class AssemblyGen {
  public:
    AssemblyGen() = default;
    ~AssemblyGen() = default;

    at::Program generate( const ast::Program& ast );

    at::FunctionDef functionDef( const ast::FunctionDef& ast );
    void            statement( const ast::Statement& ast, std::vector<at::Instruction>& instructions );
    void            ret( const ast::Return& ast, std::vector<at::Instruction>& instructions );
    void            expr( const ast::Expr& ast, std::vector<at::Instruction>& instructions );
    at::Imm         constant( const ast::Constant& ast );
};
