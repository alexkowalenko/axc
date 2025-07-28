//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 28/7/2025.
//

#pragma once

#include "arm64_at/visitor.h"

#include <string>

class PrinterARM64 : public arm64_at::Visitor<std::string> {
public:
    PrinterARM64() = default;
    ~PrinterARM64() override = default;

    std::string print( const arm64_at::Program ast );

    std::string visit_Program( const arm64_at::Program ast ) override;
};
