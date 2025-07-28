//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#include "printerARM64.h"

#include "arm64_at/includes.h"

std::string PrinterARM64::print( const arm64_at::Program ast ) {
    return ast->accept( this );
}

std::string PrinterARM64::visit_Program( const arm64_at::Program ast ) {
    return "Program";
}
