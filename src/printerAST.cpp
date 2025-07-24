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

std::string PrinterAST::visit_Program( const ast::Program ast ) {
    std::string buf = to_string( TokenType::INT );
    buf += " " + ast->function->accept( this );
    return buf;
}

std::string PrinterAST::visit_FunctionDef( const ast::FunctionDef ast ) {
    std::string buf = std::format( "{}({}) {}", ast->name, TokenType::VOID, new_line );
    buf += ast->block->accept( this );
    return buf;
}

std::string PrinterAST::block_item( const ast::BlockItem ast ) {
    return std::visit( overloaded { [ this ]( ast::Declaration ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Statement ast ) -> std::string { return statement( ast ); } },
                       ast );
}

std::string PrinterAST::visit_Declaration( const ast::Declaration ast ) {
    std::string buf = "int " + ast->name;
    if ( ast->init ) {
        buf += " = " + expr( ast->init.value() );
    }
    return buf + ";";
}

std::string PrinterAST::statement( const ast::Statement ast ) {
    return std::visit( overloaded { [ this ]( ast::Return ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::If ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Goto ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Label ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Compound ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Expr e ) -> std::string { return expr( e ) + ";"; },
                                    [ this ]( ast::Null ) -> std::string { return ""; } },
                       ast );
}

std::string PrinterAST::visit_If( const ast::If ast ) {
    std::string buf = std::format( "if({})\n", expr( ast->condition ) );
    buf += indent + statement( ast->then ) + "\n";
    if ( ast->else_stat ) {
        buf += "else\n";
        buf += indent + statement( ast->else_stat.value() ) + "\n";
    }
    return buf;
}

std::string PrinterAST::visit_Null( const ast::Null ast ) {
    return ";";
}

std::string PrinterAST::visit_Goto( const ast::Goto ast ) {
    return "goto " + ast->label + ";";
}

std::string PrinterAST::visit_Label( ast::Label ast ) {
    return ast->label + ":";
}

std::string PrinterAST::visit_Compound( const ast::Compound ast ) {
    std::string buf = "{" + new_line;
    for ( auto b : ast->block_items ) {
        buf += indent + block_item( b ) + new_line;
    }
    buf += "}";
    return buf;
}

std::string PrinterAST::expr( const ast::Expr ast ) {
    return std::visit( overloaded { [ this ]( ast::UnaryOp u ) -> std::string { return u->accept( this ); },
                                    [ this ]( ast::BinaryOp b ) -> std::string { return b->accept( this ); },
                                    [ this ]( ast::PostOp b ) -> std::string { return b->accept( this ); },
                                    [ this ]( ast::Conditional c ) -> std::string { return c->accept( this ); },
                                    [ this ]( ast::Assign a ) -> std::string { return a->accept( this ); },
                                    [ this ]( ast::Var v ) -> std::string { return v->accept( this ); },
                                    [ this ]( ast::Constant c ) -> std::string { return c->accept( this ); } },
                       ast );
}

std::string PrinterAST::visit_Return( const ast::Return ast ) {
    return "return " + expr( ast->expr ) + ";";
}

std::string PrinterAST::visit_BinaryOp( const ast::BinaryOp ast ) {
    return std::format( "({} {} {})", expr( ast->left ), ast->op, expr( ast->right ) );
}

std::string PrinterAST::visit_PostOp( const ast::PostOp ast ) {
    return std::format( "({}{})", expr( ast->operand ), ast->op );
}

std::string PrinterAST::visit_Conditional( const ast::Conditional ast ) {
    return std::format( "({} ? {} : {})", expr( ast->condition ), expr( ast->then_expr ), expr( ast->else_expr ) );
}

std::string PrinterAST::visit_UnaryOp( const ast::UnaryOp ast ) {
    if ( ast->op == TokenType::INCREMENT || ast->op == TokenType::DECREMENT ) {
        return std::format( "({}{})", expr( ast->operand ), ast->op );
    }
    return std::format( "({}{})", ast->op, expr( ast->operand ) );
};

std::string PrinterAST::visit_Assign( const ast::Assign ast ) {
    return std::format( "{} {} {}", expr( ast->left ), ast->op, expr( ast->right ) );
}

std::string PrinterAST::visit_Var( const ast::Var ast ) {
    return ast->name;
};

std::string PrinterAST::visit_Constant( const ast::Constant ast ) {
    return std::format( "{:d}", ast->value );
}
