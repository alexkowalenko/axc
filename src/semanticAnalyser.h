//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 22/7/2025.
//

#pragma once

#include <map>

#include "ast/base.h"
#include "ast/visitor.h"
#include "symbolTable.h"

class SemanticAnalyser : public ast::Visitor<void> {
  public:
    explicit SemanticAnalyser( SymbolTable& table ) : symbol_table( table ) {};
    ~SemanticAnalyser() override = default;

    void analyse( ast::Program ast );

    void visit_Program( ast::Program ast ) override;
    void visit_FunctionDef( ast::FunctionDef ast ) override;
    void visit_Declaration( ast::Declaration ast ) override;
    void statement( ast::Statement ast );
    void visit_If( ast::If ast ) override;
    void visit_Goto( ast::Goto ast ) override;
    void visit_Label( ast::Label ast ) override;
    void visit_Null( ast::Null ast ) override {};
    void visit_Return( ast::Return ast ) override;
    void expr( ast::Expr ast );
    void visit_UnaryOp( ast::UnaryOp ast ) override;
    void visit_BinaryOp( ast::BinaryOp ast ) override;
    void visit_PostOp( ast::PostOp ast ) override;
    void visit_Conditional( ast::Conditional ast ) override;
    void visit_Assign( ast::Assign ast ) override;
    void visit_Var( ast::Var ast ) override;
    void visit_Constant( ast::Constant ast ) override {};

  private:
    SymbolTable& symbol_table;
    std::map<std::string, bool> labels;
};
