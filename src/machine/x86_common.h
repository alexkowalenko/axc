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

template <typename T, typename... Args>
constexpr std::shared_ptr<T> make_AT( const std::shared_ptr<at::Base> b, const Args... args ) {
    return std::make_shared<T>( b->location, args... );
}

constexpr at::Register mk_reg( const std::shared_ptr<at::Base> b, const std::string_view name ) {
    auto reg = make_AT<at::Register_>( b );
    reg->reg = name;
    return reg;
}
