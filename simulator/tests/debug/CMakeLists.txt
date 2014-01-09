file(GLOB_RECURSE UnitTests_sources ut-*.cc)
add_executable(UnitTests ${UnitTests_sources})
target_link_libraries(UnitTests patmos-simulator-debug 
  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
add_test(UnitTests UnitTests)

#add_executable(GdbTestPipe GdbTestPipeConnection.cc)
#target_link_libraries(GdbTestPipe patmos-simulator-debug)

find_package(Threads REQUIRED)

add_executable(GdbTestTcp GdbTestTcpConnection.cc)
target_link_libraries(GdbTestTcp patmos-simulator-debug ${Boost_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})
