
include(GoogleTest)

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})

    # link the Google test infrastructure
    target_link_libraries(${TESTNAME} PRIVATE
            gtest gtest_main
            compiler
    )
    target_include_directories(${TESTNAME} PRIVATE ${LLVM_INCLUDE_DIRS})

    # gtest_discover_tests replaces gtest_add_tests,
    gtest_discover_tests(${TESTNAME}
            # set a working directory so your project root so that you can find test data via paths relative to the project root
            WORKING_DIRECTORY ${PROJECT_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/extern/googletest/googletest/include)

package_add_test(token.test token.test.cpp)
package_add_test(lexer.test lexer.test.cpp)
package_add_test(parser.test parser.test.cpp)
