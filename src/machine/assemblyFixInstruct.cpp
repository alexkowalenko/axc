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

void AssemblyFixInstruct::filter( at::Program program ) {
    program->accept( this );
}

void AssemblyFixInstruct::visit_Program( const at::Program ast ) {
    ast->function->accept( this );
}

void AssemblyFixInstruct::visit_FunctionDef( const at::FunctionDef ast ) {
    current_instructions.clear();

    // Add Allocate Stack Instruction
    auto allocate = make_AT<at::AllocateStack_>( ast );
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
        auto reg = mk_reg( ast, "R10D" );

        auto mov1 = make_AT<at::Mov_>( ast, src, reg );
        current_instructions.push_back( mov1 );
        auto mov2 = make_AT<at::Mov_>( ast, reg, dst );
        current_instructions.push_back( mov2 );
    } else {
        // Other MOV instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Idiv( const at::Idiv ast ) {
    // Can't have an Immediate as a source
    if ( std::holds_alternative<at::Imm>( ast->src ) ) {
        auto reg = mk_reg( ast, "R10D" );

        auto mov1 = make_AT<at::Mov_>( ast, ast->src, reg );
        current_instructions.push_back( mov1 );

        auto idiv = make_AT<at::Idiv_>( ast, reg );
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
            auto reg = mk_reg( ast, "R10D" );

            auto mov1 = make_AT<at::Mov_>( ast, ast->operand1, reg );
            current_instructions.push_back( mov1 );

            auto binary = make_AT<at::Binary_>( ast, ast->op, reg, ast->operand2 );
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
            auto reg = mk_reg( ast, "R11D" );

            auto mov1 = make_AT<at::Mov_>( ast, ast->operand2, reg );
            current_instructions.push_back( mov1 );

            auto binary = make_AT<at::Binary_>( ast, ast->op, ast->operand1, reg );
            current_instructions.push_back( binary );

            auto mov2 = make_AT<at::Mov_>( ast, reg, ast->operand2 );
            current_instructions.push_back( mov2 );
        } else {
            // Other Mul instructions
            current_instructions.push_back( ast );
        }
    } else if ( ast->op == at::BinaryOpType::SHL || ast->op == at::BinaryOpType::SHR ) {
        // These instruction encoding only allows a register shift count (cl) when the destination is a register.
        auto ecx = mk_reg( ast, "ecx" );
        auto eax = mk_reg( ast, "eax" );
        auto cl = mk_reg( ast, "cl" );

        auto mov1 = make_AT<at::Mov_>( ast, ast->operand2, eax );
        current_instructions.push_back( mov1 );

        auto mov2 = make_AT<at::Mov_>( ast, ast->operand1, ecx );
        current_instructions.push_back( mov2 );

        auto binary = make_AT<at::Binary_>( ast, ast->op, cl, eax );
        current_instructions.push_back( binary );

        mov2 = make_AT<at::Mov_>( ast, eax, ast->operand2 );
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
        auto reg = mk_reg( ast, "R10D" );

        auto c1 = make_AT<at::Cmp_>( ast, src, reg );
        current_instructions.push_back( c1 );
        auto c2 = make_AT<at::Cmp_>( ast, reg, dst );
        current_instructions.push_back( c2 );
    } if ( std::holds_alternative<at::Imm>( ast->operand2 ) ) {
        // Second operand can't be a constant
        auto reg = mk_reg( ast, "R11D" );

        // movl operand2, %r11d
        auto mov1 = make_AT<at::Mov_>( ast, ast->operand2, reg );
        current_instructions.push_back( mov1 );
        // cmpl operand1, %r11d
        auto mov2 = make_AT<at::Cmp_>( ast, ast->operand1, reg );
        current_instructions.push_back( mov2 );
    }else {
        // Other MOV instructions
        current_instructions.push_back( ast );
    }
}


