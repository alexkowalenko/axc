//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#include "printerTAC.h"
#include "common.h"
#include "tac/includes.h"

std::string PrinterTAC::print( const tac::Program ast ) {
    return ast->accept( this );
}

std::string PrinterTAC::visit_Program( const tac::Program ast ) {
    std::string buf;
    for ( const auto& function : ast->functions ) {
        buf += function->accept( this ) + "\n\n";
    }
    return buf;
}

std::string PrinterTAC::visit_FunctionDef( const tac::FunctionDef ast ) {
    std::string buf = std::format( "Function: {}\n", ast->name );
    for ( auto const& instr : ast->instructions ) {
        buf += indent;
        buf += std::visit( overloaded {
                               [ this ]( tac::Return r ) -> std::string { return indent + r->accept( this ); },
                               [ this ]( tac::Unary r ) -> std::string { return indent + r->accept( this ); },
                               [ this ]( tac::Binary r ) -> std::string { return indent + r->accept( this ); },
                               [ this ]( tac::Copy r ) -> std::string { return indent + r->accept( this ); },
                               [ this ]( tac::Jump r ) -> std::string { return indent + r->accept( this ); },
                               [ this ]( tac::JumpIfZero r ) -> std::string { return indent + r->accept( this ); },
                               [ this ]( tac::JumpIfNotZero r ) -> std::string { return indent + r->accept( this ); },
                               [ this ]( tac::Label r ) -> std::string { return r->accept( this ); },
                               [ this ]( tac::FunCall r ) -> std::string { return indent + r->accept( this ); },

                           },
                           instr );
        buf += "\n";
    }
    return buf;
}

std::string PrinterTAC::value( const tac::Value ast ) {
    return std::visit( overloaded { [ this ]( tac::Constant c ) -> std::string { return c->accept( this ); },
                                    [ this ]( tac::Variable v ) -> std::string { return v->accept( this ); } },
                       ast );
}

std::string PrinterTAC::visit_Return( const tac::Return ast ) {
    return std::format( "Return {} ", value( ast->value ) );
}

std::string PrinterTAC::visit_Unary( const tac::Unary ast ) {
    std::string buf = "Unary ";
    switch ( ast->op ) {
    case tac::UnaryOpType::Negate :
        buf += "Negate ";
        break;
    case tac::UnaryOpType::Complement :
        buf += "Complement ";
        break;
    case tac::UnaryOpType::Not :
        buf += "Not ";
        break;
    default :
        break;
    }
    buf += std::format( "{} {}", value( ast->src ), value( ast->dst ) );
    return buf;
}

std::string PrinterTAC::visit_Binary( const tac::Binary ast ) {
    std::string buf = "Binary ";
    switch ( ast->op ) {
    case tac::BinaryOpType::Add :
        buf += "Add ";
        break;
    case tac::BinaryOpType::Subtract :
        buf += "Sub ";
        break;
    case tac::BinaryOpType::Multiply :
        buf += "Mul ";
        break;
    case tac::BinaryOpType::Divide :
        buf += "Div ";
        break;
    case tac::BinaryOpType::Modulo :
        buf += "Mod ";
        break;
    case tac::BinaryOpType::BitwiseAnd :
        buf += "BitwiseAnd ";
        break;
    case tac::BinaryOpType::BitwiseOr :
        buf += "BitwiseOr ";
        break;
    case tac::BinaryOpType::BitwiseXor :
        buf += "BitwiseXor ";
        break;
    case tac::BinaryOpType::ShiftLeft :
        buf += "ShiftLeft ";
        break;
    case tac::BinaryOpType::ShiftRight :
        buf += "ShiftRight ";
        break;
    case tac::BinaryOpType::Equal :
        buf += "Equal ";
        break;
    case tac::BinaryOpType::NotEqual :
        buf += "NotEqual ";
        break;
    case tac::BinaryOpType::Less :
        buf += "Less ";
        break;
    case tac::BinaryOpType::LessEqual :
        buf += "LessEqual ";
        break;
    case tac::BinaryOpType::Greater :
        buf += "Greater ";
        break;
    case tac::BinaryOpType::GreaterEqual :
        buf += "GreaterEqual ";
        break;
    case tac::BinaryOpType::And :
        buf += "And ";
        break;
    case tac::BinaryOpType::Or :
        buf += "Or ";
        break;
    default :
        break;
    }
    buf += std::format( "{} {} {}", value( ast->src1 ), value( ast->src2 ), value( ast->dst ) );
    return buf;
}

std::string PrinterTAC::visit_Copy( const tac::Copy ast ) {
    return std::format( "Copy {:s}, {:s}", value( ast->src ), value( ast->dst ) );
}

std::string PrinterTAC::visit_Jump( const tac::Jump ast ) {
    return std::format( "Jump {:s}", ast->target );
}

std::string PrinterTAC::visit_JumpIfZero( const tac::JumpIfZero ast ) {
    return std::format( "JumpIfZero {:s} -> {:s}", value( ast->condition ), ast->target );
}

std::string PrinterTAC::visit_JumpIfNotZero( const tac::JumpIfNotZero ast ) {
    return std::format( "JumpIfNotZero {:s} -> {:s}", value( ast->condition ), ast->target );
}

std::string PrinterTAC::visit_Label( const tac::Label ast ) {
    return std::format( "Label: {:s}", ast->name );
}

std::string PrinterTAC::visit_FunCall( const tac::FunCall ast ) {
    std::string buf = std::format( "FunCall: {:s}(", ast->function_name );
    for ( const auto& arg : ast->arguments ) {
        buf += value( arg ) + ", ";
    }
    if ( !ast->arguments.empty() ) {
        buf.pop_back(); // Remove trailing space
        buf.pop_back(); // Remove trailing comma
    }
    buf += ")";
    return buf;
};

std::string PrinterTAC::visit_Constant( const tac::Constant ast ) {
    return std::format( "Constant( {:d})", ast->value );
}

std::string PrinterTAC::visit_Variable( const tac::Variable ast ) {
    return std::format( "Variable({})", ast->name );
}
