//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 14/8/2025.
//

#pragma once

#include "arm64_at/base.h"
#include "arm64_at/visitor.h"

#include <vector>

class FixInstruct : public arm64_at::Visitor<void> {
  public:
    FixInstruct();

    void filter( arm64_at::Program program );

    void visit_Program( arm64_at::Program ast ) override;
    void visit_FunctionDef( arm64_at::FunctionDef ast ) override;
    void visit_Mov( arm64_at::Mov ast ) override;
    void visit_Load( arm64_at::Load ast ) override;
    void visit_Store( arm64_at::Store ast ) override;
    void visit_Unary( arm64_at::Unary ast ) override;
    void visit_AllocateStack( arm64_at::AllocateStack ast ) override;
    void visit_DeallocateStack( arm64_at::DeallocateStack ast ) override;
    void visit_Ret( arm64_at::Ret ast ) override;
    void visit_Imm( arm64_at::Imm ast ) override;
    void visit_Register( arm64_at::Register ast ) override;
    void visit_Pseudo( arm64_at::Pseudo ast ) override;
    void visit_Stack( arm64_at::Stack ast ) override;

  private:
    std::vector<arm64_at::Instruction> current_instructions;

    arm64_at::Register x9;
    arm64_at::Register x10;
};
