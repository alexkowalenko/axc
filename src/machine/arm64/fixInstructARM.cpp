//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 14/8/2025.
//

#include "fixInstructARM.h"

#include "arm64_at/includes.h"
#include "common.h"

FixInstructARM::FixInstructARM() {
    x9 = std::make_shared<arm64_at::Register_>( Location(), arm64_at::RegisterName::X9 );
    x10 = std::make_shared<arm64_at::Register_>( Location(), arm64_at::RegisterName::X10 );
}

void FixInstructARM::filter( arm64_at::Program program ) {
    program->accept( this );
}

void FixInstructARM::visit_Program( arm64_at::Program ast ) {
    ast->function->accept( this );
}

void FixInstructARM::visit_FunctionDef( arm64_at::FunctionDef ast ) {
    current_instructions.clear();

    if ( ast->stack_size != 0 ) {
        auto allocate = mk_node<arm64_at::AllocateStack_>( ast, ast->stack_size );
        current_instructions.emplace_back( allocate );
    }

    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( arm64_at::Mov v ) -> void { return v->accept( this ); },
                                 [ this ]( arm64_at::Load l ) -> void { return l->accept( this ); },
                                 [ this ]( arm64_at::Store s ) -> void { return s->accept( this ); },
                                 [ this ]( arm64_at::Ret r ) -> void { return r->accept( this ); },
                                 [ this ]( arm64_at::Unary u ) -> void { return u->accept( this ); },
                                 [ this ]( arm64_at::AllocateStack a ) -> void { return a->accept( this ); },
                                 [ this ]( arm64_at::DeallocateStack d ) -> void { return d->accept( this ); } },
                    instr );
    }
    ast->instructions = current_instructions;
}

void FixInstructARM::visit_Mov( arm64_at::Mov ast ) {
    if ( std::holds_alternative<arm64_at::Stack>( ast->src ) ) {
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->src, ast->dst ) );
        return;
    }
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Load( arm64_at::Load ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Store( arm64_at::Store ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Unary( arm64_at::Unary ast ) {
    arm64_at::Operand src;
    arm64_at::Operand dst;

    if ( std::holds_alternative<arm64_at::Stack>( ast->src ) ) {
        // If stack load into register
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->src, x9 ) );
        src = x9;
    } else if ( std::holds_alternative<arm64_at::Imm>( ast->src ) ) {
        // If immediate value, move into register
        current_instructions.emplace_back( mk_node<arm64_at::Mov_>( ast, ast->src, x9 ) );
        src = x9;
    } else {
        src = ast->src;
    }

    // If destination stack store into register
    if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {
        dst = x10;
    }

    // Perform the operation with modified src, dst
    current_instructions.emplace_back( mk_node<arm64_at::Unary_>( ast, ast->op, dst, src ) );

    // If destination stack store back to stack
    if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {

        current_instructions.emplace_back( mk_node<arm64_at::Store_>( ast, x10, ast->dst ) );
    }
}

void FixInstructARM::visit_AllocateStack( arm64_at::AllocateStack ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_DeallocateStack( arm64_at::DeallocateStack ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Ret( arm64_at::Ret ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Imm( arm64_at::Imm ast ) {}
void FixInstructARM::visit_Register( arm64_at::Register ast ) {}
void FixInstructARM::visit_Pseudo( arm64_at::Pseudo ast ) {}
void FixInstructARM::visit_Stack( arm64_at::Stack ast ) {}
