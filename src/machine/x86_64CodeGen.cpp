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
#include "../common.h"
#include "x86_common.h"

std::string to_lower( const std::string& s ) {
    std::string buf = s;
    std::transform( s.begin(), s.end(), buf.begin(), []( unsigned char c ) { return std::tolower( c ); } );
    return buf;
}

std::string assemble_reg( at::Register r ) {
    std::string name = to_lower( to_string( r->reg ) );
    if ( r->reg == at::RegisterName::AX || r->reg == at::RegisterName::CX || r->reg == at::RegisterName::DX ) {
        if ( r->size == at::RegisterSize::Long ) {
            return "e" + name + "x";
        }
        return name + "l";
    }
    // RX
    if ( r->size == at::RegisterSize::Long ) {
        return name + "d";
    }
    return name + "b";
}

X86_64CodeGen::X86_64CodeGen( Option const& option ) : CodeGenerator( option ) {
    if ( option.system == System::Linux || option.system == System::FreeBSD ) {
        local_prefix = ".L";
    } else if ( option.system == System::MacOS ) {
        local_prefix = "L";
    }
}

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
                                 [ this ]( at::Cmp b ) -> void { b->accept( this ); },
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
    add_line( "movl", operand( ast->src ), operand( ast->dst ) );
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

void X86_64CodeGen::visit_Binary( const at::Binary ast ) {
    switch ( ast->op ) {
    case at::BinaryOpType::ADD :
        add_line( "addl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case at::BinaryOpType::SUB :
        add_line( "subl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case at::BinaryOpType::MUL :
        add_line( "imull", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case at::BinaryOpType::AND :
        add_line( "andl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case at::BinaryOpType::OR :
        add_line( "orl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case at::BinaryOpType::XOR :
        add_line( "xorl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case at::BinaryOpType::SHL :
        add_line( "shll", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case at::BinaryOpType::SHR :
        add_line( "sarl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    default :
    }
}

void X86_64CodeGen::visit_Idiv( const at::Idiv ast ) {
    add_line( "idivl", operand( ast->src ) );
}

void X86_64CodeGen::visit_Cmp( const at::Cmp ast ) {
    add_line( "cmpl", operand( ast->operand1 ), operand( ast->operand2 ) );
}

void X86_64CodeGen::visit_Jump( const at::Jump ast ) {
    add_line( "jmp", local_prefix + ast->target );
}

void X86_64CodeGen::visit_JumpCC( const at::JumpCC ast ) {
    add_line( std::format( "j{}", cond_code( ast->cond ) ), local_prefix + ast->target );
}

void X86_64CodeGen::visit_SetCC( const at::SetCC ast ) {
    add_line( std::format( "set{}", cond_code( ast->cond ) ), operand( ast->operand ) );
}

void X86_64CodeGen::visit_Label( const at::Label ast ) {
    add_line( local_prefix + ast->name + ":" );
}

void X86_64CodeGen::visit_Cdq( const at::Cdq ast ) {
    add_line( "cdq", "" );
}

void X86_64CodeGen::visit_Imm( const at::Imm ast ) {
    last_string = std::format( "${}", ast->value );
}

void X86_64CodeGen::visit_Register( const at::Register ast ) {
    last_string = std::format( "%{}", assemble_reg( ast ) );
}

void X86_64CodeGen::visit_Pseudo( const at::Pseudo ast ) {
    throw CodeException( ast->location, "Pseudo should not be in final code generation" );
}

void X86_64CodeGen::visit_Stack( const at::Stack ast ) {
    last_string = std::format( "{}(%rbp)", ast->offset );
}