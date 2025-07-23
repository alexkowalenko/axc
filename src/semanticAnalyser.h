//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 22/7/2025.
//

#pragma once

#include "ast/base.h"
#include "ast/visitor.h"
#include "symbolTable.h"

class SemanticAnalyser : public ast::Visitor<void> {
  public:
    SemanticAnalyser( SymbolTable& table ) : symbol_table( table ) {};
    ~SemanticAnalyser() override = default;

    void analyse( const ast::Program ast );

    void visit_Program( const ast::Program ast ) override;
    void visit_FunctionDef( const ast::FunctionDef ast ) override;
    void visit_Declaration( const ast::Declaration ast ) override;
    void statement( const ast::Statement ast );
    void visit_Null( const ast::Null ast ) override {};
    void visit_Return( const ast::Return ast ) override;
    void expr( const ast::Expr ast );
    void visit_UnaryOp( const ast::UnaryOp ast ) override;
    void visit_BinaryOp( const ast::BinaryOp ast ) override;
    void visit_PostOp( const ast::PostOp ast ) override;
    void visit_Assign( const ast::Assign ast ) override;
    void visit_Var( const ast::Var ast ) override;
    void visit_Constant( const ast::Constant ast ) override {};

  private:
    SymbolTable& symbol_table;
};
