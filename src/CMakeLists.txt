
# Generate the various ASDL from definitions in the python files in the tool directory

set(AST_HEADER ${PROJECT_SOURCE_DIR}/src/ast/includes.h)
set(AT_HEADER ${PROJECT_SOURCE_DIR}/src/at/includes.h)
set(TAC_HEADER ${PROJECT_SOURCE_DIR}/src/tac/includes.h)

macro(add_generator GEN HEADER)
    string(TOLOWER "${GEN}" DIR)
    add_custom_command(
            OUTPUT ${HEADER}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generate${GEN}.py ${PROJECT_SOURCE_DIR}/src/${DIR}
            DEPENDS ${CMAKE_SOURCE_DIR}/tools/generate${GEN}.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools
            COMMENT "Generating ${GEN} header files if missing"
    )
endmacro()

add_generator(AST ${AST_HEADER})
add_generator(AT  ${AT_HEADER})
add_generator(TAC ${TAC_HEADER})

add_library(compiler STATIC
        lexer.cpp
        parser.cpp
        printerAST.cpp
        # TAC Generation
        tacGen.cpp
        printerTAC.cpp
        # Code Gen
        codeGen.cpp
        # Machine code
        machine/assemblyGen.cpp
        machine/assemblyFilterPseudo.cpp
        machine/assemblyFixInstruct.cpp
        machine/x86_64CodeGen.cpp
        machine/printerAT.cpp
        ${AST_HEADER}
        ${AT_HEADER}
        ${TAC_HEADER}
)

target_link_libraries(compiler
        PRIVATE
        spdlog::spdlog
)
