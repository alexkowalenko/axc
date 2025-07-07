//
// AXC - C compiler
//
// Copyright  © Alex Kowalenko 2025
//

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "option.h"

void setup_logging(Option const &options) {
    spdlog::set_pattern("[%H:%M:%S.%f] %^[%l]%$ %v");
    if (options.silent) {
        spdlog::set_level(spdlog::level::off);
    }
    spdlog::set_level(spdlog::level::trace);
}

int do_args(int argc, char **argv, Option &options) {
    argparse::ArgumentParser app{"axc", "0.1"};

    app.add_argument("-s", "--silent").help("silent operation (no logging).").flag().store_into(options.silent);

    bool lex{false};
    bool parse{false};
    bool codegen{false};

    auto &group = app.add_mutually_exclusive_group();
    group.add_argument("--lex").help("run only lexer.").flag().store_into(lex);
    group.add_argument("--parse").help("run lexer and parser.").flag().store_into(parse);
    group.add_argument("--codegen").help("run lex, parser and codegen, no output.").flag().store_into(codegen);

    try {
        app.parse_args(argc, argv);
    } catch (const std::exception &err) {
        std::println("{}", err.what());
        return EXIT_FAILURE;
    }

    if (lex) {
        options.stage = Stages::Lexer;
    } else if (parse) {
        options.stage = static_cast<Stages>(Stages::Lexer | Stages::Parser);
    } else if (codegen) {
        options.stage = static_cast<Stages>(Stages::Lexer | Stages::Parser | Stages::CodeGen);
    } else {
        options.stage = Stages::All;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    Option options;

    if (auto status = do_args(argc, argv, options); status != EXIT_SUCCESS) {
        std::exit(status);
    }

    setup_logging(options);
    spdlog::info("AXC compiler 👾");

    if (options.stage & Stages::Lexer) {
        spdlog::info("Run lexer,");
    }
    if (options.stage & Stages::Parser) {
        spdlog::info("Run parser,");
    }
    if (options.stage & Stages::CodeGen) {
        spdlog::info("Run codegen,");
    }
    if (options.stage & Stages::File) {
        spdlog::info("Output file.");
    }
    return EXIT_SUCCESS;
}
