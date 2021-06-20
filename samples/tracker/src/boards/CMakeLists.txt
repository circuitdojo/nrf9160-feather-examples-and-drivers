
if(DEFINED CONFIG_BOARD_CIRCUITDOJO_FEATHER_NRF9160NS)
# Add filesystem setup, etc
FILE(GLOB sources circuitdojo_feather_nrf9160ns/*.c)
target_sources(app PRIVATE ${sources})

# Add battery related code
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../battery/src/)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../battery/src/battery.c)
endif()