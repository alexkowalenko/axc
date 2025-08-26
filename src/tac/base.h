//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#include <memory>
#include <variant>
#include <vector>

#include "../token.h"

#pragma once

namespace tac {

enum class UnaryOpType { Negate, Complement, Not };

enum class BinaryOpType {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    ShiftLeft,
    ShiftRight,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    And,
    Or
};

class Base {
  public:
    explicit Base( const Location loc ) : location( loc ) {}
    virtual ~Base() = default;

    Location location;
};

} // namespace tac