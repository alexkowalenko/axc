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
#include <vector>

namespace ast {

class Base {
  public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    virtual ~Base() = default;

    Location location;
};

class Declaration_;
using Declaration = std::shared_ptr<Declaration_>;

class Null_;
using Null = std::shared_ptr<Null_>;

class Return_;
using Return = std::shared_ptr<Return_>;

class Constant_;
using Constant = std::shared_ptr<Constant_>;

class UnaryOp_;
using UnaryOp = std::shared_ptr<UnaryOp_>;

class BinaryOp_;
using BinaryOp = std::shared_ptr<BinaryOp_>;

class Assign_;
using Assign = std::shared_ptr<Assign_>;

class Var_;
using Var = std::shared_ptr<Var_>;

using Expr = std::variant<Constant, UnaryOp, BinaryOp, Var, Assign>;

using Statement = std::variant<Return, Expr, Null>;

using BlockItem = std::variant<Statement, Declaration>;

} // namespace ast