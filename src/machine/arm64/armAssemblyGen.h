//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/program.h"
#include "tac/program.h"

class ARMAssemblyGen {
public:
    ARMAssemblyGen() = default;
    ~ARMAssemblyGen() = default;

    arm64_at::Program generate( const tac::Program atac );
};
