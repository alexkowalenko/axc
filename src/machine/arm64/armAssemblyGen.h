//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/program.h"
#include "tac/includes.h"

class ARMAssemblyGen {
  public:
    ARMAssemblyGen();
    ~ARMAssemblyGen() = default;

    arm64_at::Program generate( const tac::Program atac );

  private:
    arm64_at::FunctionDef function( const tac::FunctionDef& atac );

    void ret( const tac::Return atac, std::vector<arm64_at::Instruction>& instructions );
    void unary( const tac::Unary atac, std::vector<arm64_at::Instruction>& instructions );

    arm64_at::Operand value( const tac::Value& atac );
    arm64_at::Operand constant( const tac::Constant& atac );
    arm64_at::Operand pseudo( tac::Variable atac );

    arm64_at::Register x0;
    arm64_at::Register xzr; // Zero register
};
