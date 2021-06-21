#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_include_directories(app PRIVATE ${ASSET_TRACKER_PATH}/src/modules/)
target_sources(app PRIVATE ${ASSET_TRACKER_PATH}/src/modules/modules_common.c)
target_sources_ifdef(CONFIG_CLOUD_MODULE app PRIVATE ${ASSET_TRACKER_PATH}/src/modules/cloud_module.c)
target_sources_ifdef(CONFIG_GPS_MODULE app PRIVATE ${ASSET_TRACKER_PATH}/src/modules/gps_module.c)
target_sources_ifdef(CONFIG_UI_MODULE app PRIVATE ${ASSET_TRACKER_PATH}/src/modules/ui_module.c)
target_sources_ifdef(CONFIG_UTIL_MODULE app PRIVATE ${ASSET_TRACKER_PATH}/src/modules/util_module.c)

target_sources_ifdef(CONFIG_DATA_MODULE app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/data_module.c)
target_sources_ifdef(CONFIG_MODEM_MODULE app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/modem_module.c)
target_sources_ifdef(CONFIG_SENSOR_MODULE app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sensor_module.c)
