//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/visitor.h"
#include "arm64_at/base.h"

#include <string>

class PrinterARM64 : public arm64_at::Visitor<std::string> {
public:
    PrinterARM64() = default;
    ~PrinterARM64() override = default;

    std::string print( const arm64_at::Program ast );

    std::string visit_Program( const arm64_at::Program ast ) override;
    std::string visit_FunctionDef( const arm64_at::FunctionDef ast ) override;
    std::string visit_Mov( const arm64_at::Mov ast ) override;
    std::string visit_Ret( const arm64_at::Ret ast ) override;
    std::string visit_Imm( const arm64_at::Imm ast ) override;
    std::string visit_Register( const arm64_at::Register ast ) override;

    std::string indent { "  " };

    private:
    std::string operand( const arm64_at::Operand& op );
};
