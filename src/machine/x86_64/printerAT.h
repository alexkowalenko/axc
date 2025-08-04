//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include <string>

#include "x86_at/base.h"
#include "x86_at/visitor.h"

class PrinterAT : public x86_at::Visitor<std::string> {
  public:
    PrinterAT() = default;
    ~PrinterAT() override = default;

    std::string print( const x86_at::Program ast );

    std::string visit_Program( const x86_at::Program ast ) override;
    std::string visit_FunctionDef( const x86_at::FunctionDef ast ) override;
    std::string visit_Mov( const x86_at::Mov ast ) override;
    std::string visit_Imm( const x86_at::Imm ast ) override;
    std::string visit_Unary( const x86_at::Unary ast ) override;
    std::string visit_AllocateStack( const x86_at::AllocateStack ast ) override;
    std::string visit_DeallocateStack( const x86_at::DeallocateStack ast ) override;
    std::string visit_Push( const x86_at::Push ast ) override;
    std::string visit_Call( const x86_at::Call ast ) override;
    std::string visit_Binary( const x86_at::Binary ast ) override;
    std::string visit_Idiv( const x86_at::Idiv ast ) override;
    std::string visit_Cdq( const x86_at::Cdq ast ) override;
    std::string visit_Cmp( const x86_at::Cmp ast ) override;
    std::string visit_Jump( const x86_at::Jump ast ) override;
    std::string visit_JumpCC( const x86_at::JumpCC ast ) override;
    std::string visit_SetCC( const x86_at::SetCC ast ) override;
    std::string visit_Label( const x86_at::Label ast ) override;

    std::string visit_Register( const x86_at::Register ast ) override;
    std::string visit_Ret( const x86_at::Ret ast ) override;
    std::string visit_Pseudo( const x86_at::Pseudo ast ) override;
    std::string visit_Stack( const x86_at::Stack ast ) override;

    std::string indent { "  " };

  private:
    std::string operand( const x86_at::Operand& op );
};
