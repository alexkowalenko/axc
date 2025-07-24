//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 22/7/2025.
//

#include "symbolTable.h"

#include <format>

std::string SymbolTable::temp_name( std::string_view basename ) {
    return std::format( "{}.{}", basename, temp_counter++ );
}

std::optional<Symbol> SymbolTable::find( const std::string& name ) const {
    if ( table.contains( name ) ) {
        return table.at( name );
    }
    return std::nullopt;
}

void SymbolTable::copy( SymbolTable& other ) {
    table.insert( other.table.begin(), other.table.end() );
}

void SymbolTable::reset_current_block() {
    for ( auto& [ _, symbol ] : table ) {
        symbol.current_block = false;
    }
}
