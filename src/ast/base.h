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
    std::string ast_label {};
};

class Declaration_;
using Declaration = std::shared_ptr<Declaration_>;

class Null_;
using Null = std::shared_ptr<Null_>;

class Return_;
using Return = std::shared_ptr<Return_>;

class If_;
using If = std::shared_ptr<If_>;

class Constant_;
using Constant = std::shared_ptr<Constant_>;

class UnaryOp_;
using UnaryOp = std::shared_ptr<UnaryOp_>;

class BinaryOp_;
using BinaryOp = std::shared_ptr<BinaryOp_>;

class PostOp_;
using PostOp = std::shared_ptr<PostOp_>;

class Conditional_;
using Conditional = std::shared_ptr<Conditional_>;

class Assign_;
using Assign = std::shared_ptr<Assign_>;

class Var_;
using Var = std::shared_ptr<Var_>;

using Expr = std::variant<Constant, UnaryOp, BinaryOp, PostOp, Conditional, Var, Assign>;

class Goto_;
using Goto = std::shared_ptr<Goto_>;

class Label_;
using Label = std::shared_ptr<Label_>;

class Break_;
using Break = std::shared_ptr<Break_>;

class Continue_;
using Continue = std::shared_ptr<Continue_>;

class While_;
using While = std::shared_ptr<While_>;

class DoWhile_;
using DoWhile = std::shared_ptr<DoWhile_>;

class For_;
using For = std::shared_ptr<For_>;

using ForInit = std::variant<Declaration, Expr>;

class Compound_;
using Compound = std::shared_ptr<Compound_>;

using Statement = std::variant<Return, Expr, If, Null, Goto, Label, Break, Continue, While, DoWhile, For, Compound>;

using BlockItem = std::variant<Statement, Declaration>;

} // namespace ast