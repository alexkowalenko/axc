//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 22/7/2025.
//

#include "semanticAnalyser.h"

#include <algorithm>
#include <set>

#include "ast/includes.h"
#include "common.h"
#include "exception.h"
#include "spdlog/spdlog.h"

SemanticAnalyser::SemanticAnalyser() {};

void SemanticAnalyser::analyse( const ast::Program ast, SymbolTable& table ) {
    visit_Program( ast, table );
}

void SemanticAnalyser::visit_Program( const ast::Program ast, SymbolTable& table ) {
    for ( const auto& function : ast->functions ) {
        // top level
        nested_function = false;
        visit_FunctionDef( function, table );
    }
}

void SemanticAnalyser::visit_FunctionDef( ast::FunctionDef ast, SymbolTable& table ) {
    spdlog::debug( "Function: {}", ast->name );
    // Clear the labels for each function.
    labels.clear();

    // remove "void" parameters
    ast->params.erase( std::remove( ast->params.begin(), ast->params.end(), "void" ), ast->params.end() );

    auto symbol = table.find( ast->name );
    spdlog::debug( "symbol linkage: {:d} here: {}", static_cast<int>( symbol->linkage ), symbol->current_scope );
    if ( symbol ) {

        if ( symbol->type != Type::FUNCTION ) {
            // Another symbol is already defined with the same name, but it is not a function.
            symbol = std::nullopt;
            spdlog::debug( "Symbol {} is not a function, reusing symbol", ast->name );
        } else {
            // Function with the same name already exists.
            spdlog::debug( "Function {} already exists", ast->name );
            if ( symbol->linkage == Linkage::None && symbol->current_scope ) {
                throw SemanticException( ast->location, "Duplicate declaration: {}", ast->name );
            }
            if ( symbol->linkage == Linkage::Internal ) {
                // If the function is internal, it should not be declared again.
                throw SemanticException( ast->location, "Duplicate declaration: {}", ast->name );
            }

            if ( symbol->number != ast->params.size() ) {
                // If the number of parameters does not match, we will throw an exception later.
                spdlog::debug( "Function {} has {} parameters, but got {}", ast->name, symbol->number,
                               ast->params.size() );
            }

            spdlog::debug( "count: {} ", ast->params.size() );

            if ( symbol->number != ast->params.size() ) {
                throw SemanticException( ast->location, "Function {} expects {} arguments, but got {}.", ast->name,
                                         symbol->number, ast->params.size() );
            }
        }
    }

    auto s = symbol.value_or( Symbol { .name = ast->name, .type = Type::FUNCTION } );

    // Check if the function is defined as a nested function.
    if ( auto f = function_table.find( ast->name ) ) {
        spdlog::debug( "Function {} is defined as a nested function", ast->name );

        // type check it
        if ( f->number != ast->params.size() ) {
            throw SemanticException( ast->location, "Function {} has {} parameters, but got {}", ast->name, f->number,
                                     ast->params.size() );
        }
    }

    // check parameter names unique
    std::set<std::string> param_names;
    for ( const auto& param : ast->params ) {
        if ( param_names.contains( param ) ) {
            throw SemanticException( ast->location, "Duplicate parameter name: {}", param );
        }
        param_names.insert( param );
    }
    s.number = param_names.size();

    if ( ast->block ) {

        if ( nested_function ) {
            throw SemanticException( ast->location, "Nested functions are not allowed" );
        }

        // Add symbol with function name to the symbol table.
        s.linkage = Linkage::Internal;
        table.put( ast->name, s );

        // Create new scope
        auto new_table = new_scope( table );

        // Add parameters to the symbol table.
        for ( auto& param : ast->params ) {
            auto unique_name = table.temp_name( param );
            spdlog::debug( "Declaring param: {} as {}", param, unique_name );
            new_table.put(
                param,
                Symbol { .name = unique_name, .linkage = Linkage::None, .type = Type::INT, .current_scope = true } );
            param = unique_name;
        }

        nested_function = true;
        visit_Compound( ast->block.value(), new_table );
    } else {
        // If there is no block, it is a function declaration.
        s.linkage = Linkage::External;
        s.current_scope = true;
        table.put( ast->name, s );
        function_table.put( ast->name, s );
    }

    // Check for labels that were used but not defined.
    for ( auto [ label, defined ] : labels ) {
        if ( !defined ) {
            throw SemanticException( ast->location, "Label {} not defined", label );
        }
    }
}

void SemanticAnalyser::visit_Statement( const ast::Statement ast, SymbolTable& table ) {
    spdlog::debug( "Statement: {}" );
    if ( ast->label ) {
        visit_Label( ast->label.value() );
    }

    if ( ast->statement ) {
        statement( ast->statement.value(), table );
    }
}

void SemanticAnalyser::statement( const ast::StatementItem ast, SymbolTable& table ) {
    spdlog::debug( "statement: {}" );
    std::visit( overloaded { [ this, &table ]( ast::Return ast ) -> void { visit_Return( ast, table ); },
                             [ this, &table ]( ast::If ast ) -> void { visit_If( ast, table ); },
                             [ this ]( ast::Goto ast ) -> void { visit_Goto( ast ); },
                             [ this, &table ]( ast::Break ast ) -> void { visit_Break( ast, table ); },
                             [ this, &table ]( ast::Continue ast ) -> void { visit_Continue( ast, table ); },
                             [ this, &table ]( ast::While ast ) -> void { visit_While( ast, table ); },
                             [ this, &table ]( ast::DoWhile ast ) -> void { visit_DoWhile( ast, table ); },
                             [ this, &table ]( ast::For ast ) -> void { visit_For( ast, table ); },
                             [ this, &table ]( ast::Switch ast ) -> void { visit_Switch( ast, table ); },
                             [ this, &table ]( ast::Case ast ) -> void { visit_Case( ast, table ); },
                             [ this, &table ]( ast::Compound ast ) -> void {
                                 // Create new scope
                                 auto new_table = new_scope( table );

                                 visit_Compound( ast, new_table );
                             },
                             [ this, &table ]( ast::Expr e ) -> void { expr( e, table ); }, // expr
                             [ this ]( ast::Null ) -> void { ; } },
                ast );
}

void SemanticAnalyser::visit_Declaration( const ast::Declaration ast, SymbolTable& table ) {
    spdlog::debug( "Declaration: {}", ast->name );
    if ( auto symbol = table.find( ast->name ); symbol && symbol->current_scope ) {
        throw SemanticException( ast->location, "Duplicate declaration: {}", ast->name );
    }
    auto unique_name = table.temp_name( ast->name );
    spdlog::debug( "Declaring variable: {} as {}", ast->name, unique_name );
    table.put( ast->name,
               Symbol { .name = unique_name, .linkage = Linkage::None, .type = Type::INT, .current_scope = true } );
    ast->name = unique_name;
    if ( ast->init ) {
        expr( ast->init.value(), table );
    }
}

void SemanticAnalyser::visit_If( const ast::If ast, SymbolTable& table ) {
    expr( ast->condition, table );
    visit_Statement( ast->then, table );
    if ( ast->else_stat ) {
        visit_Statement( ast->else_stat.value(), table );
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

void SemanticAnalyser::visit_Return( const ast::Return ast, SymbolTable& table ) {
    expr( ast->expr, table );
}

void SemanticAnalyser::visit_Break( const ast::Break ast, SymbolTable& table ) {
    if ( loop_count == 0 && switch_count == 0 ) {
        throw SemanticException( ast->location, "break statement not in loop or switch statement" );
    }
    if ( last_break == TokenType::SWITCH ) {
        switch_label( ast );
    } else {
        loop_label( ast );
    }
}

void SemanticAnalyser::visit_Continue( const ast::Continue ast, SymbolTable& table ) {
    if ( loop_count == 0 ) {
        throw SemanticException( ast->location, "continue statement not in loop" );
    }
    loop_label( ast );
}

void SemanticAnalyser::visit_While( const ast::While ast, SymbolTable& table ) {
    last_break = TokenType::WHILE;
    expr( ast->condition, table );
    new_loop_label( ast );
    visit_Statement( ast->body, table );
}

void SemanticAnalyser::visit_DoWhile( const ast::DoWhile ast, SymbolTable& table ) {
    last_break = TokenType::DO;
    new_loop_label( ast );
    visit_Statement( ast->body, table );
    expr( ast->condition, table );
}

void SemanticAnalyser::for_init( ast::ForInit ast, SymbolTable& table ) {
    std::visit( overloaded { [ this, &table ]( ast::Expr e ) -> void { expr( e, table ); },
                             [ this, &table ]( ast::Declaration d ) -> void { visit_Declaration( d, table ); } },
                ast );
}

void SemanticAnalyser::visit_For( const ast::For ast, SymbolTable& table ) {
    last_break = TokenType::FOR;
    // Create new symbol table and swap
    auto new_table = new_scope( table );

    if ( ast->init ) {
        for_init( ast->init.value(), new_table );
    }
    if ( ast->condition ) {
        expr( ast->condition.value(), new_table );
    }
    if ( ast->increment ) {
        expr( ast->increment.value(), new_table );
    }
    new_loop_label( ast );
    visit_Statement( ast->body, new_table );
}

void SemanticAnalyser::visit_Switch( const ast::Switch ast, SymbolTable& table ) {
    last_break = TokenType::SWITCH;
    expr( ast->condition, table );

    new_switch_label( ast );
    // Reset case set for each switch
    std::set<ast::Constant> current_case_set;
    case_set.push( current_case_set );

    // Add the current switch to the switch stack
    switch_stack.push( ast );

    visit_Statement( ast->body, table );

    // Clear default case tracking
    if ( !last_default.empty() && last_default.top() == switch_count ) {
        last_default.pop();
    }
    // Pop the case set for this switch
    case_set.pop();
    // Pop the switch stack
    switch_stack.pop();
}

void SemanticAnalyser::visit_Case( const ast::Case ast, SymbolTable& table ) {
    spdlog::debug( "case: {}", ast->is_default ? "default" : "case" );

    // Check if we are in a switch statement
    if ( case_set.empty() ) {
        throw SemanticException( ast->location, "'{}' statement not in switch", ast->is_default ? "default" : "case" );
    }

    if ( !ast->is_default ) {
        // case:

        // constant expressions for case
        is_constant = false;
        expr( ast->value, table );
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
        std::visit( overloaded { [ this, &table ]( ast::Declaration d ) -> void { visit_Declaration( d, table ); },
                                 [ this ]( ast::FunctionDef f ) -> void {
                                     throw SemanticException( f->location, "No functions in case blocks" );
                                 },
                                 [ this, &table ]( ast::Statement s ) -> void { visit_Statement( s, table ); } },
                    item );
    }
}

void SemanticAnalyser::visit_Compound( const ast::Compound ast, SymbolTable& table ) {
    spdlog::debug( "Compound" );

    for ( const auto& item : ast->block_items ) {
        std::visit( overloaded { [ this, &table ]( ast::Declaration d ) -> void { visit_Declaration( d, table ); },
                                 [ this, &table ]( ast::FunctionDef f ) -> void { visit_FunctionDef( f, table ); },
                                 [ this, &table ]( ast::Statement s ) -> void { visit_Statement( s, table ); } },
                    item );
    }
}

void SemanticAnalyser::expr( const ast::Expr ast, SymbolTable& table ) {
    std::visit( overloaded {
                    [ this, &table ]( ast::UnaryOp u ) -> void { visit_UnaryOp( u, table ); },
                    [ this, &table ]( ast::BinaryOp b ) -> void { visit_BinaryOp( b, table ); },
                    [ this, &table ]( ast::PostOp b ) -> void { visit_PostOp( b, table ); },
                    [ this, &table ]( ast::Conditional b ) -> void { visit_Conditional( b, table ); },
                    [ this, &table ]( ast::Assign a ) -> void { visit_Assign( a, table ); },
                    [ this, &table ]( ast::Call c ) -> void { visit_Call( c, table ); },
                    [ this, &table ]( ast::Var v ) -> void { visit_Var( v, table ); },
                    [ this ]( ast::Constant c ) -> void { visit_Constant( c ); },
                },
                ast );
}

void SemanticAnalyser::visit_UnaryOp( const ast::UnaryOp ast, SymbolTable& table ) {
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        // operand must be a variable
        if ( !std::holds_alternative<ast::Var>( ast->operand ) ) {
            throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
        }
    }
    expr( ast->operand, table );

    // Constant Analysis
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        // Postfix increment/decrement is not constant
        is_constant = false;
    } else {
        // Unary operators are constant
        is_constant = true;
    }
}

void SemanticAnalyser::visit_BinaryOp( const ast::BinaryOp ast, SymbolTable& table ) {
    expr( ast->left, table );
    bool left_constant = is_constant;
    expr( ast->right, table );
    bool right_constant = is_constant;
    // Constant Analysis
    is_constant = left_constant && right_constant;
}

void SemanticAnalyser::visit_PostOp( const ast::PostOp ast, SymbolTable& table ) {
    // Check left side for postfix increment/decrement
    if ( !std::holds_alternative<ast::Var>( ast->operand ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
    }
    expr( ast->operand, table );
    // Constant Analysis - postfix operators are not constant
    is_constant = false;
}

void SemanticAnalyser::visit_Conditional( const ast::Conditional ast, SymbolTable& table ) {
    expr( ast->condition, table );
    expr( ast->then_expr, table );
    bool left_constant = is_constant;
    expr( ast->else_expr, table );
    bool right_constant = is_constant;
    // Constant Analysis
    is_constant = left_constant && right_constant;
}

void SemanticAnalyser::visit_Assign( const ast::Assign ast, SymbolTable& table ) {
    if ( !std::holds_alternative<ast::Var>( ast->left ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {}", ast->op );
    }
    expr( ast->left, table );
    expr( ast->right, table );
    // Constant Analysis
    is_constant = false; // Assignment is never constant
}

void SemanticAnalyser::visit_Call( const ast::Call ast, SymbolTable& table ) {
    // Check if the function is declared
    auto symbol = table.find( ast->function_name );
    if ( !symbol || symbol->type != Type::FUNCTION ) {
        throw SemanticException( ast->location, "Function {} not declared", ast->function_name );
    }

    if ( symbol->number != ast->arguments.size() ) {
        throw SemanticException( ast->location, "Function {} expects {} arguments, but got {}", ast->function_name,
                                 symbol->number, ast->arguments.size() );
    }

    for ( const auto& arg : ast->arguments ) {
        expr( arg, table );
    }
    // Constant Analysis
    is_constant = false; // Function calls are not constant
}

void SemanticAnalyser::visit_Var( const ast::Var ast, SymbolTable& table ) {
    if ( auto name = table.find( ast->name ) ) {

        if ( name->type == Type::FUNCTION ) {
            throw SemanticException( ast->location, "Variable {} cannot be of type function", ast->name );
        }

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

SymbolTable SemanticAnalyser::new_scope( SymbolTable& table ) {
    SymbolTable new_table;
    new_table.copy( table );
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
