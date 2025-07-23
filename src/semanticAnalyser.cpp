//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 22/7/2025.
//

#include "semanticAnalyser.h"

#include "ast/includes.h"
#include "common.h"
#include "exception.h"
#include "spdlog/spdlog.h"

void SemanticAnalyser::analyse( const ast::Program ast ) {
    ast->accept( this );
}

void SemanticAnalyser::visit_Program( const ast::Program ast ) {
    ast->function->accept( this );
}

void SemanticAnalyser::visit_FunctionDef( const ast::FunctionDef ast ) {
    for ( ast::BlockItem b : ast->block_items ) {
        std::visit( overloaded { [ this ]( ast::Declaration ast ) -> void { ast->accept( this ); },
                                 [ this ]( ast::Statement ast ) -> void { statement( ast ); } },
                    b );
    }
}

void SemanticAnalyser::statement( const ast::Statement ast ) {
    std::visit( overloaded { [ this ]( ast::Return ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Expr e ) -> void { expr( e ); }, // expr
                             [ this ]( ast::Null ) -> void { ; } },
                ast );
}

void SemanticAnalyser::visit_Declaration( const ast::Declaration ast ) {
    if ( symbol_table.find( ast->name ) ) {
        throw SemanticException( ast->location, "Duplicate declaration: {}", ast->name );
    }
    auto unique_name = symbol_table.temp_name( ast->name );
    symbol_table.put( ast->name, unique_name );
    ast->name = unique_name;
    if ( ast->init ) {
        expr( ast->init.value() );
    }
}

void SemanticAnalyser::visit_Return( const ast::Return ast ) {
    expr( ast->expr );
}

void SemanticAnalyser::expr( const ast::Expr ast ) {
    std::visit( overloaded { [ this ]( ast::UnaryOp u ) -> void { u->accept( this ); },
                             [ this ]( ast::BinaryOp b ) -> void { b->accept( this ); },
                             [ this ]( ast::Assign a ) -> void { a->accept( this ); },
                             [ this ]( ast::Var v ) -> void { v->accept( this ); },
                             [ this ]( ast::Constant c ) -> void { ; } },
                ast );
}

void SemanticAnalyser::visit_UnaryOp( const ast::UnaryOp ast ) {
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        // operand must be a variable
        if ( !std::holds_alternative<ast::Var>( ast->operand ) ) {
            throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
        }
    }
    expr( ast->operand );
}

void SemanticAnalyser::visit_BinaryOp( const ast::BinaryOp ast ) {
    // Check left side for postfix increment/decrement
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        if ( !std::holds_alternative<ast::Var>( ast->left ) ) {
            throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
        }
        expr( ast->left );
        return;
    }
    // Other operators
    expr( ast->left );
    expr( ast->right );
}

void SemanticAnalyser::visit_Assign( const ast::Assign ast ) {
    if ( !std::holds_alternative<ast::Var>( ast->left ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {}", ast->op );
    }
    expr( ast->left );
    expr( ast->right );
}

void SemanticAnalyser::visit_Var( const ast::Var ast ) {
    if ( auto name = symbol_table.find( ast->name ) ) {
        spdlog::debug( "Found var: {} for {}", *name, ast->name );
        ast->name = *name; // Change the name to the temporary.
        return;
    }
    throw SemanticException( ast->location, "variable: {} not declared", ast->name );
}