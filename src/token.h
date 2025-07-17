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
    PLUS,
    DASH,
    ASTÉRIX,
    SLASH,
    PERCENT,
    AND,
    TILDE,
    AMPERSAND,
    PIPE,
    CARET,
    EXCLAMATION,
    EQUALS,
    LESS,
    GREATER,

    DECREMENT,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LOGICAL_AND,
    LOGICAL_OR,
    COMPARISON_EQUALS,
    COMPARISON_NOT,
    LESS_EQUALS,
    GREATER_EQUALS,

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

template <> struct std::formatter<TokenType> {
    constexpr static auto parse( std::format_parse_context& ctx ) { return ctx.begin(); }

    template <typename FormatContext> static auto format( TokenType const& obj, FormatContext& ctx ) {
        return std::format_to( ctx.out(), "{}", to_string( obj ) );
    }
};

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

template <> struct std::formatter<Location> {
    constexpr static auto parse( std::format_parse_context& ctx ) { return ctx.begin(); }

    template <typename FormatContext> static auto format( Location const& obj, FormatContext& ctx ) {
        return std::format_to( ctx.out(), "{}", to_string( obj ) );
    }
};

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

template <> struct std::formatter<Token> {
    constexpr static auto parse( std::format_parse_context& ctx ) { return ctx.begin(); }

    template <typename FormatContext> static auto format( Token const& obj, FormatContext& ctx ) {
        return std::format_to( ctx.out(), "{}", to_string( obj ) );
    }
};
