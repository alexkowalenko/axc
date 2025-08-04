//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#include "assemblyFixInstruct.h"

#include "common.h"
#include "spdlog/spdlog.h"
#include "x86_at/includes.h"
#include "x86_common.h"

AssemblyFixInstruct::AssemblyFixInstruct() {
    ax = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::AX, x86_at::RegisterSize::Long );
    cl = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::CX, x86_at::RegisterSize::Byte );
    cx = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::CX, x86_at::RegisterSize::Long );
    dx = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::DX, x86_at::RegisterSize::Long );
    r10 = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::R10, x86_at::RegisterSize::Long );
    r11 = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::R11, x86_at::RegisterSize::Long );
}

void AssemblyFixInstruct::filter( x86_at::Program program ) {
    program->accept( this );
}

void AssemblyFixInstruct::visit_Program( const x86_at::Program ast ) {
    for (auto const& funct : ast->functions) {
        funct->accept( this );
    }
}

void AssemblyFixInstruct::visit_FunctionDef( const x86_at::FunctionDef ast ) {
    current_function = ast;
    current_instructions.clear();

    // Look for MOV instructions
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( x86_at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( x86_at::Unary u ) -> void { current_instructions.push_back( u ); },
                                 [ this ]( x86_at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( x86_at::Cmp b ) -> void { b->accept( this ); },
                                 [ this ]( x86_at::AllocateStack a ) -> void { a->accept( this ); },
                                 [ this ]( x86_at::DeallocateStack a ) -> void { a->accept( this ); },
                                 [ this ]( x86_at::Push p ) -> void { p->accept( this ); },
                                 [ this ]( x86_at::Call c ) -> void { c->accept( this ); },
                                 [ this ]( x86_at::Idiv i ) -> void { i->accept( this ); },
                                 [ this ]( x86_at::Cdq c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( x86_at::Jump c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( x86_at::JumpCC c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( x86_at::SetCC c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( x86_at::Label c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( x86_at::Ret r ) -> void { current_instructions.push_back( r ); } },
                    instr );
    }
    ast->instructions = current_instructions;
}

void AssemblyFixInstruct::visit_Mov( const x86_at::Mov ast ) {
    // MOV instructions can't have stack locations in both operands
    if ( std::holds_alternative<x86_at::Stack>( ast->src ) && std::holds_alternative<x86_at::Stack>( ast->dst ) ) {
        auto src = ast->src;
        auto dst = ast->dst;

        auto mov1 = mk_node<x86_at::Mov_>( ast, src, r10 );
        current_instructions.push_back( mov1 );
        auto mov2 = mk_node<x86_at::Mov_>( ast, r10, dst );
        current_instructions.push_back( mov2 );
    } else {
        // Other MOV instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Idiv( const x86_at::Idiv ast ) {
    // Can't have an Immediate as a source
    if ( std::holds_alternative<x86_at::Imm>( ast->src ) ) {

        auto mov1 = mk_node<x86_at::Mov_>( ast, ast->src, r10 );
        current_instructions.push_back( mov1 );

        auto idiv = mk_node<x86_at::Idiv_>( ast, r10 );
        current_instructions.push_back( idiv );
    } else {
        // Other Idiv instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Binary( const x86_at::Binary ast ) {
    if ( ast->op == x86_at::BinaryOpType::ADD || ast->op == x86_at::BinaryOpType::SUB ||
         ast->op == x86_at::BinaryOpType::AND || ast->op == x86_at::BinaryOpType::OR ||
         ast->op == x86_at::BinaryOpType::XOR ) {
        // These instructions can't have stack locations in both operands
        if ( std::holds_alternative<x86_at::Stack>( ast->operand1 ) &&
             std::holds_alternative<x86_at::Stack>( ast->operand2 ) ) {

            auto mov1 = mk_node<x86_at::Mov_>( ast, ast->operand1, r10 );
            current_instructions.push_back( mov1 );

            auto binary = mk_node<x86_at::Binary_>( ast, ast->op, r10, ast->operand2 );
            current_instructions.push_back( binary );

            // There is any extra rule that the second operand can't be a constant (pg. 88),
            // not implemented here.
        } else {
            // Other Add/Sub instructions
            current_instructions.push_back( ast );
        }
    } else if ( ast->op == x86_at::BinaryOpType::MUL ) {
        // This instruction can't have a stack location in the second argument
        if ( std::holds_alternative<x86_at::Stack>( ast->operand2 ) ) {

            auto mov1 = mk_node<x86_at::Mov_>( ast, ast->operand2, r11 );
            current_instructions.push_back( mov1 );

            auto binary = mk_node<x86_at::Binary_>( ast, ast->op, ast->operand1, r11 );
            current_instructions.push_back( binary );

            auto mov2 = mk_node<x86_at::Mov_>( ast, r11, ast->operand2 );
            current_instructions.push_back( mov2 );
        } else {
            // Other Mul instructions
            current_instructions.push_back( ast );
        }
    } else if ( ast->op == x86_at::BinaryOpType::SHL || ast->op == x86_at::BinaryOpType::SHR ) {
        // These instruction encoding only allows a register shift count (cl) when the destination is a register.

        auto mov1 = mk_node<x86_at::Mov_>( ast, ast->operand2, ax );
        current_instructions.push_back( mov1 );

        auto mov2 = mk_node<x86_at::Mov_>( ast, ast->operand1, cx );
        current_instructions.push_back( mov2 );

        auto binary = mk_node<x86_at::Binary_>( ast, ast->op, cl, ax );
        current_instructions.push_back( binary );

        mov2 = mk_node<x86_at::Mov_>( ast, ax, ast->operand2 );
        current_instructions.push_back( mov2 );
    } else {
        // Other Binary instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Cmp( const x86_at::Cmp ast ) {
    // CMP instructions can't have stack locations in both operands
    if ( std::holds_alternative<x86_at::Stack>( ast->operand1 ) &&
         std::holds_alternative<x86_at::Stack>( ast->operand2 ) ) {
        auto src = ast->operand1;
        auto dst = ast->operand2;

        auto c1 = mk_node<x86_at::Mov_>( ast, src, r10 );
        current_instructions.push_back( c1 );
        auto c2 = mk_node<x86_at::Cmp_>( ast, r10, dst );
        current_instructions.push_back( c2 );
        return;
    }
    if ( std::holds_alternative<x86_at::Imm>( ast->operand2 ) ) {
        // Second operand can't be a constant

        // movl operand2, %r11d
        auto mov1 = mk_node<x86_at::Mov_>( ast, ast->operand2, r11 );
        current_instructions.push_back( mov1 );
        // cmpl operand1, %r11d
        auto mov2 = mk_node<x86_at::Cmp_>( ast, ast->operand1, r11 );
        current_instructions.push_back( mov2 );
    } else {
        // Other MOV instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_AllocateStack( const x86_at::AllocateStack ast ) {
    // Add Allocate Stack Instruction
    if ( current_function->stack_size != 0) {
        auto allocate = mk_node<x86_at::AllocateStack_>( ast );
        allocate->size = stack_increment * current_function->stack_size;
        allocate->size = allocate->size + 15 & ~15; // Align to 16 bytes
        spdlog::debug( "Adding AllocateStack instruction {} - {}", ast->size, allocate->size );
        current_instructions.push_back( allocate );
    }
}

void AssemblyFixInstruct::visit_DeallocateStack( const x86_at::DeallocateStack ast ) {
    current_instructions.push_back( ast );
}

void AssemblyFixInstruct::visit_Call( const x86_at::Call ast ) {
    current_instructions.push_back( ast );
}
