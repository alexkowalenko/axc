//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/7/2025.
//

#pragma once

#include "../at/base.h"
#include "../codeGen.h"

class X86_64CodeGen : public CodeGenerator, public at::Visitor<void> {
  public:
    X86_64CodeGen( Option const& option );
    ~X86_64CodeGen() override = default;

    void generate( at::Program program ) override;

    void visit_Program( const at::Program ast ) override;
    void visit_FunctionDef( const at::FunctionDef ast ) override;
    void visit_Mov( const at::Mov ast ) override;
    void visit_Unary( const at::Unary ast ) override;
    void visit_AllocateStack( const at::AllocateStack ast ) override;
    void visit_Ret( const at::Ret ast ) override;
    void visit_Binary( const at::Binary ast ) override;
    void visit_Idiv( const at::Idiv ast ) override;
    void visit_Cmp( const at::Cmp ast ) override;
    void visit_Cdq( const at::Cdq ast ) override;
    void visit_Jump( const at::Jump ast ) override;
    void visit_JumpCC( const at::JumpCC ast ) override;
    void visit_SetCC( const at::SetCC ast ) override;
    void visit_Label( const at::Label ast ) override;

    void visit_Imm( const at::Imm ast ) override;
    void visit_Register( const at::Register ast ) override;
    void visit_Pseudo( const at::Pseudo ast ) override;
    void visit_Stack( const at::Stack ast ) override;

  private:
    std::string operand( const at::Operand& op );

    std::string local_prefix;
    std::string last_string;
};
