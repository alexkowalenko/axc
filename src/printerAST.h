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

#include "ast/visitor.h"

class PrinterAST : public ast::Visitor<std::string> {
  public:
    PrinterAST() = default;
    ~PrinterAST() override = default;

    std::string print( const ast::Program& ast );

    std::string visit_Program( const ast::Program& ast ) override;
    std::string visit_FunctionDef( const ast::FunctionDef& ast ) override;
    std::string visit_Statement( const ast::Statement& ast ) override;
    std::string visit_Return( const ast::Return& ast ) override;
    std::string visit_Expr( const ast::Expr& ast ) override;
    std::string visit_Constant( const ast::Constant& ast ) override;

    std::string indent {"  "};
};
