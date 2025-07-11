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

using Instruction = std::variant<Return, Unary>;

class Constant_;
using Constant = std::shared_ptr<Constant_>;

class Variable_;
using Variable = std::shared_ptr<Variable_>;

using Value = std::variant<Constant, Variable>;
}