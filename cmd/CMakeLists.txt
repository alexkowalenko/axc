
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(axc main.cpp
        )

target_link_libraries(axc PRIVATE
        spdlog::spdlog
        argparse
        compiler
)