//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "assemblyGen.h"

#include <map>

#include "common.h"
#include "spdlog/spdlog.h"
#include "x86_common.h"

#include <complex>

AssemblyGen::AssemblyGen( Option const& option ) : option( option ) {
    zero = std::make_shared<x86_at::Imm_>( Location(), 0 );
    ax = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::AX, x86_at::RegisterSize::Long );
    cx = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::CX, x86_at::RegisterSize::Long );
    dx = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::DX, x86_at::RegisterSize::Long );
    di = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::DI, x86_at::RegisterSize::Long );
    si = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::SI, x86_at::RegisterSize::Long );
    r8 = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::R8, x86_at::RegisterSize::Long );
    r9 = std::make_shared<x86_at::Register_>( Location(), x86_at::RegisterName::R9, x86_at::RegisterSize::Long );

    frame_registers = { di, si, dx, cx, r8, r9 };
}

x86_at::Program AssemblyGen::generate( const tac::Program atac ) {
    auto program = mk_node<x86_at::Program_>( atac );
    for ( const auto& item : atac->top_level ) {
        std::visit( overloaded { [ this, &program ]( tac::FunctionDef funct ) -> void {
                                    const auto fd = functionDef( funct );
                                    program->top_level.push_back( fd );
                                },
                                 [ this, &program ]( tac::StaticVariable s ) -> void {
                                     const auto sv = staticVariable( s );
                                     program->top_level.push_back( sv );
                                 } },
                    item );
    }
    return program;
}

x86_at::StaticVariable AssemblyGen::staticVariable( tac::StaticVariable atac ) {
    return mk_node<x86_at::StaticVariable_>( atac, atac->name, atac->global, atac->init );
}

x86_at::FunctionDef AssemblyGen::functionDef( const tac::FunctionDef atac ) {
    spdlog::debug( "functionDef: {}", atac->name );
    auto function = mk_node<x86_at::FunctionDef_>( atac );
    function->name = atac->name;
    function->global = atac->global;

    int count = 0;
    int stack_count = 16;
    for ( const auto& param : atac->params ) {
        auto p = mk_node<x86_at::Pseudo_>( atac, param );
        if ( count < frame_registers.size() ) {
            auto mov = mk_node<x86_at::Mov_>( atac, frame_registers[ count ], p );
            function->instructions.emplace_back( mov );
        } else {
            auto stack_param = mk_node<x86_at::Stack_>( atac, stack_count );
            auto mov = mk_node<x86_at::Mov_>( atac, stack_param, p );
            function->instructions.emplace_back( mov );
            stack_count += 8; // Increment stack by 8 bytes for each parameter
        }
        count++;
    }
    spdlog::debug( "Arg Count: {}, Stack count: {}", atac->params.size(), stack_count );

    for ( auto instr : atac->instructions ) {
        std::visit( overloaded { [ &function, this ]( tac::Return r ) -> void { ret( r, function->instructions ); },
                                 [ &function, this ]( tac::Unary r ) -> void { unary( r, function->instructions ); },
                                 [ &function, this ]( tac::Binary r ) -> void { binary( r, function->instructions ); },
                                 [ &function ]( tac::Copy r ) -> void { copy( r, function->instructions ); },
                                 [ &function ]( tac::Jump r ) -> void { jump( r, function->instructions ); },
                                 [ &function, this ]( tac::JumpIfZero r ) -> void {
                                     jumpIfZero<tac::JumpIfZero>( r, true, function->instructions );
                                 },
                                 [ &function, this ]( tac::JumpIfNotZero r ) -> void {
                                     jumpIfZero<tac::JumpIfNotZero>( r, false, function->instructions );
                                 },
                                 [ &function ]( tac::Label r ) -> void { label( r, function->instructions ); },
                                 [ &function, this ]( tac::FunCall atac ) -> void {
                                     functionCall( atac, function->instructions );
                                 } },
                    instr );
    }
    return function;
};

void AssemblyGen::ret( const tac::Return atac, std::vector<x86_at::Instruction>& instructions ) const {
    // Mov(value, %eax)
    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->value ), ax );
    instructions.emplace_back( mov );
    // Ret
    auto ret = mk_node<x86_at::Ret_>( atac );
    instructions.emplace_back( ret );
};

void AssemblyGen::unary( const tac::Unary atac, std::vector<x86_at::Instruction>& instructions ) const {
    // Handle not differently
    if ( atac->op == tac::UnaryOpType::Not ) {
        unary_not( atac, instructions );
        return;
    }

    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src ), value( atac->dst ) );
    instructions.emplace_back( mov );
    auto unary = mk_node<x86_at::Unary_>( atac );
    switch ( atac->op ) {
    case tac::UnaryOpType::Complement :
        unary->op = x86_at::UnaryOpType::NOT;
        break;
    case tac::UnaryOpType::Negate :
        unary->op = x86_at::UnaryOpType::NEG;
        break;
    default :
        break;
    }
    unary->operand = value( atac->dst );
    instructions.emplace_back( unary );
};

void AssemblyGen::unary_not( const tac::Unary atac, std::vector<x86_at::Instruction>& instructions ) const {
    // Cmp(Imm(0), src)
    auto cmp = mk_node<x86_at::Cmp_>( atac, zero, value( atac->src ) );
    instructions.emplace_back( cmp );
    // Mov(Imm(0), dst)
    auto mov = mk_node<x86_at::Mov_>( atac, zero, value( atac->dst ) );
    instructions.emplace_back( mov );
    // SetCC(E, dst)
    auto sete = mk_node<x86_at::SetCC_>( atac, x86_at::CondCode::E, value( atac->dst ) );
    instructions.emplace_back( sete );
}

void AssemblyGen::binary( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions ) {
    // Handle divide and mod differently
    if ( atac->op == tac::BinaryOpType::Divide || atac->op == tac::BinaryOpType::Modulo ) {
        idiv( atac, instructions );
        return;
    }

    // Handle relational operations differently
    if ( atac->op == tac::BinaryOpType::Equal || atac->op == tac::BinaryOpType::NotEqual ||
         atac->op == tac::BinaryOpType::Less || atac->op == tac::BinaryOpType::LessEqual ||
         atac->op == tac::BinaryOpType::Greater || atac->op == tac::BinaryOpType::GreaterEqual ) {
        binary_relation( atac, instructions );
        return;
    }

    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src1 ), value( atac->dst ) );
    instructions.emplace_back( mov );
    auto binary = mk_node<x86_at::Binary_>( atac );
    switch ( atac->op ) {
    case tac::BinaryOpType::Add :
        binary->op = x86_at::BinaryOpType::ADD;
        break;
    case tac::BinaryOpType::Subtract :
        binary->op = x86_at::BinaryOpType::SUB;
        break;
    case tac::BinaryOpType::Multiply :
        binary->op = x86_at::BinaryOpType::MUL;
        break;
    case tac::BinaryOpType::BitwiseAnd :
        binary->op = x86_at::BinaryOpType::AND;
        break;
    case tac::BinaryOpType::BitwiseOr :
        binary->op = x86_at::BinaryOpType::OR;
        break;
    case tac::BinaryOpType::BitwiseXor :
        binary->op = x86_at::BinaryOpType::XOR;
        break;
    case tac::BinaryOpType::ShiftLeft :
        binary->op = x86_at::BinaryOpType::SHL;
        break;
    case tac::BinaryOpType::ShiftRight :
        binary->op = x86_at::BinaryOpType::SHR;
        break;
    default :
    }
    binary->operand1 = value( atac->src2 );
    binary->operand2 = value( atac->dst );
    instructions.emplace_back( binary );
}

void AssemblyGen::idiv( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions ) {

    // Mov(src1, Reg(AX))
    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src1 ), ax );
    instructions.emplace_back( mov );

    // Cdq
    instructions.emplace_back( mk_node<x86_at::Cdq_>( atac ) );

    // Idiv(src2)
    auto idiv = mk_node<x86_at::Idiv_>( atac, value( atac->src2 ) );
    instructions.emplace_back( idiv );

    // Mov(Reg(AX), dst)
    mov = mk_node<x86_at::Mov_>( atac );
    if ( atac->op == tac::BinaryOpType::Divide ) {
        mov->src = ax;
    } else {
        // Modulo
        mov->src = dx;
    }
    mov->dst = value( atac->dst );
    instructions.emplace_back( mov );
}

const std::map<tac::BinaryOpType, x86_at::CondCode> condCodeMap = {
    { tac::BinaryOpType::Equal, x86_at::CondCode::E },   { tac::BinaryOpType::NotEqual, x86_at::CondCode::NE },
    { tac::BinaryOpType::Less, x86_at::CondCode::L },    { tac::BinaryOpType::LessEqual, x86_at::CondCode::LE },
    { tac::BinaryOpType::Greater, x86_at::CondCode::G }, { tac::BinaryOpType::GreaterEqual, x86_at::CondCode::GE },
};

void AssemblyGen::binary_relation( const tac::Binary atac, std::vector<x86_at::Instruction>& instructions ) const {
    // Cmp(Imm(0), src)
    auto cmp = mk_node<x86_at::Cmp_>( atac, value( atac->src2 ), value( atac->src1 ) );
    instructions.emplace_back( cmp );
    // Mov(Imm(0), dst)
    auto mov = mk_node<x86_at::Mov_>( atac, zero, value( atac->dst ) );
    instructions.emplace_back( mov );
    // SetCC(condCode, dst)
    auto setcc = mk_node<x86_at::SetCC_>( atac, condCodeMap.at( atac->op ), value( atac->dst ) );
    instructions.emplace_back( setcc );
}

void AssemblyGen::jump( const tac::Jump atac, std::vector<x86_at::Instruction>& instructions ) {
    auto j = mk_node<x86_at::Jump_>( atac, atac->target );
    instructions.emplace_back( j );
}

void AssemblyGen::copy( const tac::Copy atac, std::vector<x86_at::Instruction>& instructions ) {
    auto mov = mk_node<x86_at::Mov_>( atac, value( atac->src ), value( atac->dst ) );
    instructions.emplace_back( mov );
}

void AssemblyGen::label( const tac::Label atac, std::vector<x86_at::Instruction>& instructions ) {
    auto label = mk_node<x86_at::Label_>( atac, atac->name );
    instructions.emplace_back( label );
}

void AssemblyGen::functionCall( const tac::FunCall atac, std::vector<x86_at::Instruction>& instructions ) const {
    spdlog::debug( "Function call: {}", atac->function_name );
    int arg_count = atac->arguments.size();

    int stack_padding = 0;
    int stack_args = 0;
    if ( ( arg_count - 6 ) > 0 ) {
        stack_args = arg_count - 6;
        if ( stack_args % 2 != 0 )
            // If odd number of stack arguments, add padding
            stack_padding = 8;
    }
    spdlog::debug( "Arg count: {}, Stack args: {}, Stack padding: {}", arg_count, stack_args, stack_padding );

    // Fix stack alignment
    if ( stack_padding != 0 ) {
        // Push padding to stack
        auto pad = mk_node<x86_at::AllocateStack_>( atac, stack_padding );
        instructions.emplace_back( pad );
    }

    auto count = 0;
    for ( const auto& arg : atac->arguments ) {
        if ( count < frame_registers.size() ) {
            // Use registers for the first 6 arguments
            auto mov = mk_node<x86_at::Mov_>( atac, value( arg ), frame_registers[ count ] );
            instructions.emplace_back( mov );
        } else {
            break;
        }
        ++count;
    }
    if ( stack_args > 0 ) {
        // If there are more than 6 arguments, we need to push the remaining ones to the stack, in reverse order
        int s = stack_args;
        for ( auto it = atac->arguments.end() - 1; s > 0; --it, --s ) {
            spdlog::debug( "Stack args: {} ", s );
            auto v = value( *it );
            if ( std::holds_alternative<x86_at::Imm>( v ) || std::holds_alternative<x86_at::Register>( v ) ) {
                // If the value is an immediate or register, we can push it to the stack
                auto push = mk_node<x86_at::Push_>( atac, v );
                instructions.emplace_back( push );
            } else {
                // Otherwise, move it to AX, then push it
                auto mov = mk_node<x86_at::Mov_>( atac, v, ax );
                instructions.emplace_back( mov );
                auto push = mk_node<x86_at::Push_>( atac, ax );
                instructions.emplace_back( push );
            }
        }
    }

    // Emit Call
    auto call = mk_node<x86_at::Call_>( atac );
    call->function_name = atac->function_name;
    if ( ( option.system == System::Linux || option.system == System::FreeBSD ) && atac->external ) {
        call->function_name += "@PLT";
    }
    instructions.emplace_back( call );

    // Adjust stack pointer
    auto bytes_to_remove = ( stack_args * 8 + stack_padding ); // Each argument is 8 bytes
    if ( bytes_to_remove != 0 ) {
        auto deallocate = mk_node<x86_at::DeallocateStack_>( atac, bytes_to_remove );
        instructions.emplace_back( deallocate );
    }

    auto dst = value( atac->dst );
    auto mov = mk_node<x86_at::Mov_>( atac, ax, dst );
    instructions.emplace_back( mov );
}

x86_at::Operand AssemblyGen::value( tac::Value atac ) {
    return std::visit( overloaded { []( tac::Constant c ) -> x86_at::Operand { return constant( c ); },
                                    []( tac::Variable v ) -> x86_at::Operand { return pseudo( v ); } },
                       atac );
}

x86_at::Operand AssemblyGen::constant( tac::Constant atac ) {
    return mk_node<x86_at::Imm_>( atac, atac->value );
};

x86_at::Operand AssemblyGen::pseudo( tac::Variable atac ) {
    return mk_node<x86_at::Pseudo_>( atac, atac->name );
}