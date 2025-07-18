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

template <typename T>
concept HasLocation = requires( T t ) { t->location; };

template <typename T, typename... Args> constexpr std::shared_ptr<T> mk_node( const HasLocation auto b, Args... args ) {
    return std::make_shared<T>( b->location, args... );
}