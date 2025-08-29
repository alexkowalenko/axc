//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 29/8/2025.
//

#pragma once

#include <vector>

enum class Type { VOID, INT, LONG, FUNCTION };

class FunctionType {
  public:
    Type              return_type = Type::VOID;
    std::vector<Type> parameter_types;
};

constexpr std::string to_string( const Type type ) {
    switch ( type ) {
    case Type::VOID :
        return "void";
    case Type::INT :
        return "int";
    case Type::LONG :
        return "long";
    case Type::FUNCTION :
        return "function";
    }
}