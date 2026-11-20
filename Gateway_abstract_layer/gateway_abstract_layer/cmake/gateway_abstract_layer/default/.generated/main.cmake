include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(gateway_abstract_layer_default_library_list )

set(OUTPUT_SUFFIX elf)
# Handle files with suffix s, for group default-XC32
if(gateway_abstract_layer_default_default_XC32_FILE_TYPE_assemble)
add_library(gateway_abstract_layer_default_default_XC32_assemble OBJECT ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_assemble})
    gateway_abstract_layer_default_default_XC32_assemble_rule(gateway_abstract_layer_default_default_XC32_assemble)
    list(APPEND gateway_abstract_layer_default_library_list "$<TARGET_OBJECTS:gateway_abstract_layer_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(gateway_abstract_layer_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(gateway_abstract_layer_default_default_XC32_assembleWithPreprocess OBJECT ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    gateway_abstract_layer_default_default_XC32_assembleWithPreprocess_rule(gateway_abstract_layer_default_default_XC32_assembleWithPreprocess)
    list(APPEND gateway_abstract_layer_default_library_list "$<TARGET_OBJECTS:gateway_abstract_layer_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile)
add_library(gateway_abstract_layer_default_default_XC32_compile OBJECT ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile})
    gateway_abstract_layer_default_default_XC32_compile_rule(gateway_abstract_layer_default_default_XC32_compile)
    list(APPEND gateway_abstract_layer_default_library_list "$<TARGET_OBJECTS:gateway_abstract_layer_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(gateway_abstract_layer_default_default_XC32_compile_cpp OBJECT ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile_cpp})
    gateway_abstract_layer_default_default_XC32_compile_cpp_rule(gateway_abstract_layer_default_default_XC32_compile_cpp)
    list(APPEND gateway_abstract_layer_default_library_list "$<TARGET_OBJECTS:gateway_abstract_layer_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(gateway_abstract_layer_default_default_XC32_FILE_TYPE_dependentObject)
add_library(gateway_abstract_layer_default_default_XC32_dependentObject OBJECT ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_dependentObject})
    gateway_abstract_layer_default_default_XC32_dependentObject_rule(gateway_abstract_layer_default_default_XC32_dependentObject)
    list(APPEND gateway_abstract_layer_default_library_list "$<TARGET_OBJECTS:gateway_abstract_layer_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(gateway_abstract_layer_default_image_FdG4uiJW ${gateway_abstract_layer_default_library_list})

set_target_properties(gateway_abstract_layer_default_image_FdG4uiJW PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${gateway_abstract_layer_default_output_dir}")
target_link_libraries(gateway_abstract_layer_default_image_FdG4uiJW PRIVATE ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_link})
# Add the link options from the rule file.
gateway_abstract_layer_default_link_rule( gateway_abstract_layer_default_image_FdG4uiJW)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex gateway_abstract_layer_default_image_name_hex ${gateway_abstract_layer_default_image_name})
add_custom_target(gateway_abstract_layer_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${gateway_abstract_layer_default_output_dir}/${gateway_abstract_layer_default_image_name}\"
    DEPENDS "${gateway_abstract_layer_default_output_dir}/${gateway_abstract_layer_default_image_name}"
    BYPRODUCTS ${gateway_abstract_layer_default_output_dir}/${gateway_abstract_layer_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(gateway_abstract_layer_default_Bin2Hex gateway_abstract_layer_default_image_FdG4uiJW)




