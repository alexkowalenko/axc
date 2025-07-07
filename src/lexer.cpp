//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 7/7/2025.
//

#include "lexer.h"

#include <map>
#include <sstream>
#include <string>

#include "exception.h"

const std::map<std::string, TokenType> keywords = {
    { "int", TokenType::INT }, { "return", TokenType::RETURN }, { "void", TokenType::VOID } };

Lexer::Lexer( std::istream const& s ) {
    std::stringstream buffer;
    buffer << s.rdbuf();
    file = buffer.str();
    ptr = file.begin();
};

char Lexer::peek() {
    if ( ptr == file.end() ) {
        return -1;
    }
    return *ptr;
}

char Lexer::get() {
    while ( true ) {
        if ( ptr == file.end() ) {
            return -1;
        }
        char c = *ptr;
        if ( c == ' ' || c == '\t' || c == '\r' ) {
            ++ptr;
            ++pos;
            continue;
        }
        if ( c == '\n' ) {
            ++ptr;
            ++line;
            pos = 0;
            continue;
        }
        if ( c == '/' ) {
            c = *( ptr + 1 );
            if ( c == '/' ) {
                // // comments
                ++ptr;
                while ( c = *( ptr ), c != '\n' ) {
                    ++ptr;
                }
                c = *ptr;
                continue;
            }
            if ( c == '*' ) {
                // /* comments */
                ++ptr;
                do {
                    c = *ptr;
                    if ( c == '*' && *( ptr + 1 ) == '/' ) {
                        ptr += 2;
                        c = *( ptr  );
                        break;
                    }
                    ++ptr;
                } while ( true );
                c = *ptr;
                continue;
            }
        }
        ++ptr;
        return c;
    };
}

Token Lexer::get_identifier( const char c ) {
    std::string identifier( 1, c );
    char        x = peek();
    while ( std::isalnum( x ) ) {
        get();
        identifier.push_back( x );
        x = peek();
    }

    if ( keywords.contains( identifier ) ) {
        return { keywords.at( identifier ), get_location() };
    }
    return { TokenType::IDENTIFIER, get_location(), identifier };
}

Token Lexer::get_number( const char c ) {
    std::string number( 1, c );

    char x = peek();
    while ( std::isdigit( x ) ) {
        get();
        number.push_back( x );
        x = peek();
    }
    if ( std::isalpha( x ) ) {
        throw LexicalException( get_location(), "Invalid digit {:c} in number '{:s}'", x, number );
    }
    return { TokenType::CONSTANT, get_location(), number };
}

Token Lexer::make_token() {
    char const c = get();
    switch ( c ) {
    case -1 :
        return { TokenType::Eof, get_location() };
    case '(' :
        return { TokenType::L_PAREN, get_location() };
    case ')' :
        return { TokenType::R_PAREN, get_location() };
    case '{' :
        return { TokenType::L_BRACE, get_location() };
    case '}' :
        return { TokenType::R_BRACE, get_location() };
    case ';' :
        return { TokenType::SEMICOLON, get_location() };
    default :;
    };
    if ( std::isdigit( c ) ) {
        return get_number( c );
    }
    if ( std::isalpha( c ) ) {
        return get_identifier( c );
    }
    throw LexicalException( get_location(), "Unknown character '{:c}'", c );
}

Token Lexer::get_token() {
    if ( next_token ) {
        auto t = *next_token;
        next_token = std::nullopt;
        return t;
    }

    auto token = make_token();
    return token;
}

Token const& Lexer::peek_token() {
    if ( next_token ) {
        return *next_token;
    }
    next_token = get_token();
    return *next_token;
}
