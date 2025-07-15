//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 12/7/2025.
//

#pragma once

#include <map>

#include "at/visitor.h"
#include "at/base.h"

class AssemblyFilterPseudo : public at::Visitor<void> {
public:
    AssemblyFilterPseudo() = default;
    ~AssemblyFilterPseudo() override = default;

    void filter( at::Program program );
    int  get_number_stack_locations();

    void visit_Program( const at::Program ast ) override;
    void visit_FunctionDef( const at::FunctionDef ast ) override;
    void visit_Mov( const at::Mov ast ) override;
    void visit_Unary( const at::Unary ast ) override;
    void visit_Binary( const at::Binary ast ) override;
    void visit_Idiv( const at::Idiv ast ) override;
    void visit_Cdq( const at::Cdq ast ) override {};
    void visit_AllocateStack( const at::AllocateStack ast ) override {};
    void visit_Ret( const at::Ret ast ) override {};
    // Operands
    void visit_Imm( const at::Imm ast ) override {};
    void visit_Register( const at::Register ast ) override {};
    void visit_Stack( const at::Stack ast ) override {};
    void visit_Pseudo( const at::Pseudo ast ) override{};

private:
    at::Operand operand( const at::Operand& op );

    std::map<std::string, int> stack_location_map;
    int next_stack_location { 0 };
    static constexpr int stack_increment { -4 };
};