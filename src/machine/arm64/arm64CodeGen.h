//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/base.h"
#include "arm64_at/visitor.h"
#include "codeGen.h"

class Arm64CodeGen : public CodeGenerator, public arm64_at::Visitor<void> {
  public:
    explicit Arm64CodeGen( Option const& option );
    ~Arm64CodeGen() override = default;

    void generate( CodeGenBase program ) override;

    CodeGenBase run_codegen( tac::Program tac ) override;
    void        generate_output_file( CodeGenBase assembly ) override;

    void visit_Program( arm64_at::Program ast ) override;
    void visit_FunctionDef( arm64_at::FunctionDef ast ) override;
    void visit_Mov( arm64_at::Mov ast ) override;
    void visit_Ret( arm64_at::Ret ast ) override;
    void visit_Unary( arm64_at::Unary ast ) override;

    void visit_Imm( arm64_at::Imm ast ) override;
    void visit_Register( arm64_at::Register ast ) override;
    void visit_Pseudo( arm64_at::Pseudo ast ) override {};
    void visit_Stack( arm64_at::Stack ast ) override;

  private:
    std::string operand( const arm64_at::Operand& op );
    std::string last_string;
};
