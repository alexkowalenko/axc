//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#pragma once

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};