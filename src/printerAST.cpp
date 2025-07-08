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

std::string PrinterAST::print( const ast::Program& ast ) {
    return ast->accept( this );
}

std::string PrinterAST::visit_Program( const ast::Program& ast ) {
    std::string buf = to_string( TokenType::INT );
    buf += " " + ast->function->accept( this );
    return buf;
}

std::string PrinterAST::visit_FunctionDef( const ast::FunctionDef& ast ) {
    std::string buf = std::format( "{}({}) {{\n", ast->name, TokenType::VOID );
    buf += ast->statement->accept( this );
    buf += "\n}\n";
    return buf;
}

std::string PrinterAST::visit_Statement( const ast::Statement& ast ) {
    return indent + ast->ret->accept( this ) + ";";
}

std::string PrinterAST::visit_Return( const ast::Return& ast ) {
    return "return " + ast->expr->accept( this );
}

std::string PrinterAST::visit_Expr( const ast::Expr& ast ) {
    return ast->constant->accept( this );
}

std::string PrinterAST::visit_Constant( const ast::Constant& ast ) {
    return std::format( "{:d}", ast->value );
}
