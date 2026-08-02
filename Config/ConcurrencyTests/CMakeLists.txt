file(GLOB source_files ${ASPEN_SOURCE_PATH}/ConcurrencyTests/*.cpp)
add_executable(aspen_concurrency_tester ${source_files})
target_include_directories(
  aspen_concurrency_tester SYSTEM PRIVATE ${DOCTEST_INCLUDE_PATH})
if(UNIX)
  target_link_libraries(aspen_concurrency_tester PRIVATE pthread)
endif()
list(APPEND CMAKE_MODULE_PATH ${DOCTEST_INCLUDE_PATH}/scripts/cmake)
include(doctest)
doctest_discover_tests(aspen_concurrency_tester
  TEST_PREFIX concurrency_
  PROPERTIES TIMEOUT 600)
install(TARGETS aspen_concurrency_tester CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS aspen_concurrency_tester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
