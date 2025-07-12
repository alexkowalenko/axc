//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "assemblyGen.h"
#include "common.h"

namespace {

template <typename T> constexpr std::shared_ptr<T> make_AT( const std::shared_ptr<tac::Base> b ) {
    return std::make_shared<T>( b->location );
}

} // namespace

at::Program AssemblyGen::generate( const tac::Program& atac ) {
    auto program = make_AT<at::Program_>( atac );
    program->function = functionDef( atac->function );
    return program;
}

at::FunctionDef AssemblyGen::functionDef( const tac::FunctionDef& atac ) {
    auto function = make_AT<at::FunctionDef_>( atac );
    function->name = atac->name;
    function->instructions = {};
    for ( auto instr : atac->instructions ) {
        std::visit( overloaded { [ &function, this ]( tac::Return r ) -> void { ret( r, function->instructions ); },
                                 [ &function, this ]( tac::Unary r ) -> void { unary( r, function->instructions ); } },
                    instr );
    }
    return function;
};

void AssemblyGen::ret( const tac::Return& atac, std::vector<at::Instruction>& instructions ) {
    auto mov = make_AT<at::Mov_>( atac );
    mov->src = value( atac->value );
    auto reg = make_AT<at::Register_>( atac );
    reg->reg = "eax";
    mov->dst = reg;
    instructions.push_back( mov );
    auto ret = make_AT<at::Ret_>( atac );
    instructions.push_back( ret );
};

void AssemblyGen::unary( const tac::Unary& atac, std::vector<at::Instruction>& instructions ) {
    auto mov = make_AT<at::Mov_>( atac );
    mov->src = value( atac->src );
    mov->dst = value( atac->dst );
    instructions.push_back( mov );
    auto unary = make_AT<at::Unary_>( atac );
    switch ( atac->op ) {
    case TokenType::TILDE :
        unary->op = at::UnaryOpType::NOT;
        break;
    case TokenType::DASH :
        unary->op = at::UnaryOpType::NEG;
        break;
    default :
        break;
    }
    unary->operand = value( atac->dst );
    instructions.push_back( unary );
};

at::Operand AssemblyGen::value( const tac::Value& atac ) {
    return std::visit( overloaded { [ this ]( tac::Constant c ) -> at::Operand { return constant( c ); },
                                    [ this ]( tac::Variable v ) -> at::Operand { return pseudo( v ); } },
                       atac );
}

at::Operand AssemblyGen::constant( const tac::Constant& atac ) {
    auto imm = make_AT<at::Imm_>( atac );
    imm->value = atac->value;
    return imm;
};

at::Operand AssemblyGen::pseudo( tac::Variable atac ) {
    auto p = make_AT<at::Pseudo_>( atac );
    p->name = atac->name;
    return p;
}