//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include "tac/includes.h"
#include "x86_at/includes.h"

/// Convert TAC Abstract tree to AT Assembly tree
class AssemblyGen {
  public:
    AssemblyGen();
    ~AssemblyGen() = default;

    x86_at::Program generate( const tac::Program atac );

    x86_at::FunctionDef functionDef( const tac::FunctionDef atac );

    void ret( const tac::Return atac, std::vector<x86_at::Instruction>& instructions );

    void unary( const tac::Unary atac, std::vector<x86_at::Instruction>& instructions );
    void unary_not( const tac::Unary atac, std::vector<x86_at::Instruction>& instructions );

    void binary( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions );
    void idiv( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions );
    void binary_relation( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions );
    void jump( const tac::Jump atac, std::vector<x86_at::Instruction>& instructions );
    template <typename T> void jumpIfZero( const T atac, bool zerop, std::vector<x86_at::Instruction>& instructions );
    void                       copy( const tac::Copy atac, std::vector<x86_at::Instruction>& instructions );
    void                       label( const tac::Label atac, std::vector<x86_at::Instruction>& instructions );
    void                       functionCall( const tac::FunCall atac, std::vector<x86_at::Instruction>& instructions );

    x86_at::Operand value( const tac::Value& atac );
    x86_at::Operand constant( const tac::Constant& atac );
    x86_at::Operand pseudo( tac::Variable atac );

  private:
    x86_at::Imm zero;

    x86_at::Register ax;
    x86_at::Register cx;
    x86_at::Register dx;
    x86_at::Register di;
    x86_at::Register si;
    x86_at::Register r8;
    x86_at::Register r9;

    std::vector<x86_at::Register> frame_registers;
};

template <typename T>
void AssemblyGen::jumpIfZero( const T atac, bool zerop, std::vector<x86_at::Instruction>& instructions ) {
    auto zero = std::make_shared<x86_at::Imm_>( atac->location, 0 );
    auto cmp = std::make_shared<x86_at::Cmp_>( atac->location, zero, value( atac->condition ) );
    instructions.push_back( cmp );
    if ( zerop ) {
        auto j = std::make_shared<x86_at::JumpCC_>( atac->location, x86_at::CondCode::E, atac->target );
        instructions.push_back( j );
    } else {
        auto j = std::make_shared<x86_at::JumpCC_>( atac->location, x86_at::CondCode::NE, atac->target );
        instructions.push_back( j );
    }
}
