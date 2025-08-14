//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "codeGen.h"
#include "token.h"

namespace arm64_at {

enum class UnaryOpType { NEG, NOT };

enum class RegisterName {
    X0 = 0,
    X9 = 9,
    X10 = 10,
    XZR,
};

constexpr std::string to_string( const RegisterName rn ) {
    switch ( rn ) {
    case RegisterName::X0 :
        return "x0";
    case RegisterName::X9 :
        return "x9";
    case RegisterName::X10 :
        return "x10";
    case RegisterName::XZR :
        return "xzr";
    }
}

class Base : public CodeGenBase_ {
  public:
    explicit Base( const Location loc ) : location( loc ) {}
    ~Base() override = default;

    Location location;
};

class Mov_;
using Mov = std::shared_ptr<Mov_>;

class Load_;
using Load = std::shared_ptr<Load_>;

class Store_;
using Store = std::shared_ptr<Store_>;

class Ret_;
using Ret = std::shared_ptr<Ret_>;

class Unary_;
using Unary = std::shared_ptr<Unary_>;

class AllocateStack_;
using AllocateStack = std::shared_ptr<AllocateStack_>;

class DeallocateStack_;
using DeallocateStack = std::shared_ptr<DeallocateStack_>;

using Instruction = std::variant<Mov, Load, Store, Ret, Unary, AllocateStack, DeallocateStack>;

class Imm_;
using Imm = std::shared_ptr<Imm_>;

class Register_;
using Register = std::shared_ptr<Register_>;

class Pseudo_;
using Pseudo = std::shared_ptr<Pseudo_>;

class Stack_;
using Stack = std::shared_ptr<Stack_>;

using Operand = std::variant<Imm, Register, Pseudo, Stack>;

} // namespace arm64_at