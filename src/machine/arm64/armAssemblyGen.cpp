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
    program->function = function( atac->function );
    return program;
};
arm64_at::FunctionDef ARMAssemblyGen::function( const tac::FunctionDef& atac ) {
    auto funct = mk_node<arm64_at::FunctionDef_>( atac );
    funct->name = atac->name;
    for ( auto instr : atac->instructions ) {
        std::visit( overloaded {
                        [ &funct, this ]( tac::Return r ) -> void { ret( r, funct->instructions ); },
                        [ this ]( tac::Unary ) -> void {},
                        [ this ]( tac::Binary ) -> void {},
                        [ this ]( tac::Copy ) -> void {},
                        [ this ]( tac::Jump ) -> void {},
                        [ this ]( tac::JumpIfZero ) -> void {},
                        [ this ]( tac::JumpIfNotZero ) -> void {},
                        [ this ]( tac::Label ) -> void {},
                    },
                    instr );
    }
    return funct;
}

void ARMAssemblyGen::ret( const tac::Return atac, std::vector<arm64_at::Instruction>& instructions ) {
    // mov(value, x0)
    auto mov = mk_node<arm64_at::Mov_>( atac, value( atac->value ), x0 );
    instructions.push_back( mov );

    // ret
    auto ret = mk_node<arm64_at::Ret_>( atac );
    instructions.push_back( ret );
}

arm64_at::Operand ARMAssemblyGen::value( const tac::Value& atac ) {
    return std::visit( overloaded { [ this ]( tac::Constant c ) -> arm64_at::Operand { return constant( c ); },
                                    [ this ]( tac::Variable v ) -> arm64_at::Operand {
                                        throw CodeException( v->location,
                                                             "Variable not supported in ARM64 assembly generation" );
                                    } },
                       atac );
}

arm64_at::Operand ARMAssemblyGen::constant( const tac::Constant& atac ) {
    if ( atac->value == 0 ) {
        return xzr; // Use zero register for constant 0
    }
    return mk_node<arm64_at::Imm_>( atac, atac->value );
}
