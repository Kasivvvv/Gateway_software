set(DEPENDENT_MP_BIN2HEXgateway_abstract_layer_default_93X5tCtw "c:/Program Files/Microchip/xc32/v5.10/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFgateway_abstract_layer_default_93X5tCtw ${CMAKE_CURRENT_LIST_DIR}/../../../../out/gateway_abstract_layer/default.elf)
set(DEPENDENT_TARGET_DIRgateway_abstract_layer_default_93X5tCtw ${CMAKE_CURRENT_LIST_DIR}/../../../../out/gateway_abstract_layer)
set(DEPENDENT_BYPRODUCTSgateway_abstract_layer_default_93X5tCtw ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_93X5tCtw}/${sourceFileNamegateway_abstract_layer_default_93X5tCtw}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_93X5tCtw}/${sourceFileNamegateway_abstract_layer_default_93X5tCtw}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXgateway_abstract_layer_default_93X5tCtw} --image ${DEPENDENT_DEPENDENT_TARGET_ELFgateway_abstract_layer_default_93X5tCtw} --image-generated-c ${sourceFileNamegateway_abstract_layer_default_93X5tCtw}.c --image-generated-h ${sourceFileNamegateway_abstract_layer_default_93X5tCtw}.h --image-copy-mode ${modegateway_abstract_layer_default_93X5tCtw} --image-offset ${addressgateway_abstract_layer_default_93X5tCtw} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_93X5tCtw}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFgateway_abstract_layer_default_93X5tCtw})
add_custom_target(
    dependent_produced_source_artifactgateway_abstract_layer_default_93X5tCtw 
    DEPENDS ${DEPENDENT_TARGET_DIRgateway_abstract_layer_default_93X5tCtw}/${sourceFileNamegateway_abstract_layer_default_93X5tCtw}.c
    )
