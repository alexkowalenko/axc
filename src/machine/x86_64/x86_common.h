//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 17/7/2025.
//

#pragma once

#include "x86_at/base.h"

inline std::string cond_code( x86_at::CondCode code ) {
    switch ( code ) {
    case x86_at::CondCode::E :
        return "e";
    case x86_at::CondCode::NE :
        return "ne";
    case x86_at::CondCode::G :
        return "g";
    case x86_at::CondCode::GE :
        return "ge";
    case x86_at::CondCode::L :
        return "l";
    case x86_at::CondCode::LE :
        return "le";
    default :
        return "?";
    }
}

std::string assemble_reg( x86_at::Register r );
