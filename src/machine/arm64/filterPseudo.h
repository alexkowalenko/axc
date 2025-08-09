//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 9/8/2025.
//

#pragma once

#include <map>

#include "arm64_at/base.h"
#include "arm64_at/visitor.h"

class FilterPseudo : public arm64_at::Visitor<void> {
  public:
    FilterPseudo() = default;
    ~FilterPseudo() override = default;

    void filter( arm64_at::Program program );

    void visit_Program( arm64_at::Program ast ) override;
    void visit_FunctionDef( arm64_at::FunctionDef ast ) override;
    void visit_Mov( arm64_at::Mov ast ) override;
    void visit_Unary( arm64_at::Unary ast ) override;
    void visit_Ret( arm64_at::Ret ast ) override {};
    // Operands
    void visit_Imm( arm64_at::Imm ast ) override {};
    void visit_Register( arm64_at::Register ast ) override {};
    void visit_Pseudo( arm64_at::Pseudo ast ) override {};
    void visit_Stack( arm64_at::Stack ast ) override {};

  private:
    arm64_at::Operand operand( const arm64_at::Operand& op );
    int               get_number_stack_locations() const;
    void              reset_stack_info();

    std::map<std::string, int> stack_location_map;
    int                        next_stack_location { 0 };
    static constexpr int       stack_increment { -4 };
};
