
include(GoogleTest)

set_target_properties(gtest PROPERTIES FOLDER extern)
set_target_properties(gtest_main PROPERTIES FOLDER extern)

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME})

    target_sources(${TESTNAME} PRIVATE ${ARGN})

    # link the Google test infrastructure
    target_link_libraries(${TESTNAME} PRIVATE
            gtest gtest_main
            axc::compiler
    )

    # gtest_discover_tests replaces gtest_add_tests,
    gtest_discover_tests(${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

package_add_test(token.test token.test.cpp)
package_add_test(lexer.test lexer.test.cpp)
package_add_test(parser.test parser.test.cpp)
