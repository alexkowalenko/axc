//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#include "printerARM64.h"

#include "arm64_at/includes.h"
#include "common.h"

std::string PrinterARM64::print( const arm64_at::Program ast ) {
    return ast->accept( this );
}

std::string PrinterARM64::visit_Program( const arm64_at::Program ast ) {
    return ast->function->accept( this );
}

std::string PrinterARM64::visit_FunctionDef( const arm64_at::FunctionDef ast ) {
    std::string buf = std::format( "Function({})\n", ast->name );
    for ( auto const& instr : ast->instructions ) {
        buf += indent;
        buf += std::visit( overloaded { [ this ]( arm64_at::Mov v ) -> std::string { return v->accept( this ); },
                                        [ this ]( arm64_at::Ret r ) -> std::string { return r->accept( this ); } },
                           instr );
        buf += "\n";
    }
    return buf;
}
std::string PrinterARM64::visit_Mov( const arm64_at::Mov ast ) {
    return std::format( "mov({}, {})", operand( ast->src ), operand( ast->dst ) );
}

std::string PrinterARM64::visit_Ret( const arm64_at::Ret ast ) {
    return "ret";
}

std::string PrinterARM64::operand( const arm64_at::Operand& op ) {
    return std::visit( overloaded {
                           [ this ]( arm64_at::Imm v ) -> std::string { return v->accept( this ); },
                           [ this ]( arm64_at::Register r ) -> std::string { return r->accept( this ); },
                       },
                       op );
}

std::string PrinterARM64::visit_Imm( const arm64_at::Imm ast ) {
    return std::format( "#{}", ast->value );
}

std::string PrinterARM64::visit_Register( const arm64_at::Register ast ) {
    return to_string( ast->reg );
}
