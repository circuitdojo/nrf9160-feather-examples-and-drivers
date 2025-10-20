#
# Copyright (c) 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(serial_lte_modem)

# NORDIC SDK APP START
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/main.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_util.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_settings.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_host.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_commands.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_socket.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_tcp_proxy.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_udp_proxy.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_icmp.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_fota.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_uart_handler.c)
target_sources(app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_ctrl_pin.c)

# NORDIC SDK APP END
target_sources_ifdef(CONFIG_SLM_SMS app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_at_sms.c)
target_sources_ifdef(CONFIG_SLM_PPP app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_ppp.c)
target_sources_ifdef(CONFIG_SLM_CMUX app PRIVATE ../../../nrf/applications/serial_lte_modem/src/slm_cmux.c)

add_subdirectory_ifdef(CONFIG_SLM_GNSS ../../../nrf/applications/serial_lte_modem/src/gnss ${CMAKE_CURRENT_BINARY_DIR}/gnss)
add_subdirectory_ifdef(CONFIG_SLM_NRF_CLOUD ../../../nrf/applications/serial_lte_modem/src/nrfcloud ${CMAKE_CURRENT_BINARY_DIR}/nrfcloud)
add_subdirectory_ifdef(CONFIG_SLM_MQTTC ../../../nrf/applications/serial_lte_modem/src/mqtt_c ${CMAKE_CURRENT_BINARY_DIR}/mqtt_c)
add_subdirectory_ifdef(CONFIG_SLM_HTTPC ../../../nrf/applications/serial_lte_modem/src/http_c ${CMAKE_CURRENT_BINARY_DIR}/http_c)
add_subdirectory_ifdef(CONFIG_SLM_TWI ../../../nrf/applications/serial_lte_modem/src/twi ${CMAKE_CURRENT_BINARY_DIR}/twi)
add_subdirectory_ifdef(CONFIG_SLM_GPIO ../../../nrf/applications/serial_lte_modem/src/gpio ${CMAKE_CURRENT_BINARY_DIR}/gpio)
add_subdirectory_ifdef(CONFIG_SLM_CARRIER ../../../nrf/applications/serial_lte_modem/src/lwm2m_carrier ${CMAKE_CURRENT_BINARY_DIR}/lwm2m_carrier)
add_subdirectory_ifdef(CONFIG_SLM_NATIVE_TLS ../../../nrf/applications/serial_lte_modem/src/nativetls ${CMAKE_CURRENT_BINARY_DIR}/nativetls)
if (CONFIG_SLM_FTPC OR CONFIG_SLM_TFTPC)
        add_subdirectory(../../../nrf/applications/serial_lte_modem/src/ftp_c ${CMAKE_CURRENT_BINARY_DIR}/ftp_c)
endif()

zephyr_include_directories(../../../nrf/applications/serial_lte_modem/src)
