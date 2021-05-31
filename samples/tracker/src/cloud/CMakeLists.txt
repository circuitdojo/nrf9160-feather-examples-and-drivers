#
# Copyright (c) 2021 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_include_directories(app PRIVATE .)
add_subdirectory(cloud_codec)
target_sources_ifdef(CONFIG_AWS_IOT app
                     PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/aws_iot_integration.c)
target_sources_ifdef(CONFIG_PYRINAS_CLOUD_ENABLED app
                     PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pyrinas_cloud_integration.c )