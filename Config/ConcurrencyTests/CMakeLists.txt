file(GLOB source_files ${ASPEN_SOURCE_PATH}/ConcurrencyTests/*.cpp)
add_executable(aspen_concurrency_tester ${source_files})
target_include_directories(
  aspen_concurrency_tester SYSTEM PRIVATE ${DOCTEST_INCLUDE_PATH})
if(UNIX)
  target_link_libraries(aspen_concurrency_tester PRIVATE pthread)
endif()
foreach(case parent_assignment parent_destruction parent_list_mutation
    completed_child_removal)
  add_test(NAME concurrency_${case}
    COMMAND aspen_concurrency_tester -tc=${case})
  set_tests_properties(concurrency_${case} PROPERTIES TIMEOUT 600)
endforeach()
install(TARGETS aspen_concurrency_tester CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS aspen_concurrency_tester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
