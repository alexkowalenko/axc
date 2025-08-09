//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include "ast/base.h"

#include <string>

#include "ast/visitor.h"

class PrinterAST : public ast::Visitor<std::string> {
  public:
    PrinterAST() = default;
    ~PrinterAST() override = default;

    std::string print( const ast::Program& ast );

    std::string visit_Program( ast::Program ast ) override;
    std::string visit_FunctionDef( ast::FunctionDef ast ) override;
    std::string block_item( ast::BlockItem ast );
    std::string visit_Declaration( ast::Declaration ast ) override;
    std::string visit_Statement( ast::Statement ast ) override;
    std::string statement( ast::StatementItem ast );
    std::string visit_If( ast::If ast ) override;
    std::string visit_Return( ast::Return ast ) override;
    std::string visit_Null( ast::Null ast ) override;
    std::string visit_Goto( ast::Goto ast ) override;
    std::string visit_Label( ast::Label ast ) override;
    std::string visit_Break( ast::Break ast ) override;
    std::string visit_Continue( ast::Continue ast ) override;
    std::string visit_While( ast::While ast ) override;
    std::string visit_DoWhile( ast::DoWhile ast ) override;
    std::string for_init( ast::ForInit ast );
    std::string visit_For( ast::For ast ) override;
    std::string visit_Switch( ast::Switch ast ) override;
    std::string visit_Case( ast::Case ast ) override;
    std::string visit_Compound( ast::Compound ast ) override;
    std::string expr( ast::Expr ast );
    std::string visit_UnaryOp( ast::UnaryOp ast ) override;
    std::string visit_PostOp( ast::PostOp ast ) override;
    std::string visit_BinaryOp( ast::BinaryOp ast ) override;
    std::string visit_Conditional( ast::Conditional ast ) override;
    std::string visit_Assign( ast::Assign ast ) override;
    std::string visit_Call( ast::Call ast ) override;
    std::string visit_Constant( ast::Constant ast ) override;
    std::string visit_Var( ast::Var ast ) override;

    std::string indent { "  " };
    std::string new_line { "\n" };
};
