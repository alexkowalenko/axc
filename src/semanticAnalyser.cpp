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

    // Clear the labels for each function.
    labels.clear();

    ast->block->accept( this );

    // Check for labels that were used but not defined.
    for ( auto [ label, defined ] : labels ) {
        if ( !defined ) {
            throw SemanticException( ast->location, "Label {} not defined", label );
        }
    }
}

void SemanticAnalyser::statement( const ast::Statement ast ) {
    std::visit( overloaded { [ this ]( ast::Return ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::If ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Goto ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Label ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Compound ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Expr e ) -> void { expr( e ); }, // expr
                             [ this ]( ast::Null ) -> void { ; } },
                ast );
}

void SemanticAnalyser::visit_Declaration( const ast::Declaration ast ) {
    if ( auto symbol = symbol_table.find( ast->name ); symbol && symbol->current_block ) {
        throw SemanticException( ast->location, "Duplicate declaration: {}", ast->name );
    }
    auto unique_name = symbol_table.temp_name( ast->name );
    symbol_table.put( ast->name, Symbol { unique_name, true } );
    ast->name = unique_name;
    if ( ast->init ) {
        expr( ast->init.value() );
    }
}

void SemanticAnalyser::visit_If( const ast::If ast ) {
    expr( ast->condition );
    statement( ast->then );
    if ( ast->else_stat ) {
        statement( ast->else_stat.value() );
    }
}

void SemanticAnalyser::visit_Goto( const ast::Goto ast ) {
    if ( !labels.contains( ast->label ) ) {
        // If the label is not defined, we will throw an exception later.
        labels[ ast->label ] = false;
    }
}

void SemanticAnalyser::visit_Label( const ast::Label ast ) {
    if ( labels.contains( ast->label ) && labels[ ast->label ] == true ) {
        throw SemanticException( ast->location, "Duplicate label in function: {}", ast->label );
    }
    labels[ ast->label ] = true;
}

void SemanticAnalyser::visit_Return( const ast::Return ast ) {
    expr( ast->expr );
}

void SemanticAnalyser::visit_Compound( const ast::Compound ast ) {
    // Create new map and swap
    auto previous_table = symbol_table;
    symbol_table = new_scope();

    for ( const auto& item : ast->block_items ) {
        std::visit( overloaded { [ this ]( ast::Declaration d ) -> void { d->accept( this ); },
                                 [ this ]( ast::Statement s ) -> void { statement( s ); } },
                    item );
    }
    symbol_table = previous_table;
}

void SemanticAnalyser::expr( const ast::Expr ast ) {
    std::visit( overloaded { [ this ]( ast::UnaryOp u ) -> void { u->accept( this ); },
                             [ this ]( ast::BinaryOp b ) -> void { b->accept( this ); },
                             [ this ]( ast::PostOp b ) -> void { b->accept( this ); },
                             [ this ]( ast::Conditional b ) -> void { b->accept( this ); },
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
    expr( ast->left );
    expr( ast->right );
}

void SemanticAnalyser::visit_PostOp( const ast::PostOp ast ) {
    // Check left side for postfix increment/decrement
    if ( !std::holds_alternative<ast::Var>( ast->operand ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
    }
    expr( ast->operand );
}

void SemanticAnalyser::visit_Conditional( const ast::Conditional ast ) {
    expr( ast->condition );
    expr( ast->then_expr );
    expr( ast->else_expr );
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
        spdlog::debug( "Found var: {} for {}", name->name, ast->name );
        ast->name = name->name; // Change the name to the temporary.
        return;
    }
    throw SemanticException( ast->location, "variable: {} not declared", ast->name );
}

SymbolTable SemanticAnalyser::new_scope() {
    SymbolTable new_table;
    new_table.copy( symbol_table );
    new_table.reset_current_block();
    return new_table;
}