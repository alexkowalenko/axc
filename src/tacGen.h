//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

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
    tac::FunctionDef functionDef( ast::FunctionDef ast );

    void       declaration( ast::Declaration ast, std::vector<tac::Instruction>& instructions );
    void       statement( ast::Statement ast, std::vector<tac::Instruction>& instructions );
    void       ret( ast::Return ast, std::vector<tac::Instruction>& instructions );
    void       if_stat( ast::If ast, std::vector<tac::Instruction>& instructions );
    void       goto_stat( ast::Goto ast, std::vector<tac::Instruction>& instructions );
    void       label( ast::Label ast, std::vector<tac::Instruction>& instructions );
    void       break_stat( const ast::Break ast, std::vector<tac::Instruction>& instructions );
    void       continue_stat( const ast::Continue ast, std::vector<tac::Instruction>& instructions );
    void       while_stat( const ast::While ast, std::vector<tac::Instruction>& instructions );
    void       do_while_stat( const ast::DoWhile ast, std::vector<tac::Instruction>& instructions );
    void       for_stat( const ast::For ast, std::vector<tac::Instruction>& instructions );
    void       compound( ast::Compound ast, std::vector<tac::Instruction>& instructions );
    tac::Value expr( ast::Expr ast, std::vector<tac::Instruction>& instructions );

    tac::Value    unary( ast::UnaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value    binary( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value    post( ast::PostOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value    conditional( ast::Conditional ast, std::vector<tac::Instruction>& instructions );
    tac::Value    logical( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value    assign( ast::Assign ast, std::vector<tac::Instruction>& instructions );
    tac::Constant constant( ast::Constant ast );

    tac::Label generate_label( std::shared_ptr<ast::Base> b, std::string_view name );
    tac::Label generate_loop_break( std::shared_ptr<ast::Base> b );
    tac::Label generate_loop_continue( std::shared_ptr<ast::Base> b);

    SymbolTable& symbol_table;
    size_t       label_count {};
};
