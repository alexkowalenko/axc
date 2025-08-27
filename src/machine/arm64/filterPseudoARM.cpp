//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/8/2025.
//

#include "filterPseudoARM.h"

#include <spdlog/spdlog.h>

#include "arm64_at/includes.h"
#include "common.h"

void FilterPseudoARM::filter( arm64_at::Program program ) {
    program->accept( this );
}

void FilterPseudoARM::visit_Program( const arm64_at::Program ast ) {
    reset_stack_info();
    ast->function->accept( this );
}

void FilterPseudoARM::visit_FunctionDef( const arm64_at::FunctionDef ast ) {
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( arm64_at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( arm64_at::Load l ) -> void { l->accept( this ); },
                                 [ this ]( arm64_at::Store s ) -> void { s->accept( this ); },
                                 [ this ]( arm64_at::Unary u ) -> void { u->accept( this ); },
                                 [ this ]( arm64_at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( arm64_at::Ret r ) -> void { r->accept( this ); },
                                 [ this ]( arm64_at::AllocateStack a ) -> void { a->accept( this ); },
                                 [ this ]( arm64_at::DeallocateStack d ) -> void { d->accept( this ); },
                                 [ this ]( arm64_at::Branch ) -> void {}, [ this ]( arm64_at::BranchCC ) -> void {},
                                 [ this ]( arm64_at::Label ) -> void {},
                                 [ this ]( arm64_at::Cmp c ) -> void { c->accept( this ); },
                                 [ this ]( arm64_at::Cset c ) -> void { c->accept( this ); } },
                    instr );
    }
    ast->stack_size = get_number_stack_locations();
    spdlog::debug( "Function {} has {} stack locations", ast->name, get_number_stack_locations() );
}

void FilterPseudoARM::visit_Mov( const arm64_at::Mov ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

void FilterPseudoARM::visit_Load( arm64_at::Load ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

void FilterPseudoARM::visit_Store( arm64_at::Store ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

void FilterPseudoARM::visit_Unary( const arm64_at::Unary ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

void FilterPseudoARM::visit_Binary( arm64_at::Binary ast ) {
    ast->src1 = operand( ast->src1 );
    ast->src2 = operand( ast->src2 );
    ast->dst = operand( ast->dst );
}

void FilterPseudoARM::visit_Cmp( arm64_at::Cmp ast ) {
    ast->operand1 = operand( ast->operand1 );
    ast->operand2 = operand( ast->operand2 );
}

void FilterPseudoARM::visit_Cset( arm64_at::Cset ast ) {
    ast->operand = operand( ast->operand );
}

arm64_at::Operand FilterPseudoARM::operand( const arm64_at::Operand& op ) {
    if ( std::holds_alternative<arm64_at::Pseudo>( op ) ) {
        auto p = std::get<arm64_at::Pseudo>( op );
        int  location = 0;
        if ( stack_location_map.contains( p->name ) ) {
            location = stack_location_map[ p->name ];
        } else {
            next_stack_location += stack_increment;
            location = next_stack_location;
            stack_location_map[ p->name ] = location;
        }
        return mk_node<arm64_at::Stack_>( p, location );
    } else {
        return op;
    }
}

int FilterPseudoARM::get_number_stack_locations() const {
    return std::abs( next_stack_location / stack_increment );
}

void FilterPseudoARM::reset_stack_info() {
    stack_location_map.clear();
    next_stack_location = 0;
}