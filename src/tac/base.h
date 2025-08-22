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

class FunctionDef_;
using FunctionDef = std::shared_ptr<FunctionDef_>;

class StaticVariable_;
using StaticVariable = std::shared_ptr<StaticVariable_>;

using TopLevel = std::variant<FunctionDef, StaticVariable>;

class Return_;
using Return = std::shared_ptr<Return_>;

class Unary_;
using Unary = std::shared_ptr<Unary_>;

class Binary_;
using Binary = std::shared_ptr<Binary_>;

class Copy_;
using Copy = std::shared_ptr<Copy_>;

class Jump_;
using Jump = std::shared_ptr<Jump_>;

class JumpIfZero_;
using JumpIfZero = std::shared_ptr<JumpIfZero_>;

class JumpIfNotZero_;
using JumpIfNotZero = std::shared_ptr<JumpIfNotZero_>;

class Label_;
using Label = std::shared_ptr<Label_>;

class FunCall_;
using FunCall = std::shared_ptr<FunCall_>;

using Instruction = std::variant<Return, Unary, Binary, Copy, Jump, JumpIfZero, JumpIfNotZero, Label, FunCall>;

class Constant_;
using Constant = std::shared_ptr<Constant_>;

class Variable_;
using Variable = std::shared_ptr<Variable_>;

using Value = std::variant<Constant, Variable>;
} // namespace tac