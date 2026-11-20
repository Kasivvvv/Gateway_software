set(DEPENDENT_MP_BIN2HEXgateway_abstract_layer_default_FdG4uiJW "/Applications/microchip/xc32/v5.10/bin/xc32-bin2hex")
set(DEPENDENT_DEPENDENT_TARGET_ELFgateway_abstract_layer_default_FdG4uiJW ${CMAKE_CURRENT_LIST_DIR}/../../../../out/gateway_abstract_layer/default.elf)
set(DEPENDENT_TARGET_DIRgateway_abstract_layer_default_FdG4uiJW ${CMAKE_CURRENT_LIST_DIR}/../../../../out/gateway_abstract_layer)
set(DEPENDENT_BYPRODUCTSgateway_abstract_layer_default_FdG4uiJW ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_FdG4uiJW}/${sourceFileNamegateway_abstract_layer_default_FdG4uiJW}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_FdG4uiJW}/${sourceFileNamegateway_abstract_layer_default_FdG4uiJW}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXgateway_abstract_layer_default_FdG4uiJW} --image ${DEPENDENT_DEPENDENT_TARGET_ELFgateway_abstract_layer_default_FdG4uiJW} --image-generated-c ${sourceFileNamegateway_abstract_layer_default_FdG4uiJW}.c --image-generated-h ${sourceFileNamegateway_abstract_layer_default_FdG4uiJW}.h --image-copy-mode ${modegateway_abstract_layer_default_FdG4uiJW} --image-offset ${addressgateway_abstract_layer_default_FdG4uiJW} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_FdG4uiJW}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFgateway_abstract_layer_default_FdG4uiJW})
add_custom_target(
    dependent_produced_source_artifactgateway_abstract_layer_default_FdG4uiJW 
    DEPENDS ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_FdG4uiJW}/${sourceFileNamegateway_abstract_layer_default_FdG4uiJW}.c
    )
