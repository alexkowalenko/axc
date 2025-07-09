//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/7/2025.
//

#include "x86_64CodeGen.h"

#include "../at/includes.h"
#include "../exception.h"

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};

void X86_64CodeGen::generate( at::Program program ) {
    make_output_file_name();

    file.open( output, std::ios::out );
    if ( !file.is_open() ) {
        throw CodeException( program->location, "Cannot open file {}", output.string() );
    }

    program->accept( this );

    if ( option.system == System::Linux ) {
        add_line( "\t\t.section .note.GNU-stack,\"\",@progbits" );
    }
    file.close();
}

void X86_64CodeGen::visit_Program( const at::Program& ast ) {
    add_line( std::format( "# file: {}", option.input_file ) );
    ast->function->accept( this );
}

void X86_64CodeGen::visit_FunctionDef( const at::FunctionDef& ast ) {
    std::string name = ast->name;
    if ( option.system == System::MacOS ) {
        name = "_" + name;
    }

    add_line( std::format( "\t.global {}", name ) );
    add_line( std::format( "{}:", name ) );
    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( at::Ret r ) -> void { r->accept( this ); } },
                    instr );
    }
    add_line( "" );
}

std::string X86_64CodeGen::operand( const at::Operand& op ) {
    return std::visit( overloaded { [ this ]( at::Imm v ) -> std::string {
                                       v->accept( this );
                                       return last_string;
                                   },
                                    [ this ]( at::Register r ) -> std::string {
                                        r->accept( this );
                                        return last_string;
                                    } },
                       op );
}

void X86_64CodeGen::visit_Mov( const at::Mov& ast ) {
    std::string buf = std::format( "\tmovl\t{}, {}", operand( ast->src ), operand( ast->dst ) );
    add_line( buf );
}

void X86_64CodeGen::visit_Imm( const at::Imm& ast ) {
    last_string = std::format( "${}", ast->value );
}

void X86_64CodeGen::visit_Register( const at::Register& ast ) {
    last_string = std::format( "%{}", ast->reg );
}

void X86_64CodeGen::visit_Ret( const at::Ret& ast ) {
    add_line( "\tret" );
}