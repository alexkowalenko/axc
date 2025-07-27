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
                                    [ this ]( ast::Statement ast ) -> std::string { return ast->accept( this ); } },
                       ast );
}

std::string PrinterAST::visit_Declaration( const ast::Declaration ast ) {
    std::string buf = "int " + ast->name;
    if ( ast->init ) {
        buf += " = " + expr( ast->init.value() );
    }
    return buf + ";";
}

std::string PrinterAST::visit_Statement( const ast::Statement ast ) {
    std::string buf;
    if ( ast->label ) {
        buf = ast->label.value()->accept( this );
    }

    if ( ast->statement ) {
        buf += " " + statement( ast->statement.value() );
    }
    return buf;
}

std::string PrinterAST::statement( const ast::StatementItem ast ) {
    return std::visit( overloaded { [ this ]( ast::Return ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::If ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Goto ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Break ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Continue ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::While ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::DoWhile ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::For ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Switch ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Case ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Compound ast ) -> std::string { return ast->accept( this ); },
                                    [ this ]( ast::Expr e ) -> std::string { return expr( e ) + ";"; },
                                    [ this ]( ast::Null ) -> std::string { return ";"; } },
                       ast );
}

std::string PrinterAST::visit_If( const ast::If ast ) {
    std::string buf = std::format( "if({})\n", expr( ast->condition ) );
    buf += indent + ast->then->accept( this ) + "\n";
    if ( ast->else_stat ) {
        buf += "else\n";
        buf += indent + ast->else_stat.value()->accept( this ) + "\n";
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

std::string PrinterAST::visit_Break( const ast::Break ast ) {
    return "break;";
}

std::string PrinterAST::visit_Continue( const ast::Continue ast ) {
    return "continue;";
}

std::string PrinterAST::visit_While( const ast::While ast ) {
    std::string buf = "while(" + expr( ast->condition ) + ")" + new_line;
    buf += indent + ast->body->accept( this ) + new_line;
    return buf;
}

std::string PrinterAST::visit_DoWhile( const ast::DoWhile ast ) {
    std::string buf = "do" + new_line;
    buf += indent + ast->body->accept( this ) + new_line;
    buf += "while(" + expr( ast->condition ) + ");";
    return buf;
}

std::string PrinterAST::for_init( ast::ForInit ast ) {
    return std::visit( overloaded { [ this ]( ast::Expr e ) -> std::string { return expr( e ); },
                                    [ this ]( ast::Declaration d ) -> std::string { return d->accept( this ); } },
                       ast );
}

std::string PrinterAST::visit_For( const ast::For ast ) {
    std::string buf = "for(";
    if ( ast->init ) {
        buf += for_init( ast->init.value() );
    } else {
        buf += ";";
    }
    if ( ast->condition ) {
        buf += expr( ast->condition.value() );
    }
    buf += ";";
    if ( ast->increment ) {
        buf += expr( ast->increment.value() );
    }
    buf += ")" + new_line;
    buf += indent + ast->body->accept( this ) + new_line;
    return buf;
}

std::string PrinterAST::visit_Switch( const ast::Switch ast ) {
    std::string buf = "switch(" + expr( ast->condition ) + ") ";
    buf += ast->body->accept( this );
    return buf;
}

std::string PrinterAST::visit_Case( const ast::Case ast ) {
    std::string buf;
    if ( ast->is_default ) {
        buf = "default:";
    } else {
        buf = "case " + expr( ast->value ) + ":";
    }

    for ( auto b : ast->block_items ) {
        buf += indent + indent + block_item( b ) + new_line;
    }
    return buf;
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
