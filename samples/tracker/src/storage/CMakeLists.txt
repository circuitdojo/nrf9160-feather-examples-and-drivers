#
# Copyright (c) 2023 Circuit Dojo LLC
#
# SPDX-License-Identifier: Apache-2.0
#

target_include_directories(app PRIVATE .)
target_sources_ifdef(CONFIG_FILE_SYSTEM_LITTLEFS app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_storage.c)