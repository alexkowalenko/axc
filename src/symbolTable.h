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

// Symbol table - at the moment a map from string to string, and also maker of temporary names.
class SymbolTable {
  public:
    SymbolTable() = default;
    ~SymbolTable() = default;

    inline void put( std::string const& name, std::string const& value ) { table[ name ] = value; };
    [[nodiscard]] std::optional<std::string> find( const std::string& name ) const;

    std::string temp_name( std::string_view basename = "temp" );

  private:
    std::map<std::string, std::string> table;
    std::int32_t                       temp_counter = 0;
};
