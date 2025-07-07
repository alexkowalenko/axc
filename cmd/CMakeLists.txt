
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(axc main.cpp)

target_link_libraries(axc
        spdlog::spdlog
        argparse
)