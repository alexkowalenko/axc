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

const std::map<TokenType, Precedence> precedence_map = {
    //
    { TokenType::PIPE, Precedence::BitwiseOr },
    { TokenType::CARET, Precedence::BitwiseXor },
    { TokenType::AMPERSAND, Precedence::BitwiseAnd },
    { TokenType::LEFT_SHIFT, Precedence::Shift },
    { TokenType::RIGHT_SHIFT, Precedence::Shift },
    { TokenType::PLUS, Precedence::Sum },
    { TokenType::DASH, Precedence::Sum },
    { TokenType::ASTÉRIX, Precedence::Product },
    { TokenType::SLASH, Precedence::Product },
    { TokenType::PERCENT, Precedence::Product },
    { TokenType::LESS, Precedence::Comparison },
    { TokenType::LESS_EQUALS, Precedence::Comparison },
    { TokenType::GREATER, Precedence::Comparison },
    { TokenType::GREATER_EQUALS, Precedence::Comparison },
    { TokenType::COMPARISON_EQUALS, Precedence::Equals },
    { TokenType::COMPARISON_NOT, Precedence::Equals },
    { TokenType::LOGICAL_AND, Precedence::And },
    { TokenType::LOGICAL_OR, Precedence::Or },
    { TokenType::EQUALS, Precedence::Assignment },
    { TokenType::COMPOUND_PLUS, Precedence::Assignment },
    { TokenType::COMPOUND_MINUS, Precedence::Assignment },
    { TokenType::COMPOUND_ASTERIX, Precedence::Assignment },
    { TokenType::COMPOUND_SLASH, Precedence::Assignment },
    { TokenType::COMPOUND_PERCENT, Precedence::Assignment },
    { TokenType::COMPOUND_AND, Precedence::Assignment },
    { TokenType::COMPOUND_OR, Precedence::Assignment },
    { TokenType::COMPOUND_XOR, Precedence::Assignment },
    { TokenType::COMPOUND_LEFT_SHIFT, Precedence::Assignment },
    { TokenType::COMPOUND_RIGHT_SHIFT, Precedence::Assignment },
    { TokenType::INCREMENT, Precedence::Postfix },
    { TokenType::DECREMENT, Precedence::Postfix },
    { TokenType::QUESTION, Precedence::Conditional },
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
    spdlog::debug( "Finish parse." );
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

    auto token = lexer.peek_token();
    while ( token.tok != TokenType::R_BRACE ) {
        if ( token.tok == TokenType::INT ) {
            ast::BlockItem block = declaration();
            funct->block_items.push_back( block );
        } else {
            ast::BlockItem block = statement();
            funct->block_items.push_back( block );
        }
        token = lexer.peek_token();
    }

    // }
    expect_token( TokenType::R_BRACE );
    return funct;
}

ast::Declaration Parser::declaration() {
    spdlog::debug( "declaration" );
    auto decl = make_AST<ast::Declaration_>();
    expect_token( TokenType::INT );

    // Get name
    auto name = expect_token( TokenType::IDENTIFIER );
    decl->name = name.value;

    // check for =
    auto token = lexer.peek_token();
    if ( token.tok == TokenType::EQUALS ) {
        expect_token( TokenType::EQUALS );
        decl->init = expr();
    }
    expect_token( TokenType::SEMICOLON );
    return decl;
}

ast::Statement Parser::statement() {
    spdlog::debug( "statement" );
    ast::Statement statement;
    auto           token = lexer.peek_token();
    if ( token.tok == TokenType::RETURN ) {
        statement = ret();
    } else if ( token.tok == TokenType::IF ) {
        statement = if_stat();
    } else if ( token.tok == TokenType::SEMICOLON ) {
        expect_token( TokenType::SEMICOLON );
        statement = make_AST<ast::Null_>();
    } else {
        statement = expr();
        expect_token( TokenType::SEMICOLON );
    }
    return statement;
}

ast::If Parser::if_stat() {
    spdlog::debug( "if" );
    auto if_stat = make_AST<ast::If_>();
    expect_token( TokenType::IF );
    expect_token( TokenType::L_PAREN );
    if_stat->condition = expr();
    expect_token( TokenType::R_PAREN );
    if_stat->then = statement();
    auto token = lexer.peek_token();
    if ( token.tok == TokenType::ELSE ) {
        expect_token( TokenType::ELSE );
        if_stat->else_stat = statement();
    }
    return if_stat;
}

ast::Return Parser::ret() {
    spdlog::debug( "ret" );
    auto ret = make_AST<ast::Return_>();
    expect_token( TokenType::RETURN );
    ret->expr = expr();
    expect_token( TokenType::SEMICOLON );
    return ret;
}

using PrefixParselet = std::function<ast::Expr( Parser* )>;
const std::map<TokenType, PrefixParselet> prefix_map {
    { TokenType::CONSTANT, []( Parser* p ) -> ast::Expr { return p->constant(); } },
    { TokenType::IDENTIFIER, []( Parser* p ) -> ast::Expr { return p->var(); } },
    { TokenType::DASH, []( Parser* p ) -> ast::Expr { return p->unaryOp(); } },
    { TokenType::TILDE, []( Parser* p ) -> ast::Expr { return p->unaryOp(); } },
    { TokenType::EXCLAMATION, []( Parser* p ) -> ast::Expr { return p->unaryOp(); } },
    { TokenType::INCREMENT, []( Parser* p ) -> ast::Expr { return p->unaryOp(); } },
    { TokenType::DECREMENT, []( Parser* p ) -> ast::Expr { return p->unaryOp(); } },
    { TokenType::L_PAREN, []( Parser* p ) -> ast::Expr { return p->group(); } },
};

using InfixParselet = std::function<ast::Expr( Parser* p, ast::Expr left )>;
const std::map<TokenType, InfixParselet> infix_map {
    { TokenType::PLUS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::DASH, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::ASTÉRIX, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::SLASH, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::PERCENT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::AMPERSAND, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::CARET, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::PIPE, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::LEFT_SHIFT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::RIGHT_SHIFT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::LOGICAL_AND, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::LOGICAL_OR, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::COMPARISON_EQUALS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::COMPARISON_NOT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::LESS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::LESS_EQUALS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::GREATER, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::GREATER_EQUALS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->binaryOp( left ); } },
    { TokenType::EQUALS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_PLUS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_MINUS, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_ASTERIX, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_SLASH, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_PERCENT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_AND, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_OR, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_XOR, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_LEFT_SHIFT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::COMPOUND_RIGHT_SHIFT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->assign( left ); } },
    { TokenType::INCREMENT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->postfixOp( left ); } },
    { TokenType::DECREMENT, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->postfixOp( left ); } },
    { TokenType::QUESTION, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->conditional( left ); } },
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

    // Look for postfix operators
    token = lexer.peek_token();
    if ( token.tok == TokenType::INCREMENT || token.tok == TokenType::DECREMENT ) {
        return postfixOp( ast::Expr( left ) );
    }
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
    op->right = expr( static_cast<Precedence>( static_cast<int>( get_precedence( token.tok ) ) + 1 ) );
    return op;
}

ast::PostOp Parser::postfixOp( ast::Expr left ) {
    spdlog::debug( "postfixOp()" );
    auto token = lexer.get_token();
    auto op = make_AST<ast::PostOp_>();
    op->operand = std::move( left );
    op->op = token.tok;
    // No right hand for postOp
    return op;
}

ast::Conditional Parser::conditional( ast::Expr left ) {
    spdlog::debug( "conditional()" );
    auto token = lexer.get_token();
    auto op = make_AST<ast::Conditional_>();
    op->condition = std::move( left );
    op->then_expr = expr();
    expect_token( TokenType::COLON );
    op->else_expr = expr(Precedence::Conditional);
    return op;
}

ast::Assign Parser::assign( ast::Expr left ) {
    spdlog::debug( "assign()" );
    auto token = lexer.get_token();
    auto op = make_AST<ast::Assign_>();
    op->left = std::move( left );
    op->op = token.tok;
    op->right = expr( static_cast<Precedence>( static_cast<int>( get_precedence( TokenType::EQUALS ) ) ) );
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

ast::Var Parser::var() {
    spdlog::debug( "var()" );
    auto token = lexer.get_token();
    auto var = make_AST<ast::Var_>();
    var->name = token.value;
    return var;
}

Token Parser::expect_token( TokenType expected ) {
    auto token = lexer.get_token();
    if ( token.tok != expected ) {
        throw ParseException( token.location, "Expected: {} but found {}", expected, token );
    }
    return token;
}