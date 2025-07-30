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

class Symbol {
  public:
    std::string name;
    Linkage     linkage { Linkage::None };
    bool        current_scope { false };
};
