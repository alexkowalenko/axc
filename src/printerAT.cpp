//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "printerAT.h"

#include "ast/base.h"
#include "at/includes.h"
#include "common.h"

std::string PrinterAT::print( const at::Program ast ) {
    return ast->accept( this );
}

std::string PrinterAT::visit_Program( const at::Program ast ) {
    return ast->function->accept( this );
};

std::string PrinterAT::visit_FunctionDef( const at::FunctionDef ast ) {
    std::string buf = std::format( "Function({})\n", ast->name );
    for ( auto const& instr : ast->instructions ) {
        buf += indent;
        buf += std::visit( overloaded { [ this ]( at::Mov v ) -> std::string { return v->accept( this ); },
                                        [ this ]( at::Unary u ) -> std::string { return u->accept( this ); },
                                        [ this ]( at::Binary b ) -> std::string { return b->accept( this ); },
                                        [ this ]( at::AllocateStack a ) -> std::string { return a->accept( this ); },
                                        [ this ]( at::Idiv i ) -> std::string { return i->accept( this ); },
                                        [ this ]( at::Cdq c ) -> std::string { return c->accept( this ); },
                                        [ this ]( at::Ret r ) -> std::string { return r->accept( this ); } },
                           instr );
        buf += "\n";
    }
    return buf;
};

std::string PrinterAT::operand( const at::Operand &op ) {
    return std::visit( overloaded {
                           [ this ]( at::Imm v ) -> std::string { return v->accept( this ); },
                           [ this ]( at::Register r ) -> std::string { return r->accept( this ); },
                           [ this ]( at::Pseudo p ) -> std::string { return p->accept( this ); },
                           [ this ]( at::Stack s ) -> std::string { return s->accept( this ); },
                       },
                       op );
}

std::string PrinterAT::visit_Mov( const at::Mov ast ) {
    return std::format( "MOV({}, {})", operand( ast->src ), operand( ast->dst ) );
};

std::string PrinterAT::visit_Imm( const at::Imm ast ) {
    return std::format( "#{}", ast->value );
};

std::string PrinterAT::visit_Unary( const at::Unary ast ) {
    std::string buf = "Unary(";
    switch ( ast->op ) {
    case at::UnaryOpType::NEG :
        buf += "NEG";
        break;
    case at::UnaryOpType::NOT :
        buf += "NOT";
        break;
    default :
        break;
    }
    buf += ", " + operand( ast->operand ) + ")";
    return buf;
};

std::string PrinterAT::visit_Binary( const at::Binary ast ) {
    std::string buf = "Binary(";
    switch ( ast->op ) {
    case at::BinaryOpType::ADD :
        buf += "ADD";
        break;
    case at::BinaryOpType::SUB :
        buf += "SUB";
        break;
    case at::BinaryOpType::MUL :
        buf += "MUL";
        break;
    }
    buf += ", " + operand( ast->operand1 ) + ", " + operand( ast->operand2 ) + ")";
    return buf;
}

std::string PrinterAT::visit_Idiv( const at::Idiv ast ) {
    return std::format("Idiv({})", operand( ast->src ));
}

std::string PrinterAT::visit_Cdq( const at::Cdq ast ) {
    return "Cdq";
}

std::string PrinterAT::visit_AllocateStack( const at::AllocateStack ast ) {
    return std::format( "AllocateStack({})", ast->size );
};

std::string PrinterAT::visit_Register( const at::Register ast ) {
    return std::format( "%{}", ast->reg );
};

std::string PrinterAT::visit_Pseudo( const at::Pseudo ast ) {
    return std::format( "Pseudo({})", ast->name );
}

std::string PrinterAT::visit_Stack( const at::Stack ast ) {
    return std::format( "Stack({})", ast->offset );
}

std::string PrinterAT::visit_Ret( const at::Ret ast ) {
    return "Ret";
};