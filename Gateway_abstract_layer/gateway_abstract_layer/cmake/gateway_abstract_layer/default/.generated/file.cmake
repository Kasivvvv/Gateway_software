# The following variables contains the files used by the different stages of the build process.
set(gateway_abstract_layer_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${gateway_abstract_layer_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(gateway_abstract_layer_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${gateway_abstract_layer_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${gateway_abstract_layer_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/app.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/bsp/bsp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/at24/src/drv_at24.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/ethphy/src/dynamic/drv_ethphy.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/ethphy/src/dynamic/drv_extphy_ksz8061.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/gmac/src/dynamic/drv_gmac.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/gmac/src/dynamic/drv_gmac_lib_same7x_v7x.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/miim/src/dynamic/drv_miim.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/driver/miim/src/dynamic/drv_miim_gmac.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/exceptions.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/initialization.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/interrupts.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/libc_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/dhcp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/dns.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/hash_fnv.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/helpers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/oahash.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/tcpip_heap_alloc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/tcpip_heap_internal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/tcpip_helpers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/tcpip_manager.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/tcpip_notify.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/library/tcpip/src/tcpip_packet.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/clk/plib_clk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/efc/plib_efc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/mcan/plib_mcan1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/mpu/plib_mpu.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/nvic/plib_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/pio/plib_pio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/tc/plib_tc0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/twihs/master/plib_twihs0_master.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/peripheral/usart/plib_usart1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/startup_xc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/stdio/xc32_monitor.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/cache/sys_cache.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/int/src/sys_int.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/sys_time_h2_adapter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/system/time/src/sys_time.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/tasks.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/abstract_layer/modules/hal_can.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/abstract_layer/modules/hal_console.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/abstract_layer/modules/hal_marker.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/abstract_layer/modules/hal_time.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/interface/hwa_adapter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/canopen/canopen.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/software_core/core/core.c")
set_source_files_properties(${gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${gateway_abstract_layer_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(gateway_abstract_layer_default_default_XC32_FILE_TYPE_link)

# The linker script used for the build.
set(gateway_abstract_layer_default_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../config.mcc/src/config/default/ATSAMV71Q21B.ld")
set(gateway_abstract_layer_default_image_name "default.elf")
set(gateway_abstract_layer_default_image_base_name "default")

# The output directory of the final image.
set(gateway_abstract_layer_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/gateway_abstract_layer")

# The full path to the final image.
set(gateway_abstract_layer_default_full_path_to_image ${gateway_abstract_layer_default_output_dir}/${gateway_abstract_layer_default_image_name})
