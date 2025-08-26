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

#include "x86_at/includes.h"
#include "x86_at/visitor.h"

class PrinterX86 : public x86_at::Visitor<std::string> {
  public:
    PrinterX86() = default;
    ~PrinterX86() override = default;

    std::string print( x86_at::Program ast );

    std::string visit_Program( x86_at::Program ast ) override;
    std::string visit_FunctionDef( x86_at::FunctionDef ast ) override;
    std::string visit_StaticVariable( x86_at::StaticVariable ast ) override;
    std::string visit_Mov( x86_at::Mov ast ) override;
    std::string visit_Imm( x86_at::Imm ast ) override;
    std::string visit_Unary( x86_at::Unary ast ) override;
    std::string visit_AllocateStack( x86_at::AllocateStack ast ) override;
    std::string visit_DeallocateStack( x86_at::DeallocateStack ast ) override;
    std::string visit_Push( x86_at::Push ast ) override;
    std::string visit_Call( x86_at::Call ast ) override;
    std::string visit_Binary( x86_at::Binary ast ) override;
    std::string visit_Idiv( x86_at::Idiv ast ) override;
    std::string visit_Cdq( x86_at::Cdq ast ) override;
    std::string visit_Cmp( x86_at::Cmp ast ) override;
    std::string visit_Jump( x86_at::Jump ast ) override;
    std::string visit_JumpCC( x86_at::JumpCC ast ) override;
    std::string visit_SetCC( x86_at::SetCC ast ) override;
    std::string visit_Label( x86_at::Label ast ) override;

    std::string visit_Register( x86_at::Register ast ) override;
    std::string visit_Ret( x86_at::Ret ast ) override;
    std::string visit_Pseudo( x86_at::Pseudo ast ) override;
    std::string visit_Stack( x86_at::Stack ast ) override;
    std::string visit_Data( x86_at::Data ast ) override;

    std::string indent { "  " };

  private:
    std::string operand( const x86_at::Operand& op );
};
