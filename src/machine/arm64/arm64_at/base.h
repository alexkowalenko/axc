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
class Base : public CodeGenBase_ {
public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    ~Base() override = default;

    Location location;
};

}