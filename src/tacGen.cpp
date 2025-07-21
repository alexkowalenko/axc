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
#include "spdlog/fmt/bundled/chrono.h"
#include "tac/includes.h"

namespace {

std::int32_t temp_counter = 0;
std::string  temp_name() {
    return std::format( "temp.{}", temp_counter++ );
}

} // namespace

tac::Program TacGen::generate( ast::Program ast ) {
    auto program = mk_node<tac::Program_>( ast );
    program->function = functionDef( ast->function );
    return program;
}

tac::FunctionDef TacGen::functionDef( ast::FunctionDef ast ) {
    spdlog::debug( "tac::functionDef: {}", ast->name );
    auto function = mk_node<tac::FunctionDef_>( ast );
    function->name = ast->name;
    for ( auto b : ast->block_items ) {
        std::visit(
            overloaded { [ this ]( ast::Declaration ast ) -> void {}, [ this ]( ast::Statement ast ) -> void {} }, b );
    }
    return function;
}

void TacGen::ret( ast::Return ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::ret: {}" );

    // Do expression
    auto value = expr( ast->expr, instructions );

    // Do Return
    auto ret = mk_node<tac::Return_>( ast, value );
    instructions.push_back( ret );
}

tac::Value TacGen::expr( ast::Expr ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::expr" );
    return std::visit(
        overloaded { [ &instructions, this ]( ast::UnaryOp u ) -> tac::Value { return unary( u, instructions ); },
                     [ &instructions, this ]( ast::BinaryOp b ) -> tac::Value { return binary( b, instructions ); },
                     [ this ]( ast::Constant c ) -> tac::Value { return constant( c ); } },
        ast );
}

tac::Value TacGen::unary( ast::UnaryOp ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::unary: {}", to_string( ast->op ) );
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
    auto dst = mk_node<tac::Variable_>( ast, temp_name() );
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
    auto dst = std::make_shared<tac::Variable_>( ast->location );
    dst->name = temp_name();
    b->dst = dst;
    instructions.push_back( b );
    return b->dst;
}

tac::Value TacGen::logical( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::logical: {}", to_string( ast->op ) );
    auto false_label = generate_label( ast, "logicalfalse" ); // For AND
    auto true_label = generate_label( ast, "logicaltrue" );   // For OR
    auto end_label = generate_label( ast, "logicalend" );
    auto result = mk_node<tac::Variable_>( ast, temp_name() );
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

tac::Label TacGen::generate_label( const std::shared_ptr<ast::Base> b, std::string_view name ) {
    return mk_node<tac::Label_>( b, std::format( "{:s}.{:d}", name, label_count++ ) );
}

tac::Constant TacGen::constant( ast::Constant ast ) {
    spdlog::debug( "tac::constant: {}", ast->value );
    return mk_node<tac::Constant_>( ast, ast->value );
}
