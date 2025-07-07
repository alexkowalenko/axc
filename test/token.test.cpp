//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 7/7/2025.
//

#include <gtest/gtest.h>

#include <token.h>

TEST( TokenType, Basic ) { // NOLINT
    EXPECT_EQ( to_string( TokenType::CONSTANT ), "<constant>" );

    Token const t( TokenType::IDENTIFIER, Location( 0, 0 ), "FORXYZ" );
    EXPECT_EQ( to_string( t ), "<id: FORXYZ>" );
}

TEST( Location, Basic ) { // NOLINT
    EXPECT_EQ( to_string( Location { 0, 0 } ), "[0,0]" );
}