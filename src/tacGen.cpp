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

template <typename T> constexpr std::shared_ptr<T> make_tac( const std::shared_ptr<ast::Base> b ) {
    return std::make_shared<T>( b->location );
}

} // namespace

tac::Program TacGen::generate( ast::Program ast ) {
    auto program = make_tac<tac::Program_>( ast );
    program->function = functionDef( ast->function );
    return program;
}

tac::FunctionDef TacGen::functionDef( ast::FunctionDef ast ) {
    spdlog::debug( "tac::functionDef: {}", ast->name );
    auto function = make_tac<tac::FunctionDef_>( ast );
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
    auto ret = make_tac<tac::Return_>( ast );
    ret->value = value;
    instructions.push_back( ret );
    return instructions;
}

tac::Value TacGen::expr( ast::Expr ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::expr" );
    tac::Value value;
    std::visit( overloaded { [ &instructions, &value, this ]( ast::UnaryOp u ) -> void {
                                auto unaryValue = unary( u, instructions );
                                instructions.push_back( unaryValue );
                                value = unaryValue->dst;
                            },
                             []( ast::BinaryOp ) -> void {
                             },
                             [ &value, this ]( ast::Constant c ) -> void { value = constant( c ); } },
                ast );
    return value;
}

tac::Unary TacGen::unary( ast::UnaryOp ast, std::vector<tac::Instruction>& instructions ) {
    spdlog::debug( "tac::unary: {}", to_string( ast->op ) );
    auto u = make_tac<tac::Unary_>( ast );
    u->op = ast->op;
    u->src = expr( ast->operand, instructions );
    auto dst = std::make_shared<tac::Variable_>( ast->location );
    dst->name = temp_name();
    u->dst = dst;
    return u;
}

tac::Constant TacGen::constant( ast::Constant ast ) {
    spdlog::debug( "tac::constant: {}", ast->value );
    auto c = make_tac<tac::Constant_>( ast );
    c->value = ast->value;
    return c;
}
