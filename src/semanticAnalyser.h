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
#include <set>
#include <stack>

#include "ast/base.h"
#include "ast/constant.h"
#include "ast/visitor.h"
#include "symbolTable.h"

template <> struct std::less<ast::Constant> {
    bool operator()( const ast::Constant& lhs, const ast::Constant& rhs ) const { return lhs->value < rhs->value; }
};

class SemanticAnalyser {
  public:
    explicit SemanticAnalyser() = default;
    ~SemanticAnalyser() = default;

    void analyse( ast::Program ast, SymbolTable& table );

  private:
    void visit_Program( ast::Program ast, SymbolTable& table );
    void visit_FunctionDef( ast::FunctionDef ast, SymbolTable& table );
    void visit_Declaration( ast::Declaration ast, SymbolTable& table );
    void visit_Statement( ast::Statement ast, SymbolTable& table );
    void statement( ast::StatementItem ast, SymbolTable& table );
    void visit_If( ast::If ast, SymbolTable& table );
    void visit_Goto( ast::Goto ast );
    void visit_Label( ast::Label ast );
    void visit_Return( ast::Return ast, SymbolTable& table );
    void visit_Break( ast::Break ast, SymbolTable& table );
    void visit_Continue( ast::Continue ast, SymbolTable& table );
    void visit_While( ast::While ast, SymbolTable& table );
    void visit_DoWhile( ast::DoWhile ast, SymbolTable& table );
    void for_init( ast::ForInit ast, SymbolTable& table );
    void visit_For( ast::For ast, SymbolTable& table );
    void visit_Switch( ast::Switch ast, SymbolTable& table );
    void visit_Case( ast::Case ast, SymbolTable& table );
    void visit_Compound( ast::Compound ast, SymbolTable& table );
    void expr( ast::Expr ast, SymbolTable& table );
    void visit_UnaryOp( ast::UnaryOp ast, SymbolTable& table );
    void visit_BinaryOp( ast::BinaryOp ast, SymbolTable& table );
    void visit_PostOp( ast::PostOp ast, SymbolTable& table );
    void visit_Conditional( ast::Conditional ast, SymbolTable& table );
    void visit_Assign( ast::Assign ast, SymbolTable& table );
    void visit_Call( ast::Call ast, SymbolTable& table );
    void visit_Var( ast::Var ast, const SymbolTable& table );
    void visit_Constant( ast::Constant ast );

  private:
    static SymbolTable new_scope( SymbolTable& table );

    void new_loop_label( std::shared_ptr<ast::Base> b );
    void loop_label( std::shared_ptr<ast::Base> b );
    void new_switch_label( std::shared_ptr<ast::Base> b );
    void switch_label( std::shared_ptr<ast::Base> b );

    // Map of goto labels and whether they have been defined
    std::map<std::string, bool> labels;

    // Count of loops
    size_t loop_count { 0 };

    // Count of switch statements
    size_t             switch_count { 0 };
    std::stack<size_t> last_default;

    // Stack of switch case sets to check for duplicate case values
    std::stack<std::set<ast::Constant>> case_set;

    // Stack of Switch ASTs to attach Case AST to
    std::stack<ast::Switch> switch_stack;

    // Last break is switch or loop
    TokenType last_break { TokenType::Null };

    // Constant analysis
    bool is_constant { false };

    // Nested function
    bool        nested_function { false };
    // Current function parameter
    SymbolTable function_table;
};
