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
    AssemblyGen( Option const& option );
    ~AssemblyGen() = default;

    x86_at::Program generate( tac::Program atac );

    x86_at::FunctionDef functionDef( tac::FunctionDef atac );

    void ret( tac::Return atac, std::vector<x86_at::Instruction>& instructions ) const;

    void unary( tac::Unary atac, std::vector<x86_at::Instruction>& instructions ) const;
    void unary_not( tac::Unary atac, std::vector<x86_at::Instruction>& instructions ) const;

    void        binary( tac::Binary atac, std::vector<x86_at::Instruction>& instructions );
    void        idiv( tac::Binary atac, std::vector<x86_at::Instruction>& instructions );
    void        binary_relation( tac::Binary atac, std::vector<x86_at::Instruction>& instructions ) const;
    static void jump( tac::Jump atac, std::vector<x86_at::Instruction>& instructions );
    template <typename T> void jumpIfZero( T atac, bool zerop, std::vector<x86_at::Instruction>& instructions );
    static void                copy( tac::Copy atac, std::vector<x86_at::Instruction>& instructions );
    static void                label( tac::Label atac, std::vector<x86_at::Instruction>& instructions );
    void                       functionCall( tac::FunCall atac, std::vector<x86_at::Instruction>& instructions ) const;
    void staticVariable( tac::StaticVariable atac, std::vector<x86_at::Instruction>& instructions );

    static x86_at::Operand value( tac::Value atac );
    static x86_at::Operand constant( tac::Constant atac );
    static x86_at::Operand pseudo( tac::Variable atac );

  private:
    Option const& option;

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
