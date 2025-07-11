// Copyright (c) 2025.

//
// Created by Alex Kowalenko on 7/7/2025.
//

#pragma once

#include <string>

enum Stages {
    None = 0,
    Lex = 0x1,
    Parse = 0x2,
    CodeGen = 0x4,
    Tac = 0x8,
    File = 0x10,
    All = Lex | Parse | CodeGen | Tac | File
};

enum class Machine {
    X86_64,
    AArch64,
};

constexpr auto to_string( Machine m ) {
    switch ( m ) {
    case Machine::AArch64 :
        return "aarch64";
    case Machine::X86_64 :
        return "x86_64";
    default :
        return "unknown";
    }
}

enum class System {
    Linux,
    FreeBSD,
    MacOS,
};

class Option {
  public:
    bool        silent { false };
    Stages      stage { Stages::All };
    std::string input_file;
    Machine     machine { Machine::X86_64 };
    System      system { System::MacOS };
};
