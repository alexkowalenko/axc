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
    for ( const auto& function : ast->functions ) {
        // top level
        nested_function = false;
        function->accept( this );
    }
}

void SemanticAnalyser::visit_FunctionDef( const ast::FunctionDef ast ) {
    spdlog::debug( "Function: {}", ast->name );
    // Clear the labels for each function.
    labels.clear();

    auto symbol = symbol_table.find( ast->name );
    spdlog::debug( "symbol linkage: {:d} here: {}", static_cast<int>( symbol->linkage ), symbol->current_scope );
    if ( symbol && symbol->linkage == Linkage::None && symbol->current_scope ) {
        throw SemanticException( ast->location, "Duplicate declaration: {}", ast->name );
    }

    auto s = symbol ? *symbol : Symbol { .name = ast->name };

    // check parameter names unique
    std::set<std::string> param_names;
    for ( const auto& param : ast->params ) {
        if ( param_names.contains( param ) ) {
            throw SemanticException( ast->location, "Duplicate parameter name: {}", param );
        }
        param_names.insert( param );
    }

    if ( ast->block ) {

        if ( nested_function ) {
            throw SemanticException( ast->location, "Nested functions are not allowed" );
        }

        // Add symbol with function name to the symbol table.
        s.linkage = Linkage::Internal;
        symbol_table.put( ast->name, s );

        // Create new scope
        auto previous_table = symbol_table;
        symbol_table = new_scope();

        // Add parameters to the symbol table.
        for ( const auto& param : ast->params ) {
            symbol_table.put( param, Symbol { param, Linkage::None, true } );
        }

        nested_function = true;
        ast->block.value()->accept( this );

        // Restore previous symbol table
        symbol_table = previous_table;
    } else {
        // If there is no block, it is a function declaration.
        s.linkage = Linkage::External;
        s.current_scope = true;
        symbol_table.put( ast->name, s );
    }

    // Check for labels that were used but not defined.
    for ( auto [ label, defined ] : labels ) {
        if ( !defined ) {
            throw SemanticException( ast->location, "Label {} not defined", label );
        }
    }
}

void SemanticAnalyser::visit_Statement( const ast::Statement ast ) {
    spdlog::debug( "Statement: {}" );
    if ( ast->label ) {
        ast->label.value()->accept( this );
    }

    if ( ast->statement ) {
        statement( ast->statement.value() );
    }
}

void SemanticAnalyser::statement( const ast::StatementItem ast ) {
    spdlog::debug( "statement: {}" );
    std::visit( overloaded { [ this ]( ast::Return ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::If ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Goto ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Break ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Continue ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::While ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::DoWhile ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::For ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Switch ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Case ast ) -> void { ast->accept( this ); },
                             [ this ]( ast::Compound ast ) -> void {
                                 // Create new scope
                                 auto previous_table = symbol_table;
                                 symbol_table = new_scope();

                                 ast->accept( this );

                                 // Restore previous symbol table
                                 symbol_table = previous_table;
                             },
                             [ this ]( ast::Expr e ) -> void { expr( e ); }, // expr
                             [ this ]( ast::Null ) -> void { ; } },
                ast );
}

void SemanticAnalyser::visit_Declaration( const ast::Declaration ast ) {
    spdlog::debug( "Declaration: {}", ast->name );
    if ( auto symbol = symbol_table.find( ast->name ); symbol && symbol->current_scope ) {
        throw SemanticException( ast->location, "Duplicate declaration: {}", ast->name );
    }
    auto unique_name = symbol_table.temp_name( ast->name );
    spdlog::debug( "Declaring variable: {} as {}", ast->name, unique_name );
    symbol_table.put( ast->name, Symbol { unique_name, Linkage::None, true } );
    ast->name = unique_name;
    if ( ast->init ) {
        expr( ast->init.value() );
    }
}

void SemanticAnalyser::visit_If( const ast::If ast ) {
    expr( ast->condition );
    ast->then->accept( this );
    if ( ast->else_stat ) {
        ast->else_stat.value()->accept( this );
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
    if ( last_break == TokenType::SWITCH ) {
        switch_label( ast );
    } else {
        loop_label( ast );
    }
}

void SemanticAnalyser::visit_Continue( const ast::Continue ast ) {
    if ( loop_count == 0 ) {
        throw SemanticException( ast->location, "continue statement not in loop" );
    }
    loop_label( ast );
}

void SemanticAnalyser::visit_While( const ast::While ast ) {
    last_break = TokenType::WHILE;
    expr( ast->condition );
    new_loop_label( ast );
    ast->body->accept( this );
}

void SemanticAnalyser::visit_DoWhile( const ast::DoWhile ast ) {
    last_break = TokenType::DO;
    new_loop_label( ast );
    ast->body->accept( this );
    expr( ast->condition );
}

void SemanticAnalyser::for_init( ast::ForInit ast ) {
    std::visit( overloaded { [ this ]( ast::Expr e ) -> void { expr( e ); },
                             [ this ]( ast::Declaration d ) -> void { d->accept( this ); } },
                ast );
}

void SemanticAnalyser::visit_For( const ast::For ast ) {
    last_break = TokenType::FOR;
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
    ast->body->accept( this );
    // Restore previous symbol table
    symbol_table = previous_table;
}

void SemanticAnalyser::visit_Switch( const ast::Switch ast ) {
    last_break = TokenType::SWITCH;
    expr( ast->condition );

    new_switch_label( ast );
    // Reset case set for each switch
    std::set<ast::Constant> current_case_set;
    case_set.push( current_case_set );

    // Add the current switch to the switch stack
    switch_stack.push( ast );

    ast->body->accept( this );

    // Clear default case tracking
    if ( !last_default.empty() && last_default.top() == switch_count ) {
        last_default.pop();
    }
    // Pop the case set for this switch
    case_set.pop();
    // Pop the switch stack
    switch_stack.pop();
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
    switch_label( ast );
    // Add the case to the current switch
    switch_stack.top()->cases.push_back( ast );

    for ( const auto& item : ast->block_items ) {
        std::visit( overloaded { [ this ]( ast::Declaration d ) -> void { d->accept( this ); },
                                 [ this ]( ast::FunctionDef f ) -> void {
                                     throw SemanticException( f->location, "No functions in case blocks" );
                                 },
                                 [ this ]( ast::Statement s ) -> void { s->accept( this ); } },
                    item );
    }
}

void SemanticAnalyser::visit_Compound( const ast::Compound ast ) {
    spdlog::debug( "Compound" );

    for ( const auto& item : ast->block_items ) {
        std::visit( overloaded { [ this ]( ast::Declaration d ) -> void { d->accept( this ); },
                                 [ this ]( ast::FunctionDef f ) -> void { f->accept( this ); },
                                 [ this ]( ast::Statement s ) -> void { s->accept( this ); } },
                    item );
    }
}

void SemanticAnalyser::expr( const ast::Expr ast ) {
    std::visit( overloaded { [ this ]( ast::UnaryOp u ) -> void { u->accept( this ); },
                             [ this ]( ast::BinaryOp b ) -> void { b->accept( this ); },
                             [ this ]( ast::PostOp b ) -> void { b->accept( this ); },
                             [ this ]( ast::Conditional b ) -> void { b->accept( this ); },
                             [ this ]( ast::Assign a ) -> void { a->accept( this ); },
                             [ this ]( ast::Call c ) -> void { c->accept( this ); },
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

void SemanticAnalyser::visit_Call( const ast::Call ast ) {
    // Check if the function is declared
    if ( !symbol_table.find( ast->function_name ) ) {
        throw SemanticException( ast->location, "Function {} not declared", ast->function_name );
    }

    for ( const auto& arg : ast->arguments ) {
        expr( arg );
    }
    // Constant Analysis
    is_constant = false; // Function calls are not constant
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

void SemanticAnalyser::switch_label( std::shared_ptr<ast::Base> b ) {
    b->ast_label = std::format( "switch.{}", switch_count );
}
