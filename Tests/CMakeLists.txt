include_directories(SYSTEM ${CATCH2_INCLUDE_PATH})
file(GLOB source_files ${ASPEN_INCLUDE_PATH}/../Tests/Source/*.cpp)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_executable(aspen_tester ${source_files})
add_custom_command(TARGET aspen_tester POST_BUILD COMMAND aspen_tester)
install(TARGETS aspen_tester CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS aspen_tester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
