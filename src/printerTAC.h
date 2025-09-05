//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#pragma once

#include "tac/includes.h"
#include "tac/visitor.h"

class PrinterTAC : public tac::Visitor<std::string> {
  public:
    PrinterTAC() = default;
    ~PrinterTAC() override = default;

    std::string print( tac::Program ast );

    std::string visit_Program( tac::Program ast ) override;
    std::string visit_FunctionDef( tac::FunctionDef ast ) override;
    std::string value( tac::Value ast );
    std::string visit_Return( tac::Return ast ) override;
    std::string visit_Binary( tac::Binary ast ) override;
    std::string visit_Unary( tac::Unary ast ) override;
    std::string visit_Copy( tac::Copy ast ) override;
    std::string visit_Jump( tac::Jump ast ) override;
    std::string visit_JumpIfZero( tac::JumpIfZero ast ) override;
    std::string visit_JumpIfNotZero( tac::JumpIfNotZero ast ) override;
    std::string visit_Label( tac::Label ast ) override;
    std::string visit_FunCall( tac::FunCall ast ) override;
    std::string visit_SignExtend( tac::SignExtend ast ) override;
    std::string visit_Truncate( tac::Truncate ast ) override;
    std::string visit_StaticVariable( tac::StaticVariable ast ) override;

    std::string visit_ConstantInt( tac::ConstantInt ast ) override;
    std::string visit_ConstantLong( tac::ConstantLong ast ) override;
    std::string visit_Variable( tac::Variable ast ) override;

    std::string indent { "  " };
};
