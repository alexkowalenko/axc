//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#include "at/base.h"
#include "at/visitor.h"

class AssemblyFixInstruct : public at::Visitor<void> {
  public:
    AssemblyFixInstruct() = default;
    ~AssemblyFixInstruct() override = default;

    void set_number_stack_locations( const int number_stack_locations ) {
        this->number_stack_locations = number_stack_locations;
    }

    void filter( at::Program program );

  public:
    void visit_Program( const at::Program ast ) override;
    void visit_FunctionDef( const at::FunctionDef ast ) override;

    void visit_Mov( const at::Mov ast ) override {};
    void visit_Unary( const at::Unary ast ) override {};
    void visit_AllocateStack( const at::AllocateStack ast ) override {};
    void visit_Ret( const at::Ret ast ) override {};
    void visit_Imm( const at::Imm ast ) override {};
    void visit_Register( const at::Register ast ) override {};
    void visit_Pseudo( const at::Pseudo ast ) override {};
    void visit_Stack( const at::Stack ast ) override {};

  private:
    int                  number_stack_locations { 0 };
    static constexpr int stack_increment { 4 };
};