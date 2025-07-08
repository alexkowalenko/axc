//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#include "assemblyGen.h"

at::Program AssemblyGen::generate( const ast::Program& p ) {
    auto program = std::make_shared<at::Program_>( p->location );
    program->function = functionDef( p->function );
    return program;
}

at::FunctionDef AssemblyGen::functionDef( const ast::FunctionDef& ast ) {
    auto function = std::make_shared<at::FunctionDef_>( ast->location );
    function->name = ast->name;
    function->instructions = {};
    statement(ast->statement, function->instructions);
    return function;
};

void AssemblyGen::statement( const ast::Statement& ast,  std::vector<at::Instruction> & instructions ) {
    ret(ast->ret, instructions);
};

void AssemblyGen::ret( const ast::Return& ast, std::vector<at::Instruction> & instructions) {
    expr(ast->expr, instructions);
    auto ret = std::make_shared<at::Ret_>( ast->location );
    instructions.push_back( ret );
};

void AssemblyGen::expr( const ast::Expr& ast,  std::vector<at::Instruction>& instructions ) {
    auto mov = std::make_shared<at::Mov_>( ast->location );
    mov->src = constant(ast->constant);
    auto reg = std::make_shared<at::Register_>( ast->location );
    reg->reg = "eax";
    mov->dst = reg;
    instructions.push_back( mov );
};

at::Imm AssemblyGen::constant( const ast::Constant& ast ) {
    auto imm = std::make_shared<at::Imm_>( ast->location );
    imm->value = ast->value;
    return imm;
};