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

# Setup the sqlite library.
set(THREADS_PREFER_PTHREAD_FLAG YES)
find_package(Threads REQUIRED)
unset(THREADS_PREFER_PTHREAD_FLAG)
add_library(tanuki_sqlite STATIC sqlite3.c)
target_link_libraries(tanuki_sqlite PRIVATE ${CMAKE_DL_LIBS} Threads::Threads)

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
ADD_TANUKI_TESTCASE(test_const_ref_access)
ADD_TANUKI_TESTCASE(test_overaligned)
ADD_TANUKI_TESTCASE(test_ref_implicit)
ADD_TANUKI_TESTCASE(test_ref_semantics)
ADD_TANUKI_TESTCASE(test_make_invalid)
ADD_TANUKI_TESTCASE(test_emplace)
ADD_TANUKI_TESTCASE(test_invalid_composite)
ADD_TANUKI_TESTCASE(test_std_function)

ADD_TANUKI_TESTCASE(io_iterator)
ADD_TANUKI_TESTCASE(input_iterator)
ADD_TANUKI_TESTCASE(forward_iterator)
ADD_TANUKI_TESTCASE(bidirectional_iterator)
ADD_TANUKI_TESTCASE(random_access_iterator)
ADD_TANUKI_TESTCASE(input_range)
ADD_TANUKI_TESTCASE(forward_range)
ADD_TANUKI_TESTCASE(bidirectional_range)
ADD_TANUKI_TESTCASE(random_access_range)
ADD_TANUKI_TESTCASE(time_series)

if(TANUKI_WITH_BOOST_S11N)
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

ADD_TANUKI_TESTCASE(sqlite_ts)
target_link_libraries(sqlite_ts PRIVATE tanuki_sqlite)
target_compile_definitions(sqlite_ts PRIVATE "-DTANUKI_SQLITE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/simple.db\"")

if(TANUKI_TEST_PYBIND11)
  add_subdirectory(pybind11)
endif()
