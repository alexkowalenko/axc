include_directories(${PROJECT_SOURCE_DIR}/src)

add_library(x86 assemblyGen.cpp
        assemblyFilterPseudo.cpp
        assemblyFixInstruct.cpp
        x86_64CodeGen.cpp
        printerAT.cpp
)

target_link_libraries(x86 PRIVATE
        spdlog::spdlog
)