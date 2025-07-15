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

enum class UnaryOpType {
    Negate,
    Complement,
};

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
};

class Base {
  public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    virtual ~Base() = default;

    Location location;
};

class Return_;
using Return = std::shared_ptr<Return_>;

class Unary_;
using Unary = std::shared_ptr<Unary_>;

class Binary_;
using Binary = std::shared_ptr<Binary_>;

using Instruction = std::variant<Return, Unary, Binary>;

class Constant_;
using Constant = std::shared_ptr<Constant_>;

class Variable_;
using Variable = std::shared_ptr<Variable_>;

using Value = std::variant<Constant, Variable>;
} // namespace tac