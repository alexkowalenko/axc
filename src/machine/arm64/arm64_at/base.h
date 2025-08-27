//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "codeGen.h"
#include "token.h"

namespace arm64_at {

enum class UnaryOpType { NEG, NOT };

enum class BinaryOpType { ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, SHL, SHR };

enum class RegisterName {
    X0 = 0,
    X9 = 9,
    X10 = 10,
    X11 = 11,
    X12 = 12,
    XZR,
};

constexpr std::string to_string( const RegisterName rn ) {
    switch ( rn ) {
    case RegisterName::X0 :
        return "x0";
    case RegisterName::X9 :
        return "x9";
    case RegisterName::X10 :
        return "x10";
    case RegisterName::X11 :
        return "x11";
    case RegisterName::X12 :
        return "x12";
    case RegisterName::XZR :
        return "xzr";
    }
}

class Base : public CodeGenBase_ {
  public:
    explicit Base( const Location loc ) : location( loc ) {}
    ~Base() override = default;

    Location location;
};

} // namespace arm64_at