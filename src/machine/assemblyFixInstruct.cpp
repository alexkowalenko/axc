//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#include "assemblyFixInstruct.h"

#include "../at/includes.h"
#include "../common.h"
#include "x86_common.h"

AssemblyFixInstruct::AssemblyFixInstruct() {
    ax = std::make_shared<at::Register_>( Location(), at::RegisterName::AX, at::RegisterSize::Long );
    cl = std::make_shared<at::Register_>( Location(), at::RegisterName::CX, at::RegisterSize::Byte );
    cx = std::make_shared<at::Register_>( Location(), at::RegisterName::CX, at::RegisterSize::Long );
    dx = std::make_shared<at::Register_>( Location(), at::RegisterName::DX, at::RegisterSize::Long );
    r10 = std::make_shared<at::Register_>( Location(), at::RegisterName::R10, at::RegisterSize::Long );
    r11 = std::make_shared<at::Register_>( Location(), at::RegisterName::R11, at::RegisterSize::Long );
}

void AssemblyFixInstruct::filter( at::Program program ) {
    program->accept( this );
}

void AssemblyFixInstruct::visit_Program( const at::Program ast ) {
    ast->function->accept( this );
}

void AssemblyFixInstruct::visit_FunctionDef( const at::FunctionDef ast ) {
    current_instructions.clear();

    // Add Allocate Stack Instruction
    auto allocate = mk_node<at::AllocateStack_>( ast );
    allocate->size = stack_increment * number_stack_locations;
    current_instructions.push_back( allocate );

    // Look for MOV instructions
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( at::Unary u ) -> void { current_instructions.push_back( u ); },
                                 [ this ]( at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( at::Cmp b ) -> void { b->accept( this ); },
                                 [ this ]( at::AllocateStack a ) -> void { current_instructions.push_back( a ); },
                                 [ this ]( at::Idiv i ) -> void { i->accept( this ); },
                                 [ this ]( at::Cdq c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( at::Jump c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( at::JumpCC c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( at::SetCC c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( at::Label c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( at::Ret r ) -> void { current_instructions.push_back( r ); } },
                    instr );
    }
    ast->instructions = current_instructions;
}

void AssemblyFixInstruct::visit_Mov( const at::Mov ast ) {
    // MOV instructions can't have stack locations in both operands
    if ( std::holds_alternative<at::Stack>( ast->src ) && std::holds_alternative<at::Stack>( ast->dst ) ) {
        auto src = ast->src;
        auto dst = ast->dst;

        auto mov1 = mk_node<at::Mov_>( ast, src, r10 );
        current_instructions.push_back( mov1 );
        auto mov2 = mk_node<at::Mov_>( ast, r10, dst );
        current_instructions.push_back( mov2 );
    } else {
        // Other MOV instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Idiv( const at::Idiv ast ) {
    // Can't have an Immediate as a source
    if ( std::holds_alternative<at::Imm>( ast->src ) ) {

        auto mov1 = mk_node<at::Mov_>( ast, ast->src, r10 );
        current_instructions.push_back( mov1 );

        auto idiv = mk_node<at::Idiv_>( ast, r10 );
        current_instructions.push_back( idiv );
    } else {
        // Other Idiv instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Binary( const at::Binary ast ) {
    if ( ast->op == at::BinaryOpType::ADD || ast->op == at::BinaryOpType::SUB || ast->op == at::BinaryOpType::AND ||
         ast->op == at::BinaryOpType::OR || ast->op == at::BinaryOpType::XOR ) {
        // These instructions can't have stack locations in both operands
        if ( std::holds_alternative<at::Stack>( ast->operand1 ) &&
             std::holds_alternative<at::Stack>( ast->operand2 ) ) {

            auto mov1 = mk_node<at::Mov_>( ast, ast->operand1, r10 );
            current_instructions.push_back( mov1 );

            auto binary = mk_node<at::Binary_>( ast, ast->op, r10, ast->operand2 );
            current_instructions.push_back( binary );

            // There is any extra rule that the second operand can't be a constant (pg. 88),
            // not implemented here.
        } else {
            // Other Add/Sub instructions
            current_instructions.push_back( ast );
        }
    } else if ( ast->op == at::BinaryOpType::MUL ) {
        // This instruction can't have a stack location in the second argument
        if ( std::holds_alternative<at::Stack>( ast->operand2 ) ) {

            auto mov1 = mk_node<at::Mov_>( ast, ast->operand2, r11 );
            current_instructions.push_back( mov1 );

            auto binary = mk_node<at::Binary_>( ast, ast->op, ast->operand1, r11 );
            current_instructions.push_back( binary );

            auto mov2 = mk_node<at::Mov_>( ast, r11, ast->operand2 );
            current_instructions.push_back( mov2 );
        } else {
            // Other Mul instructions
            current_instructions.push_back( ast );
        }
    } else if ( ast->op == at::BinaryOpType::SHL || ast->op == at::BinaryOpType::SHR ) {
        // These instruction encoding only allows a register shift count (cl) when the destination is a register.

        auto mov1 = mk_node<at::Mov_>( ast, ast->operand2, ax );
        current_instructions.push_back( mov1 );

        auto mov2 = mk_node<at::Mov_>( ast, ast->operand1, cx );
        current_instructions.push_back( mov2 );

        auto binary = mk_node<at::Binary_>( ast, ast->op, cl, ax );
        current_instructions.push_back( binary );

        mov2 = mk_node<at::Mov_>( ast, ax, ast->operand2 );
        current_instructions.push_back( mov2 );
    } else {
        // Other Binary instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Cmp( const at::Cmp ast ) {
    // CMP instructions can't have stack locations in both operands
    if ( std::holds_alternative<at::Stack>( ast->operand1 ) && std::holds_alternative<at::Stack>( ast->operand2 ) ) {
        auto src = ast->operand1;
        auto dst = ast->operand2;

        auto c1 = mk_node<at::Mov_>( ast, src, r10 );
        current_instructions.push_back( c1 );
        auto c2 = mk_node<at::Cmp_>( ast, r10, dst );
        current_instructions.push_back( c2 );
        return;
    }
    if ( std::holds_alternative<at::Imm>( ast->operand2 ) ) {
        // Second operand can't be a constant

        // movl operand2, %r11d
        auto mov1 = mk_node<at::Mov_>( ast, ast->operand2, r11 );
        current_instructions.push_back( mov1 );
        // cmpl operand1, %r11d
        auto mov2 = mk_node<at::Cmp_>( ast, ast->operand1, r11 );
        current_instructions.push_back( mov2 );
    } else {
        // Other MOV instructions
        current_instructions.push_back( ast );
    }
}
