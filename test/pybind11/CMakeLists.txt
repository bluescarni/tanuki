# Find Python
find_package(Python3 REQUIRED COMPONENTS Interpreter Development Development.Module)

# Find pybind11.
find_package(pybind11 REQUIRED CONFIG)

function(ADD_TANUKI_PYBIND11_TESTCASE arg1)
    Python3_add_library(${arg1} MODULE WITH_SOABI ${arg1}.cpp)
    target_link_libraries(${arg1} PRIVATE tanuki)
    target_link_libraries(${arg1} PRIVATE "${pybind11_LIBRARIES}")
    target_include_directories(${arg1} SYSTEM PRIVATE "${pybind11_INCLUDE_DIR}" "${Python3_INCLUDE_DIRS}")
    target_compile_definitions(${arg1} PRIVATE "${pybind11_DEFINITIONS}")
    set_target_properties(${arg1} PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(${arg1} PROPERTIES VISIBILITY_INLINES_HIDDEN TRUE)

    target_compile_options(${arg1} PRIVATE
      "$<$<CONFIG:Debug>:${TANUKI_CXX_FLAGS_DEBUG}>"
      "$<$<CONFIG:Release>:${TANUKI_CXX_FLAGS_RELEASE}>"
      "$<$<CONFIG:RelWithDebInfo>:${TANUKI_CXX_FLAGS_RELEASE}>"
      "$<$<CONFIG:MinSizeRel>:${TANUKI_CXX_FLAGS_RELEASE}>"
    )

    # Setup the C++ standard.
    target_compile_features(${arg1} PRIVATE cxx_std_20)
    set_property(TARGET ${arg1} PROPERTY CXX_EXTENSIONS NO)

    # Copy over the Python test runner.
    # NOTE: need to use file(GENERATE) because in multi-config build systems (e.g., MSVC) we need to copy
    # the runner in a directory depending on the config type, and we need to do it at generation time.
    # We can fetch the correct directory by reading the TARGET_FILE_DIR property of the python module.
    file(GENERATE OUTPUT "$<TARGET_FILE_DIR:${arg1}>/run_${arg1}.py" INPUT "${CMAKE_CURRENT_SOURCE_DIR}/run_${arg1}.py")
    # Add the actual test.
    add_test(NAME ${arg1} COMMAND "${Python3_EXECUTABLE}" run_${arg1}.py WORKING_DIRECTORY "$<TARGET_FILE_DIR:${arg1}>")
endfunction()

ADD_TANUKI_PYBIND11_TESTCASE(basic)
