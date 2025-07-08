//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "parser.h"
#include "ast/includes.h"
#include "exception.h"

#include <expected>

ast::Program Parser::parse() {
    auto program = make_AST<ast::Program_>();
    program->function = functionDef();
    expect_token( TokenType::Eof );
    return program;
}

ast::FunctionDef Parser::functionDef() {
    auto funct = make_AST<ast::FunctionDef_>();

    // Get type
    expect_token( TokenType::INT );

    // Get name
    auto name = expect_token( TokenType::IDENTIFIER );
    funct->name = name.value;

    // ( void ) {
    expect_token( TokenType::L_PAREN );
    expect_token( TokenType::VOID );
    expect_token( TokenType::R_PAREN );
    expect_token( TokenType::L_BRACE );

    funct->statement = statement();

    // }
    expect_token( TokenType::R_BRACE );
    return funct;
}

ast::Statement Parser::statement() {
    auto statement = make_AST<ast::Statement_>();
    statement->ret = ret();
    expect_token( TokenType::SEMICOLON );
    return statement;
}

ast::Return Parser::ret() {
    auto ret = make_AST<ast::Return_>();
    expect_token( TokenType::RETURN );
    ret->expr = expr();
    return ret;
}

ast::Expr Parser::expr() {
    auto expr = make_AST<ast::Expr_>();
    expr->constant = constant();
    return expr;
}

ast::Constant Parser::constant() {
    auto constant = make_AST<ast::Constant_>();
    auto token = lexer.get_token();
    if ( token.tok == TokenType::CONSTANT ) {
        constant->value = std::stoi( token.value );
        return constant;
    }
    throw ParseException( token.location, "Expected constant but found {}", token );
}

Token Parser::expect_token( TokenType expected ) {
    auto token = lexer.get_token();
    if ( token.tok != expected ) {
        throw ParseException( token.location, "Expected: {} but found {}", expected, token );
    }
    return token;
}