//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/base.h"
#include "arm64_at/visitor.h"

#include <string>

class PrinterARM64 : public arm64_at::Visitor<std::string> {
  public:
    PrinterARM64() = default;
    ~PrinterARM64() override = default;

    std::string print( arm64_at::Program ast );

    std::string visit_Program( arm64_at::Program ast ) override;
    std::string visit_FunctionDef( arm64_at::FunctionDef ast ) override;
    std::string visit_Mov( arm64_at::Mov ast ) override;
    std::string visit_Load( arm64_at::Load ast ) override;
    std::string visit_Store( arm64_at::Store ast ) override;
    std::string visit_AllocateStack( arm64_at::AllocateStack ast ) override;
    std::string visit_DeallocateStack( arm64_at::DeallocateStack ast ) override;
    std::string visit_Ret( arm64_at::Ret ast ) override;
    std::string visit_Unary( arm64_at::Unary ast ) override;
    std::string visit_Binary( arm64_at::Binary ast ) override;
    std::string visit_Imm( arm64_at::Imm ast ) override;
    std::string visit_Register( arm64_at::Register ast ) override;
    std::string visit_Pseudo( arm64_at::Pseudo ast ) override;
    std::string visit_Stack( arm64_at::Stack ast ) override;

    std::string indent { "  " };

  private:
    std::string operand( const arm64_at::Operand& op );
};
