//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 11/7/2025.
//

#pragma once

#include <memory>
#include <string>

template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};

template <typename T>
concept HasLocation = requires( T t ) { t->location; };

template <typename T, typename... Args> constexpr std::shared_ptr<T> mk_node( const HasLocation auto b, Args... args ) {
    return std::make_shared<T>( b->location, args... );
}

enum class StorageClass {
    None = 0,
    Static,
    Extern,
    Parameter,
};

constexpr std::string to_string( const StorageClass storage ) {
    switch ( storage ) {
    case StorageClass::None :
        return "none";
    case StorageClass::Static :
        return "static";
    case StorageClass::Extern :
        return "extern";
    case StorageClass::Parameter :
        return "parameter";
    }
}

enum class AssemblyType {
    Longword,
    Quadword,
};

constexpr std::string to_string( const AssemblyType type ) {
    switch ( type ) {
    case AssemblyType::Longword :
        return "longword";
    case AssemblyType::Quadword :
        return "quadword";
    }
    return "unknown";
}