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
#include "spdlog/spdlog.h"

#include <expected>
#include <functional>
#include <map>

std::map<TokenType, Precedence> precedence_map = {
    { TokenType::PLUS, Precedence::Sum },        { TokenType::DASH, Precedence::Sum },
    { TokenType::ASTÉRIX, Precedence::Product }, { TokenType::SLASH, Precedence::Product },
    { TokenType::PERCENT, Precedence::Product },
};

constexpr Precedence get_precedence( const TokenType tok ) {
    if ( precedence_map.contains( tok ) ) {
        return precedence_map.at( tok );
    }
    return Precedence::Lowest;
}

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

using PrefixParselet = std::function<ast::Expr( Parser* )>;
const std::map<TokenType, PrefixParselet> prefix_map {
    { TokenType::CONSTANT, []( Parser* p ) -> ast::Expr { return p->constant(); } },
    { TokenType::DASH, []( Parser* p ) -> ast::Expr { return p->unaryOp(); } },
    { TokenType::TILDE, []( Parser* p ) -> ast::Expr { return p->unaryOp(); } },
    { TokenType::L_PAREN, []( Parser* p ) -> ast::Expr { return p->group(); } },
};

using InfixParselet = std::function<ast::Expr( Parser* p, ast::Expr left )>;
const std::map<TokenType, InfixParselet> infix_map {
    { TokenType::PLUS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::DASH, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::ASTÉRIX, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::SLASH, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::PERCENT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
};

ast::Expr Parser::expr( const Precedence precedence ) {
    spdlog::debug( "expr( {} )", static_cast<int>( precedence ) );
    auto left = factor();

    // Get second expression
    auto token = lexer.peek_token();
    while ( infix_map.contains( token.tok ) && get_precedence( token.tok ) >= precedence ) {
        left = infix_map.at( token.tok )( this, left );
        token = lexer.peek_token();
    }
    return left;
}

ast::Expr Parser::factor() {
    spdlog::debug( "factor()" );
    auto token = lexer.peek_token();
    auto parselet = prefix_map.find( token.tok );
    if ( parselet == prefix_map.end() ) {
        throw ParseException( token.location, "Unexpected token {}", token );
    }
    auto left = prefix_map.at( token.tok )( this );
    return left;
}

ast::UnaryOp Parser::unaryOp() {
    spdlog::debug( "unaryOp()" );
    auto token = lexer.get_token();
    auto op = make_AST<ast::UnaryOp_>();
    op->op = token.tok;
    op->operand = factor();
    return op;
}

ast::BinaryOp Parser::binaryOp( ast::Expr left ) {
    spdlog::debug( "binaryOp()" );
    auto token = lexer.get_token();
    auto op = make_AST<ast::BinaryOp_>();
    op->left = std::move( left );
    op->op = token.tok;
    op->right = expr( static_cast<Precedence>(static_cast<int>(get_precedence( token.tok )) + 1) );
    return op;
}

ast::Expr Parser::group() {
    spdlog::debug( "group()" );
    expect_token( TokenType::L_PAREN );
    auto e = expr( Precedence::Lowest );
    expect_token( TokenType::R_PAREN );
    return e;
}

ast::Constant Parser::constant() {
    spdlog::debug( "constant()" );
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