
add_library(axc.x86)
add_library(axc::x86 ALIAS axc.x86)

target_sources(axc.x86 PRIVATE
        assemblyGen.cpp
        filterPseudo.cpp
        fixInstructX86.cpp
        x86_64CodeGen.cpp
        printerX86.cpp
)

target_sources(axc.x86
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}/src/machine/x86_64
        FILES assemblyGen.h
        filterPseudo.h
        fixInstructX86.h
        x86_64CodeGen.h
        printerX86.h
)

target_link_libraries(axc.x86 PRIVATE
        project_options
        spdlog::spdlog
        axc::compiler
)