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
    for ( auto& funct : atac->functions ) {
        auto function_def = function( funct );
        // FIX
        program->function = function_def;
    }
    return program;
};
arm64_at::FunctionDef ARMAssemblyGen::function( tac::FunctionDef atac ) {
    auto funct = mk_node<arm64_at::FunctionDef_>( atac );
    funct->name = atac->name;
    for ( auto instr : atac->instructions ) {
        std::visit( overloaded {
                        [ &funct, this ]( tac::Return r ) -> void { ret( r, funct->instructions ); },
                        [ &funct, this ]( tac::Unary u ) -> void { unary( u, funct->instructions ); },
                        []( tac::Binary ) -> void {},
                        []( tac::Copy ) -> void {},
                        []( tac::Jump ) -> void {},
                        []( tac::JumpIfZero ) -> void {},
                        []( tac::JumpIfNotZero ) -> void {},
                        []( tac::Label ) -> void {},
                        []( tac::FunCall ) -> void {},
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
        unary->op = arm64_at::UnaryOpType::NOT;
        break;
    case tac::UnaryOpType::Negate :
        unary->op = arm64_at::UnaryOpType::NEG;
        break;
    default :
        break;
    }
    unary->dst = value( atac->dst );
    unary->src = value( atac->src );
    instructions.emplace_back( unary );
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
