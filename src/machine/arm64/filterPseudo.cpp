//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/8/2025.
//

#include "filterPseudo.h"

#include <spdlog/spdlog.h>

#include "arm64_at/includes.h"
#include "common.h"

void FilterPseudo::filter( arm64_at::Program program ) {
    program->accept( this );
}

void FilterPseudo::visit_Program( const arm64_at::Program ast ) {
    reset_stack_info();
    ast->function->accept( this );
}

void FilterPseudo::visit_FunctionDef( const arm64_at::FunctionDef ast ) {
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( arm64_at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( arm64_at::Unary u ) -> void { u->accept( this ); },
                                 [ this ]( arm64_at::Ret r ) -> void { r->accept( this ); } },
                    instr );
    }
    // ast->stack_size = get_number_stack_locations();
    spdlog::debug( "Function {} has {} stack locations", ast->name, get_number_stack_locations() );
}

void FilterPseudo::visit_Mov( const arm64_at::Mov ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

void FilterPseudo::visit_Unary( const arm64_at::Unary ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

arm64_at::Operand FilterPseudo::operand( const arm64_at::Operand& op ) {
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

int FilterPseudo::get_number_stack_locations() const {
    return std::abs( next_stack_location / stack_increment );
}

void FilterPseudo::reset_stack_info() {
    stack_location_map.clear();
    next_stack_location = 0;
}