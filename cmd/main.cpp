//
// AXC - C compiler
//
// Copyright  © Alex Kowalenko 2025
//

#include "exception.h"

#include <fstream>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "lexer.h"
#include "option.h"
#include "parser.h"
#include "printerAST.h"

void setup_logging( Option const& options ) {
    spdlog::set_pattern( "[%H:%M:%S.%f] %^[%l]%$ %v" );
    if ( options.silent ) {
        spdlog::set_level( spdlog::level::off );
        return;
    }
    spdlog::set_level( spdlog::level::trace );
}

int do_args( int argc, char** argv, Option& options ) {
    argparse::ArgumentParser app { "axc", "0.1" };

    app.add_argument( "-s", "--silent" ).help( "silent operation (no logging)." ).flag().store_into( options.silent );

    bool lex { false };
    bool parse { false };
    bool codegen { false };

    auto& group = app.add_mutually_exclusive_group();
    group.add_argument( "-l", "--lex" ).help( "run only lexer." ).flag().store_into( lex );
    group.add_argument( "-p", "--parse" ).help( "run lexer and parser." ).flag().store_into( parse );
    group.add_argument( "-c", "--codegen" )
        .help( "run lex, parser and codegen, no output." )
        .flag()
        .store_into( codegen );

    app.add_argument( "filename" ).help( "File to be compiled" ).store_into( options.input_file );
    try {
        app.parse_args( argc, argv );
    } catch ( const std::exception& err ) {
        std::println( "{}", err.what() );
        return EXIT_FAILURE;
    }

    if ( !std::filesystem::exists( options.input_file ) ) {
        std::println( "Input file {} does not exist.", options.input_file );
        return EXIT_FAILURE;
    }

    if ( lex ) {
        options.stage = Stages::Lex;
    } else if ( parse ) {
        options.stage = static_cast<Stages>( Stages::Lex | Stages::Parse );
    } else if ( codegen ) {
        options.stage = static_cast<Stages>( Stages::Lex | Stages::Parse | Stages::CodeGen );
    } else {
        options.stage = Stages::All;
    }

    return EXIT_SUCCESS;
}

int main( int argc, char** argv ) {
    Option options;

    if ( auto status = do_args( argc, argv, options ); status != EXIT_SUCCESS ) {
        std::exit( status );
    }

    setup_logging( options );
    spdlog::info( "AXC compiler 👾" );

    try {
        spdlog::info( "Run lexer," );
        std::ifstream file { options.input_file };
        Lexer         lexer { file };

        if ( ( options.stage & Stages::Parse ) == 0 ) {
            for ( Token token = lexer.get_token(); token.tok != TokenType::Eof; token = lexer.get_token() ) {
                std::print( "{} ", to_string( token ) );
            }
            std::println();
            return EXIT_SUCCESS;
        }

        spdlog::info( "Run parser," );
        Parser     parser { lexer };
        auto       program = parser.parse();
        PrinterAST printer;
        auto       output = printer.print( program );
        std::println( "{:s}", output );

        if ( options.stage & Stages::CodeGen ) {
            spdlog::info( "Run codegen," );
        }
        if ( options.stage & Stages::File ) {
            spdlog::info( "Output file." );
        }
    } catch ( const LexicalException& e ) {
        std::println( "Lexical error: {}", e.get_message() );
        return EXIT_FAILURE;
    } catch ( const ParseException& e ) {
        std::println( "Parse error: {}", e.get_message() );
        return EXIT_FAILURE;
    } catch ( const std::exception& err ) {
        std::println( "Exception: {}", err.what() );
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
