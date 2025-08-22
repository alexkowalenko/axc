//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#pragma once

#include "ast/base.h"
#include "ast/visitor.h"
#include "symbolTable.h"
#include "tac/base.h"
#include "tac/visitor.h"

class TacGen {
  public:
    TacGen( SymbolTable& symbol_table ) : symbol_table( symbol_table ) {};
    ~TacGen() = default;

    tac::Program generate( ast::Program ast );

  private:
    std::optional<tac::FunctionDef>    functionDef( ast::FunctionDef ast );
    std::optional<tac::StaticVariable> staticVariable( ast::VariableDef ast );

    void declaration( ast::VariableDef ast, std::vector<tac::Instruction>& instructions );
    void statement( ast::Statement, std::vector<tac::Instruction>& instructions );
    void ret( ast::Return ast, std::vector<tac::Instruction>& instructions );
    void if_stat( ast::If ast, std::vector<tac::Instruction>& instructions );
    void goto_stat( ast::Goto ast, std::vector<tac::Instruction>& instructions );
    void label( ast::Label ast, std::vector<tac::Instruction>& instructions );
    void break_stat( ast::Break ast, std::vector<tac::Instruction>& instructions );
    void continue_stat( ast::Continue ast, std::vector<tac::Instruction>& instructions );
    void while_stat( ast::While ast, std::vector<tac::Instruction>& instructions );
    void do_while_stat( ast::DoWhile ast, std::vector<tac::Instruction>& instructions );
    void for_stat( ast::For ast, std::vector<tac::Instruction>& instructions );
    void compound( ast::Compound ast, std::vector<tac::Instruction>& instructions );
    void switch_stat( ast::Switch ast, std::vector<tac::Instruction>& instructions );
    void case_stat( ast::Case ast, std::vector<tac::Instruction>& instructions );

    tac::Value expr( ast::Expr ast, std::vector<tac::Instruction>& instructions );
    tac::Value unary( ast::UnaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value binary( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value post( ast::PostOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value conditional( ast::Conditional ast, std::vector<tac::Instruction>& instructions );
    tac::Value logical( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value assign( ast::Assign ast, std::vector<tac::Instruction>& instructions );
    tac::Value call( ast::Call ast, std::vector<tac::Instruction>& instructions );

    static tac::Constant constant( ast::Constant ast );

    tac::Label        generate_label( std::shared_ptr<ast::Base> b, std::string_view name );
    static tac::Label generate_loop_break( std::shared_ptr<ast::Base> b );
    static tac::Label generate_loop_continue( std::shared_ptr<ast::Base> b );

    SymbolTable& symbol_table;
    size_t       label_count {};
};
