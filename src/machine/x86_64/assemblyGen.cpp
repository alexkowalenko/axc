//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "assemblyGen.h"

#include <map>

#include "common.h"
#include "x86_common.h"

AssemblyGen::AssemblyGen() {
    zero = std::make_shared<x86_at::Imm_>( Location(), 0 );
    ax = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::AX, x86_at::RegisterSize::Long );
    dx = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::DX, x86_at::RegisterSize::Long );
}

x86_at::Program AssemblyGen::generate( const tac::Program atac ) {
    auto program = mk_node<x86_at::Program_>( atac );
    program->function = functionDef( atac->function );
    return program;
}

x86_at::FunctionDef AssemblyGen::functionDef( const tac::FunctionDef atac ) {
    auto function = mk_node<x86_at::FunctionDef_>( atac );
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

void AssemblyGen::ret( const tac::Return atac, std::vector<x86_at::Instruction>& instructions ) {
    // Mov(value, %eax)
    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->value ), ax );
    instructions.push_back( mov );
    // Ret
    auto ret = mk_node<x86_at::Ret_>( atac );
    instructions.push_back( ret );
};

void AssemblyGen::unary( const tac::Unary atac, std::vector<x86_at::Instruction>& instructions ) {
    // Handle not differently
    if ( atac->op == tac::UnaryOpType::Not ) {
        unary_not( atac, instructions );
        return;
    }

    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src ), value( atac->dst ) );
    instructions.push_back( mov );
    auto unary = mk_node<x86_at::Unary_>( atac );
    switch ( atac->op ) {
    case tac::UnaryOpType::Complement :
        unary->op = x86_at::UnaryOpType::NOT;
        break;
    case tac::UnaryOpType::Negate :
        unary->op = x86_at::UnaryOpType::NEG;
        break;
    default :
        break;
    }
    unary->operand = value( atac->dst );
    instructions.push_back( unary );
};

void AssemblyGen::unary_not( const tac::Unary atac, std::vector<x86_at::Instruction>& instructions ) {
    // Cmp(Imm(0), src)
    auto cmp = mk_node<x86_at::Cmp_>( atac, zero, value( atac->src ) );
    instructions.push_back( cmp );
    // Mov(Imm(0), dst)
    auto mov = mk_node<x86_at::Mov_>( atac, zero, value( atac->dst ) );
    instructions.push_back( mov );
    // SetCC(E, dst)
    auto sete = mk_node<x86_at::SetCC_>( atac, x86_at::CondCode::E, value( atac->dst ) );
    instructions.push_back( sete );
}

void AssemblyGen::binary( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions ) {
    // Handle divide and mod differently
    if ( atac->op == tac::BinaryOpType::Divide || atac->op == tac::BinaryOpType::Modulo ) {
        idiv( atac, instructions );
        return;
    }

    // Handle relational operations differently
    if ( atac->op == tac::BinaryOpType::Equal || atac->op == tac::BinaryOpType::NotEqual ||
         atac->op == tac::BinaryOpType::Less || atac->op == tac::BinaryOpType::LessEqual ||
         atac->op == tac::BinaryOpType::Greater || atac->op == tac::BinaryOpType::GreaterEqual ) {
        binary_relation( atac, instructions );
        return;
    }

    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src1 ), value( atac->dst ) );
    instructions.push_back( mov );
    auto binary = mk_node<x86_at::Binary_>( atac );
    switch ( atac->op ) {
    case tac::BinaryOpType::Add :
        binary->op = x86_at::BinaryOpType::ADD;
        break;
    case tac::BinaryOpType::Subtract :
        binary->op = x86_at::BinaryOpType::SUB;
        break;
    case tac::BinaryOpType::Multiply :
        binary->op = x86_at::BinaryOpType::MUL;
        break;
    case tac::BinaryOpType::BitwiseAnd :
        binary->op = x86_at::BinaryOpType::AND;
        break;
    case tac::BinaryOpType::BitwiseOr :
        binary->op = x86_at::BinaryOpType::OR;
        break;
    case tac::BinaryOpType::BitwiseXor :
        binary->op = x86_at::BinaryOpType::XOR;
        break;
    case tac::BinaryOpType::ShiftLeft :
        binary->op = x86_at::BinaryOpType::SHL;
        break;
    case tac::BinaryOpType::ShiftRight :
        binary->op = x86_at::BinaryOpType::SHR;
        break;
    default :
    }
    binary->operand1 = value( atac->src2 );
    binary->operand2 = value( atac->dst );
    instructions.push_back( binary );
}

void AssemblyGen::idiv( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions ) {

    // Mov(src1, Reg(AX))
    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src1 ), ax );
    instructions.push_back( mov );

    // Cdq
    instructions.push_back( mk_node<x86_at::Cdq_>( atac ) );

    // Idiv(src2)
    auto idiv = mk_node<x86_at::Idiv_>( atac, value( atac->src2 ) );
    instructions.push_back( idiv );

    // Mov(Reg(AX), dst)
    mov = mk_node<x86_at::Mov_>( atac );
    if ( atac->op == tac::BinaryOpType::Divide ) {
        mov->src = ax;
    } else {
        // Modulo
        mov->src = dx;
    }
    mov->dst = value( atac->dst );
    instructions.push_back( mov );
}

const std::map<tac::BinaryOpType, x86_at::CondCode> condCodeMap = {
    { tac::BinaryOpType::Equal, x86_at::CondCode::E },   { tac::BinaryOpType::NotEqual, x86_at::CondCode::NE },
    { tac::BinaryOpType::Less, x86_at::CondCode::L },    { tac::BinaryOpType::LessEqual, x86_at::CondCode::LE },
    { tac::BinaryOpType::Greater, x86_at::CondCode::G }, { tac::BinaryOpType::GreaterEqual, x86_at::CondCode::GE },
};

void AssemblyGen::binary_relation( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions ) {
    // Cmp(Imm(0), src)
    auto cmp = mk_node<x86_at::Cmp_>( atac, value( atac->src2 ), value( atac->src1 ) );
    instructions.push_back( cmp );
    // Mov(Imm(0), dst)
    auto mov = mk_node<x86_at::Mov_>( atac, zero, value( atac->dst ) );
    instructions.push_back( mov );
    // SetCC(condCode, dst)
    auto setcc = mk_node<x86_at::SetCC_>( atac, condCodeMap.at( atac->op ), value( atac->dst ) );
    instructions.push_back( setcc );
}

void AssemblyGen::jump( const tac::Jump atac, std::vector<x86_at::Instruction>& instructions ) {
    auto j = mk_node<x86_at::Jump_>( atac, atac->target );
    instructions.push_back( j );
}

void AssemblyGen::copy( const tac::Copy atac, std::vector<x86_at::Instruction>& instructions ) {
    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src ), value( atac->dst ) );
    instructions.push_back( mov );
}

void AssemblyGen::label( const tac::Label atac, std::vector<x86_at::Instruction>& instructions ) {
    auto label = mk_node<x86_at::Label_>( atac, atac->name );
    instructions.push_back( label );
}

x86_at::Operand AssemblyGen::value( const tac::Value& atac ) {
    return std::visit( overloaded { [ this ]( tac::Constant c ) -> x86_at::Operand { return constant( c ); },
                                    [ this ]( tac::Variable v ) -> x86_at::Operand { return pseudo( v ); } },
                       atac );
}

x86_at::Operand AssemblyGen::constant( const tac::Constant& atac ) {
    return mk_node<x86_at::Imm_>( atac, atac->value );
};

x86_at::Operand AssemblyGen::pseudo( tac::Variable atac ) {
    return mk_node<x86_at::Pseudo_>( atac, atac->name );
}