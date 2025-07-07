// Copyright (c) 2025.

//
// Created by Alex Kowalenko on 7/7/2025.
//

#pragma once

enum Stages {
    None = 0,
    Lexer = 0x1,
    Parser = 0x2,
    CodeGen = 0x4,
    File = 0x8,
    All = Lexer | Parser | CodeGen | File
};

class Option {
public:
    bool silent{false};
    Stages stage{Stages::All};
};
