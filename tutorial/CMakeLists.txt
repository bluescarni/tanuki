function(ADD_TANUKI_TUTORIAL arg1)
  add_executable(${arg1} ${arg1}.cpp)
  target_link_libraries(${arg1} PRIVATE tanuki)
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

  add_test(${arg1} ${arg1})
endfunction()

ADD_TANUKI_TUTORIAL(hello_world)
ADD_TANUKI_TUTORIAL(simple_interface)
ADD_TANUKI_TUTORIAL(reference_interface)
ADD_TANUKI_TUTORIAL(custom_storage)
ADD_TANUKI_TUTORIAL(wrap_reference)
ADD_TANUKI_TUTORIAL(emplace)
