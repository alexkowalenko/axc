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
        buf += std::visit(
            overloaded { [ this ]( arm64_at::Mov v ) -> std::string { return v->accept( this ); },
                         [ this ]( arm64_at::Load l ) -> std::string { return l->accept( this ); },
                         [ this ]( arm64_at::Store s ) -> std::string { return s->accept( this ); },
                         [ this ]( arm64_at::Ret r ) -> std::string { return r->accept( this ); },
                         [ this ]( arm64_at::Unary u ) -> std::string { return u->accept( this ); },
                         [ this ]( arm64_at::Binary b ) -> std::string { return b->accept( this ); },
                         [ this ]( arm64_at::AllocateStack a ) -> std::string { return a->accept( this ); },
                         [ this ]( arm64_at::DeallocateStack d ) -> std::string { return d->accept( this ); },
                         [ this ]( arm64_at::Branch b ) -> std::string { return b->accept( this ); },
                         [ this ]( arm64_at::BranchCC b ) -> std::string { return b->accept( this ); },
                         [ this ]( arm64_at::Label l ) -> std::string { return l->accept( this ); },
                         [ this ]( arm64_at::Cmp c ) -> std::string { return c->accept( this ); },
                         [ this ]( arm64_at::Cset c ) -> std::string { return c->accept( this ); } },
            instr );
        buf += "\n";
    }
    return buf;
}
std::string PrinterARM64::visit_Mov( const arm64_at::Mov ast ) {
    return std::format( "Move({}<-{})", operand( ast->dst ), operand( ast->src ) );
}

std::string PrinterARM64::visit_Load( const arm64_at::Load ast ) {
    return std::format( "Load({}<-{})", operand( ast->dst ), operand( ast->src ) );
}

std::string PrinterARM64::visit_Store( const arm64_at::Store ast ) {
    return std::format( "Store({}<-{})", operand( ast->dst ), operand( ast->src ) );
}

std::string PrinterARM64::visit_AllocateStack( arm64_at::AllocateStack ast ) {
    return std::format( "AllocateStack({})", ast->size );
}

std::string PrinterARM64::visit_DeallocateStack( arm64_at::DeallocateStack ast ) {
    return std::format( "DeallocateStack({})", ast->size );
}

std::string PrinterARM64::visit_Ret( const arm64_at::Ret ast ) {
    return "ret";
}

std::string PrinterARM64::visit_Unary( const arm64_at::Unary ast ) {
    std::string buf = "Unary(";
    switch ( ast->op ) {
    case arm64_at::UnaryOpType::NEG :
        buf += "neg";
        break;
    case arm64_at::UnaryOpType::BITWISE_NOT :
        buf += "bitwise_not";
        break;
    case arm64_at::UnaryOpType::LOGICAL_NOT :
        buf += "logical_not";
        break;
    }
    buf += std::format( ", {}, {})", operand( ast->dst ), operand( ast->src ) );
    return buf;
}

std::string PrinterARM64::visit_Binary( const arm64_at::Binary ast ) {
    std::string buf = "Binary(";
    switch ( ast->op ) {
    case arm64_at::BinaryOpType::ADD :
        buf += "add";
        break;
    case arm64_at::BinaryOpType::SUB :
        buf += "sub";
        break;
    case arm64_at::BinaryOpType::MUL :
        buf += "mul";
        break;
    case arm64_at::BinaryOpType::DIV :
        buf += "div";
        break;
    case arm64_at::BinaryOpType::MOD :
        buf += "mod";
        break;
    case arm64_at::BinaryOpType::AND :
        buf += "and";
        break;
    case arm64_at::BinaryOpType::OR :
        buf += "or";
        break;
    case arm64_at::BinaryOpType::XOR :
        buf += "xor";
        break;
    case arm64_at::BinaryOpType::SHL :
        buf += "shl";
        break;
    case arm64_at::BinaryOpType::SHR :
        buf += "shr";
        break;
    }
    buf += std::format( ", {}, {}, {})", operand( ast->dst ), operand( ast->src1 ), operand( ast->src2 ) );
    return buf;
}

std::string PrinterARM64::visit_Branch( arm64_at::Branch ast ) {
    return std::format( "Branch({})", ast->target );
}

std::string PrinterARM64::visit_BranchCC( arm64_at::BranchCC ast ) {
    return std::format( "BranchCC({})", ast->target );
}

std::string PrinterARM64::visit_Label( arm64_at::Label ast ) {
    return std::format( "Label({})", ast->name );
}

std::string PrinterARM64::visit_Cmp( arm64_at::Cmp ast ) {
    return std::format( "Cmp({}, {})", operand( ast->operand1 ), operand( ast->operand2 ) );
}

std::string PrinterARM64::visit_Cset( arm64_at::Cset ast ) {
    std::string buf = "Cset(";
    switch ( ast->cond ) {
    case arm64_at::CondCode::EQ :
        buf += "eq";
        break;
    case arm64_at::CondCode::NE :
        buf += "ne";
        break;
    }
    buf += std::format( ", {})", operand( ast->operand ) );
    return buf;
}

std::string PrinterARM64::operand( const arm64_at::Operand& op ) {
    return std::visit( overloaded { [ this ]( arm64_at::Imm v ) -> std::string { return v->accept( this ); },
                                    [ this ]( arm64_at::Register r ) -> std::string { return r->accept( this ); },
                                    [ this ]( arm64_at::Pseudo p ) -> std::string { return p->accept( this ); },
                                    [ this ]( arm64_at::Stack s ) -> std::string { return s->accept( this ); } },
                       op );
}

std::string PrinterARM64::visit_Imm( const arm64_at::Imm ast ) {
    return std::format( "#{}", ast->value );
}

std::string PrinterARM64::visit_Register( const arm64_at::Register ast ) {
    return to_string( ast->reg );
}

std::string PrinterARM64::visit_Pseudo( const arm64_at::Pseudo ast ) {
    return std::format( "Pseudo({})", ast->name );
}

std::string PrinterARM64::visit_Stack( const arm64_at::Stack ast ) {
    return std::format( "Stack({})", ast->offset );
}
