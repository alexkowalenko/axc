
include_directories(${PROJECT_SOURCE_DIR}/src)

# Generate the various ASDL from definitions in the python files in the tool directory

set(AST_HEADER ${PROJECT_SOURCE_DIR}/src/ast/includes.h)
set(TAC_HEADER ${PROJECT_SOURCE_DIR}/src/tac/includes.h)

set(AT_HEADER ${PROJECT_SOURCE_DIR}/src/machine/x86_64/x86_at/includes.h)

macro(add_generator GEN DIR HEADER)
    add_custom_command(
            OUTPUT ${HEADER}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generate${GEN}.py ${PROJECT_SOURCE_DIR}/src/${DIR}
            DEPENDS ${CMAKE_SOURCE_DIR}/tools/generate${GEN}.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools
            COMMENT "Generating ${GEN} header files if missing"
    )
endmacro()

add_generator(AST ast ${AST_HEADER})
add_generator(TAC tac ${TAC_HEADER})

add_generator(X86_AT  machine/x86_64/x86_at ${AT_HEADER})

add_library(compiler STATIC
        token.cpp
        lexer.cpp
        parser.cpp
        printerAST.cpp
        # Semantic analysis
        symbolTable.cpp
        semanticAnalyser.cpp
        # TAC Generation
        tacGen.cpp
        printerTAC.cpp
        # Code Gen
        codeGen.cpp
        # ASTs
        ${AST_HEADER}
        ${AT_HEADER}
        ${TAC_HEADER}
)

target_link_libraries(compiler
        PRIVATE
        spdlog::spdlog
)
