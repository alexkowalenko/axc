

add_executable(axc_comp)

target_sources(axc_comp PRIVATE
        main.cpp
)

target_link_libraries(axc_comp PRIVATE
        project_options
        spdlog::spdlog
        argparse
        axc::compiler
        axc::x86
        axc::arm64
)

install(TARGETS axc_comp DESTINATION bin)
install(FILES axc.py DESTINATION bin PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)