//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 22/7/2025.
//

#include "semanticAnalyser.h"

#include <set>

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
                             [ this ]( ast::Break ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Continue ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::While ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::DoWhile ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::For ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Switch ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Case ast ) -> void { ast->accept( this ); },
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
    spdlog::debug( "Declaring variable: {} as {}", ast->name, unique_name );
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
    spdlog::debug( "Label: {}", ast->label );
    if ( labels.contains( ast->label ) && labels[ ast->label ] == true ) {
        throw SemanticException( ast->location, "Duplicate label in function: {}", ast->label );
    }
    labels[ ast->label ] = true;
}

void SemanticAnalyser::visit_Return( const ast::Return ast ) {
    expr( ast->expr );
}

void SemanticAnalyser::visit_Break( const ast::Break ast ) {
    if ( loop_count == 0 && switch_count == 0 ) {
        throw SemanticException( ast->location, "break statement not in loop or switch statement" );
    }
    loop_label( ast );
}

void SemanticAnalyser::visit_Continue( const ast::Continue ast ) {
    if ( loop_count == 0 ) {
        throw SemanticException( ast->location, "continue statement not in loop" );
    }
    loop_label( ast );
}

void SemanticAnalyser::visit_While( const ast::While ast ) {
    expr( ast->condition );
    new_loop_label( ast );
    statement( ast->body );
}

void SemanticAnalyser::visit_DoWhile( const ast::DoWhile ast ) {
    new_loop_label( ast );
    statement( ast->body );
    expr( ast->condition );
}

void SemanticAnalyser::for_init( ast::ForInit ast ) {
    std::visit( overloaded { [ this ]( ast::Expr e ) -> void { expr( e ); },
                             [ this ]( ast::Declaration d ) -> void { d->accept( this ); } },
                ast );
}

void SemanticAnalyser::visit_For( const ast::For ast ) {
    // Create new symbol table and swap
    auto previous_table = symbol_table;
    symbol_table = new_scope();

    if ( ast->init ) {
        for_init( ast->init.value() );
    }
    if ( ast->condition ) {
        expr( ast->condition.value() );
    }
    if ( ast->increment ) {
        expr( ast->increment.value() );
    }
    new_loop_label( ast );
    statement( ast->body );
    // Restore previous symbol table
    symbol_table = previous_table;
}

void SemanticAnalyser::visit_Switch( const ast::Switch ast ) {
    expr( ast->condition );

    new_switch_label( ast );
    // Reset case set for each switch
    std::set<ast::Constant> current_case_set;
    case_set.push( current_case_set );

    statement( ast->body );

    // Clear default case tracking
    if ( !last_default.empty() && last_default.top() == switch_count ) {
        last_default.pop();
    }
    // Pop the case set for this switch
    case_set.pop();
}

void SemanticAnalyser::visit_Case( const ast::Case ast ) {
    spdlog::debug( "case: {}", ast->is_default ? "default" : "case" );

    // Check if we are in a switch statement
    if ( case_set.empty() ) {
        throw SemanticException( ast->location, "'{}' statement not in switch", ast->is_default ? "default" : "case" );
    }

    if ( !ast->is_default ) {
        // case:

        // constant expressions for case
        is_constant = false;
        expr( ast->value );
        if ( !is_constant ) {
            throw SemanticException( ast->location, "Case value must be constant expression" );
        }

        // Check for duplicate case values
        // Check only ast::Constant - skip expressions
        if ( auto const_value = std::get_if<ast::Constant>( &ast->value ) ) {
            if ( case_set.top().contains( *const_value ) ) {
                throw SemanticException( ast->location, "Duplicate case value " );
            }
            case_set.top().insert( *const_value );
        }

    } else {
        // default:
        if ( !last_default.empty() && last_default.top() == switch_count ) {
            throw SemanticException( ast->location, "Duplicate default case in switch statement" );
        }
        last_default.push( switch_count );
    }

    for ( const auto& item : ast->block_items ) {
        std::visit( overloaded { [ this ]( ast::Declaration d ) -> void { d->accept( this ); },
                                 [ this ]( ast::Statement s ) -> void { statement( s ); } },
                    item );
    }
}

void SemanticAnalyser::visit_Compound( const ast::Compound ast ) {
    // Create new symbol table and swap
    auto previous_table = symbol_table;
    symbol_table = new_scope();

    for ( const auto& item : ast->block_items ) {
        std::visit( overloaded { [ this ]( ast::Declaration d ) -> void { d->accept( this ); },
                                 [ this ]( ast::Statement s ) -> void { statement( s ); } },
                    item );
    }
    // Restore previous symbol table
    symbol_table = previous_table;
}

void SemanticAnalyser::expr( const ast::Expr ast ) {
    std::visit( overloaded { [ this ]( ast::UnaryOp u ) -> void { u->accept( this ); },
                             [ this ]( ast::BinaryOp b ) -> void { b->accept( this ); },
                             [ this ]( ast::PostOp b ) -> void { b->accept( this ); },
                             [ this ]( ast::Conditional b ) -> void { b->accept( this ); },
                             [ this ]( ast::Assign a ) -> void { a->accept( this ); },
                             [ this ]( ast::Var v ) -> void { v->accept( this ); },
                             [ this ]( ast::Constant c ) -> void { c->accept( this ); } },
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

    // Constant Analysis
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        // Postfix increment/decrement is not constant
        is_constant = false;
    } else {
        // Unary operators are constant
        is_constant = true;
    }
}

void SemanticAnalyser::visit_BinaryOp( const ast::BinaryOp ast ) {
    expr( ast->left );
    bool left_constant = is_constant;
    expr( ast->right );
    bool right_constant = is_constant;
    // Constant Analysis
    is_constant = left_constant && right_constant;
}

void SemanticAnalyser::visit_PostOp( const ast::PostOp ast ) {
    // Check left side for postfix increment/decrement
    if ( !std::holds_alternative<ast::Var>( ast->operand ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
    }
    expr( ast->operand );
    // Constant Analysis - postfix operators are not constant
    is_constant = false;
}

void SemanticAnalyser::visit_Conditional( const ast::Conditional ast ) {
    expr( ast->condition );
    expr( ast->then_expr );
    bool left_constant = is_constant;
    expr( ast->else_expr );
    bool right_constant = is_constant;
    // Constant Analysis
    is_constant = left_constant && right_constant;
}

void SemanticAnalyser::visit_Assign( const ast::Assign ast ) {
    if ( !std::holds_alternative<ast::Var>( ast->left ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {}", ast->op );
    }
    expr( ast->left );
    expr( ast->right );
    // Constant Analysis
    is_constant = false; // Assignment is never constant
}

void SemanticAnalyser::visit_Var( const ast::Var ast ) {
    if ( auto name = symbol_table.find( ast->name ) ) {
        spdlog::debug( "Found var: {} for {}", name->name, ast->name );
        ast->name = name->name; // Change the name to the temporary.

        // Constant Analysis
        is_constant = false;

        return;
    }
    throw SemanticException( ast->location, "variable: {} not declared", ast->name );
}

void SemanticAnalyser::visit_Constant( ast::Constant ast ) {
    // Constant Analysis
    is_constant = true;
}

SymbolTable SemanticAnalyser::new_scope() {
    SymbolTable new_table;
    new_table.copy( symbol_table );
    new_table.reset_current_block();
    return new_table;
}

void SemanticAnalyser::new_loop_label( std::shared_ptr<ast::Base> b ) {
    b->ast_label = std::format( "loop.{}", ++loop_count );
}

void SemanticAnalyser::loop_label( std::shared_ptr<ast::Base> b ) {
    b->ast_label = std::format( "loop.{}", loop_count );
}

void SemanticAnalyser::new_switch_label( std::shared_ptr<ast::Base> b ) {
    b->ast_label = std::format( "switch.{}", ++switch_count );
}
