//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#include "tacGen.h"

#include <spdlog/spdlog.h>

#include "ast/includes.h"
#include "common.h"
#include "exception.h"
#include "spdlog/fmt/bundled/chrono.h"
#include "tac/includes.h"

tac::Program TacGen::generate( ast::Program ast ) {
    auto program = mk_node<tac::Program_>( ast );
    program->function = functionDef( ast->function );
    return program;
}

tac::FunctionDef TacGen::functionDef( ast::FunctionDef ast ) {
    spdlog::debug( "tac::functionDef: {}", ast->name );
    auto function = mk_node<tac::FunctionDef_>( ast );
    function->name = ast->name;

    std::vector<tac::Instruction> instructions;
    compound( ast->block, instructions );

    // Add a return at the end of the function
    instructions.push_back( mk_node<tac::Return_>( ast, mk_node<tac::Constant_>( ast, 0 ) ) ); // Return 0
    function->instructions = instructions;
    return function;
}

void TacGen::declaration( ast::Declaration ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::declaration: {} {}", ast->name, ast->init ? "init" : "" );
    if ( ast->init ) {
        auto result = expr( *ast->init, instructions );
        auto copy = mk_node<tac::Copy_>( ast, result, mk_node<tac::Variable_>( ast, ast->name ) );
        instructions.push_back( copy );
    }
}

void TacGen::statement( const ast::Statement ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::statement" );
    return std::visit(
        overloaded { [ this, &instructions ]( ast::Return ast ) -> void { ret( ast, instructions ); },
                     [ this, &instructions ]( ast::If ast ) -> void { if_stat( ast, instructions ); },
                     [ this, &instructions ]( ast::Goto g ) -> void { goto_stat( g, instructions ); },
                     [ this, &instructions ]( ast::Label l ) -> void { label( l, instructions ); },
                     [ this, &instructions ]( ast::Break c ) -> void { break_stat( c, instructions ); },
                     [ this, &instructions ]( ast::Continue c ) -> void { continue_stat( c, instructions ); },
                     [ this, &instructions ]( ast::While c ) -> void { while_stat( c, instructions ); },
                     [ this, &instructions ]( ast::DoWhile c ) -> void { do_while_stat( c, instructions ); },
                     [ this, &instructions ]( ast::For c ) -> void { for_stat( c, instructions ); },
                     [ this, &instructions ]( ast::Compound c ) -> void { compound( c, instructions ); },
                     [ this, &instructions ]( ast::Expr e ) -> void { expr( e, instructions ); },
                     [ this ]( ast::Null ) -> void { ; } },
        ast );
}

void TacGen::ret( ast::Return ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::ret: {}" );

    // Do expression
    auto value = expr( ast->expr, instructions );

    // Do Return
    auto ret = mk_node<tac::Return_>( ast, value );
    instructions.push_back( ret );
}

void TacGen::if_stat( ast::If ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::if_stat" );
    auto end_label = generate_label( ast, "ifend" );
    auto else_label = generate_label( ast, "else" );

    // Instructs for condition
    auto c = expr( ast->condition, instructions );

    tac::Label jump_label = ast->else_stat ? else_label : end_label;

    // JumpIfZero(c, jump_label)
    auto jump = mk_node<tac::JumpIfZero_>( ast, c, jump_label->name );
    instructions.push_back( jump );

    // Instructs for then
    statement( ast->then, instructions );

    if ( ast->else_stat ) {
        // Jump(end)
        auto jump = mk_node<tac::Jump_>( ast, end_label->name );
        instructions.push_back( jump );
        // Label(else_label)
        instructions.push_back( else_label );
        // Instructs for else
        statement( ast->else_stat.value(), instructions );
    }

    // Label(end)
    instructions.push_back( end_label );
}
void TacGen::goto_stat( ast::Goto ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::goto_stat: {}", ast->label );
    auto jump = mk_node<tac::Jump_>( ast, ast->label );
    instructions.push_back( jump );
}

void TacGen::label( ast::Label ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::label: {}", ast->label );
    auto label = mk_node<tac::Label_>( ast, ast->label );
    instructions.push_back( label );
}

void TacGen::break_stat( const ast::Break ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::break_stat: {}", ast->ast_label );
    auto jump = mk_node<tac::Jump_>( ast, "break_" + ast->ast_label );
    instructions.push_back( jump );
}

void TacGen::continue_stat( const ast::Continue ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::continue_stat: {}", ast->ast_label );
    auto jump = mk_node<tac::Jump_>( ast, "continue_" + ast->ast_label );
    instructions.push_back( jump );
}

void TacGen::while_stat( const ast::While ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::while_stat: {}", ast->ast_label );

    // Label(continue_label)
    auto continue_label = generate_loop_continue( ast );
    instructions.push_back( continue_label );

    auto break_label = generate_loop_break( ast );

    // Instructs for condition
    auto c = expr( ast->condition, instructions );

    // JumpIfZero(c, jump_label)
    auto jump = mk_node<tac::JumpIfZero_>( ast, c, break_label->name );
    instructions.push_back( jump );

    // Instructs for body
    statement( ast->body, instructions );
    // Jump(continue_label)
    instructions.push_back( mk_node<tac::Jump_>( ast, continue_label->name ) );
    // Label(break_label)
    instructions.push_back( generate_loop_break( ast ) );
}

void TacGen::do_while_stat( const ast::DoWhile ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::do_stat: {}", ast->ast_label );
    // Label(start)
    auto start = generate_label( ast, "do_while_start" );
    instructions.push_back( start );

    // Instructs for body
    statement( ast->body, instructions );

    instructions.push_back( generate_loop_continue( ast ) );
    // Instructs for condition
    auto c = expr( ast->condition, instructions );

    // JumpIfNotZero(c, jump_label)
    auto jump = mk_node<tac::JumpIfNotZero_>( ast, c, start->name );
    instructions.push_back( jump );
    instructions.push_back( generate_loop_break( ast ) );
}

void TacGen::for_stat( const ast::For ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::for_stat: {}", ast->ast_label );
    auto break_label = generate_loop_break( ast );

    // Instructions for init
    if ( ast->init ) {
        std::visit(
            overloaded { [ this, &instructions ]( ast::Expr e ) -> void { expr( e, instructions ); },
                         [ this, &instructions ]( ast::Declaration d ) -> void { declaration( d, instructions ); } },
            ast->init.value() );
    }

    // Label(start)
    auto start = generate_label( ast, "while_start" );
    instructions.push_back( start );

    if ( ast->condition ) {
        // Instructs for condition
        auto c = expr( ast->condition.value(), instructions );

        // JumpIfZero(c, break_label)
        auto jump = mk_node<tac::JumpIfZero_>( ast, c, break_label->name );
        instructions.push_back( jump );
    }

    // Instructs for body
    statement( ast->body, instructions );

    // Label(continue_label)
    auto continue_label = generate_loop_continue( ast );
    instructions.push_back( continue_label );
    if ( ast->increment ) {
        // Instructs for increment
        expr( ast->increment.value(), instructions );
    }

    // Jump(start)
    auto jump = mk_node<tac::Jump_>( ast, start->name );
    instructions.push_back( jump );
    // Label(break_label)
    instructions.push_back( break_label );
}

void TacGen::compound( ast::Compound ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::compound:" );
    for ( auto b : ast->block_items ) {
        spdlog::debug( "tac::functionDef: block" );
        std::visit(
            overloaded { [ this, &instructions ]( ast::Declaration ast ) -> void { declaration( ast, instructions ); },
                         [ this, &instructions ]( ast::Statement ast ) -> void { statement( ast, instructions ); } },
            b );
    }
}

tac::Value TacGen::expr( ast::Expr ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::expr" );
    return std::visit(
        overloaded {
            [ &instructions, this ]( ast::UnaryOp u ) -> tac::Value { return unary( u, instructions ); },
            [ &instructions, this ]( ast::BinaryOp b ) -> tac::Value { return binary( b, instructions ); },
            [ &instructions, this ]( ast::PostOp b ) -> tac::Value { return post( b, instructions ); },
            [ &instructions, this ]( ast::Conditional b ) -> tac::Value { return conditional( b, instructions ); },
            [ &instructions, this ]( ast::Assign a ) -> tac::Value { return assign( a, instructions ); },
            [ this ]( ast::Var v ) -> tac::Value { return mk_node<tac::Variable_>( v, v->name ); },
            [ this ]( ast::Constant c ) -> tac::Value { return constant( c ); } },
        ast );
}

tac::Value TacGen::unary( ast::UnaryOp ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::unary: {}", to_string( ast->op ) );

    // Handle increment and decrement
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        auto b = mk_node<tac::Binary_>( ast );
        switch ( ast->op ) {
        case TokenType::INCREMENT :
            b->op = tac::BinaryOpType::Add;
            break;
        case TokenType::DECREMENT :
            b->op = tac::BinaryOpType::Subtract;
            break;
        default :
            throw SemanticException( ast->location, "Internal: increment operator invalid: {}", to_string( ast->op ) );
        }
        b->src1 = expr( ast->operand, instructions );
        b->src2 = mk_node<tac::Constant_>( ast, 1 );
        auto temp = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );
        b->dst = temp;
        instructions.push_back( b );

        auto copy = mk_node<tac::Copy_>( ast, temp, expr( ast->operand, instructions ) );
        instructions.push_back( copy );
        return temp;
    }

    // Handle other unary operators
    auto u = mk_node<tac::Unary_>( ast );
    switch ( ast->op ) {
    case TokenType::DASH :
        u->op = tac::UnaryOpType::Negate;
        break;
    case TokenType::TILDE :
        u->op = tac::UnaryOpType::Complement;
        break;
    case TokenType::EXCLAMATION :
        u->op = tac::UnaryOpType::Not;
        break;
    default :
        break;
    }
    u->src = expr( ast->operand, instructions );
    auto dst = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );
    u->dst = dst;
    instructions.push_back( u );
    return u->dst;
}

tac::Value TacGen::binary( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::binary: {}", to_string( ast->op ) );
    auto b = mk_node<tac::Binary_>( ast );
    switch ( ast->op ) {
    case TokenType::PLUS :
        b->op = tac::BinaryOpType::Add;
        break;
    case TokenType::DASH :
        b->op = tac::BinaryOpType::Subtract;
        break;
    case TokenType::ASTÉRIX :
        b->op = tac::BinaryOpType::Multiply;
        break;
    case TokenType::SLASH :
        b->op = tac::BinaryOpType::Divide;
        break;
    case TokenType::PERCENT :
        b->op = tac::BinaryOpType::Modulo;
        break;
    case TokenType::AMPERSAND :
        b->op = tac::BinaryOpType::BitwiseAnd;
        break;
    case TokenType::CARET :
        b->op = tac::BinaryOpType::BitwiseXor;
        break;
    case TokenType::PIPE :
        b->op = tac::BinaryOpType::BitwiseOr;
        break;
    case TokenType::LEFT_SHIFT :
        b->op = tac::BinaryOpType::ShiftLeft;
        break;
    case TokenType::RIGHT_SHIFT :
        b->op = tac::BinaryOpType::ShiftRight;
        break;
    case TokenType::LOGICAL_AND :
    case TokenType::LOGICAL_OR :
        return logical( ast, instructions );
    case TokenType::COMPARISON_EQUALS :
        b->op = tac::BinaryOpType::Equal;
        break;
    case TokenType::COMPARISON_NOT :
        b->op = tac::BinaryOpType::NotEqual;
        break;
    case TokenType::LESS :
        b->op = tac::BinaryOpType::Less;
        break;
    case TokenType::LESS_EQUALS :
        b->op = tac::BinaryOpType::LessEqual;
        break;
    case TokenType::GREATER :
        b->op = tac::BinaryOpType::Greater;
        break;
    case TokenType::GREATER_EQUALS :
        b->op = tac::BinaryOpType::GreaterEqual;
        break;
    default :
        break;
    }
    b->src1 = expr( ast->left, instructions );
    b->src2 = expr( ast->right, instructions );
    auto dst = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );
    b->dst = dst;
    instructions.push_back( b );
    return b->dst;
}

tac::Value TacGen::post( ast::PostOp ast, std::vector<tac::Instruction>& instructions ) {
    // Copy(left, orig)
    auto left = expr( ast->operand, instructions );
    auto orig = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );
    auto copy = mk_node<tac::Copy_>( ast, left, orig );
    instructions.push_back( copy );

    // Binop(Add/Sub, left, 1, dst)
    auto b = mk_node<tac::Binary_>( ast );
    switch ( ast->op ) {
    case TokenType::INCREMENT :
        b->op = tac::BinaryOpType::Add;
        break;
    case TokenType::DECREMENT :
        b->op = tac::BinaryOpType::Subtract;
        break;
    default :
        throw SemanticException( ast->location, "Internal: increment operator invalid: {}", to_string( ast->op ) );
    }
    b->src1 = left;
    b->src2 = mk_node<tac::Constant_>( ast, 1 );
    auto dst = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );
    b->dst = dst;
    instructions.push_back( b );

    // Copy(dst, left)
    auto copy2 = mk_node<tac::Copy_>( ast, dst, left );
    instructions.push_back( copy2 );

    // Return orig
    return orig;
}

tac::Value TacGen::logical( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::logical: {}", to_string( ast->op ) );
    auto false_label = generate_label( ast, "logicalfalse" ); // For AND
    auto true_label = generate_label( ast, "logicaltrue" );   // For OR
    auto end_label = generate_label( ast, "logicalend" );
    auto result = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );
    auto one = mk_node<tac::Constant_>( ast, 1 );
    auto zero = mk_node<tac::Constant_>( ast, 0 );

    // v1
    auto v = expr( ast->left, instructions );
    if ( ast->op == TokenType::LOGICAL_AND ) {
        auto jump = mk_node<tac::JumpIfZero_>( ast, v, false_label->name );
        instructions.emplace_back( jump );
    } else {
        // LOGICAL OR
        auto jump = mk_node<tac::JumpIfNotZero_>( ast, v, true_label->name );
        instructions.emplace_back( jump );
    }

    // v2
    v = expr( ast->right, instructions );
    if ( ast->op == TokenType::LOGICAL_AND ) {
        auto jump = mk_node<tac::JumpIfZero_>( ast, v, false_label->name );
        instructions.emplace_back( jump );
        // result = 1
        auto copy = mk_node<tac::Copy_>( ast, one, result );
        instructions.emplace_back( copy );
    } else {
        // LOGICAL OR
        auto jump = mk_node<tac::JumpIfNotZero_>( ast, v, true_label->name );
        instructions.emplace_back( jump );
        // result = 0
        auto copy = mk_node<tac::Copy_>( ast, zero, result );
        instructions.emplace_back( copy );
    }

    // jump end
    auto jump = mk_node<tac::Jump_>( ast, end_label->name );
    instructions.emplace_back( jump );

    // label false:
    if ( ast->op == TokenType::LOGICAL_AND ) {
        instructions.emplace_back( false_label );
    } else {
        instructions.emplace_back( true_label );
    }

    if ( ast->op == TokenType::LOGICAL_AND ) {
        // result = 0
        auto copy = mk_node<tac::Copy_>( ast, zero, result );
        instructions.emplace_back( copy );
    } else {
        // result 1
        auto copy = mk_node<tac::Copy_>( ast, one, result );
        instructions.emplace_back( copy );
    }
    // label end:
    instructions.emplace_back( end_label );
    return result;
}

tac::Value TacGen::conditional( ast::Conditional ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::conditional" );
    auto end_label = generate_label( ast, "ternend" );
    auto e2_label = generate_label( ast, "terne2" );
    auto result = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );

    // Instructs for condition
    auto c = expr( ast->condition, instructions );
    // JumpIfZero(c, end)
    auto jump = mk_node<tac::JumpIfZero_>( ast, c, e2_label->name );
    instructions.push_back( jump );
    // Instructs for e1
    auto v1 = expr( ast->then_expr, instructions );
    // result = v1
    auto copy = mk_node<tac::Copy_>( ast, v1, result );
    instructions.push_back( copy );
    // Jump(end)
    auto jump2 = mk_node<tac::Jump_>( ast, end_label->name );
    instructions.push_back( jump2 );

    // Label(e2_label)
    instructions.push_back( e2_label );
    // Instructs for e2
    auto v2 = expr( ast->else_expr, instructions );
    // result = v2
    copy = mk_node<tac::Copy_>( ast, v2, result );
    instructions.push_back( copy );

    // Label(end)
    instructions.push_back( end_label );
    return result;
}

tac::Value TacGen::assign( ast::Assign ast, std::vector<tac::Instruction>& instructions ) {

    if ( ast->op == TokenType::EQUALS ) {
        // Handle normal assignment
        auto result = expr( ast->right, instructions );
        auto copy = mk_node<tac::Copy_>( ast, result, expr( ast->left, instructions ) );
        instructions.push_back( copy );
        return result;
    }

    auto b = mk_node<tac::Binary_>( ast );
    switch ( ast->op ) {
    case TokenType::COMPOUND_PLUS :
        b->op = tac::BinaryOpType::Add;
        break;
    case TokenType::COMPOUND_MINUS :
        b->op = tac::BinaryOpType::Subtract;
        break;
    case TokenType::COMPOUND_ASTERIX :
        b->op = tac::BinaryOpType::Multiply;
        break;
    case TokenType::COMPOUND_SLASH :
        b->op = tac::BinaryOpType::Divide;
        break;
    case TokenType::COMPOUND_PERCENT :
        b->op = tac::BinaryOpType::Modulo;
        break;
    case TokenType::COMPOUND_AND :
        b->op = tac::BinaryOpType::BitwiseAnd;
        break;
    case TokenType::COMPOUND_XOR :
        b->op = tac::BinaryOpType::BitwiseXor;
        break;
    case TokenType::COMPOUND_OR :
        b->op = tac::BinaryOpType::BitwiseOr;
        break;
    case TokenType::COMPOUND_LEFT_SHIFT :
        b->op = tac::BinaryOpType::ShiftLeft;
        break;
    case TokenType::COMPOUND_RIGHT_SHIFT :
        b->op = tac::BinaryOpType::ShiftRight;
        break;
    default :
        throw SemanticException( ast->location, "Internal: assignment operator invalid: {}", to_string( ast->op ) );
    }
    b->src1 = expr( ast->left, instructions );
    b->src2 = expr( ast->right, instructions );
    auto temp = mk_node<tac::Variable_>( ast, symbol_table.temp_name() );
    b->dst = temp;
    instructions.push_back( b );

    auto copy = mk_node<tac::Copy_>( ast, temp, expr( ast->left, instructions ) );
    instructions.push_back( copy );
    return temp;
}

tac::Label TacGen::generate_label( const std::shared_ptr<ast::Base> b, std::string_view name ) {
    return mk_node<tac::Label_>( b, std::format( "{:s}.{:d}", name, label_count++ ) );
}

tac::Constant TacGen::constant( ast::Constant ast ) {
    spdlog::debug( "tac::constant: {}", ast->value );
    return mk_node<tac::Constant_>( ast, ast->value );
}

tac::Label TacGen::generate_loop_break( std::shared_ptr<ast::Base> b ) {
    return mk_node<tac::Label_>( b, std::format( "break_{:s}", b->ast_label ) );
};

tac::Label TacGen::generate_loop_continue( std::shared_ptr<ast::Base> b ) {
    return mk_node<tac::Label_>( b, std::format( "continue_{:s}", b->ast_label ) );
}
