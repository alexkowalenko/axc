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

#include "token.h"
#include "codeGen.h"

namespace x86_at {

class Base : public CodeGenBase_ {
  public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    ~Base() override = default;

    Location location;
};

enum class UnaryOpType { NEG, NOT };

enum class BinaryOpType {
    ADD,
    SUB,
    MUL,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
};

enum class CondCode { E, NE, G, GE, L, LE };

enum class RegisterName {
    AX,
    CX,
    DX,
    R10,
    R11,
};

constexpr std::string to_string( const RegisterName rn ) {
    switch ( rn ) {
    case RegisterName::AX :
        return "A";
    case RegisterName::CX :
        return "C";
    case RegisterName::DX :
        return "D";
    case RegisterName::R10 :
        return "R10";
    case RegisterName::R11 :
        return "R11";
    }
}

enum class RegisterSize {
    Long,
    Byte,
};

constexpr std::string to_string( const RegisterSize rs ) {
    switch ( rs ) {
    case RegisterSize::Long :
        return "E";
    case RegisterSize::Byte :
        return "L";
    }
}

class Mov_;
using Mov = std::shared_ptr<Mov_>;

class Unary_;
using Unary = std::shared_ptr<Unary_>;

class Binary_;
using Binary = std::shared_ptr<Binary_>;

class Cmp_;
using Cmp = std::shared_ptr<Cmp_>;

class Idiv_;
using Idiv = std::shared_ptr<Idiv_>;

class Cdq_;
using Cdq = std::shared_ptr<Cdq_>;

class Jump_;
using Jump = std::shared_ptr<Jump_>;

class JumpCC_;
using JumpCC = std::shared_ptr<JumpCC_>;

class SetCC_;
using SetCC = std::shared_ptr<SetCC_>;

class Label_;
using Label = std::shared_ptr<Label_>;

class AllocateStack_;
using AllocateStack = std::shared_ptr<AllocateStack_>;

class Ret_;
using Ret = std::shared_ptr<Ret_>;

using Instruction = std::variant<Mov, Unary, Binary, Cmp, Idiv, Cdq, Jump, JumpCC, SetCC, Label, AllocateStack, Ret>;

class Imm_;
using Imm = std::shared_ptr<Imm_>;

class Register_;
using Register = std::shared_ptr<Register_>;

class Pseudo_;
using Pseudo = std::shared_ptr<Pseudo_>;

class Stack_;
using Stack = std::shared_ptr<Stack_>;

using Operand = std::variant<Imm, Register, Pseudo, Stack>;
} // namespace at
