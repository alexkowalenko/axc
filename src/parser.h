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

enum class Precedence {
    Lowest = 0,
    Or = 5,
    And = 10,
    BitwiseOr = 15,
    BitwiseXor = 20,
    BitwiseAnd = 25,
    Equals = 30,
    Comparison = 35,
    Shift = 40,
    Sum = 45,
    Product = 50,
};

class Parser {
  public:
    Parser( Lexer& lexer ) : lexer( lexer ) {};
    ~Parser() = default;

    ast::Program parse();

    ast::Expr     expr( Precedence precedence = Precedence::Lowest );
    ast::Expr factor();
    ast::UnaryOp  unaryOp();
    ast::BinaryOp binaryOp(ast::Expr left);
    ast::Expr     group();
    ast::Constant constant();

  private:
    template <class T> constexpr std::shared_ptr<T> make_AST() { return std::make_shared<T>( lexer.get_location() ); }

    ast::FunctionDef functionDef();
    ast::Statement   statement();
    ast::Return      ret();

    Token expect_token( TokenType expected );

    Lexer& lexer;
};
