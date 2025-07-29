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

enum class RegisterName {
    X0,
    XZR,
};

constexpr std::string to_string( const RegisterName rn ) {
    switch ( rn ) {
    case RegisterName::X0 :
        return "x0";
    case RegisterName::XZR :
        return "xzr";
    }
}

class Base : public CodeGenBase_ {
public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    ~Base() override = default;

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