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
#include <stack>
#include <set>

#include "ast/base.h"
#include "ast/visitor.h"
#include "symbolTable.h"
#include "ast/constant.h"

template<>
struct std::less<ast::Constant> {
    bool operator()( const ast::Constant& lhs, const ast::Constant& rhs ) const {
        return lhs->value < rhs->value;
    }
};

class SemanticAnalyser : public ast::Visitor<void> {
  public:
    explicit SemanticAnalyser( SymbolTable& table ) : symbol_table( table ) {};
    ~SemanticAnalyser() override = default;

    void analyse( ast::Program ast );

    void visit_Program( ast::Program ast ) override;
    void visit_FunctionDef( ast::FunctionDef ast ) override;
    void visit_Declaration( ast::Declaration ast ) override;
    void visit_Statement( const ast::Statement ast ) override;
    void statement( ast::StatementItem ast );
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
    void visit_Switch( const ast::Switch ast ) override;
    void visit_Case( const ast::Case ast ) override;
    void visit_Compound( const ast::Compound ast ) override;
    void expr( ast::Expr ast );
    void visit_UnaryOp( ast::UnaryOp ast ) override;
    void visit_BinaryOp( ast::BinaryOp ast ) override;
    void visit_PostOp( ast::PostOp ast ) override;
    void visit_Conditional( ast::Conditional ast ) override;
    void visit_Assign( ast::Assign ast ) override;
    void visit_Call( const ast::Call ast ) override;
    void visit_Var( ast::Var ast ) override;
    void visit_Constant( ast::Constant ast ) override;

  private:
    SymbolTable new_scope();
    void        new_loop_label( std::shared_ptr<ast::Base> b );
    void        loop_label( std::shared_ptr<ast::Base> b );
    void        new_switch_label( std::shared_ptr<ast::Base> b );
    void        switch_label( std::shared_ptr<ast::Base> b );

    SymbolTable&                symbol_table;
    // Map of goto labels and whether they have been defined
    std::map<std::string, bool> labels;

    // Count of loops
    size_t loop_count { 0 };

    // Count of switch statements
    size_t switch_count { 0 };
    std::stack<size_t> last_default;

    // Stack of switch case sets to check for duplicate case values
    std::stack<std::set<ast::Constant>> case_set;

    // Stack of Switch ASTs to attach Case AST to
    std::stack<ast::Switch> switch_stack;

    // Last break is switch or loop
    TokenType last_break { TokenType::Null };

    // Constant analysis
    bool   is_constant { false };
};
