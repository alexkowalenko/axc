//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#include "assemblyFixInstruct.h"

#include "at/includes.h"
#include "common.h"

void AssemblyFixInstruct::filter( at::Program program ) {
    program->accept( this );
}

void AssemblyFixInstruct::visit_Program( const at::Program ast ) {
    ast->function->accept( this );
}

void AssemblyFixInstruct::visit_FunctionDef( const at::FunctionDef ast ) {
    std::vector<at::Instruction> instructions;
    instructions.reserve( ast->instructions.size() + 1 );

    // Add Allocate Stack Instruction
    auto allocate = std::make_shared<at::AllocateStack_>( ast->location );
    allocate->size = stack_increment * number_stack_locations;
    instructions.push_back( allocate );

    // Look for MOV instructions
    for ( auto const& instr : ast->instructions ) {
        if ( std::holds_alternative<at::Mov>( instr ) ) {
            auto mov = std::get<at::Mov>( instr );
            if ( std::holds_alternative<at::Stack>( mov->src ) && std::holds_alternative<at::Stack>( mov->dst ) ) {
                auto src = mov->src;
                auto dst = mov->dst;
                auto reg = std::make_shared<at::Register_>( mov->location );
                reg->reg = "R10D";
                auto mov1 = std::make_shared<at::Mov_>( mov->location );
                mov1->src = src;
                mov1->dst = reg;
                instructions.push_back( mov1 );
                auto mov2 = std::make_shared<at::Mov_>( mov->location );
                mov2->src = reg;
                mov2->dst = dst;
                instructions.push_back( mov2 );
            } else {
                // Other MOV instructions
                instructions.push_back( instr );
            }
        } else {
            // Other instructions
            instructions.push_back( instr );
        }
    }
    ast->instructions = instructions;
}