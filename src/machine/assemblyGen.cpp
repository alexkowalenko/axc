//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "assemblyGen.h"
#include "../common.h"
#include "x86_common.h"

namespace {

template <typename T, typename... Args>
constexpr std::shared_ptr<T> mk_AT_TAC( const std::shared_ptr<tac::Base> b, const Args... args ) {
    return std::make_shared<T>( b->location, args... );
}

constexpr at::Register mk_reg( const std::shared_ptr<tac::Base> b, std::string const & name ) {
    return mk_node<at::Register_>( b, name );
}

} // namespace

at::Program AssemblyGen::generate( const tac::Program atac ) {
    auto program = mk_AT_TAC<at::Program_>( atac );
    program->function = functionDef( atac->function );
    return program;
}

at::FunctionDef AssemblyGen::functionDef( const tac::FunctionDef atac ) {
    auto function = mk_AT_TAC<at::FunctionDef_>( atac );
    function->name = atac->name;
    for ( auto instr : atac->instructions ) {
        std::visit( overloaded {
                        [ &function, this ]( tac::Return r ) -> void { ret( r, function->instructions ); },
                        [ &function, this ]( tac::Unary r ) -> void { unary( r, function->instructions ); },
                        [ &function, this ]( tac::Binary r ) -> void { binary( r, function->instructions ); },
                        [ &function, this ]( tac::Copy r ) -> void { copy( r, function->instructions ); },
                        [ &function, this ]( tac::Jump r ) -> void { jump( r, function->instructions ); },
                        [ &function, this ]( tac::JumpIfZero r ) -> void {
                            jumpIfZero<tac::JumpIfZero>( r, true, function->instructions );
                        },
                        [ &function, this ]( tac::JumpIfNotZero r ) -> void {
                            jumpIfZero<tac::JumpIfNotZero>( r, false, function->instructions );
                        },
                        [ &function, this ]( tac::Label r ) -> void { label( r, function->instructions ); },
                    },
                    instr );
    }
    return function;
};

void AssemblyGen::ret( const tac::Return atac, std::vector<at::Instruction>& instructions ) {
    auto mov = mk_AT_TAC<at::Mov_>( atac, value( atac->value ), mk_reg( atac, "eax" ) );
    instructions.push_back( mov );
    auto ret = mk_AT_TAC<at::Ret_>( atac );
    instructions.push_back( ret );
};

void AssemblyGen::unary( const tac::Unary atac, std::vector<at::Instruction>& instructions ) {
    auto mov = mk_AT_TAC<at::Mov_>( atac, value( atac->src ), value( atac->dst ) );
    instructions.push_back( mov );
    auto unary = mk_AT_TAC<at::Unary_>( atac );
    switch ( atac->op ) {
    case tac::UnaryOpType::Complement :
        unary->op = at::UnaryOpType::NOT;
        break;
    case tac::UnaryOpType::Negate :
        unary->op = at::UnaryOpType::NEG;
        break;
    default :
        break;
    }
    unary->operand = value( atac->dst );
    instructions.push_back( unary );
};

void AssemblyGen::binary( const tac::Binary atac, std::vector<at::Instruction>& instructions ) {
    if ( atac->op == tac::BinaryOpType::Divide || atac->op == tac::BinaryOpType::Modulo ) {
        idiv( atac, instructions );
        return;
    }

    auto mov = mk_AT_TAC<at::Mov_>( atac, value( atac->src1 ), value( atac->dst ) );
    instructions.push_back( mov );
    auto binary = mk_AT_TAC<at::Binary_>( atac );
    switch ( atac->op ) {
    case tac::BinaryOpType::Add :
        binary->op = at::BinaryOpType::ADD;
        break;
    case tac::BinaryOpType::Subtract :
        binary->op = at::BinaryOpType::SUB;
        break;
    case tac::BinaryOpType::Multiply :
        binary->op = at::BinaryOpType::MUL;
        break;
    case tac::BinaryOpType::BitwiseAnd :
        binary->op = at::BinaryOpType::AND;
        break;
    case tac::BinaryOpType::BitwiseOr :
        binary->op = at::BinaryOpType::OR;
        break;
    case tac::BinaryOpType::BitwiseXor :
        binary->op = at::BinaryOpType::XOR;
        break;
    case tac::BinaryOpType::ShiftLeft :
        binary->op = at::BinaryOpType::SHL;
        break;
    case tac::BinaryOpType::ShiftRight :
        binary->op = at::BinaryOpType::SHR;
        break;
    default :
    }
    binary->operand1 = value( atac->src2 );
    binary->operand2 = value( atac->dst );
    instructions.push_back( binary );
}

void AssemblyGen::idiv( const tac::Binary atac, std::vector<at::Instruction>& instructions ) {

    auto ax = mk_reg( atac, "eax" );
    auto dx = mk_reg( atac, "edx" );

    // Mov(src1, Reg(AX))
    auto mov = mk_AT_TAC<at::Mov_>( atac, value( atac->src1 ), ax );
    instructions.push_back( mov );

    // Cdq
    instructions.push_back( mk_AT_TAC<at::Cdq_>( atac ) );

    // Idiv(src2)
    auto idiv = mk_AT_TAC<at::Idiv_>( atac, value( atac->src2 ) );
    instructions.push_back( idiv );

    // Mov(Reg(AX), dst)
    mov = mk_AT_TAC<at::Mov_>( atac );
    if ( atac->op == tac::BinaryOpType::Divide ) {
        mov->src = ax;
    } else {
        // Modulo
        mov->src = dx;
    }
    mov->dst = value( atac->dst );
    instructions.push_back( mov );
}

void AssemblyGen::jump( const tac::Jump atac, std::vector<at::Instruction>& instructions ) {
    auto j = mk_AT_TAC<at::Jump_>( atac, atac->target );
    instructions.push_back( j );
}

void AssemblyGen::copy( const tac::Copy atac, std::vector<at::Instruction>& instructions ) {
    auto mov = mk_AT_TAC<at::Mov_>( atac, value( atac->src ), value( atac->dst ) );
    instructions.push_back( mov );
}

void AssemblyGen::label( const tac::Label atac, std::vector<at::Instruction>& instructions ) {
    auto label = mk_AT_TAC<at::Label_>( atac, atac->name );
    instructions.push_back( label );
}

at::Operand AssemblyGen::value( const tac::Value& atac ) {
    return std::visit( overloaded { [ this ]( tac::Constant c ) -> at::Operand { return constant( c ); },
                                    [ this ]( tac::Variable v ) -> at::Operand { return pseudo( v ); } },
                       atac );
}

at::Operand AssemblyGen::constant( const tac::Constant& atac ) {
    return mk_AT_TAC<at::Imm_>( atac, atac->value );
};

at::Operand AssemblyGen::pseudo( tac::Variable atac ) {
    return mk_AT_TAC<at::Pseudo_>( atac, atac->name );
}