//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include "ast/program.h"
#include "lexer.h"

class Parser {
  public:
    Parser( Lexer& lexer ) : lexer( lexer ) {};
    ~Parser() = default;

    ast::Program parse();

  private:
    template <class T> constexpr std::shared_ptr<T> make_AST() { return std::make_shared<T>( lexer.get_location() ); }

    ast::FunctionDef functionDef();
    ast::Statement   statement();
    ast::Return      ret();
    ast::Expr        expr();
    ast::UnaryOp     unaryOp();
    ast::Constant    constant();

    Token expect_token( TokenType expected );

    Lexer& lexer;
};
