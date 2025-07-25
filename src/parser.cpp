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
    return precedence_map.contains( tok ) ? precedence_map.at( tok ) : Precedence::Lowest;
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

    // ( void )
    expect_token( TokenType::L_PAREN );
    expect_token( TokenType::VOID );
    expect_token( TokenType::R_PAREN );

    funct->block = compound();

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

using StatementParselet = std::function<ast::Statement( Parser* )>;
const std::map<TokenType, StatementParselet> statement_map {
    { TokenType::RETURN, []( Parser* p ) -> ast::Statement { return p->ret(); } },
    { TokenType::IF, []( Parser* p ) -> ast::Statement { return p->if_stat(); } },
    { TokenType::GOTO, []( Parser* p ) -> ast::Statement { return p->goto_stat(); } },
    { TokenType::L_BRACE, []( Parser* p ) -> ast::Statement { return p->compound(); } },
    { TokenType::SEMICOLON, []( Parser* p ) -> ast::Statement { return p->null(); } },
    { TokenType::BREAK, []( Parser* p ) -> ast::Statement { return p->break_stat(); } },
    { TokenType::CONTINUE, []( Parser* p ) -> ast::Statement { return p->continue_stat(); } },
    { TokenType::WHILE, []( Parser* p ) -> ast::Statement { return p->while_stat(); } },
    { TokenType::DO, []( Parser* p ) -> ast::Statement { return p->do_while_stat(); } },
    { TokenType::FOR, []( Parser* p ) -> ast::Statement { return p->for_stat(); } },
    { TokenType::SWITCH, []( Parser* p ) -> ast::Statement { return p->switch_stat(); } },
    { TokenType::CASE, []( Parser* p ) -> ast::Statement { return p->case_stat(); } },
    { TokenType::DEFAULT, []( Parser* p ) -> ast::Statement { return p->case_stat(); } },
};

ast::Statement Parser::statement() {
    spdlog::debug( "statement" );
    ast::Statement statement;

    auto token = lexer.peek_token();
    if ( statement_map.contains( token.tok ) ) {
        // Use the parselet for the statement
        statement = statement_map.at( token.tok )( this );
    } else {
        // Handle label
        if ( token.tok == TokenType::IDENTIFIER && lexer.peek_token( 1 ).tok == TokenType::COLON ) {
            statement = label();
        } else {
            statement = expr();
            expect_token( TokenType::SEMICOLON );
        }
    }
    return statement;
}

ast::Compound Parser::compound() {
    spdlog::debug( "compound" );
    expect_token( TokenType::L_BRACE );
    auto compound = make_AST<ast::Compound_>();

    auto token = lexer.peek_token();
    while ( token.tok != TokenType::R_BRACE ) {
        if ( token.tok == TokenType::INT ) {
            // Check for label - not valid in C17, but allowed in C23.
            previous_label( compound );

            ast::BlockItem block = declaration();
            compound->block_items.push_back( block );
        } else {
            ast::BlockItem block = statement();
            compound->block_items.push_back( block );
        }
        token = lexer.peek_token();
    }

    // Check label without statement - not valid in C17, but allowed in C23.
    previous_label( compound );

    // }
    expect_token( TokenType::R_BRACE );
    return compound;
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

ast::Goto Parser::goto_stat() {
    spdlog::debug( "goto" );
    auto goto_stat = make_AST<ast::Goto_>();
    expect_token( TokenType::GOTO );
    goto_stat->label = expect_token( TokenType::IDENTIFIER ).value;
    expect_token( TokenType::SEMICOLON );
    return goto_stat;
}

ast::Label Parser::label() {
    spdlog::debug( "label" );
    auto label = make_AST<ast::Label_>();
    auto token = expect_token( TokenType::IDENTIFIER );
    label->label = token.value;
    expect_token( TokenType::COLON );
    return label;
}

ast::Break Parser::break_stat() {
    spdlog::debug( "break" );
    expect_token( TokenType::BREAK );
    expect_token( TokenType::SEMICOLON );
    return make_AST<ast::Break_>();
}

ast::Continue Parser::continue_stat() {
    spdlog::debug( "continue" );
    expect_token( TokenType::CONTINUE );
    expect_token( TokenType::SEMICOLON );
    return make_AST<ast::Continue_>();
}

ast::While Parser::while_stat() {
    spdlog::debug( "while" );
    auto while_stat = make_AST<ast::While_>();
    expect_token( TokenType::WHILE );
    expect_token( TokenType::L_PAREN );
    while_stat->condition = expr();
    expect_token( TokenType::R_PAREN );
    while_stat->body = statement();
    return while_stat;
}

ast::DoWhile Parser::do_while_stat() {
    spdlog::debug( "do while" );
    auto do_while_stat = make_AST<ast::DoWhile_>();
    expect_token( TokenType::DO );
    do_while_stat->body = statement();
    expect_token( TokenType::WHILE );
    expect_token( TokenType::L_PAREN );
    do_while_stat->condition = expr();
    expect_token( TokenType::R_PAREN );
    expect_token( TokenType::SEMICOLON );
    return do_while_stat;
}

ast::For Parser::for_stat() {
    spdlog::debug( "for" );
    auto for_stat = make_AST<ast::For_>();
    expect_token( TokenType::FOR );
    expect_token( TokenType::L_PAREN );

    // Init
    auto token = lexer.peek_token();
    if ( token.tok != TokenType::SEMICOLON ) {
        if ( token.tok == TokenType::INT ) {
            // Declaration
            for_stat->init = declaration();
        } else {
            // Expression
            for_stat->init = expr();
            expect_token( TokenType::SEMICOLON );
        }
    } else {
        // If there is no init, we expect a semicolon
        expect_token( TokenType::SEMICOLON );
    }

    spdlog::debug( "for - condition" );
    // Condition
    if ( lexer.peek_token().tok != TokenType::SEMICOLON ) {
        for_stat->condition = expr();
    }
    expect_token( TokenType::SEMICOLON );

    // Increment
    if ( lexer.peek_token().tok != TokenType::R_PAREN ) {
        for_stat->increment = expr();
    }
    expect_token( TokenType::R_PAREN );

    for_stat->body = statement();
    return for_stat;
}

ast::Switch Parser::switch_stat() {
    spdlog::debug( "switch" );
    auto switch_stat = make_AST<ast::Switch_>();
    expect_token( TokenType::SWITCH );
    expect_token( TokenType::L_PAREN );
    switch_stat->condition = expr();
    expect_token( TokenType::R_PAREN );
    switch_stat->body = statement();
    return switch_stat;
}

ast::Case Parser::case_stat() {
    spdlog::debug( "case" );

    auto case_stat = make_AST<ast::Case_>();
    auto token = lexer.get_token();
    spdlog::debug( "case: {}", to_string( token.tok ) );
    if ( token.tok == TokenType::DEFAULT ) {
        case_stat->is_default = true;
    } else if ( token.tok == TokenType::CASE ) {
        case_stat->is_default = false;
        case_stat->value = expr();
    } else {
        throw ParseException( token.location, "Expected 'case' or 'default', got {}", token.tok );
    }
    expect_token( TokenType::COLON );

    // Get block items
    token = lexer.peek_token();
    bool first = true;
    while ( token.tok != TokenType::CASE && token.tok != TokenType::DEFAULT && token.tok != TokenType::R_BRACE ) {
        spdlog::debug( "case: statement {}", to_string( token.tok ) );
        if ( token.tok == TokenType::INT ) {
            if ( first ) {
                throw ParseException( lexer.get_location(), "Declaration not allowed in C17." );
            }
            ast::BlockItem block = declaration();
            case_stat->block_items.push_back( block );
        } else {
            ast::BlockItem block = statement();
            case_stat->block_items.push_back( block );
        }
        first = false;
        token = lexer.peek_token();
    }

    return case_stat;
}

ast::Return Parser::ret() {
    spdlog::debug( "ret" );
    auto ret = make_AST<ast::Return_>();
    expect_token( TokenType::RETURN );
    ret->expr = expr();
    expect_token( TokenType::SEMICOLON );
    return ret;
}

ast::Null Parser::null() {
    expect_token( TokenType::SEMICOLON );
    return make_AST<ast::Null_>();
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
    op->else_expr = expr( Precedence::Conditional );
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

void Parser::previous_label( ast::Compound compound ) {
    // Check if the previous item was a label
    if ( !compound->block_items.empty() && std::holds_alternative<ast::Statement>( compound->block_items.back() ) ) {
        auto last_statement = std::get<ast::Statement>( compound->block_items.back() );
        if ( std::holds_alternative<ast::Label>( last_statement ) ) {
            throw ParseException( lexer.get_location(), "Label without statement is not allowed in C17." );
        }
    }
}