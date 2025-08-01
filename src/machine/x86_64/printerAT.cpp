//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "printerAT.h"

#include <algorithm>

#include "common.h"
#include "x86_at/includes.h"
#include "x86_common.h"

std::string to_upper( const std::string& s ) {
    std::string buf = "  ";
    std::transform( s.begin(), s.end(), buf.begin(), []( unsigned char c ) { return std::toupper( c ); } );
    return buf;
}

std::string PrinterAT::print( const x86_at::Program ast ) {
    return ast->accept( this );
}

std::string PrinterAT::visit_Program( const x86_at::Program ast ) {
    return ast->function->accept( this );
};

std::string PrinterAT::visit_FunctionDef( const x86_at::FunctionDef ast ) {
    std::string buf = std::format( "Function({})\n", ast->name );
    for ( auto const& instr : ast->instructions ) {
        buf += indent;
        buf += std::visit( overloaded { [ this ]( x86_at::Mov v ) -> std::string { return v->accept( this ); },
                                        [ this ]( x86_at::Unary u ) -> std::string { return u->accept( this ); },
                                        [ this ]( x86_at::Binary b ) -> std::string { return b->accept( this ); },
                                        [ this ]( x86_at::Cmp b ) -> std::string { return b->accept( this ); },
                                        [ this ]( x86_at::AllocateStack a ) -> std::string { return a->accept( this ); },
                                        [ this ]( x86_at::Idiv i ) -> std::string { return i->accept( this ); },
                                        [ this ]( x86_at::Cdq c ) -> std::string { return c->accept( this ); },
                                        [ this ]( x86_at::Jump c ) -> std::string { return c->accept( this ); },
                                        [ this ]( x86_at::JumpCC c ) -> std::string { return c->accept( this ); },
                                        [ this ]( x86_at::SetCC c ) -> std::string { return c->accept( this ); },
                                        [ this ]( x86_at::Label c ) -> std::string { return c->accept( this ); },
                                        [ this ]( x86_at::Ret r ) -> std::string { return r->accept( this ); } },
                           instr );
        buf += "\n";
    }
    return buf;
};

std::string PrinterAT::operand( const x86_at::Operand& op ) {
    return std::visit( overloaded {
                           [ this ]( x86_at::Imm v ) -> std::string { return v->accept( this ); },
                           [ this ]( x86_at::Register r ) -> std::string { return r->accept( this ); },
                           [ this ]( x86_at::Pseudo p ) -> std::string { return p->accept( this ); },
                           [ this ]( x86_at::Stack s ) -> std::string { return s->accept( this ); },
                       },
                       op );
}

std::string PrinterAT::visit_Mov( const x86_at::Mov ast ) {
    return std::format( "MOV({}, {})", operand( ast->src ), operand( ast->dst ) );
};

std::string PrinterAT::visit_Imm( const x86_at::Imm ast ) {
    return std::format( "#{}", ast->value );
};

std::string PrinterAT::visit_Unary( const x86_at::Unary ast ) {
    std::string buf = "Unary(";
    switch ( ast->op ) {
    case x86_at::UnaryOpType::NEG :
        buf += "NEG";
        break;
    case x86_at::UnaryOpType::NOT :
        buf += "NOT";
        break;
    default :
        break;
    }
    buf += ", " + operand( ast->operand ) + ")";
    return buf;
};

std::string PrinterAT::visit_Binary( const x86_at::Binary ast ) {
    std::string buf = "Binary(";
    switch ( ast->op ) {
    case x86_at::BinaryOpType::ADD :
        buf += "ADD";
        break;
    case x86_at::BinaryOpType::SUB :
        buf += "SUB";
        break;
    case x86_at::BinaryOpType::MUL :
        buf += "MUL";
        break;
    case x86_at::BinaryOpType::AND :
        buf += "AND";
        break;
    case x86_at::BinaryOpType::OR :
        buf += "OR";
        break;
    case x86_at::BinaryOpType::XOR :
        buf += "XOR";
        break;
    case x86_at::BinaryOpType::SHL :
        buf += "SHL";
        break;
    case x86_at::BinaryOpType::SHR :
        buf += "SHR";
        break;
    default :
    }
    buf += ", " + operand( ast->operand1 ) + ", " + operand( ast->operand2 ) + ")";
    return buf;
}

std::string PrinterAT::visit_Idiv( const x86_at::Idiv ast ) {
    return std::format( "Idiv({})", operand( ast->src ) );
}

std::string PrinterAT::visit_Cdq( const x86_at::Cdq ast ) {
    return "Cdq";
}

std::string PrinterAT::visit_Cmp( const x86_at::Cmp ast ) {
    return std::format( "Cmp({}, {})", operand( ast->operand1 ), operand( ast->operand2 ) );
}

std::string PrinterAT::visit_Jump( const x86_at::Jump ast ) {
    return std::format( "Jump({})", ast->target );
}

std::string PrinterAT::visit_JumpCC( const x86_at::JumpCC ast ) {
    return std::format( "JumpCC({} -> {})", to_upper(cond_code( ast->cond )), ast->target );
}

std::string PrinterAT::visit_SetCC( const x86_at::SetCC ast ) {
    return std::format( "SetCC({} -> {})", to_upper(cond_code( ast->cond )), operand( ast->operand ) );
}

std::string PrinterAT::visit_Label( const x86_at::Label ast ) {
    return std::format( "Label({})", ast->name );
}

std::string PrinterAT::visit_AllocateStack( const x86_at::AllocateStack ast ) {
    return std::format( "AllocateStack({})", ast->size );
};

std::string PrinterAT::visit_Register( const x86_at::Register ast ) {
    return std::format( "%({}.{})", to_string(ast->reg), to_string(ast->size) );
};

std::string PrinterAT::visit_Pseudo( const x86_at::Pseudo ast ) {
    return std::format( "Pseudo({})", ast->name );
}

std::string PrinterAT::visit_Stack( const x86_at::Stack ast ) {
    return std::format( "Stack({})", ast->offset );
}

std::string PrinterAT::visit_Ret( const x86_at::Ret ast ) {
    return "Ret";
};