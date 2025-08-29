//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include "ast/includes.h"
#include "lexer.h"

enum class Precedence {
    Lowest = 0,
    Assignment = 1,
    Conditional = 3,
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
    Postfix = 80,
    FunctionCall = 90,
};

class Parser {
  public:
    explicit Parser( Lexer& lexer ) : lexer( lexer ) {};
    ~Parser() = default;

    ast::Program parse();

    ast::Compound compound();
    ast::If       if_stat();
    ast::Goto     goto_stat();
    ast::Label    label();
    ast::Break    break_stat();
    ast::Continue continue_stat();
    ast::While    while_stat();
    ast::DoWhile  do_while_stat();
    ast::For      for_stat();
    ast::Switch   switch_stat();
    ast::Case     case_stat();
    ast::Return   ret();
    ast::Null     null();

    ast::Expr        expr( Precedence precedence = Precedence::Lowest );
    ast::Expr        factor();
    ast::UnaryOp     unaryOp();
    ast::BinaryOp    binaryOp( ast::Expr left );
    ast::PostOp      postfixOp( ast::Expr left );
    ast::Conditional conditional( ast::Expr left );
    ast::Assign      assign( ast::Expr left );
    ast::Expr        l_paren();
    ast::Cast        cast();
    ast::Expr        group();
    ast::Constant    constant();
    ast::Call        call( ast::Expr left );
    ast::Var         var();

  private:
    template <class T> constexpr std::shared_ptr<T> make_AST() { return std::make_shared<T>( lexer.get_location() ); }

    ast::Declaration declaration();
    void             function_params( ast::FunctionDef f );
    ast::FunctionDef functionDef( std::string const& name, Type type, StorageClass storage );
    ast::VariableDef variableDef( std::string const& name, Type type, StorageClass storage );
    ast::Statement   statement();

    Type type( std::vector<TokenType> const& tokens );

    Token expect_token( TokenType expected );

    Lexer& lexer;
};
