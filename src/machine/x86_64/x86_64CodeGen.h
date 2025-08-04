//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/7/2025.
//

#pragma once

#include "codeGen.h"
#include "x86_at/base.h"
#include "x86_at/visitor.h"

class X86_64CodeGen : public CodeGenerator, public x86_at::Visitor<void> {
  public:
    X86_64CodeGen( Option const& option );
    ~X86_64CodeGen() override = default;

    void generate( CodeGenBase program ) override;

    CodeGenBase run_codegen( tac::Program tac ) override;
    void        generate_output_file( CodeGenBase assembly ) override;

    void visit_Program( const x86_at::Program ast ) override;
    void visit_FunctionDef( const x86_at::FunctionDef ast ) override;
    void visit_Mov( const x86_at::Mov ast ) override;
    void visit_Unary( const x86_at::Unary ast ) override;
    void visit_AllocateStack( const x86_at::AllocateStack ast ) override;
    void visit_DeallocateStack( const x86_at::DeallocateStack ast ) override;
    void visit_Push( const x86_at::Push ast ) override;
    void visit_Call( const x86_at::Call ast ) override;
    void visit_Ret( const x86_at::Ret ast ) override;
    void visit_Binary( const x86_at::Binary ast ) override;
    void visit_Idiv( const x86_at::Idiv ast ) override;
    void visit_Cmp( const x86_at::Cmp ast ) override;
    void visit_Cdq( const x86_at::Cdq ast ) override;
    void visit_Jump( const x86_at::Jump ast ) override;
    void visit_JumpCC( const x86_at::JumpCC ast ) override;
    void visit_SetCC( const x86_at::SetCC ast ) override;
    void visit_Label( const x86_at::Label ast ) override;


    void visit_Imm( const x86_at::Imm ast ) override;
    void visit_Register( const x86_at::Register ast ) override;
    void visit_Pseudo( const x86_at::Pseudo ast ) override;
    void visit_Stack( const x86_at::Stack ast ) override;

  private:
    std::string operand( const x86_at::Operand& op );

    std::string local_prefix;
    std::string last_string;
};
