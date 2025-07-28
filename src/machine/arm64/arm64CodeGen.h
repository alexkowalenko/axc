//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/visitor.h"
#include "codeGen.h"

class Arm64CodeGen : public CodeGenerator, public arm64_at::Visitor<void> {
  public:
    explicit Arm64CodeGen( Option const& option );
    ~Arm64CodeGen() override = default;

    void generate( CodeGenBase program ) override;

    void        visit_Program( const arm64_at::Program ast ) override;

    CodeGenBase run_codegen( tac::Program tac ) override;
    void        generate_output_file( CodeGenBase assembly ) override;
};
