//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/includes.h"
#include "tac/includes.h"

class ARMAssemblyGen {
  public:
    ARMAssemblyGen();
    ~ARMAssemblyGen() = default;

    arm64_at::Program generate( tac::Program atac );

  private:
    arm64_at::FunctionDef function( tac::FunctionDef atac );

    void                       ret( tac::Return atac, std::vector<arm64_at::Instruction>& instructions );
    void                       unary( tac::Unary atac, std::vector<arm64_at::Instruction>& instructions );
    void                       binary( tac::Binary atac, std::vector<arm64_at::Instruction>& instructions );
    void                       binary_compare( tac::Binary atac, std::vector<arm64_at::Instruction>& instructions );
    void                       copy( tac::Copy atac, std::vector<arm64_at::Instruction>& instructions );
    void                       branch( tac::Jump atac, std::vector<arm64_at::Instruction>& instructions );
    template <typename T> void branchIfZero( T atac, bool zerop, std::vector<arm64_at::Instruction>& instructions );
    void                       label( tac::Label atac, std::vector<arm64_at::Instruction>& instructions );

    arm64_at::Operand        value( tac::Value atac );
    arm64_at::Operand        constant( std::int64_t value );
    static arm64_at::Operand pseudo( tac::Variable atac );

    arm64_at::Register x0;
    arm64_at::Register xzr; // Zero register
};

template <typename T>
void ARMAssemblyGen::branchIfZero( const T atac, bool zerop, std::vector<arm64_at::Instruction>& instructions ) {
    auto cmp = std::make_shared<arm64_at::Cmp_>( atac->location, xzr, value( atac->condition ) );
    instructions.push_back( cmp );
    if ( zerop ) {
        auto j = std::make_shared<arm64_at::BranchCC_>( atac->location, arm64_at::CondCode::EQ, atac->target );
        instructions.push_back( j );
    } else {
        auto j = std::make_shared<arm64_at::BranchCC_>( atac->location, arm64_at::CondCode::NE, atac->target );
        instructions.push_back( j );
    }
}
