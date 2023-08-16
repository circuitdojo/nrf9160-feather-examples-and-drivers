#
# Copyright (c) 2023 Circuit Dojo LLC
#
# SPDX-License-Identifier: Apache-2.0
#

target_include_directories(app PRIVATE .)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_event_manager.c)