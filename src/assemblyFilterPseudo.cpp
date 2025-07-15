//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#include "assemblyFilterPseudo.h"

#include "at/includes.h"
#include "common.h"

void AssemblyFilterPseudo::filter( at::Program program ) {
    program->accept( this );
}

void AssemblyFilterPseudo::visit_Program( const at::Program ast ) {
    ast->function->accept( this );
}

void AssemblyFilterPseudo::visit_FunctionDef( const at::FunctionDef ast ) {
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( at::Unary u ) -> void { return u->accept( this ); },
                                 [ this ]( at::AllocateStack a ) -> void { return a->accept( this ); },
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
        auto s = std::make_shared<at::Stack_>( p->location );
        s->offset = location;
        return s;
    } else {
        return op;
    }
}

int AssemblyFilterPseudo::get_number_stack_locations() {
    return std::abs( next_stack_location / stack_increment );
}
