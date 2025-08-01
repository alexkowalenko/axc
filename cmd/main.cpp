//
// AXC - C compiler
//
// Copyright  © Alex Kowalenko 2025
//

#include <fstream>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "codeGen.h"
#include "exception.h"
#include "lexer.h"
#include "option.h"
#include "parser.h"
#include "printerAST.h"
#include "printerTAC.h"
#include "semanticAnalyser.h"
#include "symbolTable.h"
#include "tacGen.h"

void setup_logging( Option const& options ) {
    spdlog::set_pattern( "[%H:%M:%S.%f] %^[%l]%$ %v" );
    if ( options.silent ) {
        spdlog::set_level( spdlog::level::off );
        return;
    }
    spdlog::set_level( spdlog::level::trace );
}

void get_os( Option& options ) {
#if defined( _WIN32 )
    std::cout << "Windows" << std::endl;
#elif defined( __APPLE__ ) && defined( __MACH__ )
    options.system = System::MacOS;
#elif defined( __linux__ )
    options.system = System::Linux;
#elif defined( __FreeBSD__ )
    options.system = System::FreeBSD;
#elif defined( __unix__ )
    options.system = System::Linux;
#endif
}

int do_args( int argc, char** argv, Option& options ) {

    get_os( options );
    argparse::ArgumentParser app { "axc", "0.1" };

    app.add_argument( "-s", "--silent" ).help( "silent operation (no logging)." ).flag().store_into( options.silent );
    app.add_argument( "-m", "--machine" )
        .help( "Machine architecture" )
        .choices( "x86_64", "amd64", "aarch64", "arm64" )
        .default_value( "x86_64" );

    bool lex { false };
    bool parse { false };
    bool semantic { false };
    bool codegen { false };
    bool tac { false };

    auto& group = app.add_mutually_exclusive_group();
    group.add_argument( "-l", "--lex" ).help( "run only lexer." ).flag().store_into( lex );
    group.add_argument( "-p", "--parse" ).help( "run lexer and parser." ).flag().store_into( parse );
    group.add_argument( "-v", "--validate" ).help( "run the semantic analyser." ).flag().store_into( semantic );
    group.add_argument( "-t", "--tacky" )
        .help( "run lexer, parser, semantic, tac generator." )
        .flag()
        .store_into( tac );
    group.add_argument( "-c", "--codegen" )
        .help( "run lex, parser, semantic tac and codegen, no output." )
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
    } else if ( semantic ) {
        options.stage = static_cast<Stages>( Stages::Lex | Stages::Parse | Stages::Semantic );
    } else if ( tac ) {
        options.stage = static_cast<Stages>( Stages::Lex | Stages::Parse | Stages::Semantic | Stages::Tac );
    } else if ( codegen ) {
        options.stage =
            static_cast<Stages>( Stages::Lex | Stages::Parse | Stages::Semantic | Stages::Tac | Stages::CodeGen );
    } else {
        options.stage = Stages::All;
    }

    if ( app.get( "machine" ) == "x86_64" || app.get( "machine" ) == "amd64" ) {
        options.machine = Machine::X86_64;
    } else if ( app.get( "machine" ) == "aarch64" || app.get( "machine" ) == "arm64" ) {
        options.machine = Machine::AArch64;
    } else {
        options.machine = Machine::X86_64;
    }

    return EXIT_SUCCESS;
}

Lexer run_lexer( Option const& options ) {
    spdlog::info( "Run lexer," );
    std::ifstream file { options.input_file };
    Lexer         lexer { file };
    return lexer;
}

ast::Program run_parser( Lexer& lexer ) {
    spdlog::info( "Run parser," );
    Parser parser { lexer };
    auto   program = parser.parse();

    PrinterAST printer;
    auto       output = printer.print( program );
    std::println( "Parsing Output:" );
    std::println( "--------------" );
    std::println( "{:s}", output );
    return program;
}

void run_sematic( ast::Program program, SymbolTable& symbol_table ) {
    spdlog::info( "Run semantic anylser," );
    SemanticAnalyser analyser;
    analyser.analyse( program, symbol_table );

    PrinterAST printer;
    auto       output = printer.print( program );
    std::println( "Semantic Output:" );
    std::println( "----------------" );
    std::println( "{:s}", output );
}

tac::Program run_tac( ast::Program program, SymbolTable& table ) {
    spdlog::info( "Run TAC generator," );
    TacGen tac_generator( table );
    auto   tac = tac_generator.generate( program );

    PrinterTAC tac_printer;
    auto       output = tac_printer.print( tac );
    std::println( "TAC Output:" );
    std::println( "----------" );
    std::println( "{:s}", output );
    return tac;
}

int main( int argc, char** argv ) {
    Option options;

    if ( auto status = do_args( argc, argv, options ); status != EXIT_SUCCESS ) {
        std::exit( status );
    }

    setup_logging( options );
    spdlog::info( "AXC compiler 👾" );

    try {
        // Run Lexer
        Lexer lexer = run_lexer( options );

        if ( ( options.stage & Stages::Parse ) == 0 ) {
            for ( Token token = lexer.get_token(); token.tok != TokenType::Eof; token = lexer.get_token() ) {
                std::println( "{} {} ", token.location, ( token ) );
            }
            std::println( "" );
            return EXIT_SUCCESS;
        }

        // Run Parser
        auto program = run_parser( lexer );

        if ( ( options.stage & Stages::Semantic ) == 0 ) {
            return EXIT_SUCCESS;
        }

        SymbolTable symbol_table;
        run_sematic( program, symbol_table );

        if ( ( options.stage & Stages::Tac ) == 0 ) {
            return EXIT_SUCCESS;
        }

        // Run TAC Generator
        auto tac = run_tac( program, symbol_table );

        if ( ( options.stage & Stages::CodeGen ) == 0 ) {
            return EXIT_SUCCESS;
        }

        // Run Code Gen
        auto codeGenerator = make_CodeGen( options );
        if ( !codeGenerator ) {
            throw CodeException( Location {}, "Cannot create code generator for machine: {}",
                                 to_string( options.machine ) );
        }

        auto assembly = codeGenerator->run_codegen( tac );

        if ( ( options.stage & Stages::File ) == 0 ) {
            return EXIT_SUCCESS;
        }

        codeGenerator->generate_output_file( assembly );

    } catch ( const LexicalException& e ) {
        std::println( "Lexical error: {}", e.get_message() );
        return EXIT_FAILURE;
    } catch ( const ParseException& e ) {
        std::println( "Parse error: {}", e.get_message() );
        return EXIT_FAILURE;
    } catch ( const SemanticException& e ) {
        std::println( "Semantic error: {}", e.get_message() );
        return EXIT_FAILURE;
    } catch ( const CodeException& e ) {
        std::println( "Code Generation: {}", e.get_message() );
        return EXIT_FAILURE;
    } catch ( const std::exception& err ) {
        std::println( "Exception: {}", err.what() );
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
