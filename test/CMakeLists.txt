add_subdirectory(catch)

function(ADD_TANUKI_TESTCASE arg1)
  add_executable(${arg1} ${arg1}.cpp)
  target_link_libraries(${arg1} PRIVATE tanuki Catch2::Catch2WithMain)
  target_compile_options(${arg1} PRIVATE
    "$<$<CONFIG:Debug>:${TANUKI_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${TANUKI_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${TANUKI_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${TANUKI_CXX_FLAGS_RELEASE}>"
  )

  # Setup the C++ standard.
  target_compile_features(${arg1} PRIVATE cxx_std_20)
  set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)

  add_test(${arg1} ${arg1})
endfunction()

ADD_TANUKI_TESTCASE(test_basics)