//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 22/7/2025.
//

#pragma once

#include <map>
#include <string>

#include "symbol.h"

// Symbol table - at the moment a map from string to string, and also maker of temporary names.
class SymbolTable {
  public:
    SymbolTable() = default;
    ~SymbolTable() = default;

    inline void                         put( std::string const& name, Symbol value ) { table[ name ] = value; };
    [[nodiscard]] std::optional<Symbol> find( const std::string& name ) const;
    void                                copy( SymbolTable& other );
    void                                reset_current_block();

    std::string temp_name( std::string_view basename = "temp" );

  private:
    std::map<std::string, Symbol> table;
    static std::int32_t           temp_counter;
};

inline std::int32_t SymbolTable::temp_counter = 0;