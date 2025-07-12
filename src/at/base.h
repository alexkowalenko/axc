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

enum class UnaryOpType {
    NEG,
    NOT
};

class Mov_;
using Mov = std::shared_ptr<Mov_>;

class Unary_;
using Unary = std::shared_ptr<Unary_>;

class AllocateStack_;
using AllocateStack = std::shared_ptr<AllocateStack_>;

class Ret_;
using Ret = std::shared_ptr<Ret_>;

using Instruction = std::variant<Mov, Unary, AllocateStack, Ret>;

class Imm_;
using Imm = std::shared_ptr<Imm_>;

class Register_;
using Register = std::shared_ptr<Register_>;

class Pseudo_;
using Pseudo = std::shared_ptr<Pseudo_>;

class Stack_;
using Stack = std::shared_ptr<Stack_>;

using Operand = std::variant<Imm, Register, Pseudo, Stack>;
}


