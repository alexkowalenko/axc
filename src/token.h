//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 7/7/2025.
//

#pragma once

#include <cstdint>
#include <format>

enum class TokenType : std::uint8_t {
    Null = 0,
    Eof,

    L_PAREN,
    R_PAREN,
    L_BRACE,
    R_BRACE,
    SEMICOLON,

    IDENTIFIER,
    CONSTANT,

    // Keywords
    INT,
    RETURN,
    VOID
};

constexpr auto to_string( const TokenType l ) {
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

    case IDENTIFIER :
        return "<identifier>";
    case CONSTANT :
        return "<constant>";

    case Eof :
        return "<eof>";
    case Null :
        return "<null>";

    // Keywords
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

class Location {
  public:
    constexpr Location( const size_t l, const size_t ch ) : line { l }, col { ch } {};
    constexpr Location() = default;

    size_t line { 0 };
    size_t col { 0 };
};

constexpr auto to_string( Location l ) {
    return std::format( "[{},{}]", l.line, l.col );
}

class Token {
  public:
    constexpr Token() : tok( TokenType::Null ), location( Location( 0, 0 ) ) {};
    constexpr Token( const TokenType t, const Location l ) : tok( t ), location( l ) {};
    constexpr Token( const TokenType t, const Location l, std::string v )
        : tok( t ), location( l ), value { std::move( v ) } {};

    TokenType   tok;
    Location    location;
    std::string value;
};

constexpr std::string to_string( Token const& t ) {
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
