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
#include "tac/base.h"
#include "tac/visitor.h"

class TacGen {
  public:
    TacGen() = default;
    ~TacGen() = default;

    tac::Program generate( ast::Program ast );

  private:
    tac::FunctionDef functionDef( ast::FunctionDef ast );
    void             ret( ast::Return ast, std::vector<tac::Instruction>& instructions );
    tac::Value       expr( ast::Expr ast, std::vector<tac::Instruction>& instructions );

    tac::Value    unary( ast::UnaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value    binary( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Value    logical( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions );
    tac::Constant constant( ast::Constant ast );

    tac::Label generate_label( std::shared_ptr<ast::Base> b, std::string_view name );

    size_t label_count {};
};
