# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "/Users/kasiawcislo/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(gateway_abstract_layer_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=ATSAMV71Q21B"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.13.257/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(gateway_abstract_layer_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.13.257/samv71b"
        "-mprocessor=ATSAMV71Q21B"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST}")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
function(gateway_abstract_layer_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=ATSAMV71Q21B"
        "-ffunction-sections"
        "-fdata-sections"
        "-O1"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.13.257/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "HAVE_CONFIG_H=1"
        PRIVATE "LELY_NO_MALLOC=1"
        PRIVATE "LELY_HAVE_TIMESPEC=1"
        PRIVATE "LELY_HAVE_ITIMERSPEC=1"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default"
        PRIVATE "config.mcc/src/config/default/library"
        PRIVATE "config.mcc/src/config/default/library/tcpip/src"
        PRIVATE "config.mcc/src/config/default/library/tcpip/src/common"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/packs/ATSAMV71Q21B_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/abstract_layer/interface"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/interface"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/canopen"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/canopen/config"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/core"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/ethercat"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/interface"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/lely-core/include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/lely-core/lib/can"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/lely-core/lib/co"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/lely-core/lib/compat"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/lely-core/lib/util"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/SOEM/soem"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/SOEM/osal"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/ethercat/soem_port"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.3.0/CMSIS/Core/Include")
endfunction()
function(gateway_abstract_layer_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-mprocessor=ATSAMV71Q21B"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-ffunction-sections"
        "-O1"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.13.257/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/packs/ATSAMV71Q21B_DFP"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/packs/CMSIS"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/packs/CMSIS/CMSIS/Core/Include"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../src"
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/lely-core/include"
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/6.3.0/CMSIS/Core/Include")
endfunction()
function(gateway_abstract_layer_default_dependentObject_rule target)
    set(options
        "-mprocessor=ATSAMV71Q21B"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.13.257/samv71b")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(gateway_abstract_layer_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "-mprocessor=ATSAMV71Q21B"
        "-mno-device-startup-code"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--script=${gateway_abstract_layer_default_LINKER_SCRIPT},--defsym=_min_heap_size=512,--gc-sections,-Map=mem.map,--report-mem,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAMV71_DFP/4.13.257/samv71b")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
