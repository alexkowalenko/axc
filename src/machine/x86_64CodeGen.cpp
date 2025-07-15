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

void X86_64CodeGen::visit_Program( const at::Program ast ) {
    add_line( std::format( "# file: {}", option.input_file ) );
    ast->function->accept( this );
}

void X86_64CodeGen::visit_FunctionDef( const at::FunctionDef ast ) {
    std::string name = ast->name;
    if ( option.system == System::MacOS ) {
        name = "_" + name;
    }

    add_line( ".global", name, ast->location.line );
    add_line( std::format( "{}:", name ) );

    add_line( "pushq", "%rbp" );
    add_line( "movq", "%rsp, %rbp" );

    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( at::Unary u ) -> void { u->accept( this ); },
                                 [ this ]( at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( at::AllocateStack a ) -> void { a->accept( this ); },
                                 [ this ]( at::Idiv i ) -> void { i->accept( this ); },
                                 [ this ]( at::Cdq c ) -> void { c->accept( this ); },
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
                                    },
                                    [ this ]( at::Pseudo p ) -> std::string {
                                        p->accept( this );
                                        return last_string;
                                    },
                                    [ this ]( at::Stack s ) -> std::string {
                                        s->accept( this );
                                        return last_string;
                                    } },
                       op );
}

void X86_64CodeGen::visit_Mov( const at::Mov ast ) {
    std::string buf = std::format( "{}, {}", operand( ast->src ), operand( ast->dst ) );
    add_line( "movl", buf );
}

void X86_64CodeGen::visit_Ret( const at::Ret ast ) {
    add_line( "movq", "%rbp, %rsp", ast->location.line );
    add_line( "popq", "%rbp" );
    add_line( "ret", "" );
}

void X86_64CodeGen::visit_Unary( const at::Unary ast ) {
    switch ( ast->op ) {
    case at::UnaryOpType::NEG :
        add_line( "negl", operand( ast->operand ) );
        break;
    case at::UnaryOpType::NOT :
        add_line( "notl", operand( ast->operand ) );
        break;
    default :
        throw CodeException( ast->location, "Unsupported unary operator" );
    }
}

void X86_64CodeGen::visit_AllocateStack( const at::AllocateStack ast ) {
    add_line( "subq", std::format( "${}, %rsp", ast->size ) );
}

void X86_64CodeGen::visit_Binary( const at::Binary ast ) {}
void X86_64CodeGen::visit_Idiv( const at::Idiv ast ) {}
void X86_64CodeGen::visit_Cdq( const at::Cdq ast ) {}

void X86_64CodeGen::visit_Imm( const at::Imm ast ) {
    last_string = std::format( "${}", ast->value );
}

void X86_64CodeGen::visit_Register( const at::Register ast ) {
    last_string = std::format( "%{}", ast->reg );
}

void X86_64CodeGen::visit_Pseudo( const at::Pseudo ast ) {
    throw CodeException( ast->location, "Pseudo should not be in final code generation" );
}

void X86_64CodeGen::visit_Stack( const at::Stack ast ) {
    last_string = std::format( "{}(%rbp)", ast->offset );
}