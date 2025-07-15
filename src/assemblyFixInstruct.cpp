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

template <typename T> constexpr std::shared_ptr<T> make_AT( const std::shared_ptr<at::Base> b ) {
    return std::make_shared<T>( b->location );
}

constexpr at::Register mk_reg( const std::shared_ptr<at::Base> b, const std::string_view name ) {
    auto reg = make_AT<at::Register_>( b );
    reg->reg = name;
    return reg;
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
    auto allocate = make_AT<at::AllocateStack_>( ast );
    allocate->size = stack_increment * number_stack_locations;
    current_instructions.push_back( allocate );

    // Look for MOV instructions
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( at::Unary u ) -> void { current_instructions.push_back( u ); },
                                 [ this ]( at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( at::AllocateStack a ) -> void { current_instructions.push_back( a ); },
                                 [ this ]( at::Idiv i ) -> void { i->accept( this ); },
                                 [ this ]( at::Cdq c ) -> void { current_instructions.push_back( c ); },
                                 [ this ]( at::Ret r ) -> void { current_instructions.push_back( r ); } },
                    instr );
    }
    ast->instructions = current_instructions;
}

void AssemblyFixInstruct::visit_Mov( const at::Mov ast ) {
    if ( std::holds_alternative<at::Stack>( ast->src ) && std::holds_alternative<at::Stack>( ast->dst ) ) {
        auto src = ast->src;
        auto dst = ast->dst;
        auto reg = mk_reg( ast, "R10D" );

        auto mov1 = make_AT<at::Mov_>( ast );
        mov1->src = src;
        mov1->dst = reg;
        current_instructions.push_back( mov1 );
        auto mov2 = make_AT<at::Mov_>( ast );
        mov2->src = reg;
        mov2->dst = dst;
        current_instructions.push_back( mov2 );
    } else {
        // Other MOV instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Idiv( const at::Idiv ast ) {
    if ( std::holds_alternative<at::Imm>( ast->src ) ) {
        auto reg = mk_reg( ast, "R10D" );

        auto mov1 = make_AT<at::Mov_>( ast );
        mov1->src = ast->src;
        mov1->dst = reg;
        current_instructions.push_back( mov1 );

        auto idiv = make_AT<at::Idiv_>( ast );
        idiv->src = reg;
        current_instructions.push_back( idiv );
    } else {
        // Other Idiv instructions
        current_instructions.push_back( ast );
    }
}

void AssemblyFixInstruct::visit_Binary( const at::Binary ast ) {
    if ( ast->op == at::BinaryOpType::ADD || ast->op == at::BinaryOpType::SUB ) {
        if ( std::holds_alternative<at::Stack>( ast->operand1 ) &&
             std::holds_alternative<at::Stack>( ast->operand2 ) ) {
            auto reg = mk_reg( ast, "R10D" );

            auto mov1 = make_AT<at::Mov_>( ast );
            mov1->src = ast->operand1;
            mov1->dst = reg;
            current_instructions.push_back( mov1 );

            auto binary = make_AT<at::Binary_>( ast );
            binary->op = ast->op;
            binary->operand1 = reg;
            binary->operand2 = ast->operand2;
            current_instructions.push_back( binary );
        } else {
            // Other Add/Sub instructions
            current_instructions.push_back( ast );
        }
    } else if ( ast->op == at::BinaryOpType::MUL ) {
        if ( std::holds_alternative<at::Stack>( ast->operand2 ) ) {
            auto reg = mk_reg( ast, "R11D" );

            auto mov1 = make_AT<at::Mov_>( ast );
            mov1->src = ast->operand2;
            mov1->dst = reg;
            current_instructions.push_back( mov1 );

            auto binary = make_AT<at::Binary_>( ast );
            binary->op = ast->op;
            binary->operand1 = ast->operand1;
            binary->operand2 = reg;
            current_instructions.push_back( binary );

            auto mov2 = make_AT<at::Mov_>( ast );
            mov2->src = reg;
            mov2->dst = ast->operand2;
            current_instructions.push_back( mov2 );
        } else {
            // Other Mul instructions
            current_instructions.push_back( ast );
        }
    } else {
        // Other Binary instructions
        current_instructions.push_back( ast );
    }
}
