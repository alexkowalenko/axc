//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "../at/includes.h"
#include "../tac/includes.h"

/// Convert TAC Abstract tree to AT Assembly tree
class AssemblyGen {
  public:
    AssemblyGen();
    ~AssemblyGen() = default;

    at::Program generate( const tac::Program atac );

    at::FunctionDef functionDef( const tac::FunctionDef atac );

    void ret( const tac::Return atac, std::vector<at::Instruction>& instructions );

    void unary( const tac::Unary atac, std::vector<at::Instruction>& instructions );
    void unary_not( const tac::Unary atac, std::vector<at::Instruction>& instructions );

    void                       binary( const tac::Binary atac, std::vector<at::Instruction>& instructions );
    void                       idiv( const tac::Binary atac, std::vector<at::Instruction>& instructions );
    void                       binary_relation( const tac::Binary atac, std::vector<at::Instruction>& instructions );
    void                       jump( const tac::Jump atac, std::vector<at::Instruction>& instructions );
    template <typename T> void jumpIfZero( const T atac, bool zerop, std::vector<at::Instruction>& instructions );
    void                       copy( const tac::Copy atac, std::vector<at::Instruction>& instructions );
    void                       label( const tac::Label atac, std::vector<at::Instruction>& instructions );

    at::Operand value( const tac::Value& atac );
    at::Operand constant( const tac::Constant& atac );
    at::Operand pseudo( tac::Variable atac );

  private:
    at::Imm zero;

    at::Register ax;
    at::Register dx;
};

template <typename T>
void AssemblyGen::jumpIfZero( const T atac, bool zerop, std::vector<at::Instruction>& instructions ) {
    auto zero = std::make_shared<at::Imm_>( atac->location, 0 );
    auto cmp = std::make_shared<at::Cmp_>( atac->location, zero, value( atac->condition ) );
    instructions.push_back( cmp );
    if ( zerop ) {
        auto j = std::make_shared<at::JumpCC_>( atac->location, at::CondCode::E, atac->target );
        instructions.push_back( j );
    } else {
        auto j = std::make_shared<at::JumpCC_>( atac->location, at::CondCode::NE, atac->target );
        instructions.push_back( j );
    }
}
