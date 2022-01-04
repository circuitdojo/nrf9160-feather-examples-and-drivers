#
# Copyright (c) 2021 Circuit Dojo LLC
#
# SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
#

target_include_directories(app PRIVATE .)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_event_manager.c)