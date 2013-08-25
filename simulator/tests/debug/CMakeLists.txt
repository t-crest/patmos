file(GLOB_RECURSE UnitTests_sources ut-*.cc)
add_executable(UnitTests ${UnitTests_sources})
target_link_libraries(UnitTests patmos-simulator-debug ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(UnitTests UnitTests)

add_executable(GdbTest gdbtest.cc)
target_link_libraries(GdbTest patmos-simulator-debug)
