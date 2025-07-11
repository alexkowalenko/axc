//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 7/7/2025.
//

#include <print>
#include <string>

#include <gtest/gtest.h>

#include "exception.h"
#include "lexer.h"

struct TestLexer {
    std::string input;
    TokenType   tok;
    std::string atom;
};

void test_Lexer( const std::vector<TestLexer>& tests );

TEST( Lexer, Null ) { // NOLINT
    std::istringstream is( "" );
    Lexer              lex( is );

    auto token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::Eof );
}

TEST( Lexer, Basic ) { // NOLINT
    std::vector<TestLexer> const tests = {
        { "(", TokenType::L_PAREN, "(" },    { ")", TokenType::R_PAREN, ")" },    { "{", TokenType::L_BRACE, "{" },
        { "}", TokenType::R_BRACE, "}" },    { " ;", TokenType::SEMICOLON, ";" }, { "-", TokenType::DASH, ";" },
        { "--", TokenType::DECREMENT, ";" }, { "~", TokenType::TILDE, ";" },

        { "", TokenType::Eof, "" },
    };

    test_Lexer( tests );
}

TEST( Lexer, Newline ) { // NOLINT
    std::string        test = "\n\n";
    std::istringstream is( test );
    Lexer              lex( is );

    auto token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::Eof );
    EXPECT_EQ( lex.get_location().line, 3 );
    EXPECT_EQ( lex.get_location().col, 1 );
}

TEST( Lexer, Line ) { // NOLINT
    std::string        test = "void main() { return 2;}\n";
    std::istringstream is( test );
    Lexer              lex( is );

    auto token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::VOID );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::IDENTIFIER );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::L_PAREN );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::R_PAREN );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::L_BRACE );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::RETURN );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::CONSTANT );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::SEMICOLON );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::R_BRACE );
    token = lex.get_token();
    EXPECT_EQ( token.tok, TokenType::Eof );
}

TEST( Lexer, Constant ) { // NOLINT
    std::vector<TestLexer> const tests = {
        { "1", TokenType::CONSTANT, "1" },
        { "0", TokenType::CONSTANT, "0" },
        { "13445", TokenType::CONSTANT, "13445" },
        { "28987", TokenType::CONSTANT, "28987" },
    };
    test_Lexer( tests );
}

TEST( Lexer, Identifier ) {
    std::vector<TestLexer> const tests = {
        { "      I", TokenType::IDENTIFIER, "I" },
        { "S123456", TokenType::IDENTIFIER, "S123456" },
        { "system", TokenType::IDENTIFIER, "system" },
        { "main", TokenType::IDENTIFIER, "main" },
    };
    test_Lexer( tests );
}

TEST( Lexer, Comments ) {
    std::vector<TestLexer> const tests = { { "// Hello\n 1", TokenType::CONSTANT, "1" },
                                           { "  // Hello\n 1", TokenType::CONSTANT, "1" },
                                           { "/* Hello */ 1", TokenType::CONSTANT, "1" },
                                           { "   /* Hello */1", TokenType::CONSTANT, "1" },
                                           { "   /* Hello \n*/1", TokenType::CONSTANT, "1" } };
    test_Lexer( tests );
}

TEST( Lexer, Keywords ) {
    std::vector<TestLexer> const tests = { { "int", TokenType::INT, "int" },
                                           { "void", TokenType::VOID, "void" },
                                           { "return", TokenType::RETURN, "return" } };
    test_Lexer( tests );
}

void test_Lexer( const std::vector<TestLexer>& tests ) {
    for ( const auto& test : tests ) {
        std::istringstream is( test.input );
        Lexer              lex( is );
        try {
            auto tok = lex.get_token();
            // std::println("token {} wanted {}\n", test.input, to_string(test.tok));
            EXPECT_EQ( tok.tok, test.tok );
            if ( test.tok == TokenType::CONSTANT ) {
                // std::println("     {} = {}->{}\n", test.input, test.atom, tok.value);
                EXPECT_EQ( tok.value, test.atom );
            } else if ( test.tok == TokenType::IDENTIFIER ) {
                // std::println("     {} -> {}\n", test.input, tok.value);
                EXPECT_EQ( tok.value, test.atom );
            }
        } catch ( Exception& e ) {
            FAIL() << "Exception thrown! " << e.get_message() << '\n';
        } catch ( std::exception& e ) {
            FAIL() << "Exception thrown! " << e.what() << '\n';
        }
    }
}
