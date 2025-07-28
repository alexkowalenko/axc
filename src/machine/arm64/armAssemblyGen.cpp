//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#include "armAssemblyGen.h"

#include "common.h"
#include "tac/program.h"

arm64_at::Program ARMAssemblyGen::generate( const tac::Program atac ) {
    auto program = mk_node<arm64_at::Program_>( atac );
    return program;
};