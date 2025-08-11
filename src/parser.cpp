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
    { TokenType::L_PAREN, Precedence::FunctionCall },
};

constexpr Precedence get_precedence( const TokenType tok ) {
    return precedence_map.contains( tok ) ? precedence_map.at( tok ) : Precedence::Lowest;
}

constexpr bool isTypeOrStorage( Token const& t ) {
    return t.tok == TokenType::INT || t.tok == TokenType::STATIC || t.tok == TokenType::EXTERN;
}

ast::Program Parser::parse() {
    auto program = make_AST<ast::Program_>();

    auto token = lexer.peek_token();
    while ( token.tok != TokenType::Eof ) {
        program->declarations.push_back( declaration() );
        token = lexer.peek_token();
    }
    expect_token( TokenType::Eof );
    spdlog::debug( "Finish parse." );
    return program;
}

ast::Declaration Parser::declaration() {
    spdlog::debug( "declaration" );
    ast::Declaration declaration;
    StorageClass     storage_class = StorageClass::None;
    auto             token = lexer.peek_token();
    bool             type = false;
    while ( token.tok != TokenType::IDENTIFIER ) {
        spdlog::debug( "declaration: token {}", to_string( token.tok ) );
        switch ( token.tok ) {
        case TokenType::STATIC :
            if ( storage_class != StorageClass::None ) {
                throw ParseException( lexer.get_location(), "Storage class already set." );
            }
            storage_class = StorageClass::Static;
            expect_token( TokenType::STATIC );
            break;
        case TokenType::EXTERN :
            if ( storage_class != StorageClass::None ) {
                throw ParseException( lexer.get_location(), "Storage class already set." );
            }
            storage_class = StorageClass::Extern;
            expect_token( TokenType::EXTERN );
            break;
        case TokenType::INT :
            // Not collecting type information yet.
            if ( type ) {
                throw ParseException( lexer.get_location(), "Type already set." );
            }
            type = true;
            expect_token( TokenType::INT );
            break;
        default :
            throw ParseException( lexer.get_location(), "Unexpected token: {}", to_string( token.tok ) );
        }
        token = lexer.peek_token();
    }

    // Check type found
    if ( !type ) {
        throw ParseException( lexer.get_location(), "Expected type, got identifier" );
    }

    // Get name
    const std::string name = expect_token( TokenType::IDENTIFIER ).value;
    spdlog::debug( "declaration: name {}", name );

    // Determine function or variable
    token = lexer.peek_token();
    if ( token.tok == TokenType::L_PAREN ) {
        // Function declaration
        declaration = functionDef( name, storage_class );
    } else {
        // Variable declaration
        declaration = variableDef( name, storage_class );
    }
    return declaration;
}

void Parser::function_params( ast::FunctionDef f ) {
    spdlog::debug( "function_params" );
    auto token = lexer.peek_token();
    if ( token.tok == TokenType::VOID ) {
        // void
        expect_token( TokenType::VOID );
        f->params.emplace_back( "void" );
        return;
    }
    if ( token.tok == TokenType::R_PAREN ) {
        // No parameters
        return;
    }
    // Parameters
    while ( token.tok != TokenType::R_PAREN ) {
        expect_token( TokenType::INT );
        auto param = expect_token( TokenType::IDENTIFIER );
        f->params.push_back( param.value );

        token = lexer.peek_token();
        if ( token.tok == TokenType::COMMA ) {
            expect_token( TokenType::COMMA );
            token = lexer.peek_token();
            if ( token.tok == TokenType::R_PAREN ) {
                throw ParseException( lexer.get_location(), "Expecting another parameter, got ')'" );
            }
        }
    }
}

ast::FunctionDef Parser::functionDef( std::string const& name, StorageClass storage_class ) {
    auto funct = make_AST<ast::FunctionDef_>();

    funct->name = name;
    funct->storage = storage_class;

    // ( void )
    expect_token( TokenType::L_PAREN );
    function_params( funct );
    expect_token( TokenType::R_PAREN );

    auto token = lexer.peek_token();
    if ( token.tok == TokenType::SEMICOLON ) {
        // Function declaration
        expect_token( TokenType::SEMICOLON );
        funct->block = std::nullopt; // No block for declaration
        return funct;
    }

    funct->block = compound();
    return funct;
}

ast::VariableDef Parser::variableDef( std::string const& name, StorageClass storage_class ) {
    spdlog::debug( "declaration" );
    auto decl = make_AST<ast::VariableDef_>();
    decl->name = name;
    decl->storage = storage_class;

    // check for =
    auto token = lexer.peek_token();
    if ( token.tok == TokenType::EQUALS ) {
        expect_token( TokenType::EQUALS );
        decl->init = expr();
    }
    expect_token( TokenType::SEMICOLON );
    return decl;
}

using StatementParselet = std::function<ast::StatementItem( Parser* )>;
const std::map<TokenType, StatementParselet> statement_map {
    { TokenType::RETURN, []( Parser* p ) -> ast::StatementItem { return p->ret(); } },
    { TokenType::IF, []( Parser* p ) -> ast::StatementItem { return p->if_stat(); } },
    { TokenType::GOTO, []( Parser* p ) -> ast::StatementItem { return p->goto_stat(); } },
    { TokenType::L_BRACE, []( Parser* p ) -> ast::StatementItem { return p->compound(); } },
    { TokenType::SEMICOLON, []( Parser* p ) -> ast::StatementItem { return p->null(); } },
    { TokenType::BREAK, []( Parser* p ) -> ast::StatementItem { return p->break_stat(); } },
    { TokenType::CONTINUE, []( Parser* p ) -> ast::StatementItem { return p->continue_stat(); } },
    { TokenType::WHILE, []( Parser* p ) -> ast::StatementItem { return p->while_stat(); } },
    { TokenType::DO, []( Parser* p ) -> ast::StatementItem { return p->do_while_stat(); } },
    { TokenType::FOR, []( Parser* p ) -> ast::StatementItem { return p->for_stat(); } },
    { TokenType::SWITCH, []( Parser* p ) -> ast::StatementItem { return p->switch_stat(); } },
    { TokenType::CASE, []( Parser* p ) -> ast::StatementItem { return p->case_stat(); } },
    { TokenType::DEFAULT, []( Parser* p ) -> ast::StatementItem { return p->case_stat(); } },
};

ast::Statement Parser::statement() {
    spdlog::debug( "statement" );
    ast::Statement stat = make_AST<ast::Statement_>();

    auto token = lexer.peek_token();

    // Check if the token is a label
    if ( token.tok == TokenType::IDENTIFIER && lexer.peek_token( 1 ).tok == TokenType::COLON ) {
        stat->label = label();
        token = lexer.peek_token(); // Get the next token after the label
    }

    // Check if the next token is also a label, then finish this statement
    if ( token.tok == TokenType::IDENTIFIER && lexer.peek_token( 1 ).tok == TokenType::COLON ) {
        // This is just a label, so we just return the label
        return stat;
    }

    if ( statement_map.contains( token.tok ) ) {
        // Use the parselet for the statement
        stat->statement = statement_map.at( token.tok )( this );
    } else {

        stat->statement = expr();
        expect_token( TokenType::SEMICOLON );
    }
    return stat;
}

ast::Compound Parser::compound() {
    spdlog::debug( "compound" );
    expect_token( TokenType::L_BRACE );
    auto compound = make_AST<ast::Compound_>();

    auto token = lexer.peek_token();
    while ( token.tok != TokenType::R_BRACE ) {
        if ( isTypeOrStorage( token ) ) {
            auto d = declaration();
            if ( std::holds_alternative<ast::FunctionDef>( d ) ) {
                compound->block_items.emplace_back( std::get<ast::FunctionDef>( d ) );
            } else {
                compound->block_items.emplace_back( std::get<ast::VariableDef>( d ) );
            }
        } else {
            ast::BlockItem block = statement();
            compound->block_items.push_back( block );
        }
        token = lexer.peek_token();
    }

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
        // Test for storage specifiers
        if ( isTypeOrStorage( token ) ) {
            // Declaration
            auto d = declaration();
            if ( std::holds_alternative<ast::FunctionDef>( d ) ) {
                throw ParseException( lexer.get_location(), "Function definition not allowed in C17." );
            }
            for_stat->init = std::get<ast::VariableDef>( d );
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
        if ( isTypeOrStorage( token ) ) {
            if ( first ) {
                throw ParseException( lexer.get_location(), "Declaration not allowed in C17." );
            }
            auto d = declaration();
            if ( std::holds_alternative<ast::FunctionDef>( d ) ) {
                throw ParseException( lexer.get_location(), "Function definition not allowed in C17." );
            }
            ast::BlockItem block = std::get<ast::VariableDef>( d );
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
    { TokenType::L_PAREN, []( Parser* p, ast::Expr left ) -> ast::Expr { return p->call( left ); } },
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
    } else if ( token.tok == TokenType::L_PAREN ) {
        // Function call
        return call( ast::Expr( left ) );
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

ast::Call Parser::call( ast::Expr left ) {
    spdlog::debug( "call()" );

    auto token = expect_token( TokenType::L_PAREN );
    auto call = make_AST<ast::Call_>();
    if ( std::holds_alternative<ast::Var>( left ) ) {
        call->function_name = std::get<ast::Var>( left )->name;
    } else {
        throw ParseException( token.location, "Expected variable for function call." );
    }

    token = lexer.peek_token();
    while ( token.tok != TokenType::R_PAREN ) {
        ast::Expr e = expr( Precedence::Lowest );
        call->arguments.push_back( e );
        token = lexer.peek_token();
        if ( token.tok == TokenType::COMMA ) {
            expect_token( TokenType::COMMA );
            token = lexer.peek_token();
            if ( token.tok == TokenType::R_PAREN ) {
                throw ParseException( lexer.get_location(), "Expected another argument, got ')'" );
            }
        } else if ( token.tok != TokenType::R_PAREN ) {
            throw ParseException( token.location, "Expected ',' or ')', got {}", to_string( token ) );
        }
    }
    expect_token( TokenType::R_PAREN );
    return call;
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