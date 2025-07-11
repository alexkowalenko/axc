//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "printerAT.h"

#include "at/includes.h"
#include "common.h"

std::string PrinterAT::print( const at::Program& ast ) {
    return ast->accept( this );
}

std::string PrinterAT::visit_Program( const at::Program& ast ) {
    return ast->function->accept( this );
};

std::string PrinterAT::visit_FunctionDef( const at::FunctionDef& ast ) {
    std::string buf = std::format( "Function({})\n", ast->name );
    for ( auto const& instr : ast->instructions ) {
        buf += indent;
        buf += std::visit( overloaded { [ this ]( at::Mov v ) -> std::string { return v->accept( this ); },
                                        [ this ]( at::Ret r ) -> std::string { return r->accept( this ); } },
                           instr );
        buf += "\n";
    }
    return buf;
};

std::string PrinterAT::operand( const at::Operand& op ) {
    return std::visit( overloaded { [ this ]( at::Imm v ) -> std::string { return v->accept( this ); },
                                    [ this ]( at::Register r ) -> std::string { return r->accept( this ); } },
                       op );
}

std::string PrinterAT::visit_Mov( const at::Mov& ast ) {
    return std::format( "MOV({}, {})", operand( ast->src ), operand( ast->dst ) );
};

std::string PrinterAT::visit_Imm( const at::Imm& ast ) {
    return std::format( "#{}", ast->value );
};

std::string PrinterAT::visit_Register( const at::Register& ast ) {
    return std::format( "%{}", ast->reg );
};

std::string PrinterAT::visit_Ret( const at::Ret& ast ) {
    return "RET";
};