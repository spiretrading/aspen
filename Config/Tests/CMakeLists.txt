include_directories(SYSTEM ${DOCTEST_INCLUDE_PATH})
file(GLOB_RECURSE source_files ${ASPEN_SOURCE_PATH}/Tests/*.cpp)
add_executable(aspen_tester ${source_files})
target_link_libraries(aspen_tester PRIVATE aspen)
if(UNIX)
  target_link_libraries(aspen_tester pthread)
endif()
add_custom_command(TARGET aspen_tester POST_BUILD COMMAND aspen_tester)
install(TARGETS aspen_tester CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS aspen_tester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
