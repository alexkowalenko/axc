// Copyright (c) 2025.

//
// Created by Alex Kowalenko on 7/7/2025.
//

#pragma once

enum Stages {
    None = 0,
    Lex = 0x1,
    Parse = 0x2,
    CodeGen = 0x4,
    File = 0x8,
    All = Lex | Parse | CodeGen | File
};

class Option {
public:
    bool silent{false};
    Stages stage{Stages::All};
    std::string input_file;
};
