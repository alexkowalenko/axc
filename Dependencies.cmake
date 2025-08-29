
CPMAddPackage(
        NAME GoogleTest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.17.0
        OPTIONS
        "INSTALL_GTEST OFF"
        "BUILD_GMOCK OFF"
)

CPMAddPackage(
        NAME spdlog
        GITHUB_REPOSITORY gabime/spdlog
        GIT_TAG v1.15.3
        OPTIONS
        "SPDLOG_BUILD_EXAMPLES OFF"
        "SPDLOG_BUILD_TESTS OFF"
        "SPDLOG_INSTALL OFF")

CPMAddPackage(
        NAME argparse
        GITHUB_REPOSITORY p-ranav/argparse
        GIT_TAG v3.2)

find_package(Python3 REQUIRED COMPONENTS Interpreter)