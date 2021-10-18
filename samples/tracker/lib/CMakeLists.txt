zephyr_compile_definitions(QCBOR_DISABLE_PREFERRED_FLOAT)
zephyr_compile_definitions(QCBOR_DISABLE_FLOAT_HW_USE)
zephyr_compile_definitions(QCBOR_CONFIG_DISABLE_EXP_AND_MANTISSA)

target_include_directories(app PRIVATE qcbor/inc)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qcbor/src/qcbor_decode.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qcbor/src/qcbor_encode.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qcbor/src/qcbor_err_to_str.c)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/qcbor/src/UsefulBuf.c)
