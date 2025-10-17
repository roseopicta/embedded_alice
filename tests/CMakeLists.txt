add_executable(test_all
  test_dsp.cc
)

set(TESTDATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/testdata")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/testdata_path.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/testdata_path.h
  @ONLY
)

target_include_directories(test_all PRIVATE
  ${CMAKE_SOURCE_DIR}/dsp
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(test_all PRIVATE
  GTest::gtest_main
  dsp
)

include(GoogleTest)
gtest_discover_tests(test_all)
