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

#include "../token.h"

namespace at {
class Base {
public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    virtual ~Base() = default;

    Location location;
};

class Mov_;
using Mov = std::shared_ptr<Mov_>;

class Ret_;
using Ret = std::shared_ptr<Ret_>;

using Instruction = std::variant<Mov, Ret>;

class Imm_;
using Imm = std::shared_ptr<Imm_>;

class Register_;
using Register = std::shared_ptr<Register_>;

using Operand = std::variant<Imm, Register>;
}


