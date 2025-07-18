//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#include "assemblyFilterPseudo.h"

#include "../at/includes.h"
#include "../common.h"

void AssemblyFilterPseudo::filter( at::Program program ) {
    program->accept( this );
}

void AssemblyFilterPseudo::visit_Program( const at::Program ast ) {
    ast->function->accept( this );
}

void AssemblyFilterPseudo::visit_FunctionDef( const at::FunctionDef ast ) {
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( at::Unary u ) -> void { u->accept( this ); },
                                 [ this ]( at::Cmp b ) -> void { b->accept( this ); },
                                 [ this ]( at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( at::AllocateStack a ) -> void { a->accept( this ); },
                                 [ this ]( at::Idiv i ) -> void { i->accept( this ); },
                                 [ this ]( at::Cdq c ) -> void { c->accept( this ); },
                                 [ this ]( at::Jump c ) -> void { c->accept( this ); },
                                 [ this ]( at::JumpCC c ) -> void { c->accept( this ); },
                                 [ this ]( at::SetCC c ) -> void { c->accept( this ); },
                                 [ this ]( at::Label c ) -> void { c->accept( this ); },
                                 [ this ]( at::Ret r ) -> void { r->accept( this ); } },
                    instr );
    }
}

void AssemblyFilterPseudo::visit_Mov( const at::Mov ast ) {
    ast->src = operand( ast->src );
    ast->dst = operand( ast->dst );
}

void AssemblyFilterPseudo::visit_Unary( const at::Unary ast ) {
    ast->operand = operand( ast->operand );
}

void AssemblyFilterPseudo::visit_Binary( const at::Binary ast ) {
    ast->operand1 = operand( ast->operand1 );
    ast->operand2 = operand( ast->operand2 );
}

void AssemblyFilterPseudo::visit_Idiv( const at::Idiv ast ) {
    ast->src = operand( ast->src );
}

void AssemblyFilterPseudo::visit_Cmp( const at::Cmp ast ) {
    ast->operand1 = operand( ast->operand1 );
    ast->operand2 = operand( ast->operand2 );
}

void AssemblyFilterPseudo::visit_SetCC( const at::SetCC ast ) {
    ast->operand = operand( ast->operand );
}

at::Operand AssemblyFilterPseudo::operand( const at::Operand& op ) {
    if ( std::holds_alternative<at::Pseudo>( op ) ) {
        auto p = std::get<at::Pseudo>( op );
        int  location = 0;
        if ( stack_location_map.contains( p->name ) ) {
            location = stack_location_map[ p->name ];
        } else {
            next_stack_location += stack_increment;
            location = next_stack_location;
            stack_location_map[ p->name ] = location;
        }
        return mk_node<at::Stack_>( p, location );
    } else {
        return op;
    }
}

int AssemblyFilterPseudo::get_number_stack_locations() {
    return std::abs( next_stack_location / stack_increment );
}
