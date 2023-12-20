add_subdirectory(catch)

function(ADD_TANUKI_TESTCASE arg1)
  add_executable(${arg1} ${arg1}.cpp)
  target_link_libraries(${arg1} PRIVATE tanuki Catch2::Catch2WithMain)
  if(TANUKI_WITH_BOOST_S11N)
    target_link_libraries(${arg1} PRIVATE tanuki_boost_s11n)
  endif()
  target_compile_options(${arg1} PRIVATE
    "$<$<CONFIG:Debug>:${TANUKI_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${TANUKI_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${TANUKI_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${TANUKI_CXX_FLAGS_RELEASE}>"
  )

  # Setup the C++ standard.
  target_compile_features(${arg1} PRIVATE cxx_std_20)
  set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)

  # Optional support for clang-tidy.
  if(TANUKI_CLANG_TIDY)
    set_property(TARGET ${arg1} PROPERTY CXX_CLANG_TIDY "${TANUKI_CLANG_TIDY_PATH}")
  endif()

  add_test(${arg1} ${arg1})
endfunction()

ADD_TANUKI_TESTCASE(test_basics)
ADD_TANUKI_TESTCASE(test_swap)
ADD_TANUKI_TESTCASE(test_defctor)
ADD_TANUKI_TESTCASE(test_noncopyable)
ADD_TANUKI_TESTCASE(test_nostatic)
ADD_TANUKI_TESTCASE(test_misc_utils)
ADD_TANUKI_TESTCASE(test_gen_assignment)
ADD_TANUKI_TESTCASE(test_ref_iface)
ADD_TANUKI_TESTCASE(test_trait_like)
ADD_TANUKI_TESTCASE(test_composite)
ADD_TANUKI_TESTCASE(test_invalid_iface)
ADD_TANUKI_TESTCASE(test_inf_loop_bug)
ADD_TANUKI_TESTCASE(std_function)
ADD_TANUKI_TESTCASE(io_iterator)
ADD_TANUKI_TESTCASE(input_iterator)
ADD_TANUKI_TESTCASE(forward_iterator)
ADD_TANUKI_TESTCASE(bidirectional_iterator)
ADD_TANUKI_TESTCASE(random_access_iterator)
ADD_TANUKI_TESTCASE(forward_range)
ADD_TANUKI_TESTCASE(bidirectional_range)
ADD_TANUKI_TESTCASE(random_access_range)

# Test to check explicit instantiation of the
# wrap class in an external library.
add_library(external_inst SHARED external_inst.cpp)
set_target_properties(external_inst PROPERTIES CXX_VISIBILITY_PRESET hidden)
set_target_properties(external_inst PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
target_compile_options(external_inst PRIVATE
  "$<$<CONFIG:Debug>:${TANUKI_CXX_FLAGS_DEBUG}>"
  "$<$<CONFIG:Release>:${TANUKI_CXX_FLAGS_RELEASE}>"
  "$<$<CONFIG:RelWithDebInfo>:${TANUKI_CXX_FLAGS_RELEASE}>"
  "$<$<CONFIG:MinSizeRel>:${TANUKI_CXX_FLAGS_RELEASE}>"
)
target_compile_features(external_inst PRIVATE cxx_std_20)
set_property(TARGET external_inst PROPERTY CXX_EXTENSIONS NO)
target_link_libraries(external_inst PUBLIC tanuki)
if(TANUKI_CLANG_TIDY)
  set_property(TARGET external_inst PROPERTY CXX_CLANG_TIDY "${TANUKI_CLANG_TIDY_PATH}")
endif()
ADD_TANUKI_TESTCASE(test_external_inst)
target_link_libraries(test_external_inst PUBLIC external_inst)

if(TANUKI_WITH_BOOST_S11N)
  # Test to check the proper functioning of serialisation
  # macros in external library.
  add_library(fooable SHARED fooable.cpp)
  set_target_properties(fooable PROPERTIES CXX_VISIBILITY_PRESET hidden)
  set_target_properties(fooable PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)
  target_compile_options(fooable PRIVATE
    "$<$<CONFIG:Debug>:${TANUKI_CXX_FLAGS_DEBUG}>"
    "$<$<CONFIG:Release>:${TANUKI_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:RelWithDebInfo>:${TANUKI_CXX_FLAGS_RELEASE}>"
    "$<$<CONFIG:MinSizeRel>:${TANUKI_CXX_FLAGS_RELEASE}>"
  )
  target_compile_features(fooable PRIVATE cxx_std_20)
  set_property(TARGET fooable PROPERTY CXX_EXTENSIONS NO)
  target_link_libraries(fooable PUBLIC tanuki tanuki_boost_s11n)

  # Optional support for clang-tidy.
  if(TANUKI_CLANG_TIDY)
    set_property(TARGET fooable PROPERTY CXX_CLANG_TIDY "${TANUKI_CLANG_TIDY_PATH}")
  endif()

  ADD_TANUKI_TESTCASE(test_lib_s11n)
  target_link_libraries(test_lib_s11n PRIVATE fooable)
endif()
