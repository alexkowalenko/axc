//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#include "arm64CodeGen.h"

#include <print>

#include <spdlog/spdlog.h>

#include "arm64_at/includes.h"
#include "armAssemblyGen.h"
#include "common.h"
#include "exception.h"
#include "filterPseudo.h"
#include "printerARM64.h"

Arm64CodeGen::Arm64CodeGen( Option const& option ) : CodeGenerator( option ) {
    comment_prefix = "// ";
}

CodeGenBase Arm64CodeGen::run_codegen( tac::Program tac ) {
    spdlog::info( "Run codegen," );
    ARMAssemblyGen assembler;
    auto           assembly = assembler.generate( tac );
    PrinterARM64   assemblerPrinter;
    auto           output = assemblerPrinter.print( assembly );
    std::println( "Assembly Output: {}", to_string( option.machine ) );
    std::println( "------------------------" );
    std::println( "{:s}", output );

    spdlog::info( "Filtered 1: Filter Pseudo" );
    FilterPseudo filter;
    filter.filter( assembly );
    output = assemblerPrinter.print( assembly );
    std::println( "Filtered 1:" );
    std::println( "----------" );
    std::println( "{:s}", output );

    return std::static_pointer_cast<CodeGenBase_>( assembly );
}

void Arm64CodeGen::generate_output_file( CodeGenBase assembly ) {
    spdlog::info( "Generate output file for {}.", to_string( option.machine ) );
    // Generate Assembly code
    generate( assembly );
    std::println( "{} Assembly:", to_string( option.machine ) );
    std::println( "---------------" );
    std::println( "{:s}", get_output() );
}

void Arm64CodeGen::generate( CodeGenBase program ) {
    auto arm64_program = std::dynamic_pointer_cast<arm64_at::Program_>( program );
    if ( !arm64_program ) {
        throw CodeException( Location {}, "Invalid program type for ARM64 code generation" );
    }
    make_output_file_name();

    file.open( output, std::ios::out );
    if ( !file.is_open() ) {
        throw CodeException( arm64_program->location, "Cannot open file {}", output.string() );
    }

    arm64_program->accept( this );
}

void Arm64CodeGen::visit_Program( const arm64_at::Program ast ) {
    add_line( std::format( "{} file: {}", comment_prefix, option.input_file ) );

    add_line( "\t.text" );

    ast->function->accept( this );

    file.close();
}

void Arm64CodeGen::visit_FunctionDef( const arm64_at::FunctionDef ast ) {
    std::string name = ast->name;
    if ( option.system == System::MacOS ) {
        name = "_" + name;
    }

    add_line( ".global", name, ast->location.line );

    add_line( "\t.align 2" );
    add_line( std::format( "{}:", name ) );

    for ( auto const& instr : ast->instructions ) {

        std::visit( overloaded { [ this ]( arm64_at::Mov v ) -> void { v->accept( this ); },
                                 [ this ]( arm64_at::Ret r ) -> void { r->accept( this ); },
                                 [ this ]( arm64_at::Unary u ) -> void { u->accept( this ); } },
                    instr );
    }
}

void Arm64CodeGen::visit_Mov( const arm64_at::Mov ast ) {
    add_line( "mov", operand( ast->dst ), operand( ast->src ) );
}

void Arm64CodeGen::visit_Ret( const arm64_at::Ret ast ) {
    add_line( "ret", "" );
}

void Arm64CodeGen::visit_Unary( const arm64_at::Unary ast ) {
    switch ( ast->op ) {
    case arm64_at::UnaryOpType::NEG :
        add_line( "negs", operand( ast->dst ), operand( ast->src ) );
        break;
    case arm64_at::UnaryOpType::NOT :
        add_line( "notl", operand( ast->dst ) );
        break;
    default :
        throw CodeException( ast->location, "Unsupported unary operator" );
    }
}

std::string Arm64CodeGen::operand( const arm64_at::Operand& op ) {
    return std::visit( overloaded { [ this ]( arm64_at::Imm v ) -> std::string {
                                       v->accept( this );
                                       return last_string;
                                   },
                                    [ this ]( arm64_at::Register r ) -> std::string {
                                        r->accept( this );
                                        return last_string;
                                    },
                                    [ this ]( arm64_at::Pseudo p ) -> std::string {
                                        throw CodeException( p->location, "Pseudo variable at final code generation" );
                                    },
                                    [ this ]( arm64_at::Stack s ) -> std::string {
                                        s->accept( this );
                                        return last_string;
                                    } },
                       op );
}

void Arm64CodeGen::visit_Imm( const arm64_at::Imm ast ) {
    last_string = std::format( "#{}", ast->value );
}

void Arm64CodeGen::visit_Register( const arm64_at::Register ast ) {
    last_string = std::format( "{}", to_string( ast->reg ) );
}

void Arm64CodeGen::visit_Stack( const arm64_at::Stack ast ) {}
