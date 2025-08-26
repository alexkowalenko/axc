//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "codeGen.h"
#include "token.h"

namespace x86_at {

class Base : public CodeGenBase_ {
  public:
    explicit Base( const Location loc ) : location( loc ) {}
    ~Base() override = default;

    Location location;
};

enum class UnaryOpType { NEG, NOT };

enum class BinaryOpType {
    ADD,
    SUB,
    MUL,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
};

enum class CondCode { E, NE, G, GE, L, LE };

enum class RegisterName {
    AX,
    CX,
    DX,
    DI,
    SI,
    R8,
    R9,
    R10,
    R11,
};

constexpr std::string to_string( const RegisterName rn ) {
    switch ( rn ) {
    case RegisterName::AX :
        return "A";
    case RegisterName::CX :
        return "C";
    case RegisterName::DX :
        return "D";
    case RegisterName::DI :
        return "DI";
    case RegisterName::SI :
        return "SI";
    case RegisterName::R8 :
        return "R8";
    case RegisterName::R9 :
        return "R9";
    case RegisterName::R10 :
        return "R10";
    case RegisterName::R11 :
        return "R11";
    }
}

enum class RegisterSize {
    Qword, // 64 bits
    Long,
    Byte,
};

constexpr std::string to_string( const RegisterSize rs ) {
    switch ( rs ) {
    case RegisterSize::Qword :
        return "R";
    case RegisterSize::Long :
        return "E";
    case RegisterSize::Byte :
        return "L";
    }
}

} // namespace x86_at
