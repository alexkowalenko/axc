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
#include <print>

std::string to_string( Symbol const& s ) {
    return std::format( "Symbol({} type:{} storage:{} global:{})", s.name, to_string( s.type ), to_string( s.storage ),
                        s.global );
}

AssemblyType to_assembly_type( Type const& t ) {
    switch ( t ) {
    case Type::INT :
        return AssemblyType::Longword;
    case Type::LONG :
        return AssemblyType::Quadword;
    default :
        return AssemblyType::Longword;
    }
}

std::string SymbolTable::temp_name( std::string_view basename ) {
    return std::format( "{}.{}", basename, temp_counter++ );
}

std::optional<Symbol> SymbolTable::find( const std::string& name ) const {
    if ( table.contains( name ) ) {
        return table.at( name );
    }
    return std::nullopt;
}

bool SymbolTable::contains( const std::string& name ) const {
    return table.contains( name );
}

void SymbolTable::copy( SymbolTable& other ) {
    table.insert( other.table.begin(), other.table.end() );
}

void SymbolTable::reset_current_block() {
    for ( auto& [ _, symbol ] : table ) {
        symbol.current_scope = false;
    }
}

void SymbolTable::dump() {
    for ( auto& [ name, symbol ] : table ) {
        std::println( "{}: {} ", name, to_string( symbol ) );
    }
}
