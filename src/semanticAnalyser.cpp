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

#include "spdlog/spdlog.h"

#include "ast/includes.h"
#include "common.h"
#include "enumerate.h"
#include "exception.h"

std::optional<std::int64_t> get_constant( const ast::Expr ast ) {
    if ( auto c = std::get_if<ast::Constant>( &ast ); c ) {
        if ( auto i = std::get_if<ast::ConstantInt>( c ) ) {
            return ( *i )->value;
        } else if ( auto l = std::get_if<ast::ConstantLong>( c ) ) {
            return ( *l )->value;
        }
    }
    return std::nullopt;
}

void SemanticAnalyser::analyse( const ast::Program ast, SymbolTable& table ) {
    global_table = &table;
    program( ast, table );
}

void SemanticAnalyser::program( const ast::Program ast, SymbolTable& table ) {
    for ( const auto& d : ast->declarations ) {
        // top level
        nested_function = false;

        std::visit(
            overloaded { [ this, &table ]( ast::VariableDef ast ) -> void { return file_variable_def( ast, table ); },
                         [ this, &table ]( ast::FunctionDef ast ) -> void { return function_def( ast, table ); } },
            d );
    }
}

void SemanticAnalyser::file_variable_def( ast::VariableDef ast, SymbolTable& table ) {
    spdlog::debug( "file VariableDef: {}", ast->name );
    // extern variables can't have initializers
    if ( ast->storage == StorageClass::Extern && ast->init ) {
        throw SemanticException( ast->location, "Extern variables can't have initializers" );
    }

    int value = 0;
    if ( ast->init ) {
        if ( auto const_value = get_constant( ast->init.value() ); const_value ) {
            value = *const_value;
        } else {
            throw SemanticException( ast->location, "Global variables must have constant initializers" );
        }
    }

    if ( auto old_decl = table.find( ast->name ); old_decl ) {
        if ( !is_integer( old_decl->type ) ) {
            throw SemanticException( ast->location, "Function redeclared as variable: {}", ast->name );
        }
        if ( ast->storage == StorageClass::Extern ) {
            // If extern then takes the previous defintion
            ast->storage = old_decl->storage;
            return;
        } else if ( old_decl->storage != StorageClass::None && ast->storage != StorageClass::None ) {
            if ( old_decl->storage != ast->storage ) {
                throw SemanticException( ast->location,
                                         "Can't declare the same declaration with and without linkage: {}", ast->name );
            }
            spdlog::debug( "Set tentative value {}", value );
            old_decl->number = value;
            old_decl->initaliser = Initialiser::Final;
            table.put( ast->name, *old_decl );
            return;
        } else if ( old_decl->storage == StorageClass::Static && ast->storage == StorageClass::None ) {
            throw SemanticException( ast->location, "Can't declare the same declaration with and without linkage: {}",
                                     ast->name );
        } else if ( old_decl->initaliser == Initialiser::Final && ast->init ) {
            throw SemanticException(
                ast->location, "Can't declare the same declaration with and without initialisation: {}", ast->name );
        }
        if ( ast->var_type != old_decl->type ) {
            throw SemanticException( ast->location, "Can't declare the same declaration {} with different type: {} ",
                                     ast->name, to_string( ast->var_type ) );
        }
        spdlog::debug( "What to do with variable: {}", ast->name );
    }
    spdlog::debug( "Declaring file variable: {}", ast->name );
    auto global = ast->storage != StorageClass::Static;
    table.put( ast->name, Symbol { .name = ast->name,
                                   .storage = ast->storage,
                                   .type = ast->var_type,
                                   .number = value,
                                   .current_scope = true,
                                   .initaliser = ast->init.has_value() ? Initialiser::Final : Initialiser::Tentative,
                                   .global = global } );
}

void SemanticAnalyser::function_def( ast::FunctionDef ast, SymbolTable& table ) {
    spdlog::debug( "Function: {}", ast->name );
    // Clear the labels for each function.
    labels.clear();

    // Check if the function is defined as a nested function.
    if ( ast->storage == StorageClass::Static && nested_function ) {
        throw SemanticException( ast->location, "Static variables can't be declared inside a function" );
    }

    bool global = ast->storage != StorageClass::Static && ast->block.has_value();

    // remove "void" parameters
    ast->params.erase( std::remove( ast->params.begin(), ast->params.end(), "void" ), ast->params.end() );
    ast->base_type = Type::FUNCTION;
    FunctionType function_type = { .return_type = ast->function_type.return_type,
                                   .parameter_types = ast->function_type.parameter_types };

    auto old_dec = table.find( ast->name );
    if ( old_dec ) {
        spdlog::debug( "Symbol {} already exists", ast->name );
        spdlog::debug( "symbol: {:s} current_scope: {}", to_string( *old_dec ), old_dec->current_scope );

        if ( old_dec->type != Type::FUNCTION && old_dec->global ) {
            // Another symbol is already defined with the same name, but it is not a function.
            throw SemanticException( ast->location, "Redeclaring {} as a function", ast->name );
        }

        spdlog::debug( "1" );
        if ( old_dec->storage == StorageClass::None && old_dec->current_scope && old_dec->global ) {
            throw SemanticException( ast->location, "Duplicate function declaration: {}", ast->name );
        }

        spdlog::debug( "2" );
        if ( old_dec->type != Type::FUNCTION ) {
            // Variable was declared in a another scope, discard symbol and exit this section
            old_dec = std::nullopt;
            goto out;
        }

        spdlog::debug( "3" );
        if ( old_dec->storage == StorageClass::Static && old_dec->current_scope ) {
            // If the function is internal, it should not be declared again.
            throw SemanticException( ast->location, "Duplicate function declaration: {}.", ast->name );
        }

        spdlog::debug( "4" );
        if ( ast->block && old_dec->initaliser == Initialiser::Final ) {
            // Function defined more than once
            throw SemanticException( ast->location, "Function {} already defined", ast->name );
        }

        spdlog::debug( "5" );
        if ( ast->storage == StorageClass::Static ) {
            throw SemanticException( ast->location, "Static function {} follows non-static", ast->name );
        }

        spdlog::debug( "6" );
        if ( old_dec->global && old_dec->current_scope && ast->block ) {
            throw SemanticException( ast->location, "Extern function {} follows non-extern", ast->name );
        }

        spdlog::debug( "7" );
        if ( ast->storage == StorageClass::Static ) {
            spdlog::debug( "Static function {} follows non-static", ast->name );
        }

        global = old_dec->global;

        if ( old_dec->number != ast->params.size() ) {
            throw SemanticException( ast->location, "Function {} expects {} arguments, but got {}.", ast->name,
                                     old_dec->number, ast->params.size() );
        }
        for ( auto [ i, param ] : enumerate( ast->params ) ) {
            if ( old_dec->function_type.parameter_types[ i ] != function_type.parameter_types[ i ] ) {
                throw SemanticException( ast->location, "Function {} parameter {} has type {}, but got {}.", ast->name,
                                         i, to_string( old_dec->function_type.parameter_types[ i ] ),
                                         to_string( function_type.parameter_types[ i ] ) );
            }
        }
    }

out:
    auto s = old_dec.value_or( Symbol { .name = ast->name, .type = Type::FUNCTION } );
    s.global = global;
    s.function_type = function_type;

    // Check if the function is defined as a nested function.
    if ( auto f = global_table->find( ast->name ) ) {
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
        s.storage = StorageClass::Static;
        s.current_scope = true;
        table.put( ast->name, s );

        // Create new scope
        auto new_table = new_scope( table );

        // Add parameters to the symbol table.
        for ( auto [ i, param ] : enumerate( ast->params ) ) {
            auto unique_name = table.temp_name( param );
            spdlog::debug( "Declaring param: {} as {}", param, unique_name );
            new_table.put( param, Symbol { .name = unique_name,
                                           .storage = StorageClass::Parameter,
                                           .type = ast->function_type.parameter_types[ i ],
                                           .current_scope = true } );
            param = unique_name;
        }

        nested_function = true;
        visit_Compound( ast->block.value(), new_table );
    } else {
        // If there is no block, it is a function declaration.
        spdlog::debug( "Declaring function: {} with {} parameters", ast->name, ast->params.size() );
        s.storage = ast->block ? StorageClass::Extern : StorageClass::None;
        s.current_scope = true;
        spdlog::debug( "put symbol: {:s} current_scope: {}", to_string( s ), s.current_scope );
        table.put( ast->name, s );
        global_table->put( ast->name, s );
    }

    // Check for labels that were used but not defined.
    for ( const auto& [ label, defined ] : labels ) {
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
                             []( ast::Null ) -> void { ; } },
                ast );
}

void SemanticAnalyser::block_variable_def( const ast::VariableDef ast, SymbolTable& table ) {
    spdlog::debug( "block variable def: {}", ast->name );

    if ( ast->storage == StorageClass::Extern ) {
        // extern variables can't have initializers
        if ( ast->init ) {
            throw SemanticException( ast->location, "Extern variables can't have initializers" );
        }

        if ( auto old_dec = table.find( ast->name ); old_dec ) {
            if ( !is_integer( old_dec->type ) ) {
                // Another symbol is already defined with the same name, but it is not a variable.
                throw SemanticException( ast->location, "Function {} redeclared as variable", ast->name );
            }
            // Previous declaration
            spdlog::debug( "1" );
            if ( old_dec->current_scope &&
                 ( old_dec->storage == StorageClass::None || old_dec->storage == StorageClass::Static ) ) {
                throw SemanticException( ast->location, "Conflicting local definitions: {}", ast->name );
            }

            spdlog::debug( "2" );
            // Conflicts with parameter
            if ( old_dec->current_scope && old_dec->storage == StorageClass::Parameter ) {
                throw SemanticException( ast->location, "Conflicting local parameter: {}", ast->name );
            }

            if ( old_dec->type != ast->var_type ) {
                throw SemanticException( ast->location, "Conflicting types for variable {}: {} and {}", ast->name,
                                         to_string( old_dec->type ), to_string( ast->var_type ) );
            }

            // Already declared, put in local table not global
            Symbol s {
                .name = ast->name, .storage = StorageClass::Extern, .type = ast->var_type, .current_scope = true };
            table.put( ast->name, s );
            return;
        }

        // Add the variable to the symbol table
        spdlog::debug( "Declaring extern variable: {}", ast->name );
        Symbol s { .name = ast->name, .storage = StorageClass::Extern, .type = ast->var_type, .current_scope = true };
        global_table->put( ast->name, s );
        table.put( ast->name, s );
        return;
    }

    int value = 0;
    if ( ast->storage == StorageClass::Static ) {
        if ( ast->init ) {
            if ( auto const_value = get_constant( ast->init.value() ); const_value ) {
                value = const_value.value();
            } else {
                throw SemanticException( ast->location, "static variables must have constant initializers" );
            }
        }
        if ( auto old_dec = table.find( ast->name ); old_dec ) {
            if ( !is_integer( old_dec->type ) ) {
                // Another symbol is already defined with the same name, but it is not a variable.
                throw SemanticException( ast->location, "Function {} redeclared as variable.", ast->name );
            }
            // Previous declaration
            if ( old_dec->current_scope && old_dec->storage != StorageClass::Static ) {
                throw SemanticException( ast->location, "Conflicting local definitions: {}", ast->name );
            }
        }

        auto unique_name = table.temp_name( ast->name );
        spdlog::debug( "Declaring static variable: {} as {}", ast->name, unique_name );
        auto s = Symbol { .name = unique_name,
                          .storage = StorageClass::Static,
                          .type = ast->var_type,
                          .number = value,
                          .current_scope = true };
        table.put( ast->name, s );
        s.current_scope = false;
        global_table->put( unique_name, s );
        ast->name = unique_name;
        if ( ast->init ) {
            expr( ast->init.value(), table );
        }
        return;
    }

    // No linkage
    if ( auto old_dec = table.find( ast->name ); old_dec ) {
        spdlog::debug( "Variable {} already defined - {}", ast->name, to_string( *old_dec ) );
        if ( !is_integer( old_dec->type ) && old_dec->current_scope ) {
            // Another symbol is already defined with the same name, but it is not a variable.
            throw SemanticException( ast->location, "Function {} redeclared as variable.", ast->name );
        }
        // Previous declaration
        if ( old_dec->current_scope && old_dec->storage == StorageClass::Extern ) {
            throw SemanticException( ast->location, "Conflicting local definitions: {}", ast->name );
        }
        if ( old_dec->current_scope &&
             ( old_dec->storage == StorageClass::Parameter || old_dec->storage == StorageClass::None ) ) {
            throw SemanticException( ast->location, "Conflicting local parameter: {}", ast->name );
        }
        if ( old_dec->type != ast->var_type ) {
            throw SemanticException( ast->location, "Conflicting types for variable {}: {} and {}", ast->name,
                                     to_string( old_dec->type ), to_string( ast->var_type ) );
        }
    }

    auto unique_name = table.temp_name( ast->name );
    spdlog::debug( "Declaring local variable: {} as {}", ast->name, unique_name );
    table.put(
        ast->name,
        Symbol { .name = unique_name, .storage = StorageClass::None, .type = ast->var_type, .current_scope = true } );
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
                             [ this, &table ]( ast::VariableDef d ) -> void {
                                 if ( d->storage != StorageClass::None ) {
                                     throw SemanticException( d->location,
                                                              "Invalid storage class for for loop initialisation." );
                                 }
                                 block_variable_def( d, table );
                             } },
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
    std::set<std::int64_t> current_case_set;
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
            auto v = get_constant( *const_value );
            if ( !v ) {
                throw SemanticException( ast->location, "Case value must be constant expression" );
            }
            if ( case_set.top().contains( *v ) ) {
                throw SemanticException( ast->location, "Duplicate case value " );
            }
            case_set.top().insert( *v );
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
        std::visit( overloaded { [ this, &table ]( ast::VariableDef d ) -> void { block_variable_def( d, table ); },
                                 []( ast::FunctionDef f ) -> void {
                                     throw SemanticException( f->location, "No functions in case blocks" );
                                 },
                                 [ this, &table ]( ast::Statement s ) -> void { visit_Statement( s, table ); } },
                    item );
    }
}

void SemanticAnalyser::visit_Compound( const ast::Compound ast, SymbolTable& table ) {
    spdlog::debug( "Compound" );

    for ( const auto& item : ast->block_items ) {
        std::visit( overloaded { [ this, &table ]( ast::VariableDef d ) -> void { block_variable_def( d, table ); },
                                 [ this, &table ]( ast::FunctionDef f ) -> void { function_def( f, table ); },
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
                    [ this, &table ]( ast::Cast c ) -> void { visit_Cast( c, table ); },
                    [ this, &table ]( ast::Var v ) -> void { visit_Var( v, table ); },
                    [ this ]( ast::Constant c ) -> void { visit_Constant( c ); },
                },
                ast );
}

Type SemanticAnalyser::expr_type( ast::Expr ast ) {
    return std::visit( overloaded { []( ast::UnaryOp u ) -> Type { return u->base_type; },
                                    []( ast::BinaryOp b ) -> Type { return b->base_type; },
                                    []( ast::PostOp b ) -> Type { return b->base_type; },
                                    []( ast::Conditional b ) -> Type { return b->base_type; },
                                    []( ast::Assign a ) -> Type { return a->base_type; },
                                    []( ast::Call c ) -> Type { return c->base_type; },
                                    []( ast::Cast c ) -> Type { return c->base_type; },
                                    []( ast::Var v ) -> Type { return v->base_type; },
                                    []( ast::Constant c ) -> Type {
                                        return std::holds_alternative<ast::ConstantInt>( c ) ? Type::INT : Type::LONG;
                                    } },

                       ast );
};

void SemanticAnalyser::visit_UnaryOp( const ast::UnaryOp ast, SymbolTable& table ) {
    expr( ast->operand, table );
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        // operand must be a variable
        if ( !std::holds_alternative<ast::Var>( ast->operand ) ) {
            throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
        }
        auto var = std::get<ast::Var>( ast->operand );
        ast->base_type = var->base_type;
    } else if ( ast->op == TokenType::EXCLAMATION ) {
        ast->base_type = Type::INT;
    } else {
        ast->base_type = expr_type( ast->operand );
    }

    // Constant Analysis
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        // Postfix increment/decrement is not constant
        is_constant = false;
    } else {
        // Unary operators are constant
        is_constant = true;
    }
}

constexpr Type get_common_type( const Type a, const Type b ) {
    if ( a == b ) {
        return a;
    }
    return Type::LONG;
}

void SemanticAnalyser::visit_BinaryOp( const ast::BinaryOp ast, SymbolTable& table ) {
    expr( ast->left, table );
    bool left_constant = is_constant;
    auto type_left = expr_type( ast->left );
    expr( ast->right, table );
    bool right_constant = is_constant;
    auto type_right = expr_type( ast->right );

    if ( ast->op == TokenType::COMPARISON_EQUALS || ast->op == TokenType::COMPARISON_NOT ||
         ast->op == TokenType::LESS || ast->op == TokenType::LESS_EQUALS || ast->op == TokenType::GREATER ||
         ast->op == TokenType::GREATER_EQUALS ) {
        ast->base_type = Type::INT;
    } else {
        ast->base_type = get_common_type( type_left, type_right );
    }

    // Constant Analysis
    is_constant = left_constant && right_constant;
}

void SemanticAnalyser::visit_PostOp( const ast::PostOp ast, SymbolTable& table ) {
    // Check left side for postfix increment/decrement
    expr( ast->operand, table );
    if ( !std::holds_alternative<ast::Var>( ast->operand ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {} ", ast->op );
    }
    ast->base_type = expr_type( ast->operand );

    // Constant Analysis - postfix operators are not constant
    is_constant = false;
}

void SemanticAnalyser::visit_Conditional( const ast::Conditional ast, SymbolTable& table ) {
    expr( ast->condition, table );
    expr( ast->then_expr, table );
    bool left_constant = is_constant;
    auto type_left = expr_type( ast->then_expr );
    expr( ast->else_expr, table );
    bool right_constant = is_constant;
    auto type_right = expr_type( ast->else_expr );
    ast->base_type = get_common_type( type_left, type_right );
    // Constant Analysis
    is_constant = left_constant && right_constant;
}

void SemanticAnalyser::visit_Assign( const ast::Assign ast, SymbolTable& table ) {
    if ( !std::holds_alternative<ast::Var>( ast->left ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for {}", ast->op );
    }
    if ( std::holds_alternative<ast::Cast>( ast->right ) ) {
        throw SemanticException( ast->location, "Invalid lvalue: for right hand side of {}", ast->op );
    }
    expr( ast->left, table );
    expr( ast->right, table );
    ast->base_type = expr_type( ast->right );
    // Constant Analysis
    is_constant = false; // Assignment is never constant
}

void SemanticAnalyser::visit_Call( const ast::Call ast, SymbolTable& table ) {
    spdlog::debug( "Call: {}", ast->function_name );
    // Check if the function is declared
    auto symbol = table.find( ast->function_name );
    if ( !symbol || is_integer( symbol->type ) ) {
        throw SemanticException( ast->location, "Function {} not declared", ast->function_name );
    }

    if ( symbol->number != ast->arguments.size() ) {
        throw SemanticException( ast->location, "Function {} expects {} arguments, but got {}", ast->function_name,
                                 symbol->number, ast->arguments.size() );
    }
    ast->base_type = symbol->type;

    for ( const auto& arg : ast->arguments ) {
        expr( arg, table );
    }
    // Constant Analysis
    is_constant = false; // Function calls are not constant
}

void SemanticAnalyser::visit_Cast( ast::Cast ast, SymbolTable& table ) {
    ast->base_type = ast->type;
};

void SemanticAnalyser::visit_Var( const ast::Var ast, const SymbolTable& table ) {
    if ( auto name = table.find( ast->name ) ) {

        if ( name->type == Type::FUNCTION ) {
            throw SemanticException( ast->location, "Variable {} cannot be of type function", ast->name );
        }

        spdlog::debug( "Found var: {} for {}", name->name, ast->name );
        ast->name = name->name; // Change the name to the temporary.
        ast->base_type = name->type;

        // Constant Analysis
        is_constant = false;

        return;
    }
    throw SemanticException( ast->location, "variable: {} not declared", ast->name );
}

void SemanticAnalyser::visit_Constant( ast::Constant ast ) {
    // Constant Analysis
    is_constant = true;
    std::visit( overloaded { []( ast::ConstantInt i ) { i->base_type = Type::INT; },
                             []( ast::ConstantLong l ) { l->base_type = Type::LONG; } },
                ast );
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
