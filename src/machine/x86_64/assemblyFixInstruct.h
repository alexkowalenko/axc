//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#pragma once

#include "x86_at/base.h"
#include "x86_at/visitor.h"

class AssemblyFixInstruct : public x86_at::Visitor<void> {
  public:
    AssemblyFixInstruct();
    ~AssemblyFixInstruct() override = default;

    void filter( x86_at::Program program );

  public:
    void visit_Program( x86_at::Program ast ) override;
    void visit_FunctionDef( x86_at::FunctionDef ast ) override;

    void visit_Mov( x86_at::Mov ast ) override;
    void visit_Unary( x86_at::Unary ast ) override {};
    void visit_AllocateStack( x86_at::AllocateStack ast ) override;
    void visit_DeallocateStack( x86_at::DeallocateStack ast ) override;
    void visit_Push( x86_at::Push ast ) override;
    void visit_Call( x86_at::Call ast ) override;
    void visit_Ret( x86_at::Ret ast ) override {};
    void visit_Imm( x86_at::Imm ast ) override {};
    void visit_Binary( x86_at::Binary ast ) override;
    void visit_Idiv( x86_at::Idiv ast ) override;
    void visit_Cdq( x86_at::Cdq ast ) override {};
    void visit_Cmp( x86_at::Cmp ast ) override;
    void visit_Jump( x86_at::Jump ast ) override {};
    void visit_JumpCC( x86_at::JumpCC ast ) override {};
    void visit_SetCC( x86_at::SetCC ast ) override {};
    void visit_Label( x86_at::Label ast ) override {};

    void visit_Register( x86_at::Register ast ) override {};
    void visit_Pseudo( x86_at::Pseudo ast ) override {};
    void visit_Stack( x86_at::Stack ast ) override {};

  private:
    static constexpr int             stack_increment { 4 };
    std::vector<x86_at::Instruction> current_instructions;

    x86_at::Register ax;
    x86_at::Register cx;
    x86_at::Register cl;
    x86_at::Register dx;
    x86_at::Register r10;
    x86_at::Register r11;
};