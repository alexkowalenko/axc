//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/7/2025.
//

#include "x86_64CodeGen.h"

#include <print>

#include <spdlog/spdlog.h>

#include "common.h"
#include "exception.h"
#include "x86_at/includes.h"
#include "x86_common.h"

#include "assemblyGen.h"
#include "filterPseudo.h"
#include "fixInstructX86.h"
#include "printerX86.h"

std::string to_lower( const std::string& s ) {
    std::string buf = s;
    std::transform( s.begin(), s.end(), buf.begin(), []( unsigned char c ) { return std::tolower( c ); } );
    return buf;
}

std::string assemble_reg( x86_at::Register r ) {
    std::string name = to_lower( to_string( r->reg ) );
    switch ( r->reg ) {
    case x86_at::RegisterName::AX :
    case x86_at::RegisterName::CX :
    case x86_at::RegisterName::DX : {
        switch ( r->size ) {
        case x86_at::RegisterSize::Byte :
            return name + "l"; // AL, CL, DL
        case x86_at::RegisterSize::Long :
            return "e" + name + "x";
        case x86_at::RegisterSize::Qword :
            return "r" + name + "x";
        }
    }
    case x86_at::RegisterName::DI :
    case x86_at::RegisterName::SI : {
        switch ( r->size ) {
        case x86_at::RegisterSize::Byte :
            return name + "l"; // DIL, SIL
        case x86_at::RegisterSize::Long :
            return "e" + name; // EDI, ESI
        case x86_at::RegisterSize::Qword :
            return "r" + name; // RDI, RSI
        }
    }
    default :
        // Rx
        switch ( r->size ) {
        case x86_at::RegisterSize::Byte :
            return name + "l"; // R8L, R9L, R10L, R11L
        case x86_at::RegisterSize::Long :
            return name + "d"; // R8D, R9D, R10D, R11D
        case x86_at::RegisterSize::Qword :
            return name; // R8, R9, R10, R11
        }
    }
}

X86_64CodeGen::X86_64CodeGen( Option const& option ) : CodeGenerator( option ) {
    if ( option.system == System::Linux || option.system == System::FreeBSD ) {
        local_prefix = ".L";
    } else if ( option.system == System::MacOS ) {
        local_prefix = "L";
    }
}

CodeGenBase X86_64CodeGen::run_codegen( tac::Program tac ) {
    spdlog::info( "Run codegen," );
    AssemblyGen assembler( option );
    auto        assembly = assembler.generate( tac );
    PrinterX86  assemblerPrinter;
    auto        output = assemblerPrinter.print( assembly );
    std::println( "Assembly Output: {}", to_string( option.machine ) );
    std::println( "-----------------------" );
    std::println( "{:s}", output );

    spdlog::info( "Filtered 1: Filter Pseudo" );
    FilterPseudoX86 filter;
    filter.filter( assembly );
    output = assemblerPrinter.print( assembly );
    std::println( "Filtered 1:" );
    std::println( "----------" );
    std::println( "{:s}", output );

    spdlog::info( "Filtered 2: Fix Instructions" );
    FixInstructX86 filter2;
    filter2.filter( assembly );
    output = assemblerPrinter.print( assembly );
    std::println( "Filtered 2:" );
    std::println( "----------" );
    std::println( "{:s}", output );
    return std::static_pointer_cast<CodeGenBase_>( assembly );
}

void X86_64CodeGen::generate_output_file( const CodeGenBase assembly ) {
    spdlog::info( "Generate output file for {}.", to_string( option.machine ) );

    // Generate Assembly code
    generate( assembly );
    std::println( "{} Assembly:", to_string( option.machine ) );
    std::println( "---------------" );
    std::println( "{:s}", get_output() );
}

void X86_64CodeGen::generate( const CodeGenBase program ) {

    auto x86_program = std::dynamic_pointer_cast<x86_at::Program_>( program );
    if ( !x86_program ) {
        throw CodeException( Location {}, "Invalid program type for x86_64 code generation" );
    }
    make_output_file_name();

    file.open( output, std::ios::out );
    if ( !file.is_open() ) {
        throw CodeException( x86_program->location, "Cannot open file {}", output.string() );
    }

    x86_program->accept( this );

    if ( option.system == System::Linux || option.system == System::FreeBSD ) {
        add_line( "\t\t.section .note.GNU-stack,\"\",@progbits" );
    }
    file.close();
}

void X86_64CodeGen::visit_Program( const x86_at::Program ast ) {
    add_line( comment_prefix + "X86_64 " );
    add_line( std::format( "{}file: {}", comment_prefix, option.input_file ) );
    for ( auto const& function : ast->functions ) {
        function->accept( this );
        add_line( "" );
    }
}

void X86_64CodeGen::visit_FunctionDef( const x86_at::FunctionDef ast ) {
    std::string name = function_label( ast->name );
    current_function_name = ast->name;

    add_line( ".global", name, ast->location.line );
    add_line( std::format( "{}:", name ) );

    add_line( "pushq", "%rbp" );
    add_line( "movq", "%rsp, %rbp" );

    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( x86_at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( x86_at::Unary u ) -> void { u->accept( this ); },
                                 [ this ]( x86_at::Binary b ) -> void { b->accept( this ); },
                                 [ this ]( x86_at::Cmp b ) -> void { b->accept( this ); },
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
    add_line( "" );
}

std::string X86_64CodeGen::operand( const x86_at::Operand& op ) {
    return std::visit( overloaded { [ this ]( x86_at::Imm v ) -> std::string {
                                       v->accept( this );
                                       return last_string;
                                   },
                                    [ this ]( x86_at::Register r ) -> std::string {
                                        r->accept( this );
                                        return last_string;
                                    },
                                    [ this ]( x86_at::Pseudo p ) -> std::string {
                                        p->accept( this );
                                        return last_string;
                                    },
                                    [ this ]( x86_at::Stack s ) -> std::string {
                                        s->accept( this );
                                        return last_string;
                                    } },
                       op );
}

void X86_64CodeGen::visit_Mov( const x86_at::Mov ast ) {
    add_line( "movl", operand( ast->src ), operand( ast->dst ) );
}

void X86_64CodeGen::visit_Ret( const x86_at::Ret ast ) {
    add_line( "movq", "%rbp, %rsp", ast->location.line );
    add_line( "popq", "%rbp" );
    add_line( "ret", "" );
}

void X86_64CodeGen::visit_Unary( const x86_at::Unary ast ) {
    switch ( ast->op ) {
    case x86_at::UnaryOpType::NEG :
        add_line( "negl", operand( ast->operand ) );
        break;
    case x86_at::UnaryOpType::NOT :
        add_line( "notl", operand( ast->operand ) );
        break;
    default :
        throw CodeException( ast->location, "Unsupported unary operator" );
    }
}

void X86_64CodeGen::visit_AllocateStack( const x86_at::AllocateStack ast ) {
    add_line( "subq", std::format( "${}, %rsp", ast->size ) );
}

void X86_64CodeGen::visit_DeallocateStack( const x86_at::DeallocateStack ast ) {
    add_line( "addq", std::format( "${}, %rsp", ast->size ) );
}

void X86_64CodeGen::visit_Push( const x86_at::Push ast ) {
    if ( std::holds_alternative<x86_at::Imm>( ast->operand ) ) {
        // If the operand is an immediate, then need to use pushq (64-bit immediate)
        add_line( "pushq", operand( ast->operand ) );
        return;
    }
    if ( std::holds_alternative<x86_at::Register>( ast->operand ) ) {
        // If the operand is an register then match the register size to pushq
        auto reg = std::get<x86_at::Register>( ast->operand );
        add_line( "pushq", operand( mk_node<x86_at::Register_>( ast, reg->reg, x86_at::RegisterSize::Qword ) ) );
        return;
    }
    add_line( "pushl", operand( ast->operand ) );
}

void X86_64CodeGen::visit_Call( const x86_at::Call ast ) {
    add_line( "call", function_label( ast->function_name ) );
}

void X86_64CodeGen::visit_Binary( const x86_at::Binary ast ) {
    switch ( ast->op ) {
    case x86_at::BinaryOpType::ADD :
        add_line( "addl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case x86_at::BinaryOpType::SUB :
        add_line( "subl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case x86_at::BinaryOpType::MUL :
        add_line( "imull", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case x86_at::BinaryOpType::AND :
        add_line( "andl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case x86_at::BinaryOpType::OR :
        add_line( "orl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case x86_at::BinaryOpType::XOR :
        add_line( "xorl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case x86_at::BinaryOpType::SHL :
        add_line( "shll", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    case x86_at::BinaryOpType::SHR :
        add_line( "sarl", operand( ast->operand1 ), operand( ast->operand2 ) );
        break;
    default :
    }
}

void X86_64CodeGen::visit_Idiv( const x86_at::Idiv ast ) {
    add_line( "idivl", operand( ast->src ) );
}

void X86_64CodeGen::visit_Cmp( const x86_at::Cmp ast ) {
    add_line( "cmpl", operand( ast->operand1 ), operand( ast->operand2 ) );
}

void X86_64CodeGen::visit_Jump( const x86_at::Jump ast ) {
    add_line( "jmp", jump_label( ast->target ) );
}

void X86_64CodeGen::visit_JumpCC( const x86_at::JumpCC ast ) {
    add_line( std::format( "j{}", cond_code( ast->cond ) ), jump_label( ast->target ) );
}

void X86_64CodeGen::visit_SetCC( const x86_at::SetCC ast ) {
    add_line( std::format( "set{}", cond_code( ast->cond ) ), operand( ast->operand ) );
}

void X86_64CodeGen::visit_Label( const x86_at::Label ast ) {
    add_line( jump_label( ast->name ) + ":" );
}

void X86_64CodeGen::visit_Cdq( const x86_at::Cdq ast ) {
    add_line( "cdq", "" );
}

void X86_64CodeGen::visit_Imm( const x86_at::Imm ast ) {
    last_string = std::format( "${}", ast->value );
}

void X86_64CodeGen::visit_Register( const x86_at::Register ast ) {
    last_string = std::format( "%{}", assemble_reg( ast ) );
}

void X86_64CodeGen::visit_Pseudo( const x86_at::Pseudo ast ) {
    throw CodeException( ast->location, "Pseudo should not be in final code generation" );
}

void X86_64CodeGen::visit_Stack( const x86_at::Stack ast ) {
    last_string = std::format( "{}(%rbp)", ast->offset );
}

std::string X86_64CodeGen::function_label( const std::string_view name ) const {
    std::string n { name };
    if ( option.system == System::MacOS ) {
        n = "_" + n;
    }
    return n;
}

std::string X86_64CodeGen::jump_label( std::string_view name ) {
    return std::format( "{}{}.{}", local_prefix, current_function_name, name );
}
