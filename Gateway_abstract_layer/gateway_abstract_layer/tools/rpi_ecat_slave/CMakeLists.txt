cmake_minimum_required(VERSION 3.13)
project(rpi_ecat_slave C)

# Repository root (this file lives in tools/rpi_ecat_slave)
set(REPO ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(SOES ${REPO}/third_party/SOES)
set(DEMO ${SOES}/applications/linux_lan9252demo)

add_executable(rpi_ecat_slave
    main_linux.c
    slave_app.c
    slave_sii.c
    esc_hw_soft.c
    ${REPO}/src/abstract_layer/modules/esc/soft_esc.c
    ${SOES}/soes/esc.c
    ${SOES}/soes/esc_coe.c
    ${SOES}/soes/esc_foe.c
    ${SOES}/soes/esc_eoe.c
    ${SOES}/soes/ecat_slv.c
    ${DEMO}/slave_objectlist.c
)

target_include_directories(rpi_ecat_slave PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${REPO}/src/abstract_layer/modules/esc
    ${SOES}/soes
    ${SOES}/soes/include/sys/gcc
    ${DEMO}
)

set_property(TARGET rpi_ecat_slave PROPERTY C_STANDARD 11)
target_compile_options(rpi_ecat_slave PRIVATE -O2 -Wall)
