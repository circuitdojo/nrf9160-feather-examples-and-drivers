#
# Copyright (c) 2021 Circuit Dojo LLC
#
# SPDX-License-Identifier: LicenseRef-Circuit-Dojo-5-Clause
#

target_include_directories(app PRIVATE .)

# use golioth if set
target_sources_ifdef(CONFIG_GOLIOTH app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/golioth.c)

# use pyrinas if set
target_sources_ifdef(CONFIG_PYRINAS app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/pyrinas.c)