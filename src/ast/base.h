//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include <cstdint>

#include "../token.h"

#include <variant>

namespace ast {

class Base {
  public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    virtual ~Base() = default;

    Location location;
};

class Constant_;
using Constant = std::shared_ptr<Constant_>;

class UnaryOp_;
using UnaryOp = std::shared_ptr<UnaryOp_>;

class BinaryOp_;
using BinaryOp = std::shared_ptr<BinaryOp_>;

using Expr = std::variant<Constant, UnaryOp, BinaryOp>;

} // namespace ast