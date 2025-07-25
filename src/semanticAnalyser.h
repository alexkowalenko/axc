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
    void visit_Break( const ast::Break ast ) override;
    void visit_Continue( const ast::Continue ast ) override;
    void visit_While( const ast::While ast ) override;
    void visit_DoWhile( const ast::DoWhile ast ) override;
    void for_init( ast::ForInit ast );
    void visit_For( const ast::For ast ) override;
    void visit_Compound( const ast::Compound ast ) override;
    void expr( ast::Expr ast );
    void visit_UnaryOp( ast::UnaryOp ast ) override;
    void visit_BinaryOp( ast::BinaryOp ast ) override;
    void visit_PostOp( ast::PostOp ast ) override;
    void visit_Conditional( ast::Conditional ast ) override;
    void visit_Assign( ast::Assign ast ) override;
    void visit_Var( ast::Var ast ) override;
    void visit_Constant( ast::Constant ast ) override {};

  private:
    SymbolTable new_scope();
    void        new_loop_label( std::shared_ptr<ast::Base> b );
    void        loop_label( std::shared_ptr<ast::Base> b );

    SymbolTable&                symbol_table;
    // Map of goto labels and whether they have been defined
    std::map<std::string, bool> labels;

    size_t loop_count { 0 };
};
