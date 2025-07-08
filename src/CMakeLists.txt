
set(GENERATED_HEADER ${PROJECT_SOURCE_DIR}/src/ast/includes.h)

add_custom_command(
        OUTPUT ${GENERATED_HEADER}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generateAST.py ${PROJECT_SOURCE_DIR}/src/ast
        DEPENDS ${CMAKE_SOURCE_DIR}/tools/generateAST.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools
        COMMENT "Generating AST header files if missing"
)

add_library(compiler STATIC
        lexer.cpp
        parser.cpp
        printerAST.cpp
        ${GENERATED_HEADER}
)