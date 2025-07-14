//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "at/includes.h"
#include "tac/includes.h"

/// Convert TAC Abstract tree to AT Assembly tree
class AssemblyGen {
  public:
    AssemblyGen() = default;
    ~AssemblyGen() = default;

    at::Program generate( const tac::Program& atac );

    at::FunctionDef functionDef( const tac::FunctionDef& atac );
    void            ret( const tac::Return& atac, std::vector<at::Instruction>& instructions );
    void            unary( const tac::Unary& atac, std::vector<at::Instruction>& instructions );
    void            binary( const tac::Binary& atac, std::vector<at::Instruction>& instructions );

    at::Operand value(const tac::Value& atac);
    at::Operand constant( const tac::Constant& atac );
    at::Operand pseudo( tac::Variable atac );
};
