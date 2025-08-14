//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 24/7/2025.
//

#pragma once

#include "common.h"

enum class Type { VOID, INT, FUNCTION };

class Symbol {
  public:
    std::string  name;
    StorageClass storage { StorageClass::None };
    Type         type { Type::INT };
    int          number { 0 };
    bool         current_scope { false };
    bool         has_init { false };
    bool         global { false };
};

constexpr std::string to_string( const Type type ) {
    switch ( type ) {
    case Type::VOID :
        return "void";
    case Type::INT :
        return "int";
    case Type::FUNCTION :
        return "function";
    }
}

std::string to_string( Symbol const& s );
