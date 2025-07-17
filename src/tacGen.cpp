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
#include "tac/includes.h"

namespace {

std::int32_t temp_counter = 0;
std::string  temp_name() {
    return std::format( "temp.{}", temp_counter++ );
}

template <typename T, typename... Args>
constexpr std::shared_ptr<T> mk_TAC( const std::shared_ptr<ast::Base> b, Args... args ) {
    return std::make_shared<T>( b->location, args... );
}

} // namespace

tac::Program TacGen::generate( ast::Program ast ) {
    auto program = mk_TAC<tac::Program_>( ast );
    program->function = functionDef( ast->function );
    return program;
}

tac::FunctionDef TacGen::functionDef( ast::FunctionDef ast ) {
    spdlog::debug( "tac::functionDef: {}", ast->name );
    auto function = mk_TAC<tac::FunctionDef_>( ast );
    function->name = ast->name;
    function->instructions = ret( ast->statement->ret );
    return function;
}

std::vector<tac::Instruction> TacGen::ret( ast::Return ast ) {
    spdlog::debug( "tac::ret: {}" );
    std::vector<tac::Instruction> instructions;

    // Do expression
    auto value = expr( ast->expr, instructions );

    // Do Return
    auto ret = mk_TAC<tac::Return_>( ast );
    ret->value = value;
    instructions.push_back( ret );
    return instructions;
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
    auto u = mk_TAC<tac::Unary_>( ast );
    switch ( ast->op ) {
    case TokenType::DASH :
        u->op = tac::UnaryOpType::Negate;
        break;
    case TokenType::TILDE :
        u->op = tac::UnaryOpType::Complement;
        break;
    default :
        break;
    }
    u->src = expr( ast->operand, instructions );
    auto dst = mk_TAC<tac::Variable_>( ast, temp_name() );
    u->dst = dst;
    instructions.push_back( u );
    return u->dst;
}

tac::Value TacGen::binary( ast::BinaryOp ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::binary: {}", to_string( ast->op ) );
    auto b = mk_TAC<tac::Binary_>( ast );
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
    auto false_label = generate_label( ast, "logicalfalse" );
    auto end_label = generate_label( ast, "logicalend" );

    // v1
    auto v = expr( ast->left, instructions );
    auto jump = mk_TAC<tac::JumpIfZero_>( ast, v, false_label->name );
    instructions.emplace_back( jump );

    // v2
    v = expr( ast->right, instructions );
    jump = mk_TAC<tac::JumpIfZero_>( ast, v, false_label->name );
    instructions.emplace_back( jump );

    // result = 1
    auto one = mk_TAC<tac::Constant_>( ast, 1 );
    auto result = mk_TAC<tac::Variable_>( ast, temp_name() );
    auto copy = mk_TAC<tac::Copy_>( ast, one, result );
    instructions.emplace_back( copy );
    // jump end
    auto jump2 = mk_TAC<tac::Jump_>( ast, end_label->name );
    instructions.emplace_back( jump2 );

    // label false:
    instructions.emplace_back( false_label );

    // result = 0
    auto zero = mk_TAC<tac::Constant_>( ast, 0 );
    copy = mk_TAC<tac::Copy_>( ast, zero, result );
    instructions.emplace_back( copy );
    // label end:
    instructions.emplace_back( end_label );
    return result;
}

tac::Label TacGen::generate_label( const std::shared_ptr<ast::Base> b, std::string_view name ) {
    return mk_TAC<tac::Label_>( b, std::format( "{:s}.{:d}", name, label_count++ ) );
}

tac::Constant TacGen::constant( ast::Constant ast ) {
    spdlog::debug( "tac::constant: {}", ast->value );
    auto c = mk_TAC<tac::Constant_>( ast );
    c->value = ast->value;
    return c;
}
