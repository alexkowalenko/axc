//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#include "printerTAC.h"
#include "tac/includes.h"
#include "common.h"

std::string PrinterTAC::print( const tac::Program& ast ) {
    return ast->accept(this);
}

std::string PrinterTAC::visit_Program( const tac::Program& ast ) {
    return ast->function->accept(this);
}

std::string PrinterTAC::visit_FunctionDef( const tac::FunctionDef& ast ) {
    std::string buf = std::format( "Function({})\n", ast->name );
    for ( auto const& instr : ast->instructions ) {
        buf += indent;
        buf += std::visit( overloaded { [ this ]( tac::Return r ) -> std::string { return r->accept( this ); },
                                        [ this ]( tac::Unary r ) -> std::string { return r->accept( this ); } },
                           instr );
        buf += "\n";
    }
    return buf;
}

std::string PrinterTAC::value(const tac::Value& ast) {
    return std::visit( overloaded { [ this ]( tac::Constant c ) -> std::string { return c->accept( this ); },
                                    [ this ]( tac::Variable v ) -> std::string { return v->accept( this ); } },
                       ast );
}

std::string PrinterTAC::visit_Return( const tac::Return& ast ) {
    return std::format("Return({})", value(ast->value));
}

std::string PrinterTAC::visit_Unary( const tac::Unary& ast ) {
    return  std::format( "Unary({}, {}, {})", ast->op, value(ast->src), value(ast->dst) );
}

std::string PrinterTAC::visit_Constant( const tac::Constant& ast ) {
    return std::format( "Constant({:d})", ast->value );
}

std::string PrinterTAC::visit_Variable( const tac::Variable& ast ) {
    return std::format("Variable({})",ast->name);
}
