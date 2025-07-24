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
    QUESTION,
    COLON,

    INCREMENT,
    DECREMENT,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    LOGICAL_AND,
    LOGICAL_OR,
    COMPARISON_EQUALS,
    COMPARISON_NOT,
    LESS_EQUALS,
    GREATER_EQUALS,
    COMPOUND_PLUS,
    COMPOUND_MINUS,
    COMPOUND_ASTERIX,
    COMPOUND_SLASH,
    COMPOUND_PERCENT,
    COMPOUND_AND,
    COMPOUND_OR,
    COMPOUND_XOR,
    COMPOUND_LEFT_SHIFT,
    COMPOUND_RIGHT_SHIFT,

    IDENTIFIER,
    CONSTANT,

    // Keywords
    IF,
    ELSE,
    GOTO,
    INT,
    RETURN,
    VOID
};

const char* to_string( const TokenType l );

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

std::string to_string( Token const& t );

template <> struct std::formatter<Token> {
    constexpr static auto parse( std::format_parse_context& ctx ) { return ctx.begin(); }

    template <typename FormatContext> static auto format( Token const& obj, FormatContext& ctx ) {
        return std::format_to( ctx.out(), "{}", to_string( obj ) );
    }
};
