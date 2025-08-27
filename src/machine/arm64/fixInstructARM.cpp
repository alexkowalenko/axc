//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 14/8/2025.
//

#include "fixInstructARM.h"

#include "arm64_at/includes.h"
#include "common.h"

FixInstructARM::FixInstructARM() {
    // temp registers
    x9 = std::make_shared<arm64_at::Register_>( Location(), arm64_at::RegisterName::X9 );
    x10 = std::make_shared<arm64_at::Register_>( Location(), arm64_at::RegisterName::X10 );
    x11 = std::make_shared<arm64_at::Register_>( Location(), arm64_at::RegisterName::X11 );
}

void FixInstructARM::filter( arm64_at::Program program ) {
    program->accept( this );
}

void FixInstructARM::visit_Program( arm64_at::Program ast ) {
    ast->function->accept( this );
}

void FixInstructARM::visit_FunctionDef( arm64_at::FunctionDef ast ) {
    current_instructions.clear();

    if ( ast->stack_size != 0 ) {
        auto allocate = mk_node<arm64_at::AllocateStack_>( ast, ast->stack_size );
        current_instructions.emplace_back( allocate );
    }

    for ( auto const& instr : ast->instructions ) {
        std::visit( overloaded { [ this ]( arm64_at::Mov v ) -> void { return v->accept( this ); },
                                 [ this ]( arm64_at::Load l ) -> void { return l->accept( this ); },
                                 [ this ]( arm64_at::Store s ) -> void { return s->accept( this ); },
                                 [ this ]( arm64_at::Ret r ) -> void { return r->accept( this ); },
                                 [ this ]( arm64_at::Unary u ) -> void { return u->accept( this ); },
                                 [ this ]( arm64_at::Binary b ) -> void { return b->accept( this ); },
                                 [ this ]( arm64_at::AllocateStack a ) -> void { return a->accept( this ); },
                                 [ this ]( arm64_at::DeallocateStack d ) -> void { return d->accept( this ); },
                                 [ this ]( arm64_at::Branch b ) -> void { current_instructions.emplace_back( b ); },
                                 [ this ]( arm64_at::BranchCC b ) -> void { current_instructions.emplace_back( b ); },
                                 [ this ]( arm64_at::Label l ) -> void { current_instructions.emplace_back( l ); },
                                 [ this ]( arm64_at::Cmp c ) -> void { return c->accept( this ); },
                                 [ this ]( arm64_at::Cset c ) -> void { return c->accept( this ); } },
                    instr );
    }
    ast->instructions = current_instructions;
}

void FixInstructARM::visit_Mov( arm64_at::Mov ast ) {
    if ( std::holds_alternative<arm64_at::Stack>( ast->src ) ) {
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->src, ast->dst ) );
        return;
    }
    if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {
        auto mov = mk_node<arm64_at::Mov_>( ast, ast->src, x9 );
        current_instructions.emplace_back( mov );
        current_instructions.emplace_back( mk_node<arm64_at::Store_>( ast, x9, ast->dst ) );
        return;
    }
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Load( arm64_at::Load ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Store( arm64_at::Store ast ) {
    current_instructions.emplace_back( ast );
}

arm64_at::Operand FixInstructARM::fix_operand( const HasLocation auto b, arm64_at::Operand operand,
                                               arm64_at::Register& reg ) {
    if ( std::holds_alternative<arm64_at::Stack>( operand ) ) {
        // If stack load into register
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( b, operand, reg ) );
        return reg;
    } else if ( std::holds_alternative<arm64_at::Imm>( operand ) ) {
        // If immediate value, move into register
        current_instructions.emplace_back( mk_node<arm64_at::Mov_>( b, operand, reg ) );
        return reg;
    } else {
        return operand;
    }
}

void FixInstructARM::visit_Unary( arm64_at::Unary ast ) {
    arm64_at::Operand dst;

    arm64_at::Operand src = fix_operand( ast, ast->src, x9 );

    // If destination stack store into register
    if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {
        dst = x10;
    }

    // Perform the operation with modified src, dst
    current_instructions.emplace_back( mk_node<arm64_at::Unary_>( ast, ast->op, dst, src ) );

    // If destination stack store back to stack
    if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {

        current_instructions.emplace_back( mk_node<arm64_at::Store_>( ast, x10, ast->dst ) );
    }
}

void FixInstructARM::visit_Binary( arm64_at::Binary ast ) {
    arm64_at::Operand dst;

    arm64_at::Operand src1 = fix_operand( ast, ast->src1, x9 );
    arm64_at::Operand src2;
    if ( ast->op == arm64_at::BinaryOpType::ADD || ast->op == arm64_at::BinaryOpType::SUB ) {
        // Only move second operand to register if it's a stack, and use add/sub which supports immediate < 4096 in
        // src2
        if ( std::holds_alternative<arm64_at::Stack>( ast->src2 ) ) {
            // If stack load into register
            current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->src2, x10 ) );
            src2 = x10;
        } else if ( std::holds_alternative<arm64_at::Imm>( ast->src2 ) ) {
            // If immediate value, move into register
            current_instructions.emplace_back( mk_node<arm64_at::Mov_>( ast, ast->src2, x10 ) );
            src2 = x10;
        } else {
            src2 = ast->src2;
        }
    } else {
        src2 = fix_operand( ast, ast->src2, x10 );
    }

    // If destination stack store into register
    if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {
        dst = x11;
    }

    // Perform the operation with modified src, dst
    current_instructions.emplace_back( mk_node<arm64_at::Binary_>( ast, ast->op, dst, src1, src2 ) );

    // If destination stack store back to stack
    if ( std::holds_alternative<arm64_at::Stack>( ast->dst ) ) {
        current_instructions.emplace_back( mk_node<arm64_at::Store_>( ast, x11, ast->dst ) );
    }
}

void FixInstructARM::visit_AllocateStack( arm64_at::AllocateStack ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_DeallocateStack( arm64_at::DeallocateStack ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Ret( arm64_at::Ret ast ) {
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Cmp( arm64_at::Cmp ast ) {
    if ( std::holds_alternative<arm64_at::Stack>( ast->operand1 ) ) {
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->operand1, x9 ) );
        ast->operand1 = x9;
    }
    if ( std::holds_alternative<arm64_at::Stack>( ast->operand2 ) ) {
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->operand2, x10 ) );
        ast->operand2 = x10;
    } else if ( std::holds_alternative<arm64_at::Imm>( ast->operand2 ) ) {
        // If immediate value, move into register
        current_instructions.emplace_back( mk_node<arm64_at::Mov_>( ast, ast->operand2, x10 ) );
        ast->operand2 = x10;
    }
    current_instructions.emplace_back( ast );
}

void FixInstructARM::visit_Cset( arm64_at::Cset ast ) {
    bool              put_back = false;
    arm64_at::Operand former = ast->operand;
    if ( std::holds_alternative<arm64_at::Stack>( ast->operand ) ) {
        current_instructions.emplace_back( mk_node<arm64_at::Load_>( ast, ast->operand, x9 ) );
        ast->operand = x9;
        put_back = true;
    }
    current_instructions.emplace_back( ast );
    if ( put_back ) {
        current_instructions.emplace_back( mk_node<arm64_at::Store_>( ast, x9, former ) );
    }
}
