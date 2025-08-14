//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 14/8/2025.
//

#include "fixInstruct.h"

#include "arm64_at/includes.h"
#include "common.h"

FixInstruct::FixInstruct() {
    x9 = std::make_shared<arm64_at::Register_>( Location(), arm64_at::RegisterName::X9 );
    x10 = std::make_shared<arm64_at::Register_>( Location(), arm64_at::RegisterName::X10 );
}

void FixInstruct::filter( arm64_at::Program program ) {
    program->accept( this );
}

void FixInstruct::visit_Program( arm64_at::Program ast ) {
    ast->function->accept( this );
}

void FixInstruct::visit_FunctionDef( arm64_at::FunctionDef ast ) {
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

void FixInstruct::visit_Mov( arm64_at::Mov ast ) {
    if ( std::holds_alternative<arm64_at::Stack>( ast->src ) ) {
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->src, ast->dst ) );
        return;
    }
    current_instructions.emplace_back( ast );
}

void FixInstruct::visit_Load( arm64_at::Load ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstruct::visit_Store( arm64_at::Store ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstruct::visit_Unary( arm64_at::Unary ast ) {

    if ( ast->op == arm64_at::UnaryOpType::NEG ) {
        if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {
            current_instructions.emplace_back( mk_node<arm64_at::Mov_>( ast, ast->src, x10 ) );
            current_instructions.emplace_back( mk_node<arm64_at::Unary_>( ast, arm64_at::UnaryOpType::NEG, x9, x10 ) );
            current_instructions.emplace_back( mk_node<arm64_at::Store_>( ast, x9, ast->dst ) );
            return;
        }
    }
    current_instructions.emplace_back( ast );
}

void FixInstruct::visit_AllocateStack( arm64_at::AllocateStack ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstruct::visit_DeallocateStack( arm64_at::DeallocateStack ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstruct::visit_Ret( arm64_at::Ret ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstruct::visit_Imm( arm64_at::Imm ast ) {}
void FixInstruct::visit_Register( arm64_at::Register ast ) {}
void FixInstruct::visit_Pseudo( arm64_at::Pseudo ast ) {}
void FixInstruct::visit_Stack( arm64_at::Stack ast ) {}
