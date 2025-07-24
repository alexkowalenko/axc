//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 7/7/2025.
//

#pragma once

#include <istream>
#include <optional>
#include <string>
#include <deque>

#include "token.h"

class Lexer {
  public:
    explicit Lexer( std::istream const& s );
    ~Lexer() = default;

    Token        get_token();
    Token const& peek_token(size_t offset = 0);

    [[nodiscard]] Location get_location() const { return { line, pos + 1 }; };

  private:
    char get();
    char peek();

    Token get_identifier( char c );
    Token get_number( char c );
    Token make_token();

    std::string           file;
    std::string::iterator ptr;
    size_t                line { 1 };
    size_t                pos { 1 };

    std::deque<Token> next_token;
};
