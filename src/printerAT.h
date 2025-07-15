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

#include "at/visitor.h"
#include "at/base.h"

class PrinterAT : public at::Visitor<std::string> {
  public:
    PrinterAT() = default;
    ~PrinterAT() override = default;

    std::string print( const at::Program ast );

    std::string visit_Program( const at::Program ast ) override;
    std::string visit_FunctionDef( const at::FunctionDef ast ) override;
    std::string visit_Mov( const at::Mov ast ) override;
    std::string visit_Imm( const at::Imm ast ) override;
    std::string visit_Unary( const at::Unary ast ) override;
    std::string visit_AllocateStack( const at::AllocateStack ast ) override;
    std::string visit_Binary( const at::Binary ast ) override;
    std::string visit_Idiv( const at::Idiv ast ) override;
    std::string visit_Cdq( const at::Cdq ast ) override;


    std::string visit_Register( const at::Register ast ) override;
    std::string visit_Ret( const at::Ret ast ) override;
    std::string visit_Pseudo( const at::Pseudo ast ) override;
    std::string visit_Stack( const at::Stack ast ) override;

    std::string indent { "  " };

  private:
    std::string operand( const at::Operand& op );
};
