
set(AST_HEADER ${PROJECT_SOURCE_DIR}/src/ast/includes.h)
set(AT_HEADER ${PROJECT_SOURCE_DIR}/src/at/includes.h)

add_custom_command(
        OUTPUT ${AST_HEADER}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generateAST.py ${PROJECT_SOURCE_DIR}/src/ast
        DEPENDS ${CMAKE_SOURCE_DIR}/tools/generateAST.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools
        COMMENT "Generating AST header files if missing"
)
add_custom_command(
        OUTPUT ${AT_HEADER}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generateAT.py ${PROJECT_SOURCE_DIR}/src/at
        DEPENDS ${CMAKE_SOURCE_DIR}/tools/generateAT.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools
        COMMENT "Generating AT header files if missing"
)

add_library(compiler STATIC
        lexer.cpp
        parser.cpp
        printerAST.cpp
        assemblyGen.cpp
        printerAT.cpp
        ${AST_HEADER}
        ${AT_HEADER}
)