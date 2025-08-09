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

    void visit_Program( x86_at::Program ast ) override;
    void visit_FunctionDef( x86_at::FunctionDef ast ) override;
    void visit_Mov( x86_at::Mov ast ) override;
    void visit_Unary( x86_at::Unary ast ) override;
    void visit_AllocateStack( x86_at::AllocateStack ast ) override;
    void visit_DeallocateStack( x86_at::DeallocateStack ast ) override;
    void visit_Push( x86_at::Push ast ) override;
    void visit_Call( x86_at::Call ast ) override;
    void visit_Ret( x86_at::Ret ast ) override;
    void visit_Binary( x86_at::Binary ast ) override;
    void visit_Idiv( x86_at::Idiv ast ) override;
    void visit_Cmp( x86_at::Cmp ast ) override;
    void visit_Cdq( x86_at::Cdq ast ) override;
    void visit_Jump( x86_at::Jump ast ) override;
    void visit_JumpCC( x86_at::JumpCC ast ) override;
    void visit_SetCC( x86_at::SetCC ast ) override;
    void visit_Label( x86_at::Label ast ) override;

    void visit_Imm( x86_at::Imm ast ) override;
    void visit_Register( x86_at::Register ast ) override;
    void visit_Pseudo( x86_at::Pseudo ast ) override;
    void visit_Stack( x86_at::Stack ast ) override;

  private:
    std::string operand( const x86_at::Operand& op );
    std::string function_label( std::string_view name ) const;
    std::string jump_label( std::string_view name );

    std::string local_prefix;
    std::string last_string;

    std::string current_function_name;
};
