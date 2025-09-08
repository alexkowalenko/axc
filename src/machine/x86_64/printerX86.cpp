//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "printerX86.h"

#include <algorithm>

#include "common.h"
#include "x86_at/includes.h"
#include "x86_common.h"

std::string to_upper( const std::string& s ) {
    std::string buf = "  ";
    std::transform( s.begin(), s.end(), buf.begin(), []( unsigned char c ) { return std::toupper( c ); } );
    return buf;
}

std::string PrinterX86::print( const x86_at::Program ast ) {
    return ast->accept( this );
}

std::string PrinterX86::visit_Program( const x86_at::Program ast ) {
    std::string buf;
    for ( const auto& item : ast->top_level ) {
        buf += std::visit( [ this ]( auto&& f ) -> std::string { return f->accept( this ); }, item );
    }
    return buf;
};

std::string PrinterX86::visit_FunctionDef( const x86_at::FunctionDef ast ) {
    std::string buf = std::format( "Function: {} ({})\n", ast->name, ast->global ? "global" : "static" );
    for ( auto const& instr : ast->instructions ) {
        buf += indent;
        buf += std::visit( [ this ]( auto&& v ) -> std::string { return v->accept( this ); }, instr );
        buf += "\n";
    }
    return buf;
};

std::string PrinterX86::visit_StaticVariable( x86_at::StaticVariable ast ) {
    return std::format( "StaticVariable: {} ({}, {})\n", ast->name, ast->global ? "global" : "static", ast->init );
}

std::string PrinterX86::operand( const x86_at::Operand& op ) {
    return std::visit( [ this ]( auto&& v ) -> std::string { return v->accept( this ); }, op );
}

std::string PrinterX86::visit_Mov( const x86_at::Mov ast ) {
    return std::format( "MOV({}: {}, {})", to_string( ast->type ), operand( ast->src ), operand( ast->dst ) );
};

std::string PrinterX86::visit_Movsx( x86_at::Movsx ast ) {
    return std::format( "MOVSX({}, {})", operand( ast->src ), operand( ast->dst ) );
}

std::string PrinterX86::visit_Imm( const x86_at::Imm ast ) {
    return std::format( "#{}", ast->value );
};

std::string PrinterX86::visit_Unary( const x86_at::Unary ast ) {
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

std::string PrinterX86::visit_Binary( const x86_at::Binary ast ) {
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

std::string PrinterX86::visit_Idiv( const x86_at::Idiv ast ) {
    return std::format( "Idiv({})", operand( ast->src ) );
}

std::string PrinterX86::visit_Cdq( const x86_at::Cdq ast ) {
    return "Cdq";
}

std::string PrinterX86::visit_Cmp( const x86_at::Cmp ast ) {
    return std::format( "Cmp({}, {})", operand( ast->operand1 ), operand( ast->operand2 ) );
}

std::string PrinterX86::visit_Jump( const x86_at::Jump ast ) {
    return std::format( "Jump({})", ast->target );
}

std::string PrinterX86::visit_JumpCC( const x86_at::JumpCC ast ) {
    return std::format( "JumpCC({} -> {})", to_upper( cond_code( ast->cond ) ), ast->target );
}

std::string PrinterX86::visit_SetCC( const x86_at::SetCC ast ) {
    return std::format( "SetCC({} -> {})", to_upper( cond_code( ast->cond ) ), operand( ast->operand ) );
}

std::string PrinterX86::visit_Label( const x86_at::Label ast ) {
    return std::format( "Label({})", ast->name );
}

std::string PrinterX86::visit_AllocateStack( const x86_at::AllocateStack ast ) {
    return std::format( "AllocateStack({})", ast->size );
};

std::string PrinterX86::visit_DeallocateStack( const x86_at::DeallocateStack ast ) {
    return std::format( "DeallocateStack({})", ast->size );
}

std::string PrinterX86::visit_Push( const x86_at::Push ast ) {
    return std::format( "Push({})", operand( ast->operand ) );
}

std::string PrinterX86::visit_Call( const x86_at::Call ast ) {
    std::string buf = "Call: " + ast->function_name;
    return buf;
}

std::string PrinterX86::visit_Register( const x86_at::Register ast ) {
    return std::format( "%{}", assemble_reg( ast ) );
};

std::string PrinterX86::visit_Pseudo( const x86_at::Pseudo ast ) {
    return std::format( "Pseudo({})", ast->name );
}

std::string PrinterX86::visit_Stack( const x86_at::Stack ast ) {
    return std::format( "Stack({})", ast->offset );
}

std::string PrinterX86::visit_Data( const x86_at::Data ast ) {
    return std::format( "Data({})", ast->name );
}

std::string PrinterX86::visit_Ret( const x86_at::Ret ast ) {
    return "Ret";
};