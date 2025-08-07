
add_library(axc.arm64)
add_library(axc::arm64 ALIAS axc.arm64)

target_sources(axc.arm64 PRIVATE
        armAssemblyGen.cpp
        arm64CodeGen.cpp
        printerARM64.cpp
)

target_sources(axc.arm64
        PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${PROJECT_SOURCE_DIR}/src/machine/arm64
        FILES armAssemblyGen.h
        arm64CodeGen.h
        printerARM64.h
)

target_link_libraries(axc.arm64 PRIVATE
        project_options
        spdlog::spdlog
        axc::compiler
)