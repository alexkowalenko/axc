
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/machine)

add_executable(axc_comp main.cpp
        )

target_link_libraries(axc_comp PRIVATE
        spdlog::spdlog
        argparse
        compiler
        x86
        arm64
)

install(TARGETS axc_comp DESTINATION bin)
install(FILES axc.py DESTINATION bin PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)