

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

install(TARGETS axc_comp
        COMPONENT axc
        DESTINATION bin)

install(FILES axc.py
        COMPONENT axc
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(FILES axc.py
        COMPONENT axc
        DESTINATION bin
        RENAME axc_arm64.py
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
