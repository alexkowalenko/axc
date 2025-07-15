//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#pragma once

#include "tac/visitor.h"
#include "tac/base.h"


class PrinterTAC : public tac::Visitor<std::string> {
public:
    PrinterTAC() = default;
    ~PrinterTAC() override = default;

    std::string print( const tac::Program ast );

    std::string visit_Program( const tac::Program ast ) override;
    std::string visit_FunctionDef( const tac::FunctionDef ast ) override;
    std::string value(const tac::Value ast);
    std::string visit_Return( const tac::Return ast ) override;
    std::string visit_Binary( const tac::Binary ast ) override;
    std::string visit_Unary( const tac::Unary ast ) override;
    std::string visit_Constant( const tac::Constant ast ) override;
    std::string visit_Variable( const tac::Variable ast ) override;

    std::string indent { "  " };
};

