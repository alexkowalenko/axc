//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#include "filterPseudo.h"

#include <spdlog/spdlog.h>

#include "common.h"
#include "x86_at/includes.h"

void FilterPseudoX86::filter( x86_at::Program program ) {
    program->accept( this );
}

void FilterPseudoX86::visit_Program( const x86_at::Program ast ) {
    for ( auto const& funct : ast->functions ) {
        funct->accept( this );
    }
}

void FilterPseudoX86::visit_FunctionDef( const x86_at::FunctionDef ast ) {
    reset_stack_info();
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( x86_at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( x86_at::Unary u ) -> void { u->accept( this ); },
                                 [ this ]( x86_at::Cmp b ) -> void { b->accept( this ); },
                                 [ this ]( x86_at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( x86_at::AllocateStack a ) -> void { a->accept( this ); },
                                 [ this ]( x86_at::DeallocateStack a ) -> void { a->accept( this ); },
                                 [ this ]( x86_at::Push p ) -> void { p->accept( this ); },
                                 [ this ]( x86_at::Call c ) -> void { c->accept( this ); },
                                 [ this ]( x86_at::Idiv i ) -> void { i->accept( this ); },
                                 [ this ]( x86_at::Cdq c ) -> void { c->accept( this ); },
                                 [ this ]( x86_at::Jump c ) -> void { c->accept( this ); },
                                 [ this ]( x86_at::JumpCC c ) -> void { c->accept( this ); },
                                 [ this ]( x86_at::SetCC c ) -> void { c->accept( this ); },
                                 [ this ]( x86_at::Label c ) -> void { c->accept( this ); },
                                 [ this ]( x86_at::Ret r ) -> void { r->accept( this ); } },
                    instr );
    }
    ast->stack_size = get_number_stack_locations();
    spdlog::debug( "Function {} has {} stack locations", ast->name, ast->stack_size );
}

void FilterPseudoX86::visit_Mov( const x86_at::Mov ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

void FilterPseudoX86::visit_Unary( const x86_at::Unary ast ) {
    ast->operand = operand( ast->operand );
}

void FilterPseudoX86::visit_Binary( const x86_at::Binary ast ) {
    ast->operand1 = operand( ast->operand1 );
    ast->operand2 = operand( ast->operand2 );
}

void FilterPseudoX86::visit_Idiv( const x86_at::Idiv ast ) {
    ast->src = operand( ast->src );
}

void FilterPseudoX86::visit_Cmp( const x86_at::Cmp ast ) {
    ast->operand1 = operand( ast->operand1 );
    ast->operand2 = operand( ast->operand2 );
}

void FilterPseudoX86::visit_SetCC( const x86_at::SetCC ast ) {
    ast->operand = operand( ast->operand );
}

void FilterPseudoX86::visit_Push( const x86_at::Push ast ) {
    ast->operand = operand( ast->operand );
}

x86_at::Operand FilterPseudoX86::operand( const x86_at::Operand& op ) {
    if ( std::holds_alternative<x86_at::Pseudo>( op ) ) {
        auto p = std::get<x86_at::Pseudo>( op );
        int  location = 0;
        if ( stack_location_map.contains( p->name ) ) {
            location = stack_location_map[ p->name ];
        } else {
            next_stack_location += stack_increment;
            location = next_stack_location;
            stack_location_map[ p->name ] = location;
        }
        return mk_node<x86_at::Stack_>( p, location );
    } else {
        return op;
    }
}

int FilterPseudoX86::get_number_stack_locations() const {
    return std::abs( next_stack_location / stack_increment );
}

void FilterPseudoX86::reset_stack_info() {
    stack_location_map.clear();
    next_stack_location = 0;
}
