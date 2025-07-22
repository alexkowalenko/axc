//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 7/7/2025.
//

#pragma once

#include <exception>
#include <optional>
#include <string>
#include <utility>

#include "token.h"

class Exception : public std::exception {
  public:
    explicit Exception( std::string m ) : msg( std::move( m ) ), loc( std::nullopt ) {};
    Exception( Location const& l, std::string m ) : msg( std::move( m ) ), loc( l ) {};

    template <typename... Args>
    explicit Exception( std::string fmt, const Args&... args )
        : msg( std::vformat( fmt, std::make_format_args( args... ) ) ) {};

    ~Exception() override = default;

    [[nodiscard]] std::string get_message() const {
        if ( loc ) {
            return to_string( *loc ) + " " + msg;
        }
        return msg;
    }

    [[nodiscard]] std::optional<Location> const& get_location() const { return loc; };

  protected:
    explicit Exception( Location const& l ) : loc( l ) {};

    std::string             msg;
    std::optional<Location> loc;
};

class LexicalException : public Exception {
  public:
    explicit LexicalException( std::string m ) : Exception( std::move( m ) ) {};
    LexicalException( Location const& l, std::string m ) : Exception( l, std::move( m ) ) {};

    template <typename... Args>
    LexicalException( Location const& l, std::string fmt, const Args&... args ) : Exception( l ) {
        msg = std::vformat( fmt, std::make_format_args( args... ) );
    };
    ~LexicalException() override = default;
};

class ParseException : public Exception {
  public:
    explicit ParseException( std::string m ) : Exception( std::move( m ) ) {};
    ParseException( Location const& l, std::string m ) : Exception( l, std::move( m ) ) {};

    template <typename... Args>
    ParseException( Location const& l, std::string fmt, const Args&... args ) : Exception( l ) {
        msg = std::vformat( fmt, std::make_format_args( args... ) );
    };
    ~ParseException() override = default;
};

class SemanticException : public Exception {
  public:
    explicit SemanticException( std::string m ) : Exception( std::move( m ) ) {};
    SemanticException( Location const& l, std::string m ) : Exception( l, std::move( m ) ) {};

    template <typename... Args>
    SemanticException( Location const& l, std::string fmt, const Args&... args ) : Exception( l ) {
        msg = std::vformat( fmt, std::make_format_args( args... ) );
    };
    ~SemanticException() override = default;
};

class CodeException : public Exception {
  public:
    explicit CodeException( std::string m ) : Exception( std::move( m ) ) {};
    CodeException( Location const& l, std::string m ) : Exception( l, std::move( m ) ) {};

    template <typename... Args>
    CodeException( Location const& l, std::string fmt, const Args&... args ) : Exception( l ) {
        msg = std::vformat( fmt, std::make_format_args( args... ) );
    };
    ~CodeException() override = default;
};
