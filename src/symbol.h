//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 24/7/2025.
//

#pragma once

#include "common.h"
#include "type.h"

enum class Initialiser { None, Tentative, Final };

class Symbol {
  public:
    std::string  name;
    StorageClass storage { StorageClass::None };
    Type         type { Type::INT };
    FunctionType function_type;
    int          number { 0 };
    bool         current_scope { false };
    Initialiser  initaliser { Initialiser::None };
    bool         global { false };
};

std::string to_string( Symbol const& s );
