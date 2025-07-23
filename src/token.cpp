//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 23/7/2025.
//

#include "token.h"

const char* to_string( const TokenType l ) {
    switch ( l ) {
        using enum TokenType;
    case L_PAREN :
        return "(";
    case R_PAREN :
        return ")";
    case L_BRACE :
        return "{";
    case R_BRACE :
        return "}";
    case SEMICOLON :
        return ";";
    case PLUS :
        return "+";
    case DASH :
        return "-";
    case ASTÉRIX :
        return "*";
    case SLASH :
        return "/";
    case PERCENT :
        return "%";
    case TILDE :
        return "~";
    case AMPERSAND :
        return "&";
    case PIPE :
        return "|";
    case CARET :
        return "^";
    case EXCLAMATION :
        return "!";
    case EQUALS :
        return "=";
    case GREATER :
        return ">";
    case LESS :
        return "<";
    case QUESTION :
        return "?";
    case COLON :
        return ":";

    case INCREMENT :
        return "++";
    case DECREMENT :
        return "--";
    case LEFT_SHIFT :
        return "<<";
    case RIGHT_SHIFT :
        return ">>";
    case GREATER_EQUALS :
        return ">=";
    case LESS_EQUALS :
        return "<=";
    case COMPARISON_EQUALS :
        return "==";
    case COMPARISON_NOT :
        return "!=";
    case LOGICAL_AND :
        return "&&";
    case LOGICAL_OR :
        return "||";
    case COMPOUND_PLUS :
        return "+=";
    case COMPOUND_MINUS :
        return "-=";
    case COMPOUND_ASTERIX :
        return "*=";
    case COMPOUND_SLASH :
        return "/=";
    case COMPOUND_PERCENT :
        return "%=";
    case COMPOUND_AND :
        return "&=";
    case COMPOUND_OR :
        return "|=";
    case COMPOUND_XOR :
        return "^=";
    case COMPOUND_LEFT_SHIFT :
        return "<<=";
    case COMPOUND_RIGHT_SHIFT :
        return ">>=";

    case IDENTIFIER :
        return "<identifier>";
    case CONSTANT :
        return "<constant>";

    case Eof :
        return "<eof>";
    case Null :
        return "<null>";

    // Keywords
    case IF :
        return "if";
    case ELSE :
        return "else";
    case INT :
        return "int";
    case RETURN :
        return "return";
    case VOID :
        return "void";
    default :
        return "INTERNAL-ERROR";
    }
}

std::string to_string( Token const& t ) {
    switch ( t.tok ) {
        using enum TokenType;
    case IDENTIFIER :
        return std::string( "<id: " + t.value + ">" );
    case CONSTANT :
        return std::string( "<const: " + t.value + ">" );
    default :
        return to_string( t.tok );
    }
}