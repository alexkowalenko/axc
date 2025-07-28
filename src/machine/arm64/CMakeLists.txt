include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(arm64 armAssemblyGen.cpp
        arm64CodeGen.cpp
        printerARM64.cpp
)

target_link_libraries(arm64 PRIVATE
        spdlog::spdlog
)