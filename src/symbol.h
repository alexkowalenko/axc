//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 24/7/2025.
//

#pragma once

enum class Linkage {
    None,
    Internal, // Only visible within the current translation unit
    External, // Visible across translation units
};

enum class Type {
    VOID,
    INT,
    FUNCTION
};

class Symbol {
  public:
    std::string name;
    Linkage     linkage { Linkage::None };
    Type        type { Type::INT };
    int         number { 0 };
    bool        current_scope { false };
};
