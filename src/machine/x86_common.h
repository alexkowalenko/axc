//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 17/7/2025.
//

#pragma once

#include "../at/base.h"

inline std::string cond_code( at::CondCode code ) {
    switch ( code ) {
    case at::CondCode::E :
        return "e";
    case at::CondCode::NE :
        return "ne";
    case at::CondCode::G :
        return "g";
    case at::CondCode::GE :
        return "ge";
    case at::CondCode::L :
        return "l";
    case at::CondCode::LE :
        return "le";
    default :
        return "?";
    }
}
