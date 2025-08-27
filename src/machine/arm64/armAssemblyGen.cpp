//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#include "armAssemblyGen.h"

#include "arm64_at/includes.h"
#include "common.h"
#include "exception.h"

ARMAssemblyGen::ARMAssemblyGen() {
    x0 = std::make_shared<arm64_at::Register_>( Location {}, arm64_at::RegisterName::X0 );
    xzr = std::make_shared<arm64_at::Register_>( Location {}, arm64_at::RegisterName::XZR ); // Zero register
}

arm64_at::Program ARMAssemblyGen::generate( const tac::Program atac ) {
    auto program = mk_node<arm64_at::Program_>( atac );
    for ( auto& item : atac->top_level ) {
        std::visit( overloaded { [ this, &program ]( tac::FunctionDef funct ) -> void {
                                    auto function_def = function( funct );
                                    // FIX
                                    program->function = function_def;
                                },
                                 []( tac::StaticVariable ) -> void {} },
                    item );
    }
    return program;
};
arm64_at::FunctionDef ARMAssemblyGen::function( tac::FunctionDef atac ) {
    auto funct = mk_node<arm64_at::FunctionDef_>( atac );
    funct->name = atac->name;
    for ( auto instr : atac->instructions ) {
        std::visit(
            overloaded {
                [ &funct, this ]( tac::Return r ) -> void { ret( r, funct->instructions ); },
                [ &funct, this ]( tac::Unary u ) -> void { unary( u, funct->instructions ); },
                [ &funct, this ]( tac::Binary b ) -> void { binary( b, funct->instructions ); },
                [ &funct, this ]( tac::Copy c ) -> void { copy( c, funct->instructions ); },
                [ &funct, this ]( tac::Jump j ) -> void { branch( j, funct->instructions ); },
                [ &funct, this ]( tac::JumpIfZero j ) -> void { branchIfZero( j, true, funct->instructions ); },
                [ &funct, this ]( tac::JumpIfNotZero j ) -> void { branchIfZero( j, false, funct->instructions ); },
                [ &funct, this ]( tac::Label l ) -> void { label( l, funct->instructions ); },
                []( tac::FunCall ) -> void {},
                []( tac::StaticVariable ) -> void {},
            },
            instr );
    }
    return funct;
}

void ARMAssemblyGen::ret( const tac::Return atac, std::vector<arm64_at::Instruction>& instructions ) {
    // mov(value, x0)
    auto mov = mk_node<arm64_at::Mov_>( atac, value( atac->value ), x0 );
    instructions.emplace_back( mov );

    // ret
    auto ret = mk_node<arm64_at::Ret_>( atac );
    instructions.emplace_back( ret );
}

void ARMAssemblyGen::unary( const tac::Unary atac, std::vector<arm64_at::Instruction>& instructions ) {
    auto unary = mk_node<arm64_at::Unary_>( atac );
    switch ( atac->op ) {
    case tac::UnaryOpType::Complement :
        unary->op = arm64_at::UnaryOpType::BITWISE_NOT;
        break;
    case tac::UnaryOpType::Negate :
        unary->op = arm64_at::UnaryOpType::NEG;
        break;
    case tac::UnaryOpType::Not :
        unary->op = arm64_at::UnaryOpType::LOGICAL_NOT;
        break;
    default :
        break;
    }
    unary->dst = value( atac->dst );
    unary->src = value( atac->src );
    instructions.emplace_back( unary );
}

void ARMAssemblyGen::binary( const tac::Binary atac, std::vector<arm64_at::Instruction>& instructions ) {

    // Handle relational operations differently
    if ( atac->op == tac::BinaryOpType::Equal || atac->op == tac::BinaryOpType::NotEqual ||
         atac->op == tac::BinaryOpType::Less || atac->op == tac::BinaryOpType::LessEqual ||
         atac->op == tac::BinaryOpType::Greater || atac->op == tac::BinaryOpType::GreaterEqual ) {
        binary_compare( atac, instructions );
        return;
    }

    auto binary = mk_node<arm64_at::Binary_>( atac );
    switch ( atac->op ) {
    case tac::BinaryOpType::Add :
        binary->op = arm64_at::BinaryOpType::ADD;
        break;
    case tac::BinaryOpType::Subtract :
        binary->op = arm64_at::BinaryOpType::SUB;
        break;
    case tac::BinaryOpType::Multiply :
        binary->op = arm64_at::BinaryOpType::MUL;
        break;
    case tac::BinaryOpType::Divide :
        binary->op = arm64_at::BinaryOpType::DIV;
        break;
    case tac::BinaryOpType::Modulo :
        binary->op = arm64_at::BinaryOpType::MOD;
        break;
    case tac::BinaryOpType::BitwiseAnd :
        binary->op = arm64_at::BinaryOpType::AND;
        break;
    case tac::BinaryOpType::BitwiseOr :
        binary->op = arm64_at::BinaryOpType::OR;
        break;
    case tac::BinaryOpType::BitwiseXor :
        binary->op = arm64_at::BinaryOpType::XOR;
        break;
    case tac::BinaryOpType::ShiftLeft :
        binary->op = arm64_at::BinaryOpType::SHL;
        break;
    case tac::BinaryOpType::ShiftRight :
        binary->op = arm64_at::BinaryOpType::SHR;
        break;
    default :;
    }
    binary->dst = value( atac->dst );
    binary->src1 = value( atac->src1 );
    binary->src2 = value( atac->src2 );
    instructions.emplace_back( binary );
}

const std::map<tac::BinaryOpType, arm64_at::CondCode> condCodeMap = {
    { tac::BinaryOpType::Equal, arm64_at::CondCode::EQ },   { tac::BinaryOpType::NotEqual, arm64_at::CondCode::NE },
    { tac::BinaryOpType::Less, arm64_at::CondCode::LT },    { tac::BinaryOpType::LessEqual, arm64_at::CondCode::LE },
    { tac::BinaryOpType::Greater, arm64_at::CondCode::GT }, { tac::BinaryOpType::GreaterEqual, arm64_at::CondCode::GE },
};

void ARMAssemblyGen::binary_compare( tac::Binary atac, std::vector<arm64_at::Instruction>& instructions ) {
    auto sub = mk_node<arm64_at::Binary_>( atac, arm64_at::BinaryOpType::SUB, value( atac->dst ), value( atac->src1 ),
                                           value( atac->src2 ) );
    instructions.emplace_back( sub );
    auto cmp = mk_node<arm64_at::Cset_>( atac, value( atac->dst ), condCodeMap.at( atac->op ) );
    instructions.emplace_back( cmp );
}

void ARMAssemblyGen::copy( const tac::Copy atac, std::vector<arm64_at::Instruction>& instructions ) {
    auto mov = mk_node<arm64_at::Mov_>( atac, value( atac->src ), value( atac->dst ) );
    instructions.emplace_back( mov );
}

void ARMAssemblyGen::branch( tac::Jump atac, std::vector<arm64_at::Instruction>& instructions ) {
    auto branch = mk_node<arm64_at::Branch_>( atac, atac->target );
    instructions.emplace_back( branch );
}

void ARMAssemblyGen::label( tac::Label atac, std::vector<arm64_at::Instruction>& instructions ) {
    auto label = mk_node<arm64_at::Label_>( atac, atac->name );
    instructions.emplace_back( label );
}

arm64_at::Operand ARMAssemblyGen::value( const tac::Value atac ) {
    return std::visit( overloaded { [ this ]( tac::Constant c ) -> arm64_at::Operand { return constant( c ); },
                                    []( tac::Variable v ) -> arm64_at::Operand { return pseudo( v ); } },
                       atac );
}

arm64_at::Operand ARMAssemblyGen::constant( const tac::Constant atac ) {
    if ( atac->value == 0 ) {
        return xzr; // Use zero register for constant 0
    }
    return mk_node<arm64_at::Imm_>( atac, atac->value );
}

arm64_at::Operand ARMAssemblyGen::pseudo( tac::Variable atac ) {
    return mk_node<arm64_at::Pseudo_>( atac, atac->name );
}
