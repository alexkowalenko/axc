//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 8/7/2025.
//

#pragma once

#include <cstdint>

#include "../token.h"

namespace ast {

class Base {
  public:
    explicit Base( Location loc ) : location( std::move( loc ) ) {}
    virtual ~Base() = default;

    Location location;
};

} // namespace ast