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

    void ret( tac::Return atac, std::vector<arm64_at::Instruction>& instructions );
    void unary( tac::Unary atac, std::vector<arm64_at::Instruction>& instructions );
    void binary( tac::Binary atac, std::vector<arm64_at::Instruction>& instructions );

    arm64_at::Operand        value( tac::Value atac );
    arm64_at::Operand        constant( tac::Constant atac );
    static arm64_at::Operand pseudo( tac::Variable atac );

    arm64_at::Register x0;
    arm64_at::Register xzr; // Zero register
};
