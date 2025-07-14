//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "printerAST.h"

#include "ast/includes.h"
#include "common.h"

std::string PrinterAST::print( const ast::Program& ast ) {
    return ast->accept( this );
}

std::string PrinterAST::visit_Program( const ast::Program& ast ) {
    std::string buf = to_string( TokenType::INT );
    buf += " " + ast->function->accept( this );
    return buf;
}

std::string PrinterAST::visit_FunctionDef( const ast::FunctionDef& ast ) {
    std::string buf = std::format( "{}({}) {{{}", ast->name, TokenType::VOID, new_line );
    buf += ast->statement->accept( this );
    buf += new_line + "}" + new_line;
    return buf;
}

std::string PrinterAST::visit_Statement( const ast::Statement& ast ) {
    return indent + ast->ret->accept( this ) + ";";
}

std::string PrinterAST::expr( const ast::Expr& ast ) {
    return std::format(
        "{}", std::visit( overloaded { [ this ]( ast::UnaryOp u ) -> std::string { return u->accept( this ); },
                                       [ this ]( ast::BinaryOp b ) -> std::string { return b->accept( this ); },
                                       [ this ]( ast::Constant c ) -> std::string { return c->accept( this ); } },
                          ast ) );
}

std::string PrinterAST::visit_Return( const ast::Return& ast ) {
    return "return " + expr( ast->expr );
}
std::string PrinterAST::visit_BinaryOp( const ast::BinaryOp& ast ) {
    return std::format( "({} {} {})", expr( ast->left ), ast->op, expr( ast->right ) );
}

std::string PrinterAST::visit_UnaryOp( const ast::UnaryOp& ast ) {
    return std::format( "{}{}", ast->op, expr( ast->operand ) );
};

std::string PrinterAST::visit_Constant( const ast::Constant& ast ) {
    return std::format( "{:d}", ast->value );
}
