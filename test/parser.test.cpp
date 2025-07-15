//
// AXC - C Compiler
//
// Copyright (c) 2025.
//

//
// Created by Alex Kowalenko on 14/7/2025.
//

#include <gtest/gtest.h>

#include "exception.h"
#include "parser.h"
#include "printerAST.h"

struct ParseTests {
    std::string input;
    std::string output;
    std::string error;
};

void do_parse_tests( std::vector<ParseTests> const& tests );

TEST( Parser, Constant ) { // NOLINT
    std::vector<ParseTests> tests = {
        { "int main(void) { return 2;}", "int main(void) {return 2;}", "" },
        { "int main(void) { return 0;}", "int main(void) {return 0;}", "" },
        { "int main(void) { return 123456789;}", "int main(void) {return 123456789;}", "" },
    };
    do_parse_tests( tests );
}

TEST( Parser, Parens ) { // NOLINT
    std::vector<ParseTests> tests = {
        { "int main(void) { return (2);}", "int main(void) {return 2;}", "" },
        { "int main(void) { return ((2));}", "int main(void) {return 2;}", "" },
    };
    do_parse_tests( tests );
}

TEST( Parser, Unary ) {
    std::vector<ParseTests> tests = {
        { "int main(void) { return -2;}", "int main(void) {return -2;}", "" },
        { "int main(void) { return ~0;}", "int main(void) {return ~0;}", "" },
        { "int main(void) { return -~1;}", "int main(void) {return -~1;}", "" },
        { "int main(void) { return ~-1;}", "int main(void) {return ~-1;}", "" },
        { "int main(void) { return ~-~1;}", "int main(void) {return ~-~1;}", "" },
    };
    do_parse_tests( tests );
}

TEST( Parser, Binary ) {
    std::vector<ParseTests> tests = {
        { "int main(void) { return 1+1;}", "int main(void) {return (1 + 1);}", "" },
        { "int main(void) { return 2+3*4;}", "int main(void) {return (2 + (3 * 4));}", "" },
        { "int main(void) { return 2*3+4;}", "int main(void) {return ((2 * 3) + 4);}", "" },
        { "int main(void) { return 2*3+4*5;}", "int main(void) {return ((2 * 3) + (4 * 5));}", "" },
        { "int main(void) { return 2*3+4*5/6;}", "int main(void) {return ((2 * 3) + ((4 * 5) / 6));}", "" },
        { "int main(void) { return 2*3+4*5/6-7;}", "int main(void) {return (((2 * 3) + ((4 * 5) / 6)) - 7);}", "" },
        { "int main(void) { return 2*3+4*5/6-7+8;}", "int main(void) {return ((((2 * 3) + ((4 * 5) / 6)) - 7) + 8);}",
          "" },
        { "int main(void) { return 2*3+4*5/6-7+8*9;}",
          "int main(void) {return ((((2 * 3) + ((4 * 5) / 6)) - 7) + (8 * 9));}", "" },
    };
    do_parse_tests( tests );
}

TEST( Parser, Combined ) {
    std::vector<ParseTests> tests = {
        { "int main(void) { return ~(1+1);}", "int main(void) {return ~(1 + 1);}", "" },
        { "int main(void) { return (-1)+1;}", "int main(void) {return (-1 + 1);}", "" },
        { "int main(void) { return -1+1;}", "int main(void) {return (-1 + 1);}", "" },
    };
    do_parse_tests( tests );
}

TEST (Parse, Associtivity) {
    std::vector<ParseTests> tests = {
        { "int main(void) { return 1 - 2 - 3;}", "int main(void) {return ((1 - 2) - 3);}", "" },
    };
    do_parse_tests( tests );
}

auto do_parse_tests( std::vector<ParseTests> const& tests ) -> void {

    for ( auto const& t : tests ) {

        std::istringstream is( t.input );
        Lexer              lex( is );
        Parser             parser( lex );

        std::string result;
        try {
            std::cout << t.input << std::endl;
            auto ast = parser.parse();

            PrinterAST prt;
            prt.new_line = ""; // turn off newlines
            prt.indent = "";   // turn off indents
            result = prt.print( ast );

            EXPECT_EQ( result, t.output );
        } catch ( ParseException& e ) {
            if ( t.error.empty() ) {
                std::println( "Expect: {}\ngot   : {}", t.error, e.get_message() );
                EXPECT_TRUE( false );
                continue;
            }
            EXPECT_EQ( e.get_message(), t.error );
        } catch ( LexicalException& e ) {
            if ( t.error.empty() ) {
                std::println( "Expect: {}\ngot   : {}", t.error, e.get_message() );
                EXPECT_TRUE( false );
                continue;
            }
            EXPECT_EQ( e.get_message(), t.error );
        } catch ( std::exception& e ) {
            std::println( "Exception: {}", e.what() );
            FAIL();
        }
    }
}