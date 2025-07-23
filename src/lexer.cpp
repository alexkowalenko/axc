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
                        c = *( ptr );
                        break;
                    }
                    ++ptr;
                } while ( true );
                c = *ptr;
                continue;
            }
            // otherwise return /
            ++ptr;
            return '/';
        }
        ++ptr;
        return c;
    };
}

Token Lexer::get_identifier( const char c ) {
    std::string identifier( 1, c );
    char        x = peek();
    while ( std::isalnum( x ) || x == '_' ) {
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
    case '+' : {
        if ( peek() == '+' ) {
            get();
            return { TokenType::INCREMENT, get_location() };
        }
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_PLUS, get_location() };
        }
        return { TokenType::PLUS, get_location() };
    }
    case '-' :
        if ( peek() == '-' ) {
            get();
            return { TokenType::DECREMENT, get_location() };
        }
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_MINUS, get_location() };
        }
        return { TokenType::DASH, get_location() };
    case '*' : {
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_ASTERIX, get_location() };
        }
        return { TokenType::ASTÉRIX, get_location() };
    }
    case '%' :
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_PERCENT, get_location() };
        }
        return { TokenType::PERCENT, get_location() };
    case '/' : {
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_SLASH, get_location() };
        }
        return { TokenType::SLASH, get_location() };
    }
    case '~' :
        return { TokenType::TILDE, get_location() };
    case '&' :
        if ( peek() == '&' ) {
            get();
            return { TokenType::LOGICAL_AND, get_location() };
        }
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_AND, get_location() };
        }
        return { TokenType::AMPERSAND, get_location() };
    case '|' :
        if ( peek() == '|' ) {
            get();
            return { TokenType::LOGICAL_OR, get_location() };
        }
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_OR, get_location() };
        }
        return { TokenType::PIPE, get_location() };
    case '^' : {
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPOUND_XOR, get_location() };
        }
        return { TokenType::CARET, get_location() };
    }
    case '!' :
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPARISON_NOT, get_location() };
        }
        return { TokenType::EXCLAMATION, get_location() };
    case '<' :
        if ( peek() == '<' ) {
            get();
            if ( peek() == '=' ) {
                get();
                return { TokenType::COMPOUND_LEFT_SHIFT, get_location() };
            }
            return { TokenType::LEFT_SHIFT, get_location() };
        }
        if ( peek() == '=' ) {
            get();
            return { TokenType::LESS_EQUALS, get_location() };
        }
        return { TokenType::LESS, get_location() };
    case '>' :
        if ( peek() == '>' ) {
            get();
            if ( peek() == '=' ) {
                get();
                return { TokenType::COMPOUND_RIGHT_SHIFT, get_location() };
            }
            return { TokenType::RIGHT_SHIFT, get_location() };
        }
        if ( peek() == '=' ) {
            get();
            return { TokenType::GREATER_EQUALS, get_location() };
        }
        return { TokenType::GREATER, get_location() };
    case '=' : {
        if ( peek() == '=' ) {
            get();
            return { TokenType::COMPARISON_EQUALS, get_location() };
        }
        return { TokenType::EQUALS, get_location() };
    }
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
